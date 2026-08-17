/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#pragma once

#include <cstdint>
#include <vector>
#include <QCache>
#include <QString>
#include "openglshader.h"
#include "ribbonaolayout.h"
#include "rkrenderkitprotocols.h"

struct RibbonAOCachedTextures
{
  std::vector<uint16_t> processed;
  std::vector<uint16_t> raw;
};

class OpenGLRibbonShader;
class OpenGLAtomShader;

class OpenGLRibbonAmbientOcclusionShader: public OpenGLShader
{
public:
  OpenGLRibbonAmbientOcclusionShader(OpenGLRibbonShader &ribbonShader, OpenGLAtomShader &atomShader);
  ~OpenGLRibbonAmbientOcclusionShader() override;

  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void syncRenderStructures(const std::vector<std::vector<std::shared_ptr<RKRenderObject>>> &structures);
  void loadShader(void) override;
  void reloadData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality, std::vector<std::vector<GLuint>> &ribbonTextures, std::vector<std::vector<GLuint>> &ribbonRawTextures);
  void adjustRibbonAmbientOcclusionTextureSize();
  void invalidateCachedAmbientOcclusionTexture(std::vector<std::shared_ptr<RKRenderObject>> structures);

private:
  void updateRibbonAmbientOcclusionTextures(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality, std::vector<std::vector<GLuint>> &ribbonTextures, std::vector<std::vector<GLuint>> &ribbonRawTextures);
  void deleteVertexArrayObjects();
  void generateVertexArrayObjectBuffers();
  void initializeVertexArrayObjects();

  OpenGLRibbonShader &_ribbonShader;
  OpenGLAtomShader &_atomShader;

  GLuint _ribbonAmbientOcclusionProgram = 0;
  GLuint _ribbonShadowMapProgram = 0;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<GLuint>> _ribbonAmbientOcclusionVertexArrayObject;
  std::vector<std::vector<GLuint>> _ribbonShadowMapVertexArrayObject;

  QCache<QString, RibbonAOCachedTextures> _cache;

  GLint _ribbonAmbientOcclusionShadowMapUniformLocation = -1;
  GLint _ribbonAmbientOcclusionWeightUniformLocation = -1;
  GLint _ribbonAmbientOcclusionPositionAttributeLocation = -1;
  GLint _ribbonAmbientOcclusionNormalAttributeLocation = -1;
  GLint _ribbonAmbientOcclusionSTAttributeLocation = -1;

  GLint _ribbonShadowMapPositionAttributeLocation = -1;

  static const std::string _vertexRibbonAmbientOcclusionShaderSource;
  static const std::string _fragmentRibbonAmbientOcclusionShaderSource;
  static const std::string _vertexRibbonShadowMapShaderSource;
  static const std::string _fragmentRibbonShadowMapShaderSource;
};
