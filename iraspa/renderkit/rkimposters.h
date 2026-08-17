/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "rkcamera.h"
#include "rkrenderkitprotocols.h"
#include "rkrenderuniforms.h"

// High-quality still frames and pictures shade ray-traced imposters per MSAA
// sample so silhouettes, clipping and depth are anti-aliased. Interaction
// (medium/low) uses the faster per-pixel path.
inline bool perSampleImposterShading(RKRenderQuality quality)
{
  return quality == RKRenderQuality::high || quality == RKRenderQuality::picture;
}

struct RKBackToFrontItem
{
  size_t sceneIndex = 0;
  size_t movieIndex = 0;
  uint32_t structureIndex = 0;
};

inline bool matchesRenderStructure(int filterScene, int filterMovie, size_t sceneIndex, size_t movieIndex)
{
  return filterScene < 0 || (static_cast<size_t>(filterScene) == sceneIndex && static_cast<size_t>(filterMovie) == movieIndex);
}

// Transparent objects must be composited back-to-front (farthest from the camera
// first), otherwise blending between overlapping transparent movies is incorrect.
// structureIndex is the flat offset into the structure/isosurface uniform buffers.
std::vector<RKBackToFrontItem> backToFrontRenderOrder(
    const std::vector<std::vector<std::shared_ptr<RKRenderObject>>> &structures, RKCamera *camera);
