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

#include "skcomputeisosurface.h"
#include "marchingcubes.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

#include <QDebug>
#include <QString>

namespace
{
constexpr uint32_t kClassifyLocalSize = 4;
constexpr uint32_t kTraverseWorkgroupSize = 64;
constexpr int kMaxHpLevels = 10;

struct ClassifyPushConstants
{
  int32_t dimensions[4];
  float isolevel;
};

struct TraversePushConstants
{
  int32_t dimensions[4];
  float isolevel;
  int32_t triangleCount;
  int32_t hpLevelCount;
};

uint32_t dispatchCount(uint32_t extent, uint32_t localSize)
{
  return (extent + localSize - 1) / localSize;
}

void writeStorageImage(VkDevice device, VkDescriptorSet set, uint32_t binding, VkImageView view)
{
  VkDescriptorImageInfo info{};
  info.imageView = view;
  info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = set;
  write.dstBinding = binding;
  write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  write.descriptorCount = 1;
  write.pImageInfo = &info;
  vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void writeCombinedImage(VkDevice device, VkDescriptorSet set, uint32_t binding, VkSampler sampler, VkImageView view,
                        VkImageLayout layout)
{
  VkDescriptorImageInfo info{};
  info.sampler = sampler;
  info.imageView = view;
  info.imageLayout = layout;
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = set;
  write.dstBinding = binding;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.descriptorCount = 1;
  write.pImageInfo = &info;
  vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}
} // namespace

SKComputeIsosurface::SKComputeIsosurface()
{
  SKVulkan &vulkan = SKVulkan::getInstance();
  if (!vulkan.isReady())
  {
    return;
  }

  try
  {
    VkDescriptorSetLayoutBinding classifyBindings[3]{};
    classifyBindings[0].binding = 0;
    classifyBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    classifyBindings[0].descriptorCount = 1;
    classifyBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    classifyBindings[1] = classifyBindings[0];
    classifyBindings[1].binding = 1;
    classifyBindings[2].binding = 2;
    classifyBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    classifyBindings[2].descriptorCount = 1;
    classifyBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo classifyLayoutInfo{};
    classifyLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    classifyLayoutInfo.bindingCount = 3;
    classifyLayoutInfo.pBindings = classifyBindings;
    if (vkCreateDescriptorSetLayout(vulkan.device(), &classifyLayoutInfo, nullptr, &_classifyLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("classify descriptor layout");
    }

    VkDescriptorSetLayoutBinding constructBindings[2]{};
    constructBindings[0].binding = 0;
    constructBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    constructBindings[0].descriptorCount = 1;
    constructBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    constructBindings[1].binding = 1;
    constructBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    constructBindings[1].descriptorCount = 1;
    constructBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    VkDescriptorSetLayoutCreateInfo constructLayoutInfo{};
    constructLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    constructLayoutInfo.bindingCount = 2;
    constructLayoutInfo.pBindings = constructBindings;
    if (vkCreateDescriptorSetLayout(vulkan.device(), &constructLayoutInfo, nullptr, &_constructLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("construct descriptor layout");
    }

    VkDescriptorSetLayoutBinding traverseBindings[4]{};
    traverseBindings[0].binding = 0;
    traverseBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    traverseBindings[0].descriptorCount = 1;
    traverseBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    traverseBindings[1].binding = 1;
    traverseBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    traverseBindings[1].descriptorCount = kMaxHpLevels;
    traverseBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    traverseBindings[2].binding = 11;
    traverseBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    traverseBindings[2].descriptorCount = 1;
    traverseBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    traverseBindings[3].binding = 12;
    traverseBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    traverseBindings[3].descriptorCount = 1;
    traverseBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    VkDescriptorSetLayoutCreateInfo traverseLayoutInfo{};
    traverseLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    traverseLayoutInfo.bindingCount = 4;
    traverseLayoutInfo.pBindings = traverseBindings;
    if (vkCreateDescriptorSetLayout(vulkan.device(), &traverseLayoutInfo, nullptr, &_traverseLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("traverse descriptor layout");
    }

    VkPushConstantRange classifyPush{};
    classifyPush.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    classifyPush.size = sizeof(ClassifyPushConstants);
    VkPipelineLayoutCreateInfo classifyPipelineLayoutInfo{};
    classifyPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    classifyPipelineLayoutInfo.setLayoutCount = 1;
    classifyPipelineLayoutInfo.pSetLayouts = &_classifyLayout;
    classifyPipelineLayoutInfo.pushConstantRangeCount = 1;
    classifyPipelineLayoutInfo.pPushConstantRanges = &classifyPush;
    if (vkCreatePipelineLayout(vulkan.device(), &classifyPipelineLayoutInfo, nullptr, &_classifyPipelineLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("classify pipeline layout");
    }

    VkPipelineLayoutCreateInfo constructPipelineLayoutInfo{};
    constructPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    constructPipelineLayoutInfo.setLayoutCount = 1;
    constructPipelineLayoutInfo.pSetLayouts = &_constructLayout;
    if (vkCreatePipelineLayout(vulkan.device(), &constructPipelineLayoutInfo, nullptr, &_constructPipelineLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("construct pipeline layout");
    }

    VkPushConstantRange traversePush{};
    traversePush.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    traversePush.size = sizeof(TraversePushConstants);
    VkPipelineLayoutCreateInfo traversePipelineLayoutInfo{};
    traversePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    traversePipelineLayoutInfo.setLayoutCount = 1;
    traversePipelineLayoutInfo.pSetLayouts = &_traverseLayout;
    traversePipelineLayoutInfo.pushConstantRangeCount = 1;
    traversePipelineLayoutInfo.pPushConstantRanges = &traversePush;
    if (vkCreatePipelineLayout(vulkan.device(), &traversePipelineLayoutInfo, nullptr, &_traversePipelineLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("traverse pipeline layout");
    }

    _classifyPipeline = vulkan.createComputePipeline(_classifyPipelineLayout, QStringLiteral(":/simulationkit/shaders/mc_classify.comp.spv"));
    _constructPipeline = vulkan.createComputePipeline(_constructPipelineLayout, QStringLiteral(":/simulationkit/shaders/mc_construct.comp.spv"));
    _traversePipeline = vulkan.createComputePipeline(_traversePipelineLayout, QStringLiteral(":/simulationkit/shaders/mc_traverse.comp.spv"));
    _ready = true;
  }
  catch (const std::exception &error)
  {
    qDebug() << "SKComputeIsosurface: GPU pipeline unavailable:" << error.what();
    auto destroy = [&](VkPipeline &pipeline) {
      if (pipeline)
      {
        vkDestroyPipeline(vulkan.device(), pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
      }
    };
    auto destroyLayout = [&](VkPipelineLayout &layout) {
      if (layout)
      {
        vkDestroyPipelineLayout(vulkan.device(), layout, nullptr);
        layout = VK_NULL_HANDLE;
      }
    };
    auto destroySetLayout = [&](VkDescriptorSetLayout &layout) {
      if (layout)
      {
        vkDestroyDescriptorSetLayout(vulkan.device(), layout, nullptr);
        layout = VK_NULL_HANDLE;
      }
    };
    destroy(_traversePipeline);
    destroy(_constructPipeline);
    destroy(_classifyPipeline);
    destroyLayout(_traversePipelineLayout);
    destroyLayout(_constructPipelineLayout);
    destroyLayout(_classifyPipelineLayout);
    destroySetLayout(_traverseLayout);
    destroySetLayout(_constructLayout);
    destroySetLayout(_classifyLayout);
  }
}

SKComputeIsosurface::~SKComputeIsosurface()
{
  SKVulkan &vulkan = SKVulkan::getInstance();
  if (!vulkan.isReady())
  {
    return;
  }
  if (_traversePipeline)
  {
    vkDestroyPipeline(vulkan.device(), _traversePipeline, nullptr);
  }
  if (_constructPipeline)
  {
    vkDestroyPipeline(vulkan.device(), _constructPipeline, nullptr);
  }
  if (_classifyPipeline)
  {
    vkDestroyPipeline(vulkan.device(), _classifyPipeline, nullptr);
  }
  if (_traversePipelineLayout)
  {
    vkDestroyPipelineLayout(vulkan.device(), _traversePipelineLayout, nullptr);
  }
  if (_constructPipelineLayout)
  {
    vkDestroyPipelineLayout(vulkan.device(), _constructPipelineLayout, nullptr);
  }
  if (_classifyPipelineLayout)
  {
    vkDestroyPipelineLayout(vulkan.device(), _classifyPipelineLayout, nullptr);
  }
  if (_traverseLayout)
  {
    vkDestroyDescriptorSetLayout(vulkan.device(), _traverseLayout, nullptr);
  }
  if (_constructLayout)
  {
    vkDestroyDescriptorSetLayout(vulkan.device(), _constructLayout, nullptr);
  }
  if (_classifyLayout)
  {
    vkDestroyDescriptorSetLayout(vulkan.device(), _classifyLayout, nullptr);
  }
}

std::vector<float4> SKComputeIsosurface::computeIsosurface(int3 dimensions, std::vector<float> *voxels, double isoValue)
{
  if (getInstance()._ready)
  {
    try
    {
      return getInstance().computeIsosurfaceGPUImplementation(dimensions, voxels, isoValue);
    }
    catch (const std::exception &error)
    {
      qDebug() << "SKComputeIsosurface: GPU path failed, using CPU:" << error.what();
    }
  }
  return computeIsosurfaceCPUImplementation(dimensions, voxels, isoValue);
}

std::vector<float4> SKComputeIsosurface::computeIsosurfaceGPUImplementation(int3 dimensions, std::vector<float> *voxels, double isoValue)
{
  if (!voxels)
  {
    return {};
  }

  const int largestSize = std::max({dimensions.x, dimensions.y, dimensions.z});
  int powerOfTwo = 1;
  while (largestSize > (1 << powerOfTwo))
  {
    ++powerOfTwo;
  }
  const uint32_t size = static_cast<uint32_t>(1 << powerOfTwo);

  std::vector<float> padded;
  const float *volumeData = voxels->data();
  const size_t volumeCount = static_cast<size_t>(size) * size * size;
  if (voxels->size() < volumeCount)
  {
    padded.assign(volumeCount, 0.0f);
    std::copy(voxels->begin(), voxels->end(), padded.begin());
    volumeData = padded.data();
  }

  SKVulkan &vulkan = SKVulkan::getInstance();
  vulkan.resetDescriptorPool();

  const VkImageUsageFlags hpUsage =
      VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  std::vector<SKVulkanImage3D> hpImages;
  uint32_t bufferSize = size;
  for (int i = 1; i < powerOfTwo; ++i)
  {
    hpImages.push_back(vulkan.createImage3D(bufferSize, bufferSize, bufferSize, VK_FORMAT_R32_UINT, hpUsage));
    bufferSize /= 2;
  }
  hpImages.push_back(vulkan.createImage3D(bufferSize, bufferSize, bufferSize, VK_FORMAT_R32_UINT, hpUsage));
  SKVulkanImage3D cubeIndex = vulkan.createImage3D(size, size, size, VK_FORMAT_R32_UINT, hpUsage);
  SKVulkanImage3D rawData = vulkan.createImage3D(size, size, size, VK_FORMAT_R32_SFLOAT,
                                                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

  const VkMemoryPropertyFlags hostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  SKVulkanBuffer volumeStaging = vulkan.createBuffer(volumeCount * sizeof(float), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, hostVisible);
  vulkan.writeBuffer(volumeStaging, volumeData, volumeStaging.size);
  SKVulkanBuffer topReadback = vulkan.createBuffer(8 * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, hostVisible);

  auto destroyImages = [&]() {
    vulkan.destroyBuffer(volumeStaging);
    vulkan.destroyBuffer(topReadback);
    vulkan.destroyImage(rawData);
    vulkan.destroyImage(cubeIndex);
    for (SKVulkanImage3D &image : hpImages)
    {
      vulkan.destroyImage(image);
    }
  };

  try
  {
    VkCommandBuffer commandBuffer = vulkan.beginCommands();
    SKVulkan::imageBarrier(commandBuffer, rawData.image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkBufferImageCopy volumeCopy{};
    volumeCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    volumeCopy.imageSubresource.layerCount = 1;
    volumeCopy.imageExtent = {size, size, size};
    vkCmdCopyBufferToImage(commandBuffer, volumeStaging.buffer, rawData.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &volumeCopy);
    SKVulkan::imageBarrier(commandBuffer, rawData.image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    for (SKVulkanImage3D &image : hpImages)
    {
      SKVulkan::imageBarrier(commandBuffer, image.image, 0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    SKVulkan::imageBarrier(commandBuffer, cubeIndex.image, 0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    VkDescriptorSet classifySet = vulkan.allocateDescriptorSet(_classifyLayout);
    writeStorageImage(vulkan.device(), classifySet, 0, hpImages[0].view);
    writeStorageImage(vulkan.device(), classifySet, 1, cubeIndex.view);
    writeCombinedImage(vulkan.device(), classifySet, 2, vulkan.nearestSampler(), rawData.view,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    ClassifyPushConstants classifyPush{};
    classifyPush.dimensions[0] = dimensions.x;
    classifyPush.dimensions[1] = dimensions.y;
    classifyPush.dimensions[2] = dimensions.z;
    classifyPush.dimensions[3] = 1;
    classifyPush.isolevel = float(isoValue);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _classifyPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _classifyPipelineLayout, 0, 1, &classifySet, 0,
                            nullptr);
    vkCmdPushConstants(commandBuffer, _classifyPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ClassifyPushConstants),
                       &classifyPush);
    vkCmdDispatch(commandBuffer, dispatchCount(size, kClassifyLocalSize), dispatchCount(size, kClassifyLocalSize),
                  dispatchCount(size, kClassifyLocalSize));

    for (size_t i = 0; i + 1 < hpImages.size(); ++i)
    {
      SKVulkan::imageBarrier(commandBuffer, hpImages[i].image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                             VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
      VkDescriptorSet constructSet = vulkan.allocateDescriptorSet(_constructLayout);
      writeCombinedImage(vulkan.device(), constructSet, 0, vulkan.nearestSampler(), hpImages[i].view, VK_IMAGE_LAYOUT_GENERAL);
      writeStorageImage(vulkan.device(), constructSet, 1, hpImages[i + 1].view);
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _constructPipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _constructPipelineLayout, 0, 1, &constructSet, 0,
                              nullptr);
      const uint32_t levelSize = hpImages[i + 1].width;
      vkCmdDispatch(commandBuffer, dispatchCount(levelSize, kClassifyLocalSize), dispatchCount(levelSize, kClassifyLocalSize),
                    dispatchCount(levelSize, kClassifyLocalSize));
    }

    SKVulkanImage3D &top = hpImages.back();
    SKVulkan::imageBarrier(commandBuffer, top.image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                           VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkBufferImageCopy topCopy{};
    topCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    topCopy.imageSubresource.layerCount = 1;
    topCopy.imageExtent = {2, 2, 2};
    vkCmdCopyImageToBuffer(commandBuffer, top.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, topReadback.buffer, 1, &topCopy);
    SKVulkan::imageBarrier(commandBuffer, top.image, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    vulkan.submitAndWait(commandBuffer);

    uint32_t sum[8] = {};
    vulkan.readBuffer(topReadback, sum, sizeof(sum));
    int numberOfTriangles = 0;
    for (uint32_t value : sum)
    {
      numberOfTriangles += int(value);
    }

    std::vector<float4> triangleData(static_cast<size_t>(numberOfTriangles) * 9, float4());
    if (numberOfTriangles == 0)
    {
      destroyImages();
      return triangleData;
    }

    SKVulkanBuffer vbo = vulkan.createBuffer(triangleData.size() * sizeof(float4), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, hostVisible);
    vulkan.writeBuffer(vbo, triangleData.data(), vbo.size);

    VkDescriptorSet traverseSet = vulkan.allocateDescriptorSet(_traverseLayout);
    writeCombinedImage(vulkan.device(), traverseSet, 0, vulkan.nearestSampler(), rawData.view,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VkDescriptorImageInfo hpInfos[kMaxHpLevels]{};
    for (int i = 0; i < kMaxHpLevels; ++i)
    {
      hpInfos[i].sampler = vulkan.nearestSampler();
      hpInfos[i].imageView = (i < int(hpImages.size())) ? hpImages[static_cast<size_t>(i)].view : vulkan.dummyUint3DView();
      hpInfos[i].imageLayout = (i < int(hpImages.size())) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    VkWriteDescriptorSet hpWrite{};
    hpWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    hpWrite.dstSet = traverseSet;
    hpWrite.dstBinding = 1;
    hpWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    hpWrite.descriptorCount = kMaxHpLevels;
    hpWrite.pImageInfo = hpInfos;
    vkUpdateDescriptorSets(vulkan.device(), 1, &hpWrite, 0, nullptr);
    writeCombinedImage(vulkan.device(), traverseSet, 11, vulkan.nearestSampler(), cubeIndex.view, VK_IMAGE_LAYOUT_GENERAL);
    VkDescriptorBufferInfo vboInfo{vbo.buffer, 0, vbo.size};
    VkWriteDescriptorSet vboWrite{};
    vboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vboWrite.dstSet = traverseSet;
    vboWrite.dstBinding = 12;
    vboWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vboWrite.descriptorCount = 1;
    vboWrite.pBufferInfo = &vboInfo;
    vkUpdateDescriptorSets(vulkan.device(), 1, &vboWrite, 0, nullptr);

    TraversePushConstants traversePush{};
    traversePush.dimensions[0] = dimensions.x;
    traversePush.dimensions[1] = dimensions.y;
    traversePush.dimensions[2] = dimensions.z;
    traversePush.dimensions[3] = 1;
    traversePush.isolevel = float(isoValue);
    traversePush.triangleCount = numberOfTriangles;
    traversePush.hpLevelCount = int32_t(hpImages.size());

    commandBuffer = vulkan.beginCommands();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _traversePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _traversePipelineLayout, 0, 1, &traverseSet, 0,
                            nullptr);
    vkCmdPushConstants(commandBuffer, _traversePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(TraversePushConstants),
                       &traversePush);
    vkCmdDispatch(commandBuffer, dispatchCount(static_cast<uint32_t>(numberOfTriangles), kTraverseWorkgroupSize), 1, 1);
    vulkan.submitAndWait(commandBuffer);
    vulkan.readBuffer(vbo, triangleData.data(), vbo.size);
    vulkan.destroyBuffer(vbo);
    destroyImages();
    return triangleData;
  }
  catch (...)
  {
    destroyImages();
    throw;
  }
}

std::vector<float4> SKComputeIsosurface::computeIsosurfaceCPUImplementation(int3 dimensions, std::vector<float> *voxels, double isoValue) noexcept
{
  const int largestSize = std::max({dimensions.x, dimensions.y, dimensions.z});
  int powerOfTwo = 1;
  while (largestSize > (1 << powerOfTwo))
  {
    ++powerOfTwo;
  }
  const size_t size = static_cast<size_t>(1 << powerOfTwo);

  MarchingCubes cube(dimensions.x, dimensions.y, dimensions.z);
  cube.init_all();
  for (int i = 0; i < dimensions.x; ++i)
  {
    for (int j = 0; j < dimensions.y; ++j)
    {
      for (int k = 0; k < dimensions.z; ++k)
      {
        const double value = (*voxels)[static_cast<size_t>(i) + size * static_cast<size_t>(j) + size * size * static_cast<size_t>(k)];
        cube.set_data(value, i, j, k);
      }
    }
  }
  cube.run(isoValue);

  const int numberOfTriangles = cube.ntrigs();
  std::vector<float4> data;
  data.reserve(static_cast<size_t>(3 * 3 * numberOfTriangles));
  for (int i = 0; i < numberOfTriangles; ++i)
  {
    const Triangle *tri = cube.trig(i);
    const Vertex *vertex1 = cube.vert(tri->v1);
    data.push_back(float4(float(vertex1->x / dimensions.x), float(vertex1->y / dimensions.y), float(vertex1->z / dimensions.z), 1.0f));
    data.push_back(float4(float(vertex1->nx), float(vertex1->ny), float(vertex1->nz), 0.0f));
    data.push_back(float4(0.0f, 0.0f, 0.0f, 0.0f));
    const Vertex *vertex2 = cube.vert(tri->v2);
    data.push_back(float4(float(vertex2->x / dimensions.x), float(vertex2->y / dimensions.y), float(vertex2->z / dimensions.z), 1.0f));
    data.push_back(float4(float(vertex2->nx), float(vertex2->ny), float(vertex2->nz), 0.0f));
    data.push_back(float4(0.0f, 0.0f, 0.0f, 0.0f));
    const Vertex *vertex3 = cube.vert(tri->v3);
    data.push_back(float4(float(vertex3->x / dimensions.x), float(vertex3->y / dimensions.y), float(vertex3->z / dimensions.z), 1.0f));
    data.push_back(float4(float(vertex3->nx), float(vertex3->ny), float(vertex3->nz), 0.0f));
    data.push_back(float4(0.0f, 0.0f, 0.0f, 0.0f));
  }
  return data;
}
