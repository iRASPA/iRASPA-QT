#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanBoundingBoxShader
{
public:
  explicit VulkanBoundingBoxShader(VulkanRenderer *renderer);
  ~VulkanBoundingBoxShader();

  void initialize();
  void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source);
  void reloadData();
  void paint(VkCommandBuffer commandBuffer);

private:
  void destroyInstanceBuffers();

  VulkanRenderer *_renderer = nullptr;
  std::shared_ptr<RKRenderDataSource> _dataSource;

  VkPipeline _spherePipeline = VK_NULL_HANDLE;
  VkPipeline _cylinderPipeline = VK_NULL_HANDLE;
  VulkanBuffer _sphereVertexBuffer;
  VulkanBuffer _sphereIndexBuffer;
  uint32_t _sphereIndexCount = 0;
  VulkanBuffer _cylinderVertexBuffer;
  VulkanBuffer _cylinderIndexBuffer;
  uint32_t _cylinderIndexCount = 0;

  VulkanBuffer _sphereInstanceBuffer;
  uint32_t _sphereInstanceCount = 0;
  VulkanBuffer _cylinderInstanceBuffer;
  uint32_t _cylinderInstanceCount = 0;
};
