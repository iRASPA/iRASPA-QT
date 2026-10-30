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

#include "skgeometricsurface.h"
#include <algorithm>
#include <cmath>
#include <set>

namespace
{
struct ImageShiftLess
{
  bool operator()(const std::pair<int, int3> &a, const std::pair<int, int3> &b) const
  {
    if (a.first != b.first) return a.first < b.first;
    if (a.second.x != b.second.x) return a.second.x < b.second.x;
    if (a.second.y != b.second.y) return a.second.y < b.second.y;
    return a.second.z < b.second.z;
  }
};

struct ExpandedBall
{
  double3 center;
  double radius{0.0};
  int atomIndex{0};
  int3 imageShift{0, 0, 0};
};

std::vector<ExpandedBall> buildExpandedBalls(
    const std::vector<std::pair<double3, double>> &atoms,
    const double3x3 &lattice,
    bool periodic,
    double probeRadius)
{
  std::vector<ExpandedBall> baseBalls;
  baseBalls.reserve(atoms.size());
  double maxR = 0.0;
  for (std::size_t idx = 0; idx < atoms.size(); ++idx)
  {
    double r = atoms[idx].second + probeRadius;
    if (r <= 0.0) continue;
    maxR = std::max(maxR, r);
    baseBalls.push_back(ExpandedBall{atoms[idx].first, r, static_cast<int>(idx), int3(0, 0, 0)});
  }

  if (!periodic || baseBalls.empty())
  {
    return baseBalls;
  }

  double3 a = lattice[0];
  double3 b = lattice[1];
  double3 c = lattice[2];
  double vol = std::abs(double3::dot(a, double3::cross(b, c)));
  double ha = vol / std::max(1e-12, double3::cross(b, c).length());
  double hb = vol / std::max(1e-12, double3::cross(c, a).length());
  double hc = vol / std::max(1e-12, double3::cross(a, b).length());

  int nx = std::max(1, static_cast<int>(std::ceil(2.0 * maxR / ha)));
  int ny = std::max(1, static_cast<int>(std::ceil(2.0 * maxR / hb)));
  int nz = std::max(1, static_cast<int>(std::ceil(2.0 * maxR / hc)));

  std::vector<ExpandedBall> allBalls = baseBalls;
  for (int ix = -nx; ix <= nx; ++ix)
  {
    for (int iy = -ny; iy <= ny; ++iy)
    {
      for (int iz = -nz; iz <= nz; ++iz)
      {
        if (ix == 0 && iy == 0 && iz == 0) continue;
        double3 shift = double(ix) * a + double(iy) * b + double(iz) * c;
        for (const auto &ball : baseBalls)
        {
          double3 shiftedCenter = ball.center + shift;
          bool touchesPrimary = false;
          for (const auto &primary : baseBalls)
          {
            double3 diff = shiftedCenter - primary.center;
            double sumR = ball.radius + primary.radius;
            if (double3::dot(diff, diff) < sumR * sumR + 1e-9)
            {
              touchesPrimary = true;
              break;
            }
          }
          if (touchesPrimary)
          {
            allBalls.push_back(ExpandedBall{shiftedCenter, ball.radius, ball.atomIndex, int3(ix, iy, iz)});
          }
        }
      }
    }
  }
  return allBalls;
}
} // namespace

