#include "vulkanselectionshader.h"

#include "cappedcylindergeometry.h"
#include "cappednsidedprismgeometry.h"
#include "nsidedprismgeometry.h"
#include "quadgeometry.h"
#include "rkrenderuniforms.h"
#include "spheregeometry.h"
#include "uncappedcylindergeometry.h"
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

VkVertexInputAttributeDescription vec2Attribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32_SFLOAT;
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
  return config;
}
}  // namespace

VulkanSelectionShader::VulkanSelectionShader(VulkanRenderer *renderer, VulkanRibbonShader *ribbonShader)
    : _renderer(renderer), _ribbonShader(ribbonShader)
{
}

VulkanSelectionShader::~VulkanSelectionShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyStructureBuffers();
  destroyPipelines();
  _renderer->destroyBuffer(_sphereMesh.vertexBuffer);
  _renderer->destroyBuffer(_sphereMesh.indexBuffer);
  _renderer->destroyBuffer(_quadMesh.vertexBuffer);
  _renderer->destroyBuffer(_quadMesh.indexBuffer);
}

void VulkanSelectionShader::destroyPipelines()
{
  auto destroy = [this](VkPipeline &pipeline) {
    if (pipeline)
    {
      vkDestroyPipeline(_renderer->device(), pipeline, nullptr);
      pipeline = VK_NULL_HANDLE;
    }
  };
  auto destroySet = [&](StylePipelines &set) {
    destroy(set.glow);
    destroy(set.stripes);
    destroy(set.worley);
  };
  destroySet(_atomOrthoImposterPipelines);
  destroySet(_atomPerspImposterPipelines);
  destroySet(_bondPipelines);
  destroySet(_primitiveStripPipelines);
  destroySet(_primitiveTrianglePipelines);
  destroySet(_ribbonPipelines);
}

void VulkanSelectionShader::destroyStructureBuffers()
{
  for (auto &scene : _structureBuffers)
  {
    for (auto &buffers : scene)
    {
      _renderer->destroyBuffer(buffers.atoms.buffer);
      _renderer->destroyBuffer(buffers.internalBonds.buffer);
      _renderer->destroyBuffer(buffers.externalBonds.buffer);
      _renderer->destroyBuffer(buffers.ellipsoid.buffer);
      _renderer->destroyBuffer(buffers.crystalEllipsoid.buffer);
      _renderer->destroyBuffer(buffers.cylinder.buffer);
      _renderer->destroyBuffer(buffers.crystalCylinder.buffer);
      _renderer->destroyBuffer(buffers.prism.buffer);
      _renderer->destroyBuffer(buffers.crystalPrism.buffer);
      _renderer->destroyBuffer(buffers.cylinderMesh.vertexBuffer);
      _renderer->destroyBuffer(buffers.cylinderMesh.indexBuffer);
      _renderer->destroyBuffer(buffers.prismMesh.vertexBuffer);
      _renderer->destroyBuffer(buffers.prismMesh.indexBuffer);
    }
  }
  _structureBuffers.clear();
}

