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

#pragma once

#include <cstdint>
#include <vector>

#include <QString>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <vulkan/vulkan.h>

struct SKVulkanBuffer
{
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkDeviceSize size = 0;
};

struct SKVulkanImage3D
{
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 0;
  VkFormat format = VK_FORMAT_UNDEFINED;
};

class SKVulkan
{
public:
  SKVulkan(const SKVulkan &) = delete;
  void operator=(const SKVulkan &) = delete;

  static SKVulkan &getInstance()
  {
    static SKVulkan instance;
    return instance;
  }

  bool isReady() const { return _ready; }
  VkDevice device() const { return _device; }
  VkQueue queue() const { return _queue; }
  uint32_t queueFamily() const { return _queueFamily; }
  VkSampler nearestSampler() const { return _nearestSampler; }
  VkImageView dummyUint3DView() const { return _dummyUint.view; }

  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
  SKVulkanBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
  void destroyBuffer(SKVulkanBuffer &buffer) const;
  void writeBuffer(const SKVulkanBuffer &buffer, const void *data, VkDeviceSize size) const;
  void readBuffer(const SKVulkanBuffer &buffer, void *data, VkDeviceSize size) const;

  SKVulkanImage3D createImage3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage);
  void destroyImage(SKVulkanImage3D &image) const;

  VkShaderModule loadShaderModule(const QString &resourcePath) const;
  VkPipeline createComputePipeline(VkPipelineLayout layout, const QString &resourcePath) const;

  VkCommandBuffer beginCommands() const;
  void submitAndWait(VkCommandBuffer commandBuffer) const;

  VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout layout);
  void resetDescriptorPool();

  static void imageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
                           VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage,
                           VkPipelineStageFlags dstStage);
  static void memoryBarrier(VkCommandBuffer commandBuffer, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
                            VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

private:
  SKVulkan();
  ~SKVulkan();
  void init();
  void cleanup();
  void createDummyUintImage();

  bool _ready = false;
  bool _usePortabilityEnumeration = false;
  VkInstance _instance = VK_NULL_HANDLE;
  VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
  VkDevice _device = VK_NULL_HANDLE;
  VkQueue _queue = VK_NULL_HANDLE;
  uint32_t _queueFamily = 0;
  VkCommandPool _commandPool = VK_NULL_HANDLE;
  VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
  VkSampler _nearestSampler = VK_NULL_HANDLE;
  SKVulkanImage3D _dummyUint;
  std::vector<const char *> _deviceExtensions;
};
