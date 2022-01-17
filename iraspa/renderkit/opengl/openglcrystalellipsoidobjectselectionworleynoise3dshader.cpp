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

#include "openglcrystalellipsoidobjectselectionworleynoise3dshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "cylindergeometry.h"
#include "cappedcylindergeometry.h"
#include "uncappedcylindergeometry.h"

OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader(OpenGLCrystalEllipsoidSelectionInstanceShader &instanceShader, OpenGLCrystalEllipseObjectShader &crystalEllipsoidShader):
     _instanceShader(instanceShader), _crystalEllipsoidShader(crystalEllipsoidShader)
{
}


void OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::paintGL(GLuint structureUniformBuffer)
{
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(_program);
  check_gl_error();

  size_t index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderPrimitiveObjectsSource* object = dynamic_cast<RKRenderPrimitiveObjectsSource*>(_renderStructures[i][j].get()))
      {
        if (RKRenderCrystalPrimitiveEllipsoidObjectsSource* source = dynamic_cast<RKRenderCrystalPrimitiveEllipsoidObjectsSource*>(_renderStructures[i][j].get()))
        {
          if(object->primitiveSelectionStyle() == RKSelectionStyle::WorleyNoise3D)
          {
            if(_renderStructures[i][j]->isVisible() && _crystalEllipsoidShader._numberOfIndices[i][j]>0 && _instanceShader._numberOfDrawnAtoms[i][j]>0)
            {
              glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), sizeof(RKStructureUniforms));

              glBindVertexArray(_vertexArrayObject[i][j]);
              check_gl_error();

              // draw only the front-faces for the Worley-noise-3D style (looks better)
              glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_crystalEllipsoidShader._numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_instanceShader._numberOfDrawnAtoms[i][j]));
              check_gl_error();

              glBindVertexArray(0);
            }
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




void OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glDeleteVertexArrays(size, _vertexArrayObject[i].data());
  }
}

void OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::generateBuffers()
{
  _vertexArrayObject.resize(_renderStructures.size());

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _vertexArrayObject[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glGenVertexArrays(size, _vertexArrayObject[i].data());
  }
}

void OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::initializeVertexArrayObject()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderCrystalPrimitiveEllipsoidObjectsSource* object = dynamic_cast<RKRenderCrystalPrimitiveEllipsoidObjectsSource*>(_renderStructures[i][j].get()))
      {
        glBindVertexArray(_vertexArrayObject[i][j]);
        check_gl_error();

        glBindBuffer(GL_ARRAY_BUFFER, _crystalEllipsoidShader._vertexBuffer[i][j]);
        check_gl_error();

        glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
        check_gl_error();
        glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
        check_gl_error();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _crystalEllipsoidShader._indexBuffer[i][j]);


        glBindBuffer(GL_ARRAY_BUFFER, _instanceShader._instancePositionBuffer[i][j]);


        glVertexAttribPointer(_instancePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,position));
        check_gl_error();
        glVertexAttribDivisor(_instancePositionAttributeLocation, 1);
        check_gl_error();

        glVertexAttribPointer(_instanceScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,scale));
        check_gl_error();
        glVertexAttribDivisor(_instanceScaleAttributeLocation, 1);
        check_gl_error();

        glEnableVertexAttribArray(_vertexPositionAttributeLocation);
        glEnableVertexAttribArray(_vertexNormalAttributeLocation);
        glEnableVertexAttribArray(_instancePositionAttributeLocation);
        glEnableVertexAttribArray(_instanceScaleAttributeLocation);
        check_gl_error();

        glBindVertexArray(0);
      }
    }
  }
}

void OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER, OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER, OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::_fragmentShaderSource.c_str());

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

    _vertexNormalAttributeLocation   = glGetAttribLocation(_program, "vertexNormal");
    _vertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");
    _instancePositionAttributeLocation = glGetAttribLocation(_program, "instancePosition");
    _instanceScaleAttributeLocation = glGetAttribLocation(_program, "instanceScale");

    if (_vertexNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition' attribute.";
    if (_instanceScaleAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::_vertexShaderSource  =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
std::string(R"foo(
in vec4 vertexPosition;
in vec4 vertexNormal;

in vec4 instancePosition;
in vec4 instanceScale;


// Inputs from vertex shader
out VS_OUT
{
  smooth vec3 N;
  smooth vec3 Model_N;
  smooth vec3 L;
  smooth vec3 V;
} vs_out;

void main(void)
{
  vec4 scale = structureUniforms.primitiveSelectionScaling * instanceScale;
  vec4 pos =  structureUniforms.transformationMatrix * (scale * vertexPosition) + instancePosition;

  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * structureUniforms.transformationNormalMatrix * vertexNormal).xyz;
  vs_out.Model_N = vertexPosition.xyz;

  vec4 P =  frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  // Calculate view vector
  vs_out.V = -P.xyz;

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
}
)foo");


const std::string OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
OpenGLRGBHSVStringLiteral +
OpenGLWorleyNoise3DStringLiteral +
std::string(R"foo(
// Input from vertex shader
in VS_OUT
{
  smooth vec3 N;
  smooth vec3 Model_N;
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

  vec4  ambient = structureUniforms.primitiveAmbientFrontSide;
  vec4  diffuse = max(dot(N, L), 0.0) * structureUniforms.primitiveDiffuseFrontSide;
  vec4  specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.primitiveShininessFrontSide) * structureUniforms.primitiveSpecularFrontSide;

  vec3 t1 = fs_in.Model_N;
  float frequency = structureUniforms.primitiveSelectionWorleyNoise3DFrequency;
  float jitter = structureUniforms.primitiveSelectionWorleyNoise3DJitter;
  vec2 F = cellular3D(frequency*vec3(t1.x,t1.z,t1.y), jitter);
  float n = F.y-F.x;


  vec4  color = n * vec4((ambient.xyz + diffuse.xyz + specular.xyz), 1.0);
  if (structureUniforms.primitiveFrontSideHDR)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.primitiveFrontSideHDRExposure);
    vLdrColor.a = 1.0;
    color = vLdrColor;
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.primitiveHue;
  hsv.y = hsv.y * structureUniforms.primitiveSaturation;
  hsv.z = hsv.z * structureUniforms.primitiveValue;
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.primitiveSelectionIntensity;
  vFragColor = bloomLevel * vec4(hsv2rgb(hsv),1.0);
}
)foo");