void VulkanSelectionShader::uploadMesh(Mesh &mesh, const std::vector<RKVertex> &vertices, const std::vector<short> &indices)
{
  mesh.indexCount = static_cast<uint32_t>(indices.size());
  if (!vertices.empty())
  {
    _renderer->uploadBuffer(mesh.vertexBuffer, vertices.data(), vertices.size() * sizeof(RKVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
  if (!indices.empty())
  {
    _renderer->uploadBuffer(mesh.indexBuffer, indices.data(), indices.size() * sizeof(short), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  }
}

void VulkanSelectionShader::uploadInstances(InstanceDraw &draw, const void *data, VkDeviceSize bytes, uint32_t count)
{
  draw.count = count;
  if (count > 0 && data != nullptr)
  {
    _renderer->uploadBuffer(draw.buffer, data, bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
}

VkPipeline VulkanSelectionShader::createAtomImposterPipeline(const QString &vertexResource, const QString &fragmentResource,
                                                             bool writeGlow)
{
  VulkanShader::PipelineConfig config = overlayConfig();
  config.vertexShaderResource = vertexResource;
  config.fragmentShaderResource = fragmentResource;
  config.writeGlow = writeGlow;
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

VkPipeline VulkanSelectionShader::createBondImposterPipeline(const QString &fragmentResource, bool writeGlow)
{
  VulkanShader::PipelineConfig config = overlayConfig();
  config.vertexShaderResource = QStringLiteral(":/shaders/bond_selection_imposter.vert.spv");
  config.fragmentShaderResource = fragmentResource;
  config.writeGlow = writeGlow;
  config.cullMode = VK_CULL_MODE_NONE;
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
      floatAttribute(0, 0, offsetof(RKInPerInstanceAttributesBonds, position1)),
      floatAttribute(1, 0, offsetof(RKInPerInstanceAttributesBonds, position2)),
      floatAttribute(2, 0, offsetof(RKInPerInstanceAttributesBonds, color1)),
      floatAttribute(3, 0, offsetof(RKInPerInstanceAttributesBonds, color2)),
      floatAttribute(4, 0, offsetof(RKInPerInstanceAttributesBonds, scale)),
      typeAttribute,
  };
  return VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), config);
}

VkPipeline VulkanSelectionShader::createPrimitivePipeline(const QString &fragmentResource, VkPrimitiveTopology topology,
                                                          VkCullModeFlags cullMode, bool writeGlow)
{
  VulkanShader::PipelineConfig config = overlayConfig();
  config.vertexShaderResource = QStringLiteral(":/shaders/primitive_selection.vert.spv");
  config.fragmentShaderResource = fragmentResource;
  config.writeGlow = writeGlow;
  config.cullMode = cullMode;
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
      floatAttribute(1, 0, offsetof(RKVertex, normal)),
      floatAttribute(2, 1, offsetof(RKInPerInstanceAttributesAtoms, position)),
      floatAttribute(3, 1, offsetof(RKInPerInstanceAttributesAtoms, scale)),
  };
  return VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), config);
}

VkPipeline VulkanSelectionShader::createRibbonPipeline(const QString &vertexResource, const QString &fragmentResource, bool writeGlow)
{
  VulkanShader::PipelineConfig config = overlayConfig();
  config.vertexShaderResource = vertexResource;
  config.fragmentShaderResource = fragmentResource;
  config.writeGlow = writeGlow;
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
      vec2Attribute(2, 0, offsetof(RKVertex, st)),
      vec2Attribute(3, 0, offsetof(RKVertex, pad)),
      vec2Attribute(4, 0, offsetof(RKVertex, stripeST)),
  };
  return VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), config);
}

