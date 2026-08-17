#include "vulkanribbonshader.h"

#include "rkribbonmesh.h"
#include "vulkanribbonambientocclusionshader.h"
#include "vulkanshader.h"

#include <algorithm>
#include <cstddef>
#include <set>

namespace
{
VkVertexInputAttributeDescription ribbonAttribute(uint32_t location, VkFormat format, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = 0;
  description.format = format;
  description.offset = offset;
  return description;
}
}  // namespace

VulkanRibbonShader::VulkanRibbonShader(VulkanRenderer *renderer, VulkanAtomSphereShader *atomShader)
    : _renderer(renderer), _atomShader(atomShader)
{
}

VulkanRibbonShader::~VulkanRibbonShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  _aoShader.reset();
  destroyStructureBuffers();
  if (_pipeline)
  {
    vkDestroyPipeline(_renderer->device(), _pipeline, nullptr);
  }
}

void VulkanRibbonShader::destroyStructureBuffers()
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

void VulkanRibbonShader::initialize()
{
  VulkanShader::PipelineConfig config;
  config.vertexShaderResource = QStringLiteral(":/shaders/ribbon.vert.spv");
  config.fragmentShaderResource = QStringLiteral(":/shaders/ribbon.frag.spv");
  config.cullMode = VK_CULL_MODE_NONE;
  config.depthTest = true;
  config.depthWrite = true;
  config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkVertexInputBindingDescription vertexBinding{};
  vertexBinding.binding = 0;
  vertexBinding.stride = sizeof(RKVertex);
  vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  config.bindings = {vertexBinding};
  config.attributes = {
      ribbonAttribute(0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(RKVertex, position)),
      ribbonAttribute(1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(RKVertex, normal)),
      ribbonAttribute(2, VK_FORMAT_R32G32_SFLOAT, offsetof(RKVertex, st)),
      ribbonAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(RKVertex, pad)),
  };
  _pipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), config);
  _aoSamplerSet = _renderer->allocateSamplerDescriptorSet(_renderer->whiteTexture());
  _aoShader = std::make_unique<VulkanRibbonAmbientOcclusionShader>(_renderer, this, _atomShader);
  _aoShader->initialize();
}

void VulkanRibbonShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  if (_aoShader)
  {
    _aoShader->setRenderStructures(_renderStructures);
  }
  reloadData();
}

void VulkanRibbonShader::reloadData()
{
  _renderer->waitIdle();
  destroyStructureBuffers();
  _structureBuffers.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureBuffers[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *ribbonSource = dynamic_cast<RKRenderRibbonSource *>(_renderStructures[i][j].get());
      if (!ribbonSource)
      {
        continue;
      }

      const std::vector<RKVertex> vertices = ribbonSource->renderRibbonVertices();
      const std::vector<uint32_t> indices = ribbonSource->renderRibbonIndices();
      _structureBuffers[i][j].indexCount = static_cast<uint32_t>(indices.size());
      if (!vertices.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].vertexBuffer, vertices.data(), vertices.size() * sizeof(RKVertex),
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
      }
      if (!indices.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].indexBuffer, indices.data(), indices.size() * sizeof(uint32_t),
                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
      }
    }
  }
}

void VulkanRibbonShader::reloadAmbientOcclusionData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality)
{
  if (_aoShader)
  {
    _aoShader->reloadData(dataSource, quality);
  }
}

void VulkanRibbonShader::invalidateCachedAmbientOcclusionTexture(const std::vector<std::shared_ptr<RKRenderObject>> &structures)
{
  if (_aoShader)
  {
    _aoShader->invalidateCachedAmbientOcclusionTexture(structures);
  }
}

uint32_t VulkanRibbonShader::indexCount(size_t sceneIndex, size_t movieIndex) const
{
  if (sceneIndex >= _structureBuffers.size() || movieIndex >= _structureBuffers[sceneIndex].size())
  {
    return 0;
  }
  return _structureBuffers[sceneIndex][movieIndex].indexCount;
}

VkBuffer VulkanRibbonShader::vertexBuffer(size_t sceneIndex, size_t movieIndex) const
{
  if (sceneIndex >= _structureBuffers.size() || movieIndex >= _structureBuffers[sceneIndex].size())
  {
    return VK_NULL_HANDLE;
  }
  return _structureBuffers[sceneIndex][movieIndex].vertexBuffer.buffer;
}

