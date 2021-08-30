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

#include "openglbackgroundshader.h"
#include <iostream>
#include "glgeterror.h"
#include "rkrenderuniforms.h"
#include "backplanegeometry.h"
#include "quadgeometry.h"
#include "glgeterror.h"

OpenGLBackgroundShader::OpenGLBackgroundShader()
{

}


void OpenGLBackgroundShader::paintGL()
{
  glDepthMask(false);

  glUseProgram(_program);
  glBindVertexArray(_vertexArray);
  check_gl_error();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _backgroundTexture);
  glUniform1i(_textureUniformLocation, 0);
  check_gl_error();

  glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT, nullptr);
  check_gl_error();

  glBindVertexArray(0);
  glUseProgram(0);

  glDepthMask(true);
}

void OpenGLBackgroundShader::reload(std::shared_ptr<RKRenderDataSource> source)
{
  QImage image;
  if(source)
  {
    image = source->renderBackgroundCachedImage().convertToFormat(QImage::Format_RGBA8888);
  }
  else
  {
    image = QImage(QSize(64,64),QImage::Format_RGBA8888);
    image.fill(QColor(255,255,255,255));
  }
  glBindTexture(GL_TEXTURE_2D, _backgroundTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  check_gl_error();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, 0);
}



void OpenGLBackgroundShader::initializeVertexArrayObject()
{
  glGenVertexArrays(1, &_vertexArray);
  glGenBuffers(1, &_vertexBuffer);
  glGenBuffers(1, &_indexBuffer);

  glGenTextures(1, &_backgroundTexture);
  check_gl_error();


  BackPlaneGeometry quad = BackPlaneGeometry();

  glBindVertexArray(_vertexArray);
   check_gl_error();
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
  if(quad.vertices().size()>0)
  {
    glBufferData(GL_ARRAY_BUFFER, quad.vertices().size()*sizeof(RKVertex), quad.vertices().data(), GL_STATIC_DRAW);
    check_gl_error();
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
  if(quad.indices().size()>0)
  {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad.indices().size() * sizeof(GLushort), quad.indices().data(), GL_STATIC_DRAW);
    check_gl_error();
  }

  glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, position)));
  glEnableVertexAttribArray(_vertexPositionAttributeLocation);
  check_gl_error();
  glBindVertexArray(0);

  QImage image = QImage(QSize(64,64),QImage::Format_RGBA8888);
  image.fill(QColor(255,255,255,255));

  glBindTexture(GL_TEXTURE_2D, _backgroundTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  check_gl_error();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLBackgroundShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLBackgroundShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLBackgroundShader::_fragmentShaderSource.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _program = glCreateProgram();
    check_gl_error();

    glAttachShader(_program, vertexShader);
    check_gl_error();
    glAttachShader(_program, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_program, 0, "vFragColor");

    linkProgram(_program);

    _textureUniformLocation = glGetUniformLocation(_program, "backgroundTexture");
    _vertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string OpenGLBackgroundShader::_vertexShaderSource  =
OpenGLVersionStringLiteral +
std::string(R"foo(
in vec4 vertexPosition;
out vec2 texcoord;

void main()
{
  gl_Position = vertexPosition;
  texcoord = vertexPosition.xy * vec2(0.5) + vec2(0.5);
}
)foo");

const std::string OpenGLBackgroundShader::_fragmentShaderSource  =
OpenGLVersionStringLiteral +
std::string(R"foo(
uniform sampler2D backgroundTexture;

in vec2 texcoord;
out vec4 vFragColor;

void main()
{
  vFragColor =  texture(backgroundTexture,texcoord);
}
)foo");
