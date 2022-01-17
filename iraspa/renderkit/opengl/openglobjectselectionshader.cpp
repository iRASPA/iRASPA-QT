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

#include "openglobjectselectionshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"

OpenGLObjectSelectionShader::OpenGLObjectSelectionShader(OpenGLObjectShader &_objectShader):
                           _instanceCrystalCylinderShader(),
                           _crystalCylinderStripesShader(_instanceCrystalCylinderShader, _objectShader._crystalCylinderObjectShader),
                           _crystalCylinderWorleyNoise3DShader(_instanceCrystalCylinderShader, _objectShader._crystalCylinderObjectShader),
                           _crystalCylinderGlowShader(_instanceCrystalCylinderShader, _objectShader._crystalCylinderObjectShader),
                           _crystalEllipsoidStripesShader(_instanceCrystalEllipsoidShader, _objectShader._crystalEllipseObjectShader),
                           _crystalEllipsoidWorleyNoise3DShader(_instanceCrystalEllipsoidShader, _objectShader._crystalEllipseObjectShader),
                           _crystalEllipsoidGlowShader(_instanceCrystalEllipsoidShader, _objectShader._crystalEllipseObjectShader),
                           _crystalPolygonalPrismStripesShader(_instanceCrystalPolygonalPrismShader, _objectShader._crystalPolygonalPrismObjectShader),
                           _crystalPolygonalPrismWorleyNoise3DShader(_instanceCrystalPolygonalPrismShader, _objectShader._crystalPolygonalPrismObjectShader),
                           _crystalPolygonalPrismGlowShader(_instanceCrystalPolygonalPrismShader, _objectShader._crystalPolygonalPrismObjectShader),
                           _cylinderStripesShader(_instanceCylinderShader, _objectShader._cylinderObjectShader),
                           _cylinderWorleyNoise3DShader(_instanceCylinderShader, _objectShader._cylinderObjectShader),
                           _cylinderGlowShader(_instanceCylinderShader, _objectShader._cylinderObjectShader),
                           _ellipsoidStripesShader(_instanceEllipsoidShader, _objectShader._ellipseObjectShader),
                           _ellipsoidWorleyNoise3DShader(_instanceEllipsoidShader, _objectShader._ellipseObjectShader),
                           _ellipsoidGlowShader(_instanceEllipsoidShader, _objectShader._ellipseObjectShader),
                           _polygonalPrismStripesShader(_instancePolygonalPrismShader, _objectShader._polygonalPrismObjectShader),
                           _polygonalPrismWorleyNoise3DShader(_instancePolygonalPrismShader, _objectShader._polygonalPrismObjectShader),
                           _polygonalPrismGlowShader(_instancePolygonalPrismShader, _objectShader._polygonalPrismObjectShader)
{

}

bool OpenGLObjectSelectionShader::initializeOpenGLFunctions()
{
  _instanceCrystalCylinderShader.initializeOpenGLFunctions();
  _instanceCrystalEllipsoidShader.initializeOpenGLFunctions();
  _instanceCrystalPolygonalPrismShader.initializeOpenGLFunctions();
  _instanceCylinderShader.initializeOpenGLFunctions();
  _instanceEllipsoidShader.initializeOpenGLFunctions();
  _instancePolygonalPrismShader.initializeOpenGLFunctions();

  _crystalCylinderStripesShader.initializeOpenGLFunctions();
  _crystalCylinderWorleyNoise3DShader.initializeOpenGLFunctions();
  _crystalCylinderGlowShader.initializeOpenGLFunctions();

  _crystalEllipsoidStripesShader.initializeOpenGLFunctions();
  _crystalEllipsoidWorleyNoise3DShader.initializeOpenGLFunctions();
  _crystalEllipsoidGlowShader.initializeOpenGLFunctions();

  _crystalPolygonalPrismStripesShader.initializeOpenGLFunctions();
  _crystalPolygonalPrismWorleyNoise3DShader.initializeOpenGLFunctions();
  _crystalPolygonalPrismGlowShader.initializeOpenGLFunctions();

  _cylinderStripesShader.initializeOpenGLFunctions();
  _cylinderWorleyNoise3DShader.initializeOpenGLFunctions();
  _cylinderGlowShader.initializeOpenGLFunctions();

  _ellipsoidStripesShader.initializeOpenGLFunctions();
  _ellipsoidWorleyNoise3DShader.initializeOpenGLFunctions();
  _ellipsoidGlowShader.initializeOpenGLFunctions();

  _polygonalPrismStripesShader.initializeOpenGLFunctions();
  _polygonalPrismWorleyNoise3DShader.initializeOpenGLFunctions();
  _polygonalPrismGlowShader.initializeOpenGLFunctions();

  return QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
}

void OpenGLObjectSelectionShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _instanceCrystalCylinderShader.setRenderStructures(structures);
  _instanceCrystalEllipsoidShader.setRenderStructures(structures);
  _instanceCrystalPolygonalPrismShader.setRenderStructures(structures);
  _instanceCylinderShader.setRenderStructures(structures);
  _instanceEllipsoidShader.setRenderStructures(structures);
  _instancePolygonalPrismShader.setRenderStructures(structures);

  _crystalCylinderStripesShader.setRenderStructures(structures);
  _crystalCylinderWorleyNoise3DShader.setRenderStructures(structures);
  _crystalCylinderGlowShader.setRenderStructures(structures);

  _crystalEllipsoidStripesShader.setRenderStructures(structures);
  _crystalEllipsoidWorleyNoise3DShader.setRenderStructures(structures);
  _crystalEllipsoidGlowShader.setRenderStructures(structures);

  _crystalPolygonalPrismStripesShader.setRenderStructures(structures);
  _crystalPolygonalPrismWorleyNoise3DShader.setRenderStructures(structures);
  _crystalPolygonalPrismGlowShader.setRenderStructures(structures);

  _cylinderStripesShader.setRenderStructures(structures);
  _cylinderWorleyNoise3DShader.setRenderStructures(structures);
  _cylinderGlowShader.setRenderStructures(structures);

  _ellipsoidStripesShader.setRenderStructures(structures);
  _ellipsoidWorleyNoise3DShader.setRenderStructures(structures);
  _ellipsoidGlowShader.setRenderStructures(structures);

  _polygonalPrismStripesShader.setRenderStructures(structures);
  _polygonalPrismWorleyNoise3DShader.setRenderStructures(structures);
  _polygonalPrismGlowShader.setRenderStructures(structures);
}


void OpenGLObjectSelectionShader::paintGL(GLuint structureUniformBuffer)
{
  _crystalCylinderStripesShader.paintGL(structureUniformBuffer);
  _crystalCylinderWorleyNoise3DShader.paintGL(structureUniformBuffer);

  _crystalEllipsoidStripesShader.paintGL(structureUniformBuffer);
  _crystalEllipsoidWorleyNoise3DShader.paintGL(structureUniformBuffer);

  _crystalPolygonalPrismStripesShader.paintGL(structureUniformBuffer);
  _crystalPolygonalPrismWorleyNoise3DShader.paintGL(structureUniformBuffer);

  _cylinderStripesShader.paintGL(structureUniformBuffer);
  _cylinderWorleyNoise3DShader.paintGL(structureUniformBuffer);

  _ellipsoidStripesShader.paintGL(structureUniformBuffer);
  _ellipsoidWorleyNoise3DShader.paintGL(structureUniformBuffer);

  _polygonalPrismStripesShader.paintGL(structureUniformBuffer);
  _polygonalPrismWorleyNoise3DShader.paintGL(structureUniformBuffer);
}

void OpenGLObjectSelectionShader::paintGLGlow(GLuint structureUniformBuffer)
{
  _crystalCylinderGlowShader.paintGL(structureUniformBuffer);
  _crystalEllipsoidGlowShader.paintGL(structureUniformBuffer);
  _crystalPolygonalPrismGlowShader.paintGL(structureUniformBuffer);

  _cylinderGlowShader.paintGL(structureUniformBuffer);
  _ellipsoidGlowShader.paintGL(structureUniformBuffer);
  _polygonalPrismGlowShader.paintGL(structureUniformBuffer);
}

