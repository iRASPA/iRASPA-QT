/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#include "openglribbonambientocclusionshader.h"
#include "openglribbonshader.h"
#include "openglatomshader.h"
#include "openglambientocclusionshadowmapshader.h"
#include "openglatomsphereshader.h"
#include "opengluniformstringliterals.h"
#include "glgeterror.h"
#include "rkrenderuniforms.h"
#include "rkcamera.h"
#include "rkribbonmesh.h"
#include "ribbonaolayout.h"
#include "ribbonaotexturepostprocess.h"
#include <mathkit.h>
#include <QOpenGLContext>
#include <cstddef>
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
  bool renderStructureLayoutMatches(const std::vector<std::vector<std::shared_ptr<RKRenderObject>>> &left,
                                    const std::vector<std::vector<std::shared_ptr<RKRenderObject>>> &right)
  {
    if (left.size() != right.size())
    {
      return false;
    }
    for (size_t i = 0; i < left.size(); ++i)
    {
      if (left[i].size() != right[i].size())
      {
        return false;
      }
    }
    return true;
  }
}

OpenGLRibbonAmbientOcclusionShader::OpenGLRibbonAmbientOcclusionShader(OpenGLRibbonShader &ribbonShader, OpenGLAtomShader &atomShader):
  _ribbonShader(ribbonShader),
  _atomShader(atomShader)
{
  _cache.setMaxCost(64);
}

OpenGLRibbonAmbientOcclusionShader::~OpenGLRibbonAmbientOcclusionShader()
{
  deleteVertexArrayObjects();
}

void OpenGLRibbonAmbientOcclusionShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  const bool layoutChanged = !renderStructureLayoutMatches(_renderStructures, structures);
  _renderStructures = std::move(structures);
  if (layoutChanged || _ribbonAmbientOcclusionVertexArrayObject.empty())
  {
    deleteVertexArrayObjects();
    generateVertexArrayObjectBuffers();
  }
}

void OpenGLRibbonAmbientOcclusionShader::syncRenderStructures(const std::vector<std::vector<std::shared_ptr<RKRenderObject>>> &structures)
{
  const bool layoutChanged = !renderStructureLayoutMatches(_renderStructures, structures);
  _renderStructures = structures;
  if (layoutChanged || _ribbonAmbientOcclusionVertexArrayObject.empty())
  {
    deleteVertexArrayObjects();
    generateVertexArrayObjectBuffers();
  }
}

void OpenGLRibbonAmbientOcclusionShader::deleteVertexArrayObjects()
{
  if (QOpenGLContext::currentContext() != nullptr)
  {
    for (size_t i = 0; i < _ribbonAmbientOcclusionVertexArrayObject.size(); ++i)
    {
      if (!_ribbonAmbientOcclusionVertexArrayObject[i].empty())
      {
        glDeleteVertexArrays(static_cast<GLsizei>(_ribbonAmbientOcclusionVertexArrayObject[i].size()), _ribbonAmbientOcclusionVertexArrayObject[i].data());
      }
    }
    for (size_t i = 0; i < _ribbonShadowMapVertexArrayObject.size(); ++i)
    {
      if (!_ribbonShadowMapVertexArrayObject[i].empty())
      {
        glDeleteVertexArrays(static_cast<GLsizei>(_ribbonShadowMapVertexArrayObject[i].size()), _ribbonShadowMapVertexArrayObject[i].data());
      }
    }
  }
  _ribbonAmbientOcclusionVertexArrayObject.clear();
  _ribbonShadowMapVertexArrayObject.clear();
}

void OpenGLRibbonAmbientOcclusionShader::generateVertexArrayObjectBuffers()
{
  if (QOpenGLContext::currentContext() == nullptr)
  {
    return;
  }

  _ribbonAmbientOcclusionVertexArrayObject.assign(_renderStructures.size(), {});
  _ribbonShadowMapVertexArrayObject.assign(_renderStructures.size(), {});
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    const size_t structureCount = _renderStructures[i].size();
    _ribbonAmbientOcclusionVertexArrayObject[i].assign(structureCount, 0);
    _ribbonShadowMapVertexArrayObject[i].assign(structureCount, 0);
    if (structureCount > 0)
    {
      glGenVertexArrays(static_cast<GLsizei>(structureCount), _ribbonAmbientOcclusionVertexArrayObject[i].data());
      glGenVertexArrays(static_cast<GLsizei>(structureCount), _ribbonShadowMapVertexArrayObject[i].data());
    }
  }
}

