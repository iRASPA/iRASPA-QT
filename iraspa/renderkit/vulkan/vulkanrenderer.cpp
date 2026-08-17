#include "vulkanrenderer.h"

#include "quadgeometry.h"
#include "vulkanshader.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <QColor>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QImage>

#ifdef Q_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
#include <xcb/xcb.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QtGui/qguiapplication.h>
#endif
#endif

#ifdef Q_OS_MACOS
extern "C" void *makeViewMetalCompatible(void *handle);
#endif

#ifdef __APPLE__
#ifndef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#define VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME "VK_KHR_portability_enumeration"
#endif
#ifndef VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#define VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR 0x00000001
#endif
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t,
                                                    const char *, const char *msg, void *)
{
  qDebug() << "[Vulkan]" << msg;
  return VK_FALSE;
}

static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback)
{
  auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
  return func ? func(instance, pCreateInfo, pAllocator, pCallback) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator)
{
  auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
  if (func)
  {
    func(instance, callback, pAllocator);
  }
}

VulkanRenderer::VulkanRenderer(QWindow *window) : _window(window)
{
#ifdef Q_OS_MACOS
  _metalLayer = makeViewMetalCompatible(reinterpret_cast<void *>(window->winId()));
  if (!_metalLayer)
  {
    throw std::runtime_error("failed to create CAMetalLayer for Vulkan surface");
  }
  CFRetain(_metalLayer);
#endif
  initVulkan();
}

VulkanRenderer::VulkanRenderer(uint32_t width, uint32_t height) : _offscreen(true)
{
  _offscreenExtent.width = std::max(1u, width);
  _offscreenExtent.height = std::max(1u, height);
  deviceExtensions.clear();
  initVulkan();
}

VulkanRenderer::~VulkanRenderer()
{
  cleanup();
#ifdef Q_OS_MACOS
  if (_metalLayer)
  {
    CFRelease(_metalLayer);
    _metalLayer = nullptr;
  }
#endif
}

void VulkanRenderer::initVulkan()
{
  createInstance();
  setupDebugCallback();
  if (!_offscreen)
  {
    createSurface();
  }
  pickPhysicalDevice();
  createLogicalDevice();

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(_physicalDevice, &properties);
  _minUniformBufferOffsetAlignment = properties.limits.minUniformBufferOffsetAlignment;
  _structureUniformStride = alignedUniformSize(sizeof(RKStructureUniforms), _minUniformBufferOffsetAlignment);
  _isosurfaceUniformStride = alignedUniformSize(sizeof(RKIsosurfaceUniforms), _minUniformBufferOffsetAlignment);

  if (_offscreen)
  {
    createOffscreenTarget();
  }
  else
  {
    createSwapChain();
    createImageViews();
  }
  chooseMsaaSampleCount();
  createRenderPass();
  createContinueRenderPass();
  createPickRenderPass();
  createDepthResolveRenderPass();
  createBlurRenderPass();
  createCompositeRenderPass();
  createDescriptorSetLayouts();
  createPipelineLayouts();
  createCommandPool();
  createFullscreenQuad();
  createPostProcessPipelines();
  createDepthResources();
  createSceneColorResources();
  createSceneFramebuffer();
  createBlurResources();
  createCompositeFramebuffers();
  createPickFramebuffer();
  createUniformBuffers();
  createSampler();
  createWhiteTexture();
  createDescriptorPool();
  createSceneDescriptorSet();
  createPostProcessDescriptorSets();
  createCommandBuffers();
  createSyncObjects();
}

VkDeviceSize VulkanRenderer::alignedUniformSize(VkDeviceSize size, VkDeviceSize alignment)
{
  if (alignment == 0)
  {
    return size;
  }
  return (size + alignment - 1) & ~(alignment - 1);
}

void VulkanRenderer::waitIdle()
{
  if (_device)
  {
    vkDeviceWaitIdle(_device);
  }
}

void VulkanRenderer::resize(uint32_t w, uint32_t h)
{
  if (_offscreen)
  {
    const uint32_t width = std::max(1u, w);
    const uint32_t height = std::max(1u, h);
    if (width == _swapChainExtent.width && height == _swapChainExtent.height)
    {
      return;
    }
    _offscreenExtent = {width, height};
    recreateOffscreenTargets();
    return;
  }
#ifdef Q_OS_MACOS
  if (_window)
  {
    makeViewMetalCompatible(reinterpret_cast<void *>(_window->winId()));
  }
#endif
  recreateSwapChain();
}

bool VulkanRenderer::beginFrame()
{
  if (_swapChainExtent.width == 0 || _swapChainExtent.height == 0)
  {
    return false;
  }

#ifdef __APPLE__
  vkDeviceWaitIdle(_device);
#endif

  if (_offscreen)
  {
    _currentImageIndex = 0;
  }
  else
  {
    VkResult result = vkAcquireNextImageKHR(_device, _swapChain, std::numeric_limits<uint64_t>::max(), _imageAvailableSemaphore, VK_NULL_HANDLE,
                                            &_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      recreateSwapChain();
      return false;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
      throw std::runtime_error("failed to acquire swap chain image");
    }
  }

  vkWaitForFences(_device, 1, &_waitFences[_currentImageIndex], VK_TRUE, UINT64_MAX);
  vkResetFences(_device, 1, &_waitFences[_currentImageIndex]);
  vkResetCommandBuffer(_commandBuffers[_currentImageIndex], 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(_commandBuffers[_currentImageIndex], &beginInfo);

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = _renderPass;
  renderPassInfo.framebuffer = _sceneFramebuffer;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = _swapChainExtent;

  std::array<VkClearValue, 3> clearValues{};
  clearValues[0].color = {{1.0f, 1.0f, 1.0f, 1.0f}};
  clearValues[1].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
  clearValues[2].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(_commandBuffers[_currentImageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  setDefaultViewport(_commandBuffers[_currentImageIndex]);

  _frameStarted = true;
  _continuedPass = false;
  return true;
}

VkCommandBuffer VulkanRenderer::currentCommandBuffer() const
{
  return _commandBuffers[_currentImageIndex];
}

void VulkanRenderer::setDefaultViewport(VkCommandBuffer commandBuffer)
{
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(_swapChainExtent.width);
  viewport.height = static_cast<float>(_swapChainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = _swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanRenderer::snapshotDepthAndResume()
{
  if (!_frameStarted || !_sampledDepthImage || !_continueRenderPass)
  {
    return;
  }

  VkCommandBuffer commandBuffer = _commandBuffers[_currentImageIndex];
  vkCmdEndRenderPass(commandBuffer);
  recordDepthResolve(commandBuffer);

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = _continueRenderPass;
  renderPassInfo.framebuffer = _sceneFramebuffer;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = _swapChainExtent;
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  setDefaultViewport(commandBuffer);

  _continuedPass = true;
  _sampledDepthInitialized = true;
}

void VulkanRenderer::endFrame()
{
  if (!_frameStarted)
  {
    return;
  }

  vkCmdEndRenderPass(_commandBuffers[_currentImageIndex]);
  recordBlurAndComposite(_commandBuffers[_currentImageIndex]);

  if (!_offscreen)
  {
    VkImageMemoryBarrier presentBarrier{};
    presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    presentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    presentBarrier.dstAccessMask = 0;
    presentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentBarrier.image = _swapChainImages[_currentImageIndex];
    presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    presentBarrier.subresourceRange.levelCount = 1;
    presentBarrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(_commandBuffers[_currentImageIndex], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentBarrier);
  }

  if (vkEndCommandBuffer(_commandBuffers[_currentImageIndex]) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to record command buffer");
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkSemaphore waitSemaphores[] = {_imageAvailableSemaphore};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  if (!_offscreen)
  {
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
  }
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &_commandBuffers[_currentImageIndex];
  VkSemaphore signalSemaphores[] = {_renderFinishedSemaphore};
  if (!_offscreen)
  {
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
  }

  if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _waitFences[_currentImageIndex]) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to submit draw command buffer");
  }

  if (_offscreen)
  {
    vkWaitForFences(_device, 1, &_waitFences[_currentImageIndex], VK_TRUE, UINT64_MAX);
    const uint32_t width = _swapChainExtent.width;
    const uint32_t height = _swapChainExtent.height;
    std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);
    copyImageToHost(_swapChainImages[_currentImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _swapChainImageFormat,
                    VK_IMAGE_ASPECT_COLOR_BIT, width, height, pixels.data(), pixels.size());
    QImage image(pixels.data(), static_cast<int>(width), static_cast<int>(height), static_cast<int>(width * 4), QImage::Format_ARGB32);
    _readbackImage = image.copy();
  }
  else
  {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapChain;
    presentInfo.pImageIndices = &_currentImageIndex;

    VkResult result = vkQueuePresentKHR(_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
      recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to present swap chain image");
    }
  }

  _frameStarted = false;
}

void VulkanRenderer::bindSceneDescriptors(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t structureIndex) const
{
  const uint32_t offsets[] = {static_cast<uint32_t>(structureIndex * _structureUniformStride),
                              static_cast<uint32_t>(structureIndex * _isosurfaceUniformStride)};
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &_sceneDescriptorSet, 2, offsets);
}

void VulkanRenderer::bindSamplerDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet samplerSet) const
{
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &samplerSet, 0, nullptr);
}

void VulkanRenderer::writeHostVisible(VkDeviceMemory memory, const void *data, VkDeviceSize size)
{
  void *mapped = nullptr;
  vkMapMemory(_device, memory, 0, size, 0, &mapped);
  memcpy(mapped, data, static_cast<size_t>(size));
  vkUnmapMemory(_device, memory);
}

void VulkanRenderer::updateTransformUniforms(const RKTransformationUniforms &uniforms)
{
  writeHostVisible(_frameUniformBuffer.memory, &uniforms, sizeof(uniforms));
}

void VulkanRenderer::updateStructureUniforms(const std::vector<RKStructureUniforms> &uniforms)
{
  const size_t count = std::max<size_t>(uniforms.size(), 1);
  const VkDeviceSize needed = count * _structureUniformStride;
  if (_structureUniformBuffer.size < needed)
  {
    destroyBuffer(_structureUniformBuffer);
    createBuffer(needed, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 _structureUniformBuffer);
    createSceneDescriptorSet();
  }

  std::vector<uint8_t> padded(static_cast<size_t>(needed), 0);
  for (size_t i = 0; i < uniforms.size(); ++i)
  {
    memcpy(padded.data() + i * _structureUniformStride, &uniforms[i], sizeof(RKStructureUniforms));
  }
  writeHostVisible(_structureUniformBuffer.memory, padded.data(), needed);
}

void VulkanRenderer::updateIsosurfaceUniforms(const std::vector<RKIsosurfaceUniforms> &uniforms)
{
  const size_t count = std::max<size_t>(uniforms.size(), 1);
  const VkDeviceSize needed = count * _isosurfaceUniformStride;
  if (_isosurfaceUniformBuffer.size < needed)
  {
    destroyBuffer(_isosurfaceUniformBuffer);
    createBuffer(needed, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 _isosurfaceUniformBuffer);
    createSceneDescriptorSet();
  }

  std::vector<uint8_t> padded(static_cast<size_t>(needed), 0);
  for (size_t i = 0; i < uniforms.size(); ++i)
  {
    memcpy(padded.data() + i * _isosurfaceUniformStride, &uniforms[i], sizeof(RKIsosurfaceUniforms));
  }
  writeHostVisible(_isosurfaceUniformBuffer.memory, padded.data(), needed);
}

void VulkanRenderer::updateLightUniforms(const RKLightsUniforms &uniforms)
{
  writeHostVisible(_lightsUniformBuffer.memory, uniforms.lights.data(), uniforms.lights.size() * sizeof(RKLightUniform));
}

void VulkanRenderer::updateGlobalAxesUniforms(const RKGlobalAxesUniforms &uniforms)
{
  writeHostVisible(_axesUniformBuffer.memory, &uniforms, sizeof(uniforms));
}

void VulkanRenderer::updateRibbonAODebugUniforms(const RibbonAODebugUniforms &uniforms)
{
  writeHostVisible(_ribbonAODebugUniformBuffer.memory, &uniforms, sizeof(uniforms));
}

void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanBuffer &out)
{
  createBuffer(size, usage, properties, out.buffer, out.memory);
  out.size = size;
}

