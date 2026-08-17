#include "vulkanboundingboxshader.h"

#include "cylindergeometry.h"
#include "spheregeometry.h"
#include "vulkanshader.h"

#include <cstddef>

namespace
{
VkVertexInputAttributeDescription boundingBoxAttribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}
}  // namespace

VulkanBoundingBoxShader::VulkanBoundingBoxShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanBoundingBoxShader::~VulkanBoundingBoxShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyInstanceBuffers();
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

void VulkanBoundingBoxShader::destroyInstanceBuffers()
{
  _renderer->destroyBuffer(_sphereInstanceBuffer);
  _renderer->destroyBuffer(_cylinderInstanceBuffer);
  _sphereInstanceCount = 0;
  _cylinderInstanceCount = 0;
}

void VulkanBoundingBoxShader::initialize()
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
  sphereConfig.vertexShaderResource = QStringLiteral(":/shaders/bounding_box_sphere.vert.spv");
  sphereConfig.fragmentShaderResource = QStringLiteral(":/shaders/bounding_box.frag.spv");
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
      boundingBoxAttribute(0, 0, offsetof(RKVertex, position)),
      boundingBoxAttribute(1, 0, offsetof(RKVertex, normal)),
      boundingBoxAttribute(2, 1, offsetof(RKInPerInstanceAttributesAtoms, position)),
      boundingBoxAttribute(3, 1, offsetof(RKInPerInstanceAttributesAtoms, scale)),
  };
  _spherePipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), sphereConfig);

  VulkanShader::PipelineConfig cylinderConfig;
  cylinderConfig.vertexShaderResource = QStringLiteral(":/shaders/bounding_box_cylinder.vert.spv");
  cylinderConfig.fragmentShaderResource = QStringLiteral(":/shaders/bounding_box.frag.spv");
  cylinderConfig.cullMode = VK_CULL_MODE_BACK_BIT;
  cylinderConfig.depthTest = true;
  cylinderConfig.depthWrite = true;
  cylinderConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

  VkVertexInputBindingDescription cylinderInstanceBinding{};
  cylinderInstanceBinding.binding = 1;
  cylinderInstanceBinding.stride = sizeof(RKInPerInstanceAttributesBonds);
  cylinderInstanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  cylinderConfig.bindings = {sphereVertexBinding, cylinderInstanceBinding};
  cylinderConfig.attributes = {
      boundingBoxAttribute(0, 0, offsetof(RKVertex, position)),
      boundingBoxAttribute(1, 0, offsetof(RKVertex, normal)),
      boundingBoxAttribute(2, 1, offsetof(RKInPerInstanceAttributesBonds, position1)),
      boundingBoxAttribute(3, 1, offsetof(RKInPerInstanceAttributesBonds, position2)),
      boundingBoxAttribute(4, 1, offsetof(RKInPerInstanceAttributesBonds, scale)),
  };
  _cylinderPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), cylinderConfig);
}

void VulkanBoundingBoxShader::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = std::move(source);
  reloadData();
}

void VulkanBoundingBoxShader::reloadData()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyInstanceBuffers();

  if (!_dataSource)
  {
    return;
  }

  const auto spheres = _dataSource->renderBoundingBoxSpheres();
  _sphereInstanceCount = static_cast<uint32_t>(spheres.size());
  if (!spheres.empty())
  {
    _renderer->uploadBuffer(_sphereInstanceBuffer, spheres.data(), spheres.size() * sizeof(RKInPerInstanceAttributesAtoms),
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }

  const auto cylinders = _dataSource->renderBoundingBoxCylinders();
  _cylinderInstanceCount = static_cast<uint32_t>(cylinders.size());
  if (!cylinders.empty())
  {
    _renderer->uploadBuffer(_cylinderInstanceBuffer, cylinders.data(),
                            cylinders.size() * sizeof(RKInPerInstanceAttributesBonds), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
}

void VulkanBoundingBoxShader::paint(VkCommandBuffer commandBuffer)
{
  if (!_spherePipeline || !_cylinderPipeline || !_dataSource || !_dataSource->showBoundingBox())
  {
    return;
  }

  if (_sphereInstanceCount > 0 && _sphereIndexCount > 0)
  {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _spherePipeline);
    _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), 0);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_sphereVertexBuffer.buffer, &offset);
    vkCmdBindVertexBuffers(commandBuffer, 1, 1, &_sphereInstanceBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, _sphereIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, _sphereIndexCount, _sphereInstanceCount, 0, 0, 0);
  }

  if (_cylinderInstanceCount > 0 && _cylinderIndexCount > 0)
  {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _cylinderPipeline);
    _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), 0);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_cylinderVertexBuffer.buffer, &offset);
    vkCmdBindVertexBuffers(commandBuffer, 1, 1, &_cylinderInstanceBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, _cylinderIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, _cylinderIndexCount, _cylinderInstanceCount, 0, 0, 0);
  }
}
