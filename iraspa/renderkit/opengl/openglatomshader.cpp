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

#include "openglatomshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"

OpenGLAtomShader::OpenGLAtomShader():
  _atomShader(),
  _atomOrthographicImposterShader(_atomShader),
  _atomPerspectiveImposterShader(_atomShader),
  _atomAmbientOcclusionShader(_atomShader, _atomOrthographicImposterShader)
{

}

bool OpenGLAtomShader::initializeOpenGLFunctions()
{
  _atomShader.initializeOpenGLFunctions();
  _atomOrthographicImposterShader.initializeOpenGLFunctions();
  _atomPerspectiveImposterShader.initializeOpenGLFunctions();
  _atomAmbientOcclusionShader.initializeOpenGLFunctions();

  return QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
}

void OpenGLAtomShader::invalidateCachedAmbientOcclusionTexture(std::vector<std::shared_ptr<RKRenderObject>> structure)
{
  _atomAmbientOcclusionShader.invalidateCachedAmbientOcclusionTexture(structure);
}

void OpenGLAtomShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = structures;
  _atomShader.setRenderStructures(structures);
  _atomOrthographicImposterShader.setRenderStructures(structures);
  _atomPerspectiveImposterShader.setRenderStructures(structures);
  _atomAmbientOcclusionShader.setRenderStructures(structures);

  _numberOfAtoms = 0;
}


void OpenGLAtomShader::paintGL(std::shared_ptr<RKCamera> camera, RKRenderQuality quality, GLuint structureUniformBuffer)
{
  Q_UNUSED(quality);
  if(camera->isOrthographic())
  {
    _atomOrthographicImposterShader.paintGL(_atomAmbientOcclusionShader.generatedAmbientOcclusionTextures(), structureUniformBuffer);
  }
  else
  {
    _atomPerspectiveImposterShader.paintGL(_atomAmbientOcclusionShader.generatedAmbientOcclusionTextures(), structureUniformBuffer);
  }
}

void OpenGLAtomShader::initializeVertexArrayObject()
{
  _atomShader.initializeVertexArrayObject();
  _atomOrthographicImposterShader.initializeVertexArrayObject();
  _atomPerspectiveImposterShader.initializeVertexArrayObject();
  _atomAmbientOcclusionShader.initializeVertexArrayObject();
}

void OpenGLAtomShader::reloadData()
{
  _atomShader.reloadData();
  _atomOrthographicImposterShader.reloadData();
  _atomPerspectiveImposterShader.reloadData();

  _numberOfAtoms = 0;
  for(size_t i=0;i<_atomShader._numberOfDrawnAtoms.size();i++)
  {
    for(size_t j=0;j<_atomShader._numberOfDrawnAtoms[i].size();j++)
    {
      _numberOfAtoms += _atomShader._numberOfDrawnAtoms[i][j];
    }
  }
}

void OpenGLAtomShader::reloadAmbientOcclusionData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality)
{
  _atomAmbientOcclusionShader.reloadData(dataSource, quality);
}

void OpenGLAtomShader::loadShader(void)
{
  _atomOrthographicImposterShader.loadShader();
  _atomPerspectiveImposterShader.loadShader();
  _atomAmbientOcclusionShader.loadShader();
}

void OpenGLAtomShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_atomOrthographicImposterShader.program(), glGetUniformBlockIndex(_atomOrthographicImposterShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_atomPerspectiveImposterShader.program(), glGetUniformBlockIndex(_atomPerspectiveImposterShader.program(), "FrameUniformBlock"), 0);
}

void OpenGLAtomShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_atomOrthographicImposterShader.program(), glGetUniformBlockIndex(_atomOrthographicImposterShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_atomPerspectiveImposterShader.program(), glGetUniformBlockIndex(_atomPerspectiveImposterShader.program(), "StructureUniformBlock"), 1);
}

void OpenGLAtomShader::initializeLightUniforms()
{
  glUniformBlockBinding(_atomOrthographicImposterShader.program(), glGetUniformBlockIndex(_atomOrthographicImposterShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_atomPerspectiveImposterShader.program(), glGetUniformBlockIndex(_atomPerspectiveImposterShader.program(), "LightsUniformBlock"), 3);
}
