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

#include "openglglobalaxessystemshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "axessystemdefaultgeometry.h"
#include "nspheregeometry.h"
#include "cubeprimitivegeometry.h"
#include "arrowxgeometry.h"
#include "opengluniformstringliterals.h"

OpenGLGlobalAxesSystemShader::OpenGLGlobalAxesSystemShader()
{

}

void OpenGLGlobalAxesSystemShader::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = source;
}


void OpenGLGlobalAxesSystemShader::paintGL()
{
  if(std::shared_ptr<RKRenderDataSource> dataSource = _dataSource)
  {
    glEnable(GL_DEPTH_CLAMP);
    glUseProgram(_program);
    check_gl_error();

    glBindVertexArray(_vertexArrayObject);
    check_gl_error();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_numberOfIndices), GL_UNSIGNED_SHORT, nullptr);
    check_gl_error();
    glBindVertexArray(0);

    glUseProgram(0);
    glDisable(GL_DEPTH_CLAMP);
  }
}

void OpenGLGlobalAxesSystemShader::generateBuffers()
{
  glGenVertexArrays(1, &_vertexArrayObject);
  glGenBuffers(1, &_vertexBuffer);
  glGenBuffers(1, &_indexBuffer);
}

void OpenGLGlobalAxesSystemShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLGlobalAxesSystemShader::initializeVertexArrayObject()
{
  AxesSystemDefaultGeometry axes{};
  if(_dataSource)
  {
    RKGlobalAxes::CenterType centerType = _dataSource->axes()->centerType();
    float centerRadius = _dataSource->axes()->centerScale();
    float4 centerColor = float4(_dataSource->axes()->centerDiffuseColor().redF(),
                                _dataSource->axes()->centerDiffuseColor().greenF(),
                                _dataSource->axes()->centerDiffuseColor().blueF(),
                                _dataSource->axes()->centerDiffuseColor().alphaF());
    float arrowHeight = _dataSource->axes()->shaftLength();
    float arrowRadius = _dataSource->axes()->shaftWidth();
    float4 arrowColorX = float4(_dataSource->axes()->axisXDiffuseColor().redF(),
                                _dataSource->axes()->axisXDiffuseColor().greenF(),
                                _dataSource->axes()->axisXDiffuseColor().blueF(),
                                _dataSource->axes()->axisXDiffuseColor().alphaF());
    float4 arrowColorY = float4(_dataSource->axes()->axisYDiffuseColor().redF(),
                                _dataSource->axes()->axisYDiffuseColor().greenF(),
                                _dataSource->axes()->axisYDiffuseColor().blueF(),
                                _dataSource->axes()->axisYDiffuseColor().alphaF());
    float4 arrowColorZ = float4(_dataSource->axes()->axisZDiffuseColor().redF(),
                                _dataSource->axes()->axisZDiffuseColor().greenF(),
                                _dataSource->axes()->axisZDiffuseColor().blueF(),
                                _dataSource->axes()->axisZDiffuseColor().alphaF());
    float tipHeight = _dataSource->axes()->tipLength();
    float tipRadius = _dataSource->axes()->tipWidth();
    float4 tipColorX = float4(_dataSource->axes()->axisXDiffuseColor().redF(),
                              _dataSource->axes()->axisXDiffuseColor().greenF(),
                              _dataSource->axes()->axisXDiffuseColor().blueF(),
                              _dataSource->axes()->axisXDiffuseColor().alphaF());
    float4 tipColorY = float4(_dataSource->axes()->axisYDiffuseColor().redF(),
                              _dataSource->axes()->axisYDiffuseColor().greenF(),
                              _dataSource->axes()->axisYDiffuseColor().blueF(),
                              _dataSource->axes()->axisYDiffuseColor().alphaF());
    float4 tipColorZ = float4(_dataSource->axes()->axisZDiffuseColor().redF(),
                              _dataSource->axes()->axisZDiffuseColor().greenF(),
                              _dataSource->axes()->axisZDiffuseColor().blueF(),
                              _dataSource->axes()->axisZDiffuseColor().alphaF());
    bool tipVisibility = _dataSource->axes()->tipVisibility();
    float aspectRatio = _dataSource->axes()->aspectRatio();
    int sectorCount = _dataSource->axes()->NumberOfSectors();

    axes = AxesSystemDefaultGeometry(centerType, centerRadius, centerColor,
                                       arrowHeight, arrowRadius, arrowColorX, arrowColorY, arrowColorZ,
                                       tipHeight, tipRadius, tipColorX, tipColorY, tipColorZ,
                                       tipVisibility, aspectRatio, sectorCount);
  }

  if(std::shared_ptr<RKRenderDataSource> dataSource = _dataSource)
  {
      glBindVertexArray(_vertexArrayObject);
      check_gl_error();
      glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
      if(axes.vertices().size()>0)
      {
        glBufferData(GL_ARRAY_BUFFER, axes.vertices().size() * sizeof(RKPrimitiveVertex), axes.vertices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }
      _numberOfIndices = axes.indices().size();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKPrimitiveVertex), (GLvoid *)offsetof(RKPrimitiveVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKPrimitiveVertex), (GLvoid *)offsetof(RKPrimitiveVertex,normal));
      glVertexAttribPointer(_vertexColorAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKPrimitiveVertex), (GLvoid *)offsetof(RKPrimitiveVertex,color));

      check_gl_error();
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
      if(axes.indices().size()>0)
      {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, axes.indices().size() * sizeof(GLshort), axes.indices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_vertexColorAttributeLocation);
check_gl_error();
      glBindVertexArray(0);
  }
}


void OpenGLGlobalAxesSystemShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER, OpenGLGlobalAxesSystemShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER, OpenGLGlobalAxesSystemShader::_fragmentShaderSource.c_str());

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
    _vertexNormalAttributeLocation = glGetAttribLocation(_program, "vertexNormal");
    _vertexColorAttributeLocation = glGetAttribLocation(_program, "vertexColor");

    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_vertexNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_vertexColorAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexColor' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}


const std::string  OpenGLGlobalAxesSystemShader::_vertexShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLGlobalAxesUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;
in vec4 vertexNormal;
in vec4 vertexColor;

// Inputs from vertex shader
out VS_OUT
{
  smooth vec3 N;
  smooth vec3 L;
  smooth vec4 diffuse;
} vs_out;


void main(void)
{
  vec4 scale = vec4(globalAxesUniforms.axesScale,globalAxesUniforms.axesScale,globalAxesUniforms.axesScale,1.0);
  vec4 pos = scale*vertexPosition + vec4(0.0,0.0,0.0,1.0);

  // Calculate normal in view-space
  vs_out.N = (frameUniforms.normalMatrix * vertexNormal).xyz;

  vs_out.diffuse = vertexColor;

  vec4 P = frameUniforms.axesViewMatrix * pos;

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  gl_Position = frameUniforms.axesMvpMatrix * pos;
}
)foo";

const std::string  OpenGLGlobalAxesSystemShader::_fragmentShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
R"foo(
// Input from vertex shader
in VS_OUT
{
  smooth vec3 N;
  smooth vec3 L;
  smooth vec4 diffuse;
} fs_in;

out  vec4 vFragColor;

void main(void)
{
  // Normalize the incoming N, L and V vectors
  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L);

  // Calculate R locally
  vec3 R = reflect(-L, N);

  vec4 color = max(dot(N, L), 0.0) * fs_in.diffuse;

  vec4 vLdrColor = 1.0 - exp2(-color * 1.5);
  vLdrColor.a = 1.0;
  color= vLdrColor;

  vFragColor = vec4(color.xyz, 1.0);
}
)foo";

