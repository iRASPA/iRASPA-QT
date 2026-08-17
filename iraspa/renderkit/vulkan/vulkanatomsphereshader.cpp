#include "vulkanatomsphereshader.h"

#include "quadgeometry.h"
#include "rkrenderkitprotocols.h"
#include "rkimposters.h"
#include "vulkanatomambientocclusionshader.h"
#include "vulkanshader.h"

#include <cstddef>

namespace
{
VkVertexInputAttributeDescription float4Attribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}

void fillAtomImposterBindings(VulkanShader::PipelineConfig &config)
{
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
      float4Attribute(0, 0, offsetof(RKVertex, position)),
      float4Attribute(1, 1, offsetof(RKInPerInstanceAttributesAtoms, position)),
      float4Attribute(2, 1, offsetof(RKInPerInstanceAttributesAtoms, scale)),
      float4Attribute(3, 1, offsetof(RKInPerInstanceAttributesAtoms, ambient)),
      float4Attribute(4, 1, offsetof(RKInPerInstanceAttributesAtoms, diffuse)),
      float4Attribute(5, 1, offsetof(RKInPerInstanceAttributesAtoms, specular)),
  };
}
}  // namespace

VulkanAtomSphereShader::VulkanAtomSphereShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanAtomSphereShader::~VulkanAtomSphereShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  _aoShader.reset();
  destroyStructureBuffers();
  auto destroyPipeline = [this](VkPipeline &pipeline) {
    if (pipeline)
    {
      vkDestroyPipeline(_renderer->device(), pipeline, nullptr);
      pipeline = VK_NULL_HANDLE;
    }
  };
  destroyPipeline(_orthoImposterPipeline);
  destroyPipeline(_perspImposterPipeline);
  destroyPipeline(_orthoImposterPerPixelPipeline);
  destroyPipeline(_perspImposterPerPixelPipeline);
  _renderer->destroyBuffer(_imposterVertexBuffer);
  _renderer->destroyBuffer(_imposterIndexBuffer);
}

void VulkanAtomSphereShader::destroyStructureBuffers()
{
  for (auto &scene : _structureBuffers)
  {
    for (auto &buffers : scene)
    {
      _renderer->destroyBuffer(buffers.instanceBuffer);
    }
  }
  _structureBuffers.clear();
}

void VulkanAtomSphereShader::initialize()
{
  QuadGeometry quad;
  const auto &quadVertices = quad.vertices();
  const auto &quadIndices = quad.indices();
  _imposterIndexCount = static_cast<uint32_t>(quadIndices.size());
  _renderer->uploadBuffer(_imposterVertexBuffer, quadVertices.data(), quadVertices.size() * sizeof(RKVertex),
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  _renderer->uploadBuffer(_imposterIndexBuffer, quadIndices.data(), quadIndices.size() * sizeof(short),
                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  VulkanShader::PipelineConfig orthoConfig;
  orthoConfig.vertexShaderResource = QStringLiteral(":/shaders/atom_ortho_imposter.vert.spv");
  orthoConfig.fragmentShaderResource = QStringLiteral(":/shaders/atom_ortho_imposter.frag.spv");
  orthoConfig.cullMode = VK_CULL_MODE_NONE;
  orthoConfig.depthTest = true;
  orthoConfig.depthWrite = true;
  orthoConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  fillAtomImposterBindings(orthoConfig);
  orthoConfig.sampleShading = true;
  _orthoImposterPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), orthoConfig);
  orthoConfig.sampleShading = false;
  _orthoImposterPerPixelPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), orthoConfig);

  VulkanShader::PipelineConfig perspConfig = orthoConfig;
  perspConfig.vertexShaderResource = QStringLiteral(":/shaders/atom_persp_imposter.vert.spv");
  perspConfig.fragmentShaderResource = QStringLiteral(":/shaders/atom_persp_imposter.frag.spv");
  perspConfig.sampleShading = true;
  _perspImposterPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), perspConfig);
  perspConfig.sampleShading = false;
  _perspImposterPerPixelPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), perspConfig);

  _aoSamplerSet = _renderer->allocateSamplerDescriptorSet(_renderer->whiteTexture());
  _aoShader = std::make_unique<VulkanAtomAmbientOcclusionShader>(_renderer, this);
  _aoShader->initialize();
}

void VulkanAtomSphereShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  if (_aoShader)
  {
    _aoShader->setRenderStructures(_renderStructures);
  }
  reloadData();
}

