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

#include <QtGlobal>
#include <QStringList>
#include <array>
#include <vector>
#include <optional>
#include "skopencl.h"
#include <mathkit.h>

#define CL_TARGET_OPENCL_VERSION 120
#ifdef Q_OS_MACOS
  #include <OpenCL/opencl.h>
#elif _WIN32
  #include <CL/cl.h>
#else
  #include <CL/opencl.h>
#endif

class SKComputeEnergyGrid: public SKOpenCL
{
public:
    enum class GridSizeType: qint64
    {
      custom = 0,
      size2x2x2 = 1,
      size4x4x4 = 2,
      size8x8x8 = 3,
      size16x16x16 = 4,
      size32x32x32 = 5,
      size64x64x64 = 6,
      size128x128x128 = 7,
      size256x256x256 = 8,
      size512x512x512 = 9,
      multiple_values = 10
    };

  static std::vector<cl_float> ComputeEnergyGrid(int3 size, double2 probeParameter,
                                          std::vector<double3> positions, std::vector<double2> potentialParameters,
                                          double3x3 unitCell, int3 numberOfReplicas);
private:
  SKComputeEnergyGrid();
  cl_program _program;
  cl_kernel _kernel;
  size_t _workGroupSize;
  static const char* _energyGridKernel;

  static SKComputeEnergyGrid& getInstance()
  {
    static SKComputeEnergyGrid  instance;
    return instance;
  }
  std::vector<cl_float> ComputeEnergyGridImpl(int3 size, double2 probeParameter,
                                              std::vector<double3> positions, std::vector<double2> potentialParameters,
                                              double3x3 unitCell, int3 numberOfReplicas);
public:
  SKComputeEnergyGrid(SKOpenCL const&) = delete;
  void operator=(SKComputeEnergyGrid const&) = delete;
};
