#pragma once

#include <map>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include <QString>

#include "rkfontatlas.h"
#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanAtomTextShader
{
public:
  explicit VulkanAtomTextShader(VulkanRenderer *renderer);
  ~VulkanAtomTextShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void paint(VkCommandBuffer commandBuffer);

private:
  struct StructureBuffers
  {
    VulkanBuffer instanceBuffer;
    uint32_t instanceCount = 0;
    QString fontName;
  };

  struct FontGpu
  {
    std::unique_ptr<RKFontAtlas> atlas;
    VulkanTexture texture;
    VkDescriptorSet samplerSet = VK_NULL_HANDLE;
  };

  void destroyStructureBuffers();
  void destroyFonts();
  FontGpu &fontForName(const QString &fontName);

  VulkanRenderer *_renderer = nullptr;
  VkPipeline _pipeline = VK_NULL_HANDLE;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureBuffers>> _structureBuffers;
  std::map<QString, FontGpu> _fonts;
};
