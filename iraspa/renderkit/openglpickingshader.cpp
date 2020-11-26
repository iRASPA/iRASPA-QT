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

#include "openglpickingshader.h"
#include "openglatomshader.h"
#include "openglatomorthographicimpostershader.h"

OpenGLPickingShader::OpenGLPickingShader(OpenGLAtomShader &atomShader, OpenGLBondShader &bondShader, OpenGLObjectShader &objectShader):
                                         _atomPickingShader(atomShader),
                                         _internalBondPickingShader(bondShader),
                                         _externalBondPickingShader(bondShader),
                                         _crystalCylinderPickingShader(objectShader),
                                         _crystalEllipsePickingShader(objectShader),
                                         _crystalPolyogonalPrismPickingShader(objectShader),
                                         _cylinderPickingShader(objectShader),
                                         _ellipsePickingShader(objectShader),
                                         _polygonalPrismPickingShader(objectShader)
{
}

void OpenGLPickingShader::initializeEmbeddedOpenGLFunctions()
{
  initializeOpenGLFunctions();
  _atomPickingShader.initializeOpenGLFunctions();
  _internalBondPickingShader.initializeOpenGLFunctions();
  _externalBondPickingShader.initializeOpenGLFunctions();
  _crystalCylinderPickingShader.initializeOpenGLFunctions();
  _crystalEllipsePickingShader.initializeOpenGLFunctions();
  _crystalPolyogonalPrismPickingShader.initializeOpenGLFunctions();
  _cylinderPickingShader.initializeOpenGLFunctions();
  _ellipsePickingShader.initializeOpenGLFunctions();
  _polygonalPrismPickingShader.initializeOpenGLFunctions();
}

void OpenGLPickingShader::loadShader()
{
  _atomPickingShader.loadShader();
  _internalBondPickingShader.loadShader();
  _externalBondPickingShader.loadShader();
  _crystalCylinderPickingShader.loadShader();
  _crystalEllipsePickingShader.loadShader();
  _crystalPolyogonalPrismPickingShader.loadShader();
  _cylinderPickingShader.loadShader();
  _ellipsePickingShader.loadShader();
  _polygonalPrismPickingShader.loadShader();
}

void OpenGLPickingShader::initializeVertexArrayObject()
{
  _atomPickingShader.initializeVertexArrayObject();
  _internalBondPickingShader.initializeVertexArrayObject();
  _externalBondPickingShader.initializeVertexArrayObject();
  _crystalCylinderPickingShader.initializeVertexArrayObject();
  _crystalEllipsePickingShader.initializeVertexArrayObject();
  _crystalPolyogonalPrismPickingShader.initializeVertexArrayObject();
  _cylinderPickingShader.initializeVertexArrayObject();
  _ellipsePickingShader.initializeVertexArrayObject();
  _polygonalPrismPickingShader.initializeVertexArrayObject();
}

