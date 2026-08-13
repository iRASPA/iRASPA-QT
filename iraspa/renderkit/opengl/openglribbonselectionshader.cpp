/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#include "openglribbonselectionshader.h"
#include "openglribbonshader.h"
#include "openglribbonshaderpaths.h"
#include <QDebug>
#include "glgeterror.h"
#include "opengluniformstringliterals.h"

std::string OpenGLRibbonSelectionShader::loadShaderStageSource(const char *fileName)
{
  return OpenGLRibbonShaders::loadShaderStageSource(fileName);
}

std::string OpenGLRibbonSelectionShader::buildSelectionVertexShaderSource(const char *fileName, bool includeWorleyHelpers)
{
  std::string source = OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
                       OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
                       OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
                       OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral;
  if (includeWorleyHelpers)
  {
    source += OpenGLUniformStringLiterals::OpenGLRGBHSVStringLiteral;
    source += OpenGLUniformStringLiterals::OpenGLWorleyNoise3DStringLiteral;
  }
  source += loadShaderStageSource("ribbon_selection_common.glsl");
  source += loadShaderStageSource(fileName);
  return source;
}

std::string OpenGLRibbonSelectionShader::buildSelectionFragmentShaderSource(const char *fileName, bool includeWorleyHelpers)
{
  std::string source = OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
                       OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
                       OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
                       OpenGLUniformStringLiterals::OpenGLLightUniformBlockStringLiteral;
  if (includeWorleyHelpers)
  {
    source += OpenGLUniformStringLiterals::OpenGLRGBHSVStringLiteral;
    source += OpenGLUniformStringLiterals::OpenGLWorleyNoise3DStringLiteral;
  }
  source += loadShaderStageSource(fileName);
  return source;
}

OpenGLRibbonSelectionShader::OpenGLRibbonSelectionShader(OpenGLRibbonShader &ribbonShader): _ribbonShader(ribbonShader) {}

void OpenGLRibbonSelectionShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
}

void OpenGLRibbonSelectionShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLRibbonSelectionShader::initializeVertexArrayObject()
{
  _ribbonShader.initializeVertexArrayObject();
}

void OpenGLRibbonSelectionShader::drawSelectedRanges(RKRenderRibbonSource *ribbonSource, int sceneIndex, int movieIndex, GLuint program)
{
  glUseProgram(program);
  glBindVertexArray(_ribbonShader._vertexArrayObject[static_cast<size_t>(sceneIndex)][static_cast<size_t>(movieIndex)]);

  const std::set<int> segmentIndices = ribbonSource->renderSelectedRibbonSegmentDrawRangeIndices();
  const std::set<int> residueIndices = ribbonSource->renderSelectedRibbonResidueDrawRangeIndices();
  const std::vector<RKRibbonChainDrawRange> segmentRanges = ribbonSource->ribbonSegmentDrawRanges();
  const std::vector<RKRibbonChainDrawRange> residueRanges = ribbonSource->ribbonResidueDrawRanges();

  for (int segmentIndex : segmentIndices)
  {
    if (segmentIndex < 0 || segmentIndex >= static_cast<int>(segmentRanges.size())) { continue; }
    if (ribbonSource->ribbonUsesSegmentVisibility() && !ribbonSource->isRibbonSegmentDrawRangeVisible(segmentIndex)) { continue; }
    const RKRibbonChainDrawRange &drawRange = segmentRanges[static_cast<size_t>(segmentIndex)];
    _ribbonShader.drawIndexedRange(drawRange);
  }

  for (int residueIndex : residueIndices)
  {
    if (residueIndex < 0 || residueIndex >= static_cast<int>(residueRanges.size())) { continue; }
    if (ribbonSource->ribbonUsesResidueVisibility() && !ribbonSource->isRibbonResidueDrawRangeVisible(residueIndex)) { continue; }
    const RKRibbonChainDrawRange &drawRange = residueRanges[static_cast<size_t>(residueIndex)];
    _ribbonShader.drawIndexedRange(drawRange);
  }

  glBindVertexArray(0);
}