void VulkanAtomSphereShader::reloadData()
{
  _renderer->waitIdle();
  destroyStructureBuffers();
  _structureBuffers.resize(_renderStructures.size());
  _numberOfAtoms = 0;

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureBuffers[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *source = dynamic_cast<RKRenderAtomSource *>(_renderStructures[i][j].get());
      if (!source)
      {
        continue;
      }
      const std::vector<RKInPerInstanceAttributesAtoms> atomData = source->renderAtoms();
      _structureBuffers[i][j].instanceCount = static_cast<uint32_t>(atomData.size());
      if (source->drawAtoms() && _renderStructures[i][j]->isVisible())
      {
        _numberOfAtoms += static_cast<uint32_t>(atomData.size());
      }
      if (!atomData.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].instanceBuffer, atomData.data(),
                                atomData.size() * sizeof(RKInPerInstanceAttributesAtoms), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
      }
    }
  }
}

void VulkanAtomSphereShader::reloadAmbientOcclusionData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality)
{
  if (_aoShader)
  {
    _aoShader->reloadData(dataSource, quality);
  }
}

void VulkanAtomSphereShader::invalidateCachedAmbientOcclusionTexture(const std::vector<std::shared_ptr<RKRenderObject>> &structures)
{
  if (_aoShader)
  {
    _aoShader->invalidateCachedAmbientOcclusionTexture(structures);
  }
}

uint32_t VulkanAtomSphereShader::instanceCount(size_t sceneIndex, size_t movieIndex) const
{
  if (sceneIndex >= _structureBuffers.size() || movieIndex >= _structureBuffers[sceneIndex].size())
  {
    return 0;
  }
  return _structureBuffers[sceneIndex][movieIndex].instanceCount;
}

VkBuffer VulkanAtomSphereShader::instanceBuffer(size_t sceneIndex, size_t movieIndex) const
{
  if (sceneIndex >= _structureBuffers.size() || movieIndex >= _structureBuffers[sceneIndex].size())
  {
    return VK_NULL_HANDLE;
  }
  return _structureBuffers[sceneIndex][movieIndex].instanceBuffer.buffer;
}

void VulkanAtomSphereShader::paintInstances(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkBuffer vertexBuffer,
                                            VkBuffer indexBuffer, uint32_t indexCount, bool bindAmbientOcclusion)
{
  if (!pipeline || indexCount == 0)
  {
    return;
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkDeviceSize vertexOffset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &vertexOffset);
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      const StructureBuffers &buffers = _structureBuffers[i][j];
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *source = dynamic_cast<RKRenderAtomSource *>(object);
      if (buffers.instanceCount > 0 && source && source->drawAtoms() && object->isVisible())
      {
        if (bindAmbientOcclusion)
        {
          VkDescriptorSet aoSet = _aoSamplerSet;
          if (_aoShader)
          {
            if (VkDescriptorSet generated = _aoShader->samplerSet(i, j))
            {
              aoSet = generated;
            }
          }
          _renderer->bindSamplerDescriptorSet(commandBuffer, _renderer->scenePipelineLayout(), aoSet);
        }
        _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
        VkDeviceSize instanceOffset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 1, 1, &buffers.instanceBuffer.buffer, &instanceOffset);
        vkCmdDrawIndexed(commandBuffer, indexCount, buffers.instanceCount, 0, 0, 0);
      }
      ++flatIndex;
    }
  }
}

void VulkanAtomSphereShader::paint(VkCommandBuffer commandBuffer, RKRenderQuality quality, bool orthographic)
{
  const bool perSample = perSampleImposterShading(quality);
  if (orthographic)
  {
    paintInstances(commandBuffer, perSample ? _orthoImposterPipeline : _orthoImposterPerPixelPipeline,
                   _imposterVertexBuffer.buffer, _imposterIndexBuffer.buffer, _imposterIndexCount, true);
  }
  else
  {
    paintInstances(commandBuffer, perSample ? _perspImposterPipeline : _perspImposterPerPixelPipeline,
                   _imposterVertexBuffer.buffer, _imposterIndexBuffer.buffer, _imposterIndexCount, true);
  }
}

void VulkanAtomSphereShader::paintPick(VkCommandBuffer commandBuffer, VkPipeline pipeline)
{
  paintInstances(commandBuffer, pipeline, _imposterVertexBuffer.buffer, _imposterIndexBuffer.buffer, _imposterIndexCount, false);
}