SKGeometricSurface::BufferPayload SKGeometricSurface::buildBuffers(
    const SKSphereSweep::Decomposition &decomposition,
    const std::vector<std::pair<double3, double>> &atoms,
    const double3x3 &latticeVectors,
    bool isPeriodic,
    const std::vector<float4> &atomColors)
{
  BufferPayload payload;
  double probeRadius = decomposition.probeRadius;
  if (atoms.empty()) return payload;

  std::vector<ExpandedBall> expandedBalls = buildExpandedBalls(atoms, latticeVectors, isPeriodic, probeRadius);

  auto colorForAtom = [&](int idx) -> float4
  {
    if (idx >= 0 && static_cast<std::size_t>(idx) < atomColors.size())
    {
      return atomColors[static_cast<std::size_t>(idx)];
    }
    return float4(0.75f, 0.78f, 0.85f, 1.0f);
  };

  for (const auto &face : decomposition.atomFaces)
  {
    if (face.area <= 1e-12) continue;
    int atomI = face.atomIndex;
    double rExpanded = face.radius;
    double rContact = (probeRadius > 0.0) ? std::max(1e-4, rExpanded - probeRadius) : rExpanded;
    double3 cI = face.center;

    uint32_t clipStart = static_cast<uint32_t>(payload.clips.size());
    std::set<std::pair<int, int3>, ImageShiftLess> seenNeighbors;

    for (const auto &cyc : face.cycles)
    {
      for (const auto &arc : cyc.arcs)
      {
        std::pair<int, int3> key{arc.otherBall, arc.otherImage};
        if (seenNeighbors.find(key) != seenNeighbors.end()) continue;
        seenNeighbors.insert(key);

        int jIdx = arc.otherBall;
        if (jIdx < 0 || static_cast<std::size_t>(jIdx) >= atoms.size()) continue;
        double rJ = atoms[static_cast<std::size_t>(jIdx)].second + probeRadius;
        double3 diff = arc.otherCenter - cI;
        double d2 = double3::dot(diff, diff);
        double d = std::sqrt(d2);
        if (d <= 1e-12 || d >= rExpanded + rJ - 1e-12) continue;

        double cosAlpha = std::clamp((rExpanded * rExpanded + d2 - rJ * rJ) / (2.0 * rExpanded * d), -1.0, 1.0);
        double3 axisUnit = diff / d;
        ClipConstraint clip;
        clip.normalAndOffset = float4(float(axisUnit.x), float(axisUnit.y), float(axisUnit.z), float(cosAlpha));
        clip.secondNormalAndOffset = float4(0.0f, 0.0f, 0.0f, 0.0f);
        payload.clips.push_back(clip);
      }
    }

    for (const auto &bj : expandedBalls)
    {
      if (bj.atomIndex == atomI && bj.imageShift == int3(0, 0, 0)) continue;
      std::pair<int, int3> key{bj.atomIndex, bj.imageShift};
      if (seenNeighbors.find(key) != seenNeighbors.end()) continue;
      double3 diff = bj.center - cI;
      double d2 = double3::dot(diff, diff);
      double d = std::sqrt(d2);
      if (d <= 1e-12 || d >= rExpanded + bj.radius - 1e-12) continue;
      if (d <= rExpanded - bj.radius + 1e-12) continue;

      double cosAlpha = std::clamp((rExpanded * rExpanded + d2 - bj.radius * bj.radius) / (2.0 * rExpanded * d), -1.0, 1.0);
      double3 axisUnit = diff / d;
      ClipConstraint clip;
      clip.normalAndOffset = float4(float(axisUnit.x), float(axisUnit.y), float(axisUnit.z), float(cosAlpha));
      clip.secondNormalAndOffset = float4(0.0f, 0.0f, 0.0f, 0.0f);
      payload.clips.push_back(clip);
    }

    uint32_t clipCount = static_cast<uint32_t>(payload.clips.size()) - clipStart;

    PatchInstance patch;
    patch.centerAndRadius = float4(float(cI.x), float(cI.y), float(cI.z), float(rContact));
    patch.axisAndType = float4(0.0f, 0.0f, 1.0f, 0.0f);
    patch.aux0 = float4(float(rExpanded), float(probeRadius), 0.0f, 0.0f);
    patch.aux1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    patch.aux2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    patch.color = colorForAtom(atomI);
    patch.clipStart = clipStart;
    patch.clipCount = clipCount;
    patch.flags = 0;
    payload.patches.push_back(patch);
  }

  if (probeRadius <= 0.0)
  {
    return payload;
  }

  for (const auto &sp : decomposition.saddlePatches)
  {
    if (sp.area <= 1e-12 || sp.arcs.empty()) continue;

    double thetaMax = 0.5 * M_PI - sp.psiI;
    double thetaMin = -(0.5 * M_PI - sp.psiJ);
    if (sp.majorRadius < probeRadius)
    {
      double cosLim = std::clamp(sp.majorRadius / probeRadius, -1.0, 1.0);
      double thetaSelf = std::acos(cosLim);
      thetaMax = std::min(thetaMax, thetaSelf);
      thetaMin = std::max(thetaMin, -thetaSelf);
    }
    if (thetaMax <= thetaMin) continue;

    double3 uAxis, vAxis;
    SKSphereSweep::makeCircleFrame(sp.axis, uAxis, vAxis);

    double zMin = -probeRadius * std::sin(thetaMax);
    double zMax = -probeRadius * std::sin(thetaMin);
    double hBound = std::max(std::abs(zMin), std::abs(zMax)) + 1e-3;
    double rOuter = sp.majorRadius + probeRadius + 1e-3;
    double rBound = std::sqrt(rOuter * rOuter + hBound * hBound);

    float4 colI = colorForAtom(sp.atomI);
    float4 colJ = colorForAtom(sp.atomJ);
    float4 blendedColor = float4(0.5f * (colI.x + colJ.x),
                                 0.5f * (colI.y + colJ.y),
                                 0.5f * (colI.z + colJ.z),
                                 0.5f * (colI.w + colJ.w));

    for (const auto &arcRange : sp.arcs)
    {
      double phiEnd = arcRange.first;
      double phiStart = arcRange.second;
      double isFullCircle = (phiStart == 0.0 && phiEnd == 2.0 * M_PI) ? 1.0 : 0.0;
      double dPhi = (isFullCircle > 0.5)
                        ? 2.0 * M_PI
                        : SKSphereSweep::positiveAngleDifference(phiEnd, phiStart);

      uint32_t clipStart = static_cast<uint32_t>(payload.clips.size());
      ClipConstraint clip;
      clip.normalAndOffset = float4(float(uAxis.x), float(uAxis.y), float(uAxis.z), float(phiEnd));
      clip.secondNormalAndOffset = float4(float(vAxis.x), float(vAxis.y), float(vAxis.z), float(dPhi));
      payload.clips.push_back(clip);

      double maxDist = sp.majorRadius + probeRadius;
      for (const auto &bj : expandedBalls)
      {
        if (bj.atomIndex == sp.atomI && bj.imageShift == int3(0, 0, 0)) continue;
        if (bj.atomIndex == sp.atomJ && bj.imageShift == sp.imageShiftJ) continue;
        double3 diff = bj.center - sp.torusCenter;
        double dist = diff.length();
        if (dist >= maxDist + bj.radius - 1e-9) continue;

        ClipConstraint nClip;
        nClip.normalAndOffset = float4(float(bj.center.x), float(bj.center.y), float(bj.center.z), float(bj.radius));
        nClip.secondNormalAndOffset = float4(0.0f, 0.0f, 0.0f, 0.0f);
        payload.clips.push_back(nClip);
      }
      uint32_t clipCount = static_cast<uint32_t>(payload.clips.size()) - clipStart;

      PatchInstance patch;
      patch.centerAndRadius = float4(float(sp.torusCenter.x), float(sp.torusCenter.y), float(sp.torusCenter.z), float(rBound));
      patch.axisAndType = float4(float(sp.axis.x), float(sp.axis.y), float(sp.axis.z), 1.0f);
      patch.aux0 = float4(float(sp.majorRadius), float(probeRadius), float(zMin), float(zMax));
      patch.aux1 = float4(float(sp.centerI.x), float(sp.centerI.y), float(sp.centerI.z), float(sp.radiusI + probeRadius));
      patch.aux2 = float4(float(sp.centerJ.x), float(sp.centerJ.y), float(sp.centerJ.z), float(sp.radiusJ + probeRadius));
      patch.color = blendedColor;
      patch.clipStart = clipStart;
      patch.clipCount = clipCount;
      patch.flags = (isFullCircle > 0.5) ? 1u : 0u;
      payload.patches.push_back(patch);
    }
  }

  for (const auto &cc : decomposition.concaveCorners)
  {
    if (cc.area <= 1e-12) continue;

    double3 uI = (cc.touchI - cc.probeCenter) / probeRadius;
    double3 uJ = (cc.touchJ - cc.probeCenter) / probeRadius;
    double3 uK = (cc.touchK - cc.probeCenter) / probeRadius;

    double det = double3::dot(uI, double3::cross(uJ, uK));
    double sign = (det >= 0.0) ? 1.0 : -1.0;

    double3 nIJ = sign * double3::cross(uI, uJ);
    double3 nJK = sign * double3::cross(uJ, uK);
    double3 nKI = sign * double3::cross(uK, uI);

    double lIJ = nIJ.length();
    double lJK = nJK.length();
    double lKI = nKI.length();
    if (lIJ < 1e-12 || lJK < 1e-12 || lKI < 1e-12) continue;
    nIJ /= lIJ;
    nJK /= lJK;
    nKI /= lKI;

    float4 colI = colorForAtom(cc.atomI);
    float4 colJ = colorForAtom(cc.atomJ);
    float4 colK = colorForAtom(cc.atomK);
    float4 cornerColor = float4((colI.x + colJ.x + colK.x) / 3.0f,
                                (colI.y + colJ.y + colK.y) / 3.0f,
                                (colI.z + colJ.z + colK.z) / 3.0f,
                                (colI.w + colJ.w + colK.w) / 3.0f);

    PatchInstance patch;
    patch.centerAndRadius = float4(float(cc.probeCenter.x), float(cc.probeCenter.y), float(cc.probeCenter.z), float(probeRadius));
    patch.axisAndType = float4(0.0f, 0.0f, 1.0f, 2.0f);
    patch.aux0 = float4(float(nIJ.x), float(nIJ.y), float(nIJ.z), 0.0f);
    patch.aux1 = float4(float(nJK.x), float(nJK.y), float(nJK.z), 0.0f);
    patch.aux2 = float4(float(nKI.x), float(nKI.y), float(nKI.z), 0.0f);
    patch.color = cornerColor;
    patch.clipStart = 0;
    patch.clipCount = 0;
    patch.flags = static_cast<uint32_t>(cc.multiplicity);
    payload.patches.push_back(patch);
  }

  return payload;
}

