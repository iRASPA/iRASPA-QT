#include "vulkanribbonambientocclusionshader.h"

#include "quadgeometry.h"
#include "ribbonaolayout.h"
#include "ribbonaotexturepostprocess.h"
#include "rkcamera.h"
#include "rkribbonmesh.h"
#include "vulkanatomsphereshader.h"
#include "vulkanribbonshader.h"
#include "vulkanshader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <tuple>

#include <QString>

namespace
{
constexpr uint32_t kShadowMapSize = 2048;

VkVertexInputAttributeDescription float4Attribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}

VkVertexInputAttributeDescription float2Attribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32_SFLOAT;
  description.offset = offset;
  return description;
}

void setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height)
{
  VkViewport viewport{};
  viewport.width = static_cast<float>(width);
  viewport.height = static_cast<float>(height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  VkRect2D scissor{};
  scissor.extent = {width, height};
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

bool ribbonAmbientOcclusionTextureHasContent(const std::vector<uint16_t> &data)
{
  float maxValue = 0.0f;
  for (uint16_t half : data)
  {
    maxValue = std::max(maxValue, RKHalfFloat::floatFromHalfBits(half));
  }
  return maxValue > 1.0e-5f;
}

std::vector<uint16_t> whiteAtlas(uint32_t width, uint32_t height)
{
  return std::vector<uint16_t>(static_cast<size_t>(width) * height, RKHalfFloat::halfBitsFromFloat(1.0f));
}
}  // namespace

VulkanRibbonAmbientOcclusionShader::VulkanRibbonAmbientOcclusionShader(VulkanRenderer *renderer, VulkanRibbonShader *ribbonShader,
                                                                       VulkanAtomSphereShader *atomShader)
    : _renderer(renderer), _ribbonShader(ribbonShader), _atomShader(atomShader)
{
}

VulkanRibbonAmbientOcclusionShader::~VulkanRibbonAmbientOcclusionShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyStructureResources();
  destroyPipelines();
  destroyShadowResources();
  _renderer->destroyBuffer(_aoStructureUniformBuffer);
  _renderer->destroyBuffer(_ribbonStructureUniformBuffer);
  _renderer->destroyBuffer(_shadowUniformBuffer);
  _renderer->destroyBuffer(_quadVertexBuffer);
  _renderer->destroyBuffer(_quadIndexBuffer);
  auto freeSet = [&](VkDescriptorSet &set) {
    if (set)
    {
      vkFreeDescriptorSets(_renderer->device(), _renderer->descriptorPool(), 1, &set);
      set = VK_NULL_HANDLE;
    }
  };
  freeSet(_generationSetAO);
  freeSet(_generationSetRibbon);
  freeSet(_shadowSamplerSet);
  if (_shadowPipelineLayout)
  {
    vkDestroyPipelineLayout(_renderer->device(), _shadowPipelineLayout, nullptr);
  }
  if (_accumulatePipelineLayout)
  {
    vkDestroyPipelineLayout(_renderer->device(), _accumulatePipelineLayout, nullptr);
  }
  if (_generationSetLayout)
  {
    vkDestroyDescriptorSetLayout(_renderer->device(), _generationSetLayout, nullptr);
  }
  if (_shadowSamplerSetLayout)
  {
    vkDestroyDescriptorSetLayout(_renderer->device(), _shadowSamplerSetLayout, nullptr);
  }
  if (_shadowRenderPass)
  {
    vkDestroyRenderPass(_renderer->device(), _shadowRenderPass, nullptr);
  }
  if (_aoClearRenderPass)
  {
    vkDestroyRenderPass(_renderer->device(), _aoClearRenderPass, nullptr);
  }
  if (_aoAccumulateRenderPass)
  {
    vkDestroyRenderPass(_renderer->device(), _aoAccumulateRenderPass, nullptr);
  }
}

void VulkanRibbonAmbientOcclusionShader::destroyStructureResources()
{
  for (auto &scene : _structureResources)
  {
    for (auto &resources : scene)
    {
      if (resources.samplerSet)
      {
        vkFreeDescriptorSets(_renderer->device(), _renderer->descriptorPool(), 1, &resources.samplerSet);
      }
      _renderer->destroyTexture(resources.texture);
    }
  }
  _structureResources.clear();
}

