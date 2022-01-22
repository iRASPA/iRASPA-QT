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
#include <QString>

SKComputeEnergyGrid::SKComputeEnergyGrid(): SKOpenCL()
{
  if(_isOpenCLInitialized)
  {
    cl_int err;
    const char* shaderSourceCode = SKComputeEnergyGrid::_energyGridKernel;
    _program = clCreateProgramWithSource(_clContext, 1, &shaderSourceCode, nullptr, &err);
    if (err != CL_SUCCESS) {throw "clCreateProgramWithSource failed";}

    // Build the program executable
    err = clBuildProgram(_program, 0, nullptr, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) 
    {
      size_t len;
      char buffer[2048];
      clGetProgramBuildInfo(_program, _clDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
      QString message = QString("SKComputeEnergyGrid: Failed to build program (error: %1)").arg(QString::fromUtf8(buffer));
      qDebug() << message << _clDeviceId << err;
      throw "clBuildProgram failed";
    }

    _kernel = clCreateKernel(_program, "ComputeEnergyGrid", &err);
    if (err != CL_SUCCESS) {throw "clCreateKernel failed";}

    err = clGetKernelWorkGroupInfo(_kernel, _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_workGroupSize, nullptr);
    if (err != CL_SUCCESS) {throw "clGetKernelWorkGroupInfo failed";}

    _isOpenCLReady = true;
  }
};

std::vector<cl_float> SKComputeEnergyGrid::ComputeEnergyGrid(int3 size, double2 probeParameter,
                                        std::vector<double3> positions, std::vector<double2> potentialParameters,
                                        double3x3 unitCell, int3 numberOfReplicas)
{
    return getInstance().ComputeEnergyGridImpl(size, probeParameter, positions, potentialParameters, unitCell, numberOfReplicas);
}

std::vector<cl_float> SKComputeEnergyGrid::ComputeEnergyGridImpl(int3 size, double2 probeParameter,
                                                                           std::vector<double3> positions, std::vector<double2> potentialParameters,
                                                                           double3x3 unitCell, int3 numberOfReplicas)
{
  size_t numberOfAtoms = positions.size();
  int temp = size.x*size.y*size.z;
  cl_int err = 0;

  if(!_isOpenCLReady) {throw "OpenCL not ready";}

  // make sure the the global work size is an multiple of the work group size
  // (detected on NVIDIA)
  size_t numberOfGridPoints = (temp  + _workGroupSize-1) & ~(_workGroupSize-1);
  size_t global_work_size = numberOfGridPoints;

  std::vector<cl_float4> pos(numberOfAtoms);
  std::vector<cl_float> epsilon(numberOfAtoms);
  std::vector<cl_float> sigma(numberOfAtoms);

  std::vector<cl_float> outputData = std::vector<cl_float>(numberOfGridPoints);

  std::vector<cl_float4> gridPositions(numberOfGridPoints);
  std::vector<cl_float> output(numberOfGridPoints);

  double3 correction = double3(1.0/double(numberOfReplicas.x), 1.0/double(numberOfReplicas.y), 1.0/double(numberOfReplicas.z));

  if (numberOfAtoms > 0)
  {
    for(size_t i=0 ; i<numberOfAtoms; i++)
    {
      double3 position = correction * positions[i];
      double2 currentPotentialParameters = potentialParameters[i];

      // fill in the Cartesian position
      pos[i] = {{cl_float(position.x),cl_float(position.y),cl_float(position.z),0.0f}};

      // use 4 x epsilon for a probe epsilon of unity
      epsilon[i] = cl_float(4.0*sqrt(currentPotentialParameters.x * probeParameter.x));

      // mixing rule for the atom and the probe
      sigma[i] = cl_float(0.5 * (currentPotentialParameters.y + probeParameter.y));
    }

    int index = 0;
    for(int k=0; k<size.z;k++)
    {
      for(int j=0; j<size.y; j++)
      {
        // X various the fastest (contiguous in x)
        for(int i=0 ; i<size.x;i++)
        {
          double3 position = correction * double3(double(i)/double(size.x-1),double(j)/double(size.y-1),double(k)/double(size.z-1));
          gridPositions[index] = {{cl_float(position.x),cl_float(position.y),cl_float(position.z),cl_float(0.0)}};
          index += 1;
        }
      }
    }

    cl_mem inputPos = clCreateBuffer(_clContext,  CL_MEM_READ_ONLY,  sizeof(float4) * pos.size(), nullptr, &err);
    if (err != CL_SUCCESS) {throw "clCreateBuffer failed";}

    err = clEnqueueWriteBuffer(_clCommandQueue, inputPos, CL_TRUE, 0, sizeof(float4) * pos.size(), pos.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw "_clCommandQueue failed";}

    cl_mem inputGridPos = clCreateBuffer(_clContext, CL_MEM_READ_ONLY,  sizeof(cl_float4) * gridPositions.size(), nullptr, &err);
    if (err != CL_SUCCESS) {throw "clCreateBuffer failed";}

    err = clEnqueueWriteBuffer(_clCommandQueue, inputGridPos, CL_TRUE, 0, sizeof(cl_float4) * gridPositions.size(), gridPositions.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw "clEnqueueWriteBuffer failed";}

    cl_mem inputEpsilon = clCreateBuffer(_clContext, CL_MEM_READ_ONLY,  sizeof(cl_float) * epsilon.size(), nullptr, &err);
    if (err != CL_SUCCESS) {throw "clCreateBuffer failed";}

    err = clEnqueueWriteBuffer(_clCommandQueue, inputEpsilon, CL_TRUE, 0, sizeof(cl_float) * epsilon.size(), epsilon.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw "clEnqueueWriteBuffer failed";}

    cl_mem inputSigma = clCreateBuffer(_clContext, CL_MEM_READ_ONLY, sizeof(cl_float) * sigma.size(), nullptr, &err);
    if (err != CL_SUCCESS) {throw "clCreateBuffer failed";}

    err = clEnqueueWriteBuffer(_clCommandQueue, inputSigma, CL_TRUE, 0, sizeof(cl_float) * sigma.size(), sigma.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw "_clCommandQueue failed";}

    // set work-item dimensions
    size_t totalNumberOfReplicas = numberOfReplicas.x * numberOfReplicas.y * numberOfReplicas.z;
    cl_int clNumberOfReplicas =  cl_int(totalNumberOfReplicas);
    std::vector<cl_float4> replicaVector(totalNumberOfReplicas);
    index = 0;
    for(int i=0; i<numberOfReplicas.x; i++)
    {
      for(int j=0; j<numberOfReplicas.y; j++)
      {
        for(int k=0; k<numberOfReplicas.z; k++)
        {
          replicaVector[index] = {
                   {cl_float(double(i)/double(numberOfReplicas.x)),
                   cl_float(double(j)/double(numberOfReplicas.y)),
                   cl_float(double(k)/double(numberOfReplicas.z)),
                   cl_float(0.0)}};
          index += 1;
        }
      }
    }

    // allocate xpos memory and queue it to the device
    cl_mem replicaCellBuffer = clCreateBuffer(_clContext, CL_MEM_READ_ONLY,  sizeof(cl_float4) * replicaVector.size(), nullptr, &err);
    if (err != CL_SUCCESS) {throw "clCreateBuffer failed";}

    err = clEnqueueWriteBuffer(_clCommandQueue, replicaCellBuffer, CL_TRUE, 0, sizeof(cl_float4) * replicaVector.size(), replicaVector.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw "clEnqueueWriteBuffer failed";}

    // allocate memory for the output and queue it to the device
    cl_mem outputMemory = clCreateBuffer(_clContext, CL_MEM_READ_WRITE, sizeof(cl_float) * output.size(), nullptr, &err);
    if (err != CL_SUCCESS) {throw "clCreateBuffer failed";}

    err = clEnqueueWriteBuffer(_clCommandQueue, outputMemory, CL_TRUE, 0, sizeof(cl_float) * output.size(), output.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw "clEnqueueWriteBuffer failed";}

    double3x3 replicaCell = double3x3(double(numberOfReplicas.x) * unitCell[0],
                                      double(numberOfReplicas.y) * unitCell[1],
                                      double(numberOfReplicas.z) * unitCell[2]);

    cl_float4 clCella = {{cl_float(replicaCell[0][0]), cl_float(replicaCell[1][0]), cl_float(replicaCell[2][0]), cl_float(0.0)}};
    cl_float4 clCellb = {{cl_float(replicaCell[0][1]), cl_float(replicaCell[1][1]), cl_float(replicaCell[2][1]), cl_float(0.0)}};
    cl_float4 clCellc = {{cl_float(replicaCell[0][2]), cl_float(replicaCell[1][2]), cl_float(replicaCell[2][2]), cl_float(0.0)}};

    size_t unitsOfWorkDone = 0;
    size_t sizeOfWorkBatch = 4096;
    while(unitsOfWorkDone < positions.size())
    {
      size_t numberOfAtomsPerThreadgroup = std::min(sizeOfWorkBatch, int(positions.size())-unitsOfWorkDone);

      cl_int startIndex = cl_int(unitsOfWorkDone);
      cl_int endIndex = cl_int(unitsOfWorkDone + numberOfAtomsPerThreadgroup);
      err  = clSetKernelArg(_kernel,  0, sizeof(cl_mem), &inputPos);
      err |= clSetKernelArg(_kernel,  1, sizeof(cl_mem), &inputGridPos);
      err |= clSetKernelArg(_kernel,  2, sizeof(cl_mem), &inputEpsilon);
      err |= clSetKernelArg(_kernel,  3, sizeof(cl_mem), &inputSigma);
      err |= clSetKernelArg(_kernel,  4, sizeof(cl_mem), &replicaCellBuffer);
      err |= clSetKernelArg(_kernel,  5, sizeof(cl_mem), &outputMemory);
      err |= clSetKernelArg(_kernel,  6, sizeof(cl_int), &clNumberOfReplicas);
      err |= clSetKernelArg(_kernel,  7, sizeof(cl_float4), &clCella);
      err |= clSetKernelArg(_kernel,  8, sizeof(cl_float4), &clCellb);
      err |= clSetKernelArg(_kernel,  9, sizeof(cl_float4), &clCellc);
      err |= clSetKernelArg(_kernel,  10, sizeof(cl_int), &startIndex);
      err |= clSetKernelArg(_kernel,  11, sizeof(cl_int), &endIndex);
      err |= clEnqueueNDRangeKernel(_clCommandQueue, _kernel, 1, nullptr, &global_work_size, &_workGroupSize, 0, nullptr, nullptr);
      if (err != CL_SUCCESS) {throw "clEnqueueNDRangeKernel failed";}

      clFinish(_clCommandQueue);

      unitsOfWorkDone += sizeOfWorkBatch;
    }

    // read output image using SAME size as before
    err = clEnqueueReadBuffer(_clCommandQueue, outputMemory, CL_TRUE, 0, sizeof(cl_float) * outputData.size(), outputData.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw "_clCommandQueue failed";}

    clFinish(_clCommandQueue);

    outputData.resize(size.x * size.y * size.z);

    clReleaseMemObject(inputPos);
    clReleaseMemObject(inputGridPos);
    clReleaseMemObject(inputEpsilon);
    clReleaseMemObject(inputSigma);
    clReleaseMemObject(replicaCellBuffer);
    clReleaseMemObject(outputMemory);
  }
  return outputData;
}


const char* SKComputeEnergyGrid::_energyGridKernel = R"foo(
__kernel void ComputeEnergyGrid(__global float4 *position,
                                __global float4 *gridposition,
                                __global float *epsilon,
                                __global float *sigma,
                                __global float4 *replicaCell,
                                __global float *output,
                                const int numberOfReplicas,
                                const float4 cella,
                                const float4 cellb,
                                const float4 cellc,
                                const int startIndexAtoms,
                                const int endIndexAtoms)
{
  int igrid = get_global_id(0);
  int lsize = get_local_size(0);
  int lid = get_local_id(0);

  int iatom;
  float value = 0.0f;
  float4 s,t,dr,pos;

  float4 gridpos = gridposition[igrid];

  for(int j=0;j<numberOfReplicas;j++)
  {
    for( iatom = startIndexAtoms; iatom < endIndexAtoms; iatom++)
    {
      pos = position[iatom];

      dr = gridpos - pos - replicaCell[j];

      t = dr - rint(dr);

      dr.x = dot(cella,t);
      dr.y = dot(cellb,t);
      dr.z = dot(cellc,t);
      dr.w = 0.0f;

      float size = sigma[iatom];

      float rr = dot(dr,dr);
      if (rr<12.0f*12.0f)
      {
        float temp = size*size/rr;
        float rri3 = temp * temp * temp;
        value += epsilon[iatom]*(rri3*(rri3-1.0f));
      }
    }
  }

  output[ igrid ] += min(value,10000000.0f);
}
)foo";

