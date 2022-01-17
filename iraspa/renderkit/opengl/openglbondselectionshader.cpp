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

#include "openglbondselectionshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"

OpenGLBondSelectionShader::OpenGLBondSelectionShader(OpenGLBondShader &bondShader):
  _internalBondSelectionInstanceShader(),
  _externalBondSelectionInstanceShader(),
  _internalBondSelectionStripesShader(_internalBondSelectionInstanceShader, bondShader),
  _externalBondSelectionStripesShader(_externalBondSelectionInstanceShader, bondShader),
  _internalBondSelectionWorleyNoise3DShader(_internalBondSelectionInstanceShader, bondShader),
  _externalBondSelectionWorleyNoise3DShader(_externalBondSelectionInstanceShader, bondShader),
  _internalBondSelectionGlowShader(_internalBondSelectionInstanceShader, bondShader),
  _externalBondSelectionGlowShader(_externalBondSelectionInstanceShader, bondShader)
{

}

bool OpenGLBondSelectionShader::initializeOpenGLFunctions()
{
  _internalBondSelectionInstanceShader.initializeOpenGLFunctions();
  _externalBondSelectionInstanceShader.initializeOpenGLFunctions();
  _internalBondSelectionStripesShader.initializeOpenGLFunctions();
  _externalBondSelectionStripesShader.initializeOpenGLFunctions();
  _internalBondSelectionWorleyNoise3DShader.initializeOpenGLFunctions();
  _externalBondSelectionWorleyNoise3DShader.initializeOpenGLFunctions();
  _internalBondSelectionGlowShader.initializeOpenGLFunctions();
  _externalBondSelectionGlowShader.initializeOpenGLFunctions();

  return QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
}

void OpenGLBondSelectionShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _internalBondSelectionInstanceShader.setRenderStructures(structures);
  _externalBondSelectionInstanceShader.setRenderStructures(structures);
  _internalBondSelectionStripesShader.setRenderStructures(structures);
  _externalBondSelectionStripesShader.setRenderStructures(structures);
  _internalBondSelectionWorleyNoise3DShader.setRenderStructures(structures);
  _externalBondSelectionWorleyNoise3DShader.setRenderStructures(structures);
  _internalBondSelectionGlowShader.setRenderStructures(structures);
  _externalBondSelectionGlowShader.setRenderStructures(structures);
}


void OpenGLBondSelectionShader::paintGL(GLuint structureUniformBuffer)
{
  _internalBondSelectionStripesShader.paintGL(structureUniformBuffer);
  _externalBondSelectionStripesShader.paintGL(structureUniformBuffer);
  _internalBondSelectionWorleyNoise3DShader.paintGL(structureUniformBuffer);
  _externalBondSelectionWorleyNoise3DShader.paintGL(structureUniformBuffer);

}
void OpenGLBondSelectionShader::paintGLGlow(GLuint structureUniformBuffer)
{
  _internalBondSelectionGlowShader.paintGL(structureUniformBuffer);
  _externalBondSelectionGlowShader.paintGL(structureUniformBuffer);
}

void OpenGLBondSelectionShader::initializeVertexArrayObject()
{
  _internalBondSelectionInstanceShader.initializeVertexArrayObject();
  _externalBondSelectionInstanceShader.initializeVertexArrayObject();
  _internalBondSelectionStripesShader.initializeVertexArrayObject();
  _externalBondSelectionStripesShader.initializeVertexArrayObject();
  _internalBondSelectionWorleyNoise3DShader.initializeVertexArrayObject();
  _externalBondSelectionWorleyNoise3DShader.initializeVertexArrayObject();
  _internalBondSelectionGlowShader.initializeVertexArrayObject();
  _externalBondSelectionGlowShader.initializeVertexArrayObject();
}

void OpenGLBondSelectionShader::reloadData()
{
  _internalBondSelectionInstanceShader.reloadData();
  _externalBondSelectionInstanceShader.reloadData();
  _internalBondSelectionStripesShader.reloadData();
  _externalBondSelectionStripesShader.reloadData();
  _internalBondSelectionWorleyNoise3DShader.reloadData();
  _externalBondSelectionWorleyNoise3DShader.reloadData();
  _internalBondSelectionGlowShader.reloadData();
  _externalBondSelectionGlowShader.reloadData();
}

void OpenGLBondSelectionShader::loadShader(void)
{
  _internalBondSelectionStripesShader.loadShader();
  _externalBondSelectionStripesShader.loadShader();
  _internalBondSelectionWorleyNoise3DShader.loadShader();
  _externalBondSelectionWorleyNoise3DShader.loadShader();
  _internalBondSelectionGlowShader.loadShader();
  _externalBondSelectionGlowShader.loadShader();
}

void OpenGLBondSelectionShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_internalBondSelectionStripesShader.program(), glGetUniformBlockIndex(_internalBondSelectionStripesShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_externalBondSelectionStripesShader.program(), glGetUniformBlockIndex(_externalBondSelectionStripesShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_internalBondSelectionWorleyNoise3DShader.program(), glGetUniformBlockIndex(_internalBondSelectionWorleyNoise3DShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_externalBondSelectionWorleyNoise3DShader.program(), glGetUniformBlockIndex(_externalBondSelectionWorleyNoise3DShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_internalBondSelectionGlowShader.program(), glGetUniformBlockIndex(_internalBondSelectionGlowShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_externalBondSelectionGlowShader.program(), glGetUniformBlockIndex(_externalBondSelectionGlowShader.program(), "FrameUniformBlock"), 0);
}

void OpenGLBondSelectionShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_internalBondSelectionStripesShader.program(), glGetUniformBlockIndex(_internalBondSelectionStripesShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_externalBondSelectionStripesShader.program(), glGetUniformBlockIndex(_externalBondSelectionStripesShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_internalBondSelectionWorleyNoise3DShader.program(), glGetUniformBlockIndex(_internalBondSelectionWorleyNoise3DShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_externalBondSelectionWorleyNoise3DShader.program(), glGetUniformBlockIndex(_externalBondSelectionWorleyNoise3DShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_internalBondSelectionGlowShader.program(), glGetUniformBlockIndex(_internalBondSelectionGlowShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_externalBondSelectionGlowShader.program(), glGetUniformBlockIndex(_externalBondSelectionGlowShader.program(), "StructureUniformBlock"), 1);
}

void OpenGLBondSelectionShader::initializeLightUniforms()
{
  glUniformBlockBinding(_internalBondSelectionStripesShader.program(), glGetUniformBlockIndex(_internalBondSelectionStripesShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_externalBondSelectionStripesShader.program(), glGetUniformBlockIndex(_externalBondSelectionStripesShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_internalBondSelectionWorleyNoise3DShader.program(), glGetUniformBlockIndex(_internalBondSelectionWorleyNoise3DShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_externalBondSelectionWorleyNoise3DShader.program(), glGetUniformBlockIndex(_externalBondSelectionWorleyNoise3DShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_internalBondSelectionGlowShader.program(), glGetUniformBlockIndex(_internalBondSelectionGlowShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_externalBondSelectionGlowShader.program(), glGetUniformBlockIndex(_externalBondSelectionGlowShader.program(), "LightsUniformBlock"), 3);
}