SKGeometricSurface::BufferPayload SKGeometricSurface::buildSASBuffers(
    const SKSphereSweep::Decomposition &decomposition,
    const std::vector<std::pair<double3, double>> &atoms,
    const double3x3 &latticeVectors,
    bool isPeriodic,
    const std::vector<float4> &atomColors)
{
  SKSphereSweep::Decomposition sasDecomp = decomposition;
  sasDecomp.probeRadius = 0.0;
  sasDecomp.saddlePatches.clear();
  sasDecomp.concaveCorners.clear();

  double probeRadius = decomposition.probeRadius;
  std::vector<std::pair<double3, double>> expandedAtoms;
  expandedAtoms.reserve(atoms.size());
  for (const auto &atom : atoms)
  {
    expandedAtoms.push_back({atom.first, atom.second + probeRadius});
  }

  return buildBuffers(sasDecomp, expandedAtoms, latticeVectors, isPeriodic, atomColors);
}

namespace
{
constexpr int SKExactQuadratureOrder = 10;
constexpr double SKSweepGapTolerance = 1.0e-12;
constexpr double SKCapCoverTolerance = 1.0e-9;
constexpr double SKPoleClearance = 4.0;

double3 SKPerpendicularTo(const double3 &axis)
{
  double3 helper(1.0, 0.0, 0.0);
  if (std::abs(axis.y) < std::abs(axis.x))
  {
    helper = double3(0.0, 1.0, 0.0);
  }
  if (std::abs(axis.z) < std::min(std::abs(axis.x), std::abs(axis.y)))
  {
    helper = double3(0.0, 0.0, 1.0);
  }
  double3 perpendicular = double3::cross(helper, axis);
  double length = perpendicular.length();
  return length > 0.0 ? perpendicular / length : double3(1.0, 0.0, 0.0);
}

double SKFoldedPolarAngle(double angle)
{
  double wrapped = std::fmod(std::abs(angle), 2.0 * M_PI);
  if (wrapped > M_PI)
  {
    wrapped = 2.0 * M_PI - wrapped;
  }
  return wrapped;
}

struct SweepFrame
{
  double3 first;
  double3 second;
  double3 polar;
};

SweepFrame SKSweepFrame(const std::vector<double3> &axes)
{
  const double3 candidates[6] = {
    double3(1.0, 2.0, 3.0),
    double3(-3.0, 1.0, 2.0),
    double3(2.0, -3.0, 1.0),
    double3(3.0, 2.0, -1.0),
    double3(1.0, -3.0, -2.0),
    double3(-2.0, 3.0, -1.0)
  };

  double3 polarAxis = double3::normalize(candidates[0]);
  double bestSeparation = -1.0;
  for (const double3 &candidate : candidates)
  {
    double3 direction = candidate;
    direction.normalise();
    double separation = 1.0;
    for (const double3 &axis : axes)
    {
      separation = std::min(separation, 1.0 - std::abs(double3::dot(direction, axis)));
    }
    if (separation > bestSeparation)
    {
      bestSeparation = separation;
      polarAxis = direction;
    }
    if (bestSeparation > 0.01)
    {
      break;
    }
  }

  double3 first = SKPerpendicularTo(polarAxis);
  double3 second = double3::cross(polarAxis, first);
  double secondLength = second.length();
  if (secondLength > 0.0)
  {
    second = second / secondLength;
  }
  return {first, second, polarAxis};
}

bool SKDiscWithinDisc(double cosineBetweenAxes, double cosineInner, double sineInner, double cosineOuter, double sineOuter)
{
  if (cosineOuter > cosineInner)
  {
    return false;
  }
  return cosineBetweenAxes >= cosineOuter * cosineInner + sineOuter * sineInner;
}

struct SKCapCrossing
{
  int firstCircle = 0;
  int secondCircle = 0;
  double3 direction = double3(0.0, 0.0, 0.0);
};

struct SKSweepCircle
{
  double3 axis = double3(0.0, 0.0, 0.0);
  double cosineHalfAngle = 0.0;
  double sineHalfAngle = 0.0;
  double halfAngle = 0.0;

