#pragma once

#include <QString>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanRenderer;

class VulkanShader
{
public:
  struct PipelineConfig
  {
    QString vertexShaderResource;
    QString fragmentShaderResource;
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    bool depthTest = true;
    bool depthWrite = true;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    bool depthClamp = false;
    bool stencilTest = false;
    VkStencilOpState stencil{};  // applied to both front- and back-facing primitives
    bool blend = false;
    VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    uint32_t colorAttachmentCount = 1;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
    // Shade the fragment shader at every MSAA sample (ray-traced imposter silhouettes).
    // Requires sampleRateShading on the device; ignored otherwise.
    bool sampleShading = false;
    bool writeGlow = false;
    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                           VK_COLOR_COMPONENT_A_BIT;
    VkRenderPass renderPass = VK_NULL_HANDLE;
  };

  static VkPipeline createGraphicsPipeline(VulkanRenderer *renderer, VkPipelineLayout layout, const PipelineConfig &config);
  static VkShaderModule loadShaderModule(VkDevice device, const QString &resourcePath);
};
