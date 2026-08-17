#include "vulkanvolumeshader.h"

#include "cubegeometry.h"
#include "rkrenderuniforms.h"
#include "volumetransferfunctions.h"
#include "vulkanshader.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace
{
VulkanShader::PipelineConfig volumePipelineConfig()
{
  VulkanShader::PipelineConfig config;
  config.vertexShaderResource = QStringLiteral(":/shaders/volume.vert.spv");
  config.fragmentShaderResource = QStringLiteral(":/shaders/volume.frag.spv");
  config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  config.cullMode = VK_CULL_MODE_BACK_BIT;
  config.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  config.depthTest = true;
  config.depthWrite = true;
  config.depthClamp = true;
  config.blend = true;
  config.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  config.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

  VkVertexInputBindingDescription vertexBinding{};
  vertexBinding.binding = 0;
  vertexBinding.stride = sizeof(RKVertex);
  vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  config.bindings = {vertexBinding};

  VkVertexInputAttributeDescription position{};
  position.location = 0;
  position.binding = 0;
  position.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  position.offset = offsetof(RKVertex, position);
  config.attributes = {position};
  return config;
}

int powerOfTwoCubeSize(const int3 &dimensions)
{
  const int largestSize = std::max({dimensions.x, dimensions.y, dimensions.z});
  int size = 1;
  while (size < largestSize)
  {
    size *= 2;
  }
  return std::max(size, 1);
}
}  // namespace

VulkanVolumeShader::VulkanVolumeShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanVolumeShader::~VulkanVolumeShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyStructureResources();
  _renderer->destroyTexture(_transferFunction);
  _renderer->destroyBuffer(_vertexBuffer);
  _renderer->destroyBuffer(_indexBuffer);
  if (_opaquePipeline)
  {
    vkDestroyPipeline(_renderer->device(), _opaquePipeline, nullptr);
  }
  if (_transparentPipeline)
  {
    vkDestroyPipeline(_renderer->device(), _transparentPipeline, nullptr);
  }
}

void VulkanVolumeShader::destroyStructureResources()
{
  for (auto &scene : _structureResources)
  {
    for (auto &resources : scene)
    {
      if (resources.descriptorSet)
      {
        vkFreeDescriptorSets(_renderer->device(), _renderer->descriptorPool(), 1, &resources.descriptorSet);
        resources.descriptorSet = VK_NULL_HANDLE;
      }
      _renderer->destroyTexture(resources.volume);
    }
  }
  _structureResources.clear();
}

void VulkanVolumeShader::initialize()
{
  CubeGeometry cube;
  const auto vertices = cube.vertices();
  const auto indices = cube.indices();
  _indexCount = static_cast<uint32_t>(indices.size());
  _renderer->uploadBuffer(_vertexBuffer, vertices.data(), vertices.size() * sizeof(RKVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  _renderer->uploadBuffer(_indexBuffer, indices.data(), indices.size() * sizeof(short), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  _transferFunction = _renderer->createTexture2DArray_RGBA32F(
      VolumeTransferFunctions::kTableSize, VolumeTransferFunctions::kTableCount, VolumeTransferFunctions::kTables.data());

  VulkanShader::PipelineConfig opaque = volumePipelineConfig();
  opaque.depthTest = true;
  opaque.depthWrite = true;
  _opaquePipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->volumePipelineLayout(), opaque);

  VulkanShader::PipelineConfig transparent = volumePipelineConfig();
  transparent.depthTest = false;
  transparent.depthWrite = false;
  _transparentPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->volumePipelineLayout(), transparent);
}

void VulkanVolumeShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  reloadData();
}

void VulkanVolumeShader::invalidateIsosurface(const std::vector<std::shared_ptr<RKRenderObject>> &structures)
{
  for (const auto &structure : structures)
  {
    _volumeCache.erase(structure.get());
  }
}

void VulkanVolumeShader::reloadData()
{
  destroyStructureResources();
  _structureResources.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureResources[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *object = dynamic_cast<RKRenderObject *>(_renderStructures[i][j].get());
      auto *source = dynamic_cast<RKRenderVolumetricDataSource *>(_renderStructures[i][j].get());
      if (!object || !source || source->adsorptionSurfaceRenderingMethod() != RKEnergySurfaceType::volumeRendering ||
          !source->drawAdsorptionSurface())
      {
        continue;
      }

      RKRenderObject *key = _renderStructures[i][j].get();
      std::shared_ptr<std::vector<float4>> volumeData;
      auto cacheIt = _volumeCache.find(key);
      if (cacheIt != _volumeCache.end())
      {
        volumeData = cacheIt->second;
      }
      else
      {
        std::vector<float4> gridData = source->gridValueAndGradientData();
        if (gridData.empty())
        {
          continue;
        }
        volumeData = std::make_shared<std::vector<float4>>(std::move(gridData));
        _volumeCache[key] = volumeData;
      }

      int size = powerOfTwoCubeSize(source->dimensions());
      const size_t expected = static_cast<size_t>(size) * static_cast<size_t>(size) * static_cast<size_t>(size);
      if (volumeData->size() != expected)
      {
        size = static_cast<int>(std::lround(std::cbrt(static_cast<double>(volumeData->size()))));
        if (size <= 0 || static_cast<size_t>(size) * static_cast<size_t>(size) * static_cast<size_t>(size) != volumeData->size())
        {
          continue;
        }
      }

      StructureResources &resources = _structureResources[i][j];
      resources.volume = _renderer->createTexture3D_RGBA32F(static_cast<uint32_t>(size), static_cast<uint32_t>(size),
                                                            static_cast<uint32_t>(size), volumeData->data());
      resources.descriptorSet =
          _renderer->allocateVolumeDescriptorSet(resources.volume, _renderer->sampledDepthView(), _transferFunction);
    }
  }
}

