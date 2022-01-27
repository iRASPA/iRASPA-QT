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

// two macros ensures any macro passed will
// be expanded before being stringified
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

SKComputeIsosurface::SKComputeIsosurface(): SKOpenCL()
{
  if(_isOpenCLInitialized)
  {
    cl_int err;
    const char* shaderSourceCode = SKComputeIsosurface::_marchingCubesKernel.c_str();
    _program = clCreateProgramWithSource(_clContext, 1, &shaderSourceCode, nullptr, &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateProgramWithSource failed at " __FILE__ ":" STRINGIZE(__LINE__));}

    // Build the program executable
    err = clBuildProgram(_program, 0, nullptr, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
      size_t len;
      char buffer[2048];
      clGetProgramBuildInfo(_program, _clDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
      QString message = QString("SKComputeIsosurface: OpenCL Failed to build program at %1 (line %2 error: %3)").arg(__FILE__, QString::number(__LINE__), QString::fromUtf8(buffer));
      throw std::runtime_error(message.toStdString().c_str());
    }

    _constructHPLevelKernel = clCreateKernel(_program, "constructHPLevel", &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateProgramWithSource failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    err = clGetKernelWorkGroupInfo(_constructHPLevelKernel, _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_constructHPLevelKernelWorkGroupSize, nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clGetKernelWorkGroupInfo failed at " __FILE__ ":" STRINGIZE(__LINE__));}

    _classifyCubesKernel = clCreateKernel(_program, "classifyCubes", &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateProgramWithSource failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    err = clGetKernelWorkGroupInfo(_constructHPLevelKernel, _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_classifyCubesKernelWorkGroupSize, nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clGetKernelWorkGroupInfo failed at " __FILE__ ":" STRINGIZE(__LINE__));}

    _traverseHPKernel[4] = clCreateKernel(_program, "traverseHP16", &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    err = clGetKernelWorkGroupInfo(_traverseHPKernel[4], _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_traverseHPKernelWorkGroupSize[4], nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("cOpenCL lGetKernelWorkGroupInfo failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    _traverseHPKernel[5] = clCreateKernel(_program, "traverseHP32", &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    err = clGetKernelWorkGroupInfo(_traverseHPKernel[5], _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_traverseHPKernelWorkGroupSize[5], nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clGetKernelWorkGroupInfo failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    _traverseHPKernel[6] = clCreateKernel(_program, "traverseHP64", &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    err = clGetKernelWorkGroupInfo(_traverseHPKernel[6], _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_traverseHPKernelWorkGroupSize[6], nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clGetKernelWorkGroupInfo failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    _traverseHPKernel[7] = clCreateKernel(_program, "traverseHP128", &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    err = clGetKernelWorkGroupInfo(_traverseHPKernel[7], _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_traverseHPKernelWorkGroupSize[7], nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clGetKernelWorkGroupInfo failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    _traverseHPKernel[8] = clCreateKernel(_program, "traverseHP256", &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    err = clGetKernelWorkGroupInfo(_traverseHPKernel[8], _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_traverseHPKernelWorkGroupSize[8], nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clGetKernelWorkGroupInfo failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    _traverseHPKernel[9] = clCreateKernel(_program, "traverseHP512", &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    err = clGetKernelWorkGroupInfo(_traverseHPKernel[9], _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_traverseHPKernelWorkGroupSize[9], nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clGetKernelWorkGroupInfo failed at " __FILE__ ":" STRINGIZE(__LINE__));}

    _isOpenCLReady = true;
  }
};

SKComputeIsosurface::~SKComputeIsosurface()
{
  if(_isOpenCLReady)
  {
    clReleaseKernel(_traverseHPKernel[9]);
    clReleaseKernel(_traverseHPKernel[8]);
    clReleaseKernel(_traverseHPKernel[7]);
    clReleaseKernel(_traverseHPKernel[6]);
    clReleaseKernel(_traverseHPKernel[5]);
    clReleaseKernel(_traverseHPKernel[4]);
    clReleaseKernel(_classifyCubesKernel);
    clReleaseKernel(_constructHPLevelKernel);
    clReleaseProgram(_program);
  }
}

std::vector<float4> SKComputeIsosurface::computeIsosurface(int3 dimensions, std::vector<cl_float> *voxels, double isoValue)
{
  if(getInstance()._isOpenCLReady)
  {
    return getInstance().computeIsosurfaceGPUImplementation(dimensions, voxels, isoValue);
  }
  return SKComputeIsosurface::computeIsosurfaceCPUImplementation(dimensions, voxels, isoValue);
}

std::vector<float4> SKComputeIsosurface::computeIsosurfaceGPUImplementation(int3 dimensions, std::vector<cl_float> *voxels, double isoValue) noexcept(false)
{
  cl_int err;

  int largestSize = std::max({dimensions.x,dimensions.y,dimensions.z});
  int powerOfTwo = 1;
  while(largestSize > pow(2,powerOfTwo))
  {
    powerOfTwo += 1;
  }
  size_t size = pow(2,powerOfTwo); // the encompassing size to use as textures (size 16,32,64,128,256, and 512 are supported).

  size_t bufferSize = size;
  std::vector<cl_mem> images;
  std::vector<cl_mem> buffers;

  for(int i=1; i<powerOfTwo; i++)
  {
    cl_image_format imageFormat{};
    switch(i)
    {
    case 1:
      imageFormat = cl_image_format{CL_RGBA,CL_UNSIGNED_INT8};
      break;
    case 2:
      imageFormat = cl_image_format{CL_R, CL_UNSIGNED_INT8};
      break;
    case 3:
      imageFormat = cl_image_format{CL_R, CL_UNSIGNED_INT16};
      break;
    case 4:
      imageFormat = cl_image_format{CL_R, CL_UNSIGNED_INT16};
      break;
    default:
      imageFormat = cl_image_format{CL_R, CL_UNSIGNED_INT32};
      break;
    }

    cl_image_desc imageDescriptor = cl_image_desc{CL_MEM_OBJECT_IMAGE3D, bufferSize, bufferSize, bufferSize, 0, 0, 0, 0, 0, nullptr};
    images.push_back(clCreateImage(_clContext, CL_MEM_READ_WRITE, &imageFormat, &imageDescriptor, nullptr, &err));
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateImage failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    bufferSize /= 2;
  }

  // last one should always be CL_UNSIGNED_INT32 (because the sum is read back as integers)
  cl_image_format imageFormat = cl_image_format{CL_R, CL_UNSIGNED_INT32};
  cl_image_desc imageDescriptor = cl_image_desc{CL_MEM_OBJECT_IMAGE3D, bufferSize, bufferSize, bufferSize, 0, 0, 0, 0, 0, nullptr};
  images.push_back(clCreateImage(_clContext, CL_MEM_READ_WRITE, &imageFormat, &imageDescriptor, nullptr, &err));
  if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateImage failed at " __FILE__ ":" STRINGIZE(__LINE__));}


  // Transfer dataset to device
  imageFormat = cl_image_format{CL_R, CL_FLOAT};
  imageDescriptor = cl_image_desc{CL_MEM_OBJECT_IMAGE3D, size, size, size, 0, 0, 0, 0, 0, nullptr};
  cl_mem rawData = clCreateImage(_clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &imageFormat, &imageDescriptor, voxels->data(), nullptr);
  if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateImage failed at " __FILE__ ":" STRINGIZE(__LINE__));}


  // update scalar field
  //===================================================================================================================================

  cl_float clIsoValue = cl_float(isoValue);
  cl_int4 clDimensions = {{dimensions.x,dimensions.y,dimensions.z,1}};

  clSetKernelArg(_classifyCubesKernel,  0, sizeof(cl_mem), &images[0]);
  clSetKernelArg(_classifyCubesKernel,  1, sizeof(cl_mem), &rawData);
  clSetKernelArg(_classifyCubesKernel,  2, sizeof(cl_int4), &clDimensions);
  clSetKernelArg(_classifyCubesKernel,  3, sizeof(cl_float), &clIsoValue);


  clGetKernelWorkGroupInfo(_classifyCubesKernel, _clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &_classifyCubesKernelWorkGroupSize, nullptr);

  // set work-item dimensions
  size_t global_work_size[3] = {size, size, size};

  err = clEnqueueNDRangeKernel(_clCommandQueue, _classifyCubesKernel, 3, nullptr, global_work_size, nullptr, 0, nullptr, nullptr);
  if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clEnqueueNDRangeKernel classifyCubesKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}

  // histoPyramidConstruction
  //===================================================================================================================================

  // Run base to first level
  clSetKernelArg(_constructHPLevelKernel,  0, sizeof(cl_mem), &images[0]);
  clSetKernelArg(_constructHPLevelKernel,  1, sizeof(cl_mem), &images[1]);

  size_t global_work_size2[3] = {size/2, size/2, size/2};

  err = clEnqueueNDRangeKernel(_clCommandQueue, _constructHPLevelKernel, 3, nullptr, global_work_size2, nullptr, 0, nullptr, nullptr);
  if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clEnqueueNDRangeKernel constructHPLevelKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}


  size_t previous = size / 2;
  // Run level 2 to top level
  for(int i=1; i<int(ceil(log2(double(size)))-1);i++)
  {
    clSetKernelArg(_constructHPLevelKernel,  0, sizeof(cl_mem), &images[i]);
    clSetKernelArg(_constructHPLevelKernel,  1, sizeof(cl_mem), &images[i+1]);

    previous /= 2;
    size_t global_work_size3[3] = {previous, previous, previous};
    err = clEnqueueNDRangeKernel(_clCommandQueue, _constructHPLevelKernel, 3, nullptr, global_work_size3, nullptr, 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clEnqueueNDRangeKernel constructHPLevelKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}
  }

  // Read top of histoPyramid an use this size to allocate VBO below
  //===================================================================================================================================

  cl_int sum[8] = {0,0,0,0,0,0,0,0};
  size_t origin[3] = {0,0,0};
  size_t region[3] = {2,2,2};

  err = clEnqueueReadImage(_clCommandQueue, images[images.size()-1], CL_FALSE, origin, region, 0, 0, &sum, 0, nullptr, nullptr);
  if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clEnqueueReadImage failed at " __FILE__ ":" STRINGIZE(__LINE__));}

  clFinish(_clCommandQueue);

  cl_int sum2 = sum[0] + sum[1] + sum[2] + sum[3] + sum[4] + sum[5] + sum[6] + sum[7];
  int numberOfTriangles = int(sum2);



  // get the results and convert them to an OpenGL Vertex buffer object
  //===================================================================================================================================

  std::vector<float4> triangleData(numberOfTriangles*3*3);

  if(numberOfTriangles == 0)
  {
    return triangleData;
  }

  if(numberOfTriangles>0)
  {
    cl_int clSum = cl_int(numberOfTriangles);

    // Increase the global_work_size so that it is divideable by the workgroup-size
    int workGroupSize = _traverseHPKernelWorkGroupSize[powerOfTwo];
    size_t local_work_size[1] = {size_t(workGroupSize)};
    int globalWorkSize = sum + workGroupSize - (sum - workGroupSize*(numberOfTriangles / workGroupSize));
    size_t global_work_size4[1] = {size_t(globalWorkSize)};

    for(size_t j=0; j<images.size(); j++)
    {
      clSetKernelArg(_traverseHPKernel[powerOfTwo], static_cast<cl_uint>(j), sizeof(cl_mem), &images[j]);
    }
    clSetKernelArg(_traverseHPKernel[powerOfTwo], static_cast<cl_uint>(images.size()), sizeof(cl_mem), &rawData);

    size_t i = images.size() + 1;
    cl_mem  VBOBuffer = clCreateBuffer(_clContext, CL_MEM_READ_WRITE, triangleData.size()*sizeof(cl_float4), nullptr, &err);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clCreateBuffer failed at " __FILE__ ":" STRINGIZE(__LINE__));}
    clSetKernelArg(_traverseHPKernel[powerOfTwo], static_cast<cl_uint>(i), sizeof(cl_mem), &VBOBuffer);
    clSetKernelArg(_traverseHPKernel[powerOfTwo], static_cast<cl_uint>(i+1), sizeof(cl_int4), &clDimensions);
    clSetKernelArg(_traverseHPKernel[powerOfTwo], static_cast<cl_uint>(i+2), sizeof(cl_float), &clIsoValue);
    clSetKernelArg(_traverseHPKernel[powerOfTwo], static_cast<cl_uint>(i+3), sizeof(cl_int), &clSum);

    // Run a NDRange kernel over this buffer which traverses back to the base level
    err = clEnqueueNDRangeKernel(_clCommandQueue, _traverseHPKernel[powerOfTwo], 1, nullptr, global_work_size4, local_work_size, 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clEnqueueNDRangeKernel _traverseHPKernel failed at " __FILE__ ":" STRINGIZE(__LINE__));}

    clFinish(_clCommandQueue);

    err = clEnqueueReadBuffer(_clCommandQueue, VBOBuffer, CL_TRUE, 0, triangleData.size() * sizeof(float4), triangleData.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {throw std::runtime_error("OpenCL clEnqueueReadBuffer failed at " __FILE__ ":" STRINGIZE(__LINE__));}

    clFinish(_clCommandQueue);

    clReleaseMemObject(VBOBuffer);
  }

  clReleaseMemObject(rawData);

  for(cl_mem &image: images)
  {
    clReleaseMemObject(image);
  }

  for(cl_mem &buffer:  buffers)
  {
    clReleaseMemObject(buffer);
  }

  return triangleData;
}

std::vector<float4> SKComputeIsosurface::computeIsosurfaceCPUImplementation(int3 dimensions, std::vector<cl_float> *voxels, double isoValue) noexcept
{
  int largestSize = std::max({dimensions.x,dimensions.y,dimensions.z});
  int powerOfTwo = 1;
  while(largestSize > pow(2,powerOfTwo))
  {
    powerOfTwo += 1;
  }
  size_t size = pow(2,powerOfTwo); // the encompassing size to use as textures (size 16,32,64,128,256, and 512 are supported).

  // brute force implementation
  // Create marching cubes object
  MarchingCubes cube(dimensions.x,dimensions.y,dimensions.z);

  // Initiate the cube
  cube.init_all();

  // Set the data
  for(int i=0; i<dimensions.x; i++)
  {
    for(int j=0; j<dimensions.y; j++)
    {
      for(int k=0; k<dimensions.z; k++)
      {
        double value = (*voxels)[i + size * j + k * size * size];
        cube.set_data(value, i, j, k);
      }
    }
  }

  cube.run( isoValue );

  int numberOfTriangles = cube.ntrigs();
  std::vector<float4> data{};
  data.reserve(3*3*numberOfTriangles);

  // Fetch the info
  for(int i=0; i<cube.ntrigs(); i++)
  {
    const Triangle* tri = cube.trig(i);

    const Vertex* vertex1 = cube.vert(tri->v1);
    data.push_back(double4(vertex1->x/dimensions.x,vertex1->y/dimensions.y,vertex1->z/dimensions.z,1.0));
    data.push_back(double4(vertex1->nx,vertex1->ny,vertex1->nz,0.0));
    data.push_back(double4(0.0,0.0,0.0,0.0));

    const Vertex* vertex2 = cube.vert(tri->v2);
    data.push_back(double4(vertex2->x/dimensions.x,vertex2->y/dimensions.y,vertex2->z/dimensions.z,1.0));
    data.push_back(double4(vertex2->nx,vertex2->ny,vertex2->nz,0.0));
    data.push_back(double4(0.0,0.0,0.0,0.0));

    const Vertex* vertex3 = cube.vert(tri->v3);
    data.push_back(double4(vertex3->x/dimensions.x,vertex3->y/dimensions.y,vertex3->z/dimensions.z,1.0));
    data.push_back(double4(vertex3->nx,vertex3->ny,vertex3->nz,0.0));
    data.push_back(double4(0.0,0.0,0.0,0.0));
  }
  return data;
}



std::string SKComputeIsosurface::_marchingCubesKernel = std::string(R"foo(

#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;


// Cube description:
//         7 ________ 6           _____6__             ________
//         /|       /|         7/|       /|          /|       /|
//       /  |     /  |        /  |     /5 |        /  6     /  |
//   4 /_______ /    |      /__4____ /    10     /_______3/    |
//    |     |  |5    |     |    11  |     |     |     |  |   2 |
//    |    3|__|_____|2    |     |__|__2__|     | 4   |__|_____|
//    |    /   |    /      8   3/   9    /      |    /   |    /
//    |  /     |  /        |  /     |  /1       |  /     5  /
//    |/_______|/          |/___0___|/          |/_1_____|/
//   0          1        0          1
//        Nodes                Borders               Faces


__constant int4 cubeOffsets[8] =
{
  {0, 0, 0, 0},
  {1, 0, 0, 0},
  {0, 0, 1, 0},
  {1, 0, 1, 0},
  {0, 1, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 1, 0},
  {1, 1, 1, 0}
};

__constant char  offsets3[72] =
{
  // 0
  0,0,0,
  1,0,0,
  // 1
  1,0,0,
  1,0,1,
  // 2
  1,0,1,
  0,0,1,
  // 3
  0,0,1,
  0,0,0,
  // 4
  0,1,0,
  1,1,0,
  // 5
  1,1,0,
  1,1,1,
  // 6
  1,1,1,
  0,1,1,
  // 7
  0,1,1,
  0,1,0,
  // 8
  0,0,0,
  0,1,0,
  // 9
  1,0,0,
  1,1,0,
  // 10
  1,0,1,
  1,1,1,
  // 11
  0,0,1,
  0,1,1
};


// Look up table for the number of triangles produced for each of the 256 cases. There at most 5 triangular facets necessary.
__constant uchar numberOfTriangles[256] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 2, 3, 3, 4, 3, 4, 2, 3, 3, 4, 4, 5, 4, 5, 3, 2, 3, 4, 4, 3, 4, 5, 3, 2, 4, 5, 5, 4, 5, 2, 4, 1, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 2, 4, 3, 4, 3, 5, 2, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 3, 4, 4, 3, 4, 5, 5, 4, 4, 3, 5, 2, 5, 4, 2, 1, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2, 3, 4, 4, 5, 4, 5, 5, 2, 4, 3, 5, 4, 3, 2, 4, 1, 3, 4, 4, 5, 4, 5, 3, 4, 4, 5, 5, 2, 3, 4, 2, 1, 2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0};

)foo") +

std::string(R"foo(

// The last part of the algorithm involves forming the correct facets from the positions that the isosurface intersects the edges of the grid cell.
// Again a table (by Cory Gene Bloyd) is used which this time uses the same cubeindex but allows the vertex sequence to be looked up for as many triangular
// facets are necessary to represent the isosurface within the grid cell.
__constant int triTable[4096] =
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1,
  3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1,
  3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1,
  3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1,
  9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1,
  1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1,
  9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
  2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1,
  8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1,
  9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
  4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1,
  3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1,
  1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1,
  4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1,
  4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1,
  9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1,
  1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
  5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1,
  2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1,
  9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
  0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
  2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1,
  10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1,
  4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1,
  5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1,
  5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1,
  9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1,
  0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1,
  1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1,
  10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1,
  8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1,
  2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1,
  7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1,
  9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1,
  2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1,
  11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1,
  9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1,
  5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1,
  11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1,
  11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
  1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1,
  9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1,
  5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1,
  2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
  0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
  5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1,
  6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1,
  0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1,
  3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1,
  6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1,
  5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1,
  1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
  10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1,
  6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1,
  1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1,
  8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1,
  7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1,
  3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
  5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1,
  0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1,
  9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1,
  8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1,
  5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1,
  0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1,
  6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1,
  10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1,
  10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1,
  8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1,
  1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1,
  3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1,
  0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1,
  10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1,
  0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1,
  3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1,
  6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1,
  9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1,
  8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1,
  3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1,
  6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1,
  0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1,
  10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1,
  10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1,
  1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1,
  2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1,
  7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1,
  7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1,
  2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1,
  1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1,
  11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1,
  8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1,
  0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1,
  7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
  10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
  2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
  6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1,
  7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1,
  2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1,
  1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1,
  10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1,
  10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1,
  0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1,
  7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1,
  6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1,
  8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1,
  9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1,
  6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1,
  1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1,
  4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1,
  10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1,
  8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1,
  0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1,
  1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1,
  8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1,
  10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1,
  4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1,
  10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
  5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
  11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1,
  9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
  6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1,
  7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1,
  3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1,
  7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1,
  9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1,
  3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1,
  6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1,
  9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1,
  1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1,
  4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1,
  7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1,
  6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1,
  3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1,
  0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1,
  6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1,
  1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1,
  0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1,
  11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1,
  6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1,
  5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1,
  9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1,
  1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1,
  1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1,
  10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1,
  0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1,
  5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1,
  10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1,
  11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1,
  0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1,
  9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1,
  7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1,
  2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1,
  8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1,
  9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1,
  9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1,
  1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1,
  9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1,
  9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1,
  5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1,
  0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1,
  10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1,
  2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1,
  0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1,
  0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1,
  9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1,
  5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1,
  3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1,
  5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1,
  8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1,
  0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1,
  9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1,
  0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1,
  1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1,
  3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1,
  4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1,
  9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1,
  11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1,
  11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1,
  2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1,
  9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1,
  3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1,
  1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1,
  4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1,
  4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1,
  0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1,
  3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1,
  3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1,
  0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1,
  9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1,
  1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

)foo") +

std::string(R"foo(
__kernel void constructHPLevel(
                               __read_only image3d_t readHistoPyramid,
                               __write_only image3d_t writeHistoPyramid
                               ) {

  int4 writePos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
  int4 readPos = writePos*2;
  int writeValue = read_imagei(readHistoPyramid, sampler, readPos).x + // 0
  read_imagei(readHistoPyramid, sampler, (readPos+cubeOffsets[1])).x + // 1
  read_imagei(readHistoPyramid, sampler, (readPos+cubeOffsets[2])).x + // 2
  read_imagei(readHistoPyramid, sampler, (readPos+cubeOffsets[3])).x + // 3
  read_imagei(readHistoPyramid, sampler, (readPos+cubeOffsets[4])).x + // 4
  read_imagei(readHistoPyramid, sampler, (readPos+cubeOffsets[5])).x + // 5
  read_imagei(readHistoPyramid, sampler, (readPos+cubeOffsets[6])).x + // 6
  read_imagei(readHistoPyramid, sampler, (readPos+cubeOffsets[7])).x;  // 7

  write_imagei(writeHistoPyramid, writePos, writeValue);
}

int4 scanHPLevel(int target, __read_only image3d_t hp, int4 current) {

  int8 neighbors = {
    read_imagei(hp, sampler, current).x,
    read_imagei(hp, sampler, (current + cubeOffsets[1])).x,
    read_imagei(hp, sampler, (current + cubeOffsets[2])).x,
    read_imagei(hp, sampler, (current + cubeOffsets[3])).x,
    read_imagei(hp, sampler, (current + cubeOffsets[4])).x,
    read_imagei(hp, sampler, (current + cubeOffsets[5])).x,
    read_imagei(hp, sampler, (current + cubeOffsets[6])).x,
    read_imagei(hp, sampler, (current + cubeOffsets[7])).x
  };

  int acc = current.s3 + neighbors.s0;
  int8 cmp;
  cmp.s0 = acc <= target;
  acc += neighbors.s1;
  cmp.s1 = acc <= target;
  acc += neighbors.s2;
  cmp.s2 = acc <= target;
  acc += neighbors.s3;
  cmp.s3 = acc <= target;
  acc += neighbors.s4;
  cmp.s4 = acc <= target;
  acc += neighbors.s5;
  cmp.s5 = acc <= target;
  acc += neighbors.s6;
  cmp.s6 = acc <= target;
  cmp.s7 = 0;

  current += cubeOffsets[(cmp.s0+cmp.s1+cmp.s2+cmp.s3+cmp.s4+cmp.s5+cmp.s6+cmp.s7)];
  current.s0 = current.s0*2;
  current.s1 = current.s1*2;
  current.s2 = current.s2*2;
  current.s3 = current.s3 +
  cmp.s0*neighbors.s0 +
  cmp.s1*neighbors.s1 +
  cmp.s2*neighbors.s2 +
  cmp.s3*neighbors.s3 +
  cmp.s4*neighbors.s4 +
  cmp.s5*neighbors.s5 +
  cmp.s6*neighbors.s6 +
  cmp.s7*neighbors.s7;
  return current;
}
)foo") +

std::string(R"foo(
__kernel void traverseHP16(
                         __read_only image3d_t hp0, // Largest HP
                         __read_only image3d_t hp1,
                         __read_only image3d_t hp2,
                         __read_only image3d_t hp3,
                         __read_only image3d_t rawData,
                         __global float * VBOBuffer,
                         __private int4 dimensions,
                         __private float isolevel,
                         __private int sum
                         ) {

  int target = get_global_id(0);
  if(target >= sum)
    target = 0;

  int4 cubePosition = {0,0,0,0}; // x,y,z,sum
  cubePosition = scanHPLevel(target, hp3, cubePosition);
  cubePosition = scanHPLevel(target, hp2, cubePosition);
  cubePosition = scanHPLevel(target, hp1, cubePosition);
  cubePosition = scanHPLevel(target, hp0, cubePosition);
  cubePosition.x = cubePosition.x / 2;
  cubePosition.y = cubePosition.y / 2;
  cubePosition.z = cubePosition.z / 2;

  char vertexNr = 0;
  const int4 cubeData = read_imagei(hp0, sampler, cubePosition);

  // max 5 triangles
  for(int i = (target-cubePosition.s3)*3; i < (target-cubePosition.s3+1)*3; i++)
  {
    // for each vertex in triangle
    const uchar edge = triTable[cubeData.y*16 + i];
    const int3 point0 = (int3)(cubePosition.x + offsets3[edge*6], cubePosition.y + offsets3[edge*6+1], cubePosition.z + offsets3[edge*6+2]);
    const int3 point1 = (int3)(cubePosition.x + offsets3[edge*6+3], cubePosition.y + offsets3[edge*6+4], cubePosition.z + offsets3[edge*6+5]);

    // compute normal
    const float4 forwardDifference0 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x+1, point0.y,   point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x-1, point0.y,   point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y+1, point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y-1, point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z-1, 0) % dimensions).x),
                                               0.0f
                                               );
    const float4 forwardDifference1 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x+1, point1.y,   point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x-1, point1.y,   point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y+1, point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y-1, point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z-1, 0) % dimensions).x),
                                               0.0f
                                               );


    const float value0 = read_imagef(rawData, sampler, (int4)(point0.x, point0.y, point0.z, 0) % dimensions).x;
    const float diff = native_divide(
                                     (float)(isolevel-value0),
                                     (float)(read_imagef(rawData, sampler, (int4)(point1.x, point1.y, point1.z, 0) % dimensions).x - value0));

    const float4 vertex = (float4)(point0.x, point0.y, point0.z, 1.0f) + ((float4)(point1.x, point1.y, point1.z,0.0f) - (float4)(point0.x, point0.y, point0.z,0.0f)) * diff;
    const float4 scaledVertex = (float4)(vertex.x/(float)(dimensions.x),vertex.y/(float)(dimensions.y),vertex.z/(float)(dimensions.z),1.0f);

    const float4 normal = forwardDifference0 + (forwardDifference1 - forwardDifference0) * diff;

    vstore4(scaledVertex, target*9 + vertexNr*3, VBOBuffer);
    vstore4(normal, target*9 + vertexNr*3 + 1, VBOBuffer);

    vertexNr++;
  }
}
)foo") +

