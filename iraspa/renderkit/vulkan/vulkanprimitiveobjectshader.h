#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "rkrenderkitprotocols.h"
#include "vulkanrenderer.h"

class VulkanPrimitiveObjectShader
{
public:
  explicit VulkanPrimitiveObjectShader(VulkanRenderer *renderer);
  ~VulkanPrimitiveObjectShader();

  void initialize();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData();
  void paintOpaque(VkCommandBuffer commandBuffer);
  void paintTransparent(VkCommandBuffer commandBuffer);
  void paintOpaque(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex);
  void paintTransparent(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex);
  void paintPick(VkCommandBuffer commandBuffer, VkPipeline stripPipeline, VkPipeline trianglePipeline);

private:
  struct Mesh
  {
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
    uint32_t indexCount = 0;
  };

  struct Instances
  {
    VulkanBuffer buffer;
    uint32_t count = 0;
  };

  struct StructurePrimitives
  {
    Instances ellipsoid;
    Instances crystalEllipsoid;
    Mesh cylinderMesh;
    Instances cylinder;
    Instances crystalCylinder;
    Mesh prismMesh;
    Instances prism;
    Instances crystalPrism;
  };

  struct PipelineSet
  {
    VkPipeline opaque = VK_NULL_HANDLE;
    VkPipeline transparentFront = VK_NULL_HANDLE;
    VkPipeline transparentBack = VK_NULL_HANDLE;
  };

  void destroyStructureBuffers();
  void uploadMesh(Mesh &mesh, const std::vector<RKVertex> &vertices, const std::vector<short> &indices);
  void uploadInstances(Instances &instances, const std::vector<RKInPerInstanceAttributesAtoms> &data);
  void paint(VkCommandBuffer commandBuffer, bool opaque);
  void paint(VkCommandBuffer commandBuffer, bool opaque, int sceneIndex, int movieIndex);
  void paintInstances(VkCommandBuffer commandBuffer, const PipelineSet &pipelines, const Mesh &mesh, const Instances &instances,
                      uint32_t structureIndex, bool opaque, RKRenderPrimitiveObjectsSource *source);
  void paintPickInstances(VkCommandBuffer commandBuffer, VkPipeline pipeline, const Mesh &mesh, const Instances &instances,
                          uint32_t structureIndex, RKRenderPrimitiveObjectsSource *source);

  PipelineSet createPipelines(VkPrimitiveTopology topology, VkCullModeFlags opaqueCull);

  VulkanRenderer *_renderer = nullptr;
  PipelineSet _stripPipelines;
  PipelineSet _trianglePipelines;
  Mesh _sphereMesh;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<StructurePrimitives>> _structureBuffers;
};
