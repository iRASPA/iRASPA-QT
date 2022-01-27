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

#include "openglenergysurface.h"
#include "glgeterror.h"
#include "rkrenderuniforms.h"
#include <iostream>
#include <ctime>
#include "skcomputeenergygrid.h"
#include "skcomputeisosurface.h"
#include <exception>
#include <stdexcept>
#include "opengluniformstringliterals.h"

OpenGLEnergySurface::OpenGLEnergySurface()
{

}

OpenGLEnergySurface::~OpenGLEnergySurface()
{
  for(QCache<RKRenderObject*, std::vector<cl_float>> & cache: _caches)
  {
    cache.clear();
  }
}

void OpenGLEnergySurface::invalidateIsosurface(std::vector<std::shared_ptr<RKRenderObject>> structures)
{
  for(const std::shared_ptr<RKRenderObject> &structure : structures)
  {
    for(QCache<RKRenderObject*, std::vector<cl_float>> & cache: _caches)
    {
      cache.remove(structure.get());
    }
  }
}

void OpenGLEnergySurface::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}


void OpenGLEnergySurface::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _surfaceVertexArrayObject[i].data());
    check_gl_error();
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _surfaceVertexBuffer[i].data());
    check_gl_error();
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _surfaceIndexBuffer[i].data());
    check_gl_error();
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _surfaceInstancePositionBuffer[i].data());
    check_gl_error();
  }
}

void OpenGLEnergySurface::generateBuffers()
{
  _surfaceNumberOfInstances.resize(_renderStructures.size());
  _surfaceNumberOfIndices.resize(_renderStructures.size());

  _surfaceVertexArrayObject.resize(_renderStructures.size());
  _surfaceVertexBuffer.resize(_renderStructures.size());
  _surfaceIndexBuffer.resize(_renderStructures.size());
  _surfaceInstancePositionBuffer.resize(_renderStructures.size());

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _surfaceNumberOfInstances[i].resize(_renderStructures[i].size());
    _surfaceNumberOfIndices[i].resize(_renderStructures[i].size());

    _surfaceVertexArrayObject[i].resize(_renderStructures[i].size());
    _surfaceVertexBuffer[i].resize(_renderStructures[i].size());
    _surfaceIndexBuffer[i].resize(_renderStructures[i].size());
    _surfaceInstancePositionBuffer[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _surfaceVertexArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _surfaceVertexBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _surfaceIndexBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _surfaceInstancePositionBuffer[i].data());
  }
  check_gl_error();
}

void OpenGLEnergySurface::reloadData()
{
  initializeVertexArrayObject();
  check_gl_error();
}



void OpenGLEnergySurface::paintGLOpaque(GLuint structureUniformBuffer, GLuint isosurfaceUniformBuffer)
{
  glDisable(GL_CULL_FACE);
  size_t index=0;
  glUseProgram(_program);
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
        if (RKRenderObject* renderStructure = dynamic_cast<RKRenderObject*>(_renderStructures[i][j].get()))
      if (RKRenderVolumetricDataSource* source = dynamic_cast<RKRenderVolumetricDataSource*>(_renderStructures[i][j].get()))
      {
        if (renderStructure->isVisible() && source->drawAdsorptionSurface() && source->adsorptionSurfaceOpacity()>0.99999 && _surfaceNumberOfIndices[i][j] > 0 && _surfaceNumberOfInstances[i][j]>0)
        {
          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, static_cast<GLintptr>(index * sizeof(RKStructureUniforms)), sizeof(RKStructureUniforms));
          glBindBufferRange(GL_UNIFORM_BUFFER, 2, isosurfaceUniformBuffer, static_cast<GLintptr>(index * sizeof(RKIsosurfaceUniforms)), sizeof(RKIsosurfaceUniforms));

          glBindVertexArray(_surfaceVertexArrayObject[i][j]);

          glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(3 * _surfaceNumberOfIndices[i][j]), static_cast<GLsizei>(_surfaceNumberOfInstances[i][j]));

          glBindVertexArray(0);
        }
      }
      index+=1;
    }
  }
  glUseProgram(0);
  glEnable(GL_CULL_FACE);
}

