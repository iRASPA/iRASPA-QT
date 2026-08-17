#include "vulkanprimitiveobjectshader.h"

#include "cappedcylindergeometry.h"
#include "cappednsidedprismgeometry.h"
#include "nsidedprismgeometry.h"
#include "spheregeometry.h"
#include "uncappedcylindergeometry.h"
#include "vulkanshader.h"

#include <cstddef>

namespace
{
VkVertexInputAttributeDescription primitiveAttribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}
}  // namespace

VulkanPrimitiveObjectShader::VulkanPrimitiveObjectShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanPrimitiveObjectShader::~VulkanPrimitiveObjectShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyStructureBuffers();
  auto destroySet = [this](PipelineSet &set) {
    if (set.opaque)
    {
      vkDestroyPipeline(_renderer->device(), set.opaque, nullptr);
    }
    if (set.transparentFront)
    {
      vkDestroyPipeline(_renderer->device(), set.transparentFront, nullptr);
    }
    if (set.transparentBack)
    {
      vkDestroyPipeline(_renderer->device(), set.transparentBack, nullptr);
    }
  };
  destroySet(_stripPipelines);
  destroySet(_trianglePipelines);
  _renderer->destroyBuffer(_sphereMesh.vertexBuffer);
  _renderer->destroyBuffer(_sphereMesh.indexBuffer);
}

void VulkanPrimitiveObjectShader::destroyStructureBuffers()
{
  for (auto &scene : _structureBuffers)
  {
    for (auto &buffers : scene)
    {
      _renderer->destroyBuffer(buffers.ellipsoid.buffer);
      _renderer->destroyBuffer(buffers.crystalEllipsoid.buffer);
      _renderer->destroyBuffer(buffers.cylinderMesh.vertexBuffer);
      _renderer->destroyBuffer(buffers.cylinderMesh.indexBuffer);
      _renderer->destroyBuffer(buffers.cylinder.buffer);
      _renderer->destroyBuffer(buffers.crystalCylinder.buffer);
      _renderer->destroyBuffer(buffers.prismMesh.vertexBuffer);
      _renderer->destroyBuffer(buffers.prismMesh.indexBuffer);
      _renderer->destroyBuffer(buffers.prism.buffer);
      _renderer->destroyBuffer(buffers.crystalPrism.buffer);
    }
  }
  _structureBuffers.clear();
}

void VulkanPrimitiveObjectShader::uploadMesh(Mesh &mesh, const std::vector<RKVertex> &vertices, const std::vector<short> &indices)
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

void VulkanPrimitiveObjectShader::uploadInstances(Instances &instances, const std::vector<RKInPerInstanceAttributesAtoms> &data)
{
  instances.count = static_cast<uint32_t>(data.size());
  if (!data.empty())
  {
    _renderer->uploadBuffer(instances.buffer, data.data(), data.size() * sizeof(RKInPerInstanceAttributesAtoms),
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
}

VulkanPrimitiveObjectShader::PipelineSet VulkanPrimitiveObjectShader::createPipelines(VkPrimitiveTopology topology,
                                                                                    VkCullModeFlags opaqueCull)
{
  auto makeConfig = [&](VkCullModeFlags cull, bool blend, bool depthWrite) {
    VulkanShader::PipelineConfig config;
    config.vertexShaderResource = QStringLiteral(":/shaders/primitive_object.vert.spv");
    config.fragmentShaderResource = QStringLiteral(":/shaders/primitive_object.frag.spv");
    config.topology = topology;
    config.cullMode = cull;
    config.depthTest = true;
    config.depthWrite = depthWrite;
    config.blend = blend;
    if (blend)
    {
      config.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
      config.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    }

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
        primitiveAttribute(0, 0, offsetof(RKVertex, position)),
        primitiveAttribute(1, 0, offsetof(RKVertex, normal)),
        primitiveAttribute(2, 1, offsetof(RKInPerInstanceAttributesAtoms, position)),
    };
    return config;
  };

  PipelineSet set;
  set.opaque = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(),
                                                    makeConfig(opaqueCull, false, true));
  set.transparentFront = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(),
                                                              makeConfig(VK_CULL_MODE_FRONT_BIT, true, false));
  set.transparentBack = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(),
                                                             makeConfig(VK_CULL_MODE_BACK_BIT, true, false));
  return set;
}

void VulkanPrimitiveObjectShader::initialize()
{
  SphereGeometry sphere(1.0, 41);
  uploadMesh(_sphereMesh, sphere.vertices(), sphere.indices());
  _stripPipelines = createPipelines(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_CULL_MODE_BACK_BIT);
  _trianglePipelines = createPipelines(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE);
}

void VulkanPrimitiveObjectShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  reloadData();
}

void VulkanPrimitiveObjectShader::reloadData()
{
  _renderer->waitIdle();
  destroyStructureBuffers();
  _structureBuffers.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureBuffers[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *source = dynamic_cast<RKRenderPrimitiveObjectsSource *>(_renderStructures[i][j].get());
      StructurePrimitives &buffers = _structureBuffers[i][j];

      if (auto *ellipsoid = dynamic_cast<RKRenderPrimitiveEllipsoidObjectsSource *>(_renderStructures[i][j].get()))
      {
        uploadInstances(buffers.ellipsoid, ellipsoid->renderPrimitiveEllipsoidObjects());
      }
      if (auto *crystalEllipsoid = dynamic_cast<RKRenderCrystalPrimitiveEllipsoidObjectsSource *>(_renderStructures[i][j].get()))
      {
        uploadInstances(buffers.crystalEllipsoid, crystalEllipsoid->renderCrystalPrimitiveEllipsoidObjects());
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
        uploadInstances(buffers.cylinder, cylinderSource->renderPrimitiveCylinderObjects());
      }
      if (crystalCylinderSource)
      {
        uploadInstances(buffers.crystalCylinder, crystalCylinderSource->renderCrystalPrimitiveCylinderObjects());
      }
      if (prismSource)
      {
        uploadInstances(buffers.prism, prismSource->renderPrimitivePolygonalPrismObjects());
      }
      if (crystalPrismSource)
      {
        uploadInstances(buffers.crystalPrism, crystalPrismSource->renderCrystalPrimitivePolygonalPrismObjects());
      }
    }
  }
}