void VulkanSelectionShader::initialize()
{
  SphereGeometry sphere(1.0, 41);
  uploadMesh(_sphereMesh, sphere.vertices(), sphere.indices());
  QuadGeometry quad;
  uploadMesh(_quadMesh, quad.vertices(), quad.indices());

  _atomOrthoImposterPipelines.glow = createAtomImposterPipeline(QStringLiteral(":/shaders/atom_selection_ortho_imposter.vert.spv"),
                                                                QStringLiteral(":/shaders/atom_selection_glow_imposter.frag.spv"), true);
  _atomOrthoImposterPipelines.stripes = createAtomImposterPipeline(QStringLiteral(":/shaders/atom_selection_ortho_imposter.vert.spv"),
                                                                   QStringLiteral(":/shaders/atom_selection_stripes_imposter.frag.spv"));
  _atomOrthoImposterPipelines.worley = createAtomImposterPipeline(QStringLiteral(":/shaders/atom_selection_ortho_imposter.vert.spv"),
                                                                  QStringLiteral(":/shaders/atom_selection_worley_imposter.frag.spv"));
  _atomPerspImposterPipelines.glow = createAtomImposterPipeline(QStringLiteral(":/shaders/atom_selection_persp_imposter.vert.spv"),
                                                                QStringLiteral(":/shaders/atom_selection_glow_imposter.frag.spv"), true);
  _atomPerspImposterPipelines.stripes = createAtomImposterPipeline(QStringLiteral(":/shaders/atom_selection_persp_imposter.vert.spv"),
                                                                   QStringLiteral(":/shaders/atom_selection_stripes_imposter.frag.spv"));
  _atomPerspImposterPipelines.worley = createAtomImposterPipeline(QStringLiteral(":/shaders/atom_selection_persp_imposter.vert.spv"),
                                                                  QStringLiteral(":/shaders/atom_selection_worley_imposter.frag.spv"));
  _bondPipelines.glow = createBondImposterPipeline(QStringLiteral(":/shaders/bond_selection_glow_imposter.frag.spv"), true);
  _bondPipelines.stripes = createBondImposterPipeline(QStringLiteral(":/shaders/bond_selection_stripes_imposter.frag.spv"));
  _bondPipelines.worley = createBondImposterPipeline(QStringLiteral(":/shaders/bond_selection_worley_imposter.frag.spv"));

  auto createPrimitiveSet = [&](VkPrimitiveTopology topology, VkCullModeFlags cull) {
    StylePipelines set;
    set.glow = createPrimitivePipeline(QStringLiteral(":/shaders/primitive_selection_glow.frag.spv"), topology, cull, true);
    set.stripes = createPrimitivePipeline(QStringLiteral(":/shaders/primitive_selection_stripes.frag.spv"), topology, cull);
    set.worley = createPrimitivePipeline(QStringLiteral(":/shaders/primitive_selection_worley.frag.spv"), topology, cull);
    return set;
  };
  _primitiveStripPipelines = createPrimitiveSet(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_CULL_MODE_BACK_BIT);
  _primitiveTrianglePipelines = createPrimitiveSet(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE);

  _ribbonPipelines.glow = createRibbonPipeline(QStringLiteral(":/shaders/ribbon_selection_glow.vert.spv"),
                                               QStringLiteral(":/shaders/ribbon_selection_glow.frag.spv"), true);
  _ribbonPipelines.stripes = createRibbonPipeline(QStringLiteral(":/shaders/ribbon_selection_stripes.vert.spv"),
                                                  QStringLiteral(":/shaders/ribbon_selection_stripes.frag.spv"));
  _ribbonPipelines.worley = createRibbonPipeline(QStringLiteral(":/shaders/ribbon_selection_worley.vert.spv"),
                                                 QStringLiteral(":/shaders/ribbon_selection_worley.frag.spv"));
}

void VulkanSelectionShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  reloadData();
}

