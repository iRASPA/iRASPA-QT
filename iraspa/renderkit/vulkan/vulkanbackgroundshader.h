#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanBackgroundShader
{
public:
  explicit VulkanBackgroundShader(VulkanRenderer *renderer);
  ~VulkanBackgroundShader();

  void initialize();
  void reload(std::shared_ptr<RKRenderDataSource> source);
  void paint(VkCommandBuffer commandBuffer);

private:
  VulkanRenderer *_renderer = nullptr;
  VkPipeline _pipeline = VK_NULL_HANDLE;
  VulkanBuffer _vertexBuffer;
  VulkanBuffer _indexBuffer;
  VulkanTexture _texture;
  VkDescriptorSet _samplerSet = VK_NULL_HANDLE;
  uint32_t _indexCount = 0;
};