  double polarAngle = 0.0;
  double cosinePolar = 0.0;
  double sinePolar = 0.0;
  double azimuth = 0.0;
  double cosineAzimuth = 1.0;
  double sineAzimuth = 0.0;

  double lowestLatitude = 0.0;
  double highestLatitude = 0.0;
  bool reachesOverPole = false;
  bool reachesOverAntipole = false;
};

bool SKMakeSweepCircle(const double3 &axis, double cosineHalfAngle, SKSweepCircle &circle)
{
  if (cosineHalfAngle >= 1.0)
  {
    return false;
  }
  circle = SKSweepCircle{};
  circle.axis = axis;
  circle.cosineHalfAngle = cosineHalfAngle;
  if (cosineHalfAngle <= -1.0)
  {
    circle.halfAngle = M_PI;
    circle.sineHalfAngle = 0.0;
    return true;
  }
  circle.halfAngle = std::acos(cosineHalfAngle);
  circle.sineHalfAngle = std::sin(circle.halfAngle);
  return true;
}

void SKPruneContainedDiscs(std::vector<SKSweepCircle> &circles)
{
  if (circles.size() < 2)
  {
    return;
  }
  std::vector<bool> redundant(circles.size(), false);
  for (std::size_t i = 0; i < circles.size(); ++i)
  {
    for (std::size_t j = 0; j < circles.size(); ++j)
    {
      if (i == j || redundant[j])
      {
        continue;
      }
      if (SKDiscWithinDisc(double3::dot(circles[i].axis, circles[j].axis),
                           circles[i].cosineHalfAngle,
                           circles[i].sineHalfAngle,
                           circles[j].cosineHalfAngle,
                           circles[j].sineHalfAngle))
      {
        redundant[i] = true;
        break;
      }
    }
  }

  std::vector<SKSweepCircle> kept;
  kept.reserve(circles.size());
  for (std::size_t i = 0; i < circles.size(); ++i)
  {
    if (!redundant[i])
    {
      kept.push_back(circles[i]);
    }
  }
  circles.swap(kept);
}

std::vector<SKCapCrossing> SKUncoveredCrossings(const std::vector<SKSweepCircle> &circles)
{
  std::vector<SKCapCrossing> crossings;
  if (circles.size() < 2)
  {
    return crossings;
  }
  for (std::size_t j = 0; j + 1 < circles.size(); ++j)
  {
    for (std::size_t k = j + 1; k < circles.size(); ++k)
    {
      const SKSweepCircle &first = circles[j];
      const SKSweepCircle &second = circles[k];
      double alignment = double3::dot(first.axis, second.axis);
      double denominator = 1.0 - alignment * alignment;
      if (denominator < 1.0e-14)
      {
        continue;
      }
      double alongFirst = (first.cosineHalfAngle - alignment * second.cosineHalfAngle) / denominator;
      double alongSecond = (second.cosineHalfAngle - alignment * first.cosineHalfAngle) / denominator;
      double outOfPlaneSquared = (1.0 - alongFirst * first.cosineHalfAngle - alongSecond * second.cosineHalfAngle) / denominator;
      if (outOfPlaneSquared <= 0.0)
      {
        continue;
      }
      double3 inPlane = alongFirst * first.axis + alongSecond * second.axis;
      double3 outOfPlane = std::sqrt(outOfPlaneSquared) * double3::cross(first.axis, second.axis);
      for (int side = 0; side < 2; ++side)
      {
        double3 direction = (side == 0) ? (inPlane + outOfPlane) : (inPlane - outOfPlane);
        double length = direction.length();
        if (length <= 0.0)
        {
          continue;
        }
        direction = direction / length;

        bool covered = false;
        for (std::size_t l = 0; l < circles.size(); ++l)
        {
          if (l == j || l == k)
          {
            continue;
          }
          if (double3::dot(direction, circles[l].axis) > circles[l].cosineHalfAngle + SKCapCoverTolerance)
          {
            covered = true;
            break;
          }
        }
        if (!covered)
        {
          crossings.push_back({int(j), int(k), direction});
        }
      }
    }
  }
  return crossings;
}

struct SKLatitudeGap
{
  double sineLatitude = 0.0;
  double cosineLatitude = 0.0;
  double weight = 0.0;
  double begin = 0.0;
  double end = 0.0;
  double span = 0.0;
  double cosineBegin = 1.0;
  double sineBegin = 0.0;
  double cosineEnd = 1.0;
  double sineEnd = 0.0;
};

struct SKCoveredArc
{
  double begin = 0.0;
  double end = 0.0;
  double cosineBegin = 1.0;
  double sineBegin = 0.0;
  double cosineEnd = 1.0;
  double sineEnd = 0.0;
};

struct SKSweepWorkspace
{
  std::vector<double3> axes;
  std::vector<SKCapCrossing> crossings;
  std::vector<double> breakpoints;
  std::vector<int> cutting;
  std::vector<double> panels;
  std::vector<SKCoveredArc> covered;
};

struct SKGaussRule
{
  std::array<double, SKExactQuadratureOrder> nodes{};
  std::array<double, SKExactQuadratureOrder> weights{};
};

const SKGaussRule &SKUnitIntervalGaussRule()
{
  static const SKGaussRule rule = []()
  {
    SKGaussRule constructed;
    double order = double(SKExactQuadratureOrder);
    for (int i = 0; i < SKExactQuadratureOrder; ++i)
    {
      double abscissa = std::cos(M_PI * (double(i) + 0.75) / (order + 0.5));
      double derivative = 0.0;
      for (int iter = 0; iter < 100; ++iter)
      {
        double previous = 1.0;
        double current = abscissa;
        for (int k = 2; k <= SKExactQuadratureOrder; ++k)
        {
          double next = ((2.0 * double(k) - 1.0) * abscissa * current - (double(k) - 1.0) * previous) / double(k);
          previous = current;
          current = next;
        }
        derivative = order * (abscissa * current - previous) / (abscissa * abscissa - 1.0);
        double step = current / derivative;
        abscissa -= step;
        if (std::abs(step) < 1.0e-15)
        {
          break;
        }
      }
      constructed.nodes[i] = 0.5 * (1.0 - abscissa);
      constructed.weights[i] = 1.0 / ((1.0 - abscissa * abscissa) * derivative * derivative);
    }
    return constructed;
  }();
  return rule;
}

void SKPrepareSweep(std::vector<SKSweepCircle> &circles,
                    const SweepFrame &frame,
                    SKSweepWorkspace &work)
{
  work.breakpoints.clear();
  work.breakpoints.push_back(0.0);
  work.breakpoints.push_back(M_PI);

  for (SKSweepCircle &circle : circles)
  {
    circle.polarAngle = std::acos(std::clamp(double3::dot(circle.axis, frame.polar), -1.0, 1.0));
    circle.cosinePolar = std::cos(circle.polarAngle);
    circle.sinePolar = std::sin(circle.polarAngle);
    circle.azimuth = std::atan2(double3::dot(circle.axis, frame.second), double3::dot(circle.axis, frame.first));
    if (circle.azimuth < 0.0)
    {
      circle.azimuth += 2.0 * M_PI;
    }
    circle.cosineAzimuth = std::cos(circle.azimuth);
    circle.sineAzimuth = std::sin(circle.azimuth);

    circle.lowestLatitude = SKFoldedPolarAngle(circle.polarAngle - circle.halfAngle);
    circle.highestLatitude = SKFoldedPolarAngle(circle.polarAngle + circle.halfAngle);
    circle.reachesOverPole = circle.polarAngle < circle.halfAngle;
    circle.reachesOverAntipole = M_PI - circle.polarAngle < circle.halfAngle;

    work.breakpoints.push_back(circle.lowestLatitude);
    work.breakpoints.push_back(circle.highestLatitude);
  }

  std::sort(circles.begin(), circles.end(), [](const SKSweepCircle &a, const SKSweepCircle &b)
  {
    return a.azimuth < b.azimuth;
  });

  work.crossings = SKUncoveredCrossings(circles);
  for (const SKCapCrossing &crossing : work.crossings)
  {
    work.breakpoints.push_back(std::acos(std::clamp(double3::dot(crossing.direction, frame.polar), -1.0, 1.0)));
  }

  std::sort(work.breakpoints.begin(), work.breakpoints.end());
}

void SKPanelBoundaries(double begin, double end, int subdivisions, bool cut, std::vector<double> &panels)
{
  int parts = std::max(1, subdivisions);
  panels.clear();
  for (int part = 0; part <= parts; ++part)
  {
    panels.push_back(begin + (end - begin) * double(part) / double(parts));
  }
  panels.front() = begin;
  panels.back() = end;

  if (!cut)
  {
    return;
  }

  const int halvingLimit = 60;
  double roomBelow = begin;
  for (int iter = 0; iter < halvingLimit; ++iter)
  {
    if (panels[1] - panels[0] <= SKPoleClearance * roomBelow)
    {
      break;
    }
    panels.insert(panels.begin() + 1, 0.5 * (panels[0] + panels[1]));
  }

  double roomAbove = M_PI - end;
  for (int iter = 0; iter < halvingLimit; ++iter)
  {
    std::size_t last = panels.size() - 1;
    if (panels[last] - panels[last - 1] <= SKPoleClearance * roomAbove)
    {
      break;
    }
    panels.insert(panels.begin() + static_cast<std::ptrdiff_t>(last), 0.5 * (panels[last - 1] + panels[last]));
  }
}

void sortByBeginning(std::vector<SKCoveredArc> &arcs)
{
  if (arcs.size() < 2)
  {
    return;
  }
  for (std::size_t i = 1; i < arcs.size(); ++i)
  {
    SKCoveredArc held = arcs[i];
    std::size_t j = i;
    while (j > 0 && arcs[j - 1].begin > held.begin)
    {
      arcs[j] = arcs[j - 1];
      --j;
    }
    arcs[j] = held;
  }
}

template <typename MeasureFunc>
void SKSweepExposedLatitudes(std::vector<SKSweepCircle> &circles,
                             const SweepFrame &frame,
                             int subdivisions,
                             SKSweepWorkspace &work,
                             MeasureFunc &&measure)
{
  SKPrepareSweep(circles, frame, work);

  const SKGaussRule &rule = SKUnitIntervalGaussRule();
  const double twoPi = 2.0 * M_PI;
  int parts = std::max(1, subdivisions);

  for (std::size_t piece = 0; piece + 1 < work.breakpoints.size(); ++piece)
  {
    double pieceBegin = work.breakpoints[piece];
    double pieceEnd = work.breakpoints[piece + 1];
    if (pieceEnd - pieceBegin < 1.0e-14)
    {
      continue;
    }

    double interior = 0.5 * (pieceBegin + pieceEnd);
    work.cutting.clear();
    bool buried = false;
    for (std::size_t i = 0; i < circles.size(); ++i)
    {
      const SKSweepCircle &circle = circles[i];
      if (interior > circle.lowestLatitude && interior < circle.highestLatitude)
      {
        work.cutting.push_back(int(i));
      }
      else if ((interior <= circle.lowestLatitude && circle.reachesOverPole) ||
               (interior >= circle.highestLatitude && circle.reachesOverAntipole))
      {
        buried = true;
        break;
      }
    }
    if (buried)
    {
      continue;
    }

    SKPanelBoundaries(pieceBegin, pieceEnd, parts, !work.cutting.empty(), work.panels);

    for (std::size_t panel = 0; panel + 1 < work.panels.size(); ++panel)
    {
      double begin = work.panels[panel];
      double end = work.panels[panel + 1];
      double middle = 0.5 * (begin + end);

      for (int half = 0; half < 2; ++half)
      {
        double anchor = (half == 0) ? begin : end;
        double span = (half == 0) ? (middle - begin) : (end - middle);
        double direction = (half == 0) ? 1.0 : -1.0;

        for (int node = 0; node < SKExactQuadratureOrder; ++node)
        {
          double parameter = rule.nodes[node];
          double latitude = anchor + direction * span * parameter * parameter;
          double sineLatitude = std::sin(latitude);
          if (sineLatitude <= 0.0)
          {
            continue;
          }

          SKLatitudeGap gap;
          gap.sineLatitude = sineLatitude;
          gap.cosineLatitude = std::cos(latitude);
          gap.weight = 2.0 * span * parameter * rule.weights[node];

          work.covered.clear();
          for (int idx : work.cutting)
          {
            const SKSweepCircle &circle = circles[idx];
            double cosineHalfWidth = (circle.cosineHalfAngle - gap.cosineLatitude * circle.cosinePolar) / (sineLatitude * circle.sinePolar);
            if (cosineHalfWidth >= 1.0)
            {
              continue;
            }

            bool whole = cosineHalfWidth <= -1.0;
            double halfWidth = whole ? M_PI : std::acos(cosineHalfWidth);
            double cosineHalfWidthClamped = whole ? -1.0 : cosineHalfWidth;
            double sineHalfWidth = whole ? 0.0 : std::sqrt(std::max(0.0, 1.0 - cosineHalfWidth * cosineHalfWidth));

            SKCoveredArc arc;
            arc.begin = circle.azimuth - halfWidth;
            arc.end = circle.azimuth + halfWidth;
            arc.cosineBegin = circle.cosineAzimuth * cosineHalfWidthClamped + circle.sineAzimuth * sineHalfWidth;
            arc.sineBegin = circle.sineAzimuth * cosineHalfWidthClamped - circle.cosineAzimuth * sineHalfWidth;
            arc.cosineEnd = circle.cosineAzimuth * cosineHalfWidthClamped - circle.sineAzimuth * sineHalfWidth;
            arc.sineEnd = circle.sineAzimuth * cosineHalfWidthClamped + circle.cosineAzimuth * sineHalfWidth;

            if (arc.begin < 0.0)
            {
              work.covered.push_back({arc.begin + twoPi, twoPi, arc.cosineBegin, arc.sineBegin, 1.0, 0.0});
              work.covered.push_back({0.0, arc.end, 1.0, 0.0, arc.cosineEnd, arc.sineEnd});
            }
            else if (arc.end > twoPi)
            {
              work.covered.push_back({arc.begin, twoPi, arc.cosineBegin, arc.sineBegin, 1.0, 0.0});
              work.covered.push_back({0.0, arc.end - twoPi, 1.0, 0.0, arc.cosineEnd, arc.sineEnd});
            }
            else
            {
              work.covered.push_back(arc);
            }
          }
          sortByBeginning(work.covered);

          double cursor = 0.0;
          double cosineCursor = 1.0;
          double sineCursor = 0.0;
          for (std::size_t arcIndex = 0; arcIndex <= work.covered.size(); ++arcIndex)
          {
            bool last = (arcIndex == work.covered.size());
            double gapEnd = last ? twoPi : work.covered[arcIndex].begin;

            if (gapEnd - cursor > SKSweepGapTolerance)
            {
              gap.begin = cursor;
              gap.end = gapEnd;
              gap.span = gapEnd - cursor;
              gap.cosineBegin = cosineCursor;
              gap.sineBegin = sineCursor;
              gap.cosineEnd = last ? 1.0 : work.covered[arcIndex].cosineBegin;
              gap.sineEnd = last ? 0.0 : work.covered[arcIndex].sineBegin;
              measure(gap);
            }

            if (!last && work.covered[arcIndex].end > cursor)
            {
              cursor = work.covered[arcIndex].end;
              cosineCursor = work.covered[arcIndex].cosineEnd;
              sineCursor = work.covered[arcIndex].sineEnd;
            }
          }
        }
      }
    }
  }
}
} // namespace

