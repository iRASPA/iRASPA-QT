/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <memory>
#include <mathkit.h>
#include <symmetrykit.h>
#include "skspheresweep.h"

struct SKGeometricSurfaceClip
{
  double3 center = double3(0.0, 0.0, 0.0);
  double radius = 0.0;

  SKGeometricSurfaceClip() = default;
  SKGeometricSurfaceClip(double3 center, double radius) : center(center), radius(radius) {}
};

struct SKGeometricSurfacePatchCopy
{
  double3 center = double3(0.0, 0.0, 0.0);
  std::vector<SKGeometricSurfaceClip> clips;
  double3 cellOrigin = double3(0.0, 0.0, 0.0);

  SKGeometricSurfacePatchCopy() = default;
  SKGeometricSurfacePatchCopy(double3 center, std::vector<SKGeometricSurfaceClip> clips, double3 cellOrigin)
      : center(center), clips(std::move(clips)), cellOrigin(cellOrigin) {}
};

struct SKGeometricSurfacePatch
{
  int atomIndex = 0;
  double3 center = double3(0.0, 0.0, 0.0);
  double radius = 0.0;
  std::vector<SKGeometricSurfaceClip> clips;
  double area = 0.0;

  SKGeometricSurfacePatch() = default;
  SKGeometricSurfacePatch(int atomIndex, double3 center, double radius, std::vector<SKGeometricSurfaceClip> clips, double area)
      : atomIndex(atomIndex), center(center), radius(radius), clips(std::move(clips)), area(area) {}

  std::vector<SKGeometricSurfacePatchCopy> copiesInsideUnitCell(const std::shared_ptr<SKCell> &cell) const;
  static double3 fractionalExtent(double radius, const double3x3 &inverseUnitCell);
};

class SKGeometricSurface
{
public:
  std::vector<SKGeometricSurfacePatch> patches;
  double area = 0.0;

  SKGeometricSurface() = default;
  SKGeometricSurface(std::vector<SKGeometricSurfacePatch> patches, double area)
      : patches(std::move(patches)), area(area) {}

  static constexpr double minimumProbeSigma = 0.001;
  static double clampedProbeSigma(double probeSigma) { return std::max(probeSigma, minimumProbeSigma); }
  static double inflatedRadius(double atomSigma, double probeSigma) { return 0.5 * (atomSigma + probeSigma); }
  static double inflatedVanDerWaalsRadius(double atomVDW, double probeSigma) { return atomVDW + 0.5 * probeSigma; }
  static double3 wrappedFractional(double3 s);

  static SKGeometricSurface build(const std::vector<double3> &fractionalPositions,
                                  const std::vector<double> &radii,
                                  const std::shared_ptr<SKCell> &cell,
                                  const std::vector<double4> &blockingPockets = {},
                                  int subdivisions = 1);

  static SKGeometricSurface build(const std::vector<double3> &fractionalPositions,
                                  const std::vector<double2> &potentialParameters,
                                  double probeSigma,
                                  const std::shared_ptr<SKCell> &cell,
                                  const std::vector<double4> &blockingPockets = {},
                                  int subdivisions = 1);

  static SKGeometricSurface buildVanDerWaals(const std::vector<double3> &fractionalPositions,
                                             const std::vector<int> &elementIdentifiers,
                                             double probeSigma,
                                             const std::shared_ptr<SKCell> &cell,
                                             const std::vector<double4> &blockingPockets = {},
                                             int subdivisions = 1);

  struct PatchInstance
  {
    float4 centerAndRadius = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 axisAndType = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 aux0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 aux1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 aux2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    uint32_t clipStart = 0;
    uint32_t clipCount = 0;
    uint32_t flags = 0;
    uint32_t pad = 0;
  };

  struct ClipConstraint
  {
    float4 normalAndOffset = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 secondNormalAndOffset = float4(0.0f, 0.0f, 0.0f, 0.0f);
  };

  struct BufferPayload
  {
    std::vector<PatchInstance> patches;
    std::vector<ClipConstraint> clips;
  };

  static BufferPayload buildBuffers(const SKSphereSweep::Decomposition &decomposition,
                                    const std::vector<std::pair<double3, double>> &atoms,
                                    const double3x3 &latticeVectors,
                                    bool isPeriodic,
                                    const std::vector<float4> &atomColors = {});

  static BufferPayload buildSASBuffers(const SKSphereSweep::Decomposition &decomposition,
                                       const std::vector<std::pair<double3, double>> &atoms,
                                       const double3x3 &latticeVectors,
                                       bool isPeriodic,
                                       const std::vector<float4> &atomColors = {});
};
