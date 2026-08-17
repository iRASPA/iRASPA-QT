#include "vulkanatomambientocclusionshader.h"

#include "quadgeometry.h"
#include "ribbonaolayout.h"
#include "rkcamera.h"
#include "vulkanatomsphereshader.h"
#include "vulkanribbonambientocclusionshader.h"
#include "vulkanshader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>

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
}  // namespace

VulkanAtomAmbientOcclusionShader::VulkanAtomAmbientOcclusionShader(VulkanRenderer *renderer, VulkanAtomSphereShader *atomShader)
    : _renderer(renderer), _atomShader(atomShader)
{
}

VulkanAtomAmbientOcclusionShader::~VulkanAtomAmbientOcclusionShader()
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
  _renderer->destroyBuffer(_shadowUniformBuffer);
  _renderer->destroyBuffer(_quadVertexBuffer);
  _renderer->destroyBuffer(_quadIndexBuffer);
  if (_generationSet)
  {
    vkFreeDescriptorSets(_renderer->device(), _renderer->descriptorPool(), 1, &_generationSet);
  }
  if (_shadowSamplerSet)
  {
    vkFreeDescriptorSets(_renderer->device(), _renderer->descriptorPool(), 1, &_shadowSamplerSet);
  }
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

void VulkanAtomAmbientOcclusionShader::destroyStructureResources()
{
  for (auto &scene : _structureResources)
  {
    for (auto &resources : scene)
    {
      if (resources.framebuffer)
      {
        vkDestroyFramebuffer(_renderer->device(), resources.framebuffer, nullptr);
      }
      if (resources.samplerSet)
      {
        vkFreeDescriptorSets(_renderer->device(), _renderer->descriptorPool(), 1, &resources.samplerSet);
      }
      _renderer->destroyTexture(resources.texture);
    }
  }
  _structureResources.clear();
}

void VulkanAtomAmbientOcclusionShader::destroyPipelines()
{
  if (_shadowPipeline)
  {
    vkDestroyPipeline(_renderer->device(), _shadowPipeline, nullptr);
    _shadowPipeline = VK_NULL_HANDLE;
  }
  if (_accumulatePipeline)
  {
    vkDestroyPipeline(_renderer->device(), _accumulatePipeline, nullptr);
    _accumulatePipeline = VK_NULL_HANDLE;
  }
}

void VulkanAtomAmbientOcclusionShader::destroyShadowResources()
{
  if (_shadowFramebuffer)
  {
    vkDestroyFramebuffer(_renderer->device(), _shadowFramebuffer, nullptr);
    _shadowFramebuffer = VK_NULL_HANDLE;
  }
  _renderer->destroyTexture(_shadowMap);
}

