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

#include "openglatomselectionworleynoise3dshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"

OpenGLAtomSelectionWorleyNoise3DShader::OpenGLAtomSelectionWorleyNoise3DShader()
{

}

void OpenGLAtomSelectionWorleyNoise3DShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLAtomSelectionWorleyNoise3DShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _instancePositionBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _scaleBuffer[i].data());

    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _ambientColorBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _diffuseColorBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _specularColorBuffer[i].data());
  }
}

void OpenGLAtomSelectionWorleyNoise3DShader::generateBuffers()
{
  _numberOfDrawnAtoms.resize(_renderStructures.size());
  _numberOfIndices.resize(_renderStructures.size());

  _vertexBuffer.resize(_renderStructures.size());
  _indexBuffer.resize(_renderStructures.size());
  _instancePositionBuffer.resize(_renderStructures.size());
  _scaleBuffer.resize(_renderStructures.size());

  _vertexArrayObject.resize(_renderStructures.size());
  _ambientColorBuffer.resize(_renderStructures.size());
  _diffuseColorBuffer.resize(_renderStructures.size());
  _specularColorBuffer.resize(_renderStructures.size());

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _numberOfDrawnAtoms[i].resize(_renderStructures[i].size());
    _numberOfIndices[i].resize(_renderStructures[i].size());

    _vertexBuffer[i].resize(_renderStructures[i].size());
    _indexBuffer[i].resize(_renderStructures[i].size());
    _instancePositionBuffer[i].resize(_renderStructures[i].size());
    _scaleBuffer[i].resize(_renderStructures[i].size());

    _vertexArrayObject[i].resize(_renderStructures[i].size());
    _ambientColorBuffer[i].resize(_renderStructures[i].size());
    _diffuseColorBuffer[i].resize(_renderStructures[i].size());
    _specularColorBuffer[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _instancePositionBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _scaleBuffer[i].data());

    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _ambientColorBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _diffuseColorBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _specularColorBuffer[i].data());
  }
}

void OpenGLAtomSelectionWorleyNoise3DShader::paintGL(GLuint structureUniformBuffer)
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
      if(_renderStructures[i][j]->atomSelectionStyle() == RKSelectionStyle::WorleyNoise3D && _renderStructures[i][j]->drawAtoms() && _renderStructures[i][j]->isVisible() && _numberOfIndices[i][j]>0 && _numberOfDrawnAtoms[i][j]>0)
      {
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));

        glBindVertexArray(_vertexArrayObject[i][j]);
        check_gl_error();

        glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfDrawnAtoms[i][j]));
        check_gl_error();
        glBindVertexArray(0);
      }
      index++;
    }
  }
  glUseProgram(0);

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void OpenGLAtomSelectionWorleyNoise3DShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLAtomSelectionWorleyNoise3DShader::initializeVertexArrayObject()
{
  SphereGeometry sphere = SphereGeometry(1.0,41);

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      std::vector<RKInPerInstanceAttributesAtoms> atomData = _renderStructures[i][j]->renderSelectedAtoms();
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

      glVertexAttribPointer(_ambientColorAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,ambient)));
      glVertexAttribDivisor(_ambientColorAttributeLocation, 1);

      glVertexAttribPointer(_diffuseColorAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,diffuse)));
      glVertexAttribDivisor(_diffuseColorAttributeLocation, 1);

      glVertexAttribPointer(_specularColorAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,specular)));
      glVertexAttribDivisor(_specularColorAttributeLocation, 1);

      glVertexAttribPointer(_scaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,scale)));
      glVertexAttribDivisor(_scaleAttributeLocation, 1);


      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionAttributeLocation);
      glEnableVertexAttribArray(_scaleAttributeLocation);
      glEnableVertexAttribArray(_ambientColorAttributeLocation);
      glEnableVertexAttribArray(_diffuseColorAttributeLocation);
      glEnableVertexAttribArray(_specularColorAttributeLocation);



      glBindBuffer(GLenum(GL_ARRAY_BUFFER), 0);
      glBindVertexArray(0);
    }
  }
}

