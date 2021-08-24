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

#include "openglboundingboxshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"

OpenGLBoundingBoxShader::OpenGLBoundingBoxShader():
  _boundingBoxSphereShader(),
  _boundingBoxCylinderShader()
{

}

bool OpenGLBoundingBoxShader::initializeOpenGLFunctions()
{
  _boundingBoxSphereShader.initializeOpenGLFunctions();
  _boundingBoxCylinderShader.initializeOpenGLFunctions();

  return QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
}

void OpenGLBoundingBoxShader::generateBuffers()
{
  _boundingBoxSphereShader.generateBuffers();
  _boundingBoxCylinderShader.generateBuffers();
}

void OpenGLBoundingBoxShader::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _boundingBoxSphereShader.setRenderDataSource(source);
  _boundingBoxCylinderShader.setRenderDataSource(source);
}

void OpenGLBoundingBoxShader::paintGL()
{
  _boundingBoxSphereShader.paintGL();
  _boundingBoxCylinderShader.paintGL();
}

void OpenGLBoundingBoxShader::initializeVertexArrayObject()
{
  _boundingBoxSphereShader.initializeVertexArrayObject();
  _boundingBoxCylinderShader.initializeVertexArrayObject();
}

void OpenGLBoundingBoxShader::reloadData()
{
  _boundingBoxSphereShader.reloadData();
  _boundingBoxCylinderShader.reloadData();
}


void OpenGLBoundingBoxShader::loadShader(void)
{
  _boundingBoxSphereShader.loadShader();
  _boundingBoxCylinderShader.loadShader();
}

void OpenGLBoundingBoxShader::initializeTransformUniforms()
{
  const GLchar* uniformBlockName = "FrameUniformBlock";
  GLuint program = _boundingBoxCylinderShader.program();
  GLuint index = glGetUniformBlockIndex(_boundingBoxCylinderShader.program(), uniformBlockName);
  glUniformBlockBinding(program, index, GLuint(0));
  glUniformBlockBinding(program,index, GLuint(0));
}


void OpenGLBoundingBoxShader::initializeLightUniforms()
{
  const GLchar* uniformBlockName = "LightsUniformBlock";
  GLuint program = _boundingBoxCylinderShader.program();
  GLuint index = glGetUniformBlockIndex(_boundingBoxCylinderShader.program(), uniformBlockName);
  glUniformBlockBinding(program, index, GLuint(3));
  glUniformBlockBinding(program, index, GLuint(3));

}

