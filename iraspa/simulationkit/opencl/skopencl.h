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

#define CL_TARGET_OPENCL_VERSION 120
#ifdef Q_OS_MACOS
  #include <OpenCL/opencl.h>
#elif _WIN32
  #include <CL/cl.h>
#else
  #include <CL/opencl.h>
#endif

class SKOpenCL
{
public:
  SKOpenCL();
protected:
  bool _isOpenCLInitialized = false;
  bool _isOpenCLReady = false;
  cl_context _clContext = nullptr;
  cl_device_id _clDeviceId = nullptr;
  cl_command_queue _clCommandQueue = nullptr;

  std::optional<cl_device_id> bestOpenCLDevice(cl_device_type device_type);
  bool supportsImageFormatCapabilities(cl_context &trial_clContext, cl_device_id &trial_clDeviceId);
};
