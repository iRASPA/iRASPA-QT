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

#include "openglbondshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"

OpenGLBondShader::OpenGLBondShader():
  _internalBondShader(),
  _externalBondShader()
{

}

bool OpenGLBondShader::initializeOpenGLFunctions()
{
  _internalBondShader.initializeOpenGLFunctions();
  _externalBondShader.initializeOpenGLFunctions();

  return QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
}

void OpenGLBondShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _internalBondShader.setRenderStructures(structures);
  _externalBondShader.setRenderStructures(structures);
}

void OpenGLBondShader::deletePermanentBuffers()
{
  _externalBondShader.deletePermanentBuffers();
}


void OpenGLBondShader::generatePermanentBuffers()
{
  _externalBondShader.generatePermanentBuffers();
}

void OpenGLBondShader::paintGL(GLuint structureUniformBuffer)
{
  _internalBondShader.paintGL(structureUniformBuffer);
  _externalBondShader.paintGL(structureUniformBuffer);
}

void OpenGLBondShader::initializeVertexArrayObject()
{
  _internalBondShader.initializeVertexArrayObject();
  _externalBondShader.initializeVertexArrayObject();
}

void OpenGLBondShader::reloadData()
{
  _internalBondShader.reloadData();
  _externalBondShader.reloadData();
}


void OpenGLBondShader::loadShader(void)
{
  _internalBondShader.loadShader();
  _externalBondShader.loadShader();
}

void OpenGLBondShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_internalBondShader.program(), glGetUniformBlockIndex(_internalBondShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_externalBondShader.program(), glGetUniformBlockIndex(_externalBondShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_externalBondShader.stencilProgram(), glGetUniformBlockIndex(_externalBondShader.stencilProgram(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_externalBondShader.boxProgram(), glGetUniformBlockIndex(_externalBondShader.boxProgram(), "FrameUniformBlock"), 0);
}

void OpenGLBondShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_internalBondShader.program(), glGetUniformBlockIndex(_internalBondShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_externalBondShader.program(), glGetUniformBlockIndex(_externalBondShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_externalBondShader.stencilProgram(), glGetUniformBlockIndex(_externalBondShader.stencilProgram(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_externalBondShader.boxProgram(), glGetUniformBlockIndex(_externalBondShader.boxProgram(), "StructureUniformBlock"), 1);
}

void OpenGLBondShader::initializeLightUniforms()
{
  glUniformBlockBinding(_internalBondShader.program(), glGetUniformBlockIndex(_internalBondShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_externalBondShader.program(), glGetUniformBlockIndex(_externalBondShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_externalBondShader.stencilProgram(), glGetUniformBlockIndex(_externalBondShader.stencilProgram(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_externalBondShader.boxProgram(), glGetUniformBlockIndex(_externalBondShader.boxProgram(), "LightsUniformBlock"), 3);
}
