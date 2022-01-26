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

#include "openglexternalbondpickingshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "cylindergeometry.h"
#include <algorithm>
#include <type_traits>
#include "opengluniformstringliterals.h"

OpenGLExternalBondPickingShader::OpenGLExternalBondPickingShader(OpenGLBondShader &bondShader): _externalBondShader(bondShader._externalBondShader)
{

}

void OpenGLExternalBondPickingShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLExternalBondPickingShader::paintGL(GLuint structureUniformBuffer)
{
  glEnable(GL_CLIP_DISTANCE0);
  glEnable(GL_CLIP_DISTANCE1);
  glEnable(GL_CLIP_DISTANCE2);
  glEnable(GL_CLIP_DISTANCE3);
  glEnable(GL_CLIP_DISTANCE4);
  glEnable(GL_CLIP_DISTANCE5);


  glUseProgram(_program);
  check_gl_error();

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
          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _externalBondShader._allBondNumberOfIndices[i][j]>0
             && _externalBondShader._numberOfAllBonds[i][j]>0)
          {
            glBindVertexArray(_vertexAllBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_externalBondShader._allBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_externalBondShader._numberOfAllBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }
        }
        else
        {
          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _externalBondShader._singleBondNumberOfIndices[i][j]>0
             && _externalBondShader._numberOfSingleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexSingleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_externalBondShader._singleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_externalBondShader._numberOfSingleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }

          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _externalBondShader._doubleBondNumberOfIndices[i][j]>0
             && _externalBondShader._numberOfDoubleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexDoubleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_externalBondShader._doubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_externalBondShader._numberOfDoubleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }

          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _externalBondShader._partialDoubleBondNumberOfIndices[i][j]>0
             && _externalBondShader._numberOfPartialDoubleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexPartialDoubleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_externalBondShader._partialDoubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_externalBondShader._numberOfPartialDoubleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }

          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _externalBondShader._tripleBondNumberOfIndices[i][j]>0
             && _externalBondShader._numberOfTripleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexTripleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_externalBondShader._tripleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_externalBondShader._numberOfTripleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }
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



void OpenGLExternalBondPickingShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLExternalBondPickingShader::initializeVertexArrayObject()
{
  CappedCylinderSingleBondGeometry singleBondCylinder =  CappedCylinderSingleBondGeometry(1.0,7);
  CappedCylinderDoubleBondGeometry doubleBondCylinder =  CappedCylinderDoubleBondGeometry(1.0,7);
  CappedCylinderPartialDoubleBondGeometry partialDoubleBondCylinder =  CappedCylinderPartialDoubleBondGeometry(1.0,7);
  CappedCylinderTripleBondGeometry tripleBondCylinder =  CappedCylinderTripleBondGeometry(1.0,7);

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      glBindVertexArray(_vertexAllBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexAllBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _externalBondShader._indexAllBondsBuffer[i][j]);
      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexAllBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribIPointer(_pickingTagAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesBonds,tag)));
      glVertexAttribDivisor(_pickingTagAttributeLocation, 1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_pickingTagAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);



      glBindVertexArray(_vertexSingleBondsArrayObject[i][j]);
      check_gl_error();


      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexSingleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _externalBondShader._indexSingleBondsBuffer[i][j]);

      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexSingleBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribIPointer(_pickingTagAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesBonds,tag)));
      glVertexAttribDivisor(_pickingTagAttributeLocation, 1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_pickingTagAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);


      glBindVertexArray(_vertexDoubleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexDoubleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _externalBondShader._indexDoubleBondsBuffer[i][j]);

      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexDoubleBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribIPointer(_pickingTagAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesBonds,tag)));
      glVertexAttribDivisor(_pickingTagAttributeLocation, 1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_pickingTagAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);



      glBindVertexArray(_vertexPartialDoubleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexPartialDoubleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _externalBondShader._indexPartialDoubleBondsBuffer[i][j]);

      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexPartialDoubleBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();


      glVertexAttribIPointer(_pickingTagAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesBonds,tag)));
      glVertexAttribDivisor(_pickingTagAttributeLocation, 1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_pickingTagAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);


      glBindVertexArray(_vertexTripleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexTripleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _externalBondShader._indexTripleBondsBuffer[i][j]);

      glBindBuffer(GL_ARRAY_BUFFER, _externalBondShader._vertexTripleBondsInstanceBuffer[i][j]);

      glVertexAttribPointer(_instancePositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instancePositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instancePositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instancePositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribIPointer(_pickingTagAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesBonds), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesBonds,tag)));
      glVertexAttribDivisor(_pickingTagAttributeLocation, 1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_pickingTagAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);
    }
  }
}


void OpenGLExternalBondPickingShader::deleteBuffers()
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

void OpenGLExternalBondPickingShader::generateBuffers()
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

void OpenGLExternalBondPickingShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLExternalBondPickingShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLExternalBondPickingShader::_fragmentShaderSource.c_str());

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
    _instancePositionFirstAtomAttributeLocation = glGetAttribLocation(_program, "instancePosition1");
    _instancePositionSecondAtomAttributeLocation = glGetAttribLocation(_program, "instancePosition2");
    _pickingTagAttributeLocation = glGetAttribLocation(_program, "instanceTag");

    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instancePositionFirstAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition1' attribute.";
    if (_instancePositionSecondAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition2' attribute.";
    if (_pickingTagAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceTag' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string  OpenGLExternalBondPickingShader::_vertexShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;
in vec4 instancePosition1;
in vec4 instancePosition2;
in int instanceTag;

// works for both orthogonal and perspective
float frontFacing(vec4 pos0, vec4 pos1, vec4 pos2)
{
  return pos0.x*pos1.y - pos1.x*pos0.y + pos1.x*pos2.y - pos2.x*pos1.y + pos2.x*pos0.y - pos0.x*pos2.y;
}

// Inputs from vertex shader
out VS_OUT
{
  flat int instanceId;
} vs_out;

void main(void)
{
  vec3 v1,v2;
  vec4 scale;

  vs_out.instanceId = instanceTag;

  vec4 pos =  vertexPosition;

  vec4 pos1 = instancePosition1;
  vec4 pos2 = instancePosition2;

  vec3 dr = (pos1 - pos2).xyz;
  float bondLength = length(dr);

  scale.x = structureUniforms.bondScaling;
  scale.y = bondLength;
  scale.z = structureUniforms.bondScaling;
  scale.w = 1.0;

  dr = normalize(dr);
  v1 = normalize(abs(dr.x) > abs(dr.z) ? vec3(-dr.y, dr.x, 0.0) : vec3(0.0, -dr.z, dr.y));
  v2=normalize(cross(dr,v1));
  mat4 orientationMatrix=mat4x4(vec4(-v1.x,-v1.y,-v1.z,0),
                                vec4(-dr.x,-dr.y,-dr.z,0),
                                vec4(-v2.x,-v2.y,-v2.z,0),
                                vec4(0,0,0,1));


  vec4 vertexPos = (orientationMatrix * (scale * pos) + pos1);
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * vertexPos;

  float v_clipDistLeft = dot(structureUniforms.clipPlaneLeft,vertexPos);
  float v_clipDistRight = dot(structureUniforms.clipPlaneRight,vertexPos);

  float v_clipDistTop = dot(structureUniforms.clipPlaneTop,vertexPos);
  float v_clipDistBottom = dot(structureUniforms.clipPlaneBottom,vertexPos);

  float v_clipDistFront = dot(structureUniforms.clipPlaneFront,vertexPos);
  float v_clipDistBack = dot(structureUniforms.clipPlaneBack,vertexPos);

  // compute 3 reference points to determine front- or backfacing
  mat4 mvpMatrix = frameUniforms.mvpMatrix *  structureUniforms.modelMatrix;
  vec4 boxPosition0 = mvpMatrix * (structureUniforms.boxMatrix*vec4(0.0, 0.0, 0.0, 1.0));
  vec4 boxPosition1 = mvpMatrix * (structureUniforms.boxMatrix*vec4(1.0, 0.0, 0.0, 1.0));
  vec4 boxPosition2 = mvpMatrix * (structureUniforms.boxMatrix*vec4(1.0, 1.0, 0.0, 1.0));
  vec4 boxPosition3 = mvpMatrix * (structureUniforms.boxMatrix*vec4(0.0, 1.0, 0.0, 1.0));
  vec4 boxPosition4 = mvpMatrix * (structureUniforms.boxMatrix*vec4(0.0, 0.0, 1.0, 1.0));
  vec4 boxPosition5 = mvpMatrix * (structureUniforms.boxMatrix*vec4(1.0, 0.0, 1.0, 1.0));
  vec4 boxPosition6 = mvpMatrix * (structureUniforms.boxMatrix*vec4(1.0, 1.0, 1.0, 1.0));
  vec4 boxPosition7 = mvpMatrix * (structureUniforms.boxMatrix*vec4(0.0, 1.0, 1.0, 1.0));


  // perspective division
  boxPosition0 = boxPosition0/boxPosition0.w;
  boxPosition1 = boxPosition1/boxPosition1.w;
  boxPosition2 = boxPosition2/boxPosition2.w;
  boxPosition3 = boxPosition3/boxPosition3.w;
  boxPosition4 = boxPosition4/boxPosition4.w;
  boxPosition5 = boxPosition5/boxPosition5.w;
  boxPosition6 = boxPosition6/boxPosition6.w;
  boxPosition7 = boxPosition7/boxPosition7.w;

  float leftFrontfacing = frontFacing(boxPosition0, boxPosition3, boxPosition7);
  float rightFrontfacing = frontFacing(boxPosition1, boxPosition5, boxPosition2);

  float topFrontFacing = frontFacing(boxPosition3, boxPosition2, boxPosition7);
  float bottomFrontFacing = frontFacing(boxPosition0, boxPosition4, boxPosition1);

  float frontFrontFacing = frontFacing(boxPosition4, boxPosition6, boxPosition5);
  float backFrontFacing = frontFacing(boxPosition0, boxPosition1, boxPosition2);


  gl_ClipDistance[0] = (leftFrontfacing<0.0) ? v_clipDistLeft : 0.0;
  gl_ClipDistance[1] = (rightFrontfacing<0.0) ? v_clipDistRight : 0.0;

  gl_ClipDistance[2] = (topFrontFacing<0.0) ? v_clipDistTop : 0.0;
  gl_ClipDistance[3] = (bottomFrontFacing<0.0) ? v_clipDistBottom : 0.0;

  gl_ClipDistance[4] = (frontFrontFacing<0.0) ? v_clipDistFront : 0.0;
  gl_ClipDistance[5] = (backFrontFacing<0.0) ? v_clipDistBack : 0.0;
}
)foo";

const std::string  OpenGLExternalBondPickingShader::_fragmentShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
R"foo(

// Input from vertex shader
in VS_OUT
{
  flat int instanceId;
} fs_in;

out uvec4 vFragColor;

void main(void)
{
  vFragColor = uvec4(2,structureUniforms.sceneIdentifier,structureUniforms.MovieIdentifier,fs_in.instanceId);
}
)foo";