void OpenGLPickingShader::initializeTransformUniforms()
{
  glUniformBlockBinding(_atomPickingShader.atomPickingProgram(), glGetUniformBlockIndex(_atomPickingShader.atomPickingProgram(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_internalBondPickingShader.program(), glGetUniformBlockIndex(_internalBondPickingShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_externalBondPickingShader.program(), glGetUniformBlockIndex(_externalBondPickingShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalCylinderPickingShader.program(), glGetUniformBlockIndex(_crystalCylinderPickingShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalEllipsePickingShader.program(), glGetUniformBlockIndex(_crystalEllipsePickingShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_crystalPolyogonalPrismPickingShader.program(), glGetUniformBlockIndex(_crystalPolyogonalPrismPickingShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_cylinderPickingShader.program(), glGetUniformBlockIndex(_cylinderPickingShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_ellipsePickingShader.program(), glGetUniformBlockIndex(_ellipsePickingShader.program(), "FrameUniformBlock"), 0);
  glUniformBlockBinding(_polygonalPrismPickingShader.program(), glGetUniformBlockIndex(_polygonalPrismPickingShader.program(), "FrameUniformBlock"), 0);
}

void OpenGLPickingShader::initializeStructureUniforms()
{
  glUniformBlockBinding(_atomPickingShader.atomPickingProgram(), glGetUniformBlockIndex(_atomPickingShader.atomPickingProgram(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_internalBondPickingShader.program(), glGetUniformBlockIndex(_internalBondPickingShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_externalBondPickingShader.program(), glGetUniformBlockIndex(_externalBondPickingShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalCylinderPickingShader.program(), glGetUniformBlockIndex(_crystalCylinderPickingShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalEllipsePickingShader.program(), glGetUniformBlockIndex(_crystalEllipsePickingShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_crystalPolyogonalPrismPickingShader.program(), glGetUniformBlockIndex(_crystalPolyogonalPrismPickingShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_cylinderPickingShader.program(), glGetUniformBlockIndex(_cylinderPickingShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_ellipsePickingShader.program(), glGetUniformBlockIndex(_ellipsePickingShader.program(), "StructureUniformBlock"), 1);
  glUniformBlockBinding(_polygonalPrismPickingShader.program(), glGetUniformBlockIndex(_polygonalPrismPickingShader.program(), "StructureUniformBlock"), 1);
}

void OpenGLPickingShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  _atomPickingShader.setRenderStructures(structures);
  _internalBondPickingShader.setRenderStructures(structures);
  _externalBondPickingShader.setRenderStructures(structures);
  _crystalCylinderPickingShader.setRenderStructures(structures);
  _crystalEllipsePickingShader.setRenderStructures(structures);
  _crystalPolyogonalPrismPickingShader.setRenderStructures(structures);
  _cylinderPickingShader.setRenderStructures(structures);
  _ellipsePickingShader.setRenderStructures(structures);
  _polygonalPrismPickingShader.setRenderStructures(structures);
}

void OpenGLPickingShader::reloadData()
{
  _atomPickingShader.reloadData();
  _internalBondPickingShader.reloadData();
  _externalBondPickingShader.reloadData();
  _crystalCylinderPickingShader.reloadData();
  _crystalEllipsePickingShader.reloadData();
  _crystalPolyogonalPrismPickingShader.reloadData();
  _cylinderPickingShader.reloadData();
  _ellipsePickingShader.reloadData();
  _polygonalPrismPickingShader.reloadData();
}

void OpenGLPickingShader::paintGL(int width,int height,GLuint structureUniformBuffer)
{
  GLfloat black[4] = {0.0,0.0,0.0,0.0};

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frameBufferObject);

  glViewport(0,0,width,height);
  glDisable(GL_BLEND);
  glClearBufferfv(GL_COLOR, 0, black);
  glClear(GL_DEPTH_BUFFER_BIT);

  _atomPickingShader.paintGL(structureUniformBuffer);
  _internalBondPickingShader.paintGL(structureUniformBuffer);
  _externalBondPickingShader.paintGL(structureUniformBuffer);
  _crystalCylinderPickingShader.paintGL(structureUniformBuffer);
  _crystalEllipsePickingShader.paintGL(structureUniformBuffer);
  _crystalPolyogonalPrismPickingShader.paintGL(structureUniformBuffer);
  _cylinderPickingShader.paintGL(structureUniformBuffer);
  _ellipsePickingShader.paintGL(structureUniformBuffer);
  _polygonalPrismPickingShader.paintGL(structureUniformBuffer);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

std::array<int,4> OpenGLPickingShader::pickTexture(int x, int y, int width, int height)
{
  Q_UNUSED(width);

  std::array<int,4> pixel{0,0,0,0};

  glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferObject);
  check_gl_error();

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  check_gl_error();

  glReadPixels(x, height - y, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixel.data());
  check_gl_error();

  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return pixel;
}


void OpenGLPickingShader::generateFrameBuffers()
{
  GLenum status;

  glGenFramebuffers(1, &_frameBufferObject);
  glGenTextures(1, &_texture);
  glGenTextures(1, &_depthTexture);
  check_gl_error();

  glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferObject);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, _texture);
  check_gl_error();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, 512, 512, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, nullptr);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  check_gl_error();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl_error();

  glBindTexture(GL_TEXTURE_2D, _depthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 512, 512, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
  check_gl_error();

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("initializePickingFrameBuffer fatal error: framebuffer incomplete");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLPickingShader::resizeGL(int w, int h)
{
  glBindTexture(GL_TEXTURE_2D, _texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, w, h, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, nullptr);
  check_gl_error();

  glBindTexture(GL_TEXTURE_2D, _depthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  check_gl_error();

  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl_error();
}


