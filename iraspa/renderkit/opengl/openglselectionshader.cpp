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

#include "openglselectionshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"


OpenGLSelectionShader::OpenGLSelectionShader([[maybe_unused]] OpenGLAtomShader &atomShader, OpenGLBondShader &bondShader, OpenGLObjectShader &objectShader):
                                             _atomSelectionShader(), _bondSelectionShader(bondShader), _objectSelectionShader(objectShader)
{

}

void OpenGLSelectionShader::initializeOpenGLFunctions()
{
  _atomSelectionShader.initializeOpenGLFunctions();
  _bondSelectionShader.initializeOpenGLFunctions();
  _objectSelectionShader.initializeOpenGLFunctions();
}

void OpenGLSelectionShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _atomSelectionShader.setRenderStructures(structures);
  _bondSelectionShader.setRenderStructures(structures);
  _objectSelectionShader.setRenderStructures(structures);
}


void OpenGLSelectionShader::paintGL(std::shared_ptr<RKCamera> camera, RKRenderQuality quality, GLuint structureUniformBuffer)
{
  _atomSelectionShader.paintGL(camera, quality, structureUniformBuffer);
  _bondSelectionShader.paintGL(structureUniformBuffer);
  _objectSelectionShader.paintGL(structureUniformBuffer);
}

void OpenGLSelectionShader::paintGLGlow(GLuint structureUniformBuffer)
{
  _atomSelectionShader.paintGLGlow(structureUniformBuffer);
  _bondSelectionShader.paintGLGlow(structureUniformBuffer);
  _objectSelectionShader.paintGLGlow(structureUniformBuffer);
}


void OpenGLSelectionShader::initializeVertexArrayObject()
{
  _atomSelectionShader.initializeVertexArrayObject();
  _bondSelectionShader.initializeVertexArrayObject();
  _objectSelectionShader.initializeVertexArrayObject();
}

void OpenGLSelectionShader::reloadData()
{
  _atomSelectionShader.reloadData();
  _bondSelectionShader.reloadData();
  _objectSelectionShader.reloadData();
}

void OpenGLSelectionShader::loadShader(void)
{
  _atomSelectionShader.loadShader();
  _bondSelectionShader.loadShader();
  _objectSelectionShader.loadShader();
}

void OpenGLSelectionShader::initializeTransformUniforms()
{
  _atomSelectionShader.initializeTransformUniforms();
  _bondSelectionShader.initializeTransformUniforms();
  _objectSelectionShader.initializeTransformUniforms();
}

void OpenGLSelectionShader::initializeStructureUniforms()
{
  _atomSelectionShader.initializeStructureUniforms();
  _bondSelectionShader.initializeStructureUniforms();
  _objectSelectionShader.initializeStructureUniforms();
}

void OpenGLSelectionShader::initializeLightUniforms()
{
  _atomSelectionShader.initializeLightUniforms();
  _bondSelectionShader.initializeLightUniforms();
  _objectSelectionShader.initializeLightUniforms();
}
