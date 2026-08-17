#include "vulkanmeasurementshader.h"

#include "quadgeometry.h"
#include "vulkanshader.h"

#include <algorithm>
#include <cstddef>

namespace
{
VkVertexInputAttributeDescription floatAttribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}

VulkanShader::PipelineConfig overlayConfig()
{
  VulkanShader::PipelineConfig config;
  config.depthTest = true;
  config.depthWrite = false;
  config.blend = true;
  config.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  config.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  config.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  config.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  return config;
}
}  // namespace

VulkanMeasurementShader::VulkanMeasurementShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanMeasurementShader::~VulkanMeasurementShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyInstanceBuffer();
  destroyPipelines();
  _renderer->destroyBuffer(_quadVertexBuffer);
  _renderer->destroyBuffer(_quadIndexBuffer);
}

void VulkanMeasurementShader::destroyPipelines()
{
  auto destroy = [this](VkPipeline &pipeline) {
    if (pipeline)
    {
      vkDestroyPipeline(_renderer->device(), pipeline, nullptr);
      pipeline = VK_NULL_HANDLE;
    }
  };
  destroy(_orthoPipeline);
  destroy(_perspPipeline);
}

void VulkanMeasurementShader::destroyInstanceBuffer()
{
  if (_renderer)
  {
    _renderer->destroyBuffer(_instanceBuffer);
  }
  _structureIndices.clear();
}

VkPipeline VulkanMeasurementShader::createPipeline(const QString &vertexResource)
{
  VulkanShader::PipelineConfig config = overlayConfig();
  config.vertexShaderResource = vertexResource;
  config.fragmentShaderResource = QStringLiteral(":/shaders/atom_measurement_imposter.frag.spv");
  config.cullMode = VK_CULL_MODE_NONE;
  config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

  VkVertexInputBindingDescription vertexBinding{};
  vertexBinding.binding = 0;
  vertexBinding.stride = sizeof(RKVertex);
  vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputBindingDescription instanceBinding{};
  instanceBinding.binding = 1;
  instanceBinding.stride = sizeof(RKInPerInstanceAttributesAtoms);
  instanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  config.bindings = {vertexBinding, instanceBinding};
  config.attributes = {
      floatAttribute(0, 0, offsetof(RKVertex, position)),
      floatAttribute(1, 1, offsetof(RKInPerInstanceAttributesAtoms, position)),
      floatAttribute(2, 1, offsetof(RKInPerInstanceAttributesAtoms, scale)),
      floatAttribute(3, 1, offsetof(RKInPerInstanceAttributesAtoms, ambient)),
      floatAttribute(4, 1, offsetof(RKInPerInstanceAttributesAtoms, diffuse)),
      floatAttribute(5, 1, offsetof(RKInPerInstanceAttributesAtoms, specular)),
  };
  return VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), config);
}

void VulkanMeasurementShader::initialize()
{
  QuadGeometry quad;
  _quadIndexCount = static_cast<uint32_t>(quad.indices().size());
  if (!quad.vertices().empty())
  {
    _renderer->uploadBuffer(_quadVertexBuffer, quad.vertices().data(), quad.vertices().size() * sizeof(RKVertex),
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
  if (!quad.indices().empty())
  {
    _renderer->uploadBuffer(_quadIndexBuffer, quad.indices().data(), quad.indices().size() * sizeof(short),
                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  }

  _orthoPipeline = createPipeline(QStringLiteral(":/shaders/atom_measurement_ortho_imposter.vert.spv"));
  _perspPipeline = createPipeline(QStringLiteral(":/shaders/atom_measurement_persp_imposter.vert.spv"));
}

void VulkanMeasurementShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>>)
{
}

void VulkanMeasurementShader::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = std::move(source);
}

void VulkanMeasurementShader::reloadData()
{
  destroyInstanceBuffer();
  if (!_renderer || !_dataSource)
  {
    return;
  }

  const std::vector<RKInPerInstanceAttributesAtoms> points = _dataSource->renderMeasurementPoints();
  _structureIndices.reserve(points.size());
  for (const RKInPerInstanceAttributesAtoms &point : points)
  {
    _structureIndices.push_back(static_cast<uint32_t>(std::max(point.tag, 0)));
  }

  if (!points.empty())
  {
    _renderer->uploadBuffer(_instanceBuffer, points.data(), points.size() * sizeof(RKInPerInstanceAttributesAtoms),
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
}

void VulkanMeasurementShader::paint(VkCommandBuffer commandBuffer, bool orthographic)
{
  VkPipeline pipeline = orthographic ? _orthoPipeline : _perspPipeline;
  if (!pipeline || _structureIndices.empty() || _quadIndexCount == 0 || !_quadVertexBuffer.buffer || !_quadIndexBuffer.buffer ||
      !_instanceBuffer.buffer)
  {
    return;
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  VkDeviceSize vertexOffset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_quadVertexBuffer.buffer, &vertexOffset);
  vkCmdBindIndexBuffer(commandBuffer, _quadIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

  for (size_t k = 0; k < _structureIndices.size(); ++k)
  {
    _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), _structureIndices[k]);
    VkDeviceSize instanceOffset = k * sizeof(RKInPerInstanceAttributesAtoms);
    vkCmdBindVertexBuffers(commandBuffer, 1, 1, &_instanceBuffer.buffer, &instanceOffset);
    vkCmdDrawIndexed(commandBuffer, _quadIndexCount, 1, 0, 0, 0);
  }
}
