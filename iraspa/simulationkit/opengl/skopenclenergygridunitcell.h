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
#include <mathkit.h>
#include <foundationkit.h>

#include <QOpenGLFunctions_3_3_Core>

#define CL_TARGET_OPENCL_VERSION 120
#ifdef Q_OS_MACOS
  #include <OpenCL/opencl.h>
  #include <OpenGL/OpenGL.h>
#elif _WIN32
  #include <CL/cl.h>
#else
  #include <CL/opencl.h>
#endif

class SKOpenCLEnergyGridUnitCell: public QOpenGLFunctions_3_3_Core, public LogReportingConsumer
{
public:
  SKOpenCLEnergyGridUnitCell();
  void initialize(bool isOpenCLInitialized, cl_context context, cl_device_id device_id, cl_command_queue queue, QStringList &logData);
  std::vector<cl_float>* ComputeEnergyGrid(int sizeX, int sizeY, int sizeZ, double2 probeParameter,
                                          std::vector<double3> positions, std::vector<double2> potentialParameters,
                                          double3x3 unitCell, int3 numberOfReplicas);
  void setLogReportingWidget(LogReporting *logReporting)  override final {_logReporter = logReporting;}
private:
  LogReporting* _logReporter = nullptr;
  bool _isOpenCLInitialized;
  bool _isOpenCLReady;
  cl_program _program;
  cl_context _clContext;
  cl_command_queue _clCommandQueue;
  cl_device_id _clDeviceId;
  cl_command_queue _queue;
  cl_kernel _kernel;
  size_t _workGroupSize;
  static const char* _energyGridUnitCellKernel;
  static void callBack(cl_program progran, void *user_data);
};