void OpenGLRibbonAmbientOcclusionShader::initializeVertexArrayObjects()
{
  if (_ribbonAmbientOcclusionProgram == 0 || _ribbonShadowMapProgram == 0)
  {
    return;
  }
  if (QOpenGLContext::currentContext() == nullptr)
  {
    return;
  }

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    if (i >= _ribbonAmbientOcclusionVertexArrayObject.size() || i >= _ribbonShadowMapVertexArrayObject.size())
    {
      continue;
    }
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      if (j >= _ribbonAmbientOcclusionVertexArrayObject[i].size() || j >= _ribbonShadowMapVertexArrayObject[i].size())
      {
        continue;
      }
      if (i >= _ribbonShader._vertexBuffer.size() || j >= _ribbonShader._vertexBuffer[i].size()
          || i >= _ribbonShader._indexBuffer.size() || j >= _ribbonShader._indexBuffer[i].size())
      {
        continue;
      }

      const GLuint vertexBuffer = _ribbonShader._vertexBuffer[i][j];
      const GLuint indexBuffer = _ribbonShader._indexBuffer[i][j];

      glBindVertexArray(_ribbonShadowMapVertexArrayObject[i][j]);
      glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
      glEnableVertexAttribArray(_ribbonShadowMapPositionAttributeLocation);
      glVertexAttribPointer(_ribbonShadowMapPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, position)));
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glBindVertexArray(_ribbonAmbientOcclusionVertexArrayObject[i][j]);
      glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
      glEnableVertexAttribArray(_ribbonAmbientOcclusionPositionAttributeLocation);
      glVertexAttribPointer(_ribbonAmbientOcclusionPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, position)));
      glEnableVertexAttribArray(_ribbonAmbientOcclusionNormalAttributeLocation);
      glVertexAttribPointer(_ribbonAmbientOcclusionNormalAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, normal)));
      glEnableVertexAttribArray(_ribbonAmbientOcclusionSTAttributeLocation);
      glVertexAttribPointer(_ribbonAmbientOcclusionSTAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex, st)));
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
  }
}

void OpenGLRibbonAmbientOcclusionShader::invalidateCachedAmbientOcclusionTexture(std::vector<std::shared_ptr<RKRenderObject>> structures)
{
  for (const std::shared_ptr<RKRenderObject> &structure : structures)
  {
    _cache.remove(ribbonAmbientOcclusionCacheKey(structure.get()));
  }
}

void OpenGLRibbonAmbientOcclusionShader::adjustRibbonAmbientOcclusionTextureSize()
{
  const int maxSize = 16384;
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      if (RKRenderRibbonSource *ribbonSource = dynamic_cast<RKRenderRibbonSource*>(_renderStructures[i][j].get()))
      {
        if (!ribbonSource->drawRibbon() || ribbonSource->ribbonNumberOfChains() <= 0)
        {
          continue;
        }

        RKRenderAtomSource *atomSource = dynamic_cast<RKRenderAtomSource*>(_renderStructures[i][j].get());
        const int numberOfAtoms = atomSource ? static_cast<int>(atomSource->renderAtoms().size()) : static_cast<int>(ribbonSource->ribbonResidueDrawRanges().size());
        const auto atlasDimensions = RKRibbonMesh::ambientOcclusionAtlasDimensions(
            ribbonSource->ribbonMaxSplineSampleCount(),
            ribbonSource->ribbonNumberOfChains(),
            numberOfAtoms,
            maxSize);

        ribbonSource->setRibbonAmbientOcclusionTextureWidth(std::get<0>(atlasDimensions));
        ribbonSource->setRibbonAmbientOcclusionTextureHeight(std::get<1>(atlasDimensions));
        ribbonSource->setRibbonAmbientOcclusionStripHeight(std::get<2>(atlasDimensions));
        ribbonSource->setRibbonAmbientOcclusionTextureSize(std::max(std::get<0>(atlasDimensions), std::get<1>(atlasDimensions)));
        ribbonSource->setRibbonAmbientOcclusionPatchNumber(1);
        ribbonSource->setRibbonAmbientOcclusionPatchSize(std::get<0>(atlasDimensions));
      }
    }
  }
}

