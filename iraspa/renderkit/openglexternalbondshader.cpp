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
#include "glgeterror.h"
#include "cylindergeometry.h"
#include "cappedcylindergeometry.h"
#include "cubegeometry.h"

OpenGLExternalBondShader::OpenGLExternalBondShader()
{

}

void OpenGLExternalBondShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}


void OpenGLExternalBondShader::paintGL(GLuint structureUniformBuffer)
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
      glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));

      if (_renderStructures[i][j]->isUnity())
      {
        if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _allBondNumberOfIndices[i][j]>0 && _numberOfAllBonds[i][j]>0)
        {
          glBindVertexArray(_vertexAllBondsArrayObject[i][j]);
          check_gl_error();

          glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_allBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfAllBonds[i][j]));

          check_gl_error();
          glBindVertexArray(0);
        }
      }
      else
      {
        if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _singleBondNumberOfIndices[i][j]>0 && _numberOfSingleBonds[i][j]>0)
        {
          glBindVertexArray(_vertexSingleBondsArrayObject[i][j]);
          check_gl_error();

          glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_singleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfSingleBonds[i][j]));

          check_gl_error();
          glBindVertexArray(0);
        }

        if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _doubleBondNumberOfIndices[i][j]>0 && _numberOfDoubleBonds[i][j]>0)
        {
          glBindVertexArray(_vertexDoubleBondsArrayObject[i][j]);
          check_gl_error();

          glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_doubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfDoubleBonds[i][j]));

          check_gl_error();
          glBindVertexArray(0);
        }

        if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _partialDoubleBondNumberOfIndices[i][j]>0 && _numberOfPartialDoubleBonds[i][j]>0)
        {
          glBindVertexArray(_vertexPartialDoubleBondsArrayObject[i][j]);
          check_gl_error();

          glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_partialDoubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfPartialDoubleBonds[i][j]));

          check_gl_error();
          glBindVertexArray(0);
        }

        if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible() && _tripleBondNumberOfIndices[i][j]>0 && _numberOfTripleBonds[i][j]>0)
        {
          glBindVertexArray(_vertexTripleBondsArrayObject[i][j]);
          check_gl_error();

          glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_tripleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfTripleBonds[i][j]));

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


  // draw caps on bonds

  glEnable(GL_STENCIL_TEST);

  index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if(_renderStructures[i][j]->drawBonds() && _renderStructures[i][j]->isVisible())
      {
        // Disable depth and color writes
        glDepthMask(GLboolean(GL_FALSE));
        glColorMask(GLboolean(GL_FALSE), GLboolean(GL_FALSE), GLboolean(GL_FALSE), GLboolean(GL_FALSE));


        // Invert the stencil everytimes a front or back is encountered (using a mask of 0x1; and independent of
        // whether the depth-test fails or succeeds). Insides will have count 1, outsides have count 0.
        // For this to work no two cut-bonds can be 'behind' eachother. In the shader only the front-facing
        // planes of the (unit-)cell are clipped.
        glStencilFunc(GLenum(GL_ALWAYS),1,0x1);
        glStencilOp(GLenum(GL_KEEP), GLenum(GL_INVERT), GLenum(GL_INVERT));


        // Enable the clipping planes

        glEnable(GLenum(GL_CLIP_DISTANCE0));
        glEnable(GLenum(GL_CLIP_DISTANCE1));
        glEnable(GLenum(GL_CLIP_DISTANCE2));
        glEnable(GLenum(GL_CLIP_DISTANCE3));
        glEnable(GLenum(GL_CLIP_DISTANCE4));
        glEnable(GLenum(GL_CLIP_DISTANCE5));

        glUseProgram(_programStencil);

        if (_numberOfAllBonds[i][j]>0 && _allBondNumberOfIndices[i][j]>0 && _numberOfAllBonds[i][j]>0)
        {
          // Write both the front and the back planes (we use 'GL_INVERT')
          glDisable(GLenum(GL_CULL_FACE));

          if (_renderStructures[i][j]->isUnity())
          {
            glBindVertexArray(_vertexAllBondsStencilArrayObject[i][j]);
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_allBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfAllBonds[i][j]));

            glBindVertexArray(0);
          }
          else
          {
            glBindVertexArray(_vertexSingleBondsStencilArrayObject[i][j]);
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_singleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfSingleBonds[i][j]));
            glBindVertexArray(0);

            glBindVertexArray(_vertexDoubleBondsStencilArrayObject[i][j]);
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_doubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfDoubleBonds[i][j]));
            glBindVertexArray(0);

            glBindVertexArray(_vertexPartialDoubleBondsStencilArrayObject[i][j]);
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_partialDoubleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfPartialDoubleBonds[i][j]));
            glBindVertexArray(0);

            glBindVertexArray(_vertexTripleBondsStencilArrayObject[i][j]);
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_tripleBondNumberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_numberOfTripleBonds[i][j]));
            glBindVertexArray(0);
          }
        }

        glUseProgram(0);

        // Disables the clipping planes
        glDisable(GLenum(GL_CLIP_DISTANCE0));
        glDisable(GLenum(GL_CLIP_DISTANCE1));
        glDisable(GLenum(GL_CLIP_DISTANCE2));
        glDisable(GLenum(GL_CLIP_DISTANCE3));
        glDisable(GLenum(GL_CLIP_DISTANCE4));
        glDisable(GLenum(GL_CLIP_DISTANCE5));


        // Back to 'normal' operation: enable depth-and color writes and culling
        glEnable(GLenum(GL_CULL_FACE));
        glCullFace(GLenum(GL_BACK));
        glDepthMask(GLboolean(GL_TRUE));
        glColorMask(GLboolean(GL_TRUE), GLboolean(GL_TRUE), GLboolean(GL_TRUE), GLboolean(GL_TRUE));

        // Set the stencil-buffer back to zero
        glStencilOp(GLenum(GL_KEEP), GLenum(GL_ZERO), GLenum(GL_ZERO));

        // Draw plane into colour buffer where the stencil_buffer is set
        glStencilFunc(GLenum(GL_EQUAL),1,0x1);


        // Draw the 6 planes of the (unit-)cell using the stencil-buffer
        glUseProgram(_programBox);
        glBindVertexArray(_boxVertexArray);

        glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));

        glDrawElements(GLenum(GL_TRIANGLE_STRIP), 34, GLenum(GL_UNSIGNED_SHORT), nullptr);
        glBindVertexArray(0);
        glUseProgram(0);

        index += 1;
      }
    }
  }

  glDisable(GLenum(GL_STENCIL_TEST));
}

void OpenGLExternalBondShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLExternalBondShader::initializeVertexArrayObject()
{
  CappedCylinderSingleBondGeometry singleBondCylinder =  CappedCylinderSingleBondGeometry(1.0,41);
  CappedCylinderDoubleBondGeometry doubleBondCylinder =  CappedCylinderDoubleBondGeometry(1.0,41);
  CappedCylinderPartialDoubleBondGeometry partialDoubleBondCylinder =  CappedCylinderPartialDoubleBondGeometry(1.0,41);
  CappedCylinderTripleBondGeometry tripleBondCylinder =  CappedCylinderTripleBondGeometry(1.0,41);

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      std::vector<RKInPerInstanceAttributesBonds> bondInstanceData = _renderStructures[i][j]->renderExternalBonds();
      _numberOfAllBonds[i][j] = bondInstanceData.size();
      _allBondNumberOfIndices[i][j] = singleBondCylinder.indices().size();

      std::vector<RKInPerInstanceAttributesBonds> singleBondInstanceData;
      int32_t singleBondType = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(SKAsymmetricBond::SKBondType::singleBond);
      std::copy_if(bondInstanceData.begin(), bondInstanceData.end(), std::back_inserter(singleBondInstanceData),
                   [singleBondType](RKInPerInstanceAttributesBonds &i){return i.type == singleBondType;});
      _numberOfSingleBonds[i][j] = singleBondInstanceData.size();
      _singleBondNumberOfIndices[i][j] = singleBondCylinder.indices().size();

      std::vector<RKInPerInstanceAttributesBonds> doubleBondInstanceData;
      int32_t doubleBondType = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(SKAsymmetricBond::SKBondType::doubleBond);
      std::copy_if(bondInstanceData.begin(), bondInstanceData.end(), std::back_inserter(doubleBondInstanceData),
                   [doubleBondType](RKInPerInstanceAttributesBonds &i){return i.type == doubleBondType;});
      _numberOfDoubleBonds[i][j] = doubleBondInstanceData.size();
      _doubleBondNumberOfIndices[i][j] = doubleBondCylinder.indices().size();

      std::vector<RKInPerInstanceAttributesBonds> partialDoubleBondInstanceData;
      int32_t partialDoubleBondType = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(SKAsymmetricBond::SKBondType::partialDoubleBond);
      std::copy_if(bondInstanceData.begin(), bondInstanceData.end(), std::back_inserter(partialDoubleBondInstanceData),
                   [partialDoubleBondType](RKInPerInstanceAttributesBonds &i){return i.type == partialDoubleBondType;});
      _numberOfPartialDoubleBonds[i][j] = partialDoubleBondInstanceData.size();
      _partialDoubleBondNumberOfIndices[i][j] = singleBondCylinder.indices().size();

      std::vector<RKInPerInstanceAttributesBonds> tripleBondInstanceData;
      int32_t tripleBondType = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(SKAsymmetricBond::SKBondType::tripleBond);
      std::copy_if(bondInstanceData.begin(), bondInstanceData.end(), std::back_inserter(tripleBondInstanceData),
                   [tripleBondType](RKInPerInstanceAttributesBonds &i){return i.type == tripleBondType;});
      _numberOfTripleBonds[i][j] = tripleBondInstanceData.size();
      _tripleBondNumberOfIndices[i][j] = tripleBondCylinder.indices().size();

      glBindVertexArray(_vertexAllBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexAllBondsBuffer[i][j]);
      check_gl_error();

      if(singleBondCylinder.vertices().size()>0)
      {
        glBufferData(GL_ARRAY_BUFFER, singleBondCylinder.vertices().size()*sizeof(RKVertex), singleBondCylinder.vertices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexAllBondsBuffer[i][j]);
      if(singleBondCylinder.indices().size() >0)
      {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, singleBondCylinder.indices().size() * sizeof(GLshort), singleBondCylinder.indices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glBindBuffer(GL_ARRAY_BUFFER, _vertexAllBondsInstanceBuffer[i][j]);

      if(_numberOfAllBonds[i][j]>0)
      {
        glBufferData(GL_ARRAY_BUFFER, _numberOfAllBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), bondInstanceData.data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }
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

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);


      glBindVertexArray(_vertexSingleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexSingleBondsBuffer[i][j]);
      check_gl_error();

      if(singleBondCylinder.vertices().size()>0)
      {
        glBufferData(GL_ARRAY_BUFFER, singleBondCylinder.vertices().size()*sizeof(RKVertex), singleBondCylinder.vertices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexSingleBondsBuffer[i][j]);
      if(singleBondCylinder.indices().size() >0)
      {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, singleBondCylinder.indices().size() * sizeof(GLshort), singleBondCylinder.indices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glBindBuffer(GL_ARRAY_BUFFER, _vertexSingleBondsInstanceBuffer[i][j]);

      if(_numberOfSingleBonds[i][j]>0)
      {
        glBufferData(GL_ARRAY_BUFFER, _numberOfSingleBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), singleBondInstanceData.data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }
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

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);


      glBindVertexArray(_vertexDoubleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexDoubleBondsBuffer[i][j]);
      check_gl_error();

      if(doubleBondCylinder.vertices().size()>0)
      {
        glBufferData(GL_ARRAY_BUFFER, doubleBondCylinder.vertices().size()*sizeof(RKVertex), doubleBondCylinder.vertices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexDoubleBondsBuffer[i][j]);
      if(doubleBondCylinder.indices().size() >0)
      {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, doubleBondCylinder.indices().size() * sizeof(GLshort), doubleBondCylinder.indices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glBindBuffer(GL_ARRAY_BUFFER, _vertexDoubleBondsInstanceBuffer[i][j]);

      if(_numberOfDoubleBonds[i][j]>0)
      {
        glBufferData(GL_ARRAY_BUFFER, _numberOfDoubleBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), doubleBondInstanceData.data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }
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

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);

      glBindVertexArray(_vertexPartialDoubleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexPartialDoubleBondsBuffer[i][j]);
      check_gl_error();

      if(doubleBondCylinder.vertices().size()>0)
      {
        glBufferData(GL_ARRAY_BUFFER, partialDoubleBondCylinder.vertices().size()*sizeof(RKVertex), partialDoubleBondCylinder.vertices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexPartialDoubleBondsBuffer[i][j]);
      if(partialDoubleBondCylinder.indices().size() >0)
      {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, partialDoubleBondCylinder.indices().size() * sizeof(GLshort), partialDoubleBondCylinder.indices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glBindBuffer(GL_ARRAY_BUFFER, _vertexPartialDoubleBondsInstanceBuffer[i][j]);

      if(_numberOfPartialDoubleBonds[i][j]>0)
      {
        glBufferData(GL_ARRAY_BUFFER, _numberOfPartialDoubleBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), partialDoubleBondInstanceData.data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }
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

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);

      glBindVertexArray(_vertexTripleBondsArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexTripleBondsBuffer[i][j]);
      check_gl_error();

      if(tripleBondCylinder.vertices().size()>0)
      {
        glBufferData(GL_ARRAY_BUFFER, tripleBondCylinder.vertices().size()*sizeof(RKVertex), tripleBondCylinder.vertices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexTripleBondsBuffer[i][j]);
      if(tripleBondCylinder.indices().size() >0)
      {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, tripleBondCylinder.indices().size() * sizeof(GLshort), tripleBondCylinder.indices().data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }

      glBindBuffer(GL_ARRAY_BUFFER, _vertexTripleBondsInstanceBuffer[i][j]);

      if(_numberOfTripleBonds[i][j]>0)
      {
        glBufferData(GL_ARRAY_BUFFER, _numberOfTripleBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), tripleBondInstanceData.data(), GL_DYNAMIC_DRAW);
        check_gl_error();
      }
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

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_instancePositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instancePositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);






      glBindVertexArray(_vertexAllBondsStencilArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexAllBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexStencilPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexStencilNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexAllBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _vertexAllBondsInstanceBuffer[i][j]);


      glVertexAttribPointer(_instanceStencilPositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instanceStencilPositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instanceStencilPositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilPositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceStencilColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceStencilColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceStencilScaleAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexStencilPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexStencilNormalAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);

      glBindVertexArray(_vertexSingleBondsStencilArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexSingleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexStencilPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexStencilNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexSingleBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _vertexSingleBondsInstanceBuffer[i][j]);


      glVertexAttribPointer(_instanceStencilPositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instanceStencilPositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instanceStencilPositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilPositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceStencilColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceStencilColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceStencilScaleAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexStencilPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexStencilNormalAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);

      glBindVertexArray(_vertexDoubleBondsStencilArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexDoubleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexStencilPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexStencilNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexDoubleBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _vertexDoubleBondsInstanceBuffer[i][j]);


      glVertexAttribPointer(_instanceStencilPositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instanceStencilPositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instanceStencilPositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilPositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceStencilColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceStencilColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceStencilScaleAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexStencilPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexStencilNormalAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);

      glBindVertexArray(_vertexPartialDoubleBondsStencilArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexPartialDoubleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexStencilPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexStencilNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexPartialDoubleBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _vertexPartialDoubleBondsInstanceBuffer[i][j]);


      glVertexAttribPointer(_instanceStencilPositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instanceStencilPositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instanceStencilPositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilPositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceStencilColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceStencilColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceStencilScaleAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexStencilPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexStencilNormalAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);

      glBindVertexArray(_vertexTripleBondsStencilArrayObject[i][j]);
      check_gl_error();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexTripleBondsBuffer[i][j]);
      check_gl_error();

      glVertexAttribPointer(_vertexStencilPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
      glVertexAttribPointer(_vertexStencilNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,normal));
      check_gl_error();

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexTripleBondsBuffer[i][j]);


      glBindBuffer(GL_ARRAY_BUFFER, _vertexTripleBondsInstanceBuffer[i][j]);


      glVertexAttribPointer(_instanceStencilPositionFirstAtomAttributeLocation, 4, GL_FLOAT,GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position1));
      glVertexAttribPointer(_instanceStencilPositionSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, position2));
      glVertexAttribDivisor(_instanceStencilPositionFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilPositionSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilColorFirstAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color1));
      glVertexAttribPointer(_instanceStencilColorSecondAtomAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, color2));
      glVertexAttribDivisor(_instanceStencilColorFirstAtomAttributeLocation,1);
      glVertexAttribDivisor(_instanceStencilColorSecondAtomAttributeLocation,1);
      check_gl_error();

      glVertexAttribPointer(_instanceStencilScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesBonds), (void*)offsetof(RKInPerInstanceAttributesBonds, scale));
      glVertexAttribDivisor(_instanceStencilScaleAttributeLocation,1);
      check_gl_error();

      glEnableVertexAttribArray(_vertexStencilPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexStencilNormalAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilPositionSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorFirstAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilColorSecondAtomAttributeLocation);
      glEnableVertexAttribArray(_instanceStencilScaleAttributeLocation);
      check_gl_error();
      glBindVertexArray(0);
    }
  }

  if(_renderStructures.size() > 0)
  {
    CubeGeometry cube = CubeGeometry();

    glBindVertexArray(_boxVertexArray);

    glBindBuffer(GLenum(GL_ARRAY_BUFFER), _boxVertexBuffer);
    if(cube.vertices().size()>0)
    {
      glBufferData(GLenum(GL_ARRAY_BUFFER), cube.vertices().size()*sizeof(RKVertex), cube.vertices().data(), GLenum(GL_STATIC_DRAW));
      check_gl_error();
    }

    glVertexAttribPointer(_boxVertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (void*)offsetof(RKVertex, position));
    glVertexAttribPointer(_boxVertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (void*)offsetof(RKVertex, normal));

    glBindBuffer(GLenum(GL_ELEMENT_ARRAY_BUFFER), _boxIndexBuffer);
    if(cube.indices().size()>0)
    {
      glBufferData(GLenum(GL_ELEMENT_ARRAY_BUFFER), cube.indices().size() * sizeof(GLushort), cube.indices().data(), GL_DYNAMIC_DRAW);
      check_gl_error();
    }

    glEnableVertexAttribArray(_boxVertexPositionAttributeLocation);
    glEnableVertexAttribArray(_boxVertexNormalAttributeLocation);

   glBindVertexArray(0);
  }
}

void OpenGLExternalBondShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsStencilArrayObject[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsInstanceBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexAllBondsBuffer[i].data());

    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsStencilArrayObject[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsInstanceBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexSingleBondsBuffer[i].data());

    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsStencilArrayObject[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsInstanceBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexDoubleBondsBuffer[i].data());

    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsStencilArrayObject[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsInstanceBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexPartialDoubleBondsBuffer[i].data());

    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsArrayObject[i].data());
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsStencilArrayObject[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsInstanceBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsBuffer[i].data());
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexTripleBondsBuffer[i].data());
  }
}

void OpenGLExternalBondShader::deletePermanentBuffers()
{
  glDeleteVertexArrays(1, &_boxVertexArray);
  glDeleteBuffers(1, &_boxVertexBuffer);
  glDeleteBuffers(1, &_boxIndexBuffer);
}

void OpenGLExternalBondShader::generateBuffers()
{
  _numberOfAllBonds.resize(_renderStructures.size());
  _allBondNumberOfIndices.resize(_renderStructures.size());
  _vertexAllBondsArrayObject.resize(_renderStructures.size());
  _vertexAllBondsStencilArrayObject.resize(_renderStructures.size());
  _vertexAllBondsInstanceBuffer.resize(_renderStructures.size());
  _vertexAllBondsBuffer.resize(_renderStructures.size());
  _indexAllBondsBuffer.resize(_renderStructures.size());

  _numberOfSingleBonds.resize(_renderStructures.size());
  _singleBondNumberOfIndices.resize(_renderStructures.size());
  _vertexSingleBondsArrayObject.resize(_renderStructures.size());
  _vertexSingleBondsStencilArrayObject.resize(_renderStructures.size());
  _vertexSingleBondsInstanceBuffer.resize(_renderStructures.size());
  _vertexSingleBondsBuffer.resize(_renderStructures.size());
  _indexSingleBondsBuffer.resize(_renderStructures.size());

  _numberOfDoubleBonds.resize(_renderStructures.size());
  _doubleBondNumberOfIndices.resize(_renderStructures.size());
  _vertexDoubleBondsArrayObject.resize(_renderStructures.size());
  _vertexDoubleBondsStencilArrayObject.resize(_renderStructures.size());
  _vertexDoubleBondsInstanceBuffer.resize(_renderStructures.size());
  _vertexDoubleBondsBuffer.resize(_renderStructures.size());
  _indexDoubleBondsBuffer.resize(_renderStructures.size());

  _numberOfPartialDoubleBonds.resize(_renderStructures.size());
  _partialDoubleBondNumberOfIndices.resize(_renderStructures.size());
  _vertexPartialDoubleBondsArrayObject.resize(_renderStructures.size());
  _vertexPartialDoubleBondsStencilArrayObject.resize(_renderStructures.size());
  _vertexPartialDoubleBondsInstanceBuffer.resize(_renderStructures.size());
  _vertexPartialDoubleBondsBuffer.resize(_renderStructures.size());
  _indexPartialDoubleBondsBuffer.resize(_renderStructures.size());

  _numberOfTripleBonds.resize(_renderStructures.size());
  _tripleBondNumberOfIndices.resize(_renderStructures.size());
  _vertexTripleBondsArrayObject.resize(_renderStructures.size());
  _vertexTripleBondsStencilArrayObject.resize(_renderStructures.size());
  _vertexTripleBondsInstanceBuffer.resize(_renderStructures.size());
  _vertexTripleBondsBuffer.resize(_renderStructures.size());
  _indexTripleBondsBuffer.resize(_renderStructures.size());


  for(size_t i=0;i<_renderStructures.size();i++)
  {
    _numberOfAllBonds[i].resize(_renderStructures[i].size());
    _allBondNumberOfIndices[i].resize(_renderStructures[i].size());
    _vertexAllBondsArrayObject[i].resize(_renderStructures[i].size());
    _vertexAllBondsStencilArrayObject[i].resize(_renderStructures[i].size());
    _vertexAllBondsInstanceBuffer[i].resize(_renderStructures[i].size());
    _vertexAllBondsBuffer[i].resize(_renderStructures[i].size());
    _indexAllBondsBuffer[i].resize(_renderStructures[i].size());

    _numberOfSingleBonds[i].resize(_renderStructures[i].size());
    _singleBondNumberOfIndices[i].resize(_renderStructures[i].size());
    _vertexSingleBondsArrayObject[i].resize(_renderStructures[i].size());
    _vertexSingleBondsStencilArrayObject[i].resize(_renderStructures[i].size());
    _vertexSingleBondsInstanceBuffer[i].resize(_renderStructures[i].size());
    _vertexSingleBondsBuffer[i].resize(_renderStructures[i].size());
    _indexSingleBondsBuffer[i].resize(_renderStructures[i].size());

    _numberOfDoubleBonds[i].resize(_renderStructures[i].size());
    _doubleBondNumberOfIndices[i].resize(_renderStructures[i].size());
    _vertexDoubleBondsArrayObject[i].resize(_renderStructures[i].size());
    _vertexDoubleBondsStencilArrayObject[i].resize(_renderStructures[i].size());
    _vertexDoubleBondsInstanceBuffer[i].resize(_renderStructures[i].size());
    _vertexDoubleBondsBuffer[i].resize(_renderStructures[i].size());
    _indexDoubleBondsBuffer[i].resize(_renderStructures[i].size());

    _numberOfPartialDoubleBonds[i].resize(_renderStructures[i].size());
    _partialDoubleBondNumberOfIndices[i].resize(_renderStructures[i].size());
    _vertexPartialDoubleBondsArrayObject[i].resize(_renderStructures[i].size());
    _vertexPartialDoubleBondsStencilArrayObject[i].resize(_renderStructures[i].size());
    _vertexPartialDoubleBondsInstanceBuffer[i].resize(_renderStructures[i].size());
    _vertexPartialDoubleBondsBuffer[i].resize(_renderStructures[i].size());
    _indexPartialDoubleBondsBuffer[i].resize(_renderStructures[i].size());

    _numberOfTripleBonds[i].resize(_renderStructures[i].size());
    _tripleBondNumberOfIndices[i].resize(_renderStructures[i].size());
    _vertexTripleBondsArrayObject[i].resize(_renderStructures[i].size());
    _vertexTripleBondsStencilArrayObject[i].resize(_renderStructures[i].size());
    _vertexTripleBondsInstanceBuffer[i].resize(_renderStructures[i].size());
    _vertexTripleBondsBuffer[i].resize(_renderStructures[i].size());
    _indexTripleBondsBuffer[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsArrayObject[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsStencilArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsInstanceBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexAllBondsBuffer[i].data());

    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsArrayObject[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsStencilArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsInstanceBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexSingleBondsBuffer[i].data());

    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsArrayObject[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsStencilArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsInstanceBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexDoubleBondsBuffer[i].data());

    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsArrayObject[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsStencilArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsInstanceBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexPartialDoubleBondsBuffer[i].data());

    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsArrayObject[i].data());
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsStencilArrayObject[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsInstanceBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsBuffer[i].data());
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexTripleBondsBuffer[i].data());
  }


}

