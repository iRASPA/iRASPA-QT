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

#include "openglatomselectionstripesshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"
#include "opengluniformstringliterals.h"

OpenGLAtomSelectionStripesShader::OpenGLAtomSelectionStripesShader()
{

}

void OpenGLAtomSelectionStripesShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLAtomSelectionStripesShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _instancePositionBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _scaleBuffer[i].data());

    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());
  }
}

void OpenGLAtomSelectionStripesShader::generateBuffers()
{
  _numberOfDrawnAtoms.resize(_renderStructures.size());
  _numberOfIndices.resize(_renderStructures.size());

  _vertexBuffer.resize(_renderStructures.size());
  _indexBuffer.resize(_renderStructures.size());
  _instancePositionBuffer.resize(_renderStructures.size());
  _scaleBuffer.resize(_renderStructures.size());

  _vertexArrayObject.resize(_renderStructures.size());

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _numberOfDrawnAtoms[i].resize(_renderStructures[i].size());
    _numberOfIndices[i].resize(_renderStructures[i].size());

    _vertexBuffer[i].resize(_renderStructures[i].size());
    _indexBuffer[i].resize(_renderStructures[i].size());
    _instancePositionBuffer[i].resize(_renderStructures[i].size());
    _scaleBuffer[i].resize(_renderStructures[i].size());

    _vertexArrayObject[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _instancePositionBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _scaleBuffer[i].data());

    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());
  }
}

void OpenGLAtomSelectionStripesShader::paintGL(GLuint structureUniformBuffer)
{
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glUseProgram(_program);
  check_gl_error();

  int index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderAtomSource* source = dynamic_cast<RKRenderAtomSource*>(_renderStructures[i][j].get()))
      {
        if(source->atomSelectionStyle() == RKSelectionStyle::striped && source->drawAtoms() && _renderStructures[i][j]->isVisible() && _numberOfIndices[i][j]>0 && _numberOfDrawnAtoms[i][j]>0)
        {
          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));

          glBindVertexArray(_vertexArrayObject[i][j]);
          check_gl_error();

          glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfDrawnAtoms[i][j]));
          check_gl_error();
          glBindVertexArray(0);
        }
      }
      index++;
    }
  }
  glUseProgram(0);

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void OpenGLAtomSelectionStripesShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLAtomSelectionStripesShader::initializeVertexArrayObject()
{
  SphereGeometry sphere = SphereGeometry(1.0,41);

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderAtomSource* source = dynamic_cast<RKRenderAtomSource*>(_renderStructures[i][j].get()))
      {
        std::vector<RKInPerInstanceAttributesAtoms> atomData = source->renderSelectedAtoms();
        _numberOfDrawnAtoms[i][j] = atomData.size();
        _numberOfIndices[i][j] = sphere.indices().size();

        glBindVertexArray(_vertexArrayObject[i][j]);

        // each crystal can have a different number of sphere-slices
        glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer[i][j]);

        if(sphere.vertices().size()>0)
        {
          glBufferData(GL_ARRAY_BUFFER, sphere.vertices().size() * sizeof(RKVertex), sphere.vertices().data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }

        glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex,position)));
        glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex,normal)));

        glBindBuffer(GLenum(GL_ELEMENT_ARRAY_BUFFER), _indexBuffer[i][j]);
        if(sphere.indices().size()>0)
        {
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices().size() * sizeof(GLshort), sphere.indices().data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }

        glBindBuffer(GLenum(GL_ARRAY_BUFFER), _instancePositionBuffer[i][j]);
        if(_numberOfDrawnAtoms[i][j]>0)
        {
          glBufferData(GL_ARRAY_BUFFER, _numberOfDrawnAtoms[i][j]*sizeof(RKInPerInstanceAttributesAtoms), atomData.data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }

        glVertexAttribPointer(_instancePositionAttributeLocation, 4, GL_FLOAT, GLboolean(GL_FALSE), sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,position)));
        glVertexAttribDivisor(_instancePositionAttributeLocation, 1);

        glVertexAttribPointer(_scaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,scale)));
        glVertexAttribDivisor(_scaleAttributeLocation, 1);

        glEnableVertexAttribArray(_vertexPositionAttributeLocation);
        glEnableVertexAttribArray(_vertexNormalAttributeLocation);
        glEnableVertexAttribArray(_instancePositionAttributeLocation);
        glEnableVertexAttribArray(_scaleAttributeLocation);

        glBindBuffer(GLenum(GL_ARRAY_BUFFER), 0);
        glBindVertexArray(0);
      }
    }
  }
}

void OpenGLAtomSelectionStripesShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLAtomSelectionStripesShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLAtomSelectionStripesShader::_fragmentShaderSource.c_str());

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
    _vertexNormalAttributeLocation   = glGetAttribLocation(_program, "vertexNormal");
    _instancePositionAttributeLocation = glGetAttribLocation(_program, "instancePosition");

    _scaleAttributeLocation = glGetAttribLocation(_program, "instanceScale");

    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_vertexNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_instancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition' attribute.";
    if (_scaleAttributeLocation  < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string  OpenGLAtomSelectionStripesShader::_vertexShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
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
  smooth vec3 ModelN;
  smooth vec3 L;
  smooth vec3 V;
} vs_out;

void main(void)
{
  vec4 scale = structureUniforms.atomSelectionScaling * structureUniforms.atomScaleFactor * instanceScale;
  vec4 pos = instancePosition + scale*vertexPosition;

  // Calculate normal in modelview-space
  vs_out.ModelN = vertexNormal.xyz;
  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vertexNormal).xyz;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  // Calculate view vector
  vs_out.V = -P.xyz;

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
}
)foo";

const std::string  OpenGLAtomSelectionStripesShader::_fragmentShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
R"foo(
// Input from vertex shader
in VS_OUT
{
  smooth vec3 N;
  smooth vec3 ModelN;
  smooth vec3 L;
  smooth vec3 V;
} fs_in;

out  vec4 vFragColor;

vec2 textureCoordinateForSphereSurfacePosition(vec3 sphereSurfacePosition)
{
  vec3 absoluteSphereSurfacePosition = abs(sphereSurfacePosition);
  float d = absoluteSphereSurfacePosition.x + absoluteSphereSurfacePosition.y + absoluteSphereSurfacePosition.z;

  return (sphereSurfacePosition.z > 0.0) ? sphereSurfacePosition.xy / d : sign(sphereSurfacePosition.xy) * ( 1.0 - absoluteSphereSurfacePosition.yx / d);
}

vec3 rgb2hsv(vec3 c)
{
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main(void)
{
  // Normalize the incoming N, L and V vectors
  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L);

  // Compute the diffuse and specular components for each fragment
  vec4 color = max(dot(N, L), 0.0) * vec4(1.0,1.0,0.0,1.0);

  vec3 t1 = fs_in.ModelN.xyz;

  vec2  st = vec2(0.5 + 0.5 * atan(t1.z, t1.x)/3.141592653589793, 0.5 - asin(t1.y)/3.141592653589793);
  float uDensity = structureUniforms.atomSelectionStripesDensity;
  float frequency = structureUniforms.atomSelectionStripesFrequency;
  if (fract(st.x*frequency) >= uDensity && fract(st.y*frequency) >= uDensity)
    discard;

  if (structureUniforms.atomHDR)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.atomHDRExposure);
    vLdrColor.a = 1.0;
    color= vLdrColor;
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.atomHue;
  hsv.y = hsv.y * structureUniforms.atomSaturation;
  hsv.z = hsv.z * structureUniforms.atomValue;
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.atomSelectionIntensity;
  vFragColor = vec4(hsv2rgb(hsv)*bloomLevel,bloomLevel);
}
)foo";