void VulkanRibbonAmbientOcclusionShader::destroyPipelines()
{
  auto destroyPipeline = [&](VkPipeline &pipeline) {
    if (pipeline)
    {
      vkDestroyPipeline(_renderer->device(), pipeline, nullptr);
      pipeline = VK_NULL_HANDLE;
    }
  };
  destroyPipeline(_atomShadowPipeline);
  destroyPipeline(_ribbonShadowPipeline);
  destroyPipeline(_accumulatePipeline);
}

void VulkanRibbonAmbientOcclusionShader::destroyShadowResources()
{
  if (_shadowFramebuffer)
  {
    vkDestroyFramebuffer(_renderer->device(), _shadowFramebuffer, nullptr);
    _shadowFramebuffer = VK_NULL_HANDLE;
  }
  _renderer->destroyTexture(_shadowMap);
}

void VulkanRibbonAmbientOcclusionShader::initialize()
{
  QuadGeometry quad;
  const auto vertices = quad.vertices();
  const auto indices = quad.indices();
  _quadIndexCount = static_cast<uint32_t>(indices.size());
  _renderer->uploadBuffer(_quadVertexBuffer, vertices.data(), vertices.size() * sizeof(RKVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  _renderer->uploadBuffer(_quadIndexBuffer, indices.data(), indices.size() * sizeof(short), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  _structureStride = _renderer->structureUniformStride();
  _shadowStride = VulkanRenderer::alignedUniformSize(sizeof(RKShadowUniforms), _renderer->minUniformBufferOffsetAlignment());

  createRenderPasses();
  createShadowResources();
  createGenerationDescriptors();
  createPipelines();
}

void VulkanRibbonAmbientOcclusionShader::createRenderPasses()
{
  auto makeDepthPass = [&]() {
    VkAttachmentDescription depth{};
    depth.format = _renderer->depthFormat();
    depth.samples = VK_SAMPLE_COUNT_1_BIT;
    depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthRef{};
    depthRef.attachment = 0;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &depthRef;
    VkRenderPassCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &depth;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    if (vkCreateRenderPass(_renderer->device(), &info, nullptr, &renderPass) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create ribbon AO shadow render pass");
    }
    return renderPass;
  };

  auto makeColorPass = [&](VkAttachmentLoadOp loadOp, VkImageLayout initialLayout) {
    VkAttachmentDescription color{};
    color.format = VK_FORMAT_R16_SFLOAT;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = loadOp;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = initialLayout;
    color.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    VkRenderPassCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &color;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    if (vkCreateRenderPass(_renderer->device(), &info, nullptr, &renderPass) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create ribbon AO color render pass");
    }
    return renderPass;
  };

  _shadowRenderPass = makeDepthPass();
  _aoClearRenderPass = makeColorPass(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED);
  _aoAccumulateRenderPass = makeColorPass(VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void VulkanRibbonAmbientOcclusionShader::createShadowResources()
{
  _shadowMap = _renderer->createAttachmentTexture(kShadowMapSize, kShadowMapSize, _renderer->depthFormat(),
                                                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                  VK_IMAGE_ASPECT_DEPTH_BIT);
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = _shadowRenderPass;
  framebufferInfo.attachmentCount = 1;
  framebufferInfo.pAttachments = &_shadowMap.view;
  framebufferInfo.width = kShadowMapSize;
  framebufferInfo.height = kShadowMapSize;
  framebufferInfo.layers = 1;
  if (vkCreateFramebuffer(_renderer->device(), &framebufferInfo, nullptr, &_shadowFramebuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create ribbon AO shadow framebuffer");
  }
}

void VulkanRibbonAmbientOcclusionShader::createGenerationDescriptors()
{
  VkDescriptorSetLayoutBinding structureBinding{};
  structureBinding.binding = 0;
  structureBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  structureBinding.descriptorCount = 1;
  structureBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  VkDescriptorSetLayoutBinding shadowBinding = structureBinding;
  shadowBinding.binding = 1;
  std::array<VkDescriptorSetLayoutBinding, 2> generationBindings = {structureBinding, shadowBinding};
  VkDescriptorSetLayoutCreateInfo generationLayoutInfo{};
  generationLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  generationLayoutInfo.bindingCount = 2;
  generationLayoutInfo.pBindings = generationBindings.data();
  if (vkCreateDescriptorSetLayout(_renderer->device(), &generationLayoutInfo, nullptr, &_generationSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create ribbon AO generation descriptor layout");
  }

  VkDescriptorSetLayoutBinding samplerBinding{};
  samplerBinding.binding = 0;
  samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerBinding.descriptorCount = 1;
  samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  VkDescriptorSetLayoutCreateInfo samplerLayoutInfo{};
  samplerLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  samplerLayoutInfo.bindingCount = 1;
  samplerLayoutInfo.pBindings = &samplerBinding;
  if (vkCreateDescriptorSetLayout(_renderer->device(), &samplerLayoutInfo, nullptr, &_shadowSamplerSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create ribbon AO shadow sampler layout");
  }

  VkDescriptorSetLayout shadowLayouts[] = {_generationSetLayout};
  VkPipelineLayoutCreateInfo shadowLayoutInfo{};
  shadowLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  shadowLayoutInfo.setLayoutCount = 1;
  shadowLayoutInfo.pSetLayouts = shadowLayouts;
  if (vkCreatePipelineLayout(_renderer->device(), &shadowLayoutInfo, nullptr, &_shadowPipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create ribbon AO shadow pipeline layout");
  }

  VkPushConstantRange pushRange{};
  pushRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  pushRange.size = sizeof(float);
  VkDescriptorSetLayout accumulateLayouts[] = {_generationSetLayout, _shadowSamplerSetLayout};
  VkPipelineLayoutCreateInfo accumulateLayoutInfo{};
  accumulateLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  accumulateLayoutInfo.setLayoutCount = 2;
  accumulateLayoutInfo.pSetLayouts = accumulateLayouts;
  accumulateLayoutInfo.pushConstantRangeCount = 1;
  accumulateLayoutInfo.pPushConstantRanges = &pushRange;
  if (vkCreatePipelineLayout(_renderer->device(), &accumulateLayoutInfo, nullptr, &_accumulatePipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create ribbon AO accumulate pipeline layout");
  }

  _renderer->createBuffer(_structureStride, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _aoStructureUniformBuffer);
  _renderer->createBuffer(_structureStride, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _ribbonStructureUniformBuffer);
  _renderer->createBuffer(_shadowStride, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _shadowUniformBuffer);

  _generationSetAO = allocateGenerationSet(_aoStructureUniformBuffer);
  _generationSetRibbon = allocateGenerationSet(_ribbonStructureUniformBuffer);

  VkDescriptorSetAllocateInfo samplerAlloc{};
  samplerAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  samplerAlloc.descriptorPool = _renderer->descriptorPool();
  samplerAlloc.descriptorSetCount = 1;
  samplerAlloc.pSetLayouts = &_shadowSamplerSetLayout;
  if (vkAllocateDescriptorSets(_renderer->device(), &samplerAlloc, &_shadowSamplerSet) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate ribbon AO shadow sampler set");
  }

  VkDescriptorImageInfo shadowImage{};
  shadowImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  shadowImage.imageView = _shadowMap.view;
  shadowImage.sampler = _renderer->linearSampler();
  VkWriteDescriptorSet shadowWrite{};
  shadowWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  shadowWrite.dstSet = _shadowSamplerSet;
  shadowWrite.dstBinding = 0;
  shadowWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  shadowWrite.descriptorCount = 1;
  shadowWrite.pImageInfo = &shadowImage;
  vkUpdateDescriptorSets(_renderer->device(), 1, &shadowWrite, 0, nullptr);
}

VkDescriptorSet VulkanRibbonAmbientOcclusionShader::allocateGenerationSet(const VulkanBuffer &structureBuffer)
{
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
  VkDescriptorSetAllocateInfo alloc{};
  alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc.descriptorPool = _renderer->descriptorPool();
  alloc.descriptorSetCount = 1;
  alloc.pSetLayouts = &_generationSetLayout;
  if (vkAllocateDescriptorSets(_renderer->device(), &alloc, &descriptorSet) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate ribbon AO generation descriptor set");
  }

  VkDescriptorBufferInfo structureInfo{structureBuffer.buffer, 0, sizeof(RKStructureUniforms)};
  VkDescriptorBufferInfo shadowInfo{_shadowUniformBuffer.buffer, 0, sizeof(RKShadowUniforms)};
  VkWriteDescriptorSet writes[2]{};
  writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[0].dstSet = descriptorSet;
  writes[0].dstBinding = 0;
  writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  writes[0].descriptorCount = 1;
  writes[0].pBufferInfo = &structureInfo;
  writes[1] = writes[0];
  writes[1].dstBinding = 1;
  writes[1].pBufferInfo = &shadowInfo;
  vkUpdateDescriptorSets(_renderer->device(), 2, writes, 0, nullptr);
  return descriptorSet;
}

void VulkanRibbonAmbientOcclusionShader::createPipelines()
{
  VkVertexInputBindingDescription vertexBinding{};
  vertexBinding.binding = 0;
  vertexBinding.stride = sizeof(RKVertex);
  vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VulkanShader::PipelineConfig atomShadow;
  atomShadow.vertexShaderResource = QStringLiteral(":/shaders/ao_shadow.vert.spv");
  atomShadow.fragmentShaderResource = QStringLiteral(":/shaders/ao_shadow.frag.spv");
  atomShadow.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  atomShadow.cullMode = VK_CULL_MODE_NONE;
  atomShadow.depthTest = true;
  atomShadow.depthWrite = true;
  atomShadow.depthClamp = true;
  atomShadow.colorAttachmentCount = 0;
  atomShadow.renderPass = _shadowRenderPass;
  VkVertexInputBindingDescription instanceBinding{};
  instanceBinding.binding = 1;
  instanceBinding.stride = sizeof(RKInPerInstanceAttributesAtoms);
  instanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  atomShadow.bindings = {vertexBinding, instanceBinding};
  atomShadow.attributes = {
      float4Attribute(0, 0, offsetof(RKVertex, position)),
      float4Attribute(1, 1, offsetof(RKInPerInstanceAttributesAtoms, position)),
      float4Attribute(2, 1, offsetof(RKInPerInstanceAttributesAtoms, scale)),
  };
  _atomShadowPipeline = VulkanShader::createGraphicsPipeline(_renderer, _shadowPipelineLayout, atomShadow);

  VulkanShader::PipelineConfig ribbonShadow = atomShadow;
  ribbonShadow.vertexShaderResource = QStringLiteral(":/shaders/ribbon_ao_shadow.vert.spv");
  ribbonShadow.fragmentShaderResource = QStringLiteral(":/shaders/ribbon_ao_shadow.frag.spv");
  ribbonShadow.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  ribbonShadow.bindings = {vertexBinding};
  ribbonShadow.attributes = {float4Attribute(0, 0, offsetof(RKVertex, position))};
  _ribbonShadowPipeline = VulkanShader::createGraphicsPipeline(_renderer, _shadowPipelineLayout, ribbonShadow);

  VulkanShader::PipelineConfig accumulate;
  accumulate.vertexShaderResource = QStringLiteral(":/shaders/ribbon_ao_accumulate.vert.spv");
  accumulate.fragmentShaderResource = QStringLiteral(":/shaders/ribbon_ao_accumulate.frag.spv");
  accumulate.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  accumulate.cullMode = VK_CULL_MODE_NONE;
  accumulate.depthTest = false;
  accumulate.depthWrite = false;
  accumulate.colorAttachmentCount = 1;
  accumulate.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
  accumulate.blend = true;
  accumulate.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  accumulate.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
  accumulate.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  accumulate.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  accumulate.renderPass = _aoAccumulateRenderPass;
  accumulate.bindings = {vertexBinding};
  accumulate.attributes = {
      float4Attribute(0, 0, offsetof(RKVertex, position)),
      float4Attribute(1, 0, offsetof(RKVertex, normal)),
      float2Attribute(2, 0, offsetof(RKVertex, st)),
  };
  _accumulatePipeline = VulkanShader::createGraphicsPipeline(_renderer, _accumulatePipelineLayout, accumulate);
}

void VulkanRibbonAmbientOcclusionShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
}

void VulkanRibbonAmbientOcclusionShader::invalidateCachedAmbientOcclusionTexture(
    const std::vector<std::shared_ptr<RKRenderObject>> &structures)
{
  for (const auto &structure : structures)
  {
    _cache.erase(ribbonAmbientOcclusionCacheKey(structure.get()).toStdString());
  }
}

VkDescriptorSet VulkanRibbonAmbientOcclusionShader::samplerSet(size_t sceneIndex, size_t movieIndex) const
{
  if (sceneIndex >= _structureResources.size() || movieIndex >= _structureResources[sceneIndex].size())
  {
    return VK_NULL_HANDLE;
  }
  return _structureResources[sceneIndex][movieIndex].samplerSet;
}

void VulkanRibbonAmbientOcclusionShader::reloadData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality)
{
  _renderer->waitIdle();
  destroyStructureResources();
  _structureResources.resize(_renderStructures.size());
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureResources[i].resize(_renderStructures[i].size());
  }
  adjustTextureSizes();
  generateTextures(dataSource, quality);
}

void VulkanRibbonAmbientOcclusionShader::adjustTextureSizes()
{
  const int maxSize = 16384;
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *ribbonSource = dynamic_cast<RKRenderRibbonSource *>(_renderStructures[i][j].get());
      if (!ribbonSource || !ribbonSource->drawRibbon() || ribbonSource->ribbonNumberOfChains() <= 0)
      {
        continue;
      }
      auto *atomSource = dynamic_cast<RKRenderAtomSource *>(_renderStructures[i][j].get());
      const int numberOfAtoms = atomSource ? static_cast<int>(atomSource->renderAtoms().size())
                                           : static_cast<int>(ribbonSource->ribbonResidueDrawRanges().size());
      const auto atlasDimensions = RKRibbonMesh::ambientOcclusionAtlasDimensions(
          ribbonSource->ribbonMaxSplineSampleCount(), ribbonSource->ribbonNumberOfChains(), numberOfAtoms, maxSize);
      ribbonSource->setRibbonAmbientOcclusionTextureWidth(std::get<0>(atlasDimensions));
      ribbonSource->setRibbonAmbientOcclusionTextureHeight(std::get<1>(atlasDimensions));
      ribbonSource->setRibbonAmbientOcclusionStripHeight(std::get<2>(atlasDimensions));
      ribbonSource->setRibbonAmbientOcclusionTextureSize(std::max(std::get<0>(atlasDimensions), std::get<1>(atlasDimensions)));
      ribbonSource->setRibbonAmbientOcclusionPatchNumber(1);
      ribbonSource->setRibbonAmbientOcclusionPatchSize(std::get<0>(atlasDimensions));
    }
  }
}

void VulkanRibbonAmbientOcclusionShader::recordImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspect,
                                                            VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccess,
                                                            VkAccessFlags dstAccess, VkPipelineStageFlags srcStage,
                                                            VkPipelineStageFlags dstStage)
{
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.srcAccessMask = srcAccess;
  barrier.dstAccessMask = dstAccess;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = aspect;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.layerCount = 1;
  vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanRibbonAmbientOcclusionShader::uploadPackedUniforms(VulkanBuffer &buffer, const void *items, size_t count, size_t itemSize,
                                                              VkDeviceSize stride)
{
  const VkDeviceSize bytes = std::max<VkDeviceSize>(count, 1) * stride;
  _renderer->destroyBuffer(buffer);
  _renderer->createBuffer(bytes, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer);
  std::vector<uint8_t> packed(static_cast<size_t>(bytes), 0);
  const auto *src = static_cast<const uint8_t *>(items);
  for (size_t i = 0; i < count; ++i)
  {
    std::memcpy(packed.data() + i * stride, src + i * itemSize, itemSize);
  }
  void *mapped = nullptr;
  vkMapMemory(_renderer->device(), buffer.memory, 0, bytes, 0, &mapped);
  std::memcpy(mapped, packed.data(), packed.size());
  vkUnmapMemory(_renderer->device(), buffer.memory);
}

void VulkanRibbonAmbientOcclusionShader::generateTextures(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality)
{
  if (!dataSource || !_ribbonShadowPipeline || !_accumulatePipeline || !_ribbonShader)
  {
    return;
  }

  const int maxk = (quality == RKRenderQuality::picture) ? 1992 : 360;
  std::srand(0);

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *object = _renderStructures[i][j].get();
      auto *ribbonSource = dynamic_cast<RKRenderRibbonSource *>(object);
      if (!ribbonSource || !ribbonSource->drawRibbon())
      {
        continue;
      }

      const uint32_t width = static_cast<uint32_t>(std::max(ribbonSource->ribbonAmbientOcclusionTextureWidth(), 1));
      const uint32_t height = static_cast<uint32_t>(std::max(ribbonSource->ribbonAmbientOcclusionTextureHeight(), 1));
      StructureResources &resources = _structureResources[i][j];

      auto assignTexture = [&](const std::vector<uint16_t> &data) {
        resources.texture = _renderer->createTextureR16F(width, height, data.data());
        resources.samplerSet = _renderer->allocateSamplerDescriptorSet(resources.texture);
      };

      if (!ribbonSource->ribbonAmbientOcclusion() || !object->isVisible() || ribbonSource->ribbonNumberOfChains() <= 0 ||
          _ribbonShader->indexCount(i, j) == 0)
      {
        assignTexture(whiteAtlas(width, height));
        continue;
      }

      const std::string cacheKey = ribbonAmbientOcclusionCacheKey(object).toStdString();
      auto cacheIt = _cache.find(cacheKey);
      if (cacheIt != _cache.end() && cacheIt->second && cacheIt->second->size() == static_cast<size_t>(width) * height)
      {
        assignTexture(*cacheIt->second);
        continue;
      }

      if (!object->cell())
      {
        assignTexture(whiteAtlas(width, height));
        continue;
      }

      double4x4 modelMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPointWithTranslation(
          double4x4(object->orientation()), object->cell()->boundingBox().center(), object->origin());
      std::vector<RKStructureUniforms> aoStructureUniforms;
      std::vector<RKStructureUniforms> ribbonStructureUniforms;
      aoStructureUniforms.reserve(_renderStructures[i].size());
      ribbonStructureUniforms.reserve(_renderStructures[i].size());
      for (size_t k = 0; k < _renderStructures[i].size(); ++k)
      {
        aoStructureUniforms.emplace_back(i, k, _renderStructures[i][k], double4x4::inverse(modelMatrix));
        ribbonStructureUniforms.emplace_back(i, k, _renderStructures[i][k]);
      }

      auto *atomSourceForUniforms = dynamic_cast<RKRenderAtomSource *>(ribbonSource);
      const bool includeAtomShadows =
          atomSourceForUniforms && atomSourceForUniforms->atomAmbientOcclusion() && atomSourceForUniforms->drawAtoms();
      const bool ribbonUsesRenderUniformsForShadow = !(atomSourceForUniforms && atomSourceForUniforms->drawAtoms());

      SKBoundingBox boundingBox = dataSource->renderBoundingBox();
      const double largestRadius = boundingBox.boundingSphereRadius();
      const double3 centerOfScene = boundingBox.minimum() + (boundingBox.maximum() - boundingBox.minimum()) * 0.5;
      const double3 eye = double3(centerOfScene.x, centerOfScene.y, centerOfScene.z + largestRadius);
      const double boundingBoxAspectRatio =
          std::fabs(boundingBox.maximum().x - boundingBox.minimum().x) / std::fabs(boundingBox.maximum().y - boundingBox.minimum().y);
      double left = -largestRadius;
      double right = largestRadius;
      double top = largestRadius;
      double bottom = -largestRadius;
      if (boundingBoxAspectRatio < 1.0)
      {
        left = -largestRadius / boundingBoxAspectRatio;
        right = largestRadius / boundingBoxAspectRatio;
        top = largestRadius / boundingBoxAspectRatio;
        bottom = -largestRadius / boundingBoxAspectRatio;
      }

      std::vector<RKShadowUniforms> shadowUniforms;
      shadowUniforms.reserve(static_cast<size_t>(maxk));
      for (int k = 0; k < maxk; ++k)
      {
        simd_quatd smallChangeQ = simd_quatd::smallRandomQuaternion(0.5 * 10.0 * M_PI / 180.0);
        simd_quatd q = smallChangeQ * simd_quatd::ambientOcclusionDirection(k, maxk);
        double4x4 currentModelMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(q), centerOfScene);
        double4x4 viewMatrix = RKCamera::GluLookAt(eye, centerOfScene, double3(0, 1, 0));
        double4x4 projectionMatrix = RKCamera::glFrustumfOrthographic(left, right, bottom, top, 1.0, 1000.0);
        shadowUniforms.emplace_back(projectionMatrix, viewMatrix, currentModelMatrix);
      }

      uploadPackedUniforms(_aoStructureUniformBuffer, aoStructureUniforms.data(), aoStructureUniforms.size(),
                           sizeof(RKStructureUniforms), _structureStride);
      uploadPackedUniforms(_ribbonStructureUniformBuffer, ribbonStructureUniforms.data(), ribbonStructureUniforms.size(),
                           sizeof(RKStructureUniforms), _structureStride);
      uploadPackedUniforms(_shadowUniformBuffer, shadowUniforms.data(), shadowUniforms.size(), sizeof(RKShadowUniforms),
                           _shadowStride);

      auto rewriteGenerationSet = [&](VkDescriptorSet set, const VulkanBuffer &structureBuffer) {
        VkDescriptorBufferInfo structureInfo{structureBuffer.buffer, 0, sizeof(RKStructureUniforms)};
        VkDescriptorBufferInfo shadowInfo{_shadowUniformBuffer.buffer, 0, sizeof(RKShadowUniforms)};
        VkWriteDescriptorSet writes[2]{};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = set;
        writes[0].dstBinding = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writes[0].descriptorCount = 1;
        writes[0].pBufferInfo = &structureInfo;
        writes[1] = writes[0];
        writes[1].dstBinding = 1;
        writes[1].pBufferInfo = &shadowInfo;
        vkUpdateDescriptorSets(_renderer->device(), 2, writes, 0, nullptr);
      };
      rewriteGenerationSet(_generationSetAO, _aoStructureUniformBuffer);
      rewriteGenerationSet(_generationSetRibbon, _ribbonStructureUniformBuffer);
      VkDescriptorSet ribbonGenerationSet = ribbonUsesRenderUniformsForShadow ? _generationSetRibbon : _generationSetAO;

      VulkanTexture atlas = _renderer->createAttachmentTexture(
          width, height, VK_FORMAT_R16_SFLOAT,
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
          VK_IMAGE_ASPECT_COLOR_BIT);
      VkFramebuffer atlasFramebuffer = VK_NULL_HANDLE;
      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = _aoClearRenderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = &atlas.view;
      framebufferInfo.width = width;
      framebufferInfo.height = height;
      framebufferInfo.layers = 1;
      if (vkCreateFramebuffer(_renderer->device(), &framebufferInfo, nullptr, &atlasFramebuffer) != VK_SUCCESS)
      {
        _renderer->destroyTexture(atlas);
        throw std::runtime_error("failed to create ribbon AO atlas framebuffer");
      }

      VkCommandBuffer commandBuffer = _renderer->beginOneTimeCommands();
      VkRenderPassBeginInfo aoBegin{};
      aoBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      aoBegin.renderPass = _aoClearRenderPass;
      aoBegin.framebuffer = atlasFramebuffer;
      aoBegin.renderArea.extent = {width, height};
      VkClearValue aoClear{};
      aoClear.color = {{0.0f, 0.0f, 0.0f, 0.0f}};
      aoBegin.clearValueCount = 1;
      aoBegin.pClearValues = &aoClear;
      vkCmdBeginRenderPass(commandBuffer, &aoBegin, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdEndRenderPass(commandBuffer);

      bool drewGeometry = false;
      for (int k = 0; k < maxk; ++k)
      {
        VkRenderPassBeginInfo shadowBegin{};
        shadowBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        shadowBegin.renderPass = _shadowRenderPass;
        shadowBegin.framebuffer = _shadowFramebuffer;
        shadowBegin.renderArea.extent = {kShadowMapSize, kShadowMapSize};
        VkClearValue shadowClear{};
        shadowClear.depthStencil = {1.0f, 0};
        shadowBegin.clearValueCount = 1;
        shadowBegin.pClearValues = &shadowClear;
        vkCmdBeginRenderPass(commandBuffer, &shadowBegin, VK_SUBPASS_CONTENTS_INLINE);
        setViewport(commandBuffer, kShadowMapSize, kShadowMapSize);

        VkDeviceSize vertexOffset = 0;
        for (size_t l = 0; l < _renderStructures[i].size(); ++l)
        {
          if (!_renderStructures[i][l]->isVisible())
          {
            continue;
          }
          const uint32_t offsets[] = {static_cast<uint32_t>(l * _structureStride), static_cast<uint32_t>(k * _shadowStride)};
          if (includeAtomShadows && _atomShader)
          {
            auto *atomSource = dynamic_cast<RKRenderAtomSource *>(_renderStructures[i][l].get());
            const uint32_t count = _atomShader->instanceCount(i, l);
            VkBuffer instances = _atomShader->instanceBuffer(i, l);
            if (atomSource && atomSource->atomAmbientOcclusion() && atomSource->drawAtoms() && count > 0 && instances)
            {
              vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _atomShadowPipeline);
              vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _shadowPipelineLayout, 0, 1, &_generationSetAO,
                                      2, offsets);
              vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_quadVertexBuffer.buffer, &vertexOffset);
              vkCmdBindIndexBuffer(commandBuffer, _quadIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
              vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instances, &vertexOffset);
              vkCmdDrawIndexed(commandBuffer, _quadIndexCount, count, 0, 0, 0);
            }
          }

          auto *shadowRibbon = dynamic_cast<RKRenderRibbonSource *>(_renderStructures[i][l].get());
          VkBuffer ribbonVertices = _ribbonShader->vertexBuffer(i, l);
          VkBuffer ribbonIndices = _ribbonShader->indexBuffer(i, l);
          if (shadowRibbon && shadowRibbon->drawRibbon() && _ribbonShader->indexCount(i, l) > 0 && ribbonVertices && ribbonIndices)
          {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _ribbonShadowPipeline);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _shadowPipelineLayout, 0, 1, &ribbonGenerationSet,
                                    2, offsets);
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &ribbonVertices, &vertexOffset);
            vkCmdBindIndexBuffer(commandBuffer, ribbonIndices, 0, VK_INDEX_TYPE_UINT32);
            _ribbonShader->drawAllChains(commandBuffer, shadowRibbon);
          }
        }
        vkCmdEndRenderPass(commandBuffer);

        recordImageBarrier(commandBuffer, _shadowMap.image, VK_IMAGE_ASPECT_DEPTH_BIT,
                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                           VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        VkRenderPassBeginInfo accumulateBegin{};
        accumulateBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        accumulateBegin.renderPass = _aoAccumulateRenderPass;
        accumulateBegin.framebuffer = atlasFramebuffer;
        accumulateBegin.renderArea.extent = {width, height};
        vkCmdBeginRenderPass(commandBuffer, &accumulateBegin, VK_SUBPASS_CONTENTS_INLINE);
        setViewport(commandBuffer, width, height);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _accumulatePipeline);
        const uint32_t offsets[] = {static_cast<uint32_t>(j * _structureStride), static_cast<uint32_t>(k * _shadowStride)};
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _accumulatePipelineLayout, 0, 1, &ribbonGenerationSet,
                                2, offsets);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _accumulatePipelineLayout, 1, 1, &_shadowSamplerSet, 0,
                                nullptr);
        const float weight = simd_quatd::ambientOcclusionBlendWeight(k, maxk);
        vkCmdPushConstants(commandBuffer, _accumulatePipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &weight);
        VkBuffer ribbonVertices = _ribbonShader->vertexBuffer(i, j);
        VkBuffer ribbonIndices = _ribbonShader->indexBuffer(i, j);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &ribbonVertices, &vertexOffset);
        vkCmdBindIndexBuffer(commandBuffer, ribbonIndices, 0, VK_INDEX_TYPE_UINT32);
        _ribbonShader->drawAllChains(commandBuffer, ribbonSource);
        drewGeometry = true;
        vkCmdEndRenderPass(commandBuffer);
      }

      _renderer->submitOneTimeCommands(commandBuffer);

      std::vector<uint16_t> raw(static_cast<size_t>(width) * height);
      _renderer->copyImageToHost(atlas.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_FORMAT_R16_SFLOAT,
                                 VK_IMAGE_ASPECT_COLOR_BIT, width, height, raw.data(), raw.size() * sizeof(uint16_t));
      vkDestroyFramebuffer(_renderer->device(), atlasFramebuffer, nullptr);
      _renderer->destroyTexture(atlas);

      if (drewGeometry)
      {
        std::vector<float> channel(raw.size());
        for (size_t index = 0; index < channel.size(); ++index)
        {
          channel[index] = RKHalfFloat::floatFromHalfBits(raw[index]);
        }
        RibbonAOTexturePostProcess::dilateAndSmooth(channel, static_cast<int>(width), static_cast<int>(height));
        RibbonAOTexturePostProcess::gaussianBlur(channel, static_cast<int>(width), static_cast<int>(height));
        std::vector<uint16_t> processed(channel.size());
        for (size_t index = 0; index < processed.size(); ++index)
        {
          processed[index] = RKHalfFloat::halfBitsFromFloat(channel[index]);
        }
        if (!ribbonAmbientOcclusionTextureHasContent(processed))
        {
          processed = whiteAtlas(width, height);
        }
        _cache[cacheKey] = std::make_shared<std::vector<uint16_t>>(processed);
        assignTexture(processed);
      }
      else
      {
        assignTexture(whiteAtlas(width, height));
      }
    }
  }
}
