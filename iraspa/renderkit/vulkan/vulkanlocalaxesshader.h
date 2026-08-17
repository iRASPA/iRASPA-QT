#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanLocalAxesShader
{
public:
  explicit VulkanLocalAxesShader(VulkanRenderer *renderer);
  ~VulkanLocalAxesShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void paint(VkCommandBuffer commandBuffer);

private:
  void destroyStructureBuffers();

  struct StructureBuffers
  {
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
    uint32_t indexCount = 0;
  };

  VulkanRenderer *_renderer = nullptr;
  VkPipeline _pipeline = VK_NULL_HANDLE;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureBuffers>> _structureBuffers;
};
