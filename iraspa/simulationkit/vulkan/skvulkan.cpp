/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#include "skvulkan.h"

#include <cstring>
#include <stdexcept>
#include <vector>

#include <QDebug>
#include <QFile>
#include <QtGlobal>

#ifndef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#define VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME "VK_KHR_portability_enumeration"
#endif
#ifndef VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#define VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR 0x00000001
#endif

namespace
{
bool hasExtension(const std::vector<VkExtensionProperties> &available, const char *name)
{
  for (const auto &extension : available)
  {
    if (strcmp(extension.extensionName, name) == 0)
    {
      return true;
    }
  }
  return false;
}

uint32_t findComputeQueueFamily(VkPhysicalDevice device)
{
  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
  std::vector<VkQueueFamilyProperties> families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());
  uint32_t fallback = UINT32_MAX;
  for (uint32_t i = 0; i < count; ++i)
  {
    if (families[i].queueCount == 0 || !(families[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
    {
      continue;
    }
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      fallback = i;
    }
    else
    {
      return i;
    }
  }
  return fallback;
}

int deviceScore(VkPhysicalDevice device)
{
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(device, &properties);
  int score = 0;
  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
  {
    score += 1000;
  }
  else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
  {
    score += 100;
  }
  if (properties.limits.maxComputeWorkGroupInvocations < 256)
  {
    return -1;
  }
  if (properties.limits.maxPushConstantsSize < 64)
  {
    return -1;
  }
  if (findComputeQueueFamily(device) == UINT32_MAX)
  {
    return -1;
  }
  return score;
}
} // namespace

SKVulkan::SKVulkan()
{
  try
  {
    init();
    _ready = true;
  }
  catch (const std::exception &error)
  {
    qDebug() << "SKVulkan: compute context unavailable:" << error.what();
    cleanup();
    _ready = false;
  }
}

SKVulkan::~SKVulkan()
{
  cleanup();
}

void SKVulkan::init()
{
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> available(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, available.data());

  std::vector<const char *> instanceExtensions;
#ifdef Q_OS_MACOS
  if (hasExtension(available, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
  {
    instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    _usePortabilityEnumeration = true;
  }
  if (hasExtension(available, "VK_KHR_get_physical_device_properties2"))
  {
    instanceExtensions.push_back("VK_KHR_get_physical_device_properties2");
  }
#endif

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "iRASPA SimulationKit";
  appInfo.applicationVersion = VK_MAKE_VERSION(2, 1, 0);
  appInfo.pEngineName = "iRASPA Vulkan Compute";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo instanceInfo{};
  instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceInfo.pApplicationInfo = &appInfo;
  instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
  instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
  if (_usePortabilityEnumeration)
  {
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  }
  if (vkCreateInstance(&instanceInfo, nullptr, &_instance) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create Vulkan instance");
  }

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
  if (deviceCount == 0)
  {
    throw std::runtime_error("no Vulkan physical devices");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

  int bestScore = -1;
  for (VkPhysicalDevice candidate : devices)
  {
    const int score = deviceScore(candidate);
    if (score > bestScore)
    {
      bestScore = score;
      _physicalDevice = candidate;
    }
  }
  if (_physicalDevice == VK_NULL_HANDLE || bestScore < 0)
  {
    throw std::runtime_error("no suitable compute device");
  }

#ifdef Q_OS_MACOS
  uint32_t deviceExtensionCount = 0;
  vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &deviceExtensionCount, nullptr);
  std::vector<VkExtensionProperties> deviceAvailable(deviceExtensionCount);
  vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &deviceExtensionCount, deviceAvailable.data());
  if (hasExtension(deviceAvailable, "VK_KHR_portability_subset"))
  {
    _deviceExtensions.push_back("VK_KHR_portability_subset");
  }
#endif

  _queueFamily = findComputeQueueFamily(_physicalDevice);
  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueInfo{};
  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueInfo.queueFamilyIndex = _queueFamily;
  queueInfo.queueCount = 1;
  queueInfo.pQueuePriorities = &queuePriority;

  VkPhysicalDeviceFeatures features{};
  VkDeviceCreateInfo deviceInfo{};
  deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceInfo.queueCreateInfoCount = 1;
  deviceInfo.pQueueCreateInfos = &queueInfo;
  deviceInfo.pEnabledFeatures = &features;
  deviceInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
  deviceInfo.ppEnabledExtensionNames = _deviceExtensions.data();
  if (vkCreateDevice(_physicalDevice, &deviceInfo, nullptr, &_device) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create logical device");
  }
  vkGetDeviceQueue(_device, _queueFamily, 0, &_queue);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = _queueFamily;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create command pool");
  }

  VkDescriptorPoolSize poolSizes[3]{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[0].descriptorCount = 64;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  poolSizes[1].descriptorCount = 64;
  poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[2].descriptorCount = 64;
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.maxSets = 32;
  descriptorPoolInfo.poolSizeCount = 3;
  descriptorPoolInfo.pPoolSizes = poolSizes;
  if (vkCreateDescriptorPool(_device, &descriptorPoolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor pool");
  }

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_NEAREST;
  samplerInfo.minFilter = VK_FILTER_NEAREST;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.maxLod = 0.0f;
  if (vkCreateSampler(_device, &samplerInfo, nullptr, &_nearestSampler) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create sampler");
  }

  createDummyUintImage();
}

void SKVulkan::createDummyUintImage()
{
  _dummyUint = createImage3D(1, 1, 1, VK_FORMAT_R32_UINT,
                             VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  const uint32_t zero = 0;
  SKVulkanBuffer staging = createBuffer(sizeof(zero), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  writeBuffer(staging, &zero, sizeof(zero));
  VkCommandBuffer commandBuffer = beginCommands();
  imageBarrier(commandBuffer, _dummyUint.image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
  VkBufferImageCopy region{};
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.layerCount = 1;
  region.imageExtent = {1, 1, 1};
  vkCmdCopyBufferToImage(commandBuffer, staging.buffer, _dummyUint.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  imageBarrier(commandBuffer, _dummyUint.image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
               VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  submitAndWait(commandBuffer);
  destroyBuffer(staging);
}

void SKVulkan::cleanup()
{
  if (_device)
  {
    vkDeviceWaitIdle(_device);
  }
  destroyImage(_dummyUint);
  if (_nearestSampler)
  {
    vkDestroySampler(_device, _nearestSampler, nullptr);
    _nearestSampler = VK_NULL_HANDLE;
  }
  if (_descriptorPool)
  {
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    _descriptorPool = VK_NULL_HANDLE;
  }
  if (_commandPool)
  {
    vkDestroyCommandPool(_device, _commandPool, nullptr);
    _commandPool = VK_NULL_HANDLE;
  }
  if (_device)
  {
    vkDestroyDevice(_device, nullptr);
    _device = VK_NULL_HANDLE;
  }
  if (_instance)
  {
    vkDestroyInstance(_instance, nullptr);
    _instance = VK_NULL_HANDLE;
  }
  _physicalDevice = VK_NULL_HANDLE;
  _queue = VK_NULL_HANDLE;
}

uint32_t SKVulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
  VkPhysicalDeviceMemoryProperties memoryProperties{};
  vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memoryProperties);
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
  {
    if ((typeFilter & (1u << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type");
}

SKVulkanBuffer SKVulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
  SKVulkanBuffer out;
  out.size = size;
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vkCreateBuffer(_device, &bufferInfo, nullptr, &out.buffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create buffer");
  }
  VkMemoryRequirements requirements{};
  vkGetBufferMemoryRequirements(_device, out.buffer, &requirements);
  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = requirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, properties);
  if (vkAllocateMemory(_device, &allocInfo, nullptr, &out.memory) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate buffer memory");
  }
  vkBindBufferMemory(_device, out.buffer, out.memory, 0);
  return out;
}

void SKVulkan::destroyBuffer(SKVulkanBuffer &buffer) const
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

void SKVulkan::writeBuffer(const SKVulkanBuffer &buffer, const void *data, VkDeviceSize size) const
{
  void *mapped = nullptr;
  vkMapMemory(_device, buffer.memory, 0, size, 0, &mapped);
  memcpy(mapped, data, static_cast<size_t>(size));
  vkUnmapMemory(_device, buffer.memory);
}

void SKVulkan::readBuffer(const SKVulkanBuffer &buffer, void *data, VkDeviceSize size) const
{
  void *mapped = nullptr;
  vkMapMemory(_device, buffer.memory, 0, size, 0, &mapped);
  memcpy(data, mapped, static_cast<size_t>(size));
  vkUnmapMemory(_device, buffer.memory);
}

SKVulkanImage3D SKVulkan::createImage3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format,
                                        VkImageUsageFlags usage)
{
  SKVulkanImage3D out;
  out.width = width;
  out.height = height;
  out.depth = depth;
  out.format = format;

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_3D;
  imageInfo.extent = {width, height, depth};
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vkCreateImage(_device, &imageInfo, nullptr, &out.image) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create 3D image");
  }

  VkMemoryRequirements requirements{};
  vkGetImageMemoryRequirements(_device, out.image, &requirements);
  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = requirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (vkAllocateMemory(_device, &allocInfo, nullptr, &out.memory) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate 3D image memory");
  }
  vkBindImageMemory(_device, out.image, out.memory, 0);

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = out.image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.layerCount = 1;
  if (vkCreateImageView(_device, &viewInfo, nullptr, &out.view) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create 3D image view");
  }
  return out;
}

void SKVulkan::destroyImage(SKVulkanImage3D &image) const
{
  if (image.view)
  {
    vkDestroyImageView(_device, image.view, nullptr);
    image.view = VK_NULL_HANDLE;
  }
  if (image.image)
  {
    vkDestroyImage(_device, image.image, nullptr);
    image.image = VK_NULL_HANDLE;
  }
  if (image.memory)
  {
    vkFreeMemory(_device, image.memory, nullptr);
    image.memory = VK_NULL_HANDLE;
  }
  image.width = 0;
  image.height = 0;
  image.depth = 0;
}

VkShaderModule SKVulkan::loadShaderModule(const QString &resourcePath) const
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
  if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create shader module");
  }
  return shaderModule;
}

VkPipeline SKVulkan::createComputePipeline(VkPipelineLayout layout, const QString &resourcePath) const
{
  VkShaderModule module = loadShaderModule(resourcePath);
  VkPipelineShaderStageCreateInfo stage{};
  stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  stage.module = module;
  stage.pName = "main";

  VkComputePipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.stage = stage;
  pipelineInfo.layout = layout;
  VkPipeline pipeline = VK_NULL_HANDLE;
  const VkResult result = vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
  vkDestroyShaderModule(_device, module, nullptr);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create compute pipeline");
  }
  return pipeline;
}

VkCommandBuffer SKVulkan::beginCommands() const
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = _commandPool;
  allocInfo.commandBufferCount = 1;
  VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
  vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void SKVulkan::submitAndWait(VkCommandBuffer commandBuffer) const
{
  vkEndCommandBuffer(commandBuffer);
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(_queue);
  vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

VkDescriptorSet SKVulkan::allocateDescriptorSet(VkDescriptorSetLayout layout)
{
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &layout;
  VkDescriptorSet set = VK_NULL_HANDLE;
  if (vkAllocateDescriptorSets(_device, &allocInfo, &set) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate descriptor set");
  }
  return set;
}

void SKVulkan::resetDescriptorPool()
{
  if (_descriptorPool)
  {
    vkResetDescriptorPool(_device, _descriptorPool, 0);
  }
}

void SKVulkan::imageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
                            VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage,
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
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.layerCount = 1;
  vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void SKVulkan::memoryBarrier(VkCommandBuffer commandBuffer, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
                             VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
  VkMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  barrier.srcAccessMask = srcAccess;
  barrier.dstAccessMask = dstAccess;
  vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}
