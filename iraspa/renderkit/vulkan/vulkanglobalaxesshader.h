#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanGlobalAxesShader
{
public:
  explicit VulkanGlobalAxesShader(VulkanRenderer *renderer);
  ~VulkanGlobalAxesShader();

  void initialize();
  void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source);
  void reloadData();
  void paint(VkCommandBuffer commandBuffer);

private:
  struct AxesTextInstance
  {
    float4 position;
    float4 scale;
    float4 vertexCoordinatesData;
    float4 textureCoordinatesData;
    float4 axisId;
  };

  void destroyGpuResources();
  void setAxesViewport(VkCommandBuffer commandBuffer, float x, float yOpenGL, float width, float height) const;
  void restoreViewport(VkCommandBuffer commandBuffer) const;

  VulkanRenderer *_renderer = nullptr;
  std::shared_ptr<RKRenderDataSource> _dataSource;

  VkPipeline _backgroundPipeline = VK_NULL_HANDLE;
  VkPipeline _systemPipeline = VK_NULL_HANDLE;
  VkPipeline _textPipeline = VK_NULL_HANDLE;

  VulkanBuffer _backgroundVertexBuffer;
  VulkanBuffer _backgroundIndexBuffer;
  uint32_t _backgroundIndexCount = 0;

  VulkanBuffer _systemVertexBuffer;
  VulkanBuffer _systemIndexBuffer;
  uint32_t _systemIndexCount = 0;

  VulkanBuffer _textInstanceBuffer;
  uint32_t _textInstanceCount = 0;
  VulkanTexture _fontTexture;
  VkDescriptorSet _fontSamplerSet = VK_NULL_HANDLE;
};
