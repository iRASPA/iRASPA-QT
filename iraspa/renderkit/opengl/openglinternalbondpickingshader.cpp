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

#include "openglinternalbondpickingshader.h"
#include <QDebug>
#include "glgeterror.h"
#include "opengluniformstringliterals.h"

OpenGLInternalBondPickingShader::OpenGLInternalBondPickingShader(OpenGLBondShader &bondShader): _internalBondShader(bondShader._internalBondShader)
{
}

void OpenGLInternalBondPickingShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLInternalBondPickingShader::paintGL(GLuint structureUniformBuffer)
{
  glUseProgram(_program);
  check_gl_error();
  glDisable(GL_CULL_FACE);

  int index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderBondSource* source = dynamic_cast<RKRenderBondSource*>(_renderStructures[i][j].get()))
      {
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));

        auto draw = [&](GLuint vao, size_t count, int bondType, int vertices) {
          if (!(source->drawBonds() && _renderStructures[i][j]->isVisible() && count > 0))
          {
            return;
          }
          glUniform1i(_imposterBondTypeUniformLocation, bondType);
          glBindVertexArray(vao);
          glDrawArraysInstanced(GL_TRIANGLES, 0, vertices, static_cast<GLsizei>(count));
          check_gl_error();
          glBindVertexArray(0);
        };

        if (source->isUnity())
        {
          draw(_vertexAllBondsArrayObject[i][j], _internalBondShader._numberOfAllBonds[i][j], 0, 18);
        }
        else
        {
          draw(_vertexSingleBondsArrayObject[i][j], _internalBondShader._numberOfSingleBonds[i][j], 0, 18);
          draw(_vertexDoubleBondsArrayObject[i][j], _internalBondShader._numberOfDoubleBonds[i][j], 1, 36);
          draw(_vertexPartialDoubleBondsArrayObject[i][j], _internalBondShader._numberOfPartialDoubleBonds[i][j], 2, 18);
          draw(_vertexTripleBondsArrayObject[i][j], _internalBondShader._numberOfTripleBonds[i][j], 3, 54);
        }
      }
      index++;
    }
  }
  glEnable(GL_CULL_FACE);
  glUseProgram(0);
}

void OpenGLInternalBondPickingShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLInternalBondPickingShader::initializeVertexArrayObject()
{
  auto bindImposter = [&](GLuint vao, GLuint instanceBuffer) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds),
                          (void *)offsetof(RKInPerInstanceAttributesBonds, position1));
    glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds),
                          (void *)offsetof(RKInPerInstanceAttributesBonds, position2));
    glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation, 1);
    glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation, 1);
    glVertexAttribIPointer(_pickingTagAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds),
                           reinterpret_cast<GLvoid *>(offsetof(RKInPerInstanceAttributesBonds, tag)));
    glVertexAttribDivisor(_pickingTagAttributeLocation, 1);
    glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
    glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
    glEnableVertexAttribArray(_pickingTagAttributeLocation);
    glBindVertexArray(0);
  };

  for (size_t i = 0; i < _renderStructures.size(); i++)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); j++)
    {
      bindImposter(_vertexAllBondsArrayObject[i][j], _internalBondShader._vertexAllBondsInstanceBuffer[i][j]);
      bindImposter(_vertexSingleBondsArrayObject[i][j], _internalBondShader._vertexSingleBondsInstanceBuffer[i][j]);
      bindImposter(_vertexDoubleBondsArrayObject[i][j], _internalBondShader._vertexDoubleBondsInstanceBuffer[i][j]);
      bindImposter(_vertexPartialDoubleBondsArrayObject[i][j], _internalBondShader._vertexPartialDoubleBondsInstanceBuffer[i][j]);
      bindImposter(_vertexTripleBondsArrayObject[i][j], _internalBondShader._vertexTripleBondsInstanceBuffer[i][j]);
    }
  }
}

void OpenGLInternalBondPickingShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsArrayObject[i].data());
  }
}

void OpenGLInternalBondPickingShader::generateBuffers()
{
  _vertexAllBondsArrayObject.resize(_renderStructures.size());
  _vertexSingleBondsArrayObject.resize(_renderStructures.size());
  _vertexDoubleBondsArrayObject.resize(_renderStructures.size());
  _vertexPartialDoubleBondsArrayObject.resize(_renderStructures.size());
  _vertexTripleBondsArrayObject.resize(_renderStructures.size());

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    _vertexAllBondsArrayObject[i].resize(_renderStructures[i].size());
    _vertexSingleBondsArrayObject[i].resize(_renderStructures[i].size());
    _vertexDoubleBondsArrayObject[i].resize(_renderStructures[i].size());
    _vertexPartialDoubleBondsArrayObject[i].resize(_renderStructures[i].size());
    _vertexTripleBondsArrayObject[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsArrayObject[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsArrayObject[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsArrayObject[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsArrayObject[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsArrayObject[i].data());
  }
}

void OpenGLInternalBondPickingShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLInternalBondPickingShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLInternalBondPickingShader::_fragmentShaderSource.c_str());

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

    _instancePositionFirstAtomAttributeLocation = glGetAttribLocation(_program, "instancePosition1");
    _instancePositionSecondAtomAttributeLocation = glGetAttribLocation(_program, "instancePosition2");
    _pickingTagAttributeLocation = glGetAttribLocation(_program, "instanceTag");
    _imposterBondTypeUniformLocation = glGetUniformLocation(_program, "bondType");

    if (_instancePositionFirstAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition1' attribute.";
    if (_instancePositionSecondAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition2' attribute.";
    if (_pickingTagAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceTag' attribute.";
    if (_imposterBondTypeUniformLocation < 0) qDebug() << "Shader did not contain the 'bondType' uniform.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string  OpenGLInternalBondPickingShader::_vertexShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLBondImposterStringLiteral +
OpenGLUniformStringLiterals::OpenGLBondPickingImposterVSOutStringLiteral;

const std::string  OpenGLInternalBondPickingShader::_fragmentShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLBondImposterStringLiteral +
OpenGLUniformStringLiterals::OpenGLBondPickingImposterFSInStringLiteral +
R"foo(
void main(void)
{
  bool orthographic = (frameUniforms.projectionMatrix[3][3] > 0.5);
  vec3 ro = orthographic ? vec3(fs_in.fragPos.xy, 0.0) : vec3(0.0);
  vec3 rd = orthographic ? vec3(0.0, 0.0, -1.0) : normalize(fs_in.fragPos);

  vec3 N;
  float ct;
  float t = bondImposterIntersect(ro, rd, fs_in.pointA, fs_in.pointB, fs_in.radius, N, ct);
  if (t < 0.0) discard;
  vec3 pos = ro + t * rd;
  vec4 screenPos = frameUniforms.projectionMatrix * vec4(pos, 1.0);
  float ndcDepth = screenPos.z / screenPos.w;
  gl_FragDepth = 0.5 * (gl_DepthRange.diff * ndcDepth + gl_DepthRange.near + gl_DepthRange.far);

  vFragColor = uvec4(2,structureUniforms.sceneIdentifier,structureUniforms.MovieIdentifier,fs_in.instanceId);
}
)foo";
