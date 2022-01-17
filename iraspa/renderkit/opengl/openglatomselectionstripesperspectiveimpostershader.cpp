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

#include "openglatomselectionstripesperspectiveimpostershader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "openglatomselectionworleynoise3dshader.h"
#include "quadgeometry.h"

OpenGLAtomSelectionStripesPerspectiveImposterShader::OpenGLAtomSelectionStripesPerspectiveImposterShader(OpenGLAtomSelectionWorleyNoise3DShader &atomSelectionShader): _atomSelectionShader(atomSelectionShader)
{

}

void OpenGLAtomSelectionStripesPerspectiveImposterShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLAtomSelectionStripesPerspectiveImposterShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexBuffer[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());
  }
}

void OpenGLAtomSelectionStripesPerspectiveImposterShader::generateBuffers()
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
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexBuffer[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());
  }
}

void OpenGLAtomSelectionStripesPerspectiveImposterShader::paintGL(GLuint structureUniformBuffer)
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
        if(source->atomSelectionStyle() == RKSelectionStyle::striped && source->drawAtoms() && _renderStructures[i][j]->isVisible() && _numberOfIndices[i][j]>0 && _atomSelectionShader._numberOfDrawnAtoms[i][j]>0)
        {
          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));
          glBindVertexArray(_vertexArrayObject[i][j]);
          check_gl_error();

          glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr,(GLsizei)_atomSelectionShader._numberOfDrawnAtoms[i][j]);
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

void OpenGLAtomSelectionStripesPerspectiveImposterShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLAtomSelectionStripesPerspectiveImposterShader::initializeVertexArrayObject()
{
  QuadGeometry quad = QuadGeometry();

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      glBindVertexArray(_vertexArrayObject[i][j]);
      check_gl_error();

      _numberOfIndices[i][j] = quad.indices().size();

      // each crystal can have a different number of sphere-slices
      glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer[i][j]);
      check_gl_error();

      if(quad.vertices().size()>0)
      {
        glBufferData(GL_ARRAY_BUFFER, quad.vertices().size() * sizeof(RKVertex), quad.vertices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex,position)));
      check_gl_error();


      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer[i][j]);
      check_gl_error();

      if(quad.indices().size()>0)
      {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad.indices().size() * sizeof(GLshort), quad.indices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glBindBuffer(GL_ARRAY_BUFFER, _atomSelectionShader._instancePositionBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_instancePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,position)));
      check_gl_error();
      glVertexAttribDivisor(_instancePositionAttributeLocation, 1);
      check_gl_error();

      glVertexAttribPointer(_scaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,scale)));
      check_gl_error();
      glVertexAttribDivisor(_scaleAttributeLocation, 1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_instancePositionAttributeLocation);
      glEnableVertexAttribArray(_scaleAttributeLocation);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
  }
}

void OpenGLAtomSelectionStripesPerspectiveImposterShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLAtomSelectionStripesPerspectiveImposterShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLAtomSelectionStripesPerspectiveImposterShader::_fragmentShaderSource.c_str());

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
    _instancePositionAttributeLocation = glGetAttribLocation(_program, "instancePosition");
    _scaleAttributeLocation = glGetAttribLocation(_program, "instanceScale");

    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition' attribute.";
    if (_scaleAttributeLocation  < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}


const std::string  OpenGLAtomSelectionStripesPerspectiveImposterShader::_vertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
R"foo(
// Inputs from vertex shader
out VS_OUT
{
  smooth vec4 eye_position;
  smooth vec2 texcoords;
  smooth vec3 frag_pos;
  flat vec3 frag_center;
  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
  flat vec4 sphere_radius;
  flat mat4x4 ambientOcclusionTransformMatrix;
} vs_out;


in vec4 vertexPosition;

in vec4 instancePosition;
in vec4 instanceScale;


void main(void)
{
  vec4 scale = structureUniforms.atomSelectionScaling * structureUniforms.atomScaleFactor * instanceScale;

  vs_out.N = vec3(0,0,1);

  vs_out.eye_position = frameUniforms.viewMatrix * structureUniforms.modelMatrix * instancePosition;
  vs_out.ambientOcclusionTransformMatrix = transpose(frameUniforms.normalMatrix * structureUniforms.modelMatrix);
  vs_out.frag_center= vs_out.eye_position.xyz;

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - vs_out.eye_position*lightUniforms.lights[0].position.w).xyz;

  // Calculate view vector
  vs_out.V = -vs_out.eye_position.xyz;

  vs_out.texcoords = vertexPosition.xy;
  vs_out.sphere_radius = scale;
  vec4 pos2 = frameUniforms.viewMatrix * structureUniforms.modelMatrix * instancePosition;
  pos2.xy += 1.5 * scale.xy * vec2(vertexPosition.x,vertexPosition.y);

  vs_out.frag_pos = pos2.xyz;

  gl_Position =  frameUniforms.projectionMatrix * pos2;
}
)foo";

const std::string  OpenGLAtomSelectionStripesPerspectiveImposterShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
R"foo(
// Inputs from vertex shader
in VS_OUT
{
  smooth vec4 eye_position;
  smooth vec2 texcoords;
  smooth vec3 frag_pos;
  flat vec3 frag_center;
  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
  flat vec4 sphere_radius;
  flat mat4x4 ambientOcclusionTransformMatrix;
} fs_in;


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

out vec4 vFragColor;


void main(void)
{
  vec3 rij = -fs_in.frag_center;
  vec3 vij = fs_in.frag_pos;

  float A = dot(vij, vij);
  float B = dot(rij, vij);
  float C = dot(rij, rij) - fs_in.sphere_radius.z * fs_in.sphere_radius.z;
  float argument = B * B - A * C;
  if (argument < 0.0) discard;

  float t = - C / (B - sqrt(argument));
  vec3 hit = t * vij;

  vec4 screen_pos = frameUniforms.projectionMatrix * vec4(hit, 1.0);
  gl_FragDepth = 0.5*(screen_pos.z / screen_pos.w)+0.5;


  // Normalize the incoming N, L and V vectors
  vec3 N = normalize(hit - fs_in.frag_center);
  vec3 L = normalize(fs_in.L);

  // Compute the diffuse and specular components for each fragment
  vec4 color = max(dot(N, L), 0.0) * vec4(1.0,1.0,0.0,1.0);

  vec3 t1 = (fs_in.ambientOcclusionTransformMatrix * vec4(N,1.0)).xyz;

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