void VulkanAtomAmbientOcclusionShader::initialize()
{
  QuadGeometry quad;
  const auto vertices = quad.vertices();
  const auto indices = quad.indices();
  _quadIndexCount = static_cast<uint32_t>(indices.size());
  _renderer->uploadBuffer(_quadVertexBuffer, vertices.data(), vertices.size() * sizeof(RKVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  _renderer->uploadBuffer(_quadIndexBuffer, indices.data(), indices.size() * sizeof(short), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  _aoStructureStride = _renderer->structureUniformStride();
  _shadowStride = VulkanRenderer::alignedUniformSize(sizeof(RKShadowUniforms), _renderer->minUniformBufferOffsetAlignment());

  createRenderPasses();
  createShadowResources();
  createGenerationDescriptors();
  createPipelines();
}

void VulkanAtomAmbientOcclusionShader::createRenderPasses()
{
  auto makeDepthPass = [&](VkAttachmentLoadOp loadOp, VkImageLayout initialLayout) {
    VkAttachmentDescription depth{};
    depth.format = _renderer->depthFormat();
    depth.samples = VK_SAMPLE_COUNT_1_BIT;
    depth.loadOp = loadOp;
    depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth.initialLayout = initialLayout;
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
      throw std::runtime_error("failed to create AO shadow render pass");
    }
    return renderPass;
  };

  auto makeColorPass = [&](VkAttachmentLoadOp loadOp, VkImageLayout initialLayout, VkImageLayout finalLayout) {
    VkAttachmentDescription color{};
    color.format = VK_FORMAT_R16_SFLOAT;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = loadOp;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = initialLayout;
    color.finalLayout = finalLayout;
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
      throw std::runtime_error("failed to create AO color render pass");
    }
    return renderPass;
  };

  _shadowRenderPass = makeDepthPass(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED);
  _aoClearRenderPass = makeColorPass(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  _aoAccumulateRenderPass =
      makeColorPass(VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void VulkanAtomAmbientOcclusionShader::createShadowResources()
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
    throw std::runtime_error("failed to create AO shadow framebuffer");
  }
}

void VulkanAtomAmbientOcclusionShader::createGenerationDescriptors()
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
    throw std::runtime_error("failed to create AO generation descriptor layout");
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
    throw std::runtime_error("failed to create AO shadow sampler layout");
  }

  VkDescriptorSetLayout shadowLayouts[] = {_generationSetLayout};
  VkPipelineLayoutCreateInfo shadowLayoutInfo{};
  shadowLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  shadowLayoutInfo.setLayoutCount = 1;
  shadowLayoutInfo.pSetLayouts = shadowLayouts;
  if (vkCreatePipelineLayout(_renderer->device(), &shadowLayoutInfo, nullptr, &_shadowPipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create AO shadow pipeline layout");
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
    throw std::runtime_error("failed to create AO accumulate pipeline layout");
  }

  _renderer->createBuffer(_aoStructureStride, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _aoStructureUniformBuffer);
  _renderer->createBuffer(_shadowStride, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _shadowUniformBuffer);

  VkDescriptorSetAllocateInfo generationAlloc{};
  generationAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  generationAlloc.descriptorPool = _renderer->descriptorPool();
  generationAlloc.descriptorSetCount = 1;
  generationAlloc.pSetLayouts = &_generationSetLayout;
  if (vkAllocateDescriptorSets(_renderer->device(), &generationAlloc, &_generationSet) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate AO generation descriptor set");
  }

  VkDescriptorSetAllocateInfo samplerAlloc = generationAlloc;
  samplerAlloc.pSetLayouts = &_shadowSamplerSetLayout;
  if (vkAllocateDescriptorSets(_renderer->device(), &samplerAlloc, &_shadowSamplerSet) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate AO shadow sampler set");
  }

  VkDescriptorImageInfo shadowImage{};
  shadowImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  shadowImage.imageView = _shadowMap.view;
  shadowImage.sampler = _renderer->shadowCompareSampler();
  VkWriteDescriptorSet shadowWrite{};
  shadowWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  shadowWrite.dstSet = _shadowSamplerSet;
  shadowWrite.dstBinding = 0;
  shadowWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  shadowWrite.descriptorCount = 1;
  shadowWrite.pImageInfo = &shadowImage;
  vkUpdateDescriptorSets(_renderer->device(), 1, &shadowWrite, 0, nullptr);
}

void VulkanAtomAmbientOcclusionShader::createPipelines()
{
  VulkanShader::PipelineConfig shadow;
  shadow.vertexShaderResource = QStringLiteral(":/shaders/ao_shadow.vert.spv");
  shadow.fragmentShaderResource = QStringLiteral(":/shaders/ao_shadow.frag.spv");
  shadow.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  shadow.cullMode = VK_CULL_MODE_NONE;
  shadow.depthTest = true;
  shadow.depthWrite = true;
  shadow.depthClamp = true;
  shadow.colorAttachmentCount = 0;
  shadow.renderPass = _shadowRenderPass;

  VkVertexInputBindingDescription vertexBinding{};
  vertexBinding.binding = 0;
  vertexBinding.stride = sizeof(RKVertex);
  vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  VkVertexInputBindingDescription instanceBinding{};
  instanceBinding.binding = 1;
  instanceBinding.stride = sizeof(RKInPerInstanceAttributesAtoms);
  instanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  shadow.bindings = {vertexBinding, instanceBinding};
  shadow.attributes = {
      float4Attribute(0, 0, offsetof(RKVertex, position)),
      float4Attribute(1, 1, offsetof(RKInPerInstanceAttributesAtoms, position)),
      float4Attribute(2, 1, offsetof(RKInPerInstanceAttributesAtoms, scale)),
  };
  _shadowPipeline = VulkanShader::createGraphicsPipeline(_renderer, _shadowPipelineLayout, shadow);

  VulkanShader::PipelineConfig accumulate = shadow;
  accumulate.vertexShaderResource = QStringLiteral(":/shaders/ao_accumulate.vert.spv");
  accumulate.fragmentShaderResource = QStringLiteral(":/shaders/ao_accumulate.frag.spv");
  accumulate.depthTest = false;
  accumulate.depthWrite = false;
  accumulate.depthClamp = false;
  accumulate.colorAttachmentCount = 1;
  accumulate.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
  accumulate.blend = true;
  accumulate.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  accumulate.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
  accumulate.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  accumulate.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  accumulate.renderPass = _aoAccumulateRenderPass;
  _accumulatePipeline = VulkanShader::createGraphicsPipeline(_renderer, _accumulatePipelineLayout, accumulate);
}

void VulkanAtomAmbientOcclusionShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
}

