#include "skopencl.h"
#include <QDebug>

SKOpenCL::SKOpenCL()
{
  cl_int err;

  // see first if there is a suitable GPU device for OpenCL
  std::optional<cl_device_id> bestGPUDevice = bestOpenCLDevice(CL_DEVICE_TYPE_GPU);

  if(bestGPUDevice)
  {
    cl_device_id cllDeviceId = *bestGPUDevice;

    cl_context context = clCreateContext(nullptr, 1, &cllDeviceId, nullptr, nullptr, &err);
    if(err == CL_SUCCESS)
    {
      cl_command_queue clCommandQueue = clCreateCommandQueue(context, cllDeviceId, 0, &err);
      if(err == CL_SUCCESS)
      {
        _clContext = context;
        _clDeviceId = cllDeviceId;
        _clCommandQueue = clCommandQueue;
        _isOpenCLInitialized = true;
        return;
      }
      clReleaseContext(context);
    }
  }

  // see next if there is a suitable CPU device for OpenCL if no GPU is available
  std::optional<cl_device_id> bestCPUDevice = bestOpenCLDevice(CL_DEVICE_TYPE_CPU);

  if(bestCPUDevice)
  {
    cl_device_id cllDeviceId = *bestCPUDevice;

    cl_context context = clCreateContext(nullptr, 1, &cllDeviceId, nullptr, nullptr, &err);
    if(err == CL_SUCCESS)
    {
      cl_command_queue clCommandQueue = clCreateCommandQueue(context, cllDeviceId, 0, &err);
      if(err == CL_SUCCESS)
      {
        _clContext = context;
        _clDeviceId = cllDeviceId;
        _clCommandQueue = clCommandQueue;
        _isOpenCLInitialized = true;
        return;
      }
      clReleaseContext(context);
    }
  }
}


std::optional<cl_device_id> SKOpenCL::bestOpenCLDevice(cl_device_type device_type)
{
  cl_int err;

  // get the number of platforms
  cl_uint platformCount;
  err = clGetPlatformIDs(0, nullptr, &platformCount);

  if(platformCount<=0)
  {
    return std::nullopt;
  }

  // get platform ids
  QVector<cl_platform_id> platforms;
  platforms.resize(platformCount);
  err = clGetPlatformIDs(platforms.size(), platforms.data(), nullptr);
  if(err != CL_SUCCESS)
  {
    return std::nullopt;
  }

  std::optional<std::pair<cl_device_id, cl_uint>> bestDevice = std::nullopt;

  // loop over all platforms and devices of type 'device_type'
  for (cl_uint i = 0; i < platformCount; ++i)
  {
        // get the number of devices of type 'device_type' in the platform
    cl_uint deviceCount;
    err = clGetDeviceIDs(platforms[i], device_type, 0, nullptr, &deviceCount);
    if(err != CL_SUCCESS)
    {
      continue;
    }

    // get the IDs of GPU devices
    QVector<cl_device_id> devices;
    devices.resize(deviceCount);
    err = clGetDeviceIDs(platforms[i], device_type, devices.size(), devices.data(), nullptr);
    if(err != CL_SUCCESS)
    {
      continue;
    }

    // loop over all devices
    for(cl_uint j = 0; j < deviceCount; j++)
    {
      cl_uint maxComputeUnits;
      err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, NULL);
      if(err != CL_SUCCESS)
      {
        continue;
      }

      cl_context context = clCreateContext(nullptr, 1, &devices[j], nullptr, nullptr, &err);

      if(err != CL_SUCCESS)
      {
        continue;
      }

      if(supportsImageFormatCapabilities(context, devices[j]))
      {
        cl_command_queue clCommandQueue = clCreateCommandQueue(context, devices[j], 0, &err);
        if(err != CL_SUCCESS)
        {
          clReleaseContext(context);
          continue;
        }
        clReleaseCommandQueue(clCommandQueue);

        if(bestDevice)
        {
          // use the one with the highest compute units
          if(maxComputeUnits > std::get<1>(*bestDevice))
          {
            bestDevice = std::make_pair(devices[j], maxComputeUnits);
          }
        }
        else
        {
          bestDevice = std::make_pair(devices[j], maxComputeUnits);
        }
      }
      clReleaseContext(context);
    }
  }

  if(bestDevice)
  {
    // we have a suitable device that can be used
    return std::get<0>(*bestDevice);
  }

  return std::nullopt;
}

bool SKOpenCL::supportsImageFormatCapabilities(cl_context &trial_clContext, cl_device_id &trial_clDeviceId)
{
  cl_int err;

  // check image support
  cl_bool image_support = false;
  clGetDeviceInfo(trial_clDeviceId, CL_DEVICE_IMAGE_SUPPORT, sizeof(image_support), &image_support, nullptr);

  if(!image_support)
  {
    return false;
  }

  // check the needed image formats
  cl_image_format imageFormat_RGBA_INT8{CL_RGBA,CL_UNSIGNED_INT8};
  cl_image_desc imageDescriptor_RGBA_INT8{CL_MEM_OBJECT_IMAGE3D, 256, 256, 256, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_RGBA_INT8 = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_RGBA_INT8, &imageDescriptor_RGBA_INT8, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_RGBA_INT8);

  cl_image_format imageFormat_R_INT8{CL_R,CL_UNSIGNED_INT8};
  cl_image_desc imageDescriptor_R_INT8{CL_MEM_OBJECT_IMAGE3D, 256, 256, 256, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_R_INT8 = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_R_INT8, &imageDescriptor_R_INT8, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_R_INT8);

  cl_image_format imageFormat_R_INT16{CL_R,CL_UNSIGNED_INT16};
  cl_image_desc imageDescriptor_R_INT16{CL_MEM_OBJECT_IMAGE3D, 256, 256, 256, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_R_INT16 = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_R_INT16, &imageDescriptor_R_INT16, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_R_INT16);

  cl_image_format imageFormat_R_INT32{CL_R,CL_UNSIGNED_INT16};
  cl_image_desc imageDescriptor_R_INT32{CL_MEM_OBJECT_IMAGE3D, 128, 128, 128, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_R_INT32 = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_R_INT32, &imageDescriptor_R_INT32, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_R_INT32);

  cl_image_format imageFormat_R_FLOAT{CL_R,CL_FLOAT};
  cl_image_desc imageDescriptor_R_FLOAT{CL_MEM_OBJECT_IMAGE3D, 128, 128, 128, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_R_FLOAT = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_R_FLOAT, &imageDescriptor_R_FLOAT, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_R_FLOAT);

  return true;
}
