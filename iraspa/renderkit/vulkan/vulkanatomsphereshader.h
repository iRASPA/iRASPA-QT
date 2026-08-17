#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanAtomAmbientOcclusionShader;

class VulkanAtomSphereShader
{
public:
  explicit VulkanAtomSphereShader(VulkanRenderer *renderer);
  ~VulkanAtomSphereShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void reloadAmbientOcclusionData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality,
                                  class VulkanRibbonShader *ribbonShader = nullptr);
  void invalidateCachedAmbientOcclusionTexture(const std::vector<std::shared_ptr<RKRenderObject>> &structures);
  void paint(VkCommandBuffer commandBuffer, RKRenderQuality quality, bool orthographic);
  void paintPick(VkCommandBuffer commandBuffer, VkPipeline pipeline);
  uint32_t numberOfAtoms() const { return _numberOfAtoms; }

  uint32_t instanceCount(size_t sceneIndex, size_t movieIndex) const;
  VkBuffer instanceBuffer(size_t sceneIndex, size_t movieIndex) const;
  VulkanAtomAmbientOcclusionShader *ambientOcclusionShader() const;

private:
  void destroyStructureBuffers();
  void paintInstances(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkBuffer vertexBuffer, VkBuffer indexBuffer,
                      uint32_t indexCount, bool bindAmbientOcclusion);

  struct StructureBuffers
  {
    VulkanBuffer instanceBuffer;
    uint32_t instanceCount = 0;
  };

  VulkanRenderer *_renderer = nullptr;
  VkPipeline _orthoImposterPipeline = VK_NULL_HANDLE;
  VkPipeline _perspImposterPipeline = VK_NULL_HANDLE;
  VkPipeline _orthoImposterPerPixelPipeline = VK_NULL_HANDLE;
  VkPipeline _perspImposterPerPixelPipeline = VK_NULL_HANDLE;
  VulkanBuffer _imposterVertexBuffer;
  VulkanBuffer _imposterIndexBuffer;
  uint32_t _imposterIndexCount = 0;
  uint32_t _numberOfAtoms = 0;
  VkDescriptorSet _aoSamplerSet = VK_NULL_HANDLE;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureBuffers>> _structureBuffers;
  std::unique_ptr<VulkanAtomAmbientOcclusionShader> _aoShader;
};