void VulkanAtomAmbientOcclusionShader::invalidateCachedAmbientOcclusionTexture(
    const std::vector<std::shared_ptr<RKRenderObject>> &structures)
{
  for (const auto &structure : structures)
  {
    _cache.erase(structure.get());
  }
}

VkDescriptorSet VulkanAtomAmbientOcclusionShader::samplerSet(size_t sceneIndex, size_t movieIndex) const
{
  if (sceneIndex >= _structureResources.size() || movieIndex >= _structureResources[sceneIndex].size())
  {
    return VK_NULL_HANDLE;
  }
  return _structureResources[sceneIndex][movieIndex].samplerSet;
}

bool VulkanAtomAmbientOcclusionShader::wantsBake(size_t sceneIndex, size_t movieIndex) const
{
  if (sceneIndex >= _renderStructures.size() || movieIndex >= _renderStructures[sceneIndex].size())
  {
    return false;
  }
  auto *object = dynamic_cast<RKRenderObject *>(_renderStructures[sceneIndex][movieIndex].get());
  auto *source = dynamic_cast<RKRenderAtomSource *>(_renderStructures[sceneIndex][movieIndex].get());
  return object && source && object->cell() && source->drawAtoms() && source->atomAmbientOcclusion() && object->isVisible() &&
         _atomShader->instanceCount(sceneIndex, movieIndex) > 0;
}

bool VulkanAtomAmbientOcclusionShader::hasCachedTexture(RKRenderObject *key, uint32_t textureSize) const
{
  auto cacheIt = _cache.find(key);
  return cacheIt != _cache.end() && cacheIt->second &&
         cacheIt->second->size() == static_cast<size_t>(textureSize) * textureSize;
}

void VulkanAtomAmbientOcclusionShader::setGenerationBuffers(const VulkanBuffer &structureBuffer, const VulkanBuffer &shadowBuffer)
{
  VkDescriptorBufferInfo structureInfo{structureBuffer.buffer, 0, sizeof(RKStructureUniforms)};
  VkDescriptorBufferInfo shadowInfo{shadowBuffer.buffer, 0, sizeof(RKShadowUniforms)};
  auto writeBuffer = [&](uint32_t binding, const VkDescriptorBufferInfo *info) {
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = _generationSet;
    write.dstBinding = binding;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    write.descriptorCount = 1;
    write.pBufferInfo = info;
    return write;
  };
  std::array<VkWriteDescriptorSet, 2> writes = {writeBuffer(0, &structureInfo), writeBuffer(1, &shadowInfo)};
  vkUpdateDescriptorSets(_renderer->device(), 2, writes.data(), 0, nullptr);
}

