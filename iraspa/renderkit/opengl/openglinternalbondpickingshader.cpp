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
#include <iostream>
#include "glgeterror.h"
#include "cylindergeometry.h"
#include <algorithm>
#include <type_traits>

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
          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _internalBondShader._allBondNumberOfIndices[i][j]>0
              && _internalBondShader._numberOfAllBonds[i][j]>0)
          {
            glBindVertexArray(_vertexAllBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_internalBondShader._allBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_internalBondShader._numberOfAllBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }
        }
        else
        {
          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _internalBondShader._singleBondNumberOfIndices[i][j]>0
             && _internalBondShader._numberOfSingleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexSingleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_internalBondShader._singleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_internalBondShader._numberOfSingleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }

          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _internalBondShader._doubleBondNumberOfIndices[i][j]>0
             && _internalBondShader._numberOfDoubleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexDoubleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_internalBondShader._doubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_internalBondShader._numberOfDoubleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }

          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _internalBondShader._partialDoubleBondNumberOfIndices[i][j]>0
             && _internalBondShader._numberOfPartialDoubleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexPartialDoubleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_internalBondShader._partialDoubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_internalBondShader._numberOfPartialDoubleBonds[i][j]));

            check_gl_error();
            glBindVertexArray(0);
          }

          if(source->drawBonds() && _renderStructures[i][j]->isVisible() && _internalBondShader._tripleBondNumberOfIndices[i][j]>0
             && _internalBondShader._numberOfTripleBonds[i][j]>0)
          {
            glBindVertexArray(_vertexTripleBondsArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_internalBondShader._tripleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT,
                                    nullptr, static_cast<GLsizei>(_internalBondShader._numberOfTripleBonds[i][j]));

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



void OpenGLInternalBondPickingShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLInternalBondPickingShader::initializeVertexArrayObject()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      glBindVertexArray(_vertexAllBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexAllBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _internalBondShader._indexAllBondsBuffer[i][j]);

      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexAllBondsInstanceBuffer[i][j]);


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
      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexSingleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _internalBondShader._indexSingleBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexSingleBondsInstanceBuffer[i][j]);

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
      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexDoubleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _internalBondShader._indexDoubleBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexDoubleBondsInstanceBuffer[i][j]);

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
      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexPartialDoubleBondsBuffer[i][j]);
      check_gl_error();


      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _internalBondShader._indexPartialDoubleBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexPartialDoubleBondsInstanceBuffer[i][j]);


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
      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexTripleBondsBuffer[i][j]);
      check_gl_error();


      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _internalBondShader._indexTripleBondsBuffer[i][j]);

      glBindBuffer(GL_ARRAY_BUFFER, _internalBondShader._vertexTripleBondsInstanceBuffer[i][j]);


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

const std::string  OpenGLInternalBondPickingShader::_vertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;
in vec4 instancePosition1;
in vec4 instancePosition2;
in int instanceTag;

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

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * (orientationMatrix * (scale * pos) + pos1);
}
)foo";

const std::string  OpenGLInternalBondPickingShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
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
