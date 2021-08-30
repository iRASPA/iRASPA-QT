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

#include "openglglobalaxesshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"

OpenGLGlobalAxesShader::OpenGLGlobalAxesShader():
  _axesBackgroundShader(),
  _axesSystemShader(),
  _axesTextShader()
{

}

bool OpenGLGlobalAxesShader::initializeOpenGLFunctions()
{
  _axesBackgroundShader.initializeOpenGLFunctions();
  _axesSystemShader.initializeOpenGLFunctions();
  _axesTextShader.initializeOpenGLFunctions();

  return QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
}

void OpenGLGlobalAxesShader::generateBuffers()
{
  _axesBackgroundShader.generateBuffers();
  _axesSystemShader.generateBuffers();
  _axesTextShader.generateBuffers();
}

void OpenGLGlobalAxesShader::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = source;
  _axesBackgroundShader.setRenderDataSource(source);
  _axesSystemShader.setRenderDataSource(source);
  _axesTextShader.setRenderDataSource(source);
}

void OpenGLGlobalAxesShader::paintGL(int width, int height)
{
  if(std::shared_ptr<RKRenderDataSource> dataSource = _dataSource)
  {
    std::shared_ptr<RKGlobalAxes> axes = dataSource->axes();
    if(axes->position() != RKGlobalAxes::Position::none)
    {
      double minSize = std::min(width,height);
      switch(axes->position())
      {
      case RKGlobalAxes::Position::none:
          break;
        case RKGlobalAxes::Position::bottomLeft:
          glViewport(minSize * axes->borderOffsetScreenFraction(),
                     minSize * axes->borderOffsetScreenFraction(),
                     minSize * axes->sizeScreenFraction(),
                     minSize * axes->sizeScreenFraction());
        break;
      case RKGlobalAxes::Position::midLeft:
         glViewport(minSize * axes->borderOffsetScreenFraction(),
                    0.5*minSize - 0.5*minSize * axes->sizeScreenFraction(),
                    minSize * axes->sizeScreenFraction(),
                    minSize * axes->sizeScreenFraction());
        break;
      case RKGlobalAxes::Position::topLeft:
        glViewport(minSize * axes->borderOffsetScreenFraction(),
                   height - (minSize * axes->borderOffsetScreenFraction() + minSize * axes->sizeScreenFraction()),
                   minSize * axes->sizeScreenFraction(),
                   minSize * axes->sizeScreenFraction());
         break;
      case RKGlobalAxes::Position::midTop:
        glViewport(0.5*width - 0.5*minSize * axes->sizeScreenFraction(),
                   height - (minSize * axes->borderOffsetScreenFraction() + minSize * axes->sizeScreenFraction()),
                   minSize * axes->sizeScreenFraction(),
                   minSize * axes->sizeScreenFraction());
         break;
      case RKGlobalAxes::Position::topRight:
        glViewport(width - (minSize * axes->borderOffsetScreenFraction() + minSize * axes->sizeScreenFraction()),
                   height - (minSize * axes->borderOffsetScreenFraction() + minSize * axes->sizeScreenFraction()),
                   minSize * axes->sizeScreenFraction(),
                   minSize * axes->sizeScreenFraction());
        break;
      case RKGlobalAxes::Position::midRight:
        glViewport(width - (minSize * axes->borderOffsetScreenFraction() + minSize * axes->sizeScreenFraction()),
                   0.5*minSize - 0.5*minSize * axes->sizeScreenFraction(),
                   minSize * axes->sizeScreenFraction(),
                   minSize * axes->sizeScreenFraction());
         break;
      case RKGlobalAxes::Position::bottomRight:
        glViewport(width - (minSize * axes->borderOffsetScreenFraction() + minSize * axes->sizeScreenFraction()),
                   minSize * axes->borderOffsetScreenFraction(),
                   minSize * axes->sizeScreenFraction(),
                   minSize * axes->sizeScreenFraction());
        break;
      case RKGlobalAxes::Position::midBottom:
        glViewport(0.5*width - 0.5*minSize * axes->sizeScreenFraction(),
                   minSize * axes->borderOffsetScreenFraction(),
                   minSize * axes->sizeScreenFraction(),
                   minSize * axes->sizeScreenFraction());
        break;
      case RKGlobalAxes::Position::center:
        glViewport(0.5*width - 0.5*minSize * axes->sizeScreenFraction(),
                   0.5*minSize - 0.5*minSize * axes->sizeScreenFraction(),
                   minSize * axes->sizeScreenFraction(),
                   minSize * axes->sizeScreenFraction());
        break;
      }

      _axesBackgroundShader.paintGL();
      _axesSystemShader.paintGL();
      _axesTextShader.paintGL();
    }
  }
}

void OpenGLGlobalAxesShader::initializeVertexArrayObject()
{
  _axesBackgroundShader.initializeVertexArrayObject();
  _axesSystemShader.initializeVertexArrayObject();
  _axesTextShader.initializeVertexArrayObject();
}

void OpenGLGlobalAxesShader::reloadData()
{
  _axesBackgroundShader.reloadData();
  _axesSystemShader.reloadData();
  _axesTextShader.reloadData();
}


void OpenGLGlobalAxesShader::loadShader(void)
{
  _axesBackgroundShader.loadShader();
  _axesSystemShader.loadShader();
  _axesTextShader.loadShader();
}

void OpenGLGlobalAxesShader::initializeTransformUniforms()
{
  const GLchar* uniformBlockName = "FrameUniformBlock";
  GLuint index = glGetUniformBlockIndex(_axesSystemShader.program(), uniformBlockName);
  glUniformBlockBinding(_axesSystemShader.program(), index, GLuint(0));
}

void OpenGLGlobalAxesShader::initializeLightUniforms()
{
  const GLchar* uniformBlockName = "LightsUniformBlock";
  GLuint index = glGetUniformBlockIndex(_axesSystemShader.program(), uniformBlockName);
  glUniformBlockBinding(_axesSystemShader.program(), index, GLuint(3));
}

void OpenGLGlobalAxesShader::initializeGlobalAxesUniforms()
{
  const GLchar* uniformBlockName = "GlobalAxesUniformBlock";
  GLuint index = glGetUniformBlockIndex(_axesBackgroundShader.program(), uniformBlockName);
  glUniformBlockBinding(_axesBackgroundShader.program(), index, GLuint(5));
  index = glGetUniformBlockIndex(_axesSystemShader.program(), uniformBlockName);
  glUniformBlockBinding(_axesSystemShader.program(), index, GLuint(5));
  index = glGetUniformBlockIndex(_axesTextShader.program(), uniformBlockName);
  glUniformBlockBinding(_axesTextShader.program(), index, GLuint(5));
}