void OpenGLRibbonAmbientOcclusionShader::reloadData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality, std::vector<std::vector<GLuint>> &ribbonTextures, std::vector<std::vector<GLuint>> &ribbonRawTextures)
{
  adjustRibbonAmbientOcclusionTextureSize();
  initializeVertexArrayObjects();
  updateRibbonAmbientOcclusionTextures(dataSource, quality, ribbonTextures, ribbonRawTextures);
}

void OpenGLRibbonAmbientOcclusionShader::loadShader(void)
{
  if (_ribbonAmbientOcclusionProgram != 0)
  {
    glDeleteProgram(_ribbonAmbientOcclusionProgram);
    _ribbonAmbientOcclusionProgram = 0;
  }
  if (_ribbonShadowMapProgram != 0)
  {
    glDeleteProgram(_ribbonShadowMapProgram);
    _ribbonShadowMapProgram = 0;
  }

  GLuint vertexShader = compileShaderOfType(GL_VERTEX_SHADER, _vertexRibbonAmbientOcclusionShaderSource.c_str());
  GLuint fragmentShader = compileShaderOfType(GL_FRAGMENT_SHADER, _fragmentRibbonAmbientOcclusionShaderSource.c_str());
  if (vertexShader != 0 && fragmentShader != 0)
  {
    _ribbonAmbientOcclusionProgram = glCreateProgram();
    glAttachShader(_ribbonAmbientOcclusionProgram, vertexShader);
    glAttachShader(_ribbonAmbientOcclusionProgram, fragmentShader);
    glBindFragDataLocation(_ribbonAmbientOcclusionProgram, 0, "vFragColor");
    linkProgram(_ribbonAmbientOcclusionProgram);

    glUniformBlockBinding(_ribbonAmbientOcclusionProgram, glGetUniformBlockIndex(_ribbonAmbientOcclusionProgram, "StructureUniformBlock"), 1);
    glUniformBlockBinding(_ribbonAmbientOcclusionProgram, glGetUniformBlockIndex(_ribbonAmbientOcclusionProgram, "ShadowUniformBlock"), 2);

    _ribbonAmbientOcclusionShadowMapUniformLocation = glGetUniformLocation(_ribbonAmbientOcclusionProgram, "shadowMapTexture");
    _ribbonAmbientOcclusionWeightUniformLocation = glGetUniformLocation(_ribbonAmbientOcclusionProgram, "weight");
    _ribbonAmbientOcclusionPositionAttributeLocation = glGetAttribLocation(_ribbonAmbientOcclusionProgram, "vertexPosition");
    _ribbonAmbientOcclusionNormalAttributeLocation = glGetAttribLocation(_ribbonAmbientOcclusionProgram, "vertexNormal");
    _ribbonAmbientOcclusionSTAttributeLocation = glGetAttribLocation(_ribbonAmbientOcclusionProgram, "vertexST");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }

  vertexShader = compileShaderOfType(GL_VERTEX_SHADER, _vertexRibbonShadowMapShaderSource.c_str());
  fragmentShader = compileShaderOfType(GL_FRAGMENT_SHADER, _fragmentRibbonShadowMapShaderSource.c_str());
  if (vertexShader != 0 && fragmentShader != 0)
  {
    _ribbonShadowMapProgram = glCreateProgram();
    glAttachShader(_ribbonShadowMapProgram, vertexShader);
    glAttachShader(_ribbonShadowMapProgram, fragmentShader);
    linkProgram(_ribbonShadowMapProgram);

    glUniformBlockBinding(_ribbonShadowMapProgram, glGetUniformBlockIndex(_ribbonShadowMapProgram, "StructureUniformBlock"), 1);
    glUniformBlockBinding(_ribbonShadowMapProgram, glGetUniformBlockIndex(_ribbonShadowMapProgram, "ShadowUniformBlock"), 2);

    _ribbonShadowMapPositionAttributeLocation = glGetAttribLocation(_ribbonShadowMapProgram, "vertexPosition");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }

  initializeVertexArrayObjects();
}

