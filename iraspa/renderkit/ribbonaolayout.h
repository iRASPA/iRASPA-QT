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
#include <memory>
#include <vector>
#include <QString>
#include <mathkit.h>

class RKRenderObject;

enum class RibbonAODebugMode: int32_t
{
  off = 0,
  atlasUV = 1,
  sampledAO = 2,
  atlasChecker = 3,
  baseColor = 4,
  uniformColors = 5,
  atlasMapRed = 6,
  atlasMapRedBake = 7
};

QString ribbonAODebugModeLabel(RibbonAODebugMode mode);
void cycleRibbonAODebugMode(RibbonAODebugMode &mode);
QString ribbonColorUniformDebugOverlayText(const std::vector<std::vector<std::shared_ptr<RKRenderObject>>> &structures);

struct RibbonAODebugUniforms
{
  int32_t mode = static_cast<int32_t>(RibbonAODebugMode::off);
  int32_t textureWidth = 0;
  int32_t textureHeight = 0;
  int32_t patchNumber = 1;
  float patchSize = 16.0f;
  float inverseTextureSize = 1.0f / 1024.0f;
  int32_t fastInteractionShading = 0;
  int32_t viewportWidth = 0;
  int32_t viewportHeight = 0;
};

struct RibbonAOBlurUniforms
{
  float2 inverseTextureSize = float2(0.0f, 0.0f);
};

struct RibbonAOPatchUniforms
{
  int32_t patchNumber = 1;
  float patchSize = 16.0f;
  float inverseTextureSize = 1.0f / 1024.0f;
  int32_t pad = 0;
};

struct RKAmbientOcclusionSizing
{
  static int maxTextureSize(int numberOfAtoms, int maxTextureDimension = 16384);
};

class RKRenderObject;
class RKRenderAtomSource;
class RKRenderRibbonSource;

QString ribbonAmbientOcclusionCacheKey(RKRenderObject *structure);
