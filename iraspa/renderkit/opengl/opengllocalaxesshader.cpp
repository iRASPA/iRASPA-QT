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

#include "opengllocalaxesshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "cylindergeometry.h"
#include "cappedcylindergeometry.h"
#include "uncappedcylindergeometry.h"
#include "axessystemdefaultgeometry.h"
#include "skboundingbox.h"
#include "rkrenderkitprotocols.h"

OpenGLLocalAxesShader::OpenGLLocalAxesShader()
{
}


void OpenGLLocalAxesShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLLocalAxesShader::paintGL(GLuint structureUniformBuffer)
{
  glUseProgram(_program);
  check_gl_error();

  int index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderLocalAxesSource* axes = dynamic_cast<RKRenderLocalAxesSource*>(_renderStructures[i][j].get()))
      {
        if(_renderStructures[i][j]->isVisible() && axes->renderLocalAxes().position() != RKLocalAxes::Position::none)
        {
          glBindVertexArray(_vertexArrayObject[i][j]);
          check_gl_error();

          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));
          check_gl_error();

          glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr);
          check_gl_error();
          glBindVertexArray(0);
        }
      }
      index++;
    }
  }
  glUseProgram(0);
}




void OpenGLLocalAxesShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glDeleteBuffers(size, _vertexBuffer[i].data());
    glDeleteBuffers(size, _indexBuffer[i].data());
    glDeleteVertexArrays(size, _vertexArrayObject[i].data());
  }
}

void OpenGLLocalAxesShader::generateBuffers()
{
  _numberOfIndices.resize(_renderStructures.size());

  _vertexBuffer.resize(_renderStructures.size());
  _indexBuffer.resize(_renderStructures.size());

  _vertexArrayObject.resize(_renderStructures.size());

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _numberOfIndices[i].resize(_renderStructures[i].size());
    _vertexBuffer[i].resize(_renderStructures[i].size());
    _indexBuffer[i].resize(_renderStructures[i].size());
    _vertexArrayObject[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glGenBuffers(size, _vertexBuffer[i].data());
    glGenBuffers(size, _indexBuffer[i].data());

    glGenVertexArrays(size, _vertexArrayObject[i].data());
  }
}

void OpenGLLocalAxesShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLLocalAxesShader::initializeVertexArrayObject()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderObject* object = dynamic_cast<RKRenderObject*>(_renderStructures[i][j].get()))
      {
        AxesSystemDefaultGeometry axesGeometry{};

        if (RKRenderLocalAxesSource* axes = dynamic_cast<RKRenderLocalAxesSource*>(_renderStructures[i][j].get()))
        {
          double length = axes->renderLocalAxes().length();
          double width = axes->renderLocalAxes().width();

          std::shared_ptr<SKCell> unitCell = object->cell();
          SKBoundingBox boundingBox = unitCell->boundingBox();

          switch(axes->renderLocalAxes().scalingType())
          {
            case RKLocalAxes::ScalingType::absolute:
              break;
            case RKLocalAxes::ScalingType::relative:
              length = boundingBox.shortestEdge() * axes->renderLocalAxes().length() / 100.0;
              break;
          }


          switch(axes->renderLocalAxes().style())
          {
          case RKLocalAxes::Style::defaultStyle:
            axesGeometry = AxesSystemDefaultGeometry(RKGlobalAxes::CenterType::cube, width, float4(1.0,1.0,1.0,1.0),
                                          length, width, float4(1.0,0.4,0.7,1.0), float4(0.7,1.0,0.4,1.0), float4(0.4,0.7,1.0,1.0),
                                          0.0, 1.0, float4(1.0,0.4,0.7,1.0), float4(0.7,1.0,0.4,1.0), float4(0.4,0.7,1.0,1.0),
                                          false, 1.0, 4);
            break;
          case RKLocalAxes::Style::defaultStyleRGB:
            axesGeometry = AxesSystemDefaultGeometry(RKGlobalAxes::CenterType::cube, width, float4(1.0,1.0,1.0,1.0),
                                          length, width, float4(1.0,0.0,0.0,1.0), float4(0.0,1.0,0.0,1.0), float4(0.0,0.0,1.0,1.0),
                                          0.0, 1.0, float4(1.0,0.0,0.0,1.0), float4(0.0,1.0,0.0,1.0), float4(0.0,0.0,1.0,1.0),
                                          false, 1.0, 4);
            break;
          case RKLocalAxes::Style::cylinder:
            axesGeometry = AxesSystemDefaultGeometry(RKGlobalAxes::CenterType::cube, width, float4(1.0,1.0,1.0,1.0),
                                          length, width, float4(1.0,0.4,0.0,7.0), float4(0.7,1.0,0.4,1.0), float4(0.4,0.7,1.0,1.0),
                                          0.0, 1.0, float4(1.0,0.4,0.7,1.0), float4(0.7,1.0,0.4,1.0), float4(0.4,0.7,1.0,1.0),
                                          false, 1.0, 41);
            break;
          case RKLocalAxes::Style::cylinderRGB:
            axesGeometry = AxesSystemDefaultGeometry(RKGlobalAxes::CenterType::cube, width, float4(1.0,1.0,1.0,1.0),
                                          length, width, float4(1.0,0.0,0.0,1.0), float4(0.0,1.0,0.0,1.0), float4(0.0,0.0,1.0,1.0),
                                          0.0, 1.0, float4(1.0,0.0,0.0,1.0), float4(0.0,1.0,0.0,1.0), float4(0.0,0.0,1.0,1.0),
                                          false, 1.0, 41);
            break;


          }

        }

        glBindVertexArray(_vertexArrayObject[i][j]);
        check_gl_error();
        glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer[i][j]);
        if(axesGeometry.vertices().size()>0)
        {
          glBufferData(GL_ARRAY_BUFFER, axesGeometry.vertices().size() * sizeof(RKPrimitiveVertex), axesGeometry.vertices().data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }
        _numberOfIndices[i][j] = axesGeometry.indices().size();

        glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKPrimitiveVertex), (GLvoid *)offsetof(RKPrimitiveVertex,position));
        glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKPrimitiveVertex), (GLvoid *)offsetof(RKPrimitiveVertex,normal));
        glVertexAttribPointer(_vertexColorAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKPrimitiveVertex), (GLvoid *)offsetof(RKPrimitiveVertex,color));

        check_gl_error();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer[i][j]);
        if(axesGeometry.indices().size()>0)
        {
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, axesGeometry.indices().size() * sizeof(GLshort), axesGeometry.indices().data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }

        glEnableVertexAttribArray(_vertexPositionAttributeLocation);
        glEnableVertexAttribArray(_vertexNormalAttributeLocation);
        glEnableVertexAttribArray(_vertexColorAttributeLocation);
        check_gl_error();
        glBindVertexArray(0);
      }
    }
  }
}

void OpenGLLocalAxesShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLLocalAxesShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLLocalAxesShader::_fragmentShaderSource.c_str());

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

void OpenGLLocalAxesShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
}

void OpenGLLocalAxesShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "StructureUniformBlock"), 1);
}


void OpenGLLocalAxesShader::initializeLightUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "LightsUniformBlock"), 3);
}

const std::string  OpenGLLocalAxesShader::_vertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
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
  vec4 pos = vertexPosition +structureUniforms.localAxisPosition;

  // Calculate normal in view-space
  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vertexNormal).xyz;

  vs_out.diffuse = vertexColor;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
}
)foo";

const std::string  OpenGLLocalAxesShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
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

