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
  #include <OpenGL/OpenGL.h>
#elif _WIN32
  #include <CL/cl.h>
  #include <CL/cl_gl.h>
#else
  #include <CL/opencl.h>
#endif

class SKComputeIsosurface: public SKOpenCL
{
public:

  static std::vector<float4> computeIsosurface(int3 dimensions, std::vector<cl_float>* voxels, double isoValue);
private:
  SKComputeIsosurface();
  ~SKComputeIsosurface();
  cl_program _program;
  cl_kernel _constructHPLevelKernel;
  cl_kernel _classifyCubesKernel;
  cl_kernel _traverseHPKernel[10];
  size_t _constructHPLevelKernelWorkGroupSize;
  size_t _classifyCubesKernelWorkGroupSize;
  size_t _traverseHPKernelWorkGroupSize[10];
  static std::string _marchingCubesKernel;

  static SKComputeIsosurface& getInstance()
  {
    static SKComputeIsosurface  instance;
    return instance;
  }
  std::vector<float4> computeIsosurfaceImpl(int3 dimensions, std::vector<cl_float>* voxels, double isoValue);
public:
  SKComputeIsosurface(SKOpenCL const&) = delete;
  void operator=(SKComputeIsosurface const&) = delete;
};
