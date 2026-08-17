#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "ribbonaolayout.h"
#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanAtomSphereShader;
class VulkanRibbonAmbientOcclusionShader;

class VulkanRibbonShader
{
public:
  VulkanRibbonShader(VulkanRenderer *renderer, VulkanAtomSphereShader *atomShader);
  ~VulkanRibbonShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void reloadAmbientOcclusionData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality);
  VulkanRibbonAmbientOcclusionShader *ambientOcclusionShader() const { return _aoShader.get(); }
  void invalidateCachedAmbientOcclusionTexture(const std::vector<std::shared_ptr<RKRenderObject>> &structures);
  void paint(VkCommandBuffer commandBuffer);
  RibbonAODebugMode aoDebugMode() const { return _aoDebugMode; }
  void setAoDebugMode(RibbonAODebugMode mode) { _aoDebugMode = mode; }
  RibbonAODebugUniforms debugUniforms(int viewportWidth, int viewportHeight) const;
  void paintPick(VkCommandBuffer commandBuffer, VkPipeline pipeline);
  void paintSelection(VkCommandBuffer commandBuffer, VkPipeline pipeline, size_t sceneIndex, size_t movieIndex,
                      uint32_t structureIndex);

  uint32_t indexCount(size_t sceneIndex, size_t movieIndex) const;
  VkBuffer vertexBuffer(size_t sceneIndex, size_t movieIndex) const;
  VkBuffer indexBuffer(size_t sceneIndex, size_t movieIndex) const;
  void drawAllChains(VkCommandBuffer commandBuffer, RKRenderRibbonSource *ribbonSource) const;

private:
  void destroyStructureBuffers();
  void drawRibbonRanges(VkCommandBuffer commandBuffer, RKRenderRibbonSource *ribbonSource) const;
  void drawSelectedRibbonRanges(VkCommandBuffer commandBuffer, RKRenderRibbonSource *ribbonSource) const;

  struct StructureBuffers
  {
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
    uint32_t indexCount = 0;
  };

  VulkanRenderer *_renderer = nullptr;
  VulkanAtomSphereShader *_atomShader = nullptr;
  RibbonAODebugMode _aoDebugMode = RibbonAODebugMode::off;
  VkPipeline _pipeline = VK_NULL_HANDLE;
  VkDescriptorSet _aoSamplerSet = VK_NULL_HANDLE;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureBuffers>> _structureBuffers;
  std::unique_ptr<VulkanRibbonAmbientOcclusionShader> _aoShader;
};