std::string(R"foo(
__kernel void traverseHP32(
                         __read_only image3d_t hp0, // Largest HP
                         __read_only image3d_t hp1,
                         __read_only image3d_t hp2,
                         __read_only image3d_t hp3,
                         __read_only image3d_t hp4,
                         __read_only image3d_t rawData,
                         __global float * VBOBuffer,
                         __private int4 dimensions,
                         __private float isolevel,
                         __private int sum
                         ) {

  int target = get_global_id(0);
  if(target >= sum)
    target = 0;

  int4 cubePosition = {0,0,0,0}; // x,y,z,sum
  cubePosition = scanHPLevel(target, hp4, cubePosition);
  cubePosition = scanHPLevel(target, hp3, cubePosition);
  cubePosition = scanHPLevel(target, hp2, cubePosition);
  cubePosition = scanHPLevel(target, hp1, cubePosition);
  cubePosition = scanHPLevel(target, hp0, cubePosition);
  cubePosition.x = cubePosition.x / 2;
  cubePosition.y = cubePosition.y / 2;
  cubePosition.z = cubePosition.z / 2;

  char vertexNr = 0;
  const int4 cubeData = read_imagei(hp0, sampler, cubePosition);

  // max 5 triangles
  for(int i = (target-cubePosition.s3)*3; i < (target-cubePosition.s3+1)*3; i++)
  {
    // for each vertex in triangle
    const uchar edge = triTable[cubeData.y*16 + i];
    const int3 point0 = (int3)(cubePosition.x + offsets3[edge*6], cubePosition.y + offsets3[edge*6+1], cubePosition.z + offsets3[edge*6+2]);
    const int3 point1 = (int3)(cubePosition.x + offsets3[edge*6+3], cubePosition.y + offsets3[edge*6+4], cubePosition.z + offsets3[edge*6+5]);

    // compute normal
    const float4 forwardDifference0 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x+1, point0.y,   point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x-1, point0.y,   point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y+1, point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y-1, point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z-1, 0) % dimensions).x),
                                               0.0f
                                               );
    const float4 forwardDifference1 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x+1, point1.y,   point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x-1, point1.y,   point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y+1, point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y-1, point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z-1, 0) % dimensions).x),
                                               0.0f
                                               );


    const float value0 = read_imagef(rawData, sampler, (int4)(point0.x, point0.y, point0.z, 0) % dimensions).x;
    const float diff = native_divide(
                                     (float)(isolevel-value0),
                                     (float)(read_imagef(rawData, sampler, (int4)(point1.x, point1.y, point1.z, 0) % dimensions).x - value0));

    const float4 vertex = (float4)(point0.x, point0.y, point0.z, 1.0f) + ((float4)(point1.x, point1.y, point1.z,0.0f) - (float4)(point0.x, point0.y, point0.z,0.0f)) * diff;
    const float4 scaledVertex = (float4)(vertex.x/(float)(dimensions.x),vertex.y/(float)(dimensions.y),vertex.z/(float)(dimensions.z),1.0f);

    const float4 normal = forwardDifference0 + (forwardDifference1 - forwardDifference0) * diff;

    vstore4(scaledVertex, target*9 + vertexNr*3, VBOBuffer);
    vstore4(normal, target*9 + vertexNr*3 + 1, VBOBuffer);

    vertexNr++;
  }
}
)foo") +