VkBuffer VulkanRibbonShader::indexBuffer(size_t sceneIndex, size_t movieIndex) const
{
  if (sceneIndex >= _structureBuffers.size() || movieIndex >= _structureBuffers[sceneIndex].size())
  {
    return VK_NULL_HANDLE;
  }
  return _structureBuffers[sceneIndex][movieIndex].indexBuffer.buffer;
}

void VulkanRibbonShader::drawAllChains(VkCommandBuffer commandBuffer, RKRenderRibbonSource *ribbonSource) const
{
  if (!ribbonSource)
  {
    return;
  }
  for (const RKRibbonChainDrawRange &chainRange : ribbonSource->ribbonChainDrawRanges())
  {
    if (chainRange.indexCount > 0)
    {
      vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(chainRange.indexCount), 1,
                       static_cast<uint32_t>(chainRange.indexStart), 0, 0);
    }
  }
}

void VulkanRibbonShader::drawRibbonRanges(VkCommandBuffer commandBuffer, RKRenderRibbonSource *ribbonSource) const
{
  std::vector<RKRibbonChainDrawRange> drawRanges;
  RibbonDrawVisibilityMode visibilityMode = RibbonDrawVisibilityMode::none;

  if (ribbonSource->ribbonUsesResidueVisibility() && !ribbonSource->ribbonResidueDrawRanges().empty())
  {
    drawRanges = ribbonSource->ribbonResidueDrawRanges();
    visibilityMode = RibbonDrawVisibilityMode::residue;
  }
  else if (ribbonSource->ribbonUsesSegmentVisibility() && !ribbonSource->ribbonSegmentDrawRanges().empty())
  {
    drawRanges = ribbonSource->ribbonSegmentDrawRanges();
    visibilityMode = RibbonDrawVisibilityMode::segment;
  }
  else
  {
    drawRanges = ribbonSource->ribbonChainDrawRanges();
  }

  for (size_t rangeIndex = 0; rangeIndex < drawRanges.size(); ++rangeIndex)
  {
    const RKRibbonChainDrawRange &chainRange = drawRanges[rangeIndex];
    if (chainRange.indexCount <= 0)
    {
      continue;
    }
    switch (visibilityMode)
    {
    case RibbonDrawVisibilityMode::residue:
      if (!ribbonSource->isRibbonResidueDrawRangeVisible(static_cast<int>(rangeIndex)))
      {
        continue;
      }
      break;
    case RibbonDrawVisibilityMode::segment:
      if (!ribbonSource->isRibbonSegmentDrawRangeVisible(static_cast<int>(rangeIndex)))
      {
        continue;
      }
      break;
    case RibbonDrawVisibilityMode::none:
      break;
    }
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(chainRange.indexCount), 1,
                     static_cast<uint32_t>(chainRange.indexStart), 0, 0);
  }
}

void VulkanRibbonShader::drawSelectedRibbonRanges(VkCommandBuffer commandBuffer, RKRenderRibbonSource *ribbonSource) const
{
  const std::set<int> segmentIndices = ribbonSource->renderSelectedRibbonSegmentDrawRangeIndices();
  const std::set<int> residueIndices = ribbonSource->renderSelectedRibbonResidueDrawRangeIndices();
  const std::vector<RKRibbonChainDrawRange> segmentRanges = ribbonSource->ribbonSegmentDrawRanges();
  const std::vector<RKRibbonChainDrawRange> residueRanges = ribbonSource->ribbonResidueDrawRanges();

  auto drawRange = [&](const RKRibbonChainDrawRange &chainRange) {
    if (chainRange.indexCount <= 0)
    {
      return;
    }
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(chainRange.indexCount), 1,
                     static_cast<uint32_t>(chainRange.indexStart), 0, 0);
  };

  for (int segmentIndex : segmentIndices)
  {
    if (segmentIndex < 0 || segmentIndex >= static_cast<int>(segmentRanges.size()))
    {
      continue;
    }
    if (ribbonSource->ribbonUsesSegmentVisibility() && !ribbonSource->isRibbonSegmentDrawRangeVisible(segmentIndex))
    {
      continue;
    }
    drawRange(segmentRanges[static_cast<size_t>(segmentIndex)]);
  }

  for (int residueIndex : residueIndices)
  {
    if (residueIndex < 0 || residueIndex >= static_cast<int>(residueRanges.size()))
    {
      continue;
    }
    if (ribbonSource->ribbonUsesResidueVisibility() && !ribbonSource->isRibbonResidueDrawRangeVisible(residueIndex))
    {
      continue;
    }
    drawRange(residueRanges[static_cast<size_t>(residueIndex)]);
  }
}