void OpenGLRibbonSelectionShader::paintOverlayGL(GLuint structureUniformBuffer)
{
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  size_t structureIndex = 0;
  for (size_t sceneIndex = 0; sceneIndex < _renderStructures.size(); ++sceneIndex)
  {
    for (size_t movieIndex = 0; movieIndex < _renderStructures[sceneIndex].size(); ++movieIndex)
    {
      if (RKRenderRibbonSource *ribbonSource = dynamic_cast<RKRenderRibbonSource*>(_renderStructures[sceneIndex][movieIndex].get()))
      {
        if (RKRenderAtomSource *atomSource = dynamic_cast<RKRenderAtomSource*>(_renderStructures[sceneIndex][movieIndex].get()))
        {
          if (ribbonSource->drawRibbon() &&
              _renderStructures[sceneIndex][movieIndex]->isVisible() &&
              ribbonSource->ribbonNumberOfIndices() > 0 &&
              _ribbonShader._numberOfIndices[sceneIndex][movieIndex] > 0)
          {
            const std::set<int> segmentIndices = ribbonSource->renderSelectedRibbonSegmentDrawRangeIndices();
            const std::set<int> residueIndices = ribbonSource->renderSelectedRibbonResidueDrawRangeIndices();
            if (!segmentIndices.empty() || !residueIndices.empty())
            {
              GLuint program = 0;
              switch (atomSource->atomSelectionStyle())
              {
              case RKSelectionStyle::striped:
                program = _stripedProgram;
                break;
              case RKSelectionStyle::glow:
                program = 0;
                break;
              default:
                program = _worleyProgram;
                break;
              }

              if (program != 0)
              {
                glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(structureIndex * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));
                drawSelectedRanges(ribbonSource, static_cast<int>(sceneIndex), static_cast<int>(movieIndex), program);
              }
            }
          }
        }
      }
      structureIndex++;
    }
  }

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void OpenGLRibbonSelectionShader::paintGlowGL(GLuint structureUniformBuffer)
{
  glDepthMask(GL_FALSE);

  size_t structureIndex = 0;
  for (size_t sceneIndex = 0; sceneIndex < _renderStructures.size(); ++sceneIndex)
  {
    for (size_t movieIndex = 0; movieIndex < _renderStructures[sceneIndex].size(); ++movieIndex)
    {
      if (RKRenderRibbonSource *ribbonSource = dynamic_cast<RKRenderRibbonSource*>(_renderStructures[sceneIndex][movieIndex].get()))
      {
        if (RKRenderAtomSource *atomSource = dynamic_cast<RKRenderAtomSource*>(_renderStructures[sceneIndex][movieIndex].get()))
        {
          if (ribbonSource->drawRibbon() &&
              _renderStructures[sceneIndex][movieIndex]->isVisible() &&
              ribbonSource->ribbonNumberOfIndices() > 0 &&
              atomSource->atomSelectionStyle() == RKSelectionStyle::glow &&
              _ribbonShader._numberOfIndices[sceneIndex][movieIndex] > 0)
          {
            const std::set<int> segmentIndices = ribbonSource->renderSelectedRibbonSegmentDrawRangeIndices();
            const std::set<int> residueIndices = ribbonSource->renderSelectedRibbonResidueDrawRangeIndices();
            if (!segmentIndices.empty() || !residueIndices.empty())
            {
              glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, GLintptr(structureIndex * sizeof(RKStructureUniforms)), GLsizeiptr(sizeof(RKStructureUniforms)));
              drawSelectedRanges(ribbonSource, static_cast<int>(sceneIndex), static_cast<int>(movieIndex), _glowProgram);
            }
          }
        }
      }
      structureIndex++;
    }
  }

  glDepthMask(GL_TRUE);
}

void OpenGLRibbonSelectionShader::loadShader(void)
{
  auto buildProgram = [&](const char *vertexFile, const char *fragmentFile, bool includeWorleyHelpers) -> GLuint
  {
    const GLuint vertexShader = compileShaderOfType(GL_VERTEX_SHADER, buildSelectionVertexShaderSource(vertexFile, includeWorleyHelpers).c_str());
    const GLuint fragmentShader = compileShaderOfType(GL_FRAGMENT_SHADER, buildSelectionFragmentShaderSource(fragmentFile, includeWorleyHelpers).c_str());
    if (vertexShader == 0 || fragmentShader == 0) { return 0; }

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glBindFragDataLocation(program, 0, "vFragColor");
    OpenGLShader::linkProgram(program);

    glUniformBlockBinding(program, glGetUniformBlockIndex(program, "FrameUniformBlock"), 0);
    glUniformBlockBinding(program, glGetUniformBlockIndex(program, "StructureUniformBlock"), 1);
    glUniformBlockBinding(program, glGetUniformBlockIndex(program, "LightsUniformBlock"), 3);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
  };

  _glowProgram = buildProgram("ribbon_selection_glow.vert", "ribbon_selection_glow.frag", false);
  _worleyProgram = buildProgram("ribbon_selection_worley.vert", "ribbon_selection_worley.frag", true);
  _stripedProgram = buildProgram("ribbon_selection_striped.vert", "ribbon_selection_striped.frag", true);
}
