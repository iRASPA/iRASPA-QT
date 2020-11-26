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

#include "openglatomselectionshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"

OpenGLAtomSelectionShader::OpenGLAtomSelectionShader():
  _atomSelectionWorleyNoise3DShader(),
  _atomSelectionWorleyNoise3DOrthographicImposterShader(_atomSelectionWorleyNoise3DShader),
  _atomSelectionWorleyNoise3DPerspectiveImposterShader(_atomSelectionWorleyNoise3DShader),
  _atomSelectionStripesShader(),
  _atomSelectionStripesOrthographicImposterShader(_atomSelectionWorleyNoise3DShader),
  _atomSelectionStripesPerspectiveImposterShader(_atomSelectionWorleyNoise3DShader),
  _atomSelectionGlowShader(_atomSelectionWorleyNoise3DShader)
{

}

bool OpenGLAtomSelectionShader::initializeOpenGLFunctions()
{
  _atomSelectionWorleyNoise3DShader.initializeOpenGLFunctions();
  _atomSelectionWorleyNoise3DOrthographicImposterShader.initializeOpenGLFunctions();
  _atomSelectionWorleyNoise3DPerspectiveImposterShader.initializeOpenGLFunctions();
  _atomSelectionStripesShader.initializeOpenGLFunctions();
  _atomSelectionStripesOrthographicImposterShader.initializeOpenGLFunctions();
  _atomSelectionStripesPerspectiveImposterShader.initializeOpenGLFunctions();
  _atomSelectionGlowShader.initializeOpenGLFunctions();

  return QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
}

void OpenGLAtomSelectionShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  _atomSelectionWorleyNoise3DShader.setRenderStructures(structures);
  _atomSelectionWorleyNoise3DOrthographicImposterShader.setRenderStructures(structures);
  _atomSelectionWorleyNoise3DPerspectiveImposterShader.setRenderStructures(structures);
  _atomSelectionStripesShader.setRenderStructures(structures);
  _atomSelectionStripesOrthographicImposterShader.setRenderStructures(structures);
  _atomSelectionStripesPerspectiveImposterShader.setRenderStructures(structures);
  _atomSelectionGlowShader.setRenderStructures(structures);
}


void OpenGLAtomSelectionShader::paintGL(std::shared_ptr<RKCamera> camera, RKRenderQuality quality, GLuint structureUniformBuffer)
{
  if(quality == RKRenderQuality:: high || quality == RKRenderQuality:: picture)
  {
    _atomSelectionWorleyNoise3DShader.paintGL(structureUniformBuffer);
    _atomSelectionStripesShader.paintGL(structureUniformBuffer);
  }
  else
  {
    if(camera->isOrthographic())
    {
      _atomSelectionStripesOrthographicImposterShader.paintGL(structureUniformBuffer);
      _atomSelectionWorleyNoise3DOrthographicImposterShader.paintGL(structureUniformBuffer);
    }
    else
    {
      _atomSelectionStripesPerspectiveImposterShader.paintGL(structureUniformBuffer);
      _atomSelectionWorleyNoise3DPerspectiveImposterShader.paintGL(structureUniformBuffer);
    }
  }

}
void OpenGLAtomSelectionShader::paintGLGlow(GLuint structureUniformBuffer)
{
  _atomSelectionGlowShader.paintGL(structureUniformBuffer);
}

void OpenGLAtomSelectionShader::initializeVertexArrayObject()
{
  _atomSelectionWorleyNoise3DShader.initializeVertexArrayObject();
  _atomSelectionWorleyNoise3DOrthographicImposterShader.initializeVertexArrayObject();
  _atomSelectionWorleyNoise3DPerspectiveImposterShader.initializeVertexArrayObject();
  _atomSelectionStripesShader.initializeVertexArrayObject();
  _atomSelectionStripesOrthographicImposterShader.initializeVertexArrayObject();
  _atomSelectionStripesPerspectiveImposterShader.initializeVertexArrayObject();
  _atomSelectionGlowShader.initializeVertexArrayObject();
}