void OpenGLEnergySurface::paintGLTransparent(GLuint structureUniformBuffer, GLuint isosurfaceUniformBuffer)
{
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  size_t index=0;
  glUseProgram(_program);
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderVolumetricDataSource* source = dynamic_cast<RKRenderVolumetricDataSource*>(_renderStructures[i][j].get()))
      {
        if (_renderStructures[i][j]->isVisible() && source->drawAdsorptionSurface() && source->adsorptionSurfaceRenderingMethod() == RKEnergySurfaceType::isoSurface &&
            source->adsorptionSurfaceOpacity()<=0.99999 && _surfaceNumberOfIndices[i][j] > 0 )
        {
          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, static_cast<GLintptr>(index * sizeof(RKStructureUniforms)), sizeof(RKStructureUniforms));
          glBindBufferRange(GL_UNIFORM_BUFFER, 2, isosurfaceUniformBuffer, static_cast<GLintptr>(index * sizeof(RKIsosurfaceUniforms)), sizeof(RKIsosurfaceUniforms));

          glBindVertexArray(_surfaceVertexArrayObject[i][j]);

          glCullFace(GL_FRONT);
          glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(3 * _surfaceNumberOfIndices[i][j]), static_cast<GLsizei>(_surfaceNumberOfInstances[i][j]));

          glCullFace(GL_BACK);
          glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(3 * _surfaceNumberOfIndices[i][j]), static_cast<GLsizei>(_surfaceNumberOfInstances[i][j]));

          glBindVertexArray(0);
        }
      }
      index++;
    }
  }
  glUseProgram(0);
  glEnable(GL_CULL_FACE);

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void OpenGLEnergySurface::initializeVertexArrayObject()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderObject* renderStructure = dynamic_cast<RKRenderObject*>(_renderStructures[i][j].get()))
      {
        if (RKRenderVolumetricDataSource* source = dynamic_cast<RKRenderVolumetricDataSource*>(_renderStructures[i][j].get()))
        {
          if (source->drawAdsorptionSurface() && source->adsorptionSurfaceRenderingMethod() == RKEnergySurfaceType::isoSurface )
          {
            glBindVertexArray(_surfaceVertexArrayObject[i][j]);
            check_gl_error();
            glBindBuffer(GL_ARRAY_BUFFER, _surfaceVertexBuffer[i][j]);
            check_gl_error();

            glVertexAttribPointer(_atomSurfaceVertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid *>(offsetof(RKVertex,position)));
            glVertexAttribPointer(_atomSurfaceVertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid *>(offsetof(RKVertex,normal)));

            double isoValue = source->adsorptionSurfaceIsoValue();
            int3 dimensions = source->dimensions();
            int largestSize = std::max({dimensions.x,dimensions.y,dimensions.z});
            int powerOfTwo = 1;
            while(largestSize > pow(2,powerOfTwo))
            {
              powerOfTwo += 1;
            }

            _surfaceNumberOfIndices[i][j] = 0;
            std::vector<cl_float> *energyGridPointer = nullptr;
            if(_caches[powerOfTwo].contains(_renderStructures[i][j].get()))
            {
              energyGridPointer = _caches[powerOfTwo].object(_renderStructures[i][j].get());

              try
              {
                std::vector<float4> triangleData = SKComputeIsosurface::computeIsosurface(dimensions, energyGridPointer, isoValue);
                _surfaceNumberOfIndices[i][j] = triangleData.size()/(3*3);

                if(triangleData.size()>0)
                {
                  glBufferData(GL_ARRAY_BUFFER, triangleData.size()*sizeof(float4), triangleData.data(), GL_DYNAMIC_DRAW);
                  check_gl_error();
                }
              }
              catch (char const* e)
              {
                std::cout << "Exception caught: " << e << std::endl;
                _surfaceNumberOfIndices[i][j] = 0;
                return;
              }
            }
            else
            {
              std::vector<cl_float> gridData = source->gridData();
              if (gridData.empty()) return;

              // move from stack to heap since the cache requires a pointer to the std::vector
              energyGridPointer = new std::vector<cl_float>();
              *energyGridPointer = std::move(gridData);

              try
              {
                std::vector<float4> triangleData = SKComputeIsosurface::computeIsosurface(dimensions, energyGridPointer, isoValue);
                _surfaceNumberOfIndices[i][j] = triangleData.size()/(3*3);

                if(triangleData.size()>0)
                {
                  glBufferData(GL_ARRAY_BUFFER, triangleData.size()*sizeof(float4), triangleData.data(), GL_DYNAMIC_DRAW);
                  check_gl_error();
                }
              }
              catch (char const* e)
              {
                std::cout << "Exception caught: " << e << std::endl;
                _surfaceNumberOfIndices[i][j] = 0;
                return;
              }

              // insert as last to avoid use of memory after free in case the insertion fails.
              _caches[powerOfTwo].insert(_renderStructures[i][j].get(), energyGridPointer);
            }


            std::vector<float4> renderLatticeVectors = renderStructure->cell()->renderTranslationVectors();
            _surfaceNumberOfInstances[i][j] = renderLatticeVectors.size();
            glBindBuffer(GL_ARRAY_BUFFER,_surfaceInstancePositionBuffer[i][j]);
            if(renderLatticeVectors.size()>0)
            {
              glBufferData(GL_ARRAY_BUFFER, renderLatticeVectors.size()*sizeof(float4), renderLatticeVectors.data(), GL_DYNAMIC_DRAW);
              check_gl_error();
            }

            glVertexAttribPointer(_atomSurfaceInstancePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(float4), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,position));
            glVertexAttribDivisor(_atomSurfaceInstancePositionAttributeLocation, 1);

            glEnableVertexAttribArray(_atomSurfaceVertexPositionAttributeLocation);
            glEnableVertexAttribArray(_atomSurfaceVertexNormalAttributeLocation);
            glEnableVertexAttribArray(_atomSurfaceInstancePositionAttributeLocation);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            check_gl_error();
          }
        }
      }
    }
  }
}