void OpenGLExternalBondShader::generatePermanentBuffers()
{
  glGenVertexArrays(1, &_boxVertexArray);
  glGenBuffers(1, &_boxVertexBuffer);
  glGenBuffers(1, &_boxIndexBuffer);
}


void OpenGLExternalBondShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLExternalBondShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLExternalBondShader::_fragmentShaderSource.c_str());

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

    if (_vertexNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instancePositionFirstAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition1' attribute.";
    if (_instancePositionSecondAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition2' attribute.";
    if (_instanceColorFirstAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceColor1' attribute.";
    if (_instanceColorSecondAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceColor2' attribute.";
    if (_instanceScaleAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLExternalBondShader::_vertexShaderSourceStencil.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLExternalBondShader::_fragmentShaderSourceStencil.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _programStencil = glCreateProgram();
    check_gl_error();

    glAttachShader(_programStencil, vertexShader);
    check_gl_error();
    glAttachShader(_programStencil, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_programStencil, 0, "vFragColor");

    linkProgram(_programStencil);

    _vertexStencilNormalAttributeLocation   = glGetAttribLocation(_programStencil, "vertexNormal");
    _vertexStencilPositionAttributeLocation = glGetAttribLocation(_programStencil, "vertexPosition");
    _instanceStencilPositionFirstAtomAttributeLocation = glGetAttribLocation(_programStencil, "instancePosition1");
    _instanceStencilPositionSecondAtomAttributeLocation = glGetAttribLocation(_programStencil, "instancePosition2");
    _instanceStencilColorFirstAtomAttributeLocation = glGetAttribLocation(_programStencil, "instanceColor1");
    _instanceStencilColorSecondAtomAttributeLocation = glGetAttribLocation(_programStencil, "instanceColor2");
    _instanceStencilScaleAttributeLocation = glGetAttribLocation(_programStencil, "instanceScale");

    if (_vertexStencilNormalAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexNormal' attribute.";
    if (_vertexStencilPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instanceStencilPositionFirstAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition1' attribute.";
    if (_instanceStencilPositionSecondAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition2' attribute.";
    if (_instanceStencilColorFirstAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceColor1' attribute.";
    if (_instanceStencilColorSecondAtomAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceColor2' attribute.";
    if (_instanceStencilScaleAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLExternalBondShader::_vertexShaderSourceBox.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLExternalBondShader::_fragmentShaderSourceBox.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _programBox = glCreateProgram();
    check_gl_error();

    glAttachShader(_programBox, vertexShader);
    check_gl_error();
    glAttachShader(_programBox, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_programBox, 0, "vFragColor");

    linkProgram(_programBox);

    _boxVertexPositionAttributeLocation = glGetAttribLocation(_programBox, "vertexPosition");
    _boxVertexNormalAttributeLocation   = glGetAttribLocation(_programBox, "vertexNormal");

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string  OpenGLExternalBondShader::_vertexShaderSource =
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


// Inputs from vertex shader
out VS_OUT
{
  flat vec4 color1;
  flat vec4 color2;
  smooth vec4 mix;

  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
} vs_out;


void main(void)
{
  vec3 v1,v2;

  vec4 scale = instanceScale;
  vec4 pos =  vertexPosition;

  vec4 pos1 = instancePosition1;
  vec4 pos2 = instancePosition2;

  vec3 dr = (pos1 - pos2).xyz;
  float bondLength = length(dr);

  vs_out.mix.x = clamp(structureUniforms.atomScaleFactor, 0.0, 0.7) * scale.x;
  vs_out.mix.y = vertexPosition.y;  // range 0.0..1.0
  vs_out.mix.z = 1.0 - clamp(structureUniforms.atomScaleFactor, 0.0, 0.7) * scale.z;
  vs_out.mix.w = scale.x/scale.z;


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

  vs_out.color1 = instanceColor1;
  vs_out.color2 = instanceColor2;

  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * orientationMatrix * vertexNormal).xyz;

  vec4 P =  frameUniforms.viewMatrix *  structureUniforms.modelMatrix * vec4((orientationMatrix * (scale * pos) + pos1).xyz,1.0);

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  // Calculate view vector
  vs_out.V = -P.xyz;

  vec4 vertexPos = (orientationMatrix * (scale * pos) + pos1);
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * vertexPos;

  gl_ClipDistance[0] = dot(structureUniforms.clipPlaneBack, vertexPos);
  gl_ClipDistance[1] = dot(structureUniforms.clipPlaneBottom, vertexPos);
  gl_ClipDistance[2] = dot(structureUniforms.clipPlaneLeft, vertexPos);

  gl_ClipDistance[3] = dot(structureUniforms.clipPlaneFront, vertexPos);
  gl_ClipDistance[4] = dot(structureUniforms.clipPlaneTop, vertexPos);
  gl_ClipDistance[5] = dot(structureUniforms.clipPlaneRight, vertexPos);
}
)foo";

const std::string  OpenGLExternalBondShader::_fragmentShaderSource =
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

  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
} fs_in;

void main(void)
{

  // Normalize the incoming N and L vectors
  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L);
  vec3 V = normalize(fs_in.V);

   // Calculate R locally
  vec3 R = reflect(-L, N);

  vec4 ambient = lightUniforms.lights[0].ambient * structureUniforms.bondAmbientColor;
  vec4 specular = pow(max(dot(R, V), 0.0),  lightUniforms.lights[0].shininess + structureUniforms.bondShininess) * lightUniforms.lights[0].specular * structureUniforms.bondSpecularColor;
  vec4 diffuse = vec4(max(dot(N, L), 0.0));
  float t = clamp((fs_in.mix.y - fs_in.mix.x)/(fs_in.mix.z - fs_in.mix.x),0.0,1.0);

  switch(structureUniforms.bondColorMode)
  {
    case 0:
      diffuse *= structureUniforms.bondDiffuseColor;
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
  vFragColor = vec4(hsv2rgb(hsv),1.0);
}
)foo";


const std::string  OpenGLExternalBondShader::_vertexShaderSourceStencil =
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


// Inputs from vertex shader
out VS_OUT
{
  flat vec4 color1;
  flat vec4 color2;
  smooth vec4 mix;

  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
} vs_out;

// works for both orthogonal and perspective
float frontFacing(vec4 pos0, vec4 pos1, vec4 pos2)
{
  return pos0.x*pos1.y - pos1.x*pos0.y + pos1.x*pos2.y - pos2.x*pos1.y + pos2.x*pos0.y - pos0.x*pos2.y;
}

void main(void)
{
  vec3 v1,v2;

  vec4 scale = instanceScale;
  vec4 pos =  vertexPosition;

  vec4 pos1 = instancePosition1;
  vec4 pos2 = instancePosition2;

  vec3 dr = (pos1 - pos2).xyz;
  float bondLength = length(dr);

  vs_out.mix.x = clamp(structureUniforms.atomScaleFactor, 0.0, 0.7) * scale.x;
  vs_out.mix.y = vertexPosition.y;  // range 0.0..1.0
  vs_out.mix.z = 1.0 - clamp(structureUniforms.atomScaleFactor, 0.0, 0.7) * scale.z;
  vs_out.mix.w = scale.x/scale.z;


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

  vs_out.color1 = instanceColor1;
  vs_out.color2 = instanceColor2;

  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * orientationMatrix * vertexNormal).xyz;

  vec4 P =  frameUniforms.viewMatrix *  structureUniforms.modelMatrix * vec4((orientationMatrix * (scale * pos) + pos1).xyz,1.0);

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  // Calculate view vector
  vs_out.V = -P.xyz;

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


const std::string  OpenGLExternalBondShader::_fragmentShaderSourceStencil =
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

  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
} fs_in;

void main(void)
{

  // Normalize the incoming N and L vectors
  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L);
  vec3 V = normalize(fs_in.V);

   // Calculate R locally
  vec3 R = reflect(-L, N);

  vec4 ambient = lightUniforms.lights[0].ambient * structureUniforms.bondAmbientColor;
  vec4 specular = pow(max(dot(R, V), 0.0),  lightUniforms.lights[0].shininess + structureUniforms.bondShininess) * lightUniforms.lights[0].specular * structureUniforms.bondSpecularColor;
  vec4 diffuse = vec4(max(dot(N, L), 0.0));
  float t = clamp((fs_in.mix.y - fs_in.mix.x)/(fs_in.mix.z - fs_in.mix.x),0.0,1.0);

  switch(structureUniforms.bondColorMode)
  {
    case 0:
      diffuse *= structureUniforms.bondDiffuseColor;
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
  vFragColor = vec4(hsv2rgb(hsv),1.0);
}
)foo";

const std::string  OpenGLExternalBondShader::_vertexShaderSourceBox =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;
in vec4 vertexNormal;

// Inputs from vertex shader
out VS_OUT
{
  flat vec4 ambient;
  flat vec4 diffuse;
  flat vec4 specular;
  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
} vs_out;

void main()
{
  vec4 pos;

  vs_out.ambient = lightUniforms.lights[0].ambient * structureUniforms.bondAmbientColor;
  vs_out.diffuse = lightUniforms.lights[0].diffuse * structureUniforms.bondDiffuseColor;
  vs_out.specular = lightUniforms.lights[0].specular * structureUniforms.bondSpecularColor;

  // Calculate normal in view-space
  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vertexNormal).xyz;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * structureUniforms.boxMatrix * vertexPosition;

  // Calculate light vector
  vs_out.L = (lightUniforms.lights[0].position - P*lightUniforms.lights[0].position.w).xyz;

  // Calculate view vector
  vs_out.V = -P.xyz;

  gl_Position = frameUniforms.projectionMatrix * P;
}
)foo";

const std::string  OpenGLExternalBondShader::_fragmentShaderSourceBox =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
OpenGLLightUniformBlockStringLiteral +
R"foo(
out vec4 vFragColor;

// Inputs from vertex shader
in VS_OUT
{
  flat vec4 ambient;
  flat vec4 diffuse;
  flat vec4 specular;
  smooth vec3 N;
  smooth vec3 L;
  smooth vec3 V;
} fs_in;

void main()
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
  vec4 color = vec4(ambient.xyz+diffuse.xyz,1.0);

  if (structureUniforms.atomHDR)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.atomHDRExposure);
    vLdrColor.a = 1.0;
    color= vLdrColor;
  }

  vFragColor = color;
}
)foo";
