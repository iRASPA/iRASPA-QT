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

#include <vector>
#include <mathkit.h>
#include "skvulkan.h"

class SKComputeIsosurface
{
public:
  SKComputeIsosurface(const SKComputeIsosurface &) = delete;
  void operator=(const SKComputeIsosurface &) = delete;
  static std::vector<float4> computeIsosurface(int3 dimensions, std::vector<float> *voxels, double isoValue);
  static std::vector<float4> computeIsosurfaceCPUImplementation(int3 dimensions, std::vector<float> *voxels, double isoValue) noexcept;

private:
  SKComputeIsosurface();
  ~SKComputeIsosurface();

  static SKComputeIsosurface &getInstance()
  {
    static SKComputeIsosurface instance;
    return instance;
  }

  std::vector<float4> computeIsosurfaceGPUImplementation(int3 dimensions, std::vector<float> *voxels, double isoValue);

  bool _ready = false;
  VkDescriptorSetLayout _classifyLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _constructLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _traverseLayout = VK_NULL_HANDLE;
  VkPipelineLayout _classifyPipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _constructPipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _traversePipelineLayout = VK_NULL_HANDLE;
  VkPipeline _classifyPipeline = VK_NULL_HANDLE;
  VkPipeline _constructPipeline = VK_NULL_HANDLE;
  VkPipeline _traversePipeline = VK_NULL_HANDLE;
};