void VulkanRenderer::destroyBuffer(VulkanBuffer &buffer)
{
  if (buffer.buffer)
  {
    vkDestroyBuffer(_device, buffer.buffer, nullptr);
    buffer.buffer = VK_NULL_HANDLE;
  }
  if (buffer.memory)
  {
    vkFreeMemory(_device, buffer.memory, nullptr);
    buffer.memory = VK_NULL_HANDLE;
  }
  buffer.size = 0;
}

void VulkanRenderer::uploadBuffer(VulkanBuffer &buffer, const void *data, VkDeviceSize size, VkBufferUsageFlags usage)
{
  destroyBuffer(buffer);
  if (size == 0 || data == nullptr)
  {
    return;
  }

  VulkanBuffer staging;
  createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging);
  writeHostVisible(staging.memory, data, size);

  createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer);
  copyBuffer(staging.buffer, buffer.buffer, size);
  destroyBuffer(staging);
}

void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                                 VkDeviceMemory &bufferMemory)
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create buffer");
  }

  VkMemoryRequirements memRequirements{};
  vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
  if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate buffer memory");
  }
  vkBindBufferMemory(_device, buffer, bufferMemory, 0);
}

void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
  endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::destroyTexture(VulkanTexture &texture)
{
  if (texture.view)
  {
    vkDestroyImageView(_device, texture.view, nullptr);
    texture.view = VK_NULL_HANDLE;
  }
  if (texture.image)
  {
    vkDestroyImage(_device, texture.image, nullptr);
    texture.image = VK_NULL_HANDLE;
  }
  if (texture.memory)
  {
    vkFreeMemory(_device, texture.memory, nullptr);
    texture.memory = VK_NULL_HANDLE;
  }
  texture.width = 0;
  texture.height = 0;
  texture.depth = 1;
  texture.layers = 1;
}

VulkanTexture VulkanRenderer::createTextureRGBA8(const QImage &image)
{
  QImage converted = image.convertToFormat(QImage::Format_RGBA8888);
  if (converted.isNull() || converted.width() <= 0 || converted.height() <= 0)
  {
    converted = QImage(64, 64, QImage::Format_RGBA8888);
    converted.fill(QColor(255, 255, 255, 255));
  }
  VulkanTexture texture;
  texture.width = static_cast<uint32_t>(converted.width());
  texture.height = static_cast<uint32_t>(converted.height());
  const int rowBytes = converted.width() * 4;
  const VkDeviceSize imageSize = static_cast<VkDeviceSize>(rowBytes) * static_cast<VkDeviceSize>(converted.height());
  std::vector<uint8_t> packed(static_cast<size_t>(imageSize));
  for (int y = 0; y < converted.height(); ++y)
  {
    memcpy(packed.data() + static_cast<size_t>(y * rowBytes), converted.constScanLine(y), static_cast<size_t>(rowBytes));
  }

  VulkanBuffer staging;
  createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging);
  writeHostVisible(staging.memory, packed.data(), imageSize);

  createImage(texture.width, texture.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image, texture.memory);
  transitionImageLayout(texture.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(staging.buffer, texture.image, texture.width, texture.height);
  transitionImageLayout(texture.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  texture.view = createImageView(texture.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
  destroyBuffer(staging);
  return texture;
}

VulkanTexture VulkanRenderer::createTextureR16F(uint32_t width, uint32_t height, const void *data)
{
  VulkanTexture texture;
  texture.width = width;
  texture.height = height;
  const VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * sizeof(uint16_t);
  createImage(width, height, VK_FORMAT_R16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image,
              texture.memory);
  if (data && imageSize > 0)
  {
    VulkanBuffer staging;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging);
    writeHostVisible(staging.memory, data, imageSize);
    transitionImageLayout(texture.image, VK_FORMAT_R16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(staging.buffer, texture.image, width, height);
    transitionImageLayout(texture.image, VK_FORMAT_R16_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    destroyBuffer(staging);
  }
  texture.view = createImageView(texture.image, VK_FORMAT_R16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
  return texture;
}

VkDescriptorSet VulkanRenderer::allocateSamplerDescriptorSet(const VulkanTexture &texture)
{
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &_samplerDescriptorSetLayout;

  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
  if (vkAllocateDescriptorSets(_device, &allocInfo, &descriptorSet) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate sampler descriptor set");
  }

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = texture.view;
  imageInfo.sampler = _linearSampler;

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = descriptorSet;
  write.dstBinding = 0;
  write.dstArrayElement = 0;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.descriptorCount = 1;
  write.pImageInfo = &imageInfo;
  vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);
  return descriptorSet;
}

VulkanTexture VulkanRenderer::createTexture3D_RGBA32F(uint32_t width, uint32_t height, uint32_t depth, const void *data)
{
  VulkanTexture texture;
  texture.width = width;
  texture.height = height;
  texture.depth = depth;
  texture.layers = 1;
  const VkDeviceSize imageSize =
      static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * static_cast<VkDeviceSize>(depth) * sizeof(float) * 4;

  VulkanBuffer staging;
  createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               staging);
  writeHostVisible(staging.memory, data, imageSize);

  createImage(width, height, depth, 1, VK_IMAGE_TYPE_3D, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image,
              texture.memory);
  transitionImageLayout(texture.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(staging.buffer, texture.image, width, height, depth, 1);
  transitionImageLayout(texture.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  texture.view = createImageView(texture.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_3D, 1);
  destroyBuffer(staging);
  return texture;
}

VulkanTexture VulkanRenderer::createTexture2DArray_RGBA32F(uint32_t width, uint32_t layers, const void *data)
{
  VulkanTexture texture;
  texture.width = width;
  texture.height = 1;
  texture.depth = 1;
  texture.layers = layers;
  const VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(layers) * sizeof(float) * 4;

  VulkanBuffer staging;
  createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               staging);
  writeHostVisible(staging.memory, data, imageSize);

  createImage(width, 1, 1, layers, VK_IMAGE_TYPE_2D, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image,
              texture.memory);
  transitionImageLayout(texture.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        layers);
  copyBufferToImage(staging.buffer, texture.image, width, 1, 1, layers);
  transitionImageLayout(texture.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layers);
  texture.view =
      createImageView(texture.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY, layers);
  destroyBuffer(staging);
  return texture;
}

VkDescriptorSet VulkanRenderer::allocateVolumeDescriptorSet(const VulkanTexture &volume, VkImageView depthView,
                                                            const VulkanTexture &transferFunction)
{
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &_volumeDescriptorSetLayout;

  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
  if (vkAllocateDescriptorSets(_device, &allocInfo, &descriptorSet) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate volume descriptor set");
  }
  updateVolumeDescriptorSet(descriptorSet, volume, depthView, transferFunction);
  return descriptorSet;
}

void VulkanRenderer::updateVolumeDescriptorSet(VkDescriptorSet descriptorSet, const VulkanTexture &volume, VkImageView depthView,
                                               const VulkanTexture &transferFunction)
{
  VkDescriptorImageInfo volumeInfo{};
  volumeInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  volumeInfo.imageView = volume.view;
  volumeInfo.sampler = _repeatSampler;

  VkDescriptorImageInfo depthInfo{};
  depthInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  depthInfo.imageView = depthView ? depthView : _whiteTexture.view;
  depthInfo.sampler = _nearestSampler;

  VkDescriptorImageInfo transferInfo{};
  transferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  transferInfo.imageView = transferFunction.view;
  transferInfo.sampler = _linearSampler;

  auto write = [&](uint32_t binding, const VkDescriptorImageInfo *info) {
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = info;
    return descriptorWrite;
  };

  std::array<VkWriteDescriptorSet, 3> writes = {write(0, &volumeInfo), write(1, &depthInfo), write(2, &transferInfo)};
  vkUpdateDescriptorSets(_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void VulkanRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth,
                                       uint32_t layerCount)
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  VkBufferImageCopy region{};
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.layerCount = layerCount;
  region.imageExtent = {width, height, depth};
  vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::setupDebugCallback()
{
  if (!enableValidationLayers)
  {
    return;
  }
  VkDebugReportCallbackCreateInfoEXT createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
  createInfo.pfnCallback = debugCallback;
  if (CreateDebugReportCallbackEXT(_instance, &createInfo, nullptr, &_callback) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to set up debug callback");
  }
}

void VulkanRenderer::pickPhysicalDevice()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
  if (deviceCount == 0)
  {
    throw std::runtime_error("failed to find GPUs with Vulkan support");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());
  for (const auto &candidate : devices)
  {
    if (isDeviceSuitable(candidate))
    {
      _physicalDevice = candidate;
      break;
    }
  }
  if (_physicalDevice == VK_NULL_HANDLE)
  {
    throw std::runtime_error("failed to find a suitable GPU");
  }

#ifdef __APPLE__
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, availableExtensions.data());
  for (const auto &extension : availableExtensions)
  {
    if (strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0)
    {
      deviceExtensions.push_back("VK_KHR_portability_subset");
      break;
    }
  }
#endif
}

void VulkanRenderer::createLogicalDevice()
{
  QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};
  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures supportedFeatures{};
  vkGetPhysicalDeviceFeatures(_physicalDevice, &supportedFeatures);
  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.depthClamp = supportedFeatures.depthClamp;
  deviceFeatures.independentBlend = supportedFeatures.independentBlend;
  deviceFeatures.sampleRateShading = supportedFeatures.sampleRateShading;
  _sampleRateShading = supportedFeatures.sampleRateShading == VK_TRUE;
  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();
  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }

  if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create logical device");
  }
  vkGetDeviceQueue(_device, indices.graphicsFamily, 0, &_graphicsQueue);
  vkGetDeviceQueue(_device, indices.presentFamily, 0, &_presentQueue);
}