namespace
{
  bool ribbonAmbientOcclusionTextureHasContent(const std::vector<uint16_t> &data)
  {
    float maxValue = 0.0f;
    for (uint16_t half : data)
    {
      maxValue = std::max(maxValue, RKHalfFloat::floatFromHalfBits(half));
    }
    return maxValue > 1.0e-5f;
  }

  void uploadWhiteRibbonTexture(GLuint texture, int width, int height)
  {
    std::vector<uint16_t> data(static_cast<size_t>(width) * static_cast<size_t>(height), RKHalfFloat::halfBitsFromFloat(1.0f));
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_HALF_FLOAT, data.data());
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void uploadRibbonHalfFloatTexture(GLuint texture, int width, int height, const std::vector<uint16_t> &data)
  {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_HALF_FLOAT, data.data());
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void restoreOpenGLStateAfterRibbonAOBake()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, 0);
    glUseProgram(0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
  }
}

void OpenGLRibbonAmbientOcclusionShader::updateRibbonAmbientOcclusionTextures(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality, std::vector<std::vector<GLuint>> &ribbonTextures, std::vector<std::vector<GLuint>> &ribbonRawTextures)
{
  if (ribbonTextures.size() != _renderStructures.size() || ribbonRawTextures.size() != _renderStructures.size())
  {
    return;
  }

  OpenGLAmbientOcclusionShadowMapShader &atomAmbientOcclusionShader = _atomShader._atomAmbientOcclusionShader;
  OpenGLAtomSphereShader &atomSphereShader = _atomShader._atomShader;
  const std::vector<std::vector<size_t>> &ribbonIndexCounts = _ribbonShader._numberOfIndices;
  const std::vector<std::vector<GLuint>> &ribbonAmbientOcclusionVertexArrays = _ribbonAmbientOcclusionVertexArrayObject;
  const std::vector<std::vector<GLuint>> &ribbonShadowMapVertexArrays = _ribbonShadowMapVertexArrayObject;
  const std::vector<std::vector<GLuint>> &atomShadowMapVertexArrays = atomAmbientOcclusionShader._atomShadowMapVertexArrayObject;
  const std::vector<std::vector<size_t>> &atomDrawCounts = atomSphereShader._numberOfDrawnAtoms;

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    if (i >= ribbonTextures.size() || ribbonTextures[i].size() != _renderStructures[i].size()
        || i >= ribbonRawTextures.size() || ribbonRawTextures[i].size() != _renderStructures[i].size())
    {
      continue;
    }
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      RKRenderRibbonSource *ribbonSource = dynamic_cast<RKRenderRibbonSource*>(_renderStructures[i][j].get());
      if (!ribbonSource || !ribbonSource->drawRibbon())
      {
        if (ribbonTextures[i][j] != 0)
        {
          glDeleteTextures(1, &ribbonTextures[i][j]);
          ribbonTextures[i][j] = 0;
        }
        if (ribbonRawTextures[i][j] != 0)
        {
          glDeleteTextures(1, &ribbonRawTextures[i][j]);
          ribbonRawTextures[i][j] = 0;
        }
        continue;
      }

      const int width = std::max(1, ribbonSource->ribbonAmbientOcclusionTextureWidth());
      const int height = std::max(1, ribbonSource->ribbonAmbientOcclusionTextureHeight());

      if (ribbonTextures[i][j] != 0)
      {
        glDeleteTextures(1, &ribbonTextures[i][j]);
      }
      glGenTextures(1, &ribbonTextures[i][j]);
      uploadWhiteRibbonTexture(ribbonTextures[i][j], width, height);

      if (ribbonRawTextures[i][j] != 0)
      {
        glDeleteTextures(1, &ribbonRawTextures[i][j]);
      }
      glGenTextures(1, &ribbonRawTextures[i][j]);
      uploadWhiteRibbonTexture(ribbonRawTextures[i][j], width, height);

      if (!ribbonSource->ribbonAmbientOcclusion() || !_renderStructures[i][j]->isVisible() || ribbonSource->ribbonNumberOfChains() <= 0)
      {
        continue;
      }

      const QString cacheKey = ribbonAmbientOcclusionCacheKey(_renderStructures[i][j].get());
      if (_cache.contains(cacheKey))
      {
        RibbonAOCachedTextures *cachedTextures = _cache.object(cacheKey);
        uploadRibbonHalfFloatTexture(ribbonTextures[i][j], width, height, cachedTextures->processed);
        uploadRibbonHalfFloatTexture(ribbonRawTextures[i][j], width, height, cachedTextures->raw);
        continue;
      }

      if (!dataSource || _ribbonAmbientOcclusionProgram == 0 || _ribbonShadowMapProgram == 0)
      {
        uploadWhiteRibbonTexture(ribbonTextures[i][j], width, height);
        uploadWhiteRibbonTexture(ribbonRawTextures[i][j], width, height);
        continue;
      }

      RKRenderObject *renderStructure = dynamic_cast<RKRenderObject*>(_renderStructures[i][j].get());
      if (!renderStructure)
      {
        uploadWhiteRibbonTexture(ribbonTextures[i][j], width, height);
        uploadWhiteRibbonTexture(ribbonRawTextures[i][j], width, height);
        continue;
      }

      double4x4 modelMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPointWithTranslation(
          double4x4(renderStructure->orientation()), renderStructure->cell()->boundingBox().center(), renderStructure->origin());

      std::vector<RKStructureUniforms> structureUniforms;
      std::vector<RKStructureUniforms> ribbonRenderStructureUniforms;
      structureUniforms.reserve(_renderStructures[i].size());
      ribbonRenderStructureUniforms.reserve(_renderStructures[i].size());
      for (size_t k = 0; k < _renderStructures[i].size(); ++k)
      {
        structureUniforms.emplace_back(static_cast<int>(i), static_cast<int>(k), _renderStructures[i][k], double4x4::inverse(modelMatrix));
        ribbonRenderStructureUniforms.emplace_back(static_cast<int>(i), static_cast<int>(k), _renderStructures[i][k]);
      }

      RKRenderAtomSource *atomSourceForUniforms = dynamic_cast<RKRenderAtomSource*>(ribbonSource);
      const bool ribbonUsesRenderUniformsForShadow = !(atomSourceForUniforms && atomSourceForUniforms->drawAtoms());

      SKBoundingBox boundingBox = dataSource->renderBoundingBox();
      const double largestRadius = boundingBox.boundingSphereRadius();
      const double3 centerOfScene = boundingBox.minimum() + (boundingBox.maximum() - boundingBox.minimum()) * 0.5;
      const double3 eye = double3(centerOfScene.x, centerOfScene.y, centerOfScene.z + largestRadius);

      const double boundingBoxAspectRatio = std::fabs(boundingBox.maximum().x - boundingBox.minimum().x) / std::fabs(boundingBox.maximum().y - boundingBox.minimum().y);
      double left, right, top, bottom;
      if (boundingBoxAspectRatio < 1.0)
      {
        left = -largestRadius / boundingBoxAspectRatio;
        right = largestRadius / boundingBoxAspectRatio;
        top = largestRadius / boundingBoxAspectRatio;
        bottom = -largestRadius / boundingBoxAspectRatio;
      }
      else
      {
        left = -largestRadius;
        right = largestRadius;
        top = largestRadius;
        bottom = -largestRadius;
      }

      const double nearPlane = 1.0;
      const double farPlane = 1000.0;

      int maxk = 360;
      if (quality == RKRenderQuality::picture)
      {
        maxk = 1992;
      }

      std::srand(0);

      std::vector<RKShadowUniforms> shadowMapFrameUniforms;
      shadowMapFrameUniforms.reserve(static_cast<size_t>(maxk));
      for (int k = 0; k < maxk; ++k)
      {
        simd_quatd smallChangeQ = simd_quatd::smallRandomQuaternion(0.5 * 10.0 * M_PI / 180.0);
        simd_quatd q = smallChangeQ * simd_quatd::ambientOcclusionDirection(k, maxk);
        double4x4 currentModelMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(q), centerOfScene);
        double4x4 viewMatrix = RKCamera::GluLookAt(eye, centerOfScene, double3(0, 1, 0));
        double4x4 projectionMatrix = RKCamera::glFrustumfOrthographic(left, right, bottom, top, nearPlane, farPlane);
        shadowMapFrameUniforms.emplace_back(projectionMatrix, viewMatrix, currentModelMatrix);
      }

      GLuint structureAmbientOcclusionUniformBuffer = 0;
      GLuint ribbonRenderStructureUniformBuffer = 0;
      GLuint shadowMapFrameUniformBuffer = 0;
      GLuint shadowMapFrameBufferObject = 0;
      GLuint shadowMapDepthTexture = 0;

      glGenBuffers(1, &structureAmbientOcclusionUniformBuffer);
      glGenBuffers(1, &ribbonRenderStructureUniformBuffer);
      glGenBuffers(1, &shadowMapFrameUniformBuffer);
      glGenFramebuffers(1, &shadowMapFrameBufferObject);
      glGenTextures(1, &shadowMapDepthTexture);

      glBindBuffer(GL_UNIFORM_BUFFER, structureAmbientOcclusionUniformBuffer);
      glBufferData(GL_UNIFORM_BUFFER, sizeof(RKStructureUniforms) * structureUniforms.size(), structureUniforms.data(), GL_DYNAMIC_DRAW);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);

      glBindBuffer(GL_UNIFORM_BUFFER, ribbonRenderStructureUniformBuffer);
      glBufferData(GL_UNIFORM_BUFFER, sizeof(RKStructureUniforms) * ribbonRenderStructureUniforms.size(), ribbonRenderStructureUniforms.data(), GL_DYNAMIC_DRAW);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);

      const GLuint ribbonStructureUniformBuffer = ribbonUsesRenderUniformsForShadow
          ? ribbonRenderStructureUniformBuffer
          : structureAmbientOcclusionUniformBuffer;

      glBindBuffer(GL_UNIFORM_BUFFER, shadowMapFrameUniformBuffer);
      glBufferData(GL_UNIFORM_BUFFER, sizeof(RKShadowUniforms) * shadowMapFrameUniforms.size(), shadowMapFrameUniforms.data(), GL_DYNAMIC_DRAW);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);

      glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFrameBufferObject);
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
      glBindTexture(GL_TEXTURE_2D, shadowMapDepthTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapDepthTexture, 0);
      glBindTexture(GL_TEXTURE_2D, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      glBindTexture(GL_TEXTURE_2D, ribbonTextures[i][j]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_HALF_FLOAT, nullptr);

      GLuint aoFrameBufferObject = 0;
      glGenFramebuffers(1, &aoFrameBufferObject);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, aoFrameBufferObject);
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ribbonTextures[i][j], 0);
      glViewport(0, 0, width, height);
      const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
      glClearBufferfv(GL_COLOR, 0, clearColor);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

      bool drewGeometry = false;
      const bool includeAtomShadows = atomSourceForUniforms && atomSourceForUniforms->atomAmbientOcclusion() && atomSourceForUniforms->drawAtoms();

      for (int k = 0; k < maxk; ++k)
      {
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFrameBufferObject);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 2048, 2048);

        glUseProgram(atomAmbientOcclusionShader._shadowMapProgram);
        for (size_t l = 0; l < _renderStructures[i].size(); ++l)
        {
          if (!_renderStructures[i][l]->isVisible())
          {
            continue;
          }

          if (includeAtomShadows)
          {
            if (RKRenderAtomSource *atomSource = dynamic_cast<RKRenderAtomSource*>(_renderStructures[i][l].get()))
            {
              if (atomSource->atomAmbientOcclusion() && atomSource->drawAtoms()
                  && i < atomDrawCounts.size() && l < atomDrawCounts[i].size()
                  && i < atomShadowMapVertexArrays.size() && l < atomShadowMapVertexArrays[i].size()
                  && atomDrawCounts[i][l] > 0)
              {
                glBindVertexArray(atomShadowMapVertexArrays[i][l]);
                glBindBufferRange(GL_UNIFORM_BUFFER, 2, shadowMapFrameUniformBuffer, k * sizeof(RKShadowUniforms), sizeof(RKShadowUniforms));
                glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureAmbientOcclusionUniformBuffer, l * sizeof(RKStructureUniforms), sizeof(RKStructureUniforms));
                glDrawElementsInstanced(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(atomDrawCounts[i][l]));
                glBindVertexArray(0);
              }
            }
          }

          if (RKRenderRibbonSource *shadowRibbonSource = dynamic_cast<RKRenderRibbonSource*>(_renderStructures[i][l].get()))
          {
            if (shadowRibbonSource->drawRibbon()
                && i < ribbonIndexCounts.size() && l < ribbonIndexCounts[i].size()
                && i < ribbonShadowMapVertexArrays.size() && l < ribbonShadowMapVertexArrays[i].size()
                && ribbonIndexCounts[i][l] > 0)
            {
              glDisable(GL_CULL_FACE);
              glUseProgram(_ribbonShadowMapProgram);
              glBindVertexArray(ribbonShadowMapVertexArrays[i][l]);
              glBindBufferRange(GL_UNIFORM_BUFFER, 2, shadowMapFrameUniformBuffer, k * sizeof(RKShadowUniforms), sizeof(RKShadowUniforms));
              glBindBufferRange(GL_UNIFORM_BUFFER, 1, ribbonStructureUniformBuffer, l * sizeof(RKStructureUniforms), sizeof(RKStructureUniforms));
              for (const RKRibbonChainDrawRange &chainRange : shadowRibbonSource->ribbonChainDrawRanges())
              {
                _ribbonShader.drawIndexedRange(chainRange);
              }
              glBindVertexArray(0);
              glEnable(GL_CULL_FACE);
              glUseProgram(atomAmbientOcclusionShader._shadowMapProgram);
            }
          }
        }
        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, aoFrameBufferObject);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, width, height);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glUseProgram(_ribbonAmbientOcclusionProgram);
        if (i < ribbonIndexCounts.size() && j < ribbonIndexCounts[i].size()
            && i < ribbonAmbientOcclusionVertexArrays.size() && j < ribbonAmbientOcclusionVertexArrays[i].size()
            && ribbonIndexCounts[i][j] > 0)
        {
          glBindVertexArray(ribbonAmbientOcclusionVertexArrays[i][j]);
          glBindBufferRange(GL_UNIFORM_BUFFER, 2, shadowMapFrameUniformBuffer, k * sizeof(RKShadowUniforms), sizeof(RKShadowUniforms));
          glBindBufferRange(GL_UNIFORM_BUFFER, 1, ribbonStructureUniformBuffer, j * sizeof(RKStructureUniforms), sizeof(RKStructureUniforms));
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, shadowMapDepthTexture);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glUniform1i(_ribbonAmbientOcclusionShadowMapUniformLocation, 0);
          glUniform1f(_ribbonAmbientOcclusionWeightUniformLocation, simd_quatd::ambientOcclusionBlendWeight(k, maxk));

          for (const RKRibbonChainDrawRange &chainRange : ribbonSource->ribbonChainDrawRanges())
          {
            if (chainRange.indexCount > 0)
            {
              _ribbonShader.drawIndexedRange(chainRange);
              drewGeometry = true;
            }
          }
          glBindVertexArray(0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }

      glFinish();

      if (drewGeometry)
      {
        std::vector<uint16_t> rawTextureData(static_cast<size_t>(width * height));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, aoFrameBufferObject);
        glReadPixels(0, 0, width, height, GL_RED, GL_HALF_FLOAT, rawTextureData.data());
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        uploadRibbonHalfFloatTexture(ribbonRawTextures[i][j], width, height, rawTextureData);

        std::vector<float> channelData(static_cast<size_t>(width * height));
        for (size_t index = 0; index < channelData.size(); ++index)
        {
          channelData[index] = RKHalfFloat::floatFromHalfBits(rawTextureData[index]);
        }
        RibbonAOTexturePostProcess::dilateAndSmooth(channelData, width, height);
        RibbonAOTexturePostProcess::gaussianBlur(channelData, width, height);
        std::vector<uint16_t> processedTextureData(static_cast<size_t>(width * height));
        for (size_t index = 0; index < processedTextureData.size(); ++index)
        {
          processedTextureData[index] = RKHalfFloat::halfBitsFromFloat(channelData[index]);
        }

        if (ribbonAmbientOcclusionTextureHasContent(processedTextureData))
        {
          uploadRibbonHalfFloatTexture(ribbonTextures[i][j], width, height, processedTextureData);
        }
        else
        {
          uploadWhiteRibbonTexture(ribbonTextures[i][j], width, height);
        }

        auto *cachedTextures = new RibbonAOCachedTextures();
        cachedTextures->processed = std::move(processedTextureData);
        cachedTextures->raw = std::move(rawTextureData);
        _cache.insert(cacheKey, cachedTextures);
      }
      else
      {
        uploadWhiteRibbonTexture(ribbonTextures[i][j], width, height);
        uploadWhiteRibbonTexture(ribbonRawTextures[i][j], width, height);
      }

      glBindBuffer(GL_UNIFORM_BUFFER, 0);
      glBindBufferBase(GL_UNIFORM_BUFFER, 1, 0);
      glBindBufferBase(GL_UNIFORM_BUFFER, 2, 0);

      glDeleteFramebuffers(1, &aoFrameBufferObject);
      glDeleteTextures(1, &shadowMapDepthTexture);
      glDeleteFramebuffers(1, &shadowMapFrameBufferObject);
      glDeleteBuffers(1, &shadowMapFrameUniformBuffer);
      glDeleteBuffers(1, &ribbonRenderStructureUniformBuffer);
      glDeleteBuffers(1, &structureAmbientOcclusionUniformBuffer);
    }
  }

  restoreOpenGLStateAfterRibbonAOBake();
}

