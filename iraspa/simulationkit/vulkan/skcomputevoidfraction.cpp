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

#include "skcomputevoidfraction.h"

#include <cmath>
#include <stdexcept>

#include <QDebug>
#include <QString>

namespace
{
constexpr uint32_t kVoidWorkgroupSize = 256;

struct VoidPushConstants
{
  int32_t numberOfVoxels;
};
} // namespace

SKComputeVoidFraction::SKComputeVoidFraction()
{
  SKVulkan &vulkan = SKVulkan::getInstance();
  if (!vulkan.isReady())
  {
    return;
  }

  try
  {
    VkDescriptorSetLayoutBinding bindings[2]{};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;
    if (vkCreateDescriptorSetLayout(vulkan.device(), &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("void fraction descriptor layout");
    }

    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushRange.size = sizeof(VoidPushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;
    if (vkCreatePipelineLayout(vulkan.device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("void fraction pipeline layout");
    }

    _pipeline = vulkan.createComputePipeline(_pipelineLayout, QStringLiteral(":/simulationkit/shaders/void_fraction.comp.spv"));
    _ready = true;
  }
  catch (const std::exception &error)
  {
    qDebug() << "SKComputeVoidFraction: GPU pipeline unavailable:" << error.what();
    if (_pipeline)
    {
      vkDestroyPipeline(vulkan.device(), _pipeline, nullptr);
      _pipeline = VK_NULL_HANDLE;
    }
    if (_pipelineLayout)
    {
      vkDestroyPipelineLayout(vulkan.device(), _pipelineLayout, nullptr);
      _pipelineLayout = VK_NULL_HANDLE;
    }
    if (_descriptorSetLayout)
    {
      vkDestroyDescriptorSetLayout(vulkan.device(), _descriptorSetLayout, nullptr);
      _descriptorSetLayout = VK_NULL_HANDLE;
    }
  }
}

SKComputeVoidFraction::~SKComputeVoidFraction()
{
  SKVulkan &vulkan = SKVulkan::getInstance();
  if (!vulkan.isReady())
  {
    return;
  }
  if (_pipeline)
  {
    vkDestroyPipeline(vulkan.device(), _pipeline, nullptr);
  }
  if (_pipelineLayout)
  {
    vkDestroyPipelineLayout(vulkan.device(), _pipelineLayout, nullptr);
  }
  if (_descriptorSetLayout)
  {
    vkDestroyDescriptorSetLayout(vulkan.device(), _descriptorSetLayout, nullptr);
  }
}

double SKComputeVoidFraction::ComputeVoidFraction(std::vector<float> *voxels)
{
  if (getInstance()._ready)
  {
    try
    {
      return getInstance().ComputeVoidFractionGPU(voxels);
    }
    catch (const std::exception &error)
    {
      qDebug() << "SKComputeVoidFraction: GPU path failed, using CPU:" << error.what();
    }
  }
  return ComputeVoidFractionCPU(voxels);
}

double SKComputeVoidFraction::ComputeVoidFractionCPU(std::vector<float> *voxels)
{
  if (!voxels || voxels->empty())
  {
    return 0.0;
  }
  double fraction = 0.0;
  for (float value : *voxels)
  {
    fraction += std::exp(-(1.0 / 298.0) * double(value));
  }
  return fraction / (128.0 * 128.0 * 128.0);
}

double SKComputeVoidFraction::ComputeVoidFractionGPU(std::vector<float> *voxels)
{
  if (!voxels || voxels->empty())
  {
    return 0.0;
  }

  SKVulkan &vulkan = SKVulkan::getInstance();
  vulkan.resetDescriptorPool();

  const uint32_t numberOfVoxels = static_cast<uint32_t>(voxels->size());
  const uint32_t groupCount = (numberOfVoxels + kVoidWorkgroupSize - 1) / kVoidWorkgroupSize;
  std::vector<float> partialSums(groupCount, 0.0f);

  const VkMemoryPropertyFlags hostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  SKVulkanBuffer inputBuffer = vulkan.createBuffer(sizeof(float) * voxels->size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, hostVisible);
  SKVulkanBuffer reductionBuffer = vulkan.createBuffer(sizeof(float) * partialSums.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, hostVisible);
  vulkan.writeBuffer(inputBuffer, voxels->data(), inputBuffer.size);
  vulkan.writeBuffer(reductionBuffer, partialSums.data(), reductionBuffer.size);

  VkDescriptorBufferInfo inputInfo{inputBuffer.buffer, 0, inputBuffer.size};
  VkDescriptorBufferInfo reductionInfo{reductionBuffer.buffer, 0, reductionBuffer.size};
  VkDescriptorSet descriptorSet = vulkan.allocateDescriptorSet(_descriptorSetLayout);
  VkWriteDescriptorSet writes[2]{};
  writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[0].dstSet = descriptorSet;
  writes[0].dstBinding = 0;
  writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  writes[0].descriptorCount = 1;
  writes[0].pBufferInfo = &inputInfo;
  writes[1] = writes[0];
  writes[1].dstBinding = 1;
  writes[1].pBufferInfo = &reductionInfo;
  vkUpdateDescriptorSets(vulkan.device(), 2, writes, 0, nullptr);

  VoidPushConstants push{};
  push.numberOfVoxels = int32_t(numberOfVoxels);

  VkCommandBuffer commandBuffer = vulkan.beginCommands();
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
  vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VoidPushConstants), &push);
  vkCmdDispatch(commandBuffer, groupCount, 1, 1);
  vulkan.submitAndWait(commandBuffer);
  vulkan.readBuffer(reductionBuffer, partialSums.data(), reductionBuffer.size);

  vulkan.destroyBuffer(inputBuffer);
  vulkan.destroyBuffer(reductionBuffer);

  double fraction = 0.0;
  for (float value : partialSums)
  {
    fraction += double(value);
  }
  return fraction / (128.0 * 128.0 * 128.0);
}
