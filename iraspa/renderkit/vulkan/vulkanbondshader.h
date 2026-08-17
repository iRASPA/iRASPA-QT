#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "rkrenderuniforms.h"
#include "vulkanrenderer.h"

class VulkanBondShader
{
public:
  explicit VulkanBondShader(VulkanRenderer *renderer);
  ~VulkanBondShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void paint(VkCommandBuffer commandBuffer, RKRenderQuality quality = RKRenderQuality::high);
  void paintPick(VkCommandBuffer commandBuffer, VkPipeline internalPipeline, VkPipeline externalPipeline);

private:
  struct InstanceDraw
  {
    VulkanBuffer buffer;
    uint32_t count = 0;
  };

  struct StructureBonds
  {
    InstanceDraw all;
    InstanceDraw single;
    InstanceDraw doubleBond;
    InstanceDraw partialDouble;
    InstanceDraw triple;
  };

  void destroyStructureBuffers();
  void uploadInstances(InstanceDraw &draw, const std::vector<RKInPerInstanceAttributesBonds> &data);
  void fillBondInstances(StructureBonds &bonds, const std::vector<RKInPerInstanceAttributesBonds> &data);
  void paintBondGroupImposters(VkCommandBuffer commandBuffer, VkPipeline pipeline, const StructureBonds &bonds, bool unity,
                               uint32_t structureIndex);

  VulkanRenderer *_renderer = nullptr;
  VkPipeline _internalImposterPipeline = VK_NULL_HANDLE;
  VkPipeline _externalImposterPipeline = VK_NULL_HANDLE;
  VkPipeline _internalImposterPerPixelPipeline = VK_NULL_HANDLE;
  VkPipeline _externalImposterPerPixelPipeline = VK_NULL_HANDLE;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureBonds>> _internalBonds;
  std::vector<std::vector<StructureBonds>> _externalBonds;
};
