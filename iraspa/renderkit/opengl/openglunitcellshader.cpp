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

#include "openglunitcellshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"

OpenGLUnitCellShader::OpenGLUnitCellShader():
  _unitCellSphereShader(),
  _unitCellCylinderShader()
{

}

bool OpenGLUnitCellShader::initializeOpenGLFunctions()
{
  _unitCellSphereShader.initializeOpenGLFunctions();
  _unitCellCylinderShader.initializeOpenGLFunctions();

  return QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
}

void OpenGLUnitCellShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _unitCellSphereShader.setRenderStructures(structures);
  _unitCellCylinderShader.setRenderStructures(structures);
}

void OpenGLUnitCellShader::deleteBuffers()
{
  _unitCellSphereShader.deleteBuffers();
  _unitCellCylinderShader.deleteBuffers();
}

void OpenGLUnitCellShader::generateBuffers()
{
  _unitCellSphereShader.generateBuffers();
  _unitCellCylinderShader.generateBuffers();
}


void OpenGLUnitCellShader::paintGL(GLuint structureUniformBuffer)
{
  _unitCellSphereShader.paintGL(structureUniformBuffer);
  _unitCellCylinderShader.paintGL(structureUniformBuffer);
}

void OpenGLUnitCellShader::initializeVertexArrayObject()
{
  _unitCellSphereShader.initializeVertexArrayObject();
  _unitCellCylinderShader.initializeVertexArrayObject();
}

void OpenGLUnitCellShader::reloadData()
{
  _unitCellSphereShader.reloadData();
  _unitCellCylinderShader.reloadData();
}


void OpenGLUnitCellShader::loadShader(void)
{
  _unitCellSphereShader.loadShader();
  _unitCellCylinderShader.loadShader();
}

void OpenGLUnitCellShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_unitCellSphereShader.program(), glGetUniformBlockIndex(_unitCellSphereShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_unitCellCylinderShader.program(), glGetUniformBlockIndex(_unitCellCylinderShader.program(), "FrameUniformBlock"), 0);
}

void OpenGLUnitCellShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_unitCellSphereShader.program(), glGetUniformBlockIndex(_unitCellSphereShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_unitCellCylinderShader.program(), glGetUniformBlockIndex(_unitCellCylinderShader.program(), "StructureUniformBlock"), 1);
}

void OpenGLUnitCellShader::initializeLightUniforms()
{
  glUniformBlockBinding(_unitCellSphereShader.program(), glGetUniformBlockIndex(_unitCellSphereShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_unitCellCylinderShader.program(), glGetUniformBlockIndex(_unitCellCylinderShader.program(), "LightsUniformBlock"), 3);
}
