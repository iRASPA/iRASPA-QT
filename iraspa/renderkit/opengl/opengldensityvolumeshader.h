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
#include <array>
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  #include <QGLFunctions>
#else
  #include <QOpenGLFunctions>
#endif
#include "rkrenderkitprotocols.h"
#include "rkrenderuniforms.h"

#include "openglshader.h"

// Ray
struct Ray
{
  double3 origin;
  double3 direction;

  Ray(double3 origin, double3 direction): origin(origin), direction(direction) {}
};

// Axis-aligned bounding box
struct AABB
{
  double3 top;
  double3 bottom;

  AABB(double3 top, double3 bottom): top(top), bottom(bottom) {}
};

class OpenGLDensityVolumeShader: public OpenGLShader
{
public:
  OpenGLDensityVolumeShader();

  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures);
  void paintGL(GLuint structureUniformBuffer, GLuint isosurfaceUniformBuffer, GLuint depthTexture);
  void reloadData();
  void initializeTransferFunctionTexture();
  void initializeVertexArrayObject();
  void initializeTransformUniforms();
  void initializeStructureUniforms();
  void initializeLightUniforms();
  void loadShader(void) override final;

private:
  GLuint _program;
  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> _renderStructures;

  void deleteBuffers();
  void generateBuffers();

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

  GLuint _noiseTexture;
  std::pair<double, double> _range;
  double3 _origin;
  double3 _spacing;
  int3 _size;

  GLint _volumeTextureUniformLocation{-1};
  GLint _transferFunctionUniformLocation{-1};

  GLuint _vertexPositionAttributeLocation{0};
  GLuint _depthTextureUniformLocation{0};

  static const std::string _vertexShaderSource;
  static const std::string _fragmentShaderSource;

  static std::array<float4, 256> transferFunction;
};
