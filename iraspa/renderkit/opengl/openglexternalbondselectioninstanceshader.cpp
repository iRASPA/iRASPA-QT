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

#include "openglexternalbondselectioninstanceshader.h"
#include "glgeterror.h"

OpenGLExternalBondSelectionInstanceShader::OpenGLExternalBondSelectionInstanceShader()
{

}

void OpenGLExternalBondSelectionInstanceShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLExternalBondSelectionInstanceShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsInstanceBuffer[i].data());

    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsInstanceBuffer[i].data());

    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsInstanceBuffer[i].data());

    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsInstanceBuffer[i].data());

    glDeleteBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsInstanceBuffer[i].data());
  }
}

void OpenGLExternalBondSelectionInstanceShader::generateBuffers()
{
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
    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexAllBondsInstanceBuffer[i].data());

    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexSingleBondsInstanceBuffer[i].data());

    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexDoubleBondsInstanceBuffer[i].data());

    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexPartialDoubleBondsInstanceBuffer[i].data());

    glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexTripleBondsInstanceBuffer[i].data());
  }
}


void OpenGLExternalBondSelectionInstanceShader::initializeVertexArrayObject()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderAtomicStructureSource* source = dynamic_cast<RKRenderAtomicStructureSource*>(_renderStructures[i][j].get()))
      {
        std::vector<RKInPerInstanceAttributesBonds> bondInstanceData = source->renderSelectedExternalBonds();
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

        glBindBuffer(GL_ARRAY_BUFFER, _vertexAllBondsInstanceBuffer[i][j]);
        if(_numberOfAllBonds[i][j]>0)
        {
          glBufferData(GL_ARRAY_BUFFER, _numberOfAllBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), bondInstanceData.data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }

        glBindBuffer(GL_ARRAY_BUFFER, _vertexSingleBondsInstanceBuffer[i][j]);
        if(_numberOfSingleBonds[i][j]>0)
        {
          glBufferData(GL_ARRAY_BUFFER, _numberOfSingleBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), singleBondInstanceData.data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }

        glBindBuffer(GL_ARRAY_BUFFER, _vertexDoubleBondsInstanceBuffer[i][j]);
        if(_numberOfDoubleBonds[i][j]>0)
        {
          glBufferData(GL_ARRAY_BUFFER, _numberOfDoubleBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), doubleBondInstanceData.data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }

        glBindBuffer(GL_ARRAY_BUFFER, _vertexPartialDoubleBondsInstanceBuffer[i][j]);
        if(_numberOfPartialDoubleBonds[i][j]>0)
        {
          glBufferData(GL_ARRAY_BUFFER, _numberOfPartialDoubleBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), partialDoubleBondInstanceData.data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }

        glBindBuffer(GL_ARRAY_BUFFER, _vertexTripleBondsInstanceBuffer[i][j]);
        if(_numberOfTripleBonds[i][j]>0)
        {
          glBufferData(GL_ARRAY_BUFFER, _numberOfTripleBonds[i][j]*sizeof(RKInPerInstanceAttributesBonds), tripleBondInstanceData.data(), GL_DYNAMIC_DRAW);
          check_gl_error();
        }
      }
    }
  }
}

void OpenGLExternalBondSelectionInstanceShader::reloadData()
{
  initializeVertexArrayObject();
}


