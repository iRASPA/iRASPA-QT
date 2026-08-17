#include "vulkanbondshader.h"

#include "rkimposters.h"
#include "skasymmetricbond.h"
#include "vulkanshader.h"

#include <algorithm>
#include <cstddef>

namespace
{
VkVertexInputAttributeDescription attribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}

VulkanShader::PipelineConfig bondImposterPipelineConfig(const QString &vert, const QString &frag)
{
  VulkanShader::PipelineConfig config;
  config.vertexShaderResource = vert;
  config.fragmentShaderResource = frag;
  config.cullMode = VK_CULL_MODE_NONE;
  config.depthTest = true;
  config.depthWrite = true;
  config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkVertexInputBindingDescription instanceBinding{};
  instanceBinding.binding = 0;
  instanceBinding.stride = sizeof(RKInPerInstanceAttributesBonds);
  instanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  config.bindings = {instanceBinding};

  VkVertexInputAttributeDescription typeAttribute{};
  typeAttribute.location = 5;
  typeAttribute.binding = 0;
  typeAttribute.format = VK_FORMAT_R32_SINT;
  typeAttribute.offset = offsetof(RKInPerInstanceAttributesBonds, type);

  config.attributes = {
      attribute(0, 0, offsetof(RKInPerInstanceAttributesBonds, position1)),
      attribute(1, 0, offsetof(RKInPerInstanceAttributesBonds, position2)),
      attribute(2, 0, offsetof(RKInPerInstanceAttributesBonds, color1)),
      attribute(3, 0, offsetof(RKInPerInstanceAttributesBonds, color2)),
      attribute(4, 0, offsetof(RKInPerInstanceAttributesBonds, scale)),
      typeAttribute,
  };
  return config;
}

std::vector<RKInPerInstanceAttributesBonds> filterBonds(const std::vector<RKInPerInstanceAttributesBonds> &data, int32_t type)
{
  std::vector<RKInPerInstanceAttributesBonds> filtered;
  std::copy_if(data.begin(), data.end(), std::back_inserter(filtered),
               [type](const RKInPerInstanceAttributesBonds &bond) { return bond.type == type; });
  return filtered;
}
}  // namespace

VulkanBondShader::VulkanBondShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanBondShader::~VulkanBondShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyStructureBuffers();
  auto destroyPipeline = [this](VkPipeline &pipeline) {
    if (pipeline)
    {
      vkDestroyPipeline(_renderer->device(), pipeline, nullptr);
      pipeline = VK_NULL_HANDLE;
    }
  };
  destroyPipeline(_internalImposterPipeline);
  destroyPipeline(_externalImposterPipeline);
  destroyPipeline(_internalImposterPerPixelPipeline);
  destroyPipeline(_externalImposterPerPixelPipeline);
}

void VulkanBondShader::destroyStructureBuffers()
{
  auto destroyBonds = [this](std::vector<std::vector<StructureBonds>> &scenes) {
    for (auto &scene : scenes)
    {
      for (auto &bonds : scene)
      {
        _renderer->destroyBuffer(bonds.all.buffer);
        _renderer->destroyBuffer(bonds.single.buffer);
        _renderer->destroyBuffer(bonds.doubleBond.buffer);
        _renderer->destroyBuffer(bonds.partialDouble.buffer);
        _renderer->destroyBuffer(bonds.triple.buffer);
      }
    }
    scenes.clear();
  };
  destroyBonds(_internalBonds);
  destroyBonds(_externalBonds);
}

void VulkanBondShader::uploadInstances(InstanceDraw &draw, const std::vector<RKInPerInstanceAttributesBonds> &data)
{
  draw.count = static_cast<uint32_t>(data.size());
  if (!data.empty())
  {
    _renderer->uploadBuffer(draw.buffer, data.data(), data.size() * sizeof(RKInPerInstanceAttributesBonds),
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
}

void VulkanBondShader::fillBondInstances(StructureBonds &bonds, const std::vector<RKInPerInstanceAttributesBonds> &data)
{
  uploadInstances(bonds.all, data);
  uploadInstances(bonds.single, filterBonds(data, static_cast<int32_t>(SKAsymmetricBond::SKBondType::singleBond)));
  uploadInstances(bonds.doubleBond, filterBonds(data, static_cast<int32_t>(SKAsymmetricBond::SKBondType::doubleBond)));
  uploadInstances(bonds.partialDouble, filterBonds(data, static_cast<int32_t>(SKAsymmetricBond::SKBondType::partialDoubleBond)));
  uploadInstances(bonds.triple, filterBonds(data, static_cast<int32_t>(SKAsymmetricBond::SKBondType::tripleBond)));
}

void VulkanBondShader::initialize()
{
  VulkanShader::PipelineConfig internalImposter = bondImposterPipelineConfig(
      QStringLiteral(":/shaders/bond_imposter.vert.spv"), QStringLiteral(":/shaders/bond_imposter.frag.spv"));
  internalImposter.sampleShading = true;
  _internalImposterPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), internalImposter);
  internalImposter.sampleShading = false;
  _internalImposterPerPixelPipeline =
      VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), internalImposter);

  VulkanShader::PipelineConfig externalImposter = bondImposterPipelineConfig(
      QStringLiteral(":/shaders/bond_imposter.vert.spv"), QStringLiteral(":/shaders/bond_imposter_external.frag.spv"));
  externalImposter.sampleShading = true;
  _externalImposterPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), externalImposter);
  externalImposter.sampleShading = false;
  _externalImposterPerPixelPipeline =
      VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), externalImposter);
}

void VulkanBondShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  reloadData();
}

void VulkanBondShader::reloadData()
{
  destroyStructureBuffers();
  _internalBonds.resize(_renderStructures.size());
  _externalBonds.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _internalBonds[i].resize(_renderStructures[i].size());
    _externalBonds[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *source = dynamic_cast<RKRenderBondSource *>(_renderStructures[i][j].get());
      if (!source || !source->drawBonds())
      {
        continue;
      }
      fillBondInstances(_internalBonds[i][j], source->renderInternalBonds());
      fillBondInstances(_externalBonds[i][j], source->renderExternalBonds());
    }
  }
}

void VulkanBondShader::paintBondGroupImposters(VkCommandBuffer commandBuffer, VkPipeline pipeline, const StructureBonds &bonds,
                                               bool unity, uint32_t structureIndex)
{
  auto drawImposters = [&](const InstanceDraw &instances, uint32_t vertexCount) {
    if (instances.count == 0)
    {
      return;
    }
    _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), structureIndex);
    VkDeviceSize instanceOffset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &instances.buffer.buffer, &instanceOffset);
    vkCmdDraw(commandBuffer, vertexCount, instances.count, 0, 0);
  };

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  if (unity)
  {
    drawImposters(bonds.all, 18);
    return;
  }
  drawImposters(bonds.single, 18);
  drawImposters(bonds.doubleBond, 36);
  drawImposters(bonds.partialDouble, 18);
  drawImposters(bonds.triple, 54);
}

void VulkanBondShader::paint(VkCommandBuffer commandBuffer, RKRenderQuality quality)
{
  if (!_internalImposterPipeline || !_externalImposterPipeline)
  {
    return;
  }
  const bool perSample = perSampleImposterShading(quality);
  VkPipeline internalImposter = perSample ? _internalImposterPipeline : _internalImposterPerPixelPipeline;
  VkPipeline externalImposter = perSample ? _externalImposterPipeline : _externalImposterPerPixelPipeline;

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *source = dynamic_cast<RKRenderBondSource *>(object);
      if (source && source->drawBonds() && object->isVisible())
      {
        const bool unity = source->isUnity();
        if (i < _internalBonds.size() && j < _internalBonds[i].size())
        {
          paintBondGroupImposters(commandBuffer, internalImposter, _internalBonds[i][j], unity, flatIndex);
        }
        if (source->hasExternalBonds() && i < _externalBonds.size() && j < _externalBonds[i].size())
        {
          paintBondGroupImposters(commandBuffer, externalImposter, _externalBonds[i][j], unity, flatIndex);
        }
      }
      ++flatIndex;
    }
  }
}

void VulkanBondShader::paintPick(VkCommandBuffer commandBuffer, VkPipeline internalPipeline, VkPipeline externalPipeline)
{
  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *source = dynamic_cast<RKRenderBondSource *>(object);
      if (source && source->drawBonds() && object->isVisible())
      {
        const bool unity = source->isUnity();
        if (internalPipeline && i < _internalBonds.size() && j < _internalBonds[i].size())
        {
          paintBondGroupImposters(commandBuffer, internalPipeline, _internalBonds[i][j], unity, flatIndex);
        }
        if (externalPipeline && source->hasExternalBonds() && i < _externalBonds.size() && j < _externalBonds[i].size())
        {
          paintBondGroupImposters(commandBuffer, externalPipeline, _externalBonds[i][j], unity, flatIndex);
        }
      }
      ++flatIndex;
    }
  }
}
