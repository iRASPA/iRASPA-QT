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
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  #include <QGLFunctions>
#else
  #include <QOpenGLFunctions>
#endif
#include "openglshader.h"
#include "rkrenderkitprotocols.h"
#include "openglatomselectionworleynoise3dshader.h"

class OpenGLAtomSelectionStripesPerspectiveImposterShader: public OpenGLShader
{
public:
  OpenGLAtomSelectionStripesPerspectiveImposterShader(OpenGLAtomSelectionWorleyNoise3DShader &atomSelectionShader);
  void loadShader(void) override final;
  void deleteBuffers();
  void generateBuffers();

  void paintGL(GLuint structureUniformBuffer);

  void reloadData();
  void initializeVertexArrayObject();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  GLuint program() {return _program;}
private:
  GLuint _program;
  OpenGLAtomSelectionWorleyNoise3DShader& _atomSelectionShader;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;

  std::vector<std::vector<size_t>> _numberOfIndices;

  std::vector<std::vector<GLuint>> _vertexBuffer;
  std::vector<std::vector<GLuint>> _indexBuffer;

  std::vector<std::vector<GLuint>> _vertexArrayObject;

  GLint _vertexPositionAttributeLocation;
  GLint _instancePositionAttributeLocation;
  GLint _scaleAttributeLocation;

  static const std::string _vertexShaderSource;
  static const std::string _fragmentShaderSource;
};
