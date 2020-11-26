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

#include "openglobjectshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"

OpenGLObjectShader::OpenGLObjectShader():
  _crystalEllipseObjectShader(),
  _crystalCylinderObjectShader(),
  _crystalPolygonalPrismObjectShader(),
  _ellipseObjectShader(),
  _cylinderObjectShader(),
  _polygonalPrismObjectShader()
{

}

bool OpenGLObjectShader::initializeOpenGLFunctions()
{
  _crystalEllipseObjectShader.initializeOpenGLFunctions();
  _crystalCylinderObjectShader.initializeOpenGLFunctions();
  _crystalPolygonalPrismObjectShader.initializeOpenGLFunctions();
  _ellipseObjectShader.initializeOpenGLFunctions();
  _cylinderObjectShader.initializeOpenGLFunctions();
  _polygonalPrismObjectShader.initializeOpenGLFunctions();

  return QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
}

void OpenGLObjectShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  _crystalEllipseObjectShader.setRenderStructures(structures);
  _crystalCylinderObjectShader.setRenderStructures(structures);
  _crystalPolygonalPrismObjectShader.setRenderStructures(structures);
  _ellipseObjectShader.setRenderStructures(structures);
  _cylinderObjectShader.setRenderStructures(structures);
  _polygonalPrismObjectShader.setRenderStructures(structures);
}

void OpenGLObjectShader::paintGL(GLuint structureUniformBuffer)
{
  _crystalEllipseObjectShader.paintGLOpaque(structureUniformBuffer);
  _crystalCylinderObjectShader.paintGLOpaque(structureUniformBuffer);
  _crystalPolygonalPrismObjectShader.paintGLOpaque(structureUniformBuffer);
  _ellipseObjectShader.paintGLOpaque(structureUniformBuffer);
  _cylinderObjectShader.paintGLOpaque(structureUniformBuffer);
  _polygonalPrismObjectShader.paintGLOpaque(structureUniformBuffer);
}

void OpenGLObjectShader::paintGLTransparent(GLuint structureUniformBuffer)
{
  _crystalEllipseObjectShader.paintGLTransparent(structureUniformBuffer);
  _crystalCylinderObjectShader.paintGLTransparent(structureUniformBuffer);
  _crystalPolygonalPrismObjectShader.paintGLTransparent(structureUniformBuffer);
  _ellipseObjectShader.paintGLTransparent(structureUniformBuffer);
  _cylinderObjectShader.paintGLTransparent(structureUniformBuffer);
  _polygonalPrismObjectShader.paintGLTransparent(structureUniformBuffer);
}


void OpenGLObjectShader::initializeVertexArrayObject()
{
  _crystalEllipseObjectShader.initializeVertexArrayObject();
  _crystalCylinderObjectShader.initializeVertexArrayObject();
  _crystalPolygonalPrismObjectShader.initializeVertexArrayObject();
  _ellipseObjectShader.initializeVertexArrayObject();
  _cylinderObjectShader.initializeVertexArrayObject();
  _polygonalPrismObjectShader.initializeVertexArrayObject();
}

void OpenGLObjectShader::reloadData()
{
  _crystalEllipseObjectShader.reloadData();
  _crystalCylinderObjectShader.reloadData();
  _crystalPolygonalPrismObjectShader.reloadData();
  _ellipseObjectShader.reloadData();
  _cylinderObjectShader.reloadData();
  _polygonalPrismObjectShader.reloadData();
}

void OpenGLObjectShader::loadShader(void)
{
  _crystalEllipseObjectShader.loadShader();
  _crystalCylinderObjectShader.loadShader();
  _crystalPolygonalPrismObjectShader.loadShader();
  _ellipseObjectShader.loadShader();
  _cylinderObjectShader.loadShader();
  _polygonalPrismObjectShader.loadShader();
}

void OpenGLObjectShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_crystalEllipseObjectShader.program(), glGetUniformBlockIndex(_crystalEllipseObjectShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalCylinderObjectShader.program(), glGetUniformBlockIndex(_crystalCylinderObjectShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalPolygonalPrismObjectShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismObjectShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_ellipseObjectShader.program(), glGetUniformBlockIndex(_ellipseObjectShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_cylinderObjectShader.program(), glGetUniformBlockIndex(_cylinderObjectShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_polygonalPrismObjectShader.program(), glGetUniformBlockIndex(_polygonalPrismObjectShader.program(), "FrameUniformBlock"), 0);
}

void OpenGLObjectShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_crystalEllipseObjectShader.program(), glGetUniformBlockIndex(_crystalEllipseObjectShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalCylinderObjectShader.program(), glGetUniformBlockIndex(_crystalCylinderObjectShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalPolygonalPrismObjectShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismObjectShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_ellipseObjectShader.program(), glGetUniformBlockIndex(_ellipseObjectShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_cylinderObjectShader.program(), glGetUniformBlockIndex(_cylinderObjectShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_polygonalPrismObjectShader.program(), glGetUniformBlockIndex(_polygonalPrismObjectShader.program(), "StructureUniformBlock"), 1);
}

void OpenGLObjectShader::initializeLightUniforms()
{
  glUniformBlockBinding(_crystalEllipseObjectShader.program(), glGetUniformBlockIndex(_crystalEllipseObjectShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_crystalCylinderObjectShader.program(), glGetUniformBlockIndex(_crystalCylinderObjectShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_crystalPolygonalPrismObjectShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismObjectShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_ellipseObjectShader.program(), glGetUniformBlockIndex(_ellipseObjectShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_cylinderObjectShader.program(), glGetUniformBlockIndex(_cylinderObjectShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_polygonalPrismObjectShader.program(), glGetUniformBlockIndex(_polygonalPrismObjectShader.program(), "LightsUniformBlock"), 3);
}
