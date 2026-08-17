#include "vulkanshader.h"

#include "vulkanrenderer.h"

#include <array>
#include <stdexcept>

#include <QFile>

VkShaderModule VulkanShader::loadShaderModule(VkDevice device, const QString &resourcePath)
{
  QFile file(resourcePath);
  if (!file.open(QIODevice::ReadOnly))
  {
    throw std::runtime_error(("failed to open shader " + resourcePath).toStdString());
  }
  const QByteArray code = file.readAll();
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = static_cast<size_t>(code.size());
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.constData());

  VkShaderModule shaderModule = VK_NULL_HANDLE;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create shader module");
  }
  return shaderModule;
}

VkPipeline VulkanShader::createGraphicsPipeline(VulkanRenderer *renderer, VkPipelineLayout layout, const PipelineConfig &config)
{
  VkDevice device = renderer->device();
  VkShaderModule vertShaderModule = loadShaderModule(device, config.vertexShaderResource);
  VkShaderModule fragShaderModule = loadShaderModule(device, config.fragmentShaderResource);

  VkPipelineShaderStageCreateInfo vertStage{};
  vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertStage.module = vertShaderModule;
  vertStage.pName = "main";

  VkPipelineShaderStageCreateInfo fragStage{};
  fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragStage.module = fragShaderModule;
  fragStage.pName = "main";

  VkPipelineShaderStageCreateInfo stages[] = {vertStage, fragStage};

  VkPipelineVertexInputStateCreateInfo vertexInput{};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(config.bindings.size());
  vertexInput.pVertexBindingDescriptions = config.bindings.data();
  vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(config.attributes.size());
  vertexInput.pVertexAttributeDescriptions = config.attributes.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = config.topology;
  inputAssembly.primitiveRestartEnable =
      (config.topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP || config.topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP) ? VK_TRUE : VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = config.cullMode;
  rasterizer.frontFace = config.frontFace;
  rasterizer.depthClampEnable = config.depthClamp ? VK_TRUE : VK_FALSE;

  const bool scenePass = !config.renderPass || config.renderPass == renderer->renderPass();
  VkSampleCountFlagBits sampleCount = config.samples;
  if (sampleCount == VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM)
  {
    sampleCount = scenePass ? renderer->msaaSampleCount() : VK_SAMPLE_COUNT_1_BIT;
  }

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.rasterizationSamples = sampleCount;

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = config.depthTest ? VK_TRUE : VK_FALSE;
  depthStencil.depthWriteEnable = config.depthWrite ? VK_TRUE : VK_FALSE;
  depthStencil.depthCompareOp = config.depthCompareOp;
  depthStencil.stencilTestEnable = config.stencilTest ? VK_TRUE : VK_FALSE;
  depthStencil.front = config.stencil;
  depthStencil.back = config.stencil;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = config.colorWriteMask;
  colorBlendAttachment.blendEnable = config.blend ? VK_TRUE : VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = config.srcColorBlendFactor;
  colorBlendAttachment.dstColorBlendFactor = config.dstColorBlendFactor;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = config.srcAlphaBlendFactor;
  colorBlendAttachment.dstAlphaBlendFactor = config.dstAlphaBlendFactor;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendAttachmentState disabledAttachment{};
  disabledAttachment.colorWriteMask = 0;
  disabledAttachment.blendEnable = VK_FALSE;

  std::array<VkPipelineColorBlendAttachmentState, 2> sceneBlendAttachments{};
  uint32_t attachmentCount = config.colorAttachmentCount;
  const VkPipelineColorBlendAttachmentState *blendAttachments = config.colorAttachmentCount > 0 ? &colorBlendAttachment : nullptr;
  if (scenePass && config.colorAttachmentCount == 1)
  {
    attachmentCount = 2;
    if (config.writeGlow)
    {
      sceneBlendAttachments[0] = disabledAttachment;
      sceneBlendAttachments[1] = colorBlendAttachment;
    }
    else
    {
      sceneBlendAttachments[0] = colorBlendAttachment;
      sceneBlendAttachments[1] = disabledAttachment;
    }
    blendAttachments = sceneBlendAttachments.data();
  }

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.attachmentCount = attachmentCount;
  colorBlending.pAttachments = blendAttachments;

  const VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = stages;
  pipelineInfo.pVertexInputState = &vertexInput;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = layout;
  pipelineInfo.renderPass = config.renderPass ? config.renderPass : renderer->renderPass();
  pipelineInfo.subpass = 0;

  VkPipeline pipeline = VK_NULL_HANDLE;
  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
  {
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    throw std::runtime_error("failed to create graphics pipeline");
  }

  vkDestroyShaderModule(device, fragShaderModule, nullptr);
  vkDestroyShaderModule(device, vertShaderModule, nullptr);
  return pipeline;
}