std::string(R"foo(
__kernel void traverseHP64(
                         __read_only image3d_t hp0, // Largest HP
                         __read_only image3d_t hp1,
                         __read_only image3d_t hp2,
                         __read_only image3d_t hp3,
                         __read_only image3d_t hp4,
                         __read_only image3d_t hp5,
                         __read_only image3d_t rawData,
                         __global float * VBOBuffer,
                         __private int4 dimensions,
                         __private float isolevel,
                         __private int sum
                         ) {

  int target = get_global_id(0);
  if(target >= sum)
    target = 0;

  int4 cubePosition = {0,0,0,0}; // x,y,z,sum
  cubePosition = scanHPLevel(target, hp5, cubePosition);
  cubePosition = scanHPLevel(target, hp4, cubePosition);
  cubePosition = scanHPLevel(target, hp3, cubePosition);
  cubePosition = scanHPLevel(target, hp2, cubePosition);
  cubePosition = scanHPLevel(target, hp1, cubePosition);
  cubePosition = scanHPLevel(target, hp0, cubePosition);
  cubePosition.x = cubePosition.x / 2;
  cubePosition.y = cubePosition.y / 2;
  cubePosition.z = cubePosition.z / 2;

  char vertexNr = 0;
  const int4 cubeData = read_imagei(hp0, sampler, cubePosition);

  // max 5 triangles
  for(int i = (target-cubePosition.s3)*3; i < (target-cubePosition.s3+1)*3; i++)
  {
    // for each vertex in triangle
    const uchar edge = triTable[cubeData.y*16 + i];
    const int3 point0 = (int3)(cubePosition.x + offsets3[edge*6], cubePosition.y + offsets3[edge*6+1], cubePosition.z + offsets3[edge*6+2]);
    const int3 point1 = (int3)(cubePosition.x + offsets3[edge*6+3], cubePosition.y + offsets3[edge*6+4], cubePosition.z + offsets3[edge*6+5]);

    // compute normal
    const float4 forwardDifference0 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x+1, point0.y,   point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x-1, point0.y,   point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y+1, point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y-1, point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z-1, 0) % dimensions).x),
                                               0.0f
                                               );
    const float4 forwardDifference1 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x+1, point1.y,   point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x-1, point1.y,   point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y+1, point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y-1, point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z-1, 0) % dimensions).x),
                                               0.0f
                                               );


    const float value0 = read_imagef(rawData, sampler, (int4)(point0.x, point0.y, point0.z, 0) % dimensions).x;
    const float diff = native_divide(
                                     (float)(isolevel-value0),
                                     (float)(read_imagef(rawData, sampler, (int4)(point1.x, point1.y, point1.z, 0) % dimensions).x - value0));

    const float4 vertex = (float4)(point0.x, point0.y, point0.z, 1.0f) + ((float4)(point1.x, point1.y, point1.z,0.0f) - (float4)(point0.x, point0.y, point0.z,0.0f)) * diff;
    const float4 scaledVertex = (float4)(vertex.x/(float)(dimensions.x),vertex.y/(float)(dimensions.y),vertex.z/(float)(dimensions.z),1.0f);

    const float4 normal = forwardDifference0 + (forwardDifference1 - forwardDifference0) * diff;

    vstore4(scaledVertex, target*9 + vertexNr*3, VBOBuffer);
    vstore4(normal, target*9 + vertexNr*3 + 1, VBOBuffer);

    vertexNr++;
  }
}

)foo") +

