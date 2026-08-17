#include "vulkanisosurfaceshader.h"

#include "skcell.h"
#include "skcomputeisosurface.h"
#include "vulkanshader.h"

#include <cstddef>
#include <exception>

namespace
{
constexpr uint32_t kIsosurfaceVertexStride = 3 * sizeof(float4);

VkVertexInputAttributeDescription isosurfaceAttribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}

VulkanShader::PipelineConfig isosurfacePipelineConfig()
{
  VulkanShader::PipelineConfig config;
  config.vertexShaderResource = QStringLiteral(":/shaders/isosurface.vert.spv");
  config.fragmentShaderResource = QStringLiteral(":/shaders/isosurface.frag.spv");
  config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  config.depthTest = true;

  VkVertexInputBindingDescription vertexBinding{};
  vertexBinding.binding = 0;
  vertexBinding.stride = kIsosurfaceVertexStride;
  vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  VkVertexInputBindingDescription instanceBinding{};
  instanceBinding.binding = 1;
  instanceBinding.stride = sizeof(float4);
  instanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  config.bindings = {vertexBinding, instanceBinding};
  config.attributes = {
      isosurfaceAttribute(0, 0, 0),
      isosurfaceAttribute(1, 0, sizeof(float4)),
      isosurfaceAttribute(2, 1, 0),
  };
  return config;
}
}  // namespace

VulkanIsosurfaceShader::VulkanIsosurfaceShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanIsosurfaceShader::~VulkanIsosurfaceShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyStructureBuffers();
  if (_opaquePipeline)
  {
    vkDestroyPipeline(_renderer->device(), _opaquePipeline, nullptr);
  }
  if (_transparentFrontPipeline)
  {
    vkDestroyPipeline(_renderer->device(), _transparentFrontPipeline, nullptr);
  }
  if (_transparentBackPipeline)
  {
    vkDestroyPipeline(_renderer->device(), _transparentBackPipeline, nullptr);
  }
}

void VulkanIsosurfaceShader::destroyStructureBuffers()
{
  for (auto &scene : _structureBuffers)
  {
    for (auto &buffers : scene)
    {
      _renderer->destroyBuffer(buffers.vertexBuffer);
      _renderer->destroyBuffer(buffers.instanceBuffer);
    }
  }
  _structureBuffers.clear();
}

void VulkanIsosurfaceShader::initialize()
{
  VulkanShader::PipelineConfig opaque = isosurfacePipelineConfig();
  opaque.cullMode = VK_CULL_MODE_NONE;
  opaque.depthWrite = true;
  opaque.blend = false;
  _opaquePipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), opaque);

  VulkanShader::PipelineConfig transparent = isosurfacePipelineConfig();
  transparent.blend = true;
  transparent.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  transparent.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  transparent.depthWrite = false;

  transparent.cullMode = VK_CULL_MODE_FRONT_BIT;
  _transparentFrontPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), transparent);

  transparent.cullMode = VK_CULL_MODE_BACK_BIT;
  _transparentBackPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), transparent);
}

void VulkanIsosurfaceShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  reloadData();
}

void VulkanIsosurfaceShader::invalidateIsosurface(const std::vector<std::shared_ptr<RKRenderObject>> &structures)
{
  for (const auto &structure : structures)
  {
    _gridCache.erase(structure.get());
  }
}

