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

#include "openglexternalbondselectionglowshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "cylindergeometry.h"
#include <algorithm>
#include <type_traits>

OpenGLExternalBondSelectionGlowShader::OpenGLExternalBondSelectionGlowShader(OpenGLExternalBondSelectionInstanceShader &instanceShader, OpenGLBondShader &bondShader):
  _instanceShader(instanceShader), _bondShader(bondShader._externalBondShader)
{

}

void OpenGLExternalBondSelectionGlowShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLExternalBondSelectionGlowShader::paintGL(GLuint structureUniformBuffer)
{
  glEnable(GL_CLIP_DISTANCE0);
  glEnable(GL_CLIP_DISTANCE1);
  glEnable(GL_CLIP_DISTANCE2);
  glEnable(GL_CLIP_DISTANCE3);
  glEnable(GL_CLIP_DISTANCE4);
  glEnable(GL_CLIP_DISTANCE5);

  glDepthMask(GL_FALSE);

  glUseProgram(_program);
  check_gl_error();

  int index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if(_renderStructures[i][j]->bondSelectionStyle() == RKSelectionStyle::glow)
      {
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));

        if (_renderStructures[i][j]->isUnity())
        {
          if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _bondShader._allBondNumberOfIndices[i][j]>0 && _instanceShader._numberOfAllBonds[i][j]>0)
          {
            glBindVertexArray(_vertexAllBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_bondShader._allBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_instanceShader._numberOfAllBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }
        }
        else
        {
          if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _bondShader._singleBondNumberOfIndices[i][j]>0 && _instanceShader._numberOfSingleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexSingleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_bondShader._singleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_instanceShader._numberOfSingleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }

          if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _bondShader._doubleBondNumberOfIndices[i][j]>0 && _instanceShader._numberOfDoubleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexDoubleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_bondShader._doubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_instanceShader._numberOfDoubleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }

          if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _bondShader._partialDoubleBondNumberOfIndices[i][j]>0 && _instanceShader._numberOfPartialDoubleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexPartialDoubleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_bondShader._partialDoubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_instanceShader._numberOfPartialDoubleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }

          if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _bondShader._tripleBondNumberOfIndices[i][j]>0 && _instanceShader._numberOfTripleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexTripleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_bondShader._tripleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_instanceShader._numberOfTripleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }
        }
      }
      index++;
    }
  }
  glUseProgram(0);

  glDepthMask(GL_TRUE);

  glDisable(GL_CLIP_DISTANCE0);
  glDisable(GL_CLIP_DISTANCE1);
  glDisable(GL_CLIP_DISTANCE2);
  glDisable(GL_CLIP_DISTANCE3);
  glDisable(GL_CLIP_DISTANCE4);
  glDisable(GL_CLIP_DISTANCE5);
}


void OpenGLExternalBondSelectionGlowShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLExternalBondSelectionGlowShader::initializeVertexArrayObject()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      glBindVertexArray(_vertexAllBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _bondShader._vertexAllBondsBuffer[i][j]);
      check_gl_error();


      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bondShader._indexAllBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _instanceShader._vertexAllBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceScaleAttributeLocation,1);
      check_gl_error();

      glVertexAttribIPointer(_instanceTypeAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, type));
      glVertexAttribDivisor(_instanceTypeAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      glEnableVertexAttribArray(_instanceTypeAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);



      glBindVertexArray(_vertexSingleBondsArrayObject[i][j]);
      check_gl_error();


      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _bondShader._vertexSingleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bondShader._indexSingleBondsBuffer[i][j]);

      glBindBuffer(GL_ARRAY_BUFFER, _instanceShader._vertexSingleBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceScaleAttributeLocation,1);
      check_gl_error();

      glVertexAttribIPointer(_instanceTypeAttributeLocation, 1, GL_INT,sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, type));
      glVertexAttribDivisor(_instanceTypeAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      glEnableVertexAttribArray(_instanceTypeAttributeLocation);

      check_gl_error();
      glBindVertexArray(0);


      glBindVertexArray(_vertexDoubleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _bondShader._vertexDoubleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bondShader._indexDoubleBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _instanceShader._vertexDoubleBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceScaleAttributeLocation,1);
      check_gl_error();

      glVertexAttribIPointer(_instanceTypeAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, type));
      glVertexAttribDivisor(_instanceTypeAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      glEnableVertexAttribArray(_instanceTypeAttributeLocation);

      check_gl_error();
      glBindVertexArray(0);



      glBindVertexArray(_vertexPartialDoubleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _bondShader._vertexPartialDoubleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bondShader._indexPartialDoubleBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _instanceShader._vertexPartialDoubleBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceScaleAttributeLocation,1);
      check_gl_error();

      glVertexAttribIPointer(_instanceTypeAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, type));
      glVertexAttribDivisor(_instanceTypeAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      glEnableVertexAttribArray(_instanceTypeAttributeLocation);

      check_gl_error();
      glBindVertexArray(0);


      glBindVertexArray(_vertexTripleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _bondShader._vertexTripleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bondShader._indexTripleBondsBuffer[i][j]);

      glBindBuffer(GL_ARRAY_BUFFER, _instanceShader._vertexTripleBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceScaleAttributeLocation,1);
      check_gl_error();

      glVertexAttribIPointer(_instanceTypeAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, type));
      glVertexAttribDivisor(_instanceTypeAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      glEnableVertexAttribArray(_instanceTypeAttributeLocation);

      check_gl_error();
      glBindVertexArray(0);
    }
  }
}