void OpenGLObjectSelectionShader::initializeVertexArrayObject()
{
  _instanceCrystalCylinderShader.initializeVertexArrayObject();
  _instanceCrystalEllipsoidShader.initializeVertexArrayObject();
  _instanceCrystalPolygonalPrismShader.initializeVertexArrayObject();
  _instanceCylinderShader.initializeVertexArrayObject();
  _instanceEllipsoidShader.initializeVertexArrayObject();
  _instancePolygonalPrismShader.initializeVertexArrayObject();

  _crystalCylinderStripesShader.initializeVertexArrayObject();
  _crystalCylinderWorleyNoise3DShader.initializeVertexArrayObject();
  _crystalCylinderGlowShader.initializeVertexArrayObject();

  _crystalEllipsoidStripesShader.initializeVertexArrayObject();
  _crystalEllipsoidWorleyNoise3DShader.initializeVertexArrayObject();
  _crystalEllipsoidGlowShader.initializeVertexArrayObject();

  _crystalPolygonalPrismStripesShader.initializeVertexArrayObject();
  _crystalPolygonalPrismWorleyNoise3DShader.initializeVertexArrayObject();
  _crystalPolygonalPrismGlowShader.initializeVertexArrayObject();

  _cylinderStripesShader.initializeVertexArrayObject();
  _cylinderWorleyNoise3DShader.initializeVertexArrayObject();
  _cylinderGlowShader.initializeVertexArrayObject();

  _ellipsoidStripesShader.initializeVertexArrayObject();
  _ellipsoidWorleyNoise3DShader.initializeVertexArrayObject();
  _ellipsoidGlowShader.initializeVertexArrayObject();

  _polygonalPrismStripesShader.initializeVertexArrayObject();
  _polygonalPrismWorleyNoise3DShader.initializeVertexArrayObject();
  _polygonalPrismGlowShader.initializeVertexArrayObject();
}

void OpenGLObjectSelectionShader::reloadData()
{
  _instanceCrystalCylinderShader.reloadData();
  _instanceCrystalEllipsoidShader.reloadData();
  _instanceCrystalPolygonalPrismShader.reloadData();
  _instanceCylinderShader.reloadData();
  _instanceEllipsoidShader.reloadData();
  _instancePolygonalPrismShader.reloadData();

  _crystalCylinderStripesShader.reloadData();
  _crystalCylinderWorleyNoise3DShader.reloadData();
  _crystalCylinderGlowShader.reloadData();

  _crystalEllipsoidStripesShader.reloadData();
  _crystalEllipsoidWorleyNoise3DShader.reloadData();
  _crystalEllipsoidGlowShader.reloadData();

  _crystalPolygonalPrismStripesShader.reloadData();
  _crystalPolygonalPrismWorleyNoise3DShader.reloadData();
  _crystalPolygonalPrismGlowShader.reloadData();

  _cylinderStripesShader.reloadData();
  _cylinderWorleyNoise3DShader.reloadData();
  _cylinderGlowShader.reloadData();

  _ellipsoidStripesShader.reloadData();
  _ellipsoidWorleyNoise3DShader.reloadData();
  _ellipsoidGlowShader.reloadData();

  _polygonalPrismStripesShader.reloadData();
  _polygonalPrismWorleyNoise3DShader.reloadData();
  _polygonalPrismGlowShader.reloadData();
}

void OpenGLObjectSelectionShader::loadShader(void)
{
  _crystalCylinderStripesShader.loadShader();
  _crystalCylinderWorleyNoise3DShader.loadShader();
  _crystalCylinderGlowShader.loadShader();

  _crystalEllipsoidStripesShader.loadShader();
  _crystalEllipsoidWorleyNoise3DShader.loadShader();
  _crystalEllipsoidGlowShader.loadShader();

  _crystalPolygonalPrismStripesShader.loadShader();
  _crystalPolygonalPrismWorleyNoise3DShader.loadShader();
  _crystalPolygonalPrismGlowShader.loadShader();

  _cylinderStripesShader.loadShader();
  _cylinderWorleyNoise3DShader.loadShader();
  _cylinderGlowShader.loadShader();

  _ellipsoidStripesShader.loadShader();
  _ellipsoidWorleyNoise3DShader.loadShader();
  _ellipsoidGlowShader.loadShader();

  _polygonalPrismStripesShader.loadShader();
  _polygonalPrismWorleyNoise3DShader.loadShader();
  _polygonalPrismGlowShader.loadShader();
}

void OpenGLObjectSelectionShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_crystalCylinderStripesShader.program(), glGetUniformBlockIndex(_crystalCylinderStripesShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalCylinderWorleyNoise3DShader.program(), glGetUniformBlockIndex(_crystalCylinderWorleyNoise3DShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalCylinderGlowShader.program(), glGetUniformBlockIndex(_crystalCylinderGlowShader.program(), "FrameUniformBlock"), 0);

  glUniformBlockBinding(_crystalEllipsoidStripesShader.program(), glGetUniformBlockIndex(_crystalEllipsoidStripesShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalEllipsoidWorleyNoise3DShader.program(), glGetUniformBlockIndex(_crystalEllipsoidWorleyNoise3DShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalEllipsoidGlowShader.program(), glGetUniformBlockIndex(_crystalEllipsoidGlowShader.program(), "FrameUniformBlock"), 0);

  glUniformBlockBinding(_crystalPolygonalPrismStripesShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismStripesShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalPolygonalPrismWorleyNoise3DShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismWorleyNoise3DShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalPolygonalPrismGlowShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismGlowShader.program(), "FrameUniformBlock"), 0);

  glUniformBlockBinding(_cylinderStripesShader.program(), glGetUniformBlockIndex(_cylinderStripesShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_cylinderWorleyNoise3DShader.program(), glGetUniformBlockIndex(_cylinderWorleyNoise3DShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_cylinderGlowShader.program(), glGetUniformBlockIndex(_cylinderGlowShader.program(), "FrameUniformBlock"), 0);

  glUniformBlockBinding(_ellipsoidStripesShader.program(), glGetUniformBlockIndex(_ellipsoidStripesShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_ellipsoidWorleyNoise3DShader.program(), glGetUniformBlockIndex(_ellipsoidWorleyNoise3DShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_ellipsoidGlowShader.program(), glGetUniformBlockIndex(_ellipsoidGlowShader.program(), "FrameUniformBlock"), 0);

  glUniformBlockBinding(_polygonalPrismStripesShader.program(), glGetUniformBlockIndex(_polygonalPrismStripesShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_polygonalPrismWorleyNoise3DShader.program(), glGetUniformBlockIndex(_polygonalPrismWorleyNoise3DShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_polygonalPrismGlowShader.program(), glGetUniformBlockIndex(_polygonalPrismGlowShader.program(), "FrameUniformBlock"), 0);
}

void OpenGLObjectSelectionShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_crystalCylinderStripesShader.program(), glGetUniformBlockIndex(_crystalCylinderStripesShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalCylinderWorleyNoise3DShader.program(), glGetUniformBlockIndex(_crystalCylinderWorleyNoise3DShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalCylinderGlowShader.program(), glGetUniformBlockIndex(_crystalCylinderGlowShader.program(), "StructureUniformBlock"), 1);

  glUniformBlockBinding(_crystalEllipsoidStripesShader.program(), glGetUniformBlockIndex(_crystalEllipsoidStripesShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalEllipsoidWorleyNoise3DShader.program(), glGetUniformBlockIndex(_crystalEllipsoidWorleyNoise3DShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalEllipsoidGlowShader.program(), glGetUniformBlockIndex(_crystalEllipsoidGlowShader.program(), "StructureUniformBlock"), 1);

  glUniformBlockBinding(_crystalPolygonalPrismStripesShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismStripesShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalPolygonalPrismWorleyNoise3DShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismWorleyNoise3DShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalPolygonalPrismGlowShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismGlowShader.program(), "StructureUniformBlock"), 1);

  glUniformBlockBinding(_cylinderStripesShader.program(), glGetUniformBlockIndex(_cylinderStripesShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_cylinderWorleyNoise3DShader.program(), glGetUniformBlockIndex(_cylinderWorleyNoise3DShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_cylinderGlowShader.program(), glGetUniformBlockIndex(_cylinderGlowShader.program(), "StructureUniformBlock"), 1);

  glUniformBlockBinding(_ellipsoidStripesShader.program(), glGetUniformBlockIndex(_ellipsoidStripesShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_ellipsoidWorleyNoise3DShader.program(), glGetUniformBlockIndex(_ellipsoidWorleyNoise3DShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_ellipsoidGlowShader.program(), glGetUniformBlockIndex(_ellipsoidGlowShader.program(), "StructureUniformBlock"), 1);

  glUniformBlockBinding(_polygonalPrismStripesShader.program(), glGetUniformBlockIndex(_polygonalPrismStripesShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_polygonalPrismWorleyNoise3DShader.program(), glGetUniformBlockIndex(_polygonalPrismWorleyNoise3DShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_polygonalPrismGlowShader.program(), glGetUniformBlockIndex(_polygonalPrismGlowShader.program(), "StructureUniformBlock"), 1);
}

void OpenGLObjectSelectionShader::initializeLightUniforms()
{
  glUniformBlockBinding(_crystalCylinderStripesShader.program(), glGetUniformBlockIndex(_crystalCylinderStripesShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_crystalCylinderWorleyNoise3DShader.program(), glGetUniformBlockIndex(_crystalCylinderWorleyNoise3DShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_crystalCylinderGlowShader.program(), glGetUniformBlockIndex(_crystalCylinderGlowShader.program(), "LightsUniformBlock"), 3);

  glUniformBlockBinding(_crystalEllipsoidStripesShader.program(), glGetUniformBlockIndex(_crystalEllipsoidStripesShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_crystalEllipsoidWorleyNoise3DShader.program(), glGetUniformBlockIndex(_crystalEllipsoidWorleyNoise3DShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_crystalEllipsoidGlowShader.program(), glGetUniformBlockIndex(_crystalEllipsoidGlowShader.program(), "LightsUniformBlock"), 3);

  glUniformBlockBinding(_crystalPolygonalPrismStripesShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismStripesShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_crystalPolygonalPrismWorleyNoise3DShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismWorleyNoise3DShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_crystalPolygonalPrismGlowShader.program(), glGetUniformBlockIndex(_crystalPolygonalPrismGlowShader.program(), "LightsUniformBlock"), 3);

  glUniformBlockBinding(_cylinderStripesShader.program(), glGetUniformBlockIndex(_cylinderStripesShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_cylinderWorleyNoise3DShader.program(), glGetUniformBlockIndex(_cylinderWorleyNoise3DShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_cylinderGlowShader.program(), glGetUniformBlockIndex(_cylinderGlowShader.program(), "LightsUniformBlock"), 3);

  glUniformBlockBinding(_ellipsoidStripesShader.program(), glGetUniformBlockIndex(_ellipsoidStripesShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_ellipsoidWorleyNoise3DShader.program(), glGetUniformBlockIndex(_ellipsoidWorleyNoise3DShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_ellipsoidGlowShader.program(), glGetUniformBlockIndex(_ellipsoidGlowShader.program(), "LightsUniformBlock"), 3);

  glUniformBlockBinding(_polygonalPrismStripesShader.program(), glGetUniformBlockIndex(_polygonalPrismStripesShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_polygonalPrismWorleyNoise3DShader.program(), glGetUniformBlockIndex(_polygonalPrismWorleyNoise3DShader.program(), "LightsUniformBlock"), 3);
  glUniformBlockBinding(_polygonalPrismGlowShader.program(), glGetUniformBlockIndex(_polygonalPrismGlowShader.program(), "LightsUniformBlock"), 3);
}