double3 SKGeometricSurface::wrappedFractional(double3 s)
{
  return double3(s.x - std::floor(s.x), s.y - std::floor(s.y), s.z - std::floor(s.z));
}

SKGeometricSurface SKGeometricSurface::build(const std::vector<double3> &fractionalPositions,
                                             const std::vector<double> &radii,
                                             const std::shared_ptr<SKCell> &cell,
                                             const std::vector<double4> &blockingPockets,
                                             int subdivisions)
{
  std::size_t count = std::min(fractionalPositions.size(), radii.size());
  if (count == 0 || !cell)
  {
    return SKGeometricSurface({}, 0.0);
  }

  double3x3 unitCell = cell->unitCell();
  std::vector<double3> centres;
  std::vector<double> usedRadii;
  centres.reserve(count);
  usedRadii.reserve(count);
  for (std::size_t i = 0; i < count; ++i)
  {
    double3 wrapped = SKGeometricSurface::wrappedFractional(fractionalPositions[i]);
    centres.push_back(unitCell * wrapped);
    usedRadii.push_back(radii[i]);
  }

  std::vector<double3> clipperCentres = centres;
  std::vector<double> clipperRadii = usedRadii;
  clipperCentres.reserve(count + blockingPockets.size());
  clipperRadii.reserve(count + blockingPockets.size());
  for (const double4 &pocket : blockingPockets)
  {
    if (pocket.w <= 0.0)
    {
      continue;
    }
    double3 wrapped = SKGeometricSurface::wrappedFractional(double3(pocket.x, pocket.y, pocket.z));
    clipperCentres.push_back(unitCell * wrapped);
    clipperRadii.push_back(pocket.w);
  }

  double maxRadius = 0.0;
  for (double r : usedRadii) maxRadius = std::max(maxRadius, r);
  double maxClipperRadius = maxRadius;
  for (double r : clipperRadii) maxClipperRadius = std::max(maxClipperRadius, r);
  int3 replicas = cell->numberOfReplicas(std::max(maxRadius + maxClipperRadius, 1.0));

  std::vector<SKGeometricSurfacePatch> patches;
  patches.reserve(count);
  double totalArea = 0.0;
  SKSweepWorkspace work;

  for (std::size_t atomIndex = 0; atomIndex < count; ++atomIndex)
  {
    double radius = usedRadii[atomIndex];
    if (radius <= 0.0)
    {
      continue;
    }

    std::vector<SKSweepCircle> circles;
    std::vector<SKGeometricSurfaceClip> clips;
    bool buried = false;

    int nx = replicas.x;
    int ny = replicas.y;
    int nz = replicas.z;
    for (std::size_t j = 0; j < clipperCentres.size() && !buried; ++j)
    {
      double neighbourRadius = clipperRadii[j];
      if (neighbourRadius <= 0.0)
      {
        continue;
      }
      for (int kx = -nx; kx <= nx && !buried; ++kx)
      {
        for (int ky = -ny; ky <= ny && !buried; ++ky)
        {
          for (int kz = -nz; kz <= nz; ++kz)
          {
            if (j == atomIndex && kx == 0 && ky == 0 && kz == 0)
            {
              continue;
            }
            double3 image{double(kx), double(ky), double(kz)};
            double3 neighbourCentre = clipperCentres[j] + unitCell * image;
            double3 delta = neighbourCentre - centres[atomIndex];
            double distance = delta.length();
            if (distance < 1.0e-12)
            {
              if (neighbourRadius > radius)
              {
                buried = true;
                break;
              }
              continue;
            }

            double cosineHalfAngle = (radius * radius + distance * distance - neighbourRadius * neighbourRadius) / (2.0 * radius * distance);
            if (cosineHalfAngle >= 1.0)
            {
              continue;
            }
            if (cosineHalfAngle <= -1.0)
            {
              buried = true;
              break;
            }

            double3 axis = delta / distance;
            SKSweepCircle circle;
            if (SKMakeSweepCircle(axis, cosineHalfAngle, circle))
            {
              circles.push_back(circle);
              clips.emplace_back(neighbourCentre, neighbourRadius);
            }
          }
        }
      }
    }

    if (buried)
    {
      continue;
    }

    SKPruneContainedDiscs(circles);

    double area = 0.0;
    if (circles.empty())
    {
      area = 4.0 * M_PI * radius * radius;
    }
    else
    {
      work.axes.clear();
      work.axes.reserve(circles.size());
      for (const SKSweepCircle &c : circles)
      {
        work.axes.push_back(c.axis);
      }
      SweepFrame frame = SKSweepFrame(work.axes);
      SKSweepExposedLatitudes(circles, frame, subdivisions, work, [&](const SKLatitudeGap &gap)
      {
        area += radius * radius * gap.sineLatitude * gap.span * gap.weight;
      });
    }

    if (area <= 0.0)
    {
      continue;
    }

    patches.emplace_back(int(atomIndex), centres[atomIndex], radius, std::move(clips), area);
    totalArea += area;
  }

  return SKGeometricSurface(std::move(patches), totalArea);
}

