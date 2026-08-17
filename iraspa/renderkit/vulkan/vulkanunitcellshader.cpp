#include "vulkanunitcellshader.h"

#include "cylindergeometry.h"
#include "spheregeometry.h"
#include "vulkanshader.h"

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
}  // namespace

VulkanUnitCellShader::VulkanUnitCellShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanUnitCellShader::~VulkanUnitCellShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyStructureBuffers();
  if (_spherePipeline)
  {
    vkDestroyPipeline(_renderer->device(), _spherePipeline, nullptr);
  }
  if (_cylinderPipeline)
  {
    vkDestroyPipeline(_renderer->device(), _cylinderPipeline, nullptr);
  }
  _renderer->destroyBuffer(_sphereVertexBuffer);
  _renderer->destroyBuffer(_sphereIndexBuffer);
  _renderer->destroyBuffer(_cylinderVertexBuffer);
  _renderer->destroyBuffer(_cylinderIndexBuffer);
}

void VulkanUnitCellShader::destroyStructureBuffers()
{
  for (auto &scene : _structureBuffers)
  {
    for (auto &buffers : scene)
    {
      _renderer->destroyBuffer(buffers.sphereInstanceBuffer);
      _renderer->destroyBuffer(buffers.cylinderInstanceBuffer);
    }
  }
  _structureBuffers.clear();
}

void VulkanUnitCellShader::initialize()
{
  SphereGeometry sphere(1.0, 41);
  const auto sphereVertices = sphere.vertices();
  const auto sphereIndices = sphere.indices();
  _sphereIndexCount = static_cast<uint32_t>(sphereIndices.size());
  _renderer->uploadBuffer(_sphereVertexBuffer, sphereVertices.data(), sphereVertices.size() * sizeof(RKVertex),
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  _renderer->uploadBuffer(_sphereIndexBuffer, sphereIndices.data(), sphereIndices.size() * sizeof(short),
                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  CylinderGeometry cylinder(1.0, 41);
  const auto cylinderVertices = cylinder.vertices();
  const auto cylinderIndices = cylinder.indices();
  _cylinderIndexCount = static_cast<uint32_t>(cylinderIndices.size());
  _renderer->uploadBuffer(_cylinderVertexBuffer, cylinderVertices.data(), cylinderVertices.size() * sizeof(RKVertex),
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  _renderer->uploadBuffer(_cylinderIndexBuffer, cylinderIndices.data(), cylinderIndices.size() * sizeof(short),
                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  VulkanShader::PipelineConfig sphereConfig;
  sphereConfig.vertexShaderResource = QStringLiteral(":/shaders/unit_cell_sphere.vert.spv");
  sphereConfig.fragmentShaderResource = QStringLiteral(":/shaders/unit_cell_sphere.frag.spv");
  sphereConfig.cullMode = VK_CULL_MODE_BACK_BIT;
  sphereConfig.depthTest = true;
  sphereConfig.depthWrite = true;
  sphereConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

  VkVertexInputBindingDescription sphereVertexBinding{};
  sphereVertexBinding.binding = 0;
  sphereVertexBinding.stride = sizeof(RKVertex);
  sphereVertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  VkVertexInputBindingDescription sphereInstanceBinding{};
  sphereInstanceBinding.binding = 1;
  sphereInstanceBinding.stride = sizeof(RKInPerInstanceAttributesAtoms);
  sphereInstanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  sphereConfig.bindings = {sphereVertexBinding, sphereInstanceBinding};
  sphereConfig.attributes = {
      attribute(0, 0, offsetof(RKVertex, position)),
      attribute(1, 0, offsetof(RKVertex, normal)),
      attribute(2, 1, offsetof(RKInPerInstanceAttributesAtoms, position)),
      attribute(3, 1, offsetof(RKInPerInstanceAttributesAtoms, scale)),
  };
  _spherePipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), sphereConfig);

  VulkanShader::PipelineConfig cylinderConfig;
  cylinderConfig.vertexShaderResource = QStringLiteral(":/shaders/unit_cell_cylinder.vert.spv");
  cylinderConfig.fragmentShaderResource = QStringLiteral(":/shaders/unit_cell_cylinder.frag.spv");
  cylinderConfig.cullMode = VK_CULL_MODE_BACK_BIT;
  cylinderConfig.depthTest = true;
  cylinderConfig.depthWrite = true;
  cylinderConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

  VkVertexInputBindingDescription cylinderVertexBinding = sphereVertexBinding;
  VkVertexInputBindingDescription cylinderInstanceBinding{};
  cylinderInstanceBinding.binding = 1;
  cylinderInstanceBinding.stride = sizeof(RKInPerInstanceAttributesBonds);
  cylinderInstanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  cylinderConfig.bindings = {cylinderVertexBinding, cylinderInstanceBinding};
  cylinderConfig.attributes = {
      attribute(0, 0, offsetof(RKVertex, position)),
      attribute(1, 0, offsetof(RKVertex, normal)),
      attribute(2, 1, offsetof(RKInPerInstanceAttributesBonds, position1)),
      attribute(3, 1, offsetof(RKInPerInstanceAttributesBonds, position2)),
      attribute(4, 1, offsetof(RKInPerInstanceAttributesBonds, scale)),
  };
  _cylinderPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), cylinderConfig);
}

void VulkanUnitCellShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  reloadData();
}

void VulkanUnitCellShader::reloadData()
{
  destroyStructureBuffers();
  _structureBuffers.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureBuffers[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *source = dynamic_cast<RKRenderUnitCellSource *>(_renderStructures[i][j].get());
      if (!source)
      {
        continue;
      }
      const auto spheres = source->renderUnitCellSpheres();
      _structureBuffers[i][j].sphereInstanceCount = static_cast<uint32_t>(spheres.size());
      if (!spheres.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].sphereInstanceBuffer, spheres.data(),
                                spheres.size() * sizeof(RKInPerInstanceAttributesAtoms), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
      }
      const auto cylinders = source->renderUnitCellCylinders();
      _structureBuffers[i][j].cylinderInstanceCount = static_cast<uint32_t>(cylinders.size());
      if (!cylinders.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].cylinderInstanceBuffer, cylinders.data(),
                                cylinders.size() * sizeof(RKInPerInstanceAttributesBonds), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
      }
    }
  }
}

void VulkanUnitCellShader::paint(VkCommandBuffer commandBuffer)
{
  if (!_spherePipeline || !_cylinderPipeline)
  {
    return;
  }

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *source = dynamic_cast<RKRenderUnitCellSource *>(object);
      const StructureBuffers &buffers = _structureBuffers[i][j];
      if (source && source->drawUnitCell() && object->isVisible())
      {
        if (buffers.sphereInstanceCount > 0 && _sphereIndexCount > 0)
        {
          vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _spherePipeline);
          _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
          VkDeviceSize offset = 0;
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_sphereVertexBuffer.buffer, &offset);
          vkCmdBindVertexBuffers(commandBuffer, 1, 1, &buffers.sphereInstanceBuffer.buffer, &offset);
          vkCmdBindIndexBuffer(commandBuffer, _sphereIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
          vkCmdDrawIndexed(commandBuffer, _sphereIndexCount, buffers.sphereInstanceCount, 0, 0, 0);
        }
        if (buffers.cylinderInstanceCount > 0 && _cylinderIndexCount > 0)
        {
          vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _cylinderPipeline);
          _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
          VkDeviceSize offset = 0;
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_cylinderVertexBuffer.buffer, &offset);
          vkCmdBindVertexBuffers(commandBuffer, 1, 1, &buffers.cylinderInstanceBuffer.buffer, &offset);
          vkCmdBindIndexBuffer(commandBuffer, _cylinderIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
          vkCmdDrawIndexed(commandBuffer, _cylinderIndexCount, buffers.cylinderInstanceCount, 0, 0, 0);
        }
      }
      ++flatIndex;
    }
  }
}
