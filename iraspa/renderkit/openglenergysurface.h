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
#include <QCache>
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  #include <QGLFunctions>
#else
  #include <QOpenGLFunctions>
#endif
#include "openglshader.h"
#include "rkrenderkitprotocols.h"
#include <foundationkit.h>
#include <simulationkit.h>

#ifdef Q_OS_MACOS
  #include <OpenCL/opencl.h>
  #include <OpenGL/OpenGL.h>
#else
  #include <CL/opencl.h>
#endif

class OpenGLEnergySurface: public OpenGLShader, public LogReportingConsumer
{
public:
  OpenGLEnergySurface();
  ~OpenGLEnergySurface() override final;
  void loadShader(void) override final;
  void initializeOpenCL(bool isOpenCLInitialized, cl_context _clContext, cl_device_id _clDeviceId, cl_command_queue _clCommandQueue, QStringList &logData);
  void deleteBuffers();
  void generateBuffers();
  void paintGLOpaque(GLuint structureUniformBuffer, GLuint isosurfaceUniformBuffer);
  void paintGLTransparent(GLuint structureUniformBuffer, GLuint isosurfaceUniformBuffer);

  void setLogReportingWidget(LogReporting *logReporting)  override final;

  void reloadData();
  void initializeVertexArrayObject();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures);
  GLuint program() {return _program;}
  void invalidateIsosurface(std::vector<std::shared_ptr<RKRenderStructure>> structures);
  void computeHeliumVoidFraction(std::vector<std::shared_ptr<RKRenderStructure>> structures);
  void computeNitrogenSurfaceArea(std::vector<std::shared_ptr<RKRenderStructure>> structures);
  void setGLInteroperability(bool value) {_energyGridMarchingCubes.setGLInteroperability(value);}

  void initializeTransformUniforms();
  void initializeStructureUniforms();
  void initializeIsosurfaceUniforms();
  void initializeLightUniforms();
private:
  bool _isOpenCLInitialized;
  GLuint _program;
  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> _renderStructures;

  LogReporting* _logReporter = nullptr;

  GLuint _atomSurfaceVertexNormalAttributeLocation;
  GLuint _atomSurfaceVertexPositionAttributeLocation;
  GLuint _atomSurfaceInstancePositionAttributeLocation;

  std::vector<std::vector<size_t>> _surfaceNumberOfIndices;
  std::vector<std::vector<size_t>> _surfaceNumberOfInstances;
  std::vector<std::vector<GLuint>> _surfaceVertexArrayObject;
  std::vector<std::vector<GLuint>> _surfaceVertexBuffer;
  std::vector<std::vector<GLuint>> _surfaceIndexBuffer;
  std::vector<std::vector<GLuint>> _surfaceInstancePositionBuffer;

  cl_context _clContext;
  cl_device_id _clDeviceId;
  cl_command_queue _clCommandQueue;
  SKOpenCLEnergyGridUnitCell _energyGridUnitCell;
  SKOpenCLMarchingCubes _energyGridMarchingCubes;
  SKOpenCLFindMinmumEnergyGridUnitCell _findMinimumEnergyGridUnitCell;
  SKOpenCLVoidFractionUnitCell _voidFractionUnitCell;

  QCache<RKRenderStructure*, std::vector<cl_float>> _cache;

  static const std::string _vertexShaderSource;
  static const std::string _fragmentShaderSource;
};