std::string(R"foo(
__kernel void traverseHP128(
                         __read_only image3d_t hp0, // Largest HP
                         __read_only image3d_t hp1,
                         __read_only image3d_t hp2,
                         __read_only image3d_t hp3,
                         __read_only image3d_t hp4,
                         __read_only image3d_t hp5,
                         __read_only image3d_t hp6,
                         __read_only image3d_t rawData,
                         __global float * VBOBuffer,
                         __private int4 dimensions,
                         __private float isolevel,
                         __private int sum
                         ) {

  int target = get_global_id(0);
  if(target >= sum)
    target = 0;

  int4 cubePosition = {0,0,0,0}; // x,y,z,sum
  cubePosition = scanHPLevel(target, hp6, cubePosition);
  cubePosition = scanHPLevel(target, hp5, cubePosition);
  cubePosition = scanHPLevel(target, hp4, cubePosition);
  cubePosition = scanHPLevel(target, hp3, cubePosition);
  cubePosition = scanHPLevel(target, hp2, cubePosition);
  cubePosition = scanHPLevel(target, hp1, cubePosition);
  cubePosition = scanHPLevel(target, hp0, cubePosition);
  cubePosition.x = cubePosition.x / 2;
  cubePosition.y = cubePosition.y / 2;
  cubePosition.z = cubePosition.z / 2;

  char vertexNr = 0;
  const int4 cubeData = read_imagei(hp0, sampler, cubePosition);

  // max 5 triangles
  for(int i = (target-cubePosition.s3)*3; i < (target-cubePosition.s3+1)*3; i++)
  {
    // for each vertex in triangle
    const uchar edge = triTable[cubeData.y*16 + i];
    const int3 point0 = (int3)(cubePosition.x + offsets3[edge*6], cubePosition.y + offsets3[edge*6+1], cubePosition.z + offsets3[edge*6+2]);
    const int3 point1 = (int3)(cubePosition.x + offsets3[edge*6+3], cubePosition.y + offsets3[edge*6+4], cubePosition.z + offsets3[edge*6+5]);

    // compute normal
    const float4 forwardDifference0 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x+1, point0.y,   point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x-1, point0.y,   point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y+1, point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y-1, point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z-1, 0) % dimensions).x),
                                               0.0f
                                               );
    const float4 forwardDifference1 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x+1, point1.y,   point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x-1, point1.y,   point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y+1, point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y-1, point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z-1, 0) % dimensions).x),
                                               0.0f
                                               );


    const float value0 = read_imagef(rawData, sampler, (int4)(point0.x, point0.y, point0.z, 0) % dimensions).x;
    const float diff = native_divide(
                                     (float)(isolevel-value0),
                                     (float)(read_imagef(rawData, sampler, (int4)(point1.x, point1.y, point1.z, 0) % dimensions).x - value0));

    const float4 vertex = (float4)(point0.x, point0.y, point0.z, 1.0f) + ((float4)(point1.x, point1.y, point1.z,0.0f) - (float4)(point0.x, point0.y, point0.z,0.0f)) * diff;
    const float4 scaledVertex = (float4)(vertex.x/(float)(dimensions.x),vertex.y/(float)(dimensions.y),vertex.z/(float)(dimensions.z),1.0f);

    const float4 normal = forwardDifference0 + (forwardDifference1 - forwardDifference0) * diff;

    vstore4(scaledVertex, target*9 + vertexNr*3, VBOBuffer);
    vstore4(normal, target*9 + vertexNr*3 + 1, VBOBuffer);

    vertexNr++;
  }
}
)foo") +