void VulkanRenderer::createSurface()
{
#ifdef VK_USE_PLATFORM_METAL_EXT
  VkMetalSurfaceCreateInfoEXT createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
  createInfo.pLayer = static_cast<const CAMetalLayer *>(_metalLayer);
  auto createMetalSurface = reinterpret_cast<PFN_vkCreateMetalSurfaceEXT>(vkGetInstanceProcAddr(_instance, "vkCreateMetalSurfaceEXT"));
  if (!createMetalSurface || createMetalSurface(_instance, &createInfo, nullptr, &_surface) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create Metal surface");
  }
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
  VkMacOSSurfaceCreateInfoMVK surfaceInfo{};
  surfaceInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
  surfaceInfo.pView = reinterpret_cast<void *>(_window->winId());
  if (vkCreateMacOSSurfaceMVK(_instance, &surfaceInfo, nullptr, &_surface) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create macOS surface");
  }
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hwnd = reinterpret_cast<HWND>(_window->winId());
  createInfo.hinstance = GetModuleHandle(nullptr);
  if (vkCreateWin32SurfaceKHR(_instance, &createInfo, nullptr, &_surface) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create Win32 surface");
  }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
  auto *x11 = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
  if (!x11)
  {
    throw std::runtime_error("X11 native interface is unavailable");
  }
  VkXcbSurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  createInfo.connection = x11->connection();
  createInfo.window = static_cast<xcb_window_t>(_window->winId());
  if (vkCreateXcbSurfaceKHR(_instance, &createInfo, nullptr, &_surface) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create XCB surface");
  }
#else
  throw std::runtime_error("Vulkan on Linux requires Qt 6.2 or newer");
#endif
#else
  throw std::runtime_error("Vulkan surface platform is not configured");
#endif
}

void VulkanRenderer::createOffscreenTarget()
{
  _swapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  _swapChainExtent = _offscreenExtent;
  if (_swapChainExtent.width == 0)
  {
    _swapChainExtent.width = 1;
  }
  if (_swapChainExtent.height == 0)
  {
    _swapChainExtent.height = 1;
  }
  _offscreenColor = createAttachmentTexture(_swapChainExtent.width, _swapChainExtent.height, _swapChainImageFormat,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                            VK_IMAGE_ASPECT_COLOR_BIT);
  _swapChainImages = {_offscreenColor.image};
  _swapChainImageViews = {_offscreenColor.view};
}

void VulkanRenderer::recreateOffscreenTargets()
{
  vkDeviceWaitIdle(_device);
  cleanupSwapChain(false);
  createOffscreenTarget();
  createDepthResources();
  createSceneColorResources();
  createSceneFramebuffer();
  createBlurResources();
  createCompositeFramebuffers();
  createPickFramebuffer();
  updatePostProcessDescriptors();
  createCommandBuffers();
  if (_waitFences.size() != _commandBuffers.size())
  {
    for (auto fence : _waitFences)
    {
      vkDestroyFence(_device, fence, nullptr);
    }
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    _waitFences.resize(_commandBuffers.size());
    for (auto &fence : _waitFences)
    {
      if (vkCreateFence(_device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create fences");
      }
    }
  }
}

void VulkanRenderer::createSwapChain()
{
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice);
  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = _surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};
  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create swap chain");
  }

  vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
  _swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());
  _swapChainImageFormat = surfaceFormat.format;
  _swapChainExtent = extent;
}

void VulkanRenderer::createImageViews()
{
  _swapChainImageViews.resize(_swapChainImages.size());
  for (size_t i = 0; i < _swapChainImages.size(); ++i)
  {
    _swapChainImageViews[i] = createImageView(_swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void VulkanRenderer::createRenderPass()
{
  VkAttachmentDescription sceneColor{};
  sceneColor.format = _sceneColorFormat;
  sceneColor.samples = _msaaSamples;
  sceneColor.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  sceneColor.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  sceneColor.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  sceneColor.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  sceneColor.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  sceneColor.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription glowColor = sceneColor;
  glowColor.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = _msaaSamples;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  std::array<VkAttachmentReference, 2> colorRefs{};
  colorRefs[0].attachment = 0;
  colorRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorRefs[1].attachment = 1;
  colorRefs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 2;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 2;
  subpass.pColorAttachments = colorRefs.data();
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 3> attachments = {sceneColor, glowColor, depthAttachment};
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;
  if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create render pass");
  }
}

void VulkanRenderer::createContinueRenderPass()
{
  VkAttachmentDescription sceneColor{};
  sceneColor.format = _sceneColorFormat;
  sceneColor.samples = _msaaSamples;
  sceneColor.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  sceneColor.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  sceneColor.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  sceneColor.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  sceneColor.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  sceneColor.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription glowColor = sceneColor;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = _msaaSamples;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  std::array<VkAttachmentReference, 2> colorRefs{};
  colorRefs[0].attachment = 0;
  colorRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorRefs[1].attachment = 1;
  colorRefs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 2;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 2;
  subpass.pColorAttachments = colorRefs.data();
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_SHADER_READ_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                             VK_ACCESS_SHADER_READ_BIT;

  std::array<VkAttachmentDescription, 3> attachments = {sceneColor, glowColor, depthAttachment};
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;
  if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_continueRenderPass) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create continue render pass");
  }
}

void VulkanRenderer::createPickRenderPass()
{
  _pickColorFormat = findPickColorFormat();

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = _pickColorFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;
  if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_pickRenderPass) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create pick render pass");
  }

  const auto hostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  createBuffer(sizeof(uint32_t) * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, hostVisible, _pickReadbackBuffer);
  createBuffer(sizeof(float), VK_BUFFER_USAGE_TRANSFER_DST_BIT, hostVisible, _pickDepthReadbackBuffer);
}

