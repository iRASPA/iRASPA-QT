#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "rkrenderuniforms.h"
#include "vulkanrenderer.h"

class VulkanAtomSphereShader;
class VulkanRibbonAmbientOcclusionShader;

class VulkanAtomAmbientOcclusionShader
{
public:
  VulkanAtomAmbientOcclusionShader(VulkanRenderer *renderer, VulkanAtomSphereShader *atomShader);
  ~VulkanAtomAmbientOcclusionShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality,
                  VulkanRibbonAmbientOcclusionShader *ribbonAO = nullptr);
  void invalidateCachedAmbientOcclusionTexture(const std::vector<std::shared_ptr<RKRenderObject>> &structures);
  VkDescriptorSet samplerSet(size_t sceneIndex, size_t movieIndex) const;

  bool wantsBake(size_t sceneIndex, size_t movieIndex) const;
  bool hasCachedTexture(RKRenderObject *key, uint32_t textureSize) const;
  bool prepareTarget(size_t sceneIndex, size_t movieIndex);
  void setGenerationBuffers(const VulkanBuffer &structureBuffer, const VulkanBuffer &shadowBuffer);
  void useShadowMap(VkImageView shadowMapView);
  void restoreDefaultShadowMap();
  void recordClear(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex);
  void recordAccumulate(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex, uint32_t directionIndex,
                        VkDeviceSize structureStride, VkDeviceSize shadowStride, float weight);
  void finalizeTarget(size_t sceneIndex, size_t movieIndex);

private:
  struct StructureResources
  {
    VulkanTexture texture;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkDescriptorSet samplerSet = VK_NULL_HANDLE;
    uint32_t textureSize = 0;
  };

  void destroyStructureResources();
  void destroyPipelines();
  void createRenderPasses();
  void createShadowResources();
  void destroyShadowResources();
  void createPipelines();
  void createGenerationDescriptors();
  void adjustTextureSizes();
  void generateTextures(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality,
                        VulkanRibbonAmbientOcclusionShader *ribbonAO);
  void recordImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout,
                          VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
                          VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

  VulkanRenderer *_renderer = nullptr;
  VulkanAtomSphereShader *_atomShader = nullptr;

  VkRenderPass _shadowRenderPass = VK_NULL_HANDLE;
  VkRenderPass _aoClearRenderPass = VK_NULL_HANDLE;
  VkRenderPass _aoAccumulateRenderPass = VK_NULL_HANDLE;
  VkDescriptorSetLayout _generationSetLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _shadowSamplerSetLayout = VK_NULL_HANDLE;
  VkPipelineLayout _shadowPipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _accumulatePipelineLayout = VK_NULL_HANDLE;
  VkPipeline _shadowPipeline = VK_NULL_HANDLE;
  VkPipeline _accumulatePipeline = VK_NULL_HANDLE;

  VulkanTexture _shadowMap;
  VkFramebuffer _shadowFramebuffer = VK_NULL_HANDLE;
  VkDescriptorSet _generationSet = VK_NULL_HANDLE;
  VkDescriptorSet _shadowSamplerSet = VK_NULL_HANDLE;
  VulkanBuffer _aoStructureUniformBuffer;
  VulkanBuffer _shadowUniformBuffer;
  VulkanBuffer _quadVertexBuffer;
  VulkanBuffer _quadIndexBuffer;
  uint32_t _quadIndexCount = 0;
  VkDeviceSize _aoStructureStride = 256;
  VkDeviceSize _shadowStride = 256;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureResources>> _structureResources;
  std::unordered_map<RKRenderObject *, std::shared_ptr<std::vector<uint16_t>>> _cache;
};
