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

#include "openglexternalbondshader.h"
#include "rkrenderuniforms.h"
#include <QDebug>
#include "glgeterror.h"
#include "skasymmetricbond.h"
#include <algorithm>
#include <type_traits>
#include "opengluniformstringliterals.h"
#include "openglbondshader.h"

OpenGLExternalBondShader::OpenGLExternalBondShader()
{

}

void OpenGLExternalBondShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}


void OpenGLExternalBondShader::paintGL(GLuint structureUniformBuffer)
{
  paintGLImposters(structureUniformBuffer);
}

void OpenGLExternalBondShader::paintGLImposters(GLuint structureUniformBuffer)
{
  glUseProgram(_programImposter);
  check_gl_error();

  // the view-aligned hull quads have no consistent winding
  glDisable(GL_CULL_FACE);

  int index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderBondSource* source = dynamic_cast<RKRenderBondSource*>(_renderStructures[i][j].get()))
      {
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));

        if (source->isUnity())
        {
          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _numberOfAllBonds[i][j]>0)
          {
            glUniform1i(_imposterBondTypeUniformLocation, 0);
            glBindVertexArray(_vertexAllBondsImposterArrayObject[i][j]);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 18, static_cast<GLsizei>(_numberOfAllBonds[i][j]));
            check_gl_error();
            glBindVertexArray(0);
          }
        }
        else
        {
          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _numberOfSingleBonds[i][j]>0)
          {
            glUniform1i(_imposterBondTypeUniformLocation, 0);
            glBindVertexArray(_vertexSingleBondsImposterArrayObject[i][j]);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 18, static_cast<GLsizei>(_numberOfSingleBonds[i][j]));
            check_gl_error();
            glBindVertexArray(0);
          }

          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _numberOfDoubleBonds[i][j]>0)
          {
            glUniform1i(_imposterBondTypeUniformLocation, 1);
            glBindVertexArray(_vertexDoubleBondsImposterArrayObject[i][j]);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 36, static_cast<GLsizei>(_numberOfDoubleBonds[i][j]));
            check_gl_error();
            glBindVertexArray(0);
          }

          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _numberOfPartialDoubleBonds[i][j]>0)
          {
            glUniform1i(_imposterBondTypeUniformLocation, 2);
            glBindVertexArray(_vertexPartialDoubleBondsImposterArrayObject[i][j]);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 18, static_cast<GLsizei>(_numberOfPartialDoubleBonds[i][j]));
            check_gl_error();
            glBindVertexArray(0);
          }

          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _numberOfTripleBonds[i][j]>0)
          {
            glUniform1i(_imposterBondTypeUniformLocation, 3);
            glBindVertexArray(_vertexTripleBondsImposterArrayObject[i][j]);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 54, static_cast<GLsizei>(_numberOfTripleBonds[i][j]));
            check_gl_error();
            glBindVertexArray(0);
          }
        }
      }
      index++;
    }
  }

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glUseProgram(0);
}

void OpenGLExternalBondShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLExternalBondShader::initializeImposterVertexArrayObject(GLuint vertexArrayObject, GLuint instanceBuffer)
{
  // the imposter hull is generated from gl_VertexID, so only the per-instance attributes are bound
  glBindVertexArray(vertexArrayObject);
  check_gl_error();

  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
  check_gl_error();

  glVertexAttribPointer(_imposterInstancePositionFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
  glVertexAttribPointer(_imposterInstancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
  glVertexAttribDivisor(_imposterInstancePositionFirstAtomAttributeLocation,1);
  glVertexAttribDivisor(_imposterInstancePositionSecondAtomAttributeLocation,1);
  check_gl_error();

  glVertexAttribPointer(_imposterInstanceColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
  glVertexAttribPointer(_imposterInstanceColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
  glVertexAttribDivisor(_imposterInstanceColorFirstAtomAttributeLocation,1);
  glVertexAttribDivisor(_imposterInstanceColorSecondAtomAttributeLocation,1);
  check_gl_error();

  glVertexAttribPointer(_imposterInstanceScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
  glVertexAttribDivisor(_imposterInstanceScaleAttributeLocation,1);
  check_gl_error();

  glEnableVertexAttribArray(_imposterInstancePositionFirstAtomAttributeLocation);
  glEnableVertexAttribArray(_imposterInstancePositionSecondAtomAttributeLocation);
  glEnableVertexAttribArray(_imposterInstanceColorFirstAtomAttributeLocation);
  glEnableVertexAttribArray(_imposterInstanceColorSecondAtomAttributeLocation);
  glEnableVertexAttribArray(_imposterInstanceScaleAttributeLocation);
  check_gl_error();

  glBindVertexArray(0);
}

void OpenGLExternalBondShader::initializeVertexArrayObject()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderBondSource* source = dynamic_cast<RKRenderBondSource*>(_renderStructures[i][j].get()))
      {
        std::vector<RKInPerInstanceAttributesBonds> bondInstanceData = source->renderExternalBonds();
        _numberOfAllBonds[i][j] = bondInstanceData.size();

        std::vector<RKInPerInstanceAttributesBonds> singleBondInstanceData;
        int32_t singleBondType = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(SKAsymmetricBond::SKBondType::singleBond);
        std::copy_if(bondInstanceData.begin(), bondInstanceData.end(), std::back_inserter(singleBondInstanceData),
                     [singleBondType](RKInPerInstanceAttributesBonds &i){return i.type == singleBondType;});
        _numberOfSingleBonds[i][j] = singleBondInstanceData.size();

        std::vector<RKInPerInstanceAttributesBonds> doubleBondInstanceData;
        int32_t doubleBondType = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(SKAsymmetricBond::SKBondType::doubleBond);
        std::copy_if(bondInstanceData.begin(), bondInstanceData.end(), std::back_inserter(doubleBondInstanceData),
                     [doubleBondType](RKInPerInstanceAttributesBonds &i){return i.type == doubleBondType;});
        _numberOfDoubleBonds[i][j] = doubleBondInstanceData.size();

        std::vector<RKInPerInstanceAttributesBonds> partialDoubleBondInstanceData;
        int32_t partialDoubleBondType = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(SKAsymmetricBond::SKBondType::partialDoubleBond);
        std::copy_if(bondInstanceData.begin(), bondInstanceData.end(), std::back_inserter(partialDoubleBondInstanceData),
                     [partialDoubleBondType](RKInPerInstanceAttributesBonds &i){return i.type == partialDoubleBondType;});
        _numberOfPartialDoubleBonds[i][j] = partialDoubleBondInstanceData.size();

        std::vector<RKInPerInstanceAttributesBonds> tripleBondInstanceData;
        int32_t tripleBondType = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(SKAsymmetricBond::SKBondType::tripleBond);
        std::copy_if(bondInstanceData.begin(), bondInstanceData.end(), std::back_inserter(tripleBondInstanceData),
                     [tripleBondType](RKInPerInstanceAttributesBonds &i){return i.type == tripleBondType;});
        _numberOfTripleBonds[i][j] = tripleBondInstanceData.size();

        auto upload = [this](GLuint buffer, const std::vector<RKInPerInstanceAttributesBonds> &data) {
          glBindBuffer(GL_ARRAY_BUFFER, buffer);
          if (!data.empty())
          {
            glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(RKInPerInstanceAttributesBonds), data.data(), GL_DYNAMIC_DRAW);
            check_gl_error();
          }
        };
        upload(_vertexAllBondsInstanceBuffer[i][j], bondInstanceData);
        upload(_vertexSingleBondsInstanceBuffer[i][j], singleBondInstanceData);
        upload(_vertexDoubleBondsInstanceBuffer[i][j], doubleBondInstanceData);
        upload(_vertexPartialDoubleBondsInstanceBuffer[i][j], partialDoubleBondInstanceData);
        upload(_vertexTripleBondsInstanceBuffer[i][j], tripleBondInstanceData);

        initializeImposterVertexArrayObject(_vertexAllBondsImposterArrayObject[i][j], _vertexAllBondsInstanceBuffer[i][j]);
        initializeImposterVertexArrayObject(_vertexSingleBondsImposterArrayObject[i][j], _vertexSingleBondsInstanceBuffer[i][j]);
        initializeImposterVertexArrayObject(_vertexDoubleBondsImposterArrayObject[i][j], _vertexDoubleBondsInstanceBuffer[i][j]);
        initializeImposterVertexArrayObject(_vertexPartialDoubleBondsImposterArrayObject[i][j], _vertexPartialDoubleBondsInstanceBuffer[i][j]);
        initializeImposterVertexArrayObject(_vertexTripleBondsImposterArrayObject[i][j], _vertexTripleBondsInstanceBuffer[i][j]);
      }
    }
  }
}

void OpenGLExternalBondShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsImposterArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsImposterArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsImposterArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsImposterArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsImposterArrayObject[i].data());

    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsInstanceBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsInstanceBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsInstanceBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsInstanceBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsInstanceBuffer[i].data());
  }
}

void OpenGLExternalBondShader::generateBuffers()
{
  _vertexAllBondsImposterArrayObject.resize(_renderStructures.size());
  _vertexSingleBondsImposterArrayObject.resize(_renderStructures.size());
  _vertexDoubleBondsImposterArrayObject.resize(_renderStructures.size());
  _vertexPartialDoubleBondsImposterArrayObject.resize(_renderStructures.size());
  _vertexTripleBondsImposterArrayObject.resize(_renderStructures.size());

  _numberOfAllBonds.resize(_renderStructures.size());
  _vertexAllBondsInstanceBuffer.resize(_renderStructures.size());
  _numberOfSingleBonds.resize(_renderStructures.size());
  _vertexSingleBondsInstanceBuffer.resize(_renderStructures.size());
  _numberOfDoubleBonds.resize(_renderStructures.size());
  _vertexDoubleBondsInstanceBuffer.resize(_renderStructures.size());
  _numberOfPartialDoubleBonds.resize(_renderStructures.size());
  _vertexPartialDoubleBondsInstanceBuffer.resize(_renderStructures.size());
  _numberOfTripleBonds.resize(_renderStructures.size());
  _vertexTripleBondsInstanceBuffer.resize(_renderStructures.size());

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    _vertexAllBondsImposterArrayObject[i].resize(_renderStructures[i].size());
    _vertexSingleBondsImposterArrayObject[i].resize(_renderStructures[i].size());
    _vertexDoubleBondsImposterArrayObject[i].resize(_renderStructures[i].size());
    _vertexPartialDoubleBondsImposterArrayObject[i].resize(_renderStructures[i].size());
    _vertexTripleBondsImposterArrayObject[i].resize(_renderStructures[i].size());

    _numberOfAllBonds[i].resize(_renderStructures[i].size());
    _vertexAllBondsInstanceBuffer[i].resize(_renderStructures[i].size());
    _numberOfSingleBonds[i].resize(_renderStructures[i].size());
    _vertexSingleBondsInstanceBuffer[i].resize(_renderStructures[i].size());
    _numberOfDoubleBonds[i].resize(_renderStructures[i].size());
    _vertexDoubleBondsInstanceBuffer[i].resize(_renderStructures[i].size());
    _numberOfPartialDoubleBonds[i].resize(_renderStructures[i].size());
    _vertexPartialDoubleBondsInstanceBuffer[i].resize(_renderStructures[i].size());
    _numberOfTripleBonds[i].resize(_renderStructures[i].size());
    _vertexTripleBondsInstanceBuffer[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsImposterArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsInstanceBuffer[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsImposterArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsInstanceBuffer[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsImposterArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsInstanceBuffer[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsImposterArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsInstanceBuffer[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsImposterArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsInstanceBuffer[i].data());
  }
}

void OpenGLExternalBondShader::deletePermanentBuffers()
{
}

void OpenGLExternalBondShader::generatePermanentBuffers()
{
}

void OpenGLExternalBondShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLExternalBondShader::_vertexShaderSourceImposter.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLExternalBondShader::_fragmentShaderSourceImposter.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _programImposter = glCreateProgram();
    check_gl_error();

    glAttachShader(_programImposter, vertexShader);
    check_gl_error();
    glAttachShader(_programImposter, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_programImposter, 0, "vFragColor");

    linkProgram(_programImposter);

    _imposterInstancePositionFirstAtomAttributeLocation = glGetAttribLocation(_programImposter, "instancePosition1");
    _imposterInstancePositionSecondAtomAttributeLocation = glGetAttribLocation(_programImposter, "instancePosition2");
    _imposterInstanceColorFirstAtomAttributeLocation = glGetAttribLocation(_programImposter, "instanceColor1");
    _imposterInstanceColorSecondAtomAttributeLocation = glGetAttribLocation(_programImposter, "instanceColor2");
    _imposterInstanceScaleAttributeLocation = glGetAttribLocation(_programImposter, "instanceScale");
    _imposterBondTypeUniformLocation = glGetUniformLocation(_programImposter, "bondType");

    if (_imposterInstancePositionFirstAtomAttributeLocation < 0) qDebug() << "Imposter shader did not contain the 'instancePosition1' attribute.";
    if (_imposterInstancePositionSecondAtomAttributeLocation < 0) qDebug() << "Imposter shader did not contain the 'instancePosition2' attribute.";
    if (_imposterInstanceColorFirstAtomAttributeLocation < 0) qDebug() << "Imposter shader did not contain the 'instanceColor1' attribute.";
    if (_imposterInstanceColorSecondAtomAttributeLocation < 0) qDebug() << "Imposter shader did not contain the 'instanceColor2' attribute.";
    if (_imposterInstanceScaleAttributeLocation < 0) qDebug() << "Imposter shader did not contain the 'instanceScale' attribute.";
    if (_imposterBondTypeUniformLocation < 0) qDebug() << "Imposter shader did not contain the 'bondType' uniform.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string  OpenGLExternalBondShader::_vertexShaderSourceImposter =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLBondImposterStringLiteral +
R"foo(
in vec4 instancePosition1;
in vec4 instancePosition2;
in vec4 instanceColor1;
in vec4 instanceColor2;
in vec4 instanceScale;

// single bond: 0, double bond: 1, partial double bond: 2, triple bond: 3
uniform int bondType;

// Inputs from vertex shader
out VS_OUT
{
  flat vec4 color1;
  flat vec4 color2;
  flat vec2 mixParam;

  smooth vec3 fragPos;
  flat vec3 pointA;
  flat vec3 pointB;
  flat float radius;
} vs_out;

void main(void)
{
  vec4 pos1 = instancePosition1;
  vec4 pos2 = instancePosition2;

  vs_out.mixParam.x = clamp(structureUniforms.atomScaleFactor, 0.0, 0.7) * instanceScale.x;
  vs_out.mixParam.y = 1.0 - clamp(structureUniforms.atomScaleFactor, 0.0, 0.7) * instanceScale.z;

  vs_out.color1 = instanceColor1;
  vs_out.color2 = instanceColor2;

  // sub-cylinder displacement for double/triple bonds (all bonds drawn as single cylinders in 'unity'-mode)
  int type = structureUniforms.isUnity ? 0 : bondType;
  vec2 offset = bondImposterSubCylinderOffset(type, gl_VertexID / 18);

  vec3 dr = normalize((pos1 - pos2).xyz);
  vec3 v1 = normalize(abs(dr.x) > abs(dr.z) ? vec3(-dr.y, dr.x, 0.0) : vec3(0.0, -dr.z, dr.y));
  vec3 v2 = normalize(cross(dr, v1));

  // mesh x-axis maps to -v1, mesh z-axis maps to -v2 (matches the orientationMatrix of the mesh shader)
  vec3 displacement = structureUniforms.bondScaling * (offset.x * (-v1) + offset.y * (-v2));
  float radius = structureUniforms.bondScaling;

  mat4 mv = frameUniforms.viewMatrix * structureUniforms.modelMatrix;
  vec3 a = (mv * vec4(pos1.xyz + displacement, 1.0)).xyz;
  vec3 b = (mv * vec4(pos2.xyz + displacement, 1.0)).xyz;

  bool orthographic = (frameUniforms.projectionMatrix[3][3] > 0.5);
  vec3 posEye = bondImposterHullPosition(a, b, radius, gl_VertexID, orthographic);

  vs_out.fragPos = posEye;
  vs_out.pointA = a;
  vs_out.pointB = b;
  vs_out.radius = radius;
  gl_Position = frameUniforms.projectionMatrix * vec4(posEye, 1.0);

  // invisible bonds have w set to -1, leading to clipping of the entire hull
  if (pos1.w < 0.0 || pos2.w < 0.0)
  {
    gl_Position = vec4(0.0, 0.0, 0.0, -1.0);
  }
}
)foo";

const std::string  OpenGLExternalBondShader::_fragmentShaderSourceImposter =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLBondImposterStringLiteral +
OpenGLUniformStringLiterals::OpenGLRGBHSVStringLiteral +
R"foo(

out vec4 vFragColor;

// Input from vertex shader
in VS_OUT
{
  flat vec4 color1;
  flat vec4 color2;
  flat vec2 mixParam;

  smooth vec3 fragPos;
  flat vec3 pointA;
  flat vec3 pointB;
  flat float radius;
} fs_in;

void main(void)
{
  bool orthographic = (frameUniforms.projectionMatrix[3][3] > 0.5);
  vec3 ro = orthographic ? vec3(fs_in.fragPos.xy, 0.0) : vec3(0.0);
  vec3 rd = orthographic ? vec3(0.0, 0.0, -1.0) : normalize(fs_in.fragPos);

  vec3 N;
  float ct;
  mat4 toStructure = structureUniforms.inverseModelMatrix * frameUniforms.viewMatrixInverse;
  float t = bondImposterClippedIntersect(ro, rd, fs_in.pointA, fs_in.pointB, fs_in.radius, toStructure, N, ct);
  if (t < 0.0) discard;

  vec3 pos = ro + t * rd;

  vec4 screenPos = frameUniforms.projectionMatrix * vec4(pos, 1.0);
  float ndcDepth = screenPos.z / screenPos.w;
  gl_FragDepth = 0.5 * (gl_DepthRange.diff * ndcDepth + gl_DepthRange.near + gl_DepthRange.far);

  vec3 L = normalize((lightUniforms.lights[0].position - vec4(pos, 1.0) * lightUniforms.lights[0].position.w).xyz);
  vec3 V = normalize(-pos);

  // Calculate R locally
  vec3 R = reflect(-L, N);

  vec4 ambient = lightUniforms.lights[0].ambient * structureUniforms.bondAmbientColor;
  vec4 specular = pow(max(dot(R, V), 0.0),  lightUniforms.lights[0].shininess + structureUniforms.bondShininess) * lightUniforms.lights[0].specular * structureUniforms.bondSpecularColor;
  vec4 diffuse = vec4(max(dot(N, L), 0.0));
  float tmix = clamp((ct - fs_in.mixParam.x)/(fs_in.mixParam.y - fs_in.mixParam.x),0.0,1.0);

  switch(structureUniforms.bondColorMode)
  {
    case 0:
      diffuse *= structureUniforms.bondDiffuseColor;
      break;
    case 1:
      diffuse *= (tmix < 0.5 ? fs_in.color1 : fs_in.color2);
      break;
    case 2:
      diffuse *= mix(fs_in.color1,fs_in.color2,smoothstep(0.0,1.0,tmix));
      break;
  }

  vec4 color= vec4(ambient.xyz + diffuse.xyz + specular.xyz, 1.0);

  if (structureUniforms.bondHDR)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.bondHDRExposure);
    vLdrColor.a = 1.0;
    color= vLdrColor;
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.bondHue;
  hsv.y = hsv.y * structureUniforms.bondSaturation;
  hsv.z = hsv.z * structureUniforms.bondValue;
  vFragColor = vec4(hsv2rgb(hsv),1.0);
}
)foo";


