#pragma once

#include <vulkan/vulkan.h>

#include "rkrenderuniforms.h"

class VulkanRenderer;
class VulkanAtomSphereShader;
class VulkanBondShader;
class VulkanPrimitiveObjectShader;
class VulkanRibbonShader;

class VulkanPickingShader
{
public:
  VulkanPickingShader(VulkanRenderer *renderer, VulkanAtomSphereShader *atomShader, VulkanBondShader *bondShader,
                      VulkanPrimitiveObjectShader *primitiveShader, VulkanRibbonShader *ribbonShader);
  ~VulkanPickingShader();

  void initialize();
  void paint(VkCommandBuffer commandBuffer, RKRenderQuality quality, bool orthographic);

private:
  void destroyPipelines();

  VulkanRenderer *_renderer = nullptr;
  VulkanAtomSphereShader *_atomShader = nullptr;
  VulkanBondShader *_bondShader = nullptr;
  VulkanPrimitiveObjectShader *_primitiveShader = nullptr;
  VulkanRibbonShader *_ribbonShader = nullptr;

  VkPipeline _atomOrthoImposterPipeline = VK_NULL_HANDLE;
  VkPipeline _atomPerspImposterPipeline = VK_NULL_HANDLE;
  VkPipeline _bondPipeline = VK_NULL_HANDLE;
  VkPipeline _bondExternalPipeline = VK_NULL_HANDLE;
  VkPipeline _primitiveStripPipeline = VK_NULL_HANDLE;
  VkPipeline _primitiveTrianglePipeline = VK_NULL_HANDLE;
  VkPipeline _ribbonPipeline = VK_NULL_HANDLE;
};
