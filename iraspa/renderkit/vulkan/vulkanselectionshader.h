#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

#include <QString>

class VulkanRibbonShader;

class VulkanSelectionShader
{
public:
  VulkanSelectionShader(VulkanRenderer *renderer, VulkanRibbonShader *ribbonShader);
  ~VulkanSelectionShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void paint(VkCommandBuffer commandBuffer, RKRenderQuality quality, bool orthographic);

private:
  struct Mesh
  {
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
    uint32_t indexCount = 0;
  };

  struct InstanceDraw
  {
    VulkanBuffer buffer;
    uint32_t count = 0;
  };

  struct StructureSelection
  {
    InstanceDraw atoms;
    InstanceDraw internalBonds;
    InstanceDraw externalBonds;
    InstanceDraw ellipsoid;
    InstanceDraw crystalEllipsoid;
    InstanceDraw cylinder;
    InstanceDraw crystalCylinder;
    InstanceDraw prism;
    InstanceDraw crystalPrism;
    Mesh cylinderMesh;
    Mesh prismMesh;
  };

  struct StylePipelines
  {
    VkPipeline glow = VK_NULL_HANDLE;
    VkPipeline stripes = VK_NULL_HANDLE;
    VkPipeline worley = VK_NULL_HANDLE;
  };

  void destroyStructureBuffers();
  void destroyPipelines();
  void uploadMesh(Mesh &mesh, const std::vector<RKVertex> &vertices, const std::vector<short> &indices);
  void uploadInstances(InstanceDraw &draw, const void *data, VkDeviceSize bytes, uint32_t count);
  VkPipeline createAtomImposterPipeline(const QString &vertexResource, const QString &fragmentResource, bool writeGlow = false);
  VkPipeline createBondImposterPipeline(const QString &fragmentResource, bool writeGlow = false);
  VkPipeline createPrimitivePipeline(const QString &fragmentResource, VkPrimitiveTopology topology, VkCullModeFlags cullMode,
                                     bool writeGlow = false);
  VkPipeline createRibbonPipeline(const QString &vertexResource, const QString &fragmentResource, bool writeGlow = false);
  VkPipeline pipelineForStyle(const StylePipelines &pipelines, RKSelectionStyle style) const;
  void paintPrimitiveInstances(VkCommandBuffer commandBuffer, VkPipeline pipeline, const Mesh &mesh, const InstanceDraw &instances,
                               uint32_t structureIndex);

  VulkanRenderer *_renderer = nullptr;
  VulkanRibbonShader *_ribbonShader = nullptr;
  StylePipelines _atomOrthoImposterPipelines;
  StylePipelines _atomPerspImposterPipelines;
  StylePipelines _bondPipelines;
  StylePipelines _primitiveStripPipelines;
  StylePipelines _primitiveTrianglePipelines;
  StylePipelines _ribbonPipelines;

  Mesh _sphereMesh;
  Mesh _quadMesh;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructureSelection>> _structureBuffers;
};