void VulkanSelectionShader::reloadData()
{
  destroyStructureBuffers();
  _structureBuffers.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureBuffers[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      StructureSelection &buffers = _structureBuffers[i][j];
      if (auto *atomSource = dynamic_cast<RKRenderAtomSource *>(_renderStructures[i][j].get()))
      {
        const std::vector<RKInPerInstanceAttributesAtoms> atoms = atomSource->renderSelectedAtoms();
        uploadInstances(buffers.atoms, atoms.data(), atoms.size() * sizeof(RKInPerInstanceAttributesAtoms),
                        static_cast<uint32_t>(atoms.size()));
      }
      if (auto *bondSource = dynamic_cast<RKRenderBondSource *>(_renderStructures[i][j].get());
          bondSource && bondSource->drawBonds())
      {
        const std::vector<RKInPerInstanceAttributesBonds> internal = bondSource->renderSelectedInternalBonds();
        uploadInstances(buffers.internalBonds, internal.data(), internal.size() * sizeof(RKInPerInstanceAttributesBonds),
                        static_cast<uint32_t>(internal.size()));
        const std::vector<RKInPerInstanceAttributesBonds> external = bondSource->renderSelectedExternalBonds();
        uploadInstances(buffers.externalBonds, external.data(), external.size() * sizeof(RKInPerInstanceAttributesBonds),
                        static_cast<uint32_t>(external.size()));
      }

      auto *source = dynamic_cast<RKRenderPrimitiveObjectsSource *>(_renderStructures[i][j].get());
      if (auto *ellipsoid = dynamic_cast<RKRenderPrimitiveEllipsoidObjectsSource *>(_renderStructures[i][j].get()))
      {
        const std::vector<RKInPerInstanceAttributesAtoms> data = ellipsoid->renderSelectedPrimitiveEllipsoidObjects();
        uploadInstances(buffers.ellipsoid, data.data(), data.size() * sizeof(RKInPerInstanceAttributesAtoms),
                        static_cast<uint32_t>(data.size()));
      }
      if (auto *crystalEllipsoid = dynamic_cast<RKRenderCrystalPrimitiveEllipsoidObjectsSource *>(_renderStructures[i][j].get()))
      {
        const std::vector<RKInPerInstanceAttributesAtoms> data = crystalEllipsoid->renderSelectedCrystalPrimitiveEllipsoidObjects();
        uploadInstances(buffers.crystalEllipsoid, data.data(), data.size() * sizeof(RKInPerInstanceAttributesAtoms),
                        static_cast<uint32_t>(data.size()));
      }

      auto *cylinderSource = dynamic_cast<RKRenderPrimitiveCylinderObjectsSource *>(_renderStructures[i][j].get());
      auto *crystalCylinderSource = dynamic_cast<RKRenderCrystalPrimitiveCylinderObjectsSource *>(_renderStructures[i][j].get());
      auto *prismSource = dynamic_cast<RKRenderPrimitivePolygonalPrimsObjectsSource *>(_renderStructures[i][j].get());
      auto *crystalPrismSource = dynamic_cast<RKRenderCrystalPrimitivePolygonalPrimsObjectsSource *>(_renderStructures[i][j].get());

      if (source && (cylinderSource || crystalCylinderSource))
      {
        const int sides = source->primitiveNumberOfSides();
        if (source->primitiveIsCapped())
        {
          CappedCylinderGeometry cylinder(1.0, sides);
          uploadMesh(buffers.cylinderMesh, cylinder.vertices(), cylinder.indices());
        }
        else
        {
          UnCappedCylinderGeometry cylinder(1.0, sides);
          uploadMesh(buffers.cylinderMesh, cylinder.vertices(), cylinder.indices());
        }
      }
      if (source && (prismSource || crystalPrismSource))
      {
        const int sides = source->primitiveNumberOfSides();
        if (source->primitiveIsCapped())
        {
          CappedNSidedPrismGeometry prism(1.0, sides);
          uploadMesh(buffers.prismMesh, prism.vertices(), prism.indices());
        }
        else
        {
          NSidedPrismGeometry prism(1.0, sides);
          uploadMesh(buffers.prismMesh, prism.vertices(), prism.indices());
        }
      }
      if (cylinderSource)
      {
        const std::vector<RKInPerInstanceAttributesAtoms> data = cylinderSource->renderSelectedPrimitiveCylinderObjects();
        uploadInstances(buffers.cylinder, data.data(), data.size() * sizeof(RKInPerInstanceAttributesAtoms),
                        static_cast<uint32_t>(data.size()));
      }
      if (crystalCylinderSource)
      {
        const std::vector<RKInPerInstanceAttributesAtoms> data = crystalCylinderSource->renderSelectedCrystalPrimitiveCylinderObjects();
        uploadInstances(buffers.crystalCylinder, data.data(), data.size() * sizeof(RKInPerInstanceAttributesAtoms),
                        static_cast<uint32_t>(data.size()));
      }
      if (prismSource)
      {
        const std::vector<RKInPerInstanceAttributesAtoms> data = prismSource->renderSelectedPrimitivePolygonalPrismObjects();
        uploadInstances(buffers.prism, data.data(), data.size() * sizeof(RKInPerInstanceAttributesAtoms),
                        static_cast<uint32_t>(data.size()));
      }
      if (crystalPrismSource)
      {
        const std::vector<RKInPerInstanceAttributesAtoms> data = crystalPrismSource->renderSelectedCrystalPrimitivePolygonalPrismObjects();
        uploadInstances(buffers.crystalPrism, data.data(), data.size() * sizeof(RKInPerInstanceAttributesAtoms),
                        static_cast<uint32_t>(data.size()));
      }
    }
  }
}

VkPipeline VulkanSelectionShader::pipelineForStyle(const StylePipelines &pipelines, RKSelectionStyle style) const
{
  switch (style)
  {
  case RKSelectionStyle::striped:
    return pipelines.stripes;
  case RKSelectionStyle::WorleyNoise3D:
    return pipelines.worley;
  case RKSelectionStyle::glow:
    return pipelines.glow;
  default:
    return VK_NULL_HANDLE;
  }
}

void VulkanSelectionShader::paintPrimitiveInstances(VkCommandBuffer commandBuffer, VkPipeline pipeline, const Mesh &mesh,
                                                    const InstanceDraw &instances, uint32_t structureIndex)
{
  if (!pipeline || instances.count == 0 || mesh.indexCount == 0 || !mesh.vertexBuffer.buffer || !mesh.indexBuffer.buffer ||
      !instances.buffer.buffer)
  {
    return;
  }
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), structureIndex);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offset);
  vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instances.buffer.buffer, &offset);
  vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(commandBuffer, mesh.indexCount, instances.count, 0, 0, 0);
}

