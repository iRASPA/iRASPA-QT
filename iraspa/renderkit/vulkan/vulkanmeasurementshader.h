#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanMeasurementShader
{
public:
  explicit VulkanMeasurementShader(VulkanRenderer *renderer);
  ~VulkanMeasurementShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source);
  void reloadData();
  void paint(VkCommandBuffer commandBuffer, bool orthographic);

private:
  void destroyPipelines();
  void destroyInstanceBuffer();
  VkPipeline createPipeline(const QString &vertexResource);

  VulkanRenderer *_renderer = nullptr;
  std::shared_ptr<RKRenderDataSource> _dataSource;

  VulkanBuffer _quadVertexBuffer;
  VulkanBuffer _quadIndexBuffer;
  uint32_t _quadIndexCount = 0;
  VulkanBuffer _instanceBuffer;
  std::vector<uint32_t> _structureIndices;
  VkPipeline _orthoPipeline = VK_NULL_HANDLE;
  VkPipeline _perspPipeline = VK_NULL_HANDLE;
};
