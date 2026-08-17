#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanUnitCellShader
{
public:
  explicit VulkanUnitCellShader(VulkanRenderer *renderer);
  ~VulkanUnitCellShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void paint(VkCommandBuffer commandBuffer);

private:
  void destroyStructureBuffers();

  struct StructureBuffers
  {
    VulkanBuffer sphereInstanceBuffer;
    uint32_t sphereInstanceCount = 0;
    VulkanBuffer cylinderInstanceBuffer;
    uint32_t cylinderInstanceCount = 0;
  };

  VulkanRenderer *_renderer = nullptr;
  VkPipeline _spherePipeline = VK_NULL_HANDLE;
  VkPipeline _cylinderPipeline = VK_NULL_HANDLE;
  VulkanBuffer _sphereVertexBuffer;
  VulkanBuffer _sphereIndexBuffer;
  uint32_t _sphereIndexCount = 0;
  VulkanBuffer _cylinderVertexBuffer;
  VulkanBuffer _cylinderIndexBuffer;
  uint32_t _cylinderIndexCount = 0;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureBuffers>> _structureBuffers;
};