void VulkanSelectionShader::paint(VkCommandBuffer commandBuffer, RKRenderQuality /*quality*/, bool orthographic)
{
  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      const StructureSelection &buffers = _structureBuffers[i][j];
      if (object && object->isVisible())
      {
        auto *atomSource = dynamic_cast<RKRenderAtomSource *>(object);
        const StylePipelines *atomPipelines = orthographic ? &_atomOrthoImposterPipelines : &_atomPerspImposterPipelines;
        const Mesh *atomMesh = &_quadMesh;
        VkPipeline atomPipeline = atomSource ? pipelineForStyle(*atomPipelines, atomSource->atomSelectionStyle()) : VK_NULL_HANDLE;
        if (atomPipeline && atomSource->drawAtoms() && buffers.atoms.count > 0 && atomMesh->indexCount > 0)
        {
          vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, atomPipeline);
          _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
          VkDeviceSize offset = 0;
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &atomMesh->vertexBuffer.buffer, &offset);
          vkCmdBindVertexBuffers(commandBuffer, 1, 1, &buffers.atoms.buffer.buffer, &offset);
          vkCmdBindIndexBuffer(commandBuffer, atomMesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
          vkCmdDrawIndexed(commandBuffer, atomMesh->indexCount, buffers.atoms.count, 0, 0, 0);
        }

        auto *bondSource = dynamic_cast<RKRenderBondSource *>(object);
        VkPipeline bondPipeline = bondSource ? pipelineForStyle(_bondPipelines, bondSource->bondSelectionStyle()) : VK_NULL_HANDLE;
        auto drawBonds = [&](const InstanceDraw &instances) {
          if (!bondPipeline || instances.count == 0 || !instances.buffer.buffer)
          {
            return;
          }
          vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, bondPipeline);
          _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
          VkDeviceSize offset = 0;
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &instances.buffer.buffer, &offset);
          const uint32_t vertexCount = bondSource->isUnity() ? 18u : 54u;
          vkCmdDraw(commandBuffer, vertexCount, instances.count, 0, 0);
        };
        if (bondSource && bondSource->drawBonds())
        {
          drawBonds(buffers.internalBonds);
          if (bondSource->hasExternalBonds())
          {
            drawBonds(buffers.externalBonds);
          }
        }

        auto *primitiveSource = dynamic_cast<RKRenderPrimitiveObjectsSource *>(object);
        if (primitiveSource && primitiveSource->drawAtoms())
        {
          VkPipeline stripPipeline = pipelineForStyle(_primitiveStripPipelines, primitiveSource->primitiveSelectionStyle());
          VkPipeline trianglePipeline = pipelineForStyle(_primitiveTrianglePipelines, primitiveSource->primitiveSelectionStyle());
          paintPrimitiveInstances(commandBuffer, stripPipeline, _sphereMesh, buffers.ellipsoid, flatIndex);
          paintPrimitiveInstances(commandBuffer, stripPipeline, _sphereMesh, buffers.crystalEllipsoid, flatIndex);
          paintPrimitiveInstances(commandBuffer, trianglePipeline, buffers.cylinderMesh, buffers.cylinder, flatIndex);
          paintPrimitiveInstances(commandBuffer, trianglePipeline, buffers.cylinderMesh, buffers.crystalCylinder, flatIndex);
          paintPrimitiveInstances(commandBuffer, trianglePipeline, buffers.prismMesh, buffers.prism, flatIndex);
          paintPrimitiveInstances(commandBuffer, trianglePipeline, buffers.prismMesh, buffers.crystalPrism, flatIndex);
        }

        if (_ribbonShader && atomSource)
        {
          VkPipeline ribbonPipeline = pipelineForStyle(_ribbonPipelines, atomSource->atomSelectionStyle());
          if (ribbonPipeline)
          {
            _ribbonShader->paintSelection(commandBuffer, ribbonPipeline, i, j, flatIndex);
          }
        }
      }
      ++flatIndex;
    }
  }
}