const std::string OpenGLRibbonAmbientOcclusionShader::_vertexRibbonAmbientOcclusionShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLShadowUniformBlockStringLiteral +
std::string(R"foo(

out VS_OUT
{
  vec3 worldPosition;
  vec3 worldNormal;
} vs_out;

in vec4 vertexPosition;
in vec4 vertexNormal;
in vec2 vertexST;

void main()
{
  vec2 atlasUV = vertexST;
  vec2 clipPos = atlasUV * 2.0 - 1.0;
  gl_Position = vec4(clipPos.x, -clipPos.y, 0.0, 1.0);
  vs_out.worldPosition = (structureUniforms.modelMatrix * vertexPosition).xyz;
  vs_out.worldNormal = normalize(mat3(structureUniforms.modelMatrix) * vertexNormal.xyz);
}
)foo");

const std::string OpenGLRibbonAmbientOcclusionShader::_fragmentRibbonAmbientOcclusionShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLShadowUniformBlockStringLiteral +
std::string(R"foo(

uniform sampler2D shadowMapTexture;
uniform float weight;

out float vFragColor;

in VS_OUT
{
  vec3 worldPosition;
  vec3 worldNormal;
} fs_in;

void main()
{
  vec4 shadowCoordinate = shadowUniforms.shadowMatrix * vec4(fs_in.worldPosition, 1.0);
  vec4 shadowPos = shadowCoordinate / shadowCoordinate.w;
  vec4 viewNormal = shadowUniforms.viewMatrix * vec4(normalize(fs_in.worldNormal), 0.0);
  float normalWeight = max(viewNormal.z, 0.0);
  if (normalWeight < 1.0e-4)
  {
    discard;
  }
  float mapDepth = texture(shadowMapTexture, shadowPos.xy).r;
  float visibility = step(shadowPos.z, mapDepth);
  vFragColor = weight * normalWeight * visibility;
}
)foo");

const std::string OpenGLRibbonAmbientOcclusionShader::_vertexRibbonShadowMapShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLShadowUniformBlockStringLiteral +
std::string(R"foo(

in vec4 vertexPosition;

out VS_OUT
{
  vec4 eyePosition;
} vs_out;

void main()
{
  vs_out.eyePosition = shadowUniforms.viewMatrix * structureUniforms.modelMatrix * vertexPosition;
  gl_Position = shadowUniforms.projectionMatrix * vs_out.eyePosition;
}
)foo");

const std::string OpenGLRibbonAmbientOcclusionShader::_fragmentRibbonShadowMapShaderSource =
std::string(R"foo(
#version 330

layout (std140) uniform ShadowUniformBlock
{
  mat4 projectionMatrix;
  mat4 viewMatrix;
  mat4 shadowMatrix;
  mat4 normalMatrix;
} shadowUniforms;

in VS_OUT
{
  vec4 eyePosition;
} fs_in;

void main()
{
  vec4 pos = shadowUniforms.projectionMatrix * fs_in.eyePosition;
  gl_FragDepth = 0.5 * (pos.z / pos.w) + 0.5;
}
)foo");
