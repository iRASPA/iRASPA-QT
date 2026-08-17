#include "vulkanpickingshader.h"

#include "rkrenderuniforms.h"
#include "vulkanatomsphereshader.h"
#include "vulkanbondshader.h"
#include "vulkanprimitiveobjectshader.h"
#include "vulkanrenderer.h"
#include "vulkanribbonshader.h"
#include "vulkanshader.h"

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

VkVertexInputAttributeDescription intAttribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32_SINT;
  description.offset = offset;
  return description;
}

VulkanShader::PipelineConfig pickBaseConfig(VulkanRenderer *renderer, const QString &vert, const QString &frag)
{
  VulkanShader::PipelineConfig config;
  config.vertexShaderResource = vert;
  config.fragmentShaderResource = frag;
  config.renderPass = renderer->pickRenderPass();
  config.blend = false;
  config.depthTest = true;
  config.depthWrite = true;
  config.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  return config;
}
}  // namespace

VulkanPickingShader::VulkanPickingShader(VulkanRenderer *renderer, VulkanAtomSphereShader *atomShader, VulkanBondShader *bondShader,
                                         VulkanPrimitiveObjectShader *primitiveShader, VulkanRibbonShader *ribbonShader)
    : _renderer(renderer), _atomShader(atomShader), _bondShader(bondShader), _primitiveShader(primitiveShader),
      _ribbonShader(ribbonShader)
{
}

VulkanPickingShader::~VulkanPickingShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyPipelines();
}

void VulkanPickingShader::destroyPipelines()
{
  auto destroy = [this](VkPipeline &pipeline) {
    if (pipeline)
    {
      vkDestroyPipeline(_renderer->device(), pipeline, nullptr);
      pipeline = VK_NULL_HANDLE;
    }
  };
  destroy(_atomOrthoImposterPipeline);
  destroy(_atomPerspImposterPipeline);
  destroy(_bondPipeline);
  destroy(_bondExternalPipeline);
  destroy(_primitiveStripPipeline);
  destroy(_primitiveTrianglePipeline);
  destroy(_ribbonPipeline);
}