void VulkanAtomAmbientOcclusionShader::useShadowMap(VkImageView shadowMapView)
{
  VkDescriptorImageInfo shadowImage{};
  shadowImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  shadowImage.imageView = shadowMapView;
  shadowImage.sampler = _renderer->shadowCompareSampler();
  VkWriteDescriptorSet shadowWrite{};
  shadowWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  shadowWrite.dstSet = _shadowSamplerSet;
  shadowWrite.dstBinding = 0;
  shadowWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  shadowWrite.descriptorCount = 1;
  shadowWrite.pImageInfo = &shadowImage;
  vkUpdateDescriptorSets(_renderer->device(), 1, &shadowWrite, 0, nullptr);
}

void VulkanAtomAmbientOcclusionShader::restoreDefaultShadowMap()
{
  useShadowMap(_shadowMap.view);
}

bool VulkanAtomAmbientOcclusionShader::prepareTarget(size_t sceneIndex, size_t movieIndex)
{
  if (!wantsBake(sceneIndex, movieIndex) || !_accumulatePipeline)
  {
    return false;
  }

  auto *source = dynamic_cast<RKRenderAtomSource *>(_renderStructures[sceneIndex][movieIndex].get());
  StructureResources &resources = _structureResources[sceneIndex][movieIndex];
  const uint32_t textureSize = static_cast<uint32_t>(std::max(source->atomAmbientOcclusionTextureSize(), 1));
  resources.textureSize = textureSize;

  RKRenderObject *key = _renderStructures[sceneIndex][movieIndex].get();
  if (hasCachedTexture(key, textureSize))
  {
    resources.texture = _renderer->createTextureR16F(textureSize, textureSize, _cache[key]->data());
    resources.samplerSet = _renderer->allocateSamplerDescriptorSet(resources.texture);
    return false;
  }

  resources.texture = _renderer->createAttachmentTexture(
      textureSize, textureSize, VK_FORMAT_R16_SFLOAT,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
      VK_IMAGE_ASPECT_COLOR_BIT);
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = _aoClearRenderPass;
  framebufferInfo.attachmentCount = 1;
  framebufferInfo.pAttachments = &resources.texture.view;
  framebufferInfo.width = textureSize;
  framebufferInfo.height = textureSize;
  framebufferInfo.layers = 1;
  if (vkCreateFramebuffer(_renderer->device(), &framebufferInfo, nullptr, &resources.framebuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create AO atlas framebuffer");
  }
  return true;
}

void VulkanAtomAmbientOcclusionShader::recordClear(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex)
{
  StructureResources &resources = _structureResources[sceneIndex][movieIndex];
  VkRenderPassBeginInfo aoBegin{};
  aoBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  aoBegin.renderPass = _aoClearRenderPass;
  aoBegin.framebuffer = resources.framebuffer;
  aoBegin.renderArea.extent = {resources.textureSize, resources.textureSize};
  VkClearValue aoClear{};
  aoClear.color = {{0.0f, 0.0f, 0.0f, 0.0f}};
  aoBegin.clearValueCount = 1;
  aoBegin.pClearValues = &aoClear;
  vkCmdBeginRenderPass(commandBuffer, &aoBegin, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdEndRenderPass(commandBuffer);
}

void VulkanAtomAmbientOcclusionShader::recordAccumulate(VkCommandBuffer commandBuffer, size_t sceneIndex, size_t movieIndex,
                                                        uint32_t directionIndex, VkDeviceSize structureStride,
                                                        VkDeviceSize shadowStride, float weight)
{
  StructureResources &resources = _structureResources[sceneIndex][movieIndex];
  VkBuffer instances = _atomShader->instanceBuffer(sceneIndex, movieIndex);
  const uint32_t count = _atomShader->instanceCount(sceneIndex, movieIndex);
  if (!instances || count == 0 || !resources.framebuffer)
  {
    return;
  }

  VkRenderPassBeginInfo accumulateBegin{};
  accumulateBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  accumulateBegin.renderPass = _aoAccumulateRenderPass;
  accumulateBegin.framebuffer = resources.framebuffer;
  accumulateBegin.renderArea.extent = {resources.textureSize, resources.textureSize};
  vkCmdBeginRenderPass(commandBuffer, &accumulateBegin, VK_SUBPASS_CONTENTS_INLINE);
  setViewport(commandBuffer, resources.textureSize, resources.textureSize);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _accumulatePipeline);
  VkDeviceSize vertexOffset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_quadVertexBuffer.buffer, &vertexOffset);
  vkCmdBindIndexBuffer(commandBuffer, _quadIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
  const uint32_t offsets[] = {static_cast<uint32_t>(movieIndex * structureStride),
                              static_cast<uint32_t>(directionIndex * shadowStride)};
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _accumulatePipelineLayout, 0, 1, &_generationSet, 2,
                          offsets);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _accumulatePipelineLayout, 1, 1, &_shadowSamplerSet, 0,
                          nullptr);
  vkCmdPushConstants(commandBuffer, _accumulatePipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &weight);
  vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instances, &vertexOffset);
  vkCmdDrawIndexed(commandBuffer, _quadIndexCount, count, 0, 0, 0);
  vkCmdEndRenderPass(commandBuffer);
}

