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

class OpenGLEnergyVolumeRenderedSurface final: public OpenGLShader, public LogReportingConsumer
{
public:
  OpenGLEnergyVolumeRenderedSurface();
  void initializeOpenCL(bool isOpenCLInitialized, cl_context _clContext, cl_device_id _clDeviceId, cl_command_queue _clCommandQueue, QStringList &logData);
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures);
  void paintGL(GLuint structureUniformBuffer, GLuint isosurfaceUniformBuffer, GLuint depthTexture);
  void reloadData();
  void initializeTransferFunctionTexture();
  void initializeVertexArrayObject();
  void initializeTransformUniforms();
  void initializeStructureUniforms();
  void initializeIsosurfaceUniforms();
  void initializeLightUniforms();
  void loadShader(void) override final;
  void setLogReportingWidget(LogReporting *logReporting)  override final;
  void deleteBuffers();
  void generateBuffers();
private:
  bool _isOpenCLInitialized;
  GLuint _program;
  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> _renderStructures;

  LogReporting* _logReporter = nullptr;

  std::vector<std::vector<size_t>> _numberOfIndices;

  std::vector<std::vector<GLuint>> _volumeTextures;
  GLuint _transferFunctionTexture;

  std::vector<std::vector<GLuint>> _vertexBuffer;
  std::vector<std::vector<GLuint>> _indexBuffer;
  //std::vector<std::vector<GLuint>> _instancePositionBuffer;

  std::vector<std::vector<GLuint>> _vertexArrayObject;
  //std::vector<std::vector<GLuint>> _ambientColorBuffer;
  //std::vector<std::vector<GLuint>> _diffuseColorBuffer;
  std::vector<std::vector<GLuint>> _specularColorBuffer;

  std::pair<double, double> _range;
  double3 _origin;
  double3 _spacing;
  int3 _size;

  GLint _volumeTextureUniformLocation{-1};
  GLint _transferFunctionUniformLocation{-1};
  GLint _depthTextureUniformLocation{-1};

  GLuint _vertexPositionAttributeLocation{0};
  GLuint _stepLengthUniformLocation{0};

  cl_context _clContext;
  cl_device_id _clDeviceId;
  cl_command_queue _clCommandQueue;
  SKOpenCLEnergyGridUnitCell _energyGridUnitCell;

  QCache<RKRenderStructure*, std::vector<float4>> _cache;

  static const std::string _vertexShaderSource;
  static const std::string _fragmentShaderSource;

  static std::array<float4, 256> transferFunction;
};
