#include "vulkanlocalaxesshader.h"

#include "axessystemdefaultgeometry.h"
#include "rklocalaxes.h"
#include "skboundingbox.h"
#include "skcell.h"
#include "vulkanshader.h"

#include <cstddef>

namespace
{
VkVertexInputAttributeDescription localAxesAttribute(uint32_t location, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = 0;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}

AxesSystemDefaultGeometry makeLocalAxesGeometry(RKRenderObject *object, RKRenderLocalAxesSource *axes)
{
  double length = axes->renderLocalAxes().length();
  double width = axes->renderLocalAxes().width();

  std::shared_ptr<SKCell> unitCell = object->cell();
  SKBoundingBox boundingBox = unitCell->boundingBox();

  switch (axes->renderLocalAxes().scalingType())
  {
  case RKLocalAxes::ScalingType::absolute:
    break;
  case RKLocalAxes::ScalingType::relative:
    length = boundingBox.shortestEdge() * axes->renderLocalAxes().length() / 100.0;
    break;
  default:
    break;
  }

  switch (axes->renderLocalAxes().style())
  {
  case RKLocalAxes::Style::defaultStyle:
    return AxesSystemDefaultGeometry(RKGlobalAxes::CenterType::cube, width, float4(1.0, 1.0, 1.0, 1.0), length, width,
                                     float4(1.0, 0.4, 0.7, 1.0), float4(0.7, 1.0, 0.4, 1.0), float4(0.4, 0.7, 1.0, 1.0), 0.0,
                                     1.0, float4(1.0, 0.4, 0.7, 1.0), float4(0.7, 1.0, 0.4, 1.0), float4(0.4, 0.7, 1.0, 1.0),
                                     false, 1.0, 4);
  case RKLocalAxes::Style::defaultStyleRGB:
    return AxesSystemDefaultGeometry(RKGlobalAxes::CenterType::cube, width, float4(1.0, 1.0, 1.0, 1.0), length, width,
                                     float4(1.0, 0.0, 0.0, 1.0), float4(0.0, 1.0, 0.0, 1.0), float4(0.0, 0.0, 1.0, 1.0), 0.0,
                                     1.0, float4(1.0, 0.0, 0.0, 1.0), float4(0.0, 1.0, 0.0, 1.0), float4(0.0, 0.0, 1.0, 1.0),
                                     false, 1.0, 4);
  case RKLocalAxes::Style::cylinder:
    return AxesSystemDefaultGeometry(RKGlobalAxes::CenterType::cube, width, float4(1.0, 1.0, 1.0, 1.0), length, width,
                                     float4(1.0, 0.4, 0.0, 7.0), float4(0.7, 1.0, 0.4, 1.0), float4(0.4, 0.7, 1.0, 1.0), 0.0,
                                     1.0, float4(1.0, 0.4, 0.7, 1.0), float4(0.7, 1.0, 0.4, 1.0), float4(0.4, 0.7, 1.0, 1.0),
                                     false, 1.0, 41);
  case RKLocalAxes::Style::cylinderRGB:
    return AxesSystemDefaultGeometry(RKGlobalAxes::CenterType::cube, width, float4(1.0, 1.0, 1.0, 1.0), length, width,
                                     float4(1.0, 0.0, 0.0, 1.0), float4(0.0, 1.0, 0.0, 1.0), float4(0.0, 0.0, 1.0, 1.0), 0.0,
                                     1.0, float4(1.0, 0.0, 0.0, 1.0), float4(0.0, 1.0, 0.0, 1.0), float4(0.0, 0.0, 1.0, 1.0),
                                     false, 1.0, 41);
  default:
    return AxesSystemDefaultGeometry();
  }
}
}  // namespace

VulkanLocalAxesShader::VulkanLocalAxesShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanLocalAxesShader::~VulkanLocalAxesShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyStructureBuffers();
  if (_pipeline)
  {
    vkDestroyPipeline(_renderer->device(), _pipeline, nullptr);
  }
}

void VulkanLocalAxesShader::destroyStructureBuffers()
{
  for (auto &scene : _structureBuffers)
  {
    for (auto &buffers : scene)
    {
      _renderer->destroyBuffer(buffers.vertexBuffer);
      _renderer->destroyBuffer(buffers.indexBuffer);
    }
  }
  _structureBuffers.clear();
}

void VulkanLocalAxesShader::initialize()
{
  VulkanShader::PipelineConfig config;
  config.vertexShaderResource = QStringLiteral(":/shaders/local_axes.vert.spv");
  config.fragmentShaderResource = QStringLiteral(":/shaders/local_axes.frag.spv");
  config.cullMode = VK_CULL_MODE_BACK_BIT;
  config.depthTest = true;
  config.depthWrite = true;
  config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkVertexInputBindingDescription vertexBinding{};
  vertexBinding.binding = 0;
  vertexBinding.stride = sizeof(RKPrimitiveVertex);
  vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  config.bindings = {vertexBinding};
  config.attributes = {
      localAxesAttribute(0, offsetof(RKPrimitiveVertex, position)),
      localAxesAttribute(1, offsetof(RKPrimitiveVertex, normal)),
      localAxesAttribute(2, offsetof(RKPrimitiveVertex, color)),
  };
  _pipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), config);
}

void VulkanLocalAxesShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  reloadData();
}

void VulkanLocalAxesShader::reloadData()
{
  _renderer->waitIdle();
  destroyStructureBuffers();
  _structureBuffers.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureBuffers[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *object = dynamic_cast<RKRenderObject *>(_renderStructures[i][j].get());
      auto *axes = dynamic_cast<RKRenderLocalAxesSource *>(_renderStructures[i][j].get());
      if (!object || !axes || !object->cell())
      {
        continue;
      }

      AxesSystemDefaultGeometry geometry = makeLocalAxesGeometry(object, axes);
      const auto &vertices = geometry.vertices();
      const auto &indices = geometry.indices();
      _structureBuffers[i][j].indexCount = static_cast<uint32_t>(indices.size());
      if (!vertices.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].vertexBuffer, vertices.data(),
                                vertices.size() * sizeof(RKPrimitiveVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
      }
      if (!indices.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].indexBuffer, indices.data(), indices.size() * sizeof(short),
                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
      }
    }
  }
}

void VulkanLocalAxesShader::paint(VkCommandBuffer commandBuffer)
{
  if (!_pipeline)
  {
    return;
  }

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *axes = dynamic_cast<RKRenderLocalAxesSource *>(object);
      const StructureBuffers &buffers = _structureBuffers[i][j];
      if (object && object->isVisible() && axes && axes->renderLocalAxes().position() != RKLocalAxes::Position::none &&
          buffers.indexCount > 0 && buffers.vertexBuffer.buffer && buffers.indexBuffer.buffer)
      {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
        _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers.vertexBuffer.buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, buffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandBuffer, buffers.indexCount, 1, 0, 0, 0);
      }
      ++flatIndex;
    }
  }
}
