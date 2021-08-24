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

class OpenGLBlurShader: public OpenGLShader
{
public:
  OpenGLBlurShader();
  void loadShader(void) override final;
  GLuint blurredTexture() {return _blurVerticalTexture;}
  GLuint downSampledTexture() {return _blurDownSamplerTexture;}
  void paintGL(GLuint texture, int width, int height);
  void resizeGL(int width, int height);
  void initializeFramebuffers();
  void initializeVertexArrayObject();
  GLuint blurHorizontalTexture() {return _blurHorizontalTexture;}
private:
  GLuint _blurDownSamplerProgram = 0;
  GLuint _blurHorizontalProgram = 0;
  GLuint _blurVerticalProgram = 0;

  GLuint _blurDownSamplerFrameBufferObject = 0;
  GLuint _blurHorizontalFrameBufferObject = 0;
  GLuint _blurVerticalFrameBufferObject = 0;

  GLuint _blurDownSamplerTexture = 0;
  GLuint _blurHorizontalTexture = 0;
  GLuint _blurVerticalTexture = 0;

  GLint _blurDownSamplerInputTextureUniformLocation = 0;
  GLint _blurDownSamplerVertexPositionAttributeLocation = 0;

  GLint _blurHorizontalInputTextureUniformLocation = 0;
  GLint _blurHorizontalVertexPositionAttributeLocation = 0;

  GLint _blurVerticalInputTextureUniformLocation = 0;
  GLint _blurVerticalVertexPositionAttributeLocation = 0;

  GLuint _blurDownSamplerVertexArray = 0;
  GLuint _blurDownSamplerVertexBuffer = 0;
  GLuint _blurDownSamplerIndexBuffer = 0;

  GLuint _blurHorizontalVertexArray = 0;
  GLuint _blurHorizontalVertexBuffer = 0;
  GLuint _blurHorizontalIndexBuffer = 0;

  GLuint _blurVerticalVertexArray = 0;
  GLuint _blurVerticalVertexBuffer = 0;
  GLuint _blurVerticalIndexBuffer = 0;

  static const std::string _downSamplerVertexShaderSource;
  static const std::string _downSamplerFragmentShaderSource;
  static const std::string _blurHorizontalVertexShaderSource;
  static const std::string _blurVerticalVertexShaderSource;
  static const std::string _fragmentShaderSource;
};
