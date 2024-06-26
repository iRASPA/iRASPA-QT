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
#include "openglatomshader.h"
#include "openglatomsphereshader.h"

class OpenGLTextRenderingShader: public OpenGLShader
{
public:
  OpenGLTextRenderingShader();
  ~OpenGLTextRenderingShader();
  void loadShader(void) override final;
  void paintGL(GLuint structureUniformBuffer);
  void reloadData();
  void initializeVertexArrayObject();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void deleteBuffers();
  void generateBuffers();
  void generateTextures();
  void initializeTransformUniforms();
  void initializeStructureUniforms();
  GLuint program() {return _program;}

  static QCache<QString, RKFontAtlas> _cachedFontAtlas;
private:
  GLuint _program;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;

  GLint _instancePositionAttributeLocation;
  GLint _instanceScaleAttributeLocation;
  GLint _instanceVertexAttributeLocation;
  GLint _instanceTextureUVAttributeLocation;
  GLint _fontAtlasTextureUniformLocation;

  std::vector<std::vector<size_t>> _numberOfDrawnAtoms;
  std::vector<std::vector<GLuint>> _vertexBuffer;
  std::vector<std::vector<GLuint>> _vertexArrayObject;
  std::vector<std::vector<GLuint>> _instanceBuffer;



  static const std::string _vertexShaderSource;
  static const std::string _geometryShaderSource;
  static const std::string _fragmentShaderSource;
};
