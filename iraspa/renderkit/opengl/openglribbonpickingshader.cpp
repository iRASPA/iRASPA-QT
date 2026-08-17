/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#include "openglribbonpickingshader.h"
#include "openglribbonshader.h"
#include "openglribbonshaderpaths.h"
#include <QDebug>
#include "glgeterror.h"
#include "opengluniformstringliterals.h"

std::string OpenGLRibbonPickingShader::loadShaderStageSource(const char *fileName)
{
  return OpenGLRibbonShaders::loadShaderStageSource(fileName);
}

std::string OpenGLRibbonPickingShader::buildPickingVertexShaderSource()
{
  return OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
         OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
         OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
         loadShaderStageSource("ribbon_picking.vert");
}

std::string OpenGLRibbonPickingShader::buildPickingFragmentShaderSource()
{
  return OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
         OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
         loadShaderStageSource("ribbon_picking.frag");
}

OpenGLRibbonPickingShader::OpenGLRibbonPickingShader(OpenGLRibbonShader &ribbonShader): _ribbonShader(ribbonShader) {}

void OpenGLRibbonPickingShader::deleteBuffers()
{
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    if (!_vertexArrayObject.empty() && !_vertexArrayObject[i].empty())
    {
      glDeleteVertexArrays(static_cast<GLsizei>(_vertexArrayObject[i].size()), _vertexArrayObject[i].data());
    }
  }
}

void OpenGLRibbonPickingShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = std::move(structures);
  _vertexArrayObject.resize(_renderStructures.size());
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _vertexArrayObject[i].resize(_renderStructures[i].size());
    if (!_renderStructures[i].empty())
    {
      glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());
    }
  }
}

void OpenGLRibbonPickingShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLRibbonPickingShader::initializeVertexArrayObject()
{
  _ribbonShader.initializeVertexArrayObject();

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      glBindVertexArray(_vertexArrayObject[i][j]);
      glBindBuffer(GL_ARRAY_BUFFER, _ribbonShader._vertexBuffer[i][j]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ribbonShader._indexBuffer[i][j]);

      glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, position)));
      glVertexAttribPointer(_vertexNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, normal)));
      glVertexAttribPointer(_vertexSTAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, st)));
      glVertexAttribPointer(_vertexPadAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, pad)));
      glVertexAttribPointer(_vertexStripeSTAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, stripeST)));

      glEnableVertexAttribArray(_vertexPositionAttributeLocation);
      glEnableVertexAttribArray(_vertexNormalAttributeLocation);
      glEnableVertexAttribArray(_vertexSTAttributeLocation);
      glEnableVertexAttribArray(_vertexPadAttributeLocation);
      glEnableVertexAttribArray(_vertexStripeSTAttributeLocation);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
  }
}

void OpenGLRibbonPickingShader::paintGL(GLuint structureUniformBuffer)
{
  glUseProgram(_program);

  size_t index = 0;
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      if (RKRenderRibbonSource *ribbonSource = dynamic_cast<RKRenderRibbonSource*>(_renderStructures[i][j].get()))
      {
        if (ribbonSource->drawRibbon() &&
            _renderStructures[i][j]->isVisible() &&
            ribbonSource->ribbonNumberOfIndices() > 0 &&
            _ribbonShader._numberOfIndices[i][j] > 0)
        {
          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));
          glBindVertexArray(_vertexArrayObject[i][j]);

          const std::vector<RKRibbonChainDrawRange> drawRanges = ribbonSource->ribbonDrawRangesForEncoding();
          for (const RKRibbonChainDrawRange &drawRange : drawRanges)
          {
            _ribbonShader.drawIndexedRange(drawRange);
          }
          glBindVertexArray(0);
        }
      }
      index++;
    }
  }
  glUseProgram(0);
}

void OpenGLRibbonPickingShader::loadShader(void)
{
  const GLuint vertexShader = compileShaderOfType(GL_VERTEX_SHADER, buildPickingVertexShaderSource().c_str());
  const GLuint fragmentShader = compileShaderOfType(GL_FRAGMENT_SHADER, buildPickingFragmentShaderSource().c_str());

  if (vertexShader != 0 && fragmentShader != 0)
  {
    _program = glCreateProgram();
    glAttachShader(_program, vertexShader);
    glAttachShader(_program, fragmentShader);
    glBindFragDataLocation(_program, 0, "vFragColor");
    linkProgram(_program);

    _vertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");
    _vertexNormalAttributeLocation = glGetAttribLocation(_program, "vertexNormal");
    _vertexSTAttributeLocation = glGetAttribLocation(_program, "vertexST");
    _vertexPadAttributeLocation = glGetAttribLocation(_program, "vertexPad");
    _vertexStripeSTAttributeLocation = glGetAttribLocation(_program, "vertexStripeST");

    glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
    glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "StructureUniformBlock"), 1);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }
}