SKGeometricSurface SKGeometricSurface::build(const std::vector<double3> &fractionalPositions,
                                             const std::vector<double2> &potentialParameters,
                                             double probeSigma,
                                             const std::shared_ptr<SKCell> &cell,
                                             const std::vector<double4> &blockingPockets,
                                             int subdivisions)
{
  std::size_t count = std::min(fractionalPositions.size(), potentialParameters.size());
  std::vector<double> radii;
  radii.reserve(count);
  for (std::size_t i = 0; i < count; ++i)
  {
    radii.push_back(inflatedRadius(potentialParameters[i].y, probeSigma));
  }
  std::vector<double3> positions(fractionalPositions.begin(), fractionalPositions.begin() + static_cast<std::ptrdiff_t>(count));
  return build(positions, radii, cell, blockingPockets, subdivisions);
}

SKGeometricSurface SKGeometricSurface::buildVanDerWaals(const std::vector<double3> &fractionalPositions,
                                                        const std::vector<int> &elementIdentifiers,
                                                        double probeSigma,
                                                        const std::shared_ptr<SKCell> &cell,
                                                        const std::vector<double4> &blockingPockets,
                                                        int subdivisions)
{
  std::size_t count = std::min(fractionalPositions.size(), elementIdentifiers.size());
  const std::vector<SKElement> &elements = PredefinedElements::predefinedElements;
  std::vector<double> radii;
  radii.reserve(count);
  for (std::size_t i = 0; i < count; ++i)
  {
    int elementId = elementIdentifiers[i];
    double vdw = (elementId >= 0 && static_cast<std::size_t>(elementId) < elements.size()) ? elements[static_cast<std::size_t>(elementId)]._VDWRadius : 0.0;
    radii.push_back(inflatedVanDerWaalsRadius(vdw, probeSigma));
  }
  std::vector<double3> positions(fractionalPositions.begin(), fractionalPositions.begin() + static_cast<std::ptrdiff_t>(count));
  return build(positions, radii, cell, blockingPockets, subdivisions);
}

