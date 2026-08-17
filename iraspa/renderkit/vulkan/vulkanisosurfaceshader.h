#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanIsosurfaceShader
{
public:
  explicit VulkanIsosurfaceShader(VulkanRenderer *renderer);
  ~VulkanIsosurfaceShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void invalidateIsosurface(const std::vector<std::shared_ptr<RKRenderObject>> &structures);
  void paintOpaque(VkCommandBuffer commandBuffer);
  void paintTransparent(VkCommandBuffer commandBuffer);
  void paintOpaque(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex);
  void paintTransparent(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex);

private:
  void destroyStructureBuffers();
  void paint(VkCommandBuffer commandBuffer, bool opaque);
  void paint(VkCommandBuffer commandBuffer, bool opaque, int sceneIndex, int movieIndex);

  struct StructureBuffers
  {
    VulkanBuffer vertexBuffer;
    VulkanBuffer instanceBuffer;
    uint32_t triangleCount = 0;
    uint32_t instanceCount = 0;
  };

  VulkanRenderer *_renderer = nullptr;
  VkPipeline _opaquePipeline = VK_NULL_HANDLE;
  VkPipeline _transparentFrontPipeline = VK_NULL_HANDLE;
  VkPipeline _transparentBackPipeline = VK_NULL_HANDLE;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureBuffers>> _structureBuffers;
  std::unordered_map<RKRenderObject *, std::shared_ptr<std::vector<float>>> _gridCache;
};