std::string(R"foo(
__kernel void traverseHP256(
                         __read_only image3d_t hp0, // Largest HP
                         __read_only image3d_t hp1,
                         __read_only image3d_t hp2,
                         __read_only image3d_t hp3,
                         __read_only image3d_t hp4,
                         __read_only image3d_t hp5,
                         __read_only image3d_t hp6,
                         __read_only image3d_t hp7,
                         __read_only image3d_t rawData,
                         __global float * VBOBuffer,
                         __private int4 dimensions,
                         __private float isolevel,
                         __private int sum
                         ) {

  int target = get_global_id(0);
  if(target >= sum)
    target = 0;

  int4 cubePosition = {0,0,0,0}; // x,y,z,sum
  cubePosition = scanHPLevel(target, hp7, cubePosition);
  cubePosition = scanHPLevel(target, hp6, cubePosition);
  cubePosition = scanHPLevel(target, hp5, cubePosition);
  cubePosition = scanHPLevel(target, hp4, cubePosition);
  cubePosition = scanHPLevel(target, hp3, cubePosition);
  cubePosition = scanHPLevel(target, hp2, cubePosition);
  cubePosition = scanHPLevel(target, hp1, cubePosition);
  cubePosition = scanHPLevel(target, hp0, cubePosition);
  cubePosition.x = cubePosition.x / 2;
  cubePosition.y = cubePosition.y / 2;
  cubePosition.z = cubePosition.z / 2;

  char vertexNr = 0;
  const int4 cubeData = read_imagei(hp0, sampler, cubePosition);

  // max 5 triangles
  for(int i = (target-cubePosition.s3)*3; i < (target-cubePosition.s3+1)*3; i++)
  {
    // for each vertex in triangle
    const uchar edge = triTable[cubeData.y*16 + i];
    const int3 point0 = (int3)(cubePosition.x + offsets3[edge*6], cubePosition.y + offsets3[edge*6+1], cubePosition.z + offsets3[edge*6+2]);
    const int3 point1 = (int3)(cubePosition.x + offsets3[edge*6+3], cubePosition.y + offsets3[edge*6+4], cubePosition.z + offsets3[edge*6+5]);

    // compute normal
    const float4 forwardDifference0 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x+1, point0.y,   point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x-1, point0.y,   point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y+1, point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y-1, point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z-1, 0) % dimensions).x),
                                               0.0f
                                               );
    const float4 forwardDifference1 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x+1, point1.y,   point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x-1, point1.y,   point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y+1, point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y-1, point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z-1, 0) % dimensions).x),
                                               0.0f
                                               );


    const float value0 = read_imagef(rawData, sampler, (int4)(point0.x, point0.y, point0.z, 0) % dimensions).x;
    const float diff = native_divide(
                                     (float)(isolevel-value0),
                                     (float)(read_imagef(rawData, sampler, (int4)(point1.x, point1.y, point1.z, 0) % dimensions).x - value0));

    const float4 vertex = (float4)(point0.x, point0.y, point0.z, 1.0f) + ((float4)(point1.x, point1.y, point1.z,0.0f) - (float4)(point0.x, point0.y, point0.z,0.0f)) * diff;
    const float4 scaledVertex = (float4)(vertex.x/(float)(dimensions.x),vertex.y/(float)(dimensions.y),vertex.z/(float)(dimensions.z),1.0f);

    const float4 normal = forwardDifference0 + (forwardDifference1 - forwardDifference0) * diff;

    vstore4(scaledVertex, target*9 + vertexNr*3, VBOBuffer);
    vstore4(normal, target*9 + vertexNr*3 + 1, VBOBuffer);

    vertexNr++;
  }
}
)foo") +