void OpenGLAtomSelectionShader::reloadData()
{
  _atomSelectionWorleyNoise3DShader.reloadData();
  _atomSelectionWorleyNoise3DOrthographicImposterShader.reloadData();
  _atomSelectionWorleyNoise3DPerspectiveImposterShader.reloadData();
  _atomSelectionStripesShader.reloadData();
  _atomSelectionStripesOrthographicImposterShader.reloadData();
  _atomSelectionStripesPerspectiveImposterShader.reloadData();
  _atomSelectionGlowShader.reloadData();
}

void OpenGLAtomSelectionShader::loadShader(void)
{
  _atomSelectionWorleyNoise3DShader.loadShader();
  _atomSelectionWorleyNoise3DOrthographicImposterShader.loadShader();
  _atomSelectionWorleyNoise3DPerspectiveImposterShader.loadShader();
  _atomSelectionStripesShader.loadShader();
  _atomSelectionStripesOrthographicImposterShader.loadShader();
  _atomSelectionStripesPerspectiveImposterShader.loadShader();
  _atomSelectionGlowShader.loadShader();
}

void OpenGLAtomSelectionShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_atomSelectionWorleyNoise3DShader.program(), glGetUniformBlockIndex(_atomSelectionWorleyNoise3DShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_atomSelectionWorleyNoise3DOrthographicImposterShader.program(), glGetUniformBlockIndex(_atomSelectionWorleyNoise3DOrthographicImposterShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_atomSelectionWorleyNoise3DPerspectiveImposterShader.program(), glGetUniformBlockIndex(_atomSelectionWorleyNoise3DPerspectiveImposterShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_atomSelectionStripesShader.program(), glGetUniformBlockIndex(_atomSelectionStripesShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_atomSelectionStripesOrthographicImposterShader.program(), glGetUniformBlockIndex(_atomSelectionStripesOrthographicImposterShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_atomSelectionStripesPerspectiveImposterShader.program(), glGetUniformBlockIndex(_atomSelectionStripesPerspectiveImposterShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_atomSelectionGlowShader.program(), glGetUniformBlockIndex(_atomSelectionGlowShader.program(), "FrameUniformBlock"), 0);
}

void OpenGLAtomSelectionShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_atomSelectionWorleyNoise3DShader.program(), glGetUniformBlockIndex(_atomSelectionWorleyNoise3DShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_atomSelectionWorleyNoise3DOrthographicImposterShader.program(), glGetUniformBlockIndex(_atomSelectionWorleyNoise3DOrthographicImposterShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_atomSelectionWorleyNoise3DPerspectiveImposterShader.program(), glGetUniformBlockIndex(_atomSelectionWorleyNoise3DPerspectiveImposterShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_atomSelectionStripesShader.program(), glGetUniformBlockIndex(_atomSelectionStripesShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_atomSelectionStripesOrthographicImposterShader.program(), glGetUniformBlockIndex(_atomSelectionStripesOrthographicImposterShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_atomSelectionStripesPerspectiveImposterShader.program(), glGetUniformBlockIndex(_atomSelectionStripesPerspectiveImposterShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_atomSelectionGlowShader.program(), glGetUniformBlockIndex(_atomSelectionGlowShader.program(), "StructureUniformBlock"), 1);
}

void OpenGLAtomSelectionShader::initializeLightUniforms()
{
  glUniformBlockBinding(_atomSelectionWorleyNoise3DShader.program(), glGetUniformBlockIndex(_atomSelectionWorleyNoise3DShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_atomSelectionWorleyNoise3DOrthographicImposterShader.program(), glGetUniformBlockIndex(_atomSelectionWorleyNoise3DOrthographicImposterShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_atomSelectionWorleyNoise3DPerspectiveImposterShader.program(), glGetUniformBlockIndex(_atomSelectionWorleyNoise3DPerspectiveImposterShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_atomSelectionStripesShader.program(), glGetUniformBlockIndex(_atomSelectionStripesShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_atomSelectionStripesOrthographicImposterShader.program(), glGetUniformBlockIndex(_atomSelectionStripesOrthographicImposterShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_atomSelectionStripesPerspectiveImposterShader.program(), glGetUniformBlockIndex(_atomSelectionStripesPerspectiveImposterShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_atomSelectionGlowShader.program(), glGetUniformBlockIndex(_atomSelectionGlowShader.program(), "LightsUniformBlock"), 3);
}