void VulkanPrimitiveObjectShader::paintOpaque(VkCommandBuffer commandBuffer)
{
  paint(commandBuffer, true, -1, -1);
}

void VulkanPrimitiveObjectShader::paintTransparent(VkCommandBuffer commandBuffer)
{
  paint(commandBuffer, false, -1, -1);
}

void VulkanPrimitiveObjectShader::paintOpaque(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex)
{
  paint(commandBuffer, true, static_cast<int>(sceneIndex), static_cast<int>(movieIndex));
}

void VulkanPrimitiveObjectShader::paintTransparent(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex)
{
  paint(commandBuffer, false, static_cast<int>(sceneIndex), static_cast<int>(movieIndex));
}

void VulkanPrimitiveObjectShader::paintInstances(VkCommandBuffer commandBuffer, const PipelineSet &pipelines, const Mesh &mesh,
                                                const Instances &instances, uint32_t structureIndex, bool opaque,
                                                RKRenderPrimitiveObjectsSource *source)
{
  if (!source || !source->drawAtoms() || mesh.indexCount == 0 || instances.count == 0 || !mesh.vertexBuffer.buffer ||
      !mesh.indexBuffer.buffer || !instances.buffer.buffer)
  {
    return;
  }
  const bool isOpaque = source->primitiveOpacity() > 0.99999;
  if (opaque != isOpaque)
  {
    return;
  }

  auto drawWith = [&](VkPipeline pipeline) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), structureIndex);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offset);
    vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instances.buffer.buffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, mesh.indexCount, instances.count, 0, 0, 0);
  };

  if (opaque)
  {
    drawWith(pipelines.opaque);
  }
  else
  {
    drawWith(pipelines.transparentFront);
    drawWith(pipelines.transparentBack);
  }
}

void VulkanPrimitiveObjectShader::paint(VkCommandBuffer commandBuffer, bool opaque)
{
  paint(commandBuffer, opaque, -1, -1);
}

void VulkanPrimitiveObjectShader::paint(VkCommandBuffer commandBuffer, bool opaque, int sceneIndex, int movieIndex)
{
  if (!_stripPipelines.opaque || !_trianglePipelines.opaque)
  {
    return;
  }

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *source = dynamic_cast<RKRenderPrimitiveObjectsSource *>(object);
      if (object && object->isVisible() && source &&
          (sceneIndex < 0 || (static_cast<int>(i) == sceneIndex && static_cast<int>(j) == movieIndex)))
      {
        const StructurePrimitives &buffers = _structureBuffers[i][j];
        paintInstances(commandBuffer, _stripPipelines, _sphereMesh, buffers.ellipsoid, flatIndex, opaque, source);
        paintInstances(commandBuffer, _stripPipelines, _sphereMesh, buffers.crystalEllipsoid, flatIndex, opaque, source);
        paintInstances(commandBuffer, _trianglePipelines, buffers.cylinderMesh, buffers.cylinder, flatIndex, opaque, source);
        paintInstances(commandBuffer, _trianglePipelines, buffers.cylinderMesh, buffers.crystalCylinder, flatIndex, opaque, source);
        paintInstances(commandBuffer, _trianglePipelines, buffers.prismMesh, buffers.prism, flatIndex, opaque, source);
        paintInstances(commandBuffer, _trianglePipelines, buffers.prismMesh, buffers.crystalPrism, flatIndex, opaque, source);
      }
      ++flatIndex;
    }
  }
}

void VulkanPrimitiveObjectShader::paintPickInstances(VkCommandBuffer commandBuffer, VkPipeline pipeline, const Mesh &mesh,
                                                    const Instances &instances, uint32_t structureIndex,
                                                    RKRenderPrimitiveObjectsSource *source)
{
  if (!pipeline || !source || !source->drawAtoms() || mesh.indexCount == 0 || instances.count == 0 || !mesh.vertexBuffer.buffer ||
      !mesh.indexBuffer.buffer || !instances.buffer.buffer)
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

void VulkanPrimitiveObjectShader::paintPick(VkCommandBuffer commandBuffer, VkPipeline stripPipeline, VkPipeline trianglePipeline)
{
  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *source = dynamic_cast<RKRenderPrimitiveObjectsSource *>(object);
      if (object && object->isVisible() && source)
      {
        const StructurePrimitives &buffers = _structureBuffers[i][j];
        paintPickInstances(commandBuffer, stripPipeline, _sphereMesh, buffers.ellipsoid, flatIndex, source);
        paintPickInstances(commandBuffer, stripPipeline, _sphereMesh, buffers.crystalEllipsoid, flatIndex, source);
        paintPickInstances(commandBuffer, trianglePipeline, buffers.cylinderMesh, buffers.cylinder, flatIndex, source);
        paintPickInstances(commandBuffer, trianglePipeline, buffers.cylinderMesh, buffers.crystalCylinder, flatIndex, source);
        paintPickInstances(commandBuffer, trianglePipeline, buffers.prismMesh, buffers.prism, flatIndex, source);
        paintPickInstances(commandBuffer, trianglePipeline, buffers.prismMesh, buffers.crystalPrism, flatIndex, source);
      }
      ++flatIndex;
    }
  }
}
