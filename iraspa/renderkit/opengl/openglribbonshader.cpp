/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#include "openglribbonshader.h"
#include "openglribbonshaderpaths.h"
#include <QDebug>
#include <cstdint>
#include "glgeterror.h"
#include "opengluniformstringliterals.h"

std::string OpenGLRibbonShader::loadShaderStageSource(const char *fileName)
{
  return OpenGLRibbonShaders::loadShaderStageSource(fileName);
}

std::string OpenGLRibbonShader::buildRibbonVertexShaderSource()
{
  return OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
         OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
         OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
         OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
         loadShaderStageSource("ribbon.vert");
}

std::string OpenGLRibbonShader::buildRibbonFragmentShaderSource()
{
  return OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
         OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
         OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
         OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral +
         OpenGLUniformStringLiterals::OpenGLRGBHSVStringLiteral +
         std::string(R"foo(
layout(std140) uniform RibbonAODebugUniformBlock
{
  int mode;
  int textureWidth;
  int textureHeight;
  int patchNumber;
  float patchSize;
  float inverseTextureSize;
  int fastInteractionShading;
  int viewportWidth;
  int viewportHeight;
} ribbonAODebug;
)foo") +
         loadShaderStageSource("ribbon.frag");
}

OpenGLRibbonShader::OpenGLRibbonShader() = default;

void OpenGLRibbonShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = std::move(structures);
  generateBuffers();
}

void OpenGLRibbonShader::deleteBuffers()
{
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    if (!_vertexBuffer.empty() && !_vertexBuffer[i].empty())
    {
      glDeleteBuffers(static_cast<GLsizei>(_vertexBuffer[i].size()), _vertexBuffer[i].data());
    }
    if (!_indexBuffer.empty() && !_indexBuffer[i].empty())
    {
      glDeleteBuffers(static_cast<GLsizei>(_indexBuffer[i].size()), _indexBuffer[i].data());
    }
    if (!_vertexArrayObject.empty() && !_vertexArrayObject[i].empty())
    {
      glDeleteVertexArrays(static_cast<GLsizei>(_vertexArrayObject[i].size()), _vertexArrayObject[i].data());
    }
  }
  if (_ribbonAODebugUniformBuffer != 0)
  {
    glDeleteBuffers(1, &_ribbonAODebugUniformBuffer);
    _ribbonAODebugUniformBuffer = 0;
  }
}

void OpenGLRibbonShader::generateBuffers()
{
  _vertexBuffer.resize(_renderStructures.size());
  _indexBuffer.resize(_renderStructures.size());
  _vertexArrayObject.resize(_renderStructures.size());
  _numberOfVertices.resize(_renderStructures.size());
  _numberOfIndices.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _vertexBuffer[i].resize(_renderStructures[i].size());
    _indexBuffer[i].resize(_renderStructures[i].size());
    _vertexArrayObject[i].resize(_renderStructures[i].size());
    _numberOfVertices[i].resize(_renderStructures[i].size(), 0);
    _numberOfIndices[i].resize(_renderStructures[i].size(), 0);
    if (!_renderStructures[i].empty())
    {
      glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _vertexBuffer[i].data());
      glGenBuffers(static_cast<GLsizei>(_renderStructures[i].size()), _indexBuffer[i].data());
      glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());
    }
  }
  glGenBuffers(1, &_ribbonAODebugUniformBuffer);
}

void OpenGLRibbonShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLRibbonShader::initializeVertexArrayObject()
{
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      if (RKRenderRibbonSource *ribbonSource = dynamic_cast<RKRenderRibbonSource*>(_renderStructures[i][j].get()))
      {
        if (!ribbonSource->drawRibbon())
        {
          _numberOfVertices[i][j] = 0;
          _numberOfIndices[i][j] = 0;
          continue;
        }
        const std::vector<RKVertex> vertices = ribbonSource->renderRibbonVertices();
        const std::vector<uint32_t> indices = ribbonSource->renderRibbonIndices();
        _numberOfVertices[i][j] = vertices.size();
        _numberOfIndices[i][j] = indices.size();

        glBindVertexArray(_vertexArrayObject[i][j]);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer[i][j]);
        if (!vertices.empty())
        {
          glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(RKVertex)), vertices.data(), GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer[i][j]);
        if (!indices.empty())
        {
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)), indices.data(), GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
        }

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
}

void OpenGLRibbonShader::drawRibbonRanges(RKRenderRibbonSource *ribbonSource, int /*sceneIndex*/, int /*movieIndex*/)
{
  if (!ribbonSource)
  {
    return;
  }
  for (const RKRibbonChainDrawRange &chainRange : ribbonSource->ribbonDrawRangesForEncoding())
  {
    drawIndexedRange(chainRange);
  }
}