RibbonAODebugUniforms VulkanRibbonShader::debugUniforms(int viewportWidth, int viewportHeight) const
{
  RibbonAODebugUniforms uniforms;
  uniforms.mode = static_cast<int32_t>(_aoDebugMode);
  uniforms.viewportWidth = viewportWidth;
  uniforms.viewportHeight = viewportHeight;
  uniforms.textureWidth = 1;
  uniforms.textureHeight = 1;
  uniforms.inverseTextureSize = 1.0f;

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *ribbonSource = dynamic_cast<RKRenderRibbonSource *>(_renderStructures[i][j].get());
      if (!ribbonSource || !ribbonSource->drawRibbon() || !_renderStructures[i][j]->isVisible())
      {
        continue;
      }
      const int width = std::max(ribbonSource->ribbonAmbientOcclusionTextureWidth(), 1);
      uniforms.textureWidth = width;
      uniforms.textureHeight = std::max(ribbonSource->ribbonAmbientOcclusionTextureHeight(), 1);
      uniforms.patchNumber = std::max(ribbonSource->ribbonAmbientOcclusionPatchNumber(), 1);
      uniforms.patchSize = static_cast<float>(ribbonSource->ribbonAmbientOcclusionPatchSize());
      uniforms.inverseTextureSize = 1.0f / static_cast<float>(width);
      return uniforms;
    }
  }
  return uniforms;
}

void VulkanRibbonShader::paint(VkCommandBuffer commandBuffer)
{
  if (!_pipeline)
  {
    return;
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *ribbonSource = dynamic_cast<RKRenderRibbonSource *>(object);
      const StructureBuffers &buffers = _structureBuffers[i][j];
      if (object && object->isVisible() && ribbonSource && ribbonSource->drawRibbon() &&
          ribbonSource->ribbonNumberOfIndices() > 0 && buffers.indexCount > 0 && buffers.vertexBuffer.buffer &&
          buffers.indexBuffer.buffer)
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
        _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers.vertexBuffer.buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, buffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        drawRibbonRanges(commandBuffer, ribbonSource);
      }
      ++flatIndex;
    }
  }
}

void VulkanRibbonShader::paintPick(VkCommandBuffer commandBuffer, VkPipeline pipeline)
{
  if (!pipeline)
  {
    return;
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *ribbonSource = dynamic_cast<RKRenderRibbonSource *>(object);
      const StructureBuffers &buffers = _structureBuffers[i][j];
      if (object && object->isVisible() && ribbonSource && ribbonSource->drawRibbon() &&
          ribbonSource->ribbonNumberOfIndices() > 0 && buffers.indexCount > 0 && buffers.vertexBuffer.buffer &&
          buffers.indexBuffer.buffer)
      {
        _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers.vertexBuffer.buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, buffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        drawRibbonRanges(commandBuffer, ribbonSource);
      }
      ++flatIndex;
    }
  }
}

void VulkanRibbonShader::paintSelection(VkCommandBuffer commandBuffer, VkPipeline pipeline, size_t sceneIndex, size_t movieIndex,
                                        uint32_t structureIndex)
{
  if (!pipeline || sceneIndex >= _structureBuffers.size() || movieIndex >= _structureBuffers[sceneIndex].size())
  {
    return;
  }

  auto *object = (sceneIndex < _renderStructures.size() && movieIndex < _renderStructures[sceneIndex].size())
                     ? _renderStructures[sceneIndex][movieIndex].get()
                     : nullptr;
  auto *ribbonSource = dynamic_cast<RKRenderRibbonSource *>(object);
  const StructureBuffers &buffers = _structureBuffers[sceneIndex][movieIndex];
  if (!object || !object->isVisible() || !ribbonSource || !ribbonSource->drawRibbon() ||
      ribbonSource->ribbonNumberOfIndices() <= 0 || buffers.indexCount == 0 || !buffers.vertexBuffer.buffer ||
      !buffers.indexBuffer.buffer)
  {
    return;
  }

  const std::set<int> segmentIndices = ribbonSource->renderSelectedRibbonSegmentDrawRangeIndices();
  const std::set<int> residueIndices = ribbonSource->renderSelectedRibbonResidueDrawRangeIndices();
  if (segmentIndices.empty() && residueIndices.empty())
  {
    return;
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), structureIndex);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers.vertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(commandBuffer, buffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
  drawSelectedRibbonRanges(commandBuffer, ribbonSource);
}