void VulkanRenderer::chooseMsaaSampleCount()
{
  _sceneColorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
  VkFormatProperties formatProps{};
  vkGetPhysicalDeviceFormatProperties(_physicalDevice, _sceneColorFormat, &formatProps);
  const VkFormatFeatureFlags neededColor =
      VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
  if ((formatProps.optimalTilingFeatures & neededColor) != neededColor)
  {
    _sceneColorFormat = _swapChainImageFormat;
  }

  auto sampleCountsFor = [&](VkFormat format, VkImageUsageFlags usage) {
    VkImageFormatProperties imageProps{};
    if (vkGetPhysicalDeviceImageFormatProperties(_physicalDevice, format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0,
                                                 &imageProps) != VK_SUCCESS)
    {
      return static_cast<VkSampleCountFlags>(VK_SAMPLE_COUNT_1_BIT);
    }
    return imageProps.sampleCounts;
  };

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(_physicalDevice, &properties);
  const VkSampleCountFlags supported =
      properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts &
      sampleCountsFor(_sceneColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) &
      sampleCountsFor(findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

  const VkSampleCountFlagBits candidates[] = {VK_SAMPLE_COUNT_8_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_2_BIT};
  _msaaSamples = VK_SAMPLE_COUNT_1_BIT;
  for (VkSampleCountFlagBits candidate : candidates)
  {
    if (supported & candidate)
    {
      _msaaSamples = candidate;
      break;
    }
  }
  qDebug() << "Vulkan MSAA sample count:" << static_cast<int>(_msaaSamples);
}

void VulkanRenderer::createDepthResolveRenderPass()
{
  VkAttachmentDescription depth{};
  depth.format = findDepthFormat();
  depth.samples = VK_SAMPLE_COUNT_1_BIT;
  depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  VkAttachmentReference depthRef{};
  depthRef.attachment = 0;
  depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.pDepthStencilAttachment = &depthRef;
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  VkRenderPassCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  info.attachmentCount = 1;
  info.pAttachments = &depth;
  info.subpassCount = 1;
  info.pSubpasses = &subpass;
  info.dependencyCount = 1;
  info.pDependencies = &dependency;
  if (vkCreateRenderPass(_device, &info, nullptr, &_depthResolveRenderPass) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create depth resolve render pass");
  }
}

void VulkanRenderer::createBlurRenderPass()
{
  VkAttachmentDescription color{};
  color.format = _sceneColorFormat;
  color.samples = VK_SAMPLE_COUNT_1_BIT;
  color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  VkAttachmentReference colorRef{};
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorRef;
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  VkRenderPassCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  info.attachmentCount = 1;
  info.pAttachments = &color;
  info.subpassCount = 1;
  info.pSubpasses = &subpass;
  info.dependencyCount = 1;
  info.pDependencies = &dependency;
  if (vkCreateRenderPass(_device, &info, nullptr, &_blurRenderPass) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create blur render pass");
  }
}

void VulkanRenderer::createCompositeRenderPass()
{
  VkAttachmentDescription color{};
  color.format = _swapChainImageFormat;
  color.samples = VK_SAMPLE_COUNT_1_BIT;
  color.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkAttachmentReference colorRef{};
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorRef;
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  VkRenderPassCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  info.attachmentCount = 1;
  info.pAttachments = &color;
  info.subpassCount = 1;
  info.pSubpasses = &subpass;
  info.dependencyCount = 1;
  info.pDependencies = &dependency;
  if (vkCreateRenderPass(_device, &info, nullptr, &_compositeRenderPass) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create composite render pass");
  }
}

void VulkanRenderer::destroyPickFramebuffer()
{
  if (_pickFramebuffer)
  {
    vkDestroyFramebuffer(_device, _pickFramebuffer, nullptr);
    _pickFramebuffer = VK_NULL_HANDLE;
  }
  if (_pickColorView)
  {
    vkDestroyImageView(_device, _pickColorView, nullptr);
    _pickColorView = VK_NULL_HANDLE;
  }
  if (_pickColorImage)
  {
    vkDestroyImage(_device, _pickColorImage, nullptr);
    _pickColorImage = VK_NULL_HANDLE;
  }
  if (_pickColorMemory)
  {
    vkFreeMemory(_device, _pickColorMemory, nullptr);
    _pickColorMemory = VK_NULL_HANDLE;
  }
  if (_pickDepthView)
  {
    vkDestroyImageView(_device, _pickDepthView, nullptr);
    _pickDepthView = VK_NULL_HANDLE;
  }
  if (_pickDepthImage)
  {
    vkDestroyImage(_device, _pickDepthImage, nullptr);
    _pickDepthImage = VK_NULL_HANDLE;
  }
  if (_pickDepthMemory)
  {
    vkFreeMemory(_device, _pickDepthMemory, nullptr);
    _pickDepthMemory = VK_NULL_HANDLE;
  }
}

void VulkanRenderer::createPickFramebuffer()
{
  destroyPickFramebuffer();
  if (_swapChainExtent.width == 0 || _swapChainExtent.height == 0 || !_pickRenderPass)
  {
    return;
  }

  const VkFormat depthFormat = findDepthFormat();
  createImage(_swapChainExtent.width, _swapChainExtent.height, _pickColorFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              _pickColorImage, _pickColorMemory);
  _pickColorView = createImageView(_pickColorImage, _pickColorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

  createImage(_swapChainExtent.width, _swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              _pickDepthImage, _pickDepthMemory);
  const VkImageAspectFlags pickDepthAspect =
      hasStencilComponent(depthFormat) ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT;
  _pickDepthView = createImageView(_pickDepthImage, depthFormat, pickDepthAspect);

  std::array<VkImageView, 2> attachments = {_pickColorView, _pickDepthView};
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = _pickRenderPass;
  framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  framebufferInfo.pAttachments = attachments.data();
  framebufferInfo.width = _swapChainExtent.width;
  framebufferInfo.height = _swapChainExtent.height;
  framebufferInfo.layers = 1;
  if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_pickFramebuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create pick framebuffer");
  }
}

VkCommandBuffer VulkanRenderer::beginPickPass()
{
  waitIdle();
  if (!_pickFramebuffer || !_pickRenderPass || _swapChainExtent.width == 0 || _swapChainExtent.height == 0)
  {
    return VK_NULL_HANDLE;
  }

  _pickCommandBuffer = beginSingleTimeCommands();

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = _pickRenderPass;
  renderPassInfo.framebuffer = _pickFramebuffer;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = _swapChainExtent;

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color.uint32[0] = 0;
  clearValues[0].color.uint32[1] = 0;
  clearValues[0].color.uint32[2] = 0;
  clearValues[0].color.uint32[3] = 0;
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(_pickCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(_swapChainExtent.width);
  viewport.height = static_cast<float>(_swapChainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(_pickCommandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = _swapChainExtent;
  vkCmdSetScissor(_pickCommandBuffer, 0, 1, &scissor);

  return _pickCommandBuffer;
}

std::array<int, 4> VulkanRenderer::endPickPassAndReadPixel(int x, int y, float *outDepth)
{
  if (!_pickCommandBuffer)
  {
    return {0, 0, 0, 0};
  }

  vkCmdEndRenderPass(_pickCommandBuffer);

  const int width = static_cast<int>(_swapChainExtent.width);
  const int height = static_cast<int>(_swapChainExtent.height);
  if (x < 0 || y < 0 || x >= width || y >= height || !_pickReadbackBuffer.buffer)
  {
    endSingleTimeCommands(_pickCommandBuffer);
    _pickCommandBuffer = VK_NULL_HANDLE;
    return {0, 0, 0, 0};
  }

  VkImageMemoryBarrier colorBarrier{};
  colorBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  colorBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  colorBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  colorBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  colorBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  colorBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  colorBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  colorBarrier.image = _pickColorImage;
  colorBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  colorBarrier.subresourceRange.levelCount = 1;
  colorBarrier.subresourceRange.layerCount = 1;
  vkCmdPipelineBarrier(_pickCommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &colorBarrier);

  VkBufferImageCopy colorRegion{};
  colorRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  colorRegion.imageSubresource.layerCount = 1;
  colorRegion.imageOffset = {x, y, 0};
  colorRegion.imageExtent = {1, 1, 1};
  vkCmdCopyImageToBuffer(_pickCommandBuffer, _pickColorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _pickReadbackBuffer.buffer, 1, &colorRegion);

  if (outDepth && _pickDepthReadbackBuffer.buffer && _pickDepthImage)
  {
    VkImageMemoryBarrier depthBarrier{};
    depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    depthBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    depthBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    depthBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthBarrier.image = _pickDepthImage;
    depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthBarrier.subresourceRange.levelCount = 1;
    depthBarrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(_pickCommandBuffer, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &depthBarrier);

    VkBufferImageCopy depthRegion{};
    depthRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthRegion.imageSubresource.layerCount = 1;
    depthRegion.imageOffset = {x, y, 0};
    depthRegion.imageExtent = {1, 1, 1};
    vkCmdCopyImageToBuffer(_pickCommandBuffer, _pickDepthImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _pickDepthReadbackBuffer.buffer, 1,
                           &depthRegion);
  }

  VkMemoryBarrier hostBarrier{};
  hostBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  hostBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  hostBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
  vkCmdPipelineBarrier(_pickCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &hostBarrier, 0, nullptr, 0,
                       nullptr);

  endSingleTimeCommands(_pickCommandBuffer);
  _pickCommandBuffer = VK_NULL_HANDLE;

  uint32_t pixel[4] = {0, 0, 0, 0};
  void *mapped = nullptr;
  if (vkMapMemory(_device, _pickReadbackBuffer.memory, 0, sizeof(pixel), 0, &mapped) == VK_SUCCESS && mapped)
  {
    std::memcpy(pixel, mapped, sizeof(pixel));
    vkUnmapMemory(_device, _pickReadbackBuffer.memory);
  }

  if (outDepth)
  {
    *outDepth = 1.0f;
    if (pixel[0] >= 1 && _pickDepthReadbackBuffer.memory)
    {
      void *depthMapped = nullptr;
      if (vkMapMemory(_device, _pickDepthReadbackBuffer.memory, 0, sizeof(float), 0, &depthMapped) == VK_SUCCESS && depthMapped)
      {
        std::memcpy(outDepth, depthMapped, sizeof(float));
        vkUnmapMemory(_device, _pickDepthReadbackBuffer.memory);
      }
    }
  }

  return {static_cast<int>(pixel[0]), static_cast<int>(pixel[1]), static_cast<int>(pixel[2]), static_cast<int>(pixel[3])};
}

void VulkanRenderer::createDescriptorSetLayouts()
{
  VkDescriptorSetLayoutBinding frameBinding{};
  frameBinding.binding = 0;
  frameBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  frameBinding.descriptorCount = 1;
  frameBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding structureBinding = frameBinding;
  structureBinding.binding = 1;
  structureBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

  VkDescriptorSetLayoutBinding isosurfaceBinding = structureBinding;
  isosurfaceBinding.binding = 2;

  VkDescriptorSetLayoutBinding lightsBinding = frameBinding;
  lightsBinding.binding = 3;

  VkDescriptorSetLayoutBinding ribbonAOBinding = frameBinding;
  ribbonAOBinding.binding = 4;

  VkDescriptorSetLayoutBinding axesBinding = frameBinding;
  axesBinding.binding = 5;

  std::array<VkDescriptorSetLayoutBinding, 6> sceneBindings = {frameBinding, structureBinding, isosurfaceBinding, lightsBinding,
                                                               ribbonAOBinding, axesBinding};
  VkDescriptorSetLayoutCreateInfo sceneInfo{};
  sceneInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  sceneInfo.bindingCount = static_cast<uint32_t>(sceneBindings.size());
  sceneInfo.pBindings = sceneBindings.data();
  if (vkCreateDescriptorSetLayout(_device, &sceneInfo, nullptr, &_sceneDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create scene descriptor set layout");
  }

  VkDescriptorSetLayoutBinding samplerBinding{};
  samplerBinding.binding = 0;
  samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerBinding.descriptorCount = 1;
  samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  samplerInfo.bindingCount = 1;
  samplerInfo.pBindings = &samplerBinding;
  if (vkCreateDescriptorSetLayout(_device, &samplerInfo, nullptr, &_samplerDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create sampler descriptor set layout");
  }

  VkDescriptorSetLayoutBinding volumeBinding{};
  volumeBinding.binding = 0;
  volumeBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  volumeBinding.descriptorCount = 1;
  volumeBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  VkDescriptorSetLayoutBinding depthBinding = volumeBinding;
  depthBinding.binding = 1;
  VkDescriptorSetLayoutBinding transferBinding = volumeBinding;
  transferBinding.binding = 2;
  std::array<VkDescriptorSetLayoutBinding, 3> volumeBindings = {volumeBinding, depthBinding, transferBinding};
  VkDescriptorSetLayoutCreateInfo volumeInfo{};
  volumeInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  volumeInfo.bindingCount = static_cast<uint32_t>(volumeBindings.size());
  volumeInfo.pBindings = volumeBindings.data();
  if (vkCreateDescriptorSetLayout(_device, &volumeInfo, nullptr, &_volumeDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create volume descriptor set layout");
  }

  if (vkCreateDescriptorSetLayout(_device, &samplerInfo, nullptr, &_msaaSamplerDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create MSAA sampler descriptor set layout");
  }

  VkDescriptorSetLayoutBinding sceneSamplerBinding = samplerBinding;
  VkDescriptorSetLayoutBinding blurSamplerBinding = samplerBinding;
  blurSamplerBinding.binding = 1;
  std::array<VkDescriptorSetLayoutBinding, 2> compositeBindings = {sceneSamplerBinding, blurSamplerBinding};
  VkDescriptorSetLayoutCreateInfo compositeInfo{};
  compositeInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  compositeInfo.bindingCount = static_cast<uint32_t>(compositeBindings.size());
  compositeInfo.pBindings = compositeBindings.data();
  if (vkCreateDescriptorSetLayout(_device, &compositeInfo, nullptr, &_compositeDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create composite descriptor set layout");
  }
}

void VulkanRenderer::createPipelineLayouts()
{
  VkDescriptorSetLayout sceneLayouts[] = {_sceneDescriptorSetLayout, _samplerDescriptorSetLayout};
  VkPipelineLayoutCreateInfo sceneInfo{};
  sceneInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  sceneInfo.setLayoutCount = 2;
  sceneInfo.pSetLayouts = sceneLayouts;
  if (vkCreatePipelineLayout(_device, &sceneInfo, nullptr, &_scenePipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create scene pipeline layout");
  }

  VkDescriptorSetLayout backgroundLayouts[] = {_samplerDescriptorSetLayout};
  VkPipelineLayoutCreateInfo backgroundInfo{};
  backgroundInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  backgroundInfo.setLayoutCount = 1;
  backgroundInfo.pSetLayouts = backgroundLayouts;
  if (vkCreatePipelineLayout(_device, &backgroundInfo, nullptr, &_backgroundPipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create background pipeline layout");
  }

  VkDescriptorSetLayout volumeLayouts[] = {_sceneDescriptorSetLayout, _volumeDescriptorSetLayout};
  VkPipelineLayoutCreateInfo volumeInfo{};
  volumeInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  volumeInfo.setLayoutCount = 2;
  volumeInfo.pSetLayouts = volumeLayouts;
  if (vkCreatePipelineLayout(_device, &volumeInfo, nullptr, &_volumePipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create volume pipeline layout");
  }

  VkPipelineLayoutCreateInfo msaaLayoutInfo{};
  msaaLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  msaaLayoutInfo.setLayoutCount = 1;
  msaaLayoutInfo.pSetLayouts = &_msaaSamplerDescriptorSetLayout;
  if (vkCreatePipelineLayout(_device, &msaaLayoutInfo, nullptr, &_msaaSamplerPipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create MSAA sampler pipeline layout");
  }

  VkPipelineLayoutCreateInfo samplerLayoutInfo{};
  samplerLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  samplerLayoutInfo.setLayoutCount = 1;
  samplerLayoutInfo.pSetLayouts = &_samplerDescriptorSetLayout;
  if (vkCreatePipelineLayout(_device, &samplerLayoutInfo, nullptr, &_samplerPipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create sampler pipeline layout");
  }

  VkDescriptorSetLayout compositeLayouts[] = {_sceneDescriptorSetLayout, _compositeDescriptorSetLayout};
  VkPipelineLayoutCreateInfo compositeLayoutInfo{};
  compositeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  compositeLayoutInfo.setLayoutCount = 2;
  compositeLayoutInfo.pSetLayouts = compositeLayouts;
  if (vkCreatePipelineLayout(_device, &compositeLayoutInfo, nullptr, &_compositePipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create composite pipeline layout");
  }

  VkPushConstantRange overlayPush{};
  overlayPush.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  overlayPush.offset = 0;
  overlayPush.size = 32;
  VkPipelineLayoutCreateInfo overlayLayoutInfo{};
  overlayLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  overlayLayoutInfo.pushConstantRangeCount = 1;
  overlayLayoutInfo.pPushConstantRanges = &overlayPush;
  if (vkCreatePipelineLayout(_device, &overlayLayoutInfo, nullptr, &_overlayPipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create overlay pipeline layout");
  }
}

void VulkanRenderer::createSceneFramebuffer()
{
  if (_sceneFramebuffer)
  {
    vkDestroyFramebuffer(_device, _sceneFramebuffer, nullptr);
    _sceneFramebuffer = VK_NULL_HANDLE;
  }
  std::array<VkImageView, 3> attachments = {_sceneColor.view, _glowColor.view, _depthImageView};
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = _renderPass;
  framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  framebufferInfo.pAttachments = attachments.data();
  framebufferInfo.width = _swapChainExtent.width;
  framebufferInfo.height = _swapChainExtent.height;
  framebufferInfo.layers = 1;
  if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_sceneFramebuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create scene framebuffer");
  }

  if (_sampledDepthFramebuffer)
  {
    vkDestroyFramebuffer(_device, _sampledDepthFramebuffer, nullptr);
    _sampledDepthFramebuffer = VK_NULL_HANDLE;
  }
  VkFramebufferCreateInfo depthFramebufferInfo{};
  depthFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  depthFramebufferInfo.renderPass = _depthResolveRenderPass;
  depthFramebufferInfo.attachmentCount = 1;
  depthFramebufferInfo.pAttachments = &_sampledDepthAttachmentView;
  depthFramebufferInfo.width = _swapChainExtent.width;
  depthFramebufferInfo.height = _swapChainExtent.height;
  depthFramebufferInfo.layers = 1;
  if (vkCreateFramebuffer(_device, &depthFramebufferInfo, nullptr, &_sampledDepthFramebuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create sampled depth framebuffer");
  }
}

void VulkanRenderer::createCompositeFramebuffers()
{
  for (auto framebuffer : _compositeFramebuffers)
  {
    vkDestroyFramebuffer(_device, framebuffer, nullptr);
  }
  _compositeFramebuffers.resize(_swapChainImageViews.size());
  for (size_t i = 0; i < _swapChainImageViews.size(); ++i)
  {
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = _compositeRenderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &_swapChainImageViews[i];
    framebufferInfo.width = _swapChainExtent.width;
    framebufferInfo.height = _swapChainExtent.height;
    framebufferInfo.layers = 1;
    if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_compositeFramebuffers[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create composite framebuffer");
    }
  }
}

void VulkanRenderer::createCommandPool()
{
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_physicalDevice);
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create command pool");
  }
}

VkFormat VulkanRenderer::findDepthFormat()
{
  // prefer combined depth/stencil formats: the stencil aspect is used for the
  // box-cap passes that close off bonds clipped at the unit cell
  return findSupportedFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT}, VK_IMAGE_TILING_OPTIMAL,
                             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
}

bool VulkanRenderer::sceneDepthHasStencil()
{
  return hasStencilComponent(_depthFormat != VK_FORMAT_UNDEFINED ? _depthFormat : findDepthFormat());
}

VkFormat VulkanRenderer::findPickColorFormat()
{
  return findSupportedFormat({VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SINT}, VK_IMAGE_TILING_OPTIMAL,
                             VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
}

VkFormat VulkanRenderer::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
  for (VkFormat format : candidates)
  {
    VkFormatProperties props{};
    vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
    {
      return format;
    }
    if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
    {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format");
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType,
                                            uint32_t layerCount)
{
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = viewType;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.layerCount = layerCount;
  VkImageView imageView;
  if (vkCreateImageView(_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create image view");
  }
  return imageView;
}

void VulkanRenderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory,
                                VkSampleCountFlagBits samples)
{
  createImage(width, height, 1, 1, VK_IMAGE_TYPE_2D, format, tiling, usage, properties, image, imageMemory, samples);
}

void VulkanRenderer::createImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t arrayLayers, VkImageType imageType,
                                VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                VkImage &image, VkDeviceMemory &imageMemory, VkSampleCountFlagBits samples)
{
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = imageType;
  imageInfo.extent = {width, height, depth};
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = arrayLayers;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = samples;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create image");
  }

  VkMemoryRequirements memRequirements{};
  vkGetImageMemoryRequirements(_device, image, &memRequirements);
  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
  if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate image memory");
  }
  vkBindImageMemory(_device, image, imageMemory, 0);
}

VkCommandBuffer VulkanRenderer::beginSingleTimeCommands()
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = _commandPool;
  allocInfo.commandBufferCount = 1;
  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
  vkEndCommandBuffer(commandBuffer);
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(_graphicsQueue);
  vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

VkCommandBuffer VulkanRenderer::beginOneTimeCommands()
{
  return beginSingleTimeCommands();
}

void VulkanRenderer::submitOneTimeCommands(VkCommandBuffer commandBuffer)
{
  endSingleTimeCommands(commandBuffer);
}

VulkanTexture VulkanRenderer::createAttachmentTexture(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                                                      VkImageAspectFlags aspect, VkSampleCountFlagBits samples)
{
  VulkanTexture texture;
  texture.width = width;
  texture.height = height;
  createImage(width, height, format, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image,
              texture.memory, samples);
  texture.view = createImageView(texture.image, format, aspect);
  return texture;
}

void VulkanRenderer::copyImageToHost(VkImage image, VkImageLayout srcLayout, VkFormat format, VkImageAspectFlags aspect,
                                     uint32_t width, uint32_t height, void *dst, VkDeviceSize size)
{
  VulkanBuffer staging;
  createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging);

  if (srcLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
    transitionImageLayout(image, format, srcLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  }

  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  VkBufferImageCopy region{};
  region.imageSubresource.aspectMask = aspect;
  region.imageSubresource.layerCount = 1;
  region.imageExtent = {width, height, 1};
  vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging.buffer, 1, &region);
  endSingleTimeCommands(commandBuffer);

  void *mapped = nullptr;
  if (vkMapMemory(_device, staging.memory, 0, size, 0, &mapped) == VK_SUCCESS && mapped)
  {
    memcpy(dst, mapped, static_cast<size_t>(size));
    vkUnmapMemory(_device, staging.memory);
  }
  destroyBuffer(staging);
}

bool VulkanRenderer::hasStencilComponent(VkFormat format)
{
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanRenderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                           uint32_t layerCount)
{
  auto layoutAccess = [](VkImageLayout layout, VkAccessFlags &access, VkPipelineStageFlags &stage) {
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      access = 0;
      stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      access = VK_ACCESS_TRANSFER_WRITE_BIT;
      stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      access = VK_ACCESS_TRANSFER_READ_BIT;
      stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      access = VK_ACCESS_SHADER_READ_BIT;
      stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      access = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      break;
    default:
      throw std::invalid_argument("unsupported layout transition");
    }
  };

  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.layerCount = layerCount;

  const bool depthTransition = newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                               oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                               format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                               format == VK_FORMAT_D24_UNORM_S8_UINT;
  if (depthTransition)
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (hasStencilComponent(format))
    {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  }
  else
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  layoutAccess(oldLayout, barrier.srcAccessMask, sourceStage);
  layoutAccess(newLayout, barrier.dstAccessMask, destinationStage);

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
  endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::destroySampledDepth()
{
  if (_sampledDepthView)
  {
    vkDestroyImageView(_device, _sampledDepthView, nullptr);
    _sampledDepthView = VK_NULL_HANDLE;
  }
  if (_sampledDepthAttachmentView)
  {
    vkDestroyImageView(_device, _sampledDepthAttachmentView, nullptr);
    _sampledDepthAttachmentView = VK_NULL_HANDLE;
  }
  if (_sampledDepthImage)
  {
    vkDestroyImage(_device, _sampledDepthImage, nullptr);
    _sampledDepthImage = VK_NULL_HANDLE;
  }
  if (_sampledDepthMemory)
  {
    vkFreeMemory(_device, _sampledDepthMemory, nullptr);
    _sampledDepthMemory = VK_NULL_HANDLE;
  }
  _sampledDepthInitialized = false;
}

void VulkanRenderer::createDepthResources()
{
  _depthFormat = findDepthFormat();
  // attachment views must cover all aspects of the format; sampled views must select a single aspect
  const VkImageAspectFlags attachmentAspect =
      hasStencilComponent(_depthFormat) ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT;
  createImage(_swapChainExtent.width, _swapChainExtent.height, _depthFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              _depthImage, _depthImageMemory, _msaaSamples);
  _depthImageView = createImageView(_depthImage, _depthFormat, attachmentAspect);
  _depthSampleView = createImageView(_depthImage, _depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

  destroySampledDepth();
  createImage(_swapChainExtent.width, _swapChainExtent.height, _depthFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              _sampledDepthImage, _sampledDepthMemory);
  _sampledDepthAttachmentView = createImageView(_sampledDepthImage, _depthFormat, attachmentAspect);
  _sampledDepthView = createImageView(_sampledDepthImage, _depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRenderer::createSceneColorResources()
{
  destroySceneColorResources();
  const VkImageUsageFlags colorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  _sceneColor = createAttachmentTexture(_swapChainExtent.width, _swapChainExtent.height, _sceneColorFormat, colorUsage,
                                        VK_IMAGE_ASPECT_COLOR_BIT, _msaaSamples);
  _glowColor = createAttachmentTexture(_swapChainExtent.width, _swapChainExtent.height, _sceneColorFormat, colorUsage,
                                       VK_IMAGE_ASPECT_COLOR_BIT, _msaaSamples);
}

void VulkanRenderer::destroySceneColorResources()
{
  destroyTexture(_sceneColor);
  destroyTexture(_glowColor);
}

void VulkanRenderer::createBlurResources()
{
  destroyBlurResources();
  const uint32_t width = std::max(_swapChainExtent.width, 1u);
  const uint32_t height = std::max(_swapChainExtent.height, 1u);
  const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  _blurDownsample = createAttachmentTexture(width, height, _sceneColorFormat, usage, VK_IMAGE_ASPECT_COLOR_BIT);
  _blurHorizontal = createAttachmentTexture(width, height, _sceneColorFormat, usage, VK_IMAGE_ASPECT_COLOR_BIT);
  _blurVertical = createAttachmentTexture(width, height, _sceneColorFormat, usage, VK_IMAGE_ASPECT_COLOR_BIT);

  auto makeFramebuffer = [&](VkImageView view, VkFramebuffer &framebuffer) {
    VkFramebufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = _blurRenderPass;
    info.attachmentCount = 1;
    info.pAttachments = &view;
    info.width = width;
    info.height = height;
    info.layers = 1;
    if (vkCreateFramebuffer(_device, &info, nullptr, &framebuffer) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create blur framebuffer");
    }
  };
  makeFramebuffer(_blurDownsample.view, _blurDownsampleFramebuffer);
  makeFramebuffer(_blurHorizontal.view, _blurHorizontalFramebuffer);
  makeFramebuffer(_blurVertical.view, _blurVerticalFramebuffer);
}

void VulkanRenderer::destroyBlurResources()
{
  auto destroyFramebuffer = [&](VkFramebuffer &framebuffer) {
    if (framebuffer)
    {
      vkDestroyFramebuffer(_device, framebuffer, nullptr);
      framebuffer = VK_NULL_HANDLE;
    }
  };
  destroyFramebuffer(_blurDownsampleFramebuffer);
  destroyFramebuffer(_blurHorizontalFramebuffer);
  destroyFramebuffer(_blurVerticalFramebuffer);
  destroyTexture(_blurDownsample);
  destroyTexture(_blurHorizontal);
  destroyTexture(_blurVertical);
}

void VulkanRenderer::createFullscreenQuad()
{
  QuadGeometry quad;
  const auto vertices = quad.vertices();
  const auto indices = quad.indices();
  _quadIndexCount = static_cast<uint32_t>(indices.size());
  uploadBuffer(_quadVertexBuffer, vertices.data(), vertices.size() * sizeof(RKVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  uploadBuffer(_quadIndexBuffer, indices.data(), indices.size() * sizeof(short), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void VulkanRenderer::createPostProcessPipelines()
{
  auto makeFullscreenConfig = [&](const QString &vert, const QString &frag, VkRenderPass renderPass, uint32_t colorCount,
                                  bool depthWrite) {
    VulkanShader::PipelineConfig config;
    config.vertexShaderResource = vert;
    config.fragmentShaderResource = frag;
    config.renderPass = renderPass;
    config.samples = VK_SAMPLE_COUNT_1_BIT;
    config.cullMode = VK_CULL_MODE_NONE;
    config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    config.depthTest = depthWrite;
    config.depthWrite = depthWrite;
    config.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    config.colorAttachmentCount = colorCount;
    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(RKVertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    config.bindings = {binding};
    VkVertexInputAttributeDescription attribute{};
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attribute.offset = offsetof(RKVertex, position);
    config.attributes = {attribute};
    return config;
  };

  _depthResolvePipeline = VulkanShader::createGraphicsPipeline(
      this, _msaaSamplerPipelineLayout,
      makeFullscreenConfig(QStringLiteral(":/shaders/fullscreen.vert.spv"), QStringLiteral(":/shaders/depth_resolve.frag.spv"),
                           _depthResolveRenderPass, 0, true));
  _blurDownsamplePipeline = VulkanShader::createGraphicsPipeline(
      this, _msaaSamplerPipelineLayout,
      makeFullscreenConfig(QStringLiteral(":/shaders/fullscreen.vert.spv"), QStringLiteral(":/shaders/blur_downsample.frag.spv"),
                           _blurRenderPass, 1, false));
  _blurHorizontalPipeline = VulkanShader::createGraphicsPipeline(
      this, _samplerPipelineLayout,
      makeFullscreenConfig(QStringLiteral(":/shaders/blur_horizontal.vert.spv"), QStringLiteral(":/shaders/blur.frag.spv"),
                           _blurRenderPass, 1, false));
  _blurVerticalPipeline = VulkanShader::createGraphicsPipeline(
      this, _samplerPipelineLayout,
      makeFullscreenConfig(QStringLiteral(":/shaders/blur_vertical.vert.spv"), QStringLiteral(":/shaders/blur.frag.spv"),
                           _blurRenderPass, 1, false));
  _compositePipeline = VulkanShader::createGraphicsPipeline(
      this, _compositePipelineLayout,
      makeFullscreenConfig(QStringLiteral(":/shaders/fullscreen.vert.spv"), QStringLiteral(":/shaders/composite.frag.spv"),
                           _compositeRenderPass, 1, false));

  VulkanShader::PipelineConfig overlayConfig =
      makeFullscreenConfig(QStringLiteral(":/shaders/fullscreen.vert.spv"), QStringLiteral(":/shaders/selection_overlay.frag.spv"),
                           _compositeRenderPass, 1, false);
  overlayConfig.blend = true;
  overlayConfig.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  overlayConfig.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  overlayConfig.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  overlayConfig.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  _overlayPipeline = VulkanShader::createGraphicsPipeline(this, _overlayPipelineLayout, overlayConfig);
}

void VulkanRenderer::destroyPostProcessPipelines()
{
  auto destroy = [&](VkPipeline &pipeline) {
    if (pipeline)
    {
      vkDestroyPipeline(_device, pipeline, nullptr);
      pipeline = VK_NULL_HANDLE;
    }
  };
  destroy(_depthResolvePipeline);
  destroy(_blurDownsamplePipeline);
  destroy(_blurHorizontalPipeline);
  destroy(_blurVerticalPipeline);
  destroy(_compositePipeline);
  destroy(_overlayPipeline);
}

void VulkanRenderer::createPostProcessDescriptorSets()
{
  auto allocate = [&](VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;
    VkDescriptorSet set = VK_NULL_HANDLE;
    if (vkAllocateDescriptorSets(_device, &allocInfo, &set) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate post-process descriptor set");
    }
    return set;
  };
  _depthResolveDescriptorSet = allocate(_msaaSamplerDescriptorSetLayout);
  _blurDownsampleDescriptorSet = allocate(_msaaSamplerDescriptorSetLayout);
  _blurHorizontalDescriptorSet = allocate(_samplerDescriptorSetLayout);
  _blurVerticalDescriptorSet = allocate(_samplerDescriptorSetLayout);
  _compositeDescriptorSet = allocate(_compositeDescriptorSetLayout);
  updatePostProcessDescriptors();
}

void VulkanRenderer::updatePostProcessDescriptors()
{
  auto writeSampler = [&](VkDescriptorSet set, uint32_t binding, VkImageView view, VkSampler sampler) {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = view;
    imageInfo.sampler = sampler;
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);
  };

  writeSampler(_depthResolveDescriptorSet, 0, _depthSampleView, _nearestSampler);
  writeSampler(_blurDownsampleDescriptorSet, 0, _glowColor.view, _nearestSampler);
  writeSampler(_blurHorizontalDescriptorSet, 0, _blurDownsample.view, _linearSampler);
  writeSampler(_blurVerticalDescriptorSet, 0, _blurHorizontal.view, _nearestSampler);
  writeSampler(_compositeDescriptorSet, 0, _sceneColor.view, _nearestSampler);
  writeSampler(_compositeDescriptorSet, 1, _blurVertical.view, _linearSampler);
}

void VulkanRenderer::cmdImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout,
                                    VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
                                    VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
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

void VulkanRenderer::beginSimpleColorPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer)
{
  VkRenderPassBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info.renderPass = renderPass;
  info.framebuffer = framebuffer;
  info.renderArea.extent = _swapChainExtent;
  VkClearValue clear{};
  clear.color = {{0.0f, 0.0f, 0.0f, 0.0f}};
  info.clearValueCount = 1;
  info.pClearValues = &clear;
  vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  setDefaultViewport(commandBuffer);
}

void VulkanRenderer::drawFullscreenQuad(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout layout,
                                        VkDescriptorSet set)
{
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &set, 0, nullptr);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_quadVertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(commandBuffer, _quadIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(commandBuffer, _quadIndexCount, 1, 0, 0, 0);
}

void VulkanRenderer::recordDepthResolve(VkCommandBuffer commandBuffer)
{
  const VkImageAspectFlags aspect =
      hasStencilComponent(_depthFormat) ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT;
  cmdImageBarrier(commandBuffer, _depthImage, aspect, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                  VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  VkRenderPassBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info.renderPass = _depthResolveRenderPass;
  info.framebuffer = _sampledDepthFramebuffer;
  info.renderArea.extent = _swapChainExtent;
  VkClearValue clear{};
  clear.depthStencil = {1.0f, 0};
  info.clearValueCount = 1;
  info.pClearValues = &clear;
  vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  setDefaultViewport(commandBuffer);
  drawFullscreenQuad(commandBuffer, _depthResolvePipeline, _msaaSamplerPipelineLayout, _depthResolveDescriptorSet);
  vkCmdEndRenderPass(commandBuffer);

  cmdImageBarrier(commandBuffer, _depthImage, aspect, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_READ_BIT,
                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
}

void VulkanRenderer::recordBlurAndComposite(VkCommandBuffer commandBuffer)
{
  cmdImageBarrier(commandBuffer, _sceneColor.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  cmdImageBarrier(commandBuffer, _glowColor.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  beginSimpleColorPass(commandBuffer, _blurRenderPass, _blurDownsampleFramebuffer);
  drawFullscreenQuad(commandBuffer, _blurDownsamplePipeline, _msaaSamplerPipelineLayout, _blurDownsampleDescriptorSet);
  vkCmdEndRenderPass(commandBuffer);

  beginSimpleColorPass(commandBuffer, _blurRenderPass, _blurHorizontalFramebuffer);
  drawFullscreenQuad(commandBuffer, _blurHorizontalPipeline, _samplerPipelineLayout, _blurHorizontalDescriptorSet);
  vkCmdEndRenderPass(commandBuffer);

  beginSimpleColorPass(commandBuffer, _blurRenderPass, _blurVerticalFramebuffer);
  drawFullscreenQuad(commandBuffer, _blurVerticalPipeline, _samplerPipelineLayout, _blurVerticalDescriptorSet);
  vkCmdEndRenderPass(commandBuffer);

  VkRenderPassBeginInfo compositeInfo{};
  compositeInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  compositeInfo.renderPass = _compositeRenderPass;
  compositeInfo.framebuffer = _compositeFramebuffers[_currentImageIndex];
  compositeInfo.renderArea.extent = _swapChainExtent;
  vkCmdBeginRenderPass(commandBuffer, &compositeInfo, VK_SUBPASS_CONTENTS_INLINE);
  setDefaultViewport(commandBuffer);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _compositePipeline);
  const uint32_t dynamicOffsets[] = {0, 0};
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _compositePipelineLayout, 0, 1, &_sceneDescriptorSet, 2,
                          dynamicOffsets);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _compositePipelineLayout, 1, 1, &_compositeDescriptorSet,
                          0, nullptr);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_quadVertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(commandBuffer, _quadIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(commandBuffer, _quadIndexCount, 1, 0, 0, 0);
  recordSelectionOverlay(commandBuffer);
  vkCmdEndRenderPass(commandBuffer);
}

void VulkanRenderer::setSelectionOverlay(const QRect &rect, bool dashed, float pixelScale)
{
  _selectionOverlayRect = rect.normalized();
  _selectionOverlayDashed = dashed;
  _selectionOverlayPixelScale = pixelScale > 0.0f ? pixelScale : 1.0f;
  _selectionOverlayEnabled = _selectionOverlayRect.width() >= 2 && _selectionOverlayRect.height() >= 2;
}

void VulkanRenderer::clearSelectionOverlay()
{
  _selectionOverlayEnabled = false;
  _selectionOverlayDashed = false;
  _selectionOverlayPixelScale = 1.0f;
  _selectionOverlayRect = QRect();
}

void VulkanRenderer::recordSelectionOverlay(VkCommandBuffer commandBuffer)
{
  if (!_selectionOverlayEnabled || !_overlayPipeline || !_overlayPipelineLayout || _quadIndexCount == 0)
  {
    return;
  }

  struct OverlayPushConstants
  {
    float minX;
    float minY;
    float maxX;
    float maxY;
    float dashed;
    float enabled;
    float pixelScale;
    float pad;
  } constants{};
  constants.minX = static_cast<float>(_selectionOverlayRect.left());
  constants.minY = static_cast<float>(_selectionOverlayRect.top());
  constants.maxX = static_cast<float>(_selectionOverlayRect.right() + 1);
  constants.maxY = static_cast<float>(_selectionOverlayRect.bottom() + 1);
  constants.dashed = _selectionOverlayDashed ? 1.0f : 0.0f;
  constants.enabled = 1.0f;
  constants.pixelScale = _selectionOverlayPixelScale;

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _overlayPipeline);
  vkCmdPushConstants(commandBuffer, _overlayPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), &constants);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_quadVertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(commandBuffer, _quadIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(commandBuffer, _quadIndexCount, 1, 0, 0, 0);
}

void VulkanRenderer::createCommandBuffers()
{
  _commandBuffers.resize(_swapChainImages.size());
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = _commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());
  if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate command buffers");
  }
}

void VulkanRenderer::createSyncObjects()
{
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore) != VK_SUCCESS ||
      vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create semaphores");
  }

  VkFenceCreateInfo fenceCreateInfo{};
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  _waitFences.resize(_commandBuffers.size());
  for (auto &fence : _waitFences)
  {
    if (vkCreateFence(_device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create fences");
    }
  }
}

void VulkanRenderer::createUniformBuffers()
{
  const auto hostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  createBuffer(sizeof(RKTransformationUniforms), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hostVisible, _frameUniformBuffer);
  createBuffer(_structureUniformStride, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hostVisible, _structureUniformBuffer);
  createBuffer(_isosurfaceUniformStride, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hostVisible, _isosurfaceUniformBuffer);
  createBuffer(sizeof(RKLightUniform) * 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hostVisible, _lightsUniformBuffer);
  createBuffer(sizeof(RKGlobalAxesUniforms), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hostVisible, _axesUniformBuffer);
  createBuffer(256, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hostVisible, _ribbonAODebugUniformBuffer);

  RKTransformationUniforms frame{};
  RKStructureUniforms structure{};
  RKIsosurfaceUniforms isosurface{};
  RKLightsUniforms lights{};
  RKGlobalAxesUniforms axes(nullptr);
  updateTransformUniforms(frame);
  updateStructureUniforms({structure});
  updateIsosurfaceUniforms({isosurface});
  updateLightUniforms(lights);
  updateGlobalAxesUniforms(axes);
  updateRibbonAODebugUniforms(RibbonAODebugUniforms{});
}

void VulkanRenderer::createDescriptorPool()
{
  std::array<VkDescriptorPoolSize, 3> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = 48;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  poolSizes[1].descriptorCount = 32;
  poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[2].descriptorCount = 256;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = 200;
  if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor pool");
  }
}

void VulkanRenderer::createSceneDescriptorSet()
{
  if (_sceneDescriptorSet)
  {
    vkFreeDescriptorSets(_device, _descriptorPool, 1, &_sceneDescriptorSet);
    _sceneDescriptorSet = VK_NULL_HANDLE;
  }

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &_sceneDescriptorSetLayout;
  if (vkAllocateDescriptorSets(_device, &allocInfo, &_sceneDescriptorSet) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate scene descriptor set");
  }

  VkDescriptorBufferInfo frameInfo{_frameUniformBuffer.buffer, 0, sizeof(RKTransformationUniforms)};
  VkDescriptorBufferInfo structureInfo{_structureUniformBuffer.buffer, 0, sizeof(RKStructureUniforms)};
  VkDescriptorBufferInfo isosurfaceInfo{_isosurfaceUniformBuffer.buffer, 0, sizeof(RKIsosurfaceUniforms)};
  VkDescriptorBufferInfo lightsInfo{_lightsUniformBuffer.buffer, 0, sizeof(RKLightUniform) * 4};
  VkDescriptorBufferInfo ribbonAOInfo{_ribbonAODebugUniformBuffer.buffer, 0, 256};
  VkDescriptorBufferInfo axesInfo{_axesUniformBuffer.buffer, 0, sizeof(RKGlobalAxesUniforms)};

  auto write = [&](uint32_t binding, VkDescriptorType type, const VkDescriptorBufferInfo *info) {
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = _sceneDescriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.descriptorType = type;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = info;
    return descriptorWrite;
  };

  std::array<VkWriteDescriptorSet, 6> writes = {
      write(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &frameInfo),
      write(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &structureInfo),
      write(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &isosurfaceInfo),
      write(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &lightsInfo),
      write(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &ribbonAOInfo),
      write(5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &axesInfo)};
  vkUpdateDescriptorSets(_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void VulkanRenderer::createSampler()
{
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  if (vkCreateSampler(_device, &samplerInfo, nullptr, &_linearSampler) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create sampler");
  }

  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  if (vkCreateSampler(_device, &samplerInfo, nullptr, &_repeatSampler) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create repeat sampler");
  }

  samplerInfo.magFilter = VK_FILTER_NEAREST;
  samplerInfo.minFilter = VK_FILTER_NEAREST;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  if (vkCreateSampler(_device, &samplerInfo, nullptr, &_nearestSampler) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create nearest sampler");
  }

  samplerInfo.compareEnable = VK_TRUE;
  samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  if (vkCreateSampler(_device, &samplerInfo, nullptr, &_shadowCompareSampler) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create shadow compare sampler");
  }
}

void VulkanRenderer::createWhiteTexture()
{
  QImage image(8, 8, QImage::Format_RGBA8888);
  image.fill(QColor(255, 255, 255, 255));
  _whiteTexture = createTextureRGBA8(image);
}

void VulkanRenderer::recreateSwapChain()
{
  if (!_window || _window->width() <= 0 || _window->height() <= 0)
  {
    return;
  }
  vkDeviceWaitIdle(_device);
  cleanupSwapChain(false);
  createSwapChain();
  createImageViews();
  createDepthResources();
  createSceneColorResources();
  createSceneFramebuffer();
  createBlurResources();
  createCompositeFramebuffers();
  createPickFramebuffer();
  updatePostProcessDescriptors();
  createCommandBuffers();
  if (_waitFences.size() != _commandBuffers.size())
  {
    for (auto fence : _waitFences)
    {
      vkDestroyFence(_device, fence, nullptr);
    }
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    _waitFences.resize(_commandBuffers.size());
    for (auto &fence : _waitFences)
    {
      if (vkCreateFence(_device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create fences");
      }
    }
  }
}

void VulkanRenderer::cleanup()
{
  if (!_device)
  {
    return;
  }
  vkDeviceWaitIdle(_device);
  cleanupSwapChain(true);

  destroyTexture(_whiteTexture);
  destroyBuffer(_frameUniformBuffer);
  destroyBuffer(_structureUniformBuffer);
  destroyBuffer(_isosurfaceUniformBuffer);
  destroyBuffer(_lightsUniformBuffer);
  destroyBuffer(_axesUniformBuffer);
  destroyBuffer(_ribbonAODebugUniformBuffer);
  destroyBuffer(_pickReadbackBuffer);
  destroyBuffer(_pickDepthReadbackBuffer);
  destroyBuffer(_quadVertexBuffer);
  destroyBuffer(_quadIndexBuffer);

  if (_linearSampler)
  {
    vkDestroySampler(_device, _linearSampler, nullptr);
  }
  if (_repeatSampler)
  {
    vkDestroySampler(_device, _repeatSampler, nullptr);
  }
  if (_nearestSampler)
  {
    vkDestroySampler(_device, _nearestSampler, nullptr);
  }
  if (_shadowCompareSampler)
  {
    vkDestroySampler(_device, _shadowCompareSampler, nullptr);
  }
  if (_descriptorPool)
  {
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
  }
  if (_scenePipelineLayout)
  {
    vkDestroyPipelineLayout(_device, _scenePipelineLayout, nullptr);
  }
  if (_backgroundPipelineLayout)
  {
    vkDestroyPipelineLayout(_device, _backgroundPipelineLayout, nullptr);
  }
  if (_volumePipelineLayout)
  {
    vkDestroyPipelineLayout(_device, _volumePipelineLayout, nullptr);
  }
  if (_msaaSamplerPipelineLayout)
  {
    vkDestroyPipelineLayout(_device, _msaaSamplerPipelineLayout, nullptr);
  }
  if (_samplerPipelineLayout)
  {
    vkDestroyPipelineLayout(_device, _samplerPipelineLayout, nullptr);
  }
  if (_compositePipelineLayout)
  {
    vkDestroyPipelineLayout(_device, _compositePipelineLayout, nullptr);
  }
  if (_overlayPipelineLayout)
  {
    vkDestroyPipelineLayout(_device, _overlayPipelineLayout, nullptr);
  }
  if (_sceneDescriptorSetLayout)
  {
    vkDestroyDescriptorSetLayout(_device, _sceneDescriptorSetLayout, nullptr);
  }
  if (_samplerDescriptorSetLayout)
  {
    vkDestroyDescriptorSetLayout(_device, _samplerDescriptorSetLayout, nullptr);
  }
  if (_volumeDescriptorSetLayout)
  {
    vkDestroyDescriptorSetLayout(_device, _volumeDescriptorSetLayout, nullptr);
  }
  if (_msaaSamplerDescriptorSetLayout)
  {
    vkDestroyDescriptorSetLayout(_device, _msaaSamplerDescriptorSetLayout, nullptr);
  }
  if (_compositeDescriptorSetLayout)
  {
    vkDestroyDescriptorSetLayout(_device, _compositeDescriptorSetLayout, nullptr);
  }
  for (auto fence : _waitFences)
  {
    vkDestroyFence(_device, fence, nullptr);
  }
  if (_renderFinishedSemaphore)
  {
    vkDestroySemaphore(_device, _renderFinishedSemaphore, nullptr);
  }
  if (_imageAvailableSemaphore)
  {
    vkDestroySemaphore(_device, _imageAvailableSemaphore, nullptr);
  }
  if (_commandPool)
  {
    vkDestroyCommandPool(_device, _commandPool, nullptr);
  }
  vkDestroyDevice(_device, nullptr);
  DestroyDebugReportCallbackEXT(_instance, _callback, nullptr);
  if (_surface)
  {
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
  }
  vkDestroyInstance(_instance, nullptr);
}

void VulkanRenderer::cleanupSwapChain(bool destroyRenderPass)
{
  destroyPickFramebuffer();
  destroyBlurResources();
  if (_sceneFramebuffer)
  {
    vkDestroyFramebuffer(_device, _sceneFramebuffer, nullptr);
    _sceneFramebuffer = VK_NULL_HANDLE;
  }
  if (_sampledDepthFramebuffer)
  {
    vkDestroyFramebuffer(_device, _sampledDepthFramebuffer, nullptr);
    _sampledDepthFramebuffer = VK_NULL_HANDLE;
  }
  destroySceneColorResources();
  destroySampledDepth();
  if (_depthImageView)
  {
    vkDestroyImageView(_device, _depthImageView, nullptr);
    _depthImageView = VK_NULL_HANDLE;
  }
  if (_depthSampleView)
  {
    vkDestroyImageView(_device, _depthSampleView, nullptr);
    _depthSampleView = VK_NULL_HANDLE;
  }
  if (_depthImage)
  {
    vkDestroyImage(_device, _depthImage, nullptr);
    _depthImage = VK_NULL_HANDLE;
  }
  if (_depthImageMemory)
  {
    vkFreeMemory(_device, _depthImageMemory, nullptr);
    _depthImageMemory = VK_NULL_HANDLE;
  }
  for (auto framebuffer : _compositeFramebuffers)
  {
    vkDestroyFramebuffer(_device, framebuffer, nullptr);
  }
  _compositeFramebuffers.clear();
  if (!_commandBuffers.empty())
  {
    vkFreeCommandBuffers(_device, _commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
    _commandBuffers.clear();
  }
  if (destroyRenderPass)
  {
    destroyPostProcessPipelines();
  }
  if (destroyRenderPass && _renderPass)
  {
    vkDestroyRenderPass(_device, _renderPass, nullptr);
    _renderPass = VK_NULL_HANDLE;
  }
  if (destroyRenderPass && _continueRenderPass)
  {
    vkDestroyRenderPass(_device, _continueRenderPass, nullptr);
    _continueRenderPass = VK_NULL_HANDLE;
  }
  if (destroyRenderPass && _pickRenderPass)
  {
    vkDestroyRenderPass(_device, _pickRenderPass, nullptr);
    _pickRenderPass = VK_NULL_HANDLE;
  }
  if (destroyRenderPass && _depthResolveRenderPass)
  {
    vkDestroyRenderPass(_device, _depthResolveRenderPass, nullptr);
    _depthResolveRenderPass = VK_NULL_HANDLE;
  }
  if (destroyRenderPass && _blurRenderPass)
  {
    vkDestroyRenderPass(_device, _blurRenderPass, nullptr);
    _blurRenderPass = VK_NULL_HANDLE;
  }
  if (destroyRenderPass && _compositeRenderPass)
  {
    vkDestroyRenderPass(_device, _compositeRenderPass, nullptr);
    _compositeRenderPass = VK_NULL_HANDLE;
  }
  if (_offscreen)
  {
    _swapChainImageViews.clear();
    _swapChainImages.clear();
    destroyTexture(_offscreenColor);
  }
  else
  {
    for (auto imageView : _swapChainImageViews)
    {
      vkDestroyImageView(_device, imageView, nullptr);
    }
    _swapChainImageViews.clear();
    if (_swapChain)
    {
      vkDestroySwapchainKHR(_device, _swapChain, nullptr);
      _swapChain = VK_NULL_HANDLE;
    }
  }
}

void VulkanRenderer::createInstance()
{
  if (enableValidationLayers && !checkValidationLayerSupport())
  {
    throw std::runtime_error("validation layers requested, but not available");
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "iRASPA";
  appInfo.applicationVersion = VK_MAKE_VERSION(2, 1, 0);
  appInfo.pEngineName = "iRASPA Vulkan";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  auto extensions = getRequiredExtensions();
  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();
  if (_usePortabilityEnumeration)
  {
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  }
  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
  if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create instance");
  }
}

void VulkanRenderer::checkExtensions()
{
}

bool VulkanRenderer::checkValidationLayerSupport()
{
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
  for (const char *layerName : validationLayers)
  {
    bool layerFound = false;
    for (const auto &layerProperties : availableLayers)
    {
      if (strcmp(layerName, layerProperties.layerName) == 0)
      {
        layerFound = true;
        break;
      }
    }
    if (!layerFound)
    {
      return false;
    }
  }
  return true;
}

std::vector<const char *> VulkanRenderer::getRequiredExtensions()
{
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> available(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, available.data());
  auto hasExtension = [&available](const char *name) {
    for (const auto &extension : available)
    {
      if (strcmp(extension.extensionName, name) == 0)
      {
        return true;
      }
    }
    return false;
  };

  std::vector<const char *> extensions;
  if (!_offscreen)
  {
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef Q_OS_MACOS
    if (hasExtension(VK_EXT_METAL_SURFACE_EXTENSION_NAME))
    {
      extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    }
    else
    {
      extensions.push_back("VK_MVK_macos_surface");
    }
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    extensions.push_back("VK_KHR_win32_surface");
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    extensions.push_back("VK_KHR_xcb_surface");
#endif
  }
#ifdef Q_OS_MACOS
  if (hasExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
  {
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    _usePortabilityEnumeration = true;
  }
  if (hasExtension("VK_KHR_get_physical_device_properties2"))
  {
    extensions.push_back("VK_KHR_get_physical_device_properties2");
  }
#endif
  if (enableValidationLayers)
  {
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }
  return extensions;
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device)
{
  QueueFamilyIndices indices = findQueueFamilies(device);
  bool extensionsSupported = checkDeviceExtensionSupport(device);
  if (_offscreen)
  {
    return indices.isComplete() && extensionsSupported;
  }
  bool swapChainAdequate = false;
  if (extensionsSupported)
  {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }
  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
  for (const auto &extension : availableExtensions)
  {
    requiredExtensions.erase(extension.extensionName);
  }
  return requiredExtensions.empty();
}

QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device)
{
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  for (uint32_t i = 0; i < queueFamilies.size(); ++i)
  {
    if (queueFamilies[i].queueCount > 0 && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
    {
      indices.graphicsFamily = i;
      if (_offscreen)
      {
        indices.presentFamily = i;
      }
    }
    if (!_offscreen && _surface)
    {
      VkBool32 presentSupport = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
      if (queueFamilies[i].queueCount > 0 && presentSupport)
      {
        indices.presentFamily = i;
      }
    }
    if (indices.isComplete())
    {
      break;
    }
  }
  return indices;
}

SwapChainSupportDetails VulkanRenderer::querySwapChainSupport(VkPhysicalDevice device)
{
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
  if (formatCount != 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
  }
  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);
  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
  }
  return details;
}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
  for (const auto &availableFormat : availableFormats)
  {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      return availableFormat;
    }
  }
  for (const auto &availableFormat : availableFormats)
  {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
    {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &)
{
  // Event-driven renderer: FIFO vsync. MAILBOX can display an in-flight image
  // as soon as it finishes, which made a stale camera frame visible on project switch.
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  }
  const qreal dpr = _window ? _window->devicePixelRatio() : 1.0;
  VkExtent2D actualExtent = {static_cast<uint32_t>(_window->width() * dpr), static_cast<uint32_t>(_window->height() * dpr)};
  actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
  actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
  return actualExtent;
}

VkShaderModule VulkanRenderer::createShaderModule(const QByteArray &code)
{
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = static_cast<size_t>(code.size());
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.constData());
  VkShaderModule shaderModule;
  if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create shader module");
  }
  return shaderModule;
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memProperties{};
  vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
  {
    if ((typeFilter & (1u << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type");
}
