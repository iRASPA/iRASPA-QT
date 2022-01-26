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

#include "openglboundingboxsphereshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"
#include "cylindergeometry.h"
#include "opengluniformstringliterals.h"

OpenGLBoundingBoxSphereShader::OpenGLBoundingBoxSphereShader()
{

}

void OpenGLBoundingBoxSphereShader::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = source;
}


void OpenGLBoundingBoxSphereShader::paintGL()
{ 
  if(std::shared_ptr<RKRenderDataSource> dataSource = _dataSource)
  {
    if(dataSource->showBoundingBox())
    {
      glUseProgram(_program);
      check_gl_error();

      glBindVertexArray(_vertexArrayObject);
      check_gl_error();

      glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices), GL_UNSIGNED_SHORT, nullptr,(GLsizei)_numberOfUnitCellSpheres);
      check_gl_error();
      glBindVertexArray(0);

      glUseProgram(0);
    }
  }
}

void OpenGLBoundingBoxSphereShader::generateBuffers()
{
  glGenVertexArrays(1, &_vertexArrayObject);
  glGenBuffers(1, &_vertexInstanceBuffer);
  glGenBuffers(1, &_vertexBuffer);
  glGenBuffers(1, &_indexBuffer);
}

void OpenGLBoundingBoxSphereShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLBoundingBoxSphereShader::initializeVertexArrayObject()
{
  SphereGeometry sphere = SphereGeometry(1.0,41);

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
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));

      check_gl_error();
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
      if(sphere.indices().size()>0)
      {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices().size() * sizeof(GLshort), sphere.indices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      std::vector<RKInPerInstanceAttributesAtoms> unitCellSphereInstanceData = dataSource->renderBoundingBoxSpheres();
      _numberOfUnitCellSpheres = unitCellSphereInstanceData.size();

      glBindBuffer(GL_ARRAY_BUFFER, _vertexInstanceBuffer);
      if(unitCellSphereInstanceData.size()>0)
      {
        glBufferData(GL_ARRAY_BUFFER, unitCellSphereInstanceData.size()*sizeof(RKInPerInstanceAttributesAtoms), unitCellSphereInstanceData.data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }
      glVertexAttribPointer(_instancePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,position));
      glVertexAttribDivisor(_instancePositionAttributeLocation,1);
check_gl_error();


      glVertexAttribPointer(_scaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,scale));
      glVertexAttribDivisor(_scaleAttributeLocation, 1);
check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionAttributeLocation);
      glEnableVertexAttribArray(_scaleAttributeLocation);
check_gl_error();
      glBindVertexArray(0);
  }
}


void OpenGLBoundingBoxSphereShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER, OpenGLBoundingBoxSphereShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER, OpenGLBoundingBoxSphereShader::_fragmentShaderSource.c_str());

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

    _vertexNormalAttributeLocation = glGetAttribLocation(_program, "vertexNormal");
    _vertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");
    _instancePositionAttributeLocation = glGetAttribLocation(_program, "instancePosition");
    _scaleAttributeLocation = glGetAttribLocation(_program, "instanceScale");

    if (_vertexNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition' attribute.";
    if (_scaleAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}


const std::string  OpenGLBoundingBoxSphereShader::_vertexShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;
in vec4 vertexNormal;

in vec4 instancePosition;
in vec4 instanceScale;

// Inputs from vertex shader
out VS_OUT
{
  smooth vec3 N;
  smooth vec3 L;
  smooth vec4 diffuse;
} vs_out;


void main(void)
{
  vec4 scale = instanceScale;
  vec4 pos = instancePosition + scale*vertexPosition;

  // Calculate normal in view-space
  vs_out.N = (frameUniforms.normalMatrix * vertexNormal).xyz;

  vs_out.diffuse = vec4(1.0,1.0,1.0,1.0);

  vec4 P = frameUniforms.viewMatrix * pos;

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  gl_Position = frameUniforms.mvpMatrix * pos;
}
)foo";

const std::string  OpenGLBoundingBoxSphereShader::_fragmentShaderSource =
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
  vFragColor = vec4((vec3(0.0,0.75,1.0) * color.xyz), 1.0);
}
)foo";

