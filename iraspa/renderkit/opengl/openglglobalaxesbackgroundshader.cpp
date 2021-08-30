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

#include "openglglobalaxesbackgroundshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "backplanegeometry.h"
#include "quadgeometry.h"

OpenGLGlobalAxesBackgroundShader::OpenGLGlobalAxesBackgroundShader()
{

}

void OpenGLGlobalAxesBackgroundShader::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = source;
}


void OpenGLGlobalAxesBackgroundShader::paintGL()
{
  if(std::shared_ptr<RKRenderDataSource> dataSource = _dataSource)
  {
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(_program);
    check_gl_error();

    glBindVertexArray(_vertexArrayObject);
    check_gl_error();

    glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices), GL_UNSIGNED_SHORT, nullptr);
    check_gl_error();
    glBindVertexArray(0);

    glUseProgram(0);

    glDisable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
  }
}

void OpenGLGlobalAxesBackgroundShader::generateBuffers()
{
  glGenVertexArrays(1, &_vertexArrayObject);
  glGenBuffers(1, &_vertexBuffer);
  glGenBuffers(1, &_indexBuffer);
}

void OpenGLGlobalAxesBackgroundShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLGlobalAxesBackgroundShader::initializeVertexArrayObject()
{
  BackPlaneGeometry sphere = BackPlaneGeometry();
  //  QuadGeometry sphere = QuadGeometry();

  if(std::shared_ptr<RKRenderDataSource> dataSource = _dataSource)
  {
      glBindVertexArray(_vertexArrayObject);
      check_gl_error();
      glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
      if(sphere.vertices().size()>0)
      {
        glBufferData(GL_ARRAY_BUFFER, sphere.vertices().size() * sizeof(RKVertex), sphere.vertices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }
      _numberOfIndices = sphere.indices().size();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));

      check_gl_error();
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
      if(sphere.indices().size()>0)
      {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices().size() * sizeof(GLshort), sphere.indices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }
check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
check_gl_error();
      glBindVertexArray(0);
  }
}


void OpenGLGlobalAxesBackgroundShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER, OpenGLGlobalAxesBackgroundShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER, OpenGLGlobalAxesBackgroundShader::_fragmentShaderSource.c_str());

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

    _vertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");

    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}


const std::string  OpenGLGlobalAxesBackgroundShader::_vertexShaderSource =
OpenGLVersionStringLiteral +
R"foo(
in vec4 vertexPosition;

out vec2 texcoord;

void main()
{
  gl_Position = vertexPosition;
  texcoord = vertexPosition.xy * vec2(0.5) + vec2(0.5);
}
)foo";

const std::string  OpenGLGlobalAxesBackgroundShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLGlobalAxesUniformBlockStringLiteral +
R"foo(
in vec2 texcoord;
out vec4 vFragColor;

// https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float Sphere(vec2 p, float s)
{
  return length(p)-s;
}

float RoundedBox( vec2 p, vec2 b, float r )
{
  return length(max(abs(p)-b,0.0))-r;
}

float RoundedBoxBorder( vec2 p, vec2 b, float r, float borderFactor )
{
  return max(-RoundedBox(p, b*borderFactor, r), RoundedBox(p, b, r));
}

float Rectangle(vec2 uv, vec2 pos, vec2 size)
{
  return (step(pos.x, uv.x)         - step(pos.x + size.x,uv.x))
       * (step(pos.y - size.y,uv.y) - step(pos.y, uv.y));
}

void main(void)
{
  float alpha = globalAxesUniforms.axesBackgroundColor.w;

  switch(globalAxesUniforms.axesBackGroundStyle)
  {
    case 0:
      alpha = 0.0;
    case 1: // filled circle
      if (Sphere(texcoord - vec2(0.5,0.5), 0.5) > 0.0)
        alpha = 0.0;
    case 2: // filled square
      break;
    case 3:  // filled square
      if(RoundedBox(texcoord - vec2(0.5,0.5), vec2(0.3,0.3), 0.2 )>0.0)
        alpha = 0.0;
      break;
    case 4:  // circle
      if (max(-Sphere(texcoord - vec2(0.5,0.5), 0.48), Sphere(texcoord - vec2(0.5,0.5), 0.5)) > 0.0)
        alpha = 0.0;
      break;
    case 5:  // square
      if(Rectangle(texcoord- vec2(0.5,0.5), vec2(-0.48,0.48), vec2(0.96,0.96)) > 0.0)
        alpha = 0.0;
      break;
    case 6:  //  rounded square
      if(max(-RoundedBox(texcoord - vec2(0.5,0.5), vec2(0.30,0.30), 0.17 ), RoundedBox(texcoord - vec2(0.5,0.5), vec2(0.3,0.3), 0.2 ))>0.0)
        alpha = 0.0;
    default:
      break;
  }

  vFragColor = vec4(globalAxesUniforms.axesBackgroundColor.x * alpha,
                    globalAxesUniforms.axesBackgroundColor.y * alpha,
                    globalAxesUniforms.axesBackgroundColor.z * alpha,
                    alpha);

}
)foo";