void VulkanAtomAmbientOcclusionShader::finalizeTarget(size_t sceneIndex, size_t movieIndex)
{
  StructureResources &resources = _structureResources[sceneIndex][movieIndex];
  RKRenderObject *key = _renderStructures[sceneIndex][movieIndex].get();
  if (resources.textureSize < 2048)
  {
    auto cached = std::make_shared<std::vector<uint16_t>>(static_cast<size_t>(resources.textureSize) * resources.textureSize);
    _renderer->copyImageToHost(resources.texture.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_FORMAT_R16_SFLOAT,
                               VK_IMAGE_ASPECT_COLOR_BIT, resources.textureSize, resources.textureSize, cached->data(),
                               cached->size() * sizeof(uint16_t));
    _renderer->transitionImageLayout(resources.texture.image, VK_FORMAT_R16_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    _cache[key] = cached;
  }
  else
  {
    _renderer->transitionImageLayout(resources.texture.image, VK_FORMAT_R16_SFLOAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
  resources.samplerSet = _renderer->allocateSamplerDescriptorSet(resources.texture);
}

void VulkanAtomAmbientOcclusionShader::reloadData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality,
                                                 VulkanRibbonAmbientOcclusionShader *ribbonAO)
{
  destroyStructureResources();
  _structureResources.resize(_renderStructures.size());
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureResources[i].resize(_renderStructures[i].size());
  }
  adjustTextureSizes();
  generateTextures(dataSource, quality, ribbonAO);
}

void VulkanAtomAmbientOcclusionShader::adjustTextureSizes()
{
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *source = dynamic_cast<RKRenderAtomSource *>(_renderStructures[i][j].get());
      if (!source || !source->drawAtoms() || !source->atomAmbientOcclusion())
      {
        continue;
      }
      const size_t numberOfAtoms = _atomShader->instanceCount(i, j);
      int textureSize = 256;
      if (numberOfAtoms < 64)
      {
        textureSize = 256;
      }
      else if (numberOfAtoms < 256)
      {
        textureSize = 512;
      }
      else if (numberOfAtoms < 1024)
      {
        textureSize = 1024;
      }
      else if (numberOfAtoms < 65536)
      {
        textureSize = 2048;
      }
      else if (numberOfAtoms < 524288)
      {
        textureSize = 4096;
      }
      else
      {
        textureSize = 8192;
      }
      source->setAtomAmbientOcclusionTextureSize(textureSize);
      source->setAtomAmbientOcclusionPatchNumber(static_cast<int>(std::sqrt(static_cast<double>(numberOfAtoms))) + 1);
      source->setAtomAmbientOcclusionPatchSize(source->atomAmbientOcclusionTextureSize() /
                                               std::max(source->atomAmbientOcclusionPatchNumber(), 1));
    }
  }
}

