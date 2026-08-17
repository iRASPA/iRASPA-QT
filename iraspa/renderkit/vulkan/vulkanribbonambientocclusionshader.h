#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "rkrenderuniforms.h"
#include "vulkanrenderer.h"

class VulkanAtomSphereShader;
class VulkanRibbonShader;

class VulkanRibbonAmbientOcclusionShader
{
public:
  VulkanRibbonAmbientOcclusionShader(VulkanRenderer *renderer, VulkanRibbonShader *ribbonShader,
                                     VulkanAtomSphereShader *atomShader);
  ~VulkanRibbonAmbientOcclusionShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality);
  void invalidateCachedAmbientOcclusionTexture(const std::vector<std::shared_ptr<RKRenderObject>> &structures);
  VkDescriptorSet samplerSet(size_t sceneIndex, size_t movieIndex) const;
  bool hasCachedTexture(RKRenderObject *structure, uint32_t width, uint32_t height) const;

private:
  struct StructureResources
  {
    VulkanTexture texture;
    VkDescriptorSet samplerSet = VK_NULL_HANDLE;
  };

  void destroyStructureResources();
  void destroyPipelines();
  void createRenderPasses();
  void createShadowResources();
  void destroyShadowResources();
  void createPipelines();
  void createGenerationDescriptors();
  void adjustTextureSizes();
  void generateTextures(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality);
  void createBlurResources();
  void blurAtlas(VulkanTexture &atlas, uint32_t width, uint32_t height);
  void recordImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout,
                          VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
                          VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);
  void uploadPackedUniforms(VulkanBuffer &buffer, const void *items, size_t count, size_t itemSize, VkDeviceSize stride);
  VkDescriptorSet allocateGenerationSet(const VulkanBuffer &structureBuffer);

  VulkanRenderer *_renderer = nullptr;
  VulkanRibbonShader *_ribbonShader = nullptr;
  VulkanAtomSphereShader *_atomShader = nullptr;

  VkRenderPass _shadowRenderPass = VK_NULL_HANDLE;
  VkRenderPass _aoClearRenderPass = VK_NULL_HANDLE;
  VkRenderPass _aoAccumulateRenderPass = VK_NULL_HANDLE;
  VkDescriptorSetLayout _generationSetLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _shadowSamplerSetLayout = VK_NULL_HANDLE;
  VkPipelineLayout _shadowPipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _accumulatePipelineLayout = VK_NULL_HANDLE;
  VkPipeline _atomShadowPipeline = VK_NULL_HANDLE;
  VkPipeline _ribbonShadowPipeline = VK_NULL_HANDLE;
  VkPipeline _accumulatePipeline = VK_NULL_HANDLE;
  VkPipeline _blurPipeline = VK_NULL_HANDLE;
  VkPipelineLayout _blurPipelineLayout = VK_NULL_HANDLE;
  VkRenderPass _blurRenderPass = VK_NULL_HANDLE;
  VkDescriptorSetLayout _blurSamplerSetLayout = VK_NULL_HANDLE;
  VkDescriptorSet _blurSamplerSet = VK_NULL_HANDLE;

  VulkanTexture _shadowMap;
  VkFramebuffer _shadowFramebuffer = VK_NULL_HANDLE;
  VkDescriptorSet _generationSetAO = VK_NULL_HANDLE;
  VkDescriptorSet _generationSetRibbon = VK_NULL_HANDLE;
  VkDescriptorSet _shadowSamplerSet = VK_NULL_HANDLE;
  VulkanBuffer _aoStructureUniformBuffer;
  VulkanBuffer _ribbonStructureUniformBuffer;
  VulkanBuffer _shadowUniformBuffer;
  VulkanBuffer _quadVertexBuffer;
  VulkanBuffer _quadIndexBuffer;
  uint32_t _quadIndexCount = 0;
  VkDeviceSize _structureStride = 256;
  VkDeviceSize _shadowStride = 256;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureResources>> _structureResources;
  std::unordered_map<std::string, std::shared_ptr<std::vector<uint16_t>>> _cache;
};