void VulkanIsosurfaceShader::reloadData()
{
  destroyStructureBuffers();
  _structureBuffers.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureBuffers[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *object = dynamic_cast<RKRenderObject *>(_renderStructures[i][j].get());
      auto *source = dynamic_cast<RKRenderVolumetricDataSource *>(_renderStructures[i][j].get());
      if (!object || !source || !object->cell())
      {
        continue;
      }
      if (!source->drawAdsorptionSurface() || source->adsorptionSurfaceRenderingMethod() != RKEnergySurfaceType::isoSurface)
      {
        continue;
      }

      const double isoValue = source->adsorptionSurfaceIsoValue();
      const int3 dimensions = source->dimensions();
      RKRenderObject *key = _renderStructures[i][j].get();

      std::shared_ptr<std::vector<float>> energyGrid;
      auto cacheIt = _gridCache.find(key);
      if (cacheIt != _gridCache.end())
      {
        energyGrid = cacheIt->second;
      }
      else
      {
        std::vector<float> gridData = source->gridData();
        if (gridData.empty())
        {
          continue;
        }
        energyGrid = std::make_shared<std::vector<float>>(std::move(gridData));
        _gridCache[key] = energyGrid;
      }

      std::vector<float4> triangleData;
      try
      {
        triangleData = SKComputeIsosurface::computeIsosurface(dimensions, energyGrid.get(), isoValue);
      }
      catch (const std::exception &)
      {
        triangleData = SKComputeIsosurface::computeIsosurfaceCPUImplementation(dimensions, energyGrid.get(), isoValue);
      }

      _structureBuffers[i][j].triangleCount = static_cast<uint32_t>(triangleData.size() / 9);
      if (!triangleData.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].vertexBuffer, triangleData.data(),
                                triangleData.size() * sizeof(float4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
      }

      const std::vector<float4> latticeVectors = object->cell()->renderTranslationVectors();
      _structureBuffers[i][j].instanceCount = static_cast<uint32_t>(latticeVectors.size());
      if (!latticeVectors.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].instanceBuffer, latticeVectors.data(),
                                latticeVectors.size() * sizeof(float4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
      }
    }
  }
}

void VulkanIsosurfaceShader::paintOpaque(VkCommandBuffer commandBuffer)
{
  paint(commandBuffer, true, -1, -1);
}

void VulkanIsosurfaceShader::paintTransparent(VkCommandBuffer commandBuffer)
{
  paint(commandBuffer, false, -1, -1);
}

void VulkanIsosurfaceShader::paintOpaque(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex)
{
  paint(commandBuffer, true, static_cast<int>(sceneIndex), static_cast<int>(movieIndex));
}

void VulkanIsosurfaceShader::paintTransparent(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex)
{
  paint(commandBuffer, false, static_cast<int>(sceneIndex), static_cast<int>(movieIndex));
}

void VulkanIsosurfaceShader::paint(VkCommandBuffer commandBuffer, bool opaque)
{
  paint(commandBuffer, opaque, -1, -1);
}

void VulkanIsosurfaceShader::paint(VkCommandBuffer commandBuffer, bool opaque, int sceneIndex, int movieIndex)
{
  if (!_opaquePipeline || !_transparentFrontPipeline || !_transparentBackPipeline)
  {
    return;
  }

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *source = dynamic_cast<RKRenderVolumetricDataSource *>(object);
      const StructureBuffers &buffers = _structureBuffers[i][j];
      const bool visible = object && object->isVisible() && source && source->drawAdsorptionSurface() &&
                           buffers.triangleCount > 0 && buffers.instanceCount > 0 && buffers.vertexBuffer.buffer &&
                           buffers.instanceBuffer.buffer;
      const bool isOpaque = source && source->adsorptionSurfaceOpacity() > 0.99999;
      const bool drawThis = visible && (opaque ? isOpaque
                                               : (!isOpaque && source->adsorptionSurfaceRenderingMethod() == RKEnergySurfaceType::isoSurface)) &&
                            (sceneIndex < 0 || (static_cast<int>(i) == sceneIndex && static_cast<int>(j) == movieIndex));
      if (drawThis)
      {
        auto drawWith = [&](VkPipeline pipeline) {
          vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
          _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
          VkDeviceSize offset = 0;
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers.vertexBuffer.buffer, &offset);
          vkCmdBindVertexBuffers(commandBuffer, 1, 1, &buffers.instanceBuffer.buffer, &offset);
          vkCmdDraw(commandBuffer, 3 * buffers.triangleCount, buffers.instanceCount, 0, 0);
        };

        if (opaque)
        {
          drawWith(_opaquePipeline);
        }
        else
        {
          drawWith(_transparentFrontPipeline);
          drawWith(_transparentBackPipeline);
        }
      }
      ++flatIndex;
    }
  }
}
