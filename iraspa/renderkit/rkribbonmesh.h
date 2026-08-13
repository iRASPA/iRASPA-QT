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
#include <algorithm>
#include "rkrenderuniforms.h"
#include "ribbonaolayout.h"

struct RKRibbonChainDrawRange
{
  int indexStart = 0;
  int indexCount = 0;

  RKRibbonChainDrawRange() = default;
  RKRibbonChainDrawRange(int indexStart, int indexCount): indexStart(indexStart), indexCount(indexCount) {}
};

struct RKRibbonMesh
{
  static constexpr int subdivisionsPerSegment = 24;
  static constexpr int lightmapStripHeight = 32;
  static constexpr int lightmapWidth = 2048;

  std::vector<RKVertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<RKRibbonChainDrawRange> chainDrawRanges;
  std::vector<RKRibbonChainDrawRange> segmentDrawRanges;
  std::vector<RKRibbonChainDrawRange> residueDrawRanges;
  std::vector<int> segmentAlphaCarbonTags;
  std::vector<int> residueAlphaCarbonTags;
  std::vector<int> chainSplineSampleCounts;

  int numberOfChains() const { return static_cast<int>(chainDrawRanges.size()); }

  int maxSplineSampleCount() const
  {
    if (chainSplineSampleCounts.empty()) { return 1; }
    return *std::max_element(chainSplineSampleCounts.begin(), chainSplineSampleCounts.end());
  }

  int numberOfRings() const { return maxSplineSampleCount(); }

  static std::tuple<int, int, int> ambientOcclusionAtlasDimensions(int maxSplineSampleCount,
                                                                   int numberOfChains,
                                                                   int numberOfAtoms,
                                                                   int maxTextureDimension = 16384);

  RKRibbonMesh() = default;
  RKRibbonMesh(std::vector<RKVertex> vertices, std::vector<uint32_t> indices, std::vector<RKRibbonChainDrawRange> chainDrawRanges):
    vertices(std::move(vertices)), indices(std::move(indices)), chainDrawRanges(std::move(chainDrawRanges)) {}
};