bool VulkanVolumeShader::structureIsVolume(size_t sceneIndex, size_t movieIndex, bool opaque) const
{
  if (sceneIndex >= _renderStructures.size() || movieIndex >= _renderStructures[sceneIndex].size())
  {
    return false;
  }
  auto *object = _renderStructures[sceneIndex][movieIndex].get();
  auto *source = dynamic_cast<RKRenderVolumetricDataSource *>(object);
  if (!object || !object->isVisible() || !source || !source->drawAdsorptionSurface() ||
      source->adsorptionSurfaceRenderingMethod() != RKEnergySurfaceType::volumeRendering)
  {
    return false;
  }
  const bool isRaspaPes =
      source->adsorptionVolumeTransferFunction() == RKPredefinedVolumeRenderingTransferFunction::RASPA_PES;
  return opaque ? isRaspaPes : !isRaspaPes;
}

bool VulkanVolumeShader::hasVisibleVolumes() const
{
  return hasOpaqueVolumes() || hasTransparentVolumes();
}

bool VulkanVolumeShader::hasOpaqueVolumes() const
{
  for (size_t i = 0; i < _structureResources.size(); ++i)
  {
    for (size_t j = 0; j < _structureResources[i].size(); ++j)
    {
      if (structureIsVolume(i, j, true) && _structureResources[i][j].volume.view && _structureResources[i][j].descriptorSet)
      {
        return true;
      }
    }
  }
  return false;
}

bool VulkanVolumeShader::hasTransparentVolumes() const
{
  for (size_t i = 0; i < _structureResources.size(); ++i)
  {
    for (size_t j = 0; j < _structureResources[i].size(); ++j)
    {
      if (structureIsVolume(i, j, false) && _structureResources[i][j].volume.view && _structureResources[i][j].descriptorSet)
      {
        return true;
      }
    }
  }
  return false;
}

void VulkanVolumeShader::paintOpaque(VkCommandBuffer commandBuffer)
{
  paint(commandBuffer, true, -1, -1);
}

void VulkanVolumeShader::paintTransparent(VkCommandBuffer commandBuffer)
{
  paint(commandBuffer, false, -1, -1);
}

void VulkanVolumeShader::paintOpaque(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex)
{
  paint(commandBuffer, true, static_cast<int>(sceneIndex), static_cast<int>(movieIndex));
}

void VulkanVolumeShader::paintTransparent(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex)
{
  paint(commandBuffer, false, static_cast<int>(sceneIndex), static_cast<int>(movieIndex));
}

void VulkanVolumeShader::paint(VkCommandBuffer commandBuffer, bool opaque)
{
  paint(commandBuffer, opaque, -1, -1);
}

void VulkanVolumeShader::paint(VkCommandBuffer commandBuffer, bool opaque, int sceneIndex, int movieIndex)
{
  const VkPipeline pipeline = opaque ? _opaquePipeline : _transparentPipeline;
  if (!pipeline || !_indexCount || !_vertexBuffer.buffer || !_indexBuffer.buffer)
  {
    return;
  }

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureResources.size(); ++i)
  {
    for (size_t j = 0; j < _structureResources[i].size(); ++j)
    {
      StructureResources &resources = _structureResources[i][j];
      if ((sceneIndex < 0 || (static_cast<int>(i) == sceneIndex && static_cast<int>(j) == movieIndex)) &&
          structureIsVolume(i, j, opaque) && resources.volume.view && resources.descriptorSet)
      {
        _renderer->updateVolumeDescriptorSet(resources.descriptorSet, resources.volume, _renderer->sampledDepthView(),
                                             _transferFunction);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        _renderer->bindSceneDescriptors(commandBuffer, _renderer->volumePipelineLayout(), flatIndex);
        _renderer->bindSamplerDescriptorSet(commandBuffer, _renderer->volumePipelineLayout(), resources.descriptorSet);
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_vertexBuffer.buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, _indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandBuffer, _indexCount, 1, 0, 0, 0);
      }
      ++flatIndex;
    }
  }
}
