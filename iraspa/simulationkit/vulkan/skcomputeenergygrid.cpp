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

#include "skcomputeenergygrid.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

#include <QDebug>
#include <QString>

namespace
{
constexpr uint32_t kEnergyWorkgroupSize = 64;
constexpr size_t kAtomBatchSize = 4096;

struct EnergyPushConstants
{
  float cella[4];
  float cellb[4];
  float cellc[4];
  int32_t numberOfReplicas;
  int32_t startIndexAtoms;
  int32_t endIndexAtoms;
  int32_t numberOfGridPoints;
};

VkWriteDescriptorSet storageWrite(VkDescriptorSet set, uint32_t binding, const VkDescriptorBufferInfo *info)
{
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = set;
  write.dstBinding = binding;
  write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  write.descriptorCount = 1;
  write.pBufferInfo = info;
  return write;
}
} // namespace

SKComputeEnergyGrid::SKComputeEnergyGrid()
{
  SKVulkan &vulkan = SKVulkan::getInstance();
  if (!vulkan.isReady())
  {
    return;
  }

  try
  {
    VkDescriptorSetLayoutBinding bindings[6]{};
    for (uint32_t i = 0; i < 6; ++i)
    {
      bindings[i].binding = i;
      bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      bindings[i].descriptorCount = 1;
      bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 6;
    layoutInfo.pBindings = bindings;
    if (vkCreateDescriptorSetLayout(vulkan.device(), &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("energy grid descriptor layout");
    }

    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushRange.size = sizeof(EnergyPushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;
    if (vkCreatePipelineLayout(vulkan.device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("energy grid pipeline layout");
    }

    _pipeline = vulkan.createComputePipeline(_pipelineLayout, QStringLiteral(":/simulationkit/shaders/energy_grid.comp.spv"));
    _ready = true;
  }
  catch (const std::exception &error)
  {
    qDebug() << "SKComputeEnergyGrid: GPU pipeline unavailable:" << error.what();
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

SKComputeEnergyGrid::~SKComputeEnergyGrid()
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

std::vector<float> SKComputeEnergyGrid::computeEnergyGrid(int3 size, double2 probeParameter,
                                                          std::vector<double3> positions, std::vector<double2> potentialParameters,
                                                          double3x3 unitCell, int3 numberOfReplicas) noexcept(false)
{
  if (getInstance()._ready)
  {
    try
    {
      return getInstance().computeEnergyGridGPUImplementation(size, probeParameter, positions, potentialParameters,
                                                              unitCell, numberOfReplicas);
    }
    catch (const std::exception &error)
    {
      qDebug() << "SKComputeEnergyGrid: GPU path failed, using CPU:" << error.what();
    }
  }
  return computeEnergyGridCPUImplementation(size, probeParameter, positions, potentialParameters, unitCell, numberOfReplicas);
}

std::vector<float> SKComputeEnergyGrid::computeEnergyGridGPUImplementation(int3 size, double2 probeParameter,
                                                                           std::vector<double3> positions,
                                                                           std::vector<double2> potentialParameters,
                                                                           double3x3 unitCell, int3 numberOfReplicas)
{
  const int numberOfGridPoints = size.x * size.y * size.z;
  std::vector<float> outputData(static_cast<size_t>(numberOfGridPoints), 0.0f);
  if (positions.empty() || numberOfGridPoints <= 0)
  {
    return outputData;
  }

  SKVulkan &vulkan = SKVulkan::getInstance();
  vulkan.resetDescriptorPool();

  const size_t numberOfAtoms = positions.size();
  const double3 correction = double3(1.0 / double(numberOfReplicas.x), 1.0 / double(numberOfReplicas.y),
                                     1.0 / double(numberOfReplicas.z));

  std::vector<float4> pos(numberOfAtoms);
  std::vector<float> epsilon(numberOfAtoms);
  std::vector<float> sigma(numberOfAtoms);
  for (size_t i = 0; i < numberOfAtoms; ++i)
  {
    const double3 position = correction * positions[i];
    const double2 currentPotentialParameters = potentialParameters[i];
    pos[i] = float4(float(position.x), float(position.y), float(position.z), 0.0f);
    epsilon[i] = float(4.0 * sqrt(currentPotentialParameters.x * probeParameter.x));
    sigma[i] = float(0.5 * (currentPotentialParameters.y + probeParameter.y));
  }

  std::vector<float4> gridPositions(static_cast<size_t>(numberOfGridPoints));
  int index = 0;
  for (int k = 0; k < size.z; ++k)
  {
    for (int j = 0; j < size.y; ++j)
    {
      for (int i = 0; i < size.x; ++i)
      {
        const double3 position = correction * double3(double(i) / double(size.x - 1), double(j) / double(size.y - 1),
                                                      double(k) / double(size.z - 1));
        gridPositions[static_cast<size_t>(index)] = float4(float(position.x), float(position.y), float(position.z), 0.0f);
        ++index;
      }
    }
  }

  const int totalNumberOfReplicas = numberOfReplicas.x * numberOfReplicas.y * numberOfReplicas.z;
  std::vector<float4> replicaVector(static_cast<size_t>(totalNumberOfReplicas));
  index = 0;
  for (int i = 0; i < numberOfReplicas.x; ++i)
  {
    for (int j = 0; j < numberOfReplicas.y; ++j)
    {
      for (int k = 0; k < numberOfReplicas.z; ++k)
      {
        replicaVector[static_cast<size_t>(index)] = float4(float(double(i) / double(numberOfReplicas.x)),
                                                           float(double(j) / double(numberOfReplicas.y)),
                                                           float(double(k) / double(numberOfReplicas.z)), 0.0f);
        ++index;
      }
    }
  }

  const double3x3 replicaCell = double3x3(double(numberOfReplicas.x) * unitCell[0],
                                          double(numberOfReplicas.y) * unitCell[1],
                                          double(numberOfReplicas.z) * unitCell[2]);

  EnergyPushConstants push{};
  push.cella[0] = float(replicaCell[0][0]);
  push.cella[1] = float(replicaCell[1][0]);
  push.cella[2] = float(replicaCell[2][0]);
  push.cellb[0] = float(replicaCell[0][1]);
  push.cellb[1] = float(replicaCell[1][1]);
  push.cellb[2] = float(replicaCell[2][1]);
  push.cellc[0] = float(replicaCell[0][2]);
  push.cellc[1] = float(replicaCell[1][2]);
  push.cellc[2] = float(replicaCell[2][2]);
  push.numberOfReplicas = totalNumberOfReplicas;
  push.numberOfGridPoints = numberOfGridPoints;

  const VkMemoryPropertyFlags hostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  const VkBufferUsageFlags storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  SKVulkanBuffer posBuffer = vulkan.createBuffer(sizeof(float4) * pos.size(), storage, hostVisible);
  SKVulkanBuffer gridBuffer = vulkan.createBuffer(sizeof(float4) * gridPositions.size(), storage, hostVisible);
  SKVulkanBuffer epsilonBuffer = vulkan.createBuffer(sizeof(float) * epsilon.size(), storage, hostVisible);
  SKVulkanBuffer sigmaBuffer = vulkan.createBuffer(sizeof(float) * sigma.size(), storage, hostVisible);
  SKVulkanBuffer replicaBuffer = vulkan.createBuffer(sizeof(float4) * replicaVector.size(), storage, hostVisible);
  SKVulkanBuffer outputBuffer = vulkan.createBuffer(sizeof(float) * outputData.size(), storage, hostVisible);
  vulkan.writeBuffer(posBuffer, pos.data(), posBuffer.size);
  vulkan.writeBuffer(gridBuffer, gridPositions.data(), gridBuffer.size);
  vulkan.writeBuffer(epsilonBuffer, epsilon.data(), epsilonBuffer.size);
  vulkan.writeBuffer(sigmaBuffer, sigma.data(), sigmaBuffer.size);
  vulkan.writeBuffer(replicaBuffer, replicaVector.data(), replicaBuffer.size);
  vulkan.writeBuffer(outputBuffer, outputData.data(), outputBuffer.size);

  VkDescriptorBufferInfo bufferInfos[6]{};
  bufferInfos[0] = {posBuffer.buffer, 0, posBuffer.size};
  bufferInfos[1] = {gridBuffer.buffer, 0, gridBuffer.size};
  bufferInfos[2] = {epsilonBuffer.buffer, 0, epsilonBuffer.size};
  bufferInfos[3] = {sigmaBuffer.buffer, 0, sigmaBuffer.size};
  bufferInfos[4] = {replicaBuffer.buffer, 0, replicaBuffer.size};
  bufferInfos[5] = {outputBuffer.buffer, 0, outputBuffer.size};

  VkDescriptorSet descriptorSet = vulkan.allocateDescriptorSet(_descriptorSetLayout);
  VkWriteDescriptorSet writes[6];
  for (uint32_t i = 0; i < 6; ++i)
  {
    writes[i] = storageWrite(descriptorSet, i, &bufferInfos[i]);
  }
  vkUpdateDescriptorSets(vulkan.device(), 6, writes, 0, nullptr);

  VkCommandBuffer commandBuffer = vulkan.beginCommands();
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

  const uint32_t groupCount = (static_cast<uint32_t>(numberOfGridPoints) + kEnergyWorkgroupSize - 1) / kEnergyWorkgroupSize;
  size_t unitsOfWorkDone = 0;
  bool firstBatch = true;
  while (unitsOfWorkDone < numberOfAtoms)
  {
    const size_t numberOfAtomsPerBatch = std::min(kAtomBatchSize, numberOfAtoms - unitsOfWorkDone);
    push.startIndexAtoms = int32_t(unitsOfWorkDone);
    push.endIndexAtoms = int32_t(unitsOfWorkDone + numberOfAtomsPerBatch);
    if (!firstBatch)
    {
      SKVulkan::memoryBarrier(commandBuffer, VK_ACCESS_SHADER_WRITE_BIT,
                              VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(EnergyPushConstants), &push);
    vkCmdDispatch(commandBuffer, groupCount, 1, 1);
    firstBatch = false;
    unitsOfWorkDone += kAtomBatchSize;
  }
  vulkan.submitAndWait(commandBuffer);
  vulkan.readBuffer(outputBuffer, outputData.data(), outputBuffer.size);

  vulkan.destroyBuffer(posBuffer);
  vulkan.destroyBuffer(gridBuffer);
  vulkan.destroyBuffer(epsilonBuffer);
  vulkan.destroyBuffer(sigmaBuffer);
  vulkan.destroyBuffer(replicaBuffer);
  vulkan.destroyBuffer(outputBuffer);
  return outputData;
}

std::vector<float> SKComputeEnergyGrid::computeEnergyGridCPUImplementation(int3 size, double2 probeParameter,
                                                                           std::vector<double3> positions,
                                                                           std::vector<double2> potentialParameters,
                                                                           double3x3 unitCell, int3 numberOfReplicas) noexcept
{
  const size_t numberOfAtoms = positions.size();
  const int temp = size.x * size.y * size.z;
  std::vector<float> outputData(static_cast<size_t>(temp), 0.0f);

  const double3 correction = double3(1.0 / double(numberOfReplicas.x), 1.0 / double(numberOfReplicas.y),
                                     1.0 / double(numberOfReplicas.z));
  const double3x3 replicaCell = double3x3(double(numberOfReplicas.x) * unitCell[0],
                                          double(numberOfReplicas.y) * unitCell[1],
                                          double(numberOfReplicas.z) * unitCell[2]);

  const int totalNumberOfReplicas = numberOfReplicas.x * numberOfReplicas.y * numberOfReplicas.z;
  std::vector<double3> replicaVector(static_cast<size_t>(totalNumberOfReplicas));
  int index = 0;
  for (int i = 0; i < numberOfReplicas.x; ++i)
  {
    for (int j = 0; j < numberOfReplicas.y; ++j)
    {
      for (int k = 0; k < numberOfReplicas.z; ++k)
      {
        replicaVector[static_cast<size_t>(index)] = double3(double(i) / double(numberOfReplicas.x),
                                                            double(j) / double(numberOfReplicas.y),
                                                            double(k) / double(numberOfReplicas.z));
        ++index;
      }
    }
  }

  for (int z = 0; z < size.z; ++z)
  {
    for (int y = 0; y < size.y; ++y)
    {
      for (int x = 0; x < size.x; ++x)
      {
        const double3 gridPosition = correction * double3(double(x) / double(size.x - 1), double(y) / double(size.y - 1),
                                                          double(z) / double(size.z - 1));
        double value = 0.0;
        for (size_t i = 0; i < numberOfAtoms; ++i)
        {
          const double3 position = correction * positions[i];
          const double2 currentPotentialParameters = potentialParameters[i];
          const double epsilon = 4.0 * sqrt(currentPotentialParameters.x * probeParameter.x);
          const double sigma = 0.5 * (currentPotentialParameters.y + probeParameter.y);
          for (int j = 0; j < totalNumberOfReplicas; ++j)
          {
            double3 ds = gridPosition - position - replicaVector[static_cast<size_t>(j)];
            ds.x -= std::rint(ds.x);
            ds.y -= std::rint(ds.y);
            ds.z -= std::rint(ds.z);
            const double3 dr = replicaCell * ds;
            const double rr = dr.x * dr.x + dr.y * dr.y + dr.z * dr.z;
            if (rr < 12.0 * 12.0)
            {
              const double sigma2rr = sigma * sigma / rr;
              const double rri3 = sigma2rr * sigma2rr * sigma2rr;
              value += epsilon * (rri3 * (rri3 - 1.0));
            }
          }
        }
        outputData[static_cast<size_t>(x + y * size.x + z * size.x * size.y)] += float(std::min(value, 10000000.0));
      }
    }
  }
  return outputData;
}