void OpenGLExternalBondSelectionGlowShader::deleteBuffers()
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

void OpenGLExternalBondSelectionGlowShader::generateBuffers()
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

void OpenGLExternalBondSelectionGlowShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLExternalBondSelectionGlowShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLExternalBondSelectionGlowShader::_fragmentShaderSource.c_str());

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
    _instancePositionFirstAtomAttributeLocation = glGetAttribLocation(_program, "instancePosition1");
    _instancePositionSecondAtomAttributeLocation = glGetAttribLocation(_program, "instancePosition2");
    _instanceColorFirstAtomAttributeLocation = glGetAttribLocation(_program, "instanceColor1");
    _instanceColorSecondAtomAttributeLocation = glGetAttribLocation(_program, "instanceColor2");
    _instanceScaleAttributeLocation = glGetAttribLocation(_program, "instanceScale");
    _instanceTypeAttributeLocation = glGetAttribLocation(_program, "instanceType");

    if (_vertexNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instancePositionFirstAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition1' attribute.";
    if (_instancePositionSecondAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition2' attribute.";
    if (_instanceColorFirstAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceColor1' attribute.";
    if (_instanceColorSecondAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceColor2' attribute.";
    if (_instanceScaleAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";
    if (_instanceTypeAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceType' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string  OpenGLExternalBondSelectionGlowShader::_vertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;
in vec4 vertexNormal;

in vec4 instancePosition1;
in vec4 instancePosition2;
in vec4 instanceColor1;
in vec4 instanceColor2;
in vec4 instanceScale;
in int instanceType;


// Inputs from vertex shader
out VS_OUT
{
  flat vec4 color1;
  flat vec4 color2;
  smooth vec4 mix;

  vec4 ambient;
  vec4 specular;

  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
  smooth vec3 Model_N;
} vs_out;


void main(void)
{
  vec3 v1,v2;

  vec4 scale = instanceScale;

  vec4 pos1 = instancePosition1;
  vec4 pos2 = instancePosition2;

  vec3 dr = (pos1 - pos2).xyz;
  float bondLength = length(dr);

  vs_out.mix.x = clamp(structureUniforms.atomScaleFactor, 0.0, 0.7) * scale.x;
  vs_out.mix.y = vertexPosition.y;  // range 0.0..1.0
  vs_out.mix.z = 1.0 - clamp(structureUniforms.atomScaleFactor, 0.0, 0.7) * scale.z;
  vs_out.mix.w = scale.x/scale.z;

  vs_out.Model_N = vertexPosition.xyz;

  scale.x = structureUniforms.bondScaling;
  scale.y = bondLength;
  scale.z = structureUniforms.bondScaling;
  scale.w = 1.0;

  vec4 pos;
  vec4 scaleFactor = vec4(1.01 * structureUniforms.bondSelectionScaling,1.0,1.01 * structureUniforms.bondSelectionScaling,1.0);

  if(instanceType == 1) // double bond
  {
    pos = (vertexPosition-vec4(sign(vertexPosition.x),0.0,0.0,0.0))*scaleFactor+vec4(sign(vertexPosition.x),0.0,0.0,0.0);
  }
  else if (instanceType == 2) // triple bond
  {
    if(vertexPosition.x<0.0 && vertexPosition.z<0.0)
    {
      pos = (vertexPosition+vec4(1.0,0.0,0.5*sqrt(3.0),0.0))*scaleFactor-vec4(1.0,0.0,0.5*sqrt(3.0),0.0);
    }
    else if(vertexPosition.x>0.0 && vertexPosition.z<0.0)
    {
      pos = (vertexPosition+vec4(-1.0,0.0,0.5*sqrt(3.0),0.0))*scaleFactor-vec4(-1.0,0.0,0.5*sqrt(3.0),0.0);
    }
    else
    {
      pos = (vertexPosition-vec4(0.0,0.0,0.5*sqrt(3.0),0.0))*scaleFactor+vec4(0.0,0.0,0.5*sqrt(3.0),0.0);
    }
  }
  else // single bond
  {
    pos = vertexPosition * scaleFactor;
  }

  dr = normalize(dr);
  v1 = normalize(abs(dr.x) > abs(dr.z) ? vec3(-dr.y, dr.x, 0.0) : vec3(0.0, -dr.z, dr.y));
  v2=normalize(cross(dr,v1));
  mat4 orientationMatrix=mat4x4(vec4(-v1.x,-v1.y,-v1.z,0),
                                vec4(-dr.x,-dr.y,-dr.z,0),
                                vec4(-v2.x,-v2.y,-v2.z,0),
                                vec4(0,0,0,1));

  vs_out.ambient = lightUniforms.lights[0].ambient * structureUniforms.bondAmbientColor;
  vs_out.specular = lightUniforms.lights[0].specular * structureUniforms.bondSpecularColor;
  if (structureUniforms.bondColorMode == 0)
  {
    vs_out.color1 = lightUniforms.lights[0].diffuse * structureUniforms.bondDiffuseColor* instanceColor1;
    vs_out.color2 = lightUniforms.lights[0].diffuse * structureUniforms.bondDiffuseColor * instanceColor2;
  }
  else
  {
    vs_out.color1 = lightUniforms.lights[0].diffuse * structureUniforms.atomDiffuseColor * instanceColor1;
    vs_out.color2 = lightUniforms.lights[0].diffuse * structureUniforms.atomDiffuseColor * instanceColor2;
  }

  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * orientationMatrix * vertexNormal).xyz;

  vec4 P =  frameUniforms.viewMatrix *  structureUniforms.modelMatrix * vec4((orientationMatrix * (scale * pos) + pos1).xyz,1.0);

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  // Calculate view vector
  vs_out.V = -P.xyz;

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * (orientationMatrix * (scale * pos) + pos1);

  vec4 vertexPos = (orientationMatrix * (scale * pos) + pos1);
  gl_ClipDistance[0] = dot(structureUniforms.clipPlaneBack, vertexPos);
  gl_ClipDistance[1] = dot(structureUniforms.clipPlaneBottom, vertexPos);
  gl_ClipDistance[2] = dot(structureUniforms.clipPlaneLeft, vertexPos);

  gl_ClipDistance[3] = dot(structureUniforms.clipPlaneFront, vertexPos);
  gl_ClipDistance[4] = dot(structureUniforms.clipPlaneTop, vertexPos);
  gl_ClipDistance[5] = dot(structureUniforms.clipPlaneRight, vertexPos);
}
)foo";

const std::string  OpenGLExternalBondSelectionGlowShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
OpenGLRGBHSVStringLiteral +
R"foo(
out vec4 vFragColor;

// Input from vertex shader
in VS_OUT
{
  flat vec4 color1;
  flat vec4 color2;
  smooth vec4 mix;

  vec4 ambient;
  vec4 specular;

  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
  smooth vec3 Model_N;
} fs_in;

void main(void)
{

  // Normalize the incoming N and L vectors
  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L);
  vec3 V = normalize(fs_in.V);

   // Calculate R locally
  vec3 R = reflect(-L, N);

  vec4 ambient = fs_in.ambient;
  vec4 specular = pow(max(dot(R, V), 0.0),  lightUniforms.lights[0].shininess + structureUniforms.bondShininess) * fs_in.specular;
  vec4 diffuse = vec4(max(dot(N, L), 0.0));
  float t = clamp((fs_in.mix.y - fs_in.mix.x)/(fs_in.mix.z - fs_in.mix.x),0.0,1.0);

  switch(structureUniforms.bondColorMode)
  {
    case 0:
      diffuse *= lightUniforms.lights[0].diffuse * structureUniforms.bondDiffuseColor;
      break;
    case 1:
      diffuse *= (t < 0.5 ? fs_in.color1 : fs_in.color2);
      break;
    case 2:
      diffuse *= mix(fs_in.color1,fs_in.color2,smoothstep(0.0,1.0,t));
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
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.bondSelectionIntensity;
  vFragColor = bloomLevel * vec4(hsv2rgb(hsv),1.0);
}
)foo";
