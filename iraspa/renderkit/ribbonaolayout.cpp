/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#include "ribbonaolayout.h"
#include "rkrenderkitprotocols.h"
#include <memory>
#include <vector>

namespace
{
  QString formatRibbonColor3(const float3 &color)
  {
    return QStringLiteral("(%1, %2, %3)")
        .arg(double(color.x), 0, 'f', 2)
        .arg(double(color.y), 0, 'f', 2)
        .arg(double(color.z), 0, 'f', 2);
  }
}

QString ribbonAODebugModeLabel(RibbonAODebugMode mode)
{
  switch (mode)
  {
  case RibbonAODebugMode::off: return QStringLiteral("off");
  case RibbonAODebugMode::atlasUV: return QStringLiteral("atlas UV (red=U, green=V)");
  case RibbonAODebugMode::sampledAO: return QStringLiteral("sampled AO (grayscale)");
  case RibbonAODebugMode::atlasChecker: return QStringLiteral("atlas texel checker");
  case RibbonAODebugMode::baseColor: return QStringLiteral("base color (SS type, no lighting)");
  case RibbonAODebugMode::uniformColors: return QStringLiteral("uniform colors (swatches + RGB text)");
  case RibbonAODebugMode::atlasMapRed: return QStringLiteral("atlas map red (direct UV, black=empty)");
  case RibbonAODebugMode::atlasMapRedBake: return QStringLiteral("atlas map red (pre-post-process bake)");
  }
  return QStringLiteral("off");
}

void cycleRibbonAODebugMode(RibbonAODebugMode &mode)
{
  const int next = (static_cast<int>(mode) + 1) % 8;
  mode = static_cast<RibbonAODebugMode>(next);
}

QString ribbonColorUniformDebugOverlayText(const std::vector<std::vector<std::shared_ptr<RKRenderObject>>> &structures)
{
  for (const std::vector<std::shared_ptr<RKRenderObject>> &scene : structures)
  {
    for (const std::shared_ptr<RKRenderObject> &structure : scene)
    {
      if (!structure || !structure->isVisible())
      {
        continue;
      }
      if (RKRenderRibbonSource *ribbonSource = dynamic_cast<RKRenderRibbonSource*>(structure.get()))
      {
        if (!ribbonSource->drawRibbon())
        {
          continue;
        }
        return QStringLiteral("Coil %1   Helix %2   Sheet %3")
            .arg(formatRibbonColor3(ribbonSource->ribbonCoilColor()))
            .arg(formatRibbonColor3(ribbonSource->ribbonHelixColor()))
            .arg(formatRibbonColor3(ribbonSource->ribbonSheetColor()));
      }
    }
  }
  return QString();
}

int RKAmbientOcclusionSizing::maxTextureSize(int numberOfAtoms, int maxTextureDimension)
{
  const int cappedMax = std::min(maxTextureDimension, 16384);
  if (numberOfAtoms <= 64) return std::min(256, cappedMax);
  if (numberOfAtoms <= 256) return std::min(512, cappedMax);
  if (numberOfAtoms <= 1024) return std::min(1024, cappedMax);
  if (numberOfAtoms <= 65536) return std::min(2048, cappedMax);
  if (numberOfAtoms <= 524288) return std::min(4096, cappedMax);
  return std::min(8192, cappedMax);
}

bool shouldBakeRibbonAmbientOcclusion(RKRenderObject *object)
{
  auto *ribbonSource = dynamic_cast<RKRenderRibbonSource *>(object);
  return object && object->isVisible() && ribbonSource && ribbonSource->drawRibbon() && ribbonSource->ribbonAmbientOcclusion() &&
         ribbonSource->ribbonNumberOfChains() > 0;
}

QString ribbonAmbientOcclusionCacheKey(RKRenderObject *structure)
{
  RKRenderAtomSource *atomSource = dynamic_cast<RKRenderAtomSource*>(structure);
  const bool includeAtomShadows = atomSource && atomSource->atomAmbientOcclusion() && atomSource->drawAtoms();
  const bool drawAtoms = atomSource && atomSource->drawAtoms();

  RKRenderRibbonSource *ribbonSource = dynamic_cast<RKRenderRibbonSource*>(structure);
  const bool ribbonAOEnabled = ribbonSource && ribbonSource->ribbonAmbientOcclusion();
  const int textureWidth = ribbonSource ? ribbonSource->ribbonAmbientOcclusionTextureWidth() : 0;
  const int textureHeight = ribbonSource ? ribbonSource->ribbonAmbientOcclusionTextureHeight() : 0;
  const int maxSamples = ribbonSource ? ribbonSource->ribbonMaxSplineSampleCount() : 0;

  return QString("ribbon-ao-v65-dilate-gpublur-%1x%2-rings-%3-ribbonAO-%4-drawAtoms-%5-atomShadows-%6-%7")
      .arg(textureWidth)
      .arg(textureHeight)
      .arg(maxSamples)
      .arg(ribbonAOEnabled)
      .arg(drawAtoms)
      .arg(includeAtomShadows)
      .arg(reinterpret_cast<quintptr>(structure));
}
