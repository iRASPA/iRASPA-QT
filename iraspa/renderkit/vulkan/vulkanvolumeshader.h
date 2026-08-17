#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanVolumeShader
{
public:
  explicit VulkanVolumeShader(VulkanRenderer *renderer);
  ~VulkanVolumeShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void invalidateIsosurface(const std::vector<std::shared_ptr<RKRenderObject>> &structures);
  bool hasVisibleVolumes() const;
  bool hasOpaqueVolumes() const;
  bool hasTransparentVolumes() const;
  void paintOpaque(VkCommandBuffer commandBuffer);
  void paintTransparent(VkCommandBuffer commandBuffer);
  void paintOpaque(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex);
  void paintTransparent(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex);

private:
  void destroyStructureResources();
  void paint(VkCommandBuffer commandBuffer, bool opaque);
  void paint(VkCommandBuffer commandBuffer, bool opaque, int sceneIndex, int movieIndex);
  bool structureIsVolume(size_t sceneIndex, size_t movieIndex, bool opaque) const;

  struct StructureResources
  {
    VulkanTexture volume;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
  };

  VulkanRenderer *_renderer = nullptr;
  VkPipeline _opaquePipeline = VK_NULL_HANDLE;
  VkPipeline _transparentPipeline = VK_NULL_HANDLE;
  VulkanBuffer _vertexBuffer;
  VulkanBuffer _indexBuffer;
  uint32_t _indexCount = 0;
  VulkanTexture _transferFunction;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureResources>> _structureResources;
  std::unordered_map<RKRenderObject *, std::shared_ptr<std::vector<float4>>> _volumeCache;
};