double3 SKGeometricSurfacePatch::fractionalExtent(double radius, const double3x3 &inverseUnitCell)
{
  const double3x3 &inv = inverseUnitCell;
  return double3(radius * double3(inv.ax, inv.bx, inv.cx).length(),
                 radius * double3(inv.ay, inv.by, inv.cy).length(),
                 radius * double3(inv.az, inv.bz, inv.cz).length());
}

std::vector<SKGeometricSurfacePatchCopy> SKGeometricSurfacePatch::copiesInsideUnitCell(const std::shared_ptr<SKCell> &cell) const
{
  if (!cell)
  {
    return {};
  }
  double3x3 inverse = cell->inverseUnitCell();
  double3x3 unitCell = cell->unitCell();
  double3 fractional = SKGeometricSurface::wrappedFractional(inverse * center);
  double3 wrappedCenter = unitCell * fractional;
  double3 wrapShift = wrappedCenter - center;
  std::vector<SKGeometricSurfaceClip> wrappedClips;
  wrappedClips.reserve(clips.size());
  for (const SKGeometricSurfaceClip &clip : clips)
  {
    wrappedClips.emplace_back(clip.center + wrapShift, clip.radius);
  }

  double3 extent = SKGeometricSurfacePatch::fractionalExtent(radius, inverse);
  int nx = std::max(1, int(std::ceil(extent.x)));
  int ny = std::max(1, int(std::ceil(extent.y)));
  int nz = std::max(1, int(std::ceil(extent.z)));

  std::vector<SKGeometricSurfacePatchCopy> copies;
  for (int wx = -nx; wx <= nx; ++wx)
  {
    for (int wy = -ny; wy <= ny; ++wy)
    {
      for (int wz = -nz; wz <= nz; ++wz)
      {
        double3 offset{double(wx), double(wy), double(wz)};
        double3 image = fractional + offset;
        if (image.x + extent.x > 0.0 && image.x - extent.x < 1.0 &&
            image.y + extent.y > 0.0 && image.y - extent.y < 1.0 &&
            image.z + extent.z > 0.0 && image.z - extent.z < 1.0)
        {
          double3 shift = unitCell * offset;
          std::vector<SKGeometricSurfaceClip> shiftedClips;
          shiftedClips.reserve(wrappedClips.size());
          for (const SKGeometricSurfaceClip &clip : wrappedClips)
          {
            shiftedClips.emplace_back(clip.center + shift, clip.radius);
          }
          copies.emplace_back(wrappedCenter + shift, std::move(shiftedClips), double3(0.0, 0.0, 0.0));
        }
      }
    }
  }
  return copies;
}
