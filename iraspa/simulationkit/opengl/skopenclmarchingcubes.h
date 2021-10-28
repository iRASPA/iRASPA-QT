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
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#include <QOpenGLFunctions_3_3_Core>
#include <foundationkit.h>

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

class SKOpenCLMarchingCubes: protected QOpenGLFunctions_3_3_Core, public LogReportingConsumer
{
public:
  SKOpenCLMarchingCubes();
  void initialize(bool isOpenCLInitialized, cl_context context, cl_device_id device_id, cl_command_queue queue, QStringList &logData);
  int computeIsosurface(size_t size, std::vector<cl_float>* voxels, double isoValue, cl_GLuint OpenGLVertextBufferObject);
  bool glInteroperability() {return _glInteroperability;}
  void setGLInteroperability(bool interoperability) {_glInteroperability = interoperability;}
  void setLogReportingWidget(LogReporting *logReporting)  override final {_logReporter = logReporting;}
private:
  LogReporting* _logReporter = nullptr;
  bool _isOpenCLInitialized;
  bool _isOpenCLReady;
  cl_program _program;
  cl_context _clContext;
  cl_command_queue _clCommandQueue;
  cl_device_id _clDeviceId;
  cl_kernel _classifyCubesKernel;
  cl_kernel _traverseHPKernel;
  cl_kernel _constructHPLevelKernel;
  size_t _workGroupSize;
  bool _glInteroperability = true;
  static std::string _marchingCubesKernelPart;
  static void callBack(cl_program progran, void *user_data);
};