void OpenGLEnergySurface::initializeTransformUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
  check_gl_error();
}

void OpenGLEnergySurface::initializeStructureUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "StructureUniformBlock"), 1);
  check_gl_error();
}

void OpenGLEnergySurface::initializeIsosurfaceUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "IsosurfaceUniformBlock"), 2);
  check_gl_error();
}

void OpenGLEnergySurface::initializeLightUniforms()
{
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "LightsUniformBlock"), 3);
  check_gl_error();
}

void OpenGLEnergySurface::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLEnergySurface::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLEnergySurface::_fragmentShaderSource.c_str());

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

    _atomSurfaceVertexNormalAttributeLocation   = glGetAttribLocation(_program, "vertexNormal");
    _atomSurfaceVertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");
    _atomSurfaceInstancePositionAttributeLocation = glGetAttribLocation(_program, "instancePosition");

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string  OpenGLEnergySurface::_vertexShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLIsosurfaceUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;
in vec4 vertexNormal;
in vec4 instancePosition;

// Inputs from vertex shader
out VS_OUT
{
  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
} vs_out;

void main(void)
{
  vec4 pos = isosurfaceUniforms.unitCellMatrix * (vertexPosition+instancePosition);

  // Calculate normal in modelview-space
  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * isosurfaceUniforms.unitCellNormalMatrix * vertexNormal).xyz;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  // Calculate view vector
  vs_out.V = -P.xyz;

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
}
)foo";


const std::string  OpenGLEnergySurface::_fragmentShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLIsosurfaceUniformBlockStringLiteral +
R"foo(
// Input from vertex shader
in VS_OUT
{
  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
} fs_in;

out  vec4 vFragColor;

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
  vec3 V = normalize(fs_in.V);

  vec4 ambient;
  vec4 specular;
  vec4 diffuse;
  vec4 color;

  if (gl_FrontFacing)
  {
    // Calculate R locally
    vec3 R = reflect(-L, N);

    // Compute the diffuse and specular components for each fragment
    ambient = isosurfaceUniforms.ambientFrontSide;
    diffuse = max(dot(N, L), 0.0) * isosurfaceUniforms.diffuseFrontSide;
    specular = pow(max(dot(R, V), 0.0), isosurfaceUniforms.shininessFrontSide) * isosurfaceUniforms.specularFrontSide;
    color = vec4(ambient.xyz + diffuse.xyz + specular.xyz, 1.0);
    if (isosurfaceUniforms.frontHDR)
    {
      vec4 vLdrColor = 1.0 - exp2(-color * isosurfaceUniforms.frontHDRExposure);
      vLdrColor.a = 1.0;
      color = vLdrColor;
    }
  }
  else
  {
    // Calculate R locally
    vec3 R = reflect(-L, -N);

    // Compute the diffuse and specular components for each fragment
    ambient = isosurfaceUniforms.ambientBackSide;
    diffuse = max(dot(-N, L), 0.0) * isosurfaceUniforms.diffuseBackSide;
    specular = pow(max(dot(R, V), 0.0), isosurfaceUniforms.shininessBackSide) * isosurfaceUniforms.specularBackSide;
    color = vec4(ambient.xyz + diffuse.xyz + specular.xyz, 1.0);
    if (isosurfaceUniforms.backHDR)
    {
      vec4 vLdrColor = 1.0 - exp2(-color * isosurfaceUniforms.backHDRExposure);
      vLdrColor.a = 1.0;
      color = vLdrColor;
    }
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * isosurfaceUniforms.hue;
  hsv.y = hsv.y * isosurfaceUniforms.saturation;
  hsv.z = hsv.z * isosurfaceUniforms.value;
  vFragColor = vec4(hsv2rgb(hsv) * isosurfaceUniforms.diffuseFrontSide.w,isosurfaceUniforms.diffuseFrontSide.w);
}
)foo";
