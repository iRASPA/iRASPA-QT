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

#include "openglcylinderselectioninstanceshader.h"
#include "glgeterror.h"

OpenGLCylinderSelectionInstanceShader::OpenGLCylinderSelectionInstanceShader()
{

}

void OpenGLCylinderSelectionInstanceShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}

void OpenGLCylinderSelectionInstanceShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glDeleteBuffers(size, _instancePositionBuffer[i].data());
  }
}

void OpenGLCylinderSelectionInstanceShader::generateBuffers()
{
  _numberOfDrawnAtoms.resize(_renderStructures.size());
  _instancePositionBuffer.resize(_renderStructures.size());


  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _numberOfDrawnAtoms[i].resize(_renderStructures[i].size());
    _instancePositionBuffer[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glGenBuffers(size, _instancePositionBuffer[i].data());
  }
}


void OpenGLCylinderSelectionInstanceShader::initializeVertexArrayObject()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderPrimitiveCylinderObjectsSource* object = dynamic_cast<RKRenderPrimitiveCylinderObjectsSource*>(_renderStructures[i][j].get()))
      {
        std::vector<RKInPerInstanceAttributesAtoms> atomData = object->renderSelectedPrimitiveCylinderObjects();
        _numberOfDrawnAtoms[i][j] = atomData.size();

        glBindBuffer(GL_ARRAY_BUFFER, _instancePositionBuffer[i][j]);
        check_gl_error();
        if(_numberOfDrawnAtoms[i][j]>0)
        {
          glBufferData(GL_ARRAY_BUFFER, _numberOfDrawnAtoms[i][j]*sizeof(RKInPerInstanceAttributesAtoms), atomData.data(), GL_DYNAMIC_DRAW);
        }
        check_gl_error();
      }
    }
  }
}

void OpenGLCylinderSelectionInstanceShader::reloadData()
{
  initializeVertexArrayObject();
}