std::string(R"foo(
__kernel void traverseHP512(
                         __read_only image3d_t hp0, // Largest HP
                         __read_only image3d_t hp1,
                         __read_only image3d_t hp2,
                         __read_only image3d_t hp3,
                         __read_only image3d_t hp4,
                         __read_only image3d_t hp5,
                         __read_only image3d_t hp6,
                         __read_only image3d_t hp7,
                         __read_only image3d_t hp8,
                         __read_only image3d_t rawData,
                         __global float * VBOBuffer,
                         __private int4 dimensions,
                         __private float isolevel,
                         __private int sum
                         ) {

  int target = get_global_id(0);
  if(target >= sum)
    target = 0;

  int4 cubePosition = {0,0,0,0}; // x,y,z,sum
  cubePosition = scanHPLevel(target, hp8, cubePosition);
  cubePosition = scanHPLevel(target, hp7, cubePosition);
  cubePosition = scanHPLevel(target, hp6, cubePosition);
  cubePosition = scanHPLevel(target, hp5, cubePosition);
  cubePosition = scanHPLevel(target, hp4, cubePosition);
  cubePosition = scanHPLevel(target, hp3, cubePosition);
  cubePosition = scanHPLevel(target, hp2, cubePosition);
  cubePosition = scanHPLevel(target, hp1, cubePosition);
  cubePosition = scanHPLevel(target, hp0, cubePosition);
  cubePosition.x = cubePosition.x / 2;
  cubePosition.y = cubePosition.y / 2;
  cubePosition.z = cubePosition.z / 2;

  char vertexNr = 0;
  const int4 cubeData = read_imagei(hp0, sampler, cubePosition);

  // max 5 triangles
  for(int i = (target-cubePosition.s3)*3; i < (target-cubePosition.s3+1)*3; i++)
  {
    // for each vertex in triangle
    const uchar edge = triTable[cubeData.y*16 + i];
    const int3 point0 = (int3)(cubePosition.x + offsets3[edge*6], cubePosition.y + offsets3[edge*6+1], cubePosition.z + offsets3[edge*6+2]);
    const int3 point1 = (int3)(cubePosition.x + offsets3[edge*6+3], cubePosition.y + offsets3[edge*6+4], cubePosition.z + offsets3[edge*6+5]);

    // compute normal
    const float4 forwardDifference0 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x+1, point0.y,   point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x-1, point0.y,   point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y+1, point0.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y-1, point0.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point0.x,   point0.y,   point0.z-1, 0) % dimensions).x),
                                               0.0f
                                               );
    const float4 forwardDifference1 = (float4)(
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x+1, point1.y,   point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x-1, point1.y,   point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y+1, point1.z,   0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y-1, point1.z,   0) % dimensions).x),
                                               (float)(-read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z+1, 0) % dimensions).x+
                                                        read_imagef(rawData, sampler, (int4)(point1.x,   point1.y,   point1.z-1, 0) % dimensions).x),
                                               0.0f
                                               );


    const float value0 = read_imagef(rawData, sampler, (int4)(point0.x, point0.y, point0.z, 0) % dimensions).x;
    const float diff = native_divide(
                                     (float)(isolevel-value0),
                                     (float)(read_imagef(rawData, sampler, (int4)(point1.x, point1.y, point1.z, 0) % dimensions).x - value0));

    const float4 vertex = (float4)(point0.x, point0.y, point0.z, 1.0f) + ((float4)(point1.x, point1.y, point1.z,0.0f) - (float4)(point0.x, point0.y, point0.z,0.0f)) * diff;
    const float4 scaledVertex = (float4)(vertex.x/(float)(dimensions.x),vertex.y/(float)(dimensions.y),vertex.z/(float)(dimensions.z),1.0f);

    const float4 normal = forwardDifference0 + (forwardDifference1 - forwardDifference0) * diff;

    vstore4(scaledVertex, target*9 + vertexNr*3, VBOBuffer);
    vstore4(normal, target*9 + vertexNr*3 + 1, VBOBuffer);

    vertexNr++;
  }
}
)foo") +