void OpenGLRibbonShader::drawIndexedRange(const RKRibbonChainDrawRange &drawRange)
{
  if (drawRange.indexCount <= 0) { return; }
  glDrawElements(GL_TRIANGLES,
                 drawRange.indexCount,
                 GL_UNSIGNED_INT,
                 reinterpret_cast<const GLvoid*>(static_cast<uintptr_t>(drawRange.indexStart) * sizeof(uint32_t)));
}

void OpenGLRibbonShader::paintGL(std::vector<std::vector<GLuint>> &ribbonTextures, std::vector<std::vector<GLuint>> &ribbonRawTextures, GLuint structureUniformBuffer, GLuint fallbackAmbientOcclusionTexture, int viewportWidth, int viewportHeight)
{
  if (_program == 0)
  {
    return;
  }

  glDisable(GL_CULL_FACE);
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
            _numberOfIndices[i][j] > 0)
        {
          glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(index * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));

          RibbonAODebugUniforms debugUniforms;
          debugUniforms.mode = static_cast<int32_t>(_aoDebugMode);
          debugUniforms.textureWidth = ribbonSource->ribbonAmbientOcclusionTextureWidth();
          debugUniforms.textureHeight = ribbonSource->ribbonAmbientOcclusionTextureHeight();
          debugUniforms.inverseTextureSize = float(1.0 / double(std::max(ribbonSource->ribbonAmbientOcclusionTextureWidth(), 1)));
          debugUniforms.fastInteractionShading = 0;
          debugUniforms.viewportWidth = viewportWidth;
          debugUniforms.viewportHeight = viewportHeight;
          glBindBuffer(GL_UNIFORM_BUFFER, _ribbonAODebugUniformBuffer);
          glBufferData(GL_UNIFORM_BUFFER, sizeof(RibbonAODebugUniforms), &debugUniforms, GL_DYNAMIC_DRAW);
          glBindBufferBase(GL_UNIFORM_BUFFER, 4, _ribbonAODebugUniformBuffer);

          if (i < ribbonTextures.size() && j < ribbonTextures[i].size())
          {
            glActiveTexture(GL_TEXTURE0);
            GLuint ambientOcclusionTexture = ribbonTextures[i][j];
            if (ambientOcclusionTexture == 0)
            {
              ambientOcclusionTexture = fallbackAmbientOcclusionTexture;
            }
            glBindTexture(GL_TEXTURE_2D, ambientOcclusionTexture);
            glUniform1i(_ambientOcclusionTextureUniformLocation, 0);

            glActiveTexture(GL_TEXTURE1);
            GLuint ambientOcclusionRawTexture = fallbackAmbientOcclusionTexture;
            if (i < ribbonRawTextures.size() && j < ribbonRawTextures[i].size() && ribbonRawTextures[i][j] != 0)
            {
              ambientOcclusionRawTexture = ribbonRawTextures[i][j];
            }
            glBindTexture(GL_TEXTURE_2D, ambientOcclusionRawTexture);
            glUniform1i(_ambientOcclusionRawTextureUniformLocation, 1);
          }
          else if (fallbackAmbientOcclusionTexture != 0)
          {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fallbackAmbientOcclusionTexture);
            glUniform1i(_ambientOcclusionTextureUniformLocation, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, fallbackAmbientOcclusionTexture);
            glUniform1i(_ambientOcclusionRawTextureUniformLocation, 1);
          }

          glBindVertexArray(_vertexArrayObject[i][j]);
          drawRibbonRanges(ribbonSource, static_cast<int>(i), static_cast<int>(j));
          glBindVertexArray(0);
        }
      }
      index++;
    }
  }
  glUseProgram(0);
}

void OpenGLRibbonShader::loadShader(void)
{
  const GLuint vertexShader = compileShaderOfType(GL_VERTEX_SHADER, buildRibbonVertexShaderSource().c_str());
  const GLuint fragmentShader = compileShaderOfType(GL_FRAGMENT_SHADER, buildRibbonFragmentShaderSource().c_str());

  if (vertexShader != 0 && fragmentShader != 0)
  {
    _program = glCreateProgram();
    glAttachShader(_program, vertexShader);
    glAttachShader(_program, fragmentShader);
    glBindFragDataLocation(_program, 0, "vFragColor");
    linkProgram(_program);

    _ambientOcclusionTextureUniformLocation = glGetUniformLocation(_program, "ambientOcclusionTexture");
    _ambientOcclusionRawTextureUniformLocation = glGetUniformLocation(_program, "ambientOcclusionRawTexture");
    _vertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");
    _vertexNormalAttributeLocation = glGetAttribLocation(_program, "vertexNormal");
    _vertexSTAttributeLocation = glGetAttribLocation(_program, "vertexST");
    _vertexPadAttributeLocation = glGetAttribLocation(_program, "vertexPad");
    _vertexStripeSTAttributeLocation = glGetAttribLocation(_program, "vertexStripeST");

    glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
    glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "StructureUniformBlock"), 1);
    glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "LightsUniformBlock"), 3);
    glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "RibbonAODebugUniformBlock"), 4);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }
}
