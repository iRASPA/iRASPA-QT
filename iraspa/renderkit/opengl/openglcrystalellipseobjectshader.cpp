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

#include "openglcrystalellipseobjectshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"
#include "opengluniformstringliterals.h"

OpenGLCrystalEllipseObjectShader::OpenGLCrystalEllipseObjectShader()
{
}


void OpenGLCrystalEllipseObjectShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLCrystalEllipseObjectShader::paintGLOpaque(GLuint structureUniformBuffer)
{
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glUseProgram(_program);
  check_gl_error();

  size_t index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderPrimitiveObjectsSource* source = dynamic_cast<RKRenderPrimitiveObjectsSource*>(_renderStructures[i][j].get()))
      {
        if (RKRenderCrystalPrimitiveEllipsoidObjectsSource* object = dynamic_cast<RKRenderCrystalPrimitiveEllipsoidObjectsSource*>(_renderStructures[i][j].get()))
        {
          if(source->primitiveOpacity()>0.99999 && source->drawAtoms() && _renderStructures[i][j]->isVisible() && _numberOfIndices[i][j]>0 && _numberOfDrawnAtoms[i][j]>0)
          {
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), sizeof(RKStructureUniforms));

            glBindVertexArray(_vertexArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfDrawnAtoms[i][j]));
            check_gl_error();
            glBindVertexArray(0);
          }
        }
      }
      index++;
    }
  }
  glUseProgram(0);
}


void OpenGLCrystalEllipseObjectShader::paintGLTransparent(GLuint structureUniformBuffer)
{
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  glUseProgram(_program);
  check_gl_error();

  size_t index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderPrimitiveObjectsSource* source = dynamic_cast<RKRenderPrimitiveObjectsSource*>(_renderStructures[i][j].get()))
      {
        if (RKRenderCrystalPrimitiveEllipsoidObjectsSource* object = dynamic_cast<RKRenderCrystalPrimitiveEllipsoidObjectsSource*>(_renderStructures[i][j].get()))
        {
          if(source->primitiveOpacity()<=0.99999 && source->drawAtoms() && _renderStructures[i][j]->isVisible() && _numberOfIndices[i][j]>0 && _numberOfDrawnAtoms[i][j]>0)
          {
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), sizeof(RKStructureUniforms));

            glBindVertexArray(_vertexArrayObject[i][j]);
            check_gl_error();

            glCullFace(GL_FRONT);
            glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfDrawnAtoms[i][j]));
            check_gl_error();

            glCullFace(GL_BACK);
            glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfDrawnAtoms[i][j]));
            check_gl_error();

            glBindVertexArray(0);
          }
        }
      }
      index++;
    }
  }
  glUseProgram(0);

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}


void OpenGLCrystalEllipseObjectShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glDeleteBuffers(size, _vertexBuffer[i].data());
    glDeleteBuffers(size, _indexBuffer[i].data());
    glDeleteBuffers(size, _instancePositionBuffer[i].data());

    glDeleteVertexArrays(size, _vertexArrayObject[i].data());
    glDeleteBuffers(size, _ambientColorBuffer[i].data());
    glDeleteBuffers(size, _diffuseColorBuffer[i].data());
    glDeleteBuffers(size, _specularColorBuffer[i].data());
  }
}

void OpenGLCrystalEllipseObjectShader::generateBuffers()
{
  _numberOfDrawnAtoms.resize(_renderStructures.size());
  _numberOfIndices.resize(_renderStructures.size());

  _vertexBuffer.resize(_renderStructures.size());
  _indexBuffer.resize(_renderStructures.size());
  _instancePositionBuffer.resize(_renderStructures.size());

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

    _vertexArrayObject[i].resize(_renderStructures[i].size());
    _ambientColorBuffer[i].resize(_renderStructures[i].size());
    _diffuseColorBuffer[i].resize(_renderStructures[i].size());
    _specularColorBuffer[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glGenBuffers(size, _vertexBuffer[i].data());
    glGenBuffers(size, _indexBuffer[i].data());
    glGenBuffers(size, _instancePositionBuffer[i].data());

    glGenVertexArrays(size, _vertexArrayObject[i].data());
    glGenBuffers(size, _ambientColorBuffer[i].data());
    glGenBuffers(size, _diffuseColorBuffer[i].data());
    glGenBuffers(size, _specularColorBuffer[i].data());
  }
}

void OpenGLCrystalEllipseObjectShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLCrystalEllipseObjectShader::initializeVertexArrayObject()
{
  SphereGeometry sphere = SphereGeometry(1.0,41);

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderCrystalPrimitiveEllipsoidObjectsSource* object = dynamic_cast<RKRenderCrystalPrimitiveEllipsoidObjectsSource*>(_renderStructures[i][j].get()))
      {
        glBindVertexArray(_vertexArrayObject[i][j]);
        check_gl_error();

        std::vector<RKInPerInstanceAttributesAtoms> atomData = object->renderCrystalPrimitiveEllipsoidObjects();
        _numberOfDrawnAtoms[i][j] = atomData.size();
        _numberOfIndices[i][j] = sphere.indices().size();

        glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer[i][j]);
        check_gl_error();

        if(sphere.vertices().size()>0)
        {
          glBufferData(GL_ARRAY_BUFFER, sphere.vertices().size() * sizeof(RKVertex), sphere.vertices().data(), GL_DYNAMIC_DRAW);
        }
        check_gl_error();

        glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
        check_gl_error();
        glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
        check_gl_error();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer[i][j]);
        check_gl_error();
        if(sphere.indices().size()>0)
        {
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices().size() * sizeof(GLshort), sphere.indices().data(), GL_DYNAMIC_DRAW);
        }
        check_gl_error();

        glBindBuffer(GL_ARRAY_BUFFER, _instancePositionBuffer[i][j]);
        check_gl_error();
        if(_numberOfDrawnAtoms[i][j]>0)
        {
          glBufferData(GL_ARRAY_BUFFER, _numberOfDrawnAtoms[i][j]*sizeof(RKInPerInstanceAttributesAtoms), atomData.data(), GL_DYNAMIC_DRAW);
        }
        check_gl_error();

        glVertexAttribPointer(_instancePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,position));
        check_gl_error();
        glVertexAttribDivisor(_instancePositionAttributeLocation, 1);
        check_gl_error();

        glEnableVertexAttribArray(_vertexPositionAttributeLocation);
        glEnableVertexAttribArray(_vertexNormalAttributeLocation);
        glEnableVertexAttribArray(_instancePositionAttributeLocation);
        check_gl_error();

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
      }
    }
  }
}

void OpenGLCrystalEllipseObjectShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER, OpenGLCrystalEllipseObjectShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER, OpenGLCrystalEllipseObjectShader::_fragmentShaderSource.c_str());

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

    _ambientOcclusionTextureUniformLocation   = glGetUniformLocation(_program, "ambientOcclusionTexture");
    _vertexNormalAttributeLocation   = glGetAttribLocation(_program, "vertexNormal");
    _vertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");
    _instancePositionAttributeLocation = glGetAttribLocation(_program, "instancePosition");

    if (_vertexNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string OpenGLCrystalEllipseObjectShader::_vertexShaderSource  =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
std::string(R"foo(
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
  vec4 pos = instancePosition + structureUniforms.transformationMatrix * vertexPosition;

  // Calculate normal in modelview-space
  vs_out.ModelN = vertexNormal.xyz;
  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * structureUniforms.transformationNormalMatrix * vertexNormal).xyz;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  // Calculate view vector
  vs_out.V = -P.xyz;

  int iid = gl_InstanceID;
  int patchNumber=structureUniforms.ambientOcclusionPatchNumber;

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
}
)foo");


const std::string OpenGLCrystalEllipseObjectShader::_fragmentShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLRGBHSVStringLiteral +
std::string(R"foo(
// Input from vertex shader
in VS_OUT
{
  smooth vec3 N;
  smooth vec3 ModelN;
  smooth vec3 L;
  smooth vec3 V;
} fs_in;

out  vec4 vFragColor;

void main(void)
{

  // Normalize the incoming N, L and V vectors
  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L);
  vec3 V = normalize(fs_in.V);

  // Calculate R locally
  vec3 R = reflect(-L, N);

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 color;
  if (gl_FrontFacing)
  {
    ambient = structureUniforms.primitiveAmbientFrontSide;
    diffuse = max(dot(N, L), 0.0) * structureUniforms.primitiveDiffuseFrontSide;
    specular = pow(max(dot(R, V), 0.0), structureUniforms.primitiveShininessFrontSide) * structureUniforms.primitiveSpecularFrontSide;

    color= vec4((ambient.xyz + diffuse.xyz + specular.xyz), 1.0);
    if (structureUniforms.primitiveFrontSideHDR)
    {
      vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.primitiveFrontSideHDRExposure);
      vLdrColor.a = 1.0;
      color = vLdrColor;
    }
  }
  else
  {
    ambient = structureUniforms.primitiveAmbientBackSide;
    diffuse = max(dot(-N, L), 0.0) * structureUniforms.primitiveDiffuseBackSide;
    specular = pow(max(dot(R, V), 0.0), structureUniforms.primitiveShininessBackSide) * structureUniforms.primitiveSpecularBackSide;

    color = vec4((ambient.xyz + diffuse.xyz + specular.xyz), 1.0);
    if (structureUniforms.primitiveBackSideHDR)
    {
      vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.primitiveBackSideHDRExposure);
      vLdrColor.a = 1.0;
      color = vLdrColor;
    }
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.primitiveHue;
  hsv.y = hsv.y * structureUniforms.primitiveSaturation;
  hsv.z = hsv.z * structureUniforms.primitiveValue;
  vFragColor = structureUniforms.primitiveOpacity * vec4(hsv2rgb(hsv),1.0);
}
)foo");
