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
#include <cstring>
#include <string>
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#include <QGLFunctions>
#include "openglshader.h"
#include "rkrenderkitprotocols.h"

class OpenGLPolygonalPrismObjectPickingShader;

class OpenGLPolygonalPrismObjectSelectionStripesShader;
class OpenGLPolygonalPrismObjectSelectionWorleyNoise3DShader;
class OpenGLPolygonalPrismObjectSelectionGlowShader;

class OpenGLPolygonalPrismObjectShader: public OpenGLShader
{
public:
  OpenGLPolygonalPrismObjectShader();
  void loadShader(void) override;
  void deleteBuffers();
  void generateBuffers();

  void paintGLOpaque(GLuint structureUniformBuffer);
  void paintGLTransparent(GLuint structureUniformBuffer);

  void reloadData();
  void initializeVertexArrayObject();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures);
  GLuint program() {return _program;}
private:
  GLuint _program;
  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> _renderStructures;

  std::vector<std::vector<size_t>> _numberOfIndices;
  std::vector<std::vector<size_t>> _numberOfDrawnAtoms;

  std::vector<std::vector<GLuint>> _vertexBuffer;
  std::vector<std::vector<GLuint>> _indexBuffer;
  std::vector<std::vector<GLuint>> _instancePositionBuffer;

  std::vector<std::vector<GLuint>> _vertexArrayObject;
  std::vector<std::vector<GLuint>> _ambientColorBuffer;
  std::vector<std::vector<GLuint>> _diffuseColorBuffer;
  std::vector<std::vector<GLuint>> _specularColorBuffer;

  GLint _ambientOcclusionTextureUniformLocation;
  GLint _vertexNormalAttributeLocation;
  GLint _vertexPositionAttributeLocation;
  GLint _instancePositionAttributeLocation;

  static const std::string _vertexShaderSource;
  static const std::string _fragmentShaderSource;

  friend OpenGLPolygonalPrismObjectPickingShader;

  friend OpenGLPolygonalPrismObjectSelectionStripesShader;
  friend OpenGLPolygonalPrismObjectSelectionWorleyNoise3DShader;
  friend OpenGLPolygonalPrismObjectSelectionGlowShader;
};

