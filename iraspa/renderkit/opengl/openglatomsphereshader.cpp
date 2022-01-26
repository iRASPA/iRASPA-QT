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

#include "openglatomsphereshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"
#include "opengluniformstringliterals.h"

OpenGLAtomSphereShader::OpenGLAtomSphereShader()
{
}

void OpenGLAtomSphereShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLAtomSphereShader::paintGL(std::vector<std::vector<GLuint>>& atomTextures, GLuint structureUniformBuffer)
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
      if (RKRenderAtomSource* source = dynamic_cast<RKRenderAtomSource*>(_renderStructures[i][j].get()))
      {
        if(source->drawAtoms() && _renderStructures[i][j]->isVisible() && _numberOfIndices[i][j]>0 && _numberOfDrawnAtoms[i][j]>0)
        {
          if(source->clipAtomsAtUnitCell())
          {
            glEnable(GL_CLIP_DISTANCE0);
            glEnable(GL_CLIP_DISTANCE1);
            glEnable(GL_CLIP_DISTANCE2);
            glEnable(GL_CLIP_DISTANCE3);
            glEnable(GL_CLIP_DISTANCE4);
            glEnable(GL_CLIP_DISTANCE5);
          }
          else
          {
            glDisable(GL_CLIP_DISTANCE0);
            glDisable(GL_CLIP_DISTANCE1);
            glDisable(GL_CLIP_DISTANCE2);
            glDisable(GL_CLIP_DISTANCE3);
            glDisable(GL_CLIP_DISTANCE4);
            glDisable(GL_CLIP_DISTANCE5);
          }

          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), sizeof(RKStructureUniforms));


          glBindVertexArray(_vertexArrayObject[i][j]);
          check_gl_error();

          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, atomTextures[i][j]);
          glUniform1i(_ambientOcclusionTextureUniformLocation,0);

          glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfDrawnAtoms[i][j]));
          check_gl_error();
          glBindVertexArray(0);
        }
      }
      index++;
    }
  }
  glUseProgram(0);

  glDisable(GL_CLIP_DISTANCE0);
  glDisable(GL_CLIP_DISTANCE1);
  glDisable(GL_CLIP_DISTANCE2);
  glDisable(GL_CLIP_DISTANCE3);
  glDisable(GL_CLIP_DISTANCE4);
  glDisable(GL_CLIP_DISTANCE5);
}



void OpenGLAtomSphereShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glDeleteBuffers(size, _vertexBuffer[i].data());
    glDeleteBuffers(size, _indexBuffer[i].data());
    glDeleteBuffers(size, _instancePositionBuffer[i].data());
    glDeleteBuffers(size, _scaleBuffer[i].data());

    glDeleteVertexArrays(size, _vertexArrayObject[i].data());
    glDeleteBuffers(size, _ambientColorBuffer[i].data());
    glDeleteBuffers(size, _diffuseColorBuffer[i].data());
    glDeleteBuffers(size, _specularColorBuffer[i].data());
  }
}

void OpenGLAtomSphereShader::generateBuffers()
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
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glGenBuffers(size, _vertexBuffer[i].data());
    glGenBuffers(size, _indexBuffer[i].data());
    glGenBuffers(size, _instancePositionBuffer[i].data());
    glGenBuffers(size, _scaleBuffer[i].data());

    glGenVertexArrays(size, _vertexArrayObject[i].data());
    glGenBuffers(size, _ambientColorBuffer[i].data());
    glGenBuffers(size, _diffuseColorBuffer[i].data());
    glGenBuffers(size, _specularColorBuffer[i].data());
  }
}

void OpenGLAtomSphereShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLAtomSphereShader::initializeVertexArrayObject()
{
  SphereGeometry sphere = SphereGeometry(1.0,41);

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderAtomSource* source = dynamic_cast<RKRenderAtomSource*>(_renderStructures[i][j].get()))
      {
        glBindVertexArray(_vertexArrayObject[i][j]);
        check_gl_error();

        std::vector<RKInPerInstanceAttributesAtoms> atomData = source->renderAtoms();
        _numberOfDrawnAtoms[i][j] = atomData.size();
        _numberOfIndices[i][j] = sphere.indices().size();

        // each crystal can have a different number of sphere-slices
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

        glVertexAttribPointer(_ambientColorAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,ambient));
        check_gl_error();
        glVertexAttribDivisor(_ambientColorAttributeLocation, 1);
        check_gl_error();

        glVertexAttribPointer(_diffuseColorAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,diffuse));
        check_gl_error();
        glVertexAttribDivisor(_diffuseColorAttributeLocation, 1);
        check_gl_error();

        glVertexAttribPointer(_specularColorAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,specular));
        check_gl_error();
        glVertexAttribDivisor(_specularColorAttributeLocation, 1);
        check_gl_error();

        glVertexAttribPointer(_scaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), (GLvoid *)offsetof(RKInPerInstanceAttributesAtoms,scale));
        check_gl_error();
        glVertexAttribDivisor(_scaleAttributeLocation, 1);
        check_gl_error();

        glEnableVertexAttribArray(_vertexPositionAttributeLocation);
        glEnableVertexAttribArray(_vertexNormalAttributeLocation);
        glEnableVertexAttribArray(_instancePositionAttributeLocation);
        glEnableVertexAttribArray(_scaleAttributeLocation);
        glEnableVertexAttribArray(_ambientColorAttributeLocation);
        glEnableVertexAttribArray(_diffuseColorAttributeLocation);
        glEnableVertexAttribArray(_specularColorAttributeLocation);
        check_gl_error();

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
      }
    }
  }
}

void OpenGLAtomSphereShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLAtomSphereShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLAtomSphereShader::_fragmentShaderSource.c_str());

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
    _scaleAttributeLocation = glGetAttribLocation(_program, "instanceScale");
    _ambientColorAttributeLocation = glGetAttribLocation(_program, "instanceAmbientColor");
    _diffuseColorAttributeLocation = glGetAttribLocation(_program, "instanceDiffuseColor");
    _specularColorAttributeLocation = glGetAttribLocation(_program, "instanceSpecularColor");

    if (_ambientOcclusionTextureUniformLocation < 0) qDebug() << "Shader did not contain the 'ambientOcclusionTexture' attribute.";
    if (_vertexNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition' attribute.";
    if (_scaleAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";
    if (_ambientColorAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceAmbientColor' attribute.";
    if (_diffuseColorAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceAmbientColor' attribute.";
    if (_specularColorAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceSpecularColor' attribute.";


    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string OpenGLAtomSphereShader::_vertexShaderSource  =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
std::string(R"foo(
in vec4 vertexPosition;
in vec4 vertexNormal;

in vec4 instancePosition;
in vec4 instanceScale;
in vec4 instanceAmbientColor;
in vec4 instanceDiffuseColor;
in vec4 instanceSpecularColor;


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
    flat float k1;
    flat float k2;
} vs_out;

void main(void)
{
    if (structureUniforms.colorAtomsWithBondColor)
    {
      vs_out.ambient=lightUniforms.lights[0].ambient * structureUniforms.bondAmbientColor;
      vs_out.diffuse=lightUniforms.lights[0].diffuse * structureUniforms.bondDiffuseColor;
      vs_out.specular=lightUniforms.lights[0].specular * structureUniforms.bondSpecularColor;
    }
    else
    {
      vs_out.ambient=lightUniforms.lights[0].ambient * structureUniforms.atomAmbientColor * instanceAmbientColor;
      vs_out.diffuse=lightUniforms.lights[0].diffuse * structureUniforms.atomDiffuseColor * instanceDiffuseColor;
      vs_out.specular=lightUniforms.lights[0].specular * structureUniforms.atomSpecularColor * instanceSpecularColor;
    }

    vec4 scale = structureUniforms.atomScaleFactor * instanceScale;
    vec4 pos = instancePosition + scale*vertexPosition;

    // Calculate normal in modelview-space
    vs_out.ModelN = vertexNormal.xyz;
    vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vertexNormal).xyz;

    vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;

    // Calculate light vector
    vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

    // Calculate view vector
    vs_out.V = -P.xyz;

    int iid = gl_InstanceID;
    int patchNumber=structureUniforms.ambientOcclusionPatchNumber;
    vs_out.k1=iid%patchNumber;
    vs_out.k2=iid/patchNumber;

    gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;

    if(structureUniforms.clipAtomsAtUnitCell)
    {
      gl_ClipDistance[0] = dot(structureUniforms.clipPlaneBack,pos);
      gl_ClipDistance[1] = dot(structureUniforms.clipPlaneBottom,pos);
      gl_ClipDistance[2] = dot(structureUniforms.clipPlaneLeft,pos);

      gl_ClipDistance[3] = dot(structureUniforms.clipPlaneFront,pos);
      gl_ClipDistance[4] = dot(structureUniforms.clipPlaneTop,pos);
      gl_ClipDistance[5] = dot(structureUniforms.clipPlaneRight,pos);
    }
}
)foo");


const std::string OpenGLAtomSphereShader::_fragmentShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
std::string(R"foo(
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
  flat float k1;
  flat float k2;
} fs_in;

uniform sampler2D ambientOcclusionTexture;
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
  vec3 V = normalize(fs_in.V);

  // Calculate R locally
  vec3 R = reflect(-L, N);

  // Compute the diffuse and specular components for each fragment
  vec3 ambient = fs_in.ambient.xyz;
  vec3 diffuse = max(dot(N, L), 0.0) * fs_in.diffuse.xyz;
  vec3 specular = pow(max(dot(R, V), 0.0),  lightUniforms.lights[0].shininess + structureUniforms.atomShininess) * fs_in.specular.xyz;


  float ao = 1.0;
  if (structureUniforms.ambientOcclusion)
  {
    float patchSize=structureUniforms.ambientOcclusionPatchSize;
    vec3 t1 = fs_in.ModelN.xyz;
    vec2 m2 = (vec2(patchSize*(fs_in.k1+0.5),patchSize*(fs_in.k2+0.5))+0.5*(patchSize-1.0)*(textureCoordinateForSphereSurfacePosition(t1)))*structureUniforms.ambientOcclusionInverseTextureSize;

    ao = texture(ambientOcclusionTexture,m2).r;
  }

  vec4 color= vec4(ao * (ambient.xyz + diffuse.xyz + specular.xyz), 1.0);

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
  vFragColor = vec4(hsv2rgb(hsv),1.0);
}
)foo");