void OpenGLAtomSelectionWorleyNoise3DShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLAtomSelectionWorleyNoise3DShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLAtomSelectionWorleyNoise3DShader::_fragmentShaderSource.c_str());

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
    _ambientColorAttributeLocation = glGetAttribLocation(_program, "instanceAmbientColor");
    _diffuseColorAttributeLocation = glGetAttribLocation(_program, "instanceDiffuseColor");
    _specularColorAttributeLocation = glGetAttribLocation(_program, "instanceSpecularColor");
    _scaleAttributeLocation = glGetAttribLocation(_program, "instanceScale");

    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_vertexNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_instancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition' attribute.";
    if (_ambientColorAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceAmbientColor' attribute.";
    if (_diffuseColorAttributeLocation  < 0) qDebug() << "Shader did not contain the 'instanceDiffuseColor' attribute.";
    if (_specularColorAttributeLocation  < 0) qDebug() << "Shader did not contain the 'instanceSpecularColor' attribute.";
    if (_scaleAttributeLocation  < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string  OpenGLAtomSelectionWorleyNoise3DShader::_vertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
R"foo(
// Inputs from vertex shader
out VS_OUT
{
  flat vec4 ambient;
  flat vec4 diffuse;
  flat vec4 specular;
  smooth vec3 N;
  smooth vec3 ModelN;
  smooth vec3 L;
  smooth vec3 V;
} vs_out;

in vec4 vertexPosition;
in vec4 vertexNormal;

in vec4 instancePosition;
in vec4 instanceScale;
in vec4 instanceAmbientColor;
in vec4 instanceDiffuseColor;
in vec4 instanceSpecularColor;

void main(void)
{
  vs_out.ambient=lightUniforms.lights[0].ambient * structureUniforms.atomAmbientColor * instanceAmbientColor;
  vs_out.diffuse=lightUniforms.lights[0].diffuse * structureUniforms.atomDiffuseColor * instanceDiffuseColor;
  vs_out.specular=lightUniforms.lights[0].specular * structureUniforms.atomSpecularColor * instanceSpecularColor;
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

const std::string  OpenGLAtomSelectionWorleyNoise3DShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
OpenGLRGBHSVStringLiteral+
OpenGLWorleyNoise3DStringLiteral +
R"foo(

// Inputs from vertex shader
// Input from vertex shader
in VS_OUT
{
  flat vec4 ambient;
  flat vec4 diffuse;
  flat vec4 specular;
  smooth vec3 N;
  smooth vec3 ModelN;
  smooth vec3 L;
  smooth vec3 V;
} fs_in;

vec2 textureCoordinateForSphereSurfacePosition(vec3 sphereSurfacePosition)
{
  vec3 absoluteSphereSurfacePosition = abs(sphereSurfacePosition);
  float d = absoluteSphereSurfacePosition.x + absoluteSphereSurfacePosition.y + absoluteSphereSurfacePosition.z;

  return (sphereSurfacePosition.z > 0.0) ? sphereSurfacePosition.xy / d : sign(sphereSurfacePosition.xy) * ( 1.0 - absoluteSphereSurfacePosition.yx / d);
}

out vec4 vFragColor;

void main(void)
{

  // Normalize the incoming N, L and V vectors
  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L);
  vec3 V = normalize(fs_in.V);

  // Calculate R locally
  vec3 R = reflect(-L, N);

  // Compute the diffuse and specular components for each fragment
  vec4 ambient = fs_in.ambient;
  vec4 diffuse = max(dot(N, L), 0.0) * fs_in.diffuse;
  vec4 specular = pow(max(dot(R, V), 0.0),  lightUniforms.lights[0].shininess + structureUniforms.atomShininess) * fs_in.specular;

  vec3 t1 = fs_in.ModelN.xyz;

  float frequency = structureUniforms.atomSelectionWorleyNoise3DFrequency;
  float jitter = structureUniforms.atomSelectionWorleyNoise3DJitter;
  vec2 F = cellular3D(frequency*t1, jitter);
  float n = F.y-F.x;

  vec4 color = n * (ambient + diffuse + specular);

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