std::string(R"foo(
// The first part of the algorithm uses a table (edgeTable) which maps the vertices under the isosurface to the intersecting edges.
// An 8 bit index is formed where each bit corresponds to a vertex.
__kernel void classifyCubes(__write_only image3d_t histoPyramid,
                            __read_only image3d_t rawData,
                            __private int4 dimensions,
                            __private float isolevel)
{
  int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

  if(any(pos>=dimensions))
  {
    write_imageui(histoPyramid, pos, (uint4)(0, 0, 0, 0));
    return;
  }

  // Find cube class nr
  const float first = read_imagef(rawData, sampler, pos).x;
  const uchar cubeindex =
       ((first > isolevel)) |
       ((read_imagef(rawData, sampler, (pos + cubeOffsets[1]) % dimensions).x > isolevel) << 1) |
       ((read_imagef(rawData, sampler, (pos + cubeOffsets[3]) % dimensions).x > isolevel) << 2) |
       ((read_imagef(rawData, sampler, (pos + cubeOffsets[2]) % dimensions).x > isolevel) << 3) |
       ((read_imagef(rawData, sampler, (pos + cubeOffsets[4]) % dimensions).x > isolevel) << 4) |
       ((read_imagef(rawData, sampler, (pos + cubeOffsets[5]) % dimensions).x > isolevel) << 5) |
       ((read_imagef(rawData, sampler, (pos + cubeOffsets[7]) % dimensions).x > isolevel) << 6) |
       ((read_imagef(rawData, sampler, (pos + cubeOffsets[6]) % dimensions).x > isolevel) << 7);

  // Store number of triangles
  write_imageui(histoPyramid, pos, (uint4)(numberOfTriangles[cubeindex], cubeindex, first, 0));
}
)foo");