void VulkanPickingShader::initialize()
{
  if (!_renderer || !_renderer->pickRenderPass())
  {
    return;
  }

  auto makeAtomImposterConfig = [&](const QString &vertexResource) {
    VulkanShader::PipelineConfig config =
        pickBaseConfig(_renderer, vertexResource, QStringLiteral(":/shaders/pick_atom_imposter.frag.spv"));
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
        intAttribute(3, 1, offsetof(RKInPerInstanceAttributesAtoms, tag)),
    };
    return config;
  };
  _atomOrthoImposterPipeline = VulkanShader::createGraphicsPipeline(
      _renderer, _renderer->scenePipelineLayout(),
      makeAtomImposterConfig(QStringLiteral(":/shaders/pick_atom_ortho_imposter.vert.spv")));
  _atomPerspImposterPipeline = VulkanShader::createGraphicsPipeline(
      _renderer, _renderer->scenePipelineLayout(),
      makeAtomImposterConfig(QStringLiteral(":/shaders/pick_atom_persp_imposter.vert.spv")));

  auto makeBondImposterConfig = [&](const QString &fragmentResource) {
    VulkanShader::PipelineConfig config =
        pickBaseConfig(_renderer, QStringLiteral(":/shaders/pick_bond_imposter.vert.spv"), fragmentResource);
    config.cullMode = VK_CULL_MODE_NONE;
    config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkVertexInputBindingDescription instanceBinding{};
    instanceBinding.binding = 0;
    instanceBinding.stride = sizeof(RKInPerInstanceAttributesBonds);
    instanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    config.bindings = {instanceBinding};
    config.attributes = {
        floatAttribute(0, 0, offsetof(RKInPerInstanceAttributesBonds, position1)),
        floatAttribute(1, 0, offsetof(RKInPerInstanceAttributesBonds, position2)),
        floatAttribute(2, 0, offsetof(RKInPerInstanceAttributesBonds, color1)),
        floatAttribute(3, 0, offsetof(RKInPerInstanceAttributesBonds, color2)),
        floatAttribute(4, 0, offsetof(RKInPerInstanceAttributesBonds, scale)),
        intAttribute(5, 0, offsetof(RKInPerInstanceAttributesBonds, type)),
        intAttribute(6, 0, offsetof(RKInPerInstanceAttributesBonds, tag)),
    };
    return config;
  };
  _bondPipeline = VulkanShader::createGraphicsPipeline(
      _renderer, _renderer->scenePipelineLayout(),
      makeBondImposterConfig(QStringLiteral(":/shaders/pick_bond_imposter.frag.spv")));
  _bondExternalPipeline = VulkanShader::createGraphicsPipeline(
      _renderer, _renderer->scenePipelineLayout(),
      makeBondImposterConfig(QStringLiteral(":/shaders/pick_bond_imposter_external.frag.spv")));

  auto makePrimitiveConfig = [&](VkPrimitiveTopology topology, VkCullModeFlags cull) {
    VulkanShader::PipelineConfig config = pickBaseConfig(
        _renderer, QStringLiteral(":/shaders/pick_primitive.vert.spv"), QStringLiteral(":/shaders/pick_primitive.frag.spv"));
    config.cullMode = cull;
    config.topology = topology;

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
        intAttribute(2, 1, offsetof(RKInPerInstanceAttributesAtoms, tag)),
    };
    return config;
  };
  _primitiveStripPipeline = VulkanShader::createGraphicsPipeline(
      _renderer, _renderer->scenePipelineLayout(), makePrimitiveConfig(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_CULL_MODE_BACK_BIT));
  _primitiveTrianglePipeline = VulkanShader::createGraphicsPipeline(
      _renderer, _renderer->scenePipelineLayout(), makePrimitiveConfig(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE));

  {
    VulkanShader::PipelineConfig config =
        pickBaseConfig(_renderer, QStringLiteral(":/shaders/pick_ribbon.vert.spv"), QStringLiteral(":/shaders/pick_ribbon.frag.spv"));
    config.cullMode = VK_CULL_MODE_NONE;
    config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkVertexInputBindingDescription vertexBinding{};
    vertexBinding.binding = 0;
    vertexBinding.stride = sizeof(RKVertex);
    vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    config.bindings = {vertexBinding};
    config.attributes = {
        floatAttribute(0, 0, offsetof(RKVertex, position)),
        floatAttribute(1, 0, offsetof(RKVertex, normal)),
        VkVertexInputAttributeDescription{2, 0, VK_FORMAT_R32G32_SFLOAT, static_cast<uint32_t>(offsetof(RKVertex, st))},
        VkVertexInputAttributeDescription{3, 0, VK_FORMAT_R32G32_SFLOAT, static_cast<uint32_t>(offsetof(RKVertex, pad))},
    };
    _ribbonPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), config);
  }
}

void VulkanPickingShader::paint(VkCommandBuffer commandBuffer, RKRenderQuality /*quality*/, bool orthographic)
{
  if (_atomShader)
  {
    VkPipeline pipeline = orthographic ? _atomOrthoImposterPipeline : _atomPerspImposterPipeline;
    if (pipeline)
    {
      _atomShader->paintPick(commandBuffer, pipeline);
    }
  }
  if (_bondShader && _bondPipeline && _bondExternalPipeline)
  {
    _bondShader->paintPick(commandBuffer, _bondPipeline, _bondExternalPipeline);
  }
  if (_primitiveShader && _primitiveStripPipeline && _primitiveTrianglePipeline)
  {
    _primitiveShader->paintPick(commandBuffer, _primitiveStripPipeline, _primitiveTrianglePipeline);
  }
  if (_ribbonShader && _ribbonPipeline)
  {
    _ribbonShader->paintPick(commandBuffer, _ribbonPipeline);
  }
}
