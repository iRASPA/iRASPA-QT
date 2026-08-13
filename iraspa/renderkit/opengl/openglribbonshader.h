/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#pragma once

#include <vector>
#include <string>
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  #include <QGLFunctions>
#else
  #include <QOpenGLFunctions>
#endif
#include "openglshader.h"
#include "rkrenderkitprotocols.h"
#include "ribbonaolayout.h"

class OpenGLRibbonSelectionShader;
class OpenGLRibbonPickingShader;
class OpenGLRibbonAmbientOcclusionShader;

class OpenGLRibbonShader: public OpenGLShader
{
public:
  OpenGLRibbonShader();
  void loadShader(void) override;
  void deleteBuffers();
  void generateBuffers();

  void paintGL(std::vector<std::vector<GLuint>> &ribbonTextures, std::vector<std::vector<GLuint>> &ribbonRawTextures, GLuint structureUniformBuffer, GLuint fallbackAmbientOcclusionTexture, int viewportWidth, int viewportHeight);

  void reloadData();
  void initializeVertexArrayObject();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);

  GLuint program() const { return _program; }
  RibbonAODebugMode aoDebugMode() const { return _aoDebugMode; }
  void setAoDebugMode(RibbonAODebugMode mode) { _aoDebugMode = mode; }

private:
  enum class RibbonDrawVisibilityMode { none, segment, residue };

  void drawRibbonRanges(RKRenderRibbonSource *ribbonSource, int sceneIndex, int movieIndex);
  void drawIndexedRange(const RKRibbonChainDrawRange &drawRange);

  GLuint _program = 0;
  RibbonAODebugMode _aoDebugMode = RibbonAODebugMode::off;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;

  std::vector<std::vector<GLuint>> _vertexBuffer;
  std::vector<std::vector<GLuint>> _indexBuffer;
  std::vector<std::vector<GLuint>> _vertexArrayObject;
  std::vector<std::vector<size_t>> _numberOfVertices;
  std::vector<std::vector<size_t>> _numberOfIndices;

  GLint _ambientOcclusionTextureUniformLocation = -1;
  GLint _ambientOcclusionRawTextureUniformLocation = -1;
  GLint _vertexPositionAttributeLocation = -1;
  GLint _vertexNormalAttributeLocation = -1;
  GLint _vertexSTAttributeLocation = -1;
  GLint _vertexPadAttributeLocation = -1;
  GLint _vertexStripeSTAttributeLocation = -1;
  GLuint _ribbonAODebugUniformBuffer = 0;

  static std::string loadShaderStageSource(const char *fileName);
  static std::string buildRibbonVertexShaderSource();
  static std::string buildRibbonFragmentShaderSource();

  friend OpenGLRibbonSelectionShader;
  friend OpenGLRibbonPickingShader;
  friend OpenGLRibbonAmbientOcclusionShader;
};
