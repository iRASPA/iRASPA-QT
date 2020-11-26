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

#include "skopenclvoidfractionunitcell.h"
#include <iostream>
#include <algorithm>
#include <QDebug>

SKOpenCLVoidFractionUnitCell::SKOpenCLVoidFractionUnitCell(): _isOpenCLInitialized(false), _isOpenCLReady(false)
{

}


void SKOpenCLVoidFractionUnitCell::initialize(bool isOpenCLInitialized, cl_context context, cl_device_id device_id, cl_command_queue queue)
{
  cl_int err;

  _isOpenCLInitialized = isOpenCLInitialized;
  if(!_isOpenCLInitialized)
  {
    return;
  }

  _queue = queue;
  _clContext = context;
  _clCommandQueue = queue;
  _clDeviceId = device_id;

  const char* shaderSourceCode = SKOpenCLVoidFractionUnitCell::_voidFractionKernelSource;
  _program = clCreateProgramWithSource(context, 1, &shaderSourceCode, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    QString message = QString("OpenCL voidfraction: Failed to create program (error: %1)").arg(QString::number(err));
    _logReporter->logMessage(LogReporting::ErrorLevel::error, message);
    return;
  }

  // Build the program executable
  err = clBuildProgram(_program, 0, nullptr, nullptr, &SKOpenCLVoidFractionUnitCell::callBack, this);
  if (err != CL_SUCCESS)
  {
    size_t len;
    char buffer[2048];
    clGetProgramBuildInfo(_program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    QString message = QString("OpenCL voidfraction: Failed to build program (error: %1)").arg(QString::fromUtf8(buffer));
    _logReporter->logMessage(LogReporting::ErrorLevel::error, message);
  }
}

void SKOpenCLVoidFractionUnitCell::callBack(cl_program program, void *user_data)
{
  cl_int err;

  SKOpenCLVoidFractionUnitCell *ptr = reinterpret_cast<SKOpenCLVoidFractionUnitCell*>(user_data);

  size_t len;
  char buffer[2048];
  cl_build_status bldstatus;
  clGetProgramBuildInfo(program, ptr->_clDeviceId, CL_PROGRAM_BUILD_STATUS, sizeof(bldstatus), (void *)&bldstatus, &len);

  switch(bldstatus)
  {
    case CL_BUILD_NONE:
      ptr->_logReporter->logMessage(LogReporting::ErrorLevel::error, "OpenCL voidfraction: No build was performed");
      break;
    case CL_BUILD_ERROR:
      {
        clGetProgramBuildInfo(program, ptr->_clDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        QString message = QString("OpenCL voidfraction: Failed to build program (error: %1)").arg(QString::fromUtf8(buffer));
        ptr->_logReporter->logMessage(LogReporting::ErrorLevel::error, message);
      }
      break;
    case CL_BUILD_SUCCESS:
      ptr->_logReporter->logMessage(LogReporting::ErrorLevel::info, "OpenCL voidfraction: Build success");
      break;
    case CL_BUILD_IN_PROGRESS:
      ptr->_logReporter->logMessage(LogReporting::ErrorLevel::warning, "OpenCL voidfraction: Build still in progress");
      break;
  }


  ptr->_kernel = clCreateKernel(program, "ComputeVoidFraction", &err);
  if (err != CL_SUCCESS)
  {
    QString message = QString("OpenCL voidfraction: Failed to create OpenCL ComputeVoidFraction kernel (error: %1)").arg(QString::number(err));
    ptr->_logReporter->logMessage(LogReporting::ErrorLevel::error, message);
    return;
  }

  err = clGetKernelWorkGroupInfo(ptr->_kernel, ptr->_clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &ptr->_workGroupSize, nullptr);
  if (err != CL_SUCCESS)
  {
    QString message = QString("OpenCL voidfraction: Failed in clGetKernelWorkGroupInfo (error: %1)").arg(QString::number(err));
    ptr->_logReporter->logMessage(LogReporting::ErrorLevel::error, message);
    return;
  }

  QString message = QString("OpenCL voidfraction: work-group size set to %1").arg(QString::number(ptr->_workGroupSize));
  ptr->_logReporter->logMessage(LogReporting::ErrorLevel::verbose, message);

  ptr->_isOpenCLReady = true;
}

double SKOpenCLVoidFractionUnitCell::computeVoidFraction(std::vector<cl_float>* voxels)
{
  cl_int err;

  if(!_isOpenCLInitialized)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::error, "OpenCL voidfraction: OpenCL not available");
    return 0.0;
  }

  if(!_isOpenCLReady)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::error, "OpenCL voidfraction: OpenCL not yet ready");
    return 0.0;
  }

  // make sure the the global work size is an multiple of the work group size
  size_t temp = voxels->size();
  size_t numberOfGridPoints = (temp  + _workGroupSize-1) & ~(_workGroupSize-1);
  size_t global_work_size = numberOfGridPoints;


  // Transfer dataset to device
  cl_mem rawData = clCreateBuffer(_clContext,  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(cl_float) * voxels->size(), voxels->data(), &err);
  if (err != CL_SUCCESS)
  {
    QString message = QString("OpenCL voidfraction: error in clCreateBuffer (error: %1)").arg(QString::number(err));
    _logReporter->logMessage(LogReporting::ErrorLevel::error, message);
    return 0.0;
  }

  size_t nWorkGroups = numberOfGridPoints/_workGroupSize;

  // Allocate cumulative error array
  float* sumReduction = new float[nWorkGroups];
  cl_mem reductionBuffer = clCreateBuffer(_clContext, CL_MEM_READ_ONLY, nWorkGroups*sizeof(float), nullptr, &err);

  err = clEnqueueWriteBuffer(_clCommandQueue, reductionBuffer, CL_TRUE, 0, nWorkGroups*sizeof(float), sumReduction, 0, nullptr, nullptr);
  if (err != CL_SUCCESS)
  {
    QString message = QString("OpenCL voidfraction: error in clEnqueueWriteBuffer (error: %1)").arg(QString::number(err));
    _logReporter->logMessage(LogReporting::ErrorLevel::error, message);
    return 0.0;
  }

  // Set the arguments of the kernel
  clSetKernelArg(_kernel, 0, sizeof(cl_mem), &rawData);
  clSetKernelArg(_kernel, 1, sizeof(cl_mem), &reductionBuffer);
  clSetKernelArg(_kernel, 2, _workGroupSize*sizeof(cl_float),nullptr);

  // Execute kernel code
  err = clEnqueueNDRangeKernel(_clCommandQueue, _kernel, 1, nullptr, &global_work_size, &_workGroupSize, 0, nullptr, nullptr);
  if (err != CL_SUCCESS)
  {
    QString message = QString("OpenCL voidfraction: error in clEnqueueNDRangeKernel (error: %1)").arg(QString::number(err));
    _logReporter->logMessage(LogReporting::ErrorLevel::error, message);
    return 0.0;
  }

     // Read the buffer back to the array
  err = clEnqueueReadBuffer(_clCommandQueue, reductionBuffer, CL_TRUE, 0, nWorkGroups*sizeof(float), sumReduction, 0, nullptr, nullptr);
  if (err != CL_SUCCESS)
  {
    QString message = QString("OpenCL voidfraction: error in clEnqueueReadBuffer (error: %1)").arg(QString::number(err));
    _logReporter->logMessage(LogReporting::ErrorLevel::error, message);
    return 0.0;
  }

  clFinish(_clCommandQueue);

  // Final summation with CPU
  double fraction = 0.0;
  for (size_t i=0; i<nWorkGroups; i++)
    fraction += double(sumReduction[i]);

  clReleaseMemObject(rawData);
  clReleaseMemObject(reductionBuffer);

  return fraction/(128.0*128.0*128.0);
}

const char* SKOpenCLVoidFractionUnitCell::_voidFractionKernelSource = R"foo(
__kernel void ComputeVoidFraction (__global const float *input,
                           __global float *partialSums,
                           __local float *localSums)
{
   size_t local_id = get_local_id(0);
   size_t global_id = get_global_id(0);
   size_t group_size = get_local_size(0);
   size_t group_id = get_group_id(0);

   // Copy from global memory to local memory
   localSums[local_id] = exp(-(1.0f/298.0f)*input[global_id]);

   // Loop for computing localSums
   for (uint stride = group_size/2; stride>0; stride/=2)
   {
      // Waiting for each 2x2 addition into given workgroup
      barrier(CLK_LOCAL_MEM_FENCE);

      // Divide WorkGroup into 2 parts and add elements 2 by 2
      // between local_id and local_id + stride
      if (local_id < stride)
        localSums[local_id] += localSums[local_id + stride];
   }

   // Write result into partialSums[nWorkGroups]
   if (local_id == 0)
     partialSums[group_id] = localSums[0];
}
)foo";