void VulkanAtomAmbientOcclusionShader::recordImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspect,
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

void VulkanAtomAmbientOcclusionShader::generateTextures(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality,
                                                        VulkanRibbonAmbientOcclusionShader *ribbonAO)
{
  if (!dataSource || !_shadowPipeline || !_accumulatePipeline)
  {
    return;
  }

  const int maxk = (quality == RKRenderQuality::picture) ? 1992 : 360;

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *object = dynamic_cast<RKRenderObject *>(_renderStructures[i][j].get());
      auto *source = dynamic_cast<RKRenderAtomSource *>(_renderStructures[i][j].get());
      if (!object || !source || !object->cell() || !source->drawAtoms() || !source->atomAmbientOcclusion() ||
          !object->isVisible() || _atomShader->instanceCount(i, j) == 0)
      {
        continue;
      }

      const uint32_t atomTextureSize = static_cast<uint32_t>(std::max(source->atomAmbientOcclusionTextureSize(), 1));
      auto *ribbonSource = dynamic_cast<RKRenderRibbonSource *>(object);
      const bool ribbonFreshBake =
          ribbonAO && shouldBakeRibbonAmbientOcclusion(object) && ribbonSource &&
          !ribbonAO->hasCachedTexture(object,
                                      static_cast<uint32_t>(std::max(ribbonSource->ribbonAmbientOcclusionTextureWidth(), 1)),
                                      static_cast<uint32_t>(std::max(ribbonSource->ribbonAmbientOcclusionTextureHeight(), 1)));
      if (ribbonFreshBake && !hasCachedTexture(object, atomTextureSize))
      {
        continue;
      }

      StructureResources &resources = _structureResources[i][j];
      const uint32_t textureSize = static_cast<uint32_t>(std::max(source->atomAmbientOcclusionTextureSize(), 1));
      resources.textureSize = textureSize;

      RKRenderObject *key = _renderStructures[i][j].get();
      auto cacheIt = _cache.find(key);
      if (cacheIt != _cache.end() && cacheIt->second &&
          cacheIt->second->size() == static_cast<size_t>(textureSize) * textureSize)
      {
        resources.texture = _renderer->createTextureR16F(textureSize, textureSize, cacheIt->second->data());
        resources.samplerSet = _renderer->allocateSamplerDescriptorSet(resources.texture);
        continue;
      }

      resources.texture = _renderer->createAttachmentTexture(
          textureSize, textureSize, VK_FORMAT_R16_SFLOAT,
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
          VK_IMAGE_ASPECT_COLOR_BIT);
      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = _aoClearRenderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = &resources.texture.view;
      framebufferInfo.width = textureSize;
      framebufferInfo.height = textureSize;
      framebufferInfo.layers = 1;
      if (vkCreateFramebuffer(_renderer->device(), &framebufferInfo, nullptr, &resources.framebuffer) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create AO atlas framebuffer");
      }

      double4x4 modelMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPointWithTranslation(
          double4x4(object->orientation()), object->cell()->boundingBox().center(), object->origin());
      std::vector<RKStructureUniforms> structureUniforms;
      structureUniforms.reserve(_renderStructures[i].size());
      for (size_t k = 0; k < _renderStructures[i].size(); ++k)
      {
        structureUniforms.emplace_back(i, k, _renderStructures[i][k], double4x4::inverse(modelMatrix));
      }

      const VkDeviceSize structureBytes = std::max<VkDeviceSize>(structureUniforms.size(), 1) * _aoStructureStride;
      _renderer->destroyBuffer(_aoStructureUniformBuffer);
      _renderer->createBuffer(structureBytes, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              _aoStructureUniformBuffer);
      std::vector<uint8_t> structurePacked(static_cast<size_t>(structureBytes), 0);
      for (size_t k = 0; k < structureUniforms.size(); ++k)
      {
        std::memcpy(structurePacked.data() + k * _aoStructureStride, &structureUniforms[k], sizeof(RKStructureUniforms));
      }
      void *mapped = nullptr;
      vkMapMemory(_renderer->device(), _aoStructureUniformBuffer.memory, 0, structureBytes, 0, &mapped);
      std::memcpy(mapped, structurePacked.data(), structurePacked.size());
      vkUnmapMemory(_renderer->device(), _aoStructureUniformBuffer.memory);

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
      const double near1 = 1.0;
      const double far1 = 1000.0;

      std::vector<RKShadowUniforms> shadowUniforms;
      shadowUniforms.reserve(static_cast<size_t>(maxk));
      for (int k = 0; k < maxk; ++k)
      {
        simd_quatd smallChangeQ = simd_quatd::smallRandomQuaternion(0.5 * 10.0 * M_PI / 180.0);
        simd_quatd q = smallChangeQ * simd_quatd::ambientOcclusionDirection(k, maxk);
        double4x4 currentModelMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(q), centerOfScene);
        double4x4 viewMatrix = RKCamera::GluLookAt(eye, centerOfScene, double3(0, 1, 0));
        double4x4 projectionMatrix = RKCamera::glFrustumfOrthographic(left, right, bottom, top, near1, far1);
        shadowUniforms.emplace_back(projectionMatrix, viewMatrix, currentModelMatrix);
      }

      const VkDeviceSize shadowBytes = static_cast<VkDeviceSize>(maxk) * _shadowStride;
      _renderer->destroyBuffer(_shadowUniformBuffer);
      _renderer->createBuffer(shadowBytes, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _shadowUniformBuffer);
      std::vector<uint8_t> shadowPacked(static_cast<size_t>(shadowBytes), 0);
      for (int k = 0; k < maxk; ++k)
      {
        std::memcpy(shadowPacked.data() + static_cast<size_t>(k) * _shadowStride, &shadowUniforms[static_cast<size_t>(k)],
                    sizeof(RKShadowUniforms));
      }
      mapped = nullptr;
      vkMapMemory(_renderer->device(), _shadowUniformBuffer.memory, 0, shadowBytes, 0, &mapped);
      std::memcpy(mapped, shadowPacked.data(), shadowPacked.size());
      vkUnmapMemory(_renderer->device(), _shadowUniformBuffer.memory);

      VkDescriptorBufferInfo structureInfo{_aoStructureUniformBuffer.buffer, 0, sizeof(RKStructureUniforms)};
      VkDescriptorBufferInfo shadowInfo{_shadowUniformBuffer.buffer, 0, sizeof(RKShadowUniforms)};
      auto writeBuffer = [&](uint32_t binding, const VkDescriptorBufferInfo *info) {
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = _generationSet;
        write.dstBinding = binding;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        write.descriptorCount = 1;
        write.pBufferInfo = info;
        return write;
      };
      std::array<VkWriteDescriptorSet, 2> writes = {writeBuffer(0, &structureInfo), writeBuffer(1, &shadowInfo)};
      vkUpdateDescriptorSets(_renderer->device(), 2, writes.data(), 0, nullptr);

      VkCommandBuffer commandBuffer = _renderer->beginOneTimeCommands();

      VkRenderPassBeginInfo aoBegin{};
      aoBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      aoBegin.renderPass = _aoClearRenderPass;
      aoBegin.framebuffer = resources.framebuffer;
      aoBegin.renderArea.extent = {textureSize, textureSize};
      VkClearValue aoClear{};
      aoClear.color = {{0.0f, 0.0f, 0.0f, 0.0f}};
      aoBegin.clearValueCount = 1;
      aoBegin.pClearValues = &aoClear;
      vkCmdBeginRenderPass(commandBuffer, &aoBegin, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdEndRenderPass(commandBuffer);

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
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _shadowPipeline);
        VkDeviceSize vertexOffset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_quadVertexBuffer.buffer, &vertexOffset);
        vkCmdBindIndexBuffer(commandBuffer, _quadIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

        for (size_t l = 0; l < _renderStructures[i].size(); ++l)
        {
          const uint32_t count = _atomShader->instanceCount(i, l);
          VkBuffer instances = _atomShader->instanceBuffer(i, l);
          if (count == 0 || !instances || !_renderStructures[i][l]->isVisible())
          {
            continue;
          }
          const uint32_t offsets[] = {static_cast<uint32_t>(l * _aoStructureStride), static_cast<uint32_t>(k * _shadowStride)};
          vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _shadowPipelineLayout, 0, 1, &_generationSet, 2,
                                  offsets);
          vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instances, &vertexOffset);
          vkCmdDrawIndexed(commandBuffer, _quadIndexCount, count, 0, 0, 0);
        }
        vkCmdEndRenderPass(commandBuffer);

        recordImageBarrier(commandBuffer, _shadowMap.image, VK_IMAGE_ASPECT_DEPTH_BIT,
                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                           VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        VkRenderPassBeginInfo accumulateBegin{};
        accumulateBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        accumulateBegin.renderPass = _aoAccumulateRenderPass;
        accumulateBegin.framebuffer = resources.framebuffer;
        accumulateBegin.renderArea.extent = {textureSize, textureSize};
        vkCmdBeginRenderPass(commandBuffer, &accumulateBegin, VK_SUBPASS_CONTENTS_INLINE);
        setViewport(commandBuffer, textureSize, textureSize);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _accumulatePipeline);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_quadVertexBuffer.buffer, &vertexOffset);
        vkCmdBindIndexBuffer(commandBuffer, _quadIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
        const uint32_t offsets[] = {static_cast<uint32_t>(j * _aoStructureStride), static_cast<uint32_t>(k * _shadowStride)};
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _accumulatePipelineLayout, 0, 1, &_generationSet, 2,
                                offsets);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _accumulatePipelineLayout, 1, 1, &_shadowSamplerSet,
                                0, nullptr);
        const float weight = simd_quatd::ambientOcclusionBlendWeight(k, maxk);
        vkCmdPushConstants(commandBuffer, _accumulatePipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &weight);
        VkBuffer instances = _atomShader->instanceBuffer(i, j);
        vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instances, &vertexOffset);
        vkCmdDrawIndexed(commandBuffer, _quadIndexCount, _atomShader->instanceCount(i, j), 0, 0, 0);
        vkCmdEndRenderPass(commandBuffer);
      }

      _renderer->submitOneTimeCommands(commandBuffer);

      // Large atlases stay on the GPU. A host readback stalls the UI for tens of
      // milliseconds extra and is only useful when switching back to a cached project.
      if (textureSize < 2048)
      {
        auto cached = std::make_shared<std::vector<uint16_t>>(static_cast<size_t>(textureSize) * textureSize);
        _renderer->copyImageToHost(resources.texture.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_FORMAT_R16_SFLOAT,
                                   VK_IMAGE_ASPECT_COLOR_BIT, textureSize, textureSize, cached->data(),
                                   cached->size() * sizeof(uint16_t));
        _renderer->transitionImageLayout(resources.texture.image, VK_FORMAT_R16_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        _cache[key] = cached;
      }
      else
      {
        _renderer->transitionImageLayout(resources.texture.image, VK_FORMAT_R16_SFLOAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      }
      resources.samplerSet = _renderer->allocateSamplerDescriptorSet(resources.texture);
    }
  }
}
