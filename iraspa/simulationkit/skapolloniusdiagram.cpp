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

#include "skapolloniusdiagram.h"
#include "skvoronoi.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace
{
constexpr double EPSILON = 1e-9;

using GeneratorID = std::pair<std::size_t, int3>;

struct GeneratorIDLess
{
  bool operator()(const GeneratorID &a, const GeneratorID &b) const
  {
    if (a.first != b.first) return a.first < b.first;
    if (a.second.x != b.second.x) return a.second.x < b.second.x;
    if (a.second.y != b.second.y) return a.second.y < b.second.y;
    return a.second.z < b.second.z;
  }
};

bool solve3x3(const double A[3][3], const double b[3], double x[3])
{
  double M[3][4];
  for (int r = 0; r < 3; ++r)
  {
    for (int c = 0; c < 3; ++c) M[r][c] = A[r][c];
    M[r][3] = b[r];
  }

  for (int col = 0; col < 3; ++col)
  {
    int pivot = col;
    double maxVal = std::abs(M[col][col]);
    for (int row = col + 1; row < 3; ++row)
    {
      if (std::abs(M[row][col]) > maxVal)
      {
        maxVal = std::abs(M[row][col]);
        pivot = row;
      }
    }
    if (maxVal < 1e-11) return false;
    if (pivot != col)
    {
      for (int c = col; c < 4; ++c) std::swap(M[col][c], M[pivot][c]);
    }
    for (int row = col + 1; row < 3; ++row)
    {
      double factor = M[row][col] / M[col][col];
      for (int c = col; c < 4; ++c)
      {
        M[row][c] -= factor * M[col][c];
      }
    }
  }

  for (int row = 2; row >= 0; --row)
  {
    double sum = M[row][3];
    for (int c = row + 1; c < 3; ++c)
    {
      sum -= M[row][c] * x[c];
    }
    x[row] = sum / M[row][row];
  }
  return true;
}

bool refineOnTripletBisector(double3 &pos,
                             double &clearance,
                             const std::array<double3, 3> &centers,
                             const std::array<double, 3> &radii)
{
  for (int iter = 0; iter < 15; ++iter)
  {
    double3 d0 = pos - centers[0];
    double3 d1 = pos - centers[1];
    double3 d2 = pos - centers[2];
    double l0 = d0.length();
    double l1 = d1.length();
    double l2 = d2.length();
    if (l0 < EPSILON || l1 < EPSILON || l2 < EPSILON) return false;

    double f1 = (l1 - radii[1]) - (l0 - radii[0]);
    double f2 = (l2 - radii[2]) - (l0 - radii[0]);
    if (std::abs(f1) < 1e-12 && std::abs(f2) < 1e-12)
    {
      clearance = l0 - radii[0];
      return true;
    }

    double3 g1 = (d1 / l1) - (d0 / l0);
    double3 g2 = (d2 / l2) - (d0 / l0);

    double g11 = double3::dot(g1, g1);
    double g12 = double3::dot(g1, g2);
    double g22 = double3::dot(g2, g2);
    double det = g11 * g22 - g12 * g12;
    if (std::abs(det) < 1e-18) break;

    double alpha = (g22 * f1 - g12 * f2) / det;
    double beta = (-g12 * f1 + g11 * f2) / det;
    pos = pos - (alpha * g1 + beta * g2);
  }
  clearance = (pos - centers[0]).length() - radii[0];
  return true;
}

SKApolloniusDiagram::EdgeCurvatureType classifyTripletEdge(const std::array<double3, 3> &centers,
                                                           const std::array<double, 3> &radii)
{
  bool r01 = std::abs(radii[0] - radii[1]) < 1e-6;
  bool r12 = std::abs(radii[1] - radii[2]) < 1e-6;
  if (r01 && r12)
  {
    return SKApolloniusDiagram::EdgeCurvatureType::StraightLine;
  }

  double3 v1 = centers[1] - centers[0];
  double3 v2 = centers[2] - centers[0];
  double3 planeNormal = double3::cross(v1, v2);
  double normalLen = planeNormal.length();
  if (normalLen < 1e-9)
  {
    return SKApolloniusDiagram::EdgeCurvatureType::Hyperbola;
  }

  if ((r01 && !r12) || (r12 && !r01) || (std::abs(radii[0] - radii[2]) < 1e-6 && !r01))
  {
    return SKApolloniusDiagram::EdgeCurvatureType::Hyperbola;
  }

  return SKApolloniusDiagram::EdgeCurvatureType::Hyperbola;
}

bool solvePointOnBisectorWithPlane(const std::array<double3, 3> &centers,
                                   const std::array<double, 3> &radii,
                                   const double3 &planeNormal,
                                   double planeD,
                                   double3 &pos,
                                   double &clearance)
{
  for (int iter = 0; iter < 25; ++iter)
  {
    double3 d0 = pos - centers[0];
    double3 d1 = pos - centers[1];
    double3 d2 = pos - centers[2];
    double l0 = d0.length();
    double l1 = d1.length();
    double l2 = d2.length();
    if (l0 < EPSILON || l1 < EPSILON || l2 < EPSILON) return false;

    double f[3] = {
        (l1 - radii[1]) - (l0 - radii[0]),
        (l2 - radii[2]) - (l0 - radii[0]),
        double3::dot(planeNormal, pos) - planeD
    };

    if (std::abs(f[0]) < 1e-11 && std::abs(f[1]) < 1e-11 && std::abs(f[2]) < 1e-11)
    {
      clearance = l0 - radii[0];
      return true;
    }

    double3 g0 = (d1 / l1) - (d0 / l0);
    double3 g1 = (d2 / l2) - (d0 / l0);
    double J[3][3] = {
        {g0.x, g0.y, g0.z},
        {g1.x, g1.y, g1.z},
        {planeNormal.x, planeNormal.y, planeNormal.z}
    };
    double rhs[3] = {-f[0], -f[1], -f[2]};
    double step[3] = {0.0, 0.0, 0.0};
    if (!solve3x3(J, rhs, step))
    {
      return refineOnTripletBisector(pos, clearance, centers, radii);
    }
    pos += double3(step[0], step[1], step[2]);
  }
  clearance = (pos - centers[0]).length() - radii[0];
  double err1 = std::abs(((pos - centers[1]).length() - radii[1]) - clearance);
  double err2 = std::abs(((pos - centers[2]).length() - radii[2]) - clearance);
  return (err1 < 1e-5 && err2 < 1e-5);
}

struct TripletConicParam
{
  bool valid{false};
  int refIdx{0};
  double3 cRef;
  double rRef{0.0};
  double3 p0;
  double3 vDir;
  double a{0.0};
  double b{0.0};
  double c{0.0};
};

TripletConicParam buildTripletConicParam(const std::array<double3, 3> &centers,
                                         const std::array<double, 3> &radii)
{
  TripletConicParam param;
  int ref = 0;
  if (radii[1] < radii[ref]) ref = 1;
  if (radii[2] < radii[ref]) ref = 2;

  int i1 = (ref + 1) % 3;
  int i2 = (ref + 2) % 3;

  param.refIdx = ref;
  param.cRef = centers[static_cast<std::size_t>(ref)];
  param.rRef = radii[static_cast<std::size_t>(ref)];

  double3 d1 = centers[static_cast<std::size_t>(i1)] - param.cRef;
  double3 d2 = centers[static_cast<std::size_t>(i2)] - param.cRef;
  double dr1 = radii[static_cast<std::size_t>(i1)] - param.rRef;
  double dr2 = radii[static_cast<std::size_t>(i2)] - param.rRef;

  double3 n = double3::cross(d1, d2);
  double nSq = double3::dot(n, n);
  if (nSq < 1e-14)
  {
    return param;
  }

  double rhs0_1 = 0.5 * (double3::dot(d1, d1) - dr1 * dr1);
  double rhs0_2 = 0.5 * (double3::dot(d2, d2) - dr2 * dr2);

  double d1Sq = double3::dot(d1, d1);
  double d2Sq = double3::dot(d2, d2);
  double d12 = double3::dot(d1, d2);

  double alpha0 = (rhs0_1 * d2Sq - rhs0_2 * d12) / nSq;
  double beta0 = (rhs0_2 * d1Sq - rhs0_1 * d12) / nSq;
  param.p0 = alpha0 * d1 + beta0 * d2;

  double alphaV = (-dr1 * d2Sq + dr2 * d12) / nSq;
  double betaV = (-dr2 * d1Sq + dr1 * d12) / nSq;
  param.vDir = alphaV * d1 + betaV * d2;

  param.a = double3::dot(param.vDir, param.vDir) - 1.0;
  param.b = double3::dot(param.p0, param.vDir);
  param.c = double3::dot(param.p0, param.p0);
  param.valid = true;
  return param;
}

bool evaluateTripletConicAtS(const TripletConicParam &param,
                             const std::array<double3, 3> &centers,
                             const std::array<double, 3> &radii,
                             double s,
                             const double3 &hintPos,
                             double3 &outPos,
                             double &outClearance)
{
  if (!param.valid || s < 0.0) return false;
  double3 nUnit = double3::normalize(double3::cross(
      centers[static_cast<std::size_t>((param.refIdx + 1) % 3)] - param.cRef,
      centers[static_cast<std::size_t>((param.refIdx + 2) % 3)] - param.cRef));

  double hSq = s * s - (param.a * s * s + 2.0 * param.b * s + param.c);
  if (hSq < -1e-7) return false;
  double h = std::sqrt(std::max(0.0, hSq));

  double3 base = param.cRef + param.p0 + s * param.vDir;
  double3 candPlus = base + h * nUnit;
  double3 candMinus = base - h * nUnit;

  double distPlusSq = double3::dot(candPlus - hintPos, candPlus - hintPos);
  double distMinusSq = double3::dot(candMinus - hintPos, candMinus - hintPos);
  outPos = (distPlusSq <= distMinusSq) ? candPlus : candMinus;
  outClearance = s - param.rRef;

  refineOnTripletBisector(outPos, outClearance, centers, radii);
  return true;
}

} // namespace

std::vector<std::pair<double3, double>> SKApolloniusDiagram::solveApolloniusQuadruplet(
    const std::array<double3, 4> &centers,
    const std::array<double, 4> &radii)
{
  std::vector<std::pair<double3, double>> solutions;

  int ref = 0;
  for (int i = 1; i < 4; ++i)
  {
    if (radii[static_cast<std::size_t>(i)] < radii[static_cast<std::size_t>(ref)])
    {
      ref = i;
    }
  }

  const double3 &c0 = centers[static_cast<std::size_t>(ref)];
  double r0 = radii[static_cast<std::size_t>(ref)];

  double M[3][3];
  double rhsConst[3];
  double rhsR[3];

  int row = 0;
  for (int i = 0; i < 4; ++i)
  {
    if (i == ref) continue;
    double3 dc = centers[static_cast<std::size_t>(i)] - c0;
    double dr = radii[static_cast<std::size_t>(i)] - r0;

    M[row][0] = dc.x;
    M[row][1] = dc.y;
    M[row][2] = dc.z;

    rhsConst[row] = 0.5 * (double3::dot(dc, dc) - dr * dr);
    rhsR[row] = -dr;
    ++row;
  }

  double p0Arr[3] = {0.0, 0.0, 0.0};
  double vArr[3] = {0.0, 0.0, 0.0};
  if (!solve3x3(M, rhsConst, p0Arr) || !solve3x3(M, rhsR, vArr))
  {
    return solutions;
  }

  double3 p0(p0Arr[0], p0Arr[1], p0Arr[2]);
  double3 v(vArr[0], vArr[1], vArr[2]);

  double a = double3::dot(v, v) - 1.0;
  double b = double3::dot(p0, v);
  double c = double3::dot(p0, p0);

  std::vector<double> candidateS;
  if (std::abs(a) < 1e-11)
  {
    if (std::abs(b) > 1e-11)
    {
      candidateS.push_back(-c / (2.0 * b));
    }
  }
  else
  {
    double disc = b * b - a * c;
    if (disc >= -1e-9)
    {
      double sqrtDisc = std::sqrt(std::max(0.0, disc));
      candidateS.push_back((-b - sqrtDisc) / a);
      if (sqrtDisc > 1e-9)
      {
        candidateS.push_back((-b + sqrtDisc) / a);
      }
    }
  }

  for (double s : candidateS)
  {
    if (s < -1e-7) continue;
    double rTangent = s - r0;
    double3 center = c0 + p0 + s * v;

    bool valid = true;
    for (std::size_t i = 0; i < 4; ++i)
    {
      double dist = (center - centers[i]).length();
      double expected = rTangent + radii[i];
      if ( expected < -1e-6 || std::abs(dist - expected) > 1e-4)
      {
        valid = false;
        break;
      }
    }

    if (valid)
    {
      for (int iter = 0; iter < 8; ++iter)
      {
        double3 d0 = center - centers[0];
        double l0 = d0.length();
        if (l0 < EPSILON) break;
        double J[3][3];
        double f[3];
        bool ok = true;
        for (int k = 1; k <= 3; ++k)
        {
          double3 dk = center - centers[static_cast<std::size_t>(k)];
          double lk = dk.length();
          if (lk < EPSILON) { ok = false; break; }
          f[k - 1] = -((lk - radii[static_cast<std::size_t>(k)]) - (l0 - radii[0]));
          double3 gk = (dk / lk) - (d0 / l0);
          J[k - 1][0] = gk.x;
          J[k - 1][1] = gk.y;
          J[k - 1][2] = gk.z;
        }
        if (!ok) break;
        double step[3] = {0.0, 0.0, 0.0};
        if (!solve3x3(J, f, step)) break;
        center += double3(step[0], step[1], step[2]);
      }
      rTangent = (center - centers[0]).length() - radii[0];

      bool duplicate = false;
      for (const auto &existing : solutions)
      {
        if ((existing.first - center).length_squared() < 1e-10)
        {
          duplicate = true;
          break;
        }
      }
      if (!duplicate)
      {
        solutions.emplace_back(center, rTangent);
      }
    }
  }

  return solutions;
}
namespace
{
struct CurvedSurfaceFrame
{
  double3 cA;
  double rA;
  double3 cB;
  double rB;
  double3 axis;
  double3 u;
  double3 v;
  double d;
  double dr; // rA - rB
  bool isPlanar;
};

CurvedSurfaceFrame makeCurvedSurfaceFrame(const double3 &cA, double rA, const double3 &cB, double rB)
{
  CurvedSurfaceFrame f;
  f.cA = cA;
  f.rA = rA;
  f.cB = cB;
  f.rB = rB;
  double3 diff = cB - cA;
  f.d = diff.length();
  f.axis = (f.d > EPSILON) ? (diff / f.d) : double3(1.0, 0.0, 0.0);
  double3 ref = (std::abs(f.axis.x) < 0.9) ? double3(1.0, 0.0, 0.0) : double3(0.0, 1.0, 0.0);
  f.u = double3::normalize(double3::cross(f.axis, ref));
  f.v = double3::cross(f.axis, f.u);
  f.dr = rA - rB;
  f.isPlanar = (std::abs(f.dr) < 1e-7 || f.d <= std::abs(f.dr) + 1e-6);
  return f;
}

std::pair<double, double> projectToBisectorUV(const CurvedSurfaceFrame &f, const double3 &pos)
{
  double3 rel = pos - f.cA;
  return {double3::dot(rel, f.u), double3::dot(rel, f.v)};
}

double3 liftFromBisectorUV(const CurvedSurfaceFrame &f, double uCoord, double vCoord)
{
  double rhoSq = uCoord * uCoord + vCoord * vCoord;
  double zCoord = 0.5 * f.d;

  if (!f.isPlanar && f.d > std::abs(f.dr) + 1e-6)
  {
    double a = f.dr * 0.5; // half of (rA - rB)
    double c = f.d * 0.5;
    double bSq = c * c - a * a;
    if (bSq > 1e-10)
    {
      double offset = std::abs(a) * std::sqrt(1.0 + rhoSq / bSq);
      zCoord = (f.dr > 0.0) ? (c + offset) : (c - offset);
    }
  }

  return f.cA + zCoord * f.axis + uCoord * f.u + vCoord * f.v;
}

double3 bisectorNormalAt(const CurvedSurfaceFrame &f, const double3 &pos)
{
  double3 dA = pos - f.cA;
  double3 dB = pos - f.cB;
  double lA = dA.length();
  double lB = dB.length();
  if (lA < EPSILON || lB < EPSILON) return f.axis;
  double3 g = (dB / lB) - (dA / lA);
  double gLen = g.length();
  return (gLen > EPSILON) ? (-g / gLen) : f.axis;
}

void triangulateCurvedFace(SKApolloniusDiagram::ApolloniusFace &face,
                           const std::vector<double3> &loopPoints,
                           const double3 &cA, double rA,
                           const double3 &cB, double rB,
                           std::size_t gridSteps)
{
  face.triangles.clear();
  face.triangleNormals.clear();
  if (loopPoints.size() < 3) return;

  CurvedSurfaceFrame frame = makeCurvedSurfaceFrame(cA, rA, cB, rB);

  std::size_t n = loopPoints.size();
  std::vector<std::pair<double, double>> uvLoop(n);
  double centerU = 0.0, centerV = 0.0;
  for (std::size_t i = 0; i < n; ++i)
  {
    uvLoop[i] = projectToBisectorUV(frame, loopPoints[i]);
    centerU += uvLoop[i].first;
    centerV += uvLoop[i].second;
  }
  centerU /= static_cast<double>(n);
  centerV /= static_cast<double>(n);

  std::size_t rings = frame.isPlanar ? 1 : std::max<std::size_t>(2, gridSteps / 4);

  auto getPoint = [&](std::size_t r, std::size_t idx) -> double3
  {
    if (r == 0)
    {
      return liftFromBisectorUV(frame, centerU, centerV);
    }
    if (r == rings)
    {
      return loopPoints[idx % n];
    }
    double frac = static_cast<double>(r) / static_cast<double>(rings);
    double u = centerU + frac * (uvLoop[idx % n].first - centerU);
    double v = centerV + frac * (uvLoop[idx % n].second - centerV);
    return liftFromBisectorUV(frame, u, v);
  };

  auto emitTriangle = [&](const double3 &p0, const double3 &p1, const double3 &p2)
  {
    if ((p1 - p0).length_squared() < 1e-14 ||
        (p2 - p1).length_squared() < 1e-14 ||
        (p0 - p2).length_squared() < 1e-14)
    {
      return;
    }
    face.triangles.push_back(p0);
    face.triangles.push_back(p1);
    face.triangles.push_back(p2);
    face.triangleNormals.push_back(bisectorNormalAt(frame, p0));
    face.triangleNormals.push_back(bisectorNormalAt(frame, p1));
    face.triangleNormals.push_back(bisectorNormalAt(frame, p2));
  };

  for (std::size_t i = 0; i < n; ++i)
  {
    std::size_t next = (i + 1) % n;
    emitTriangle(getPoint(0, 0), getPoint(1, i), getPoint(1, next));

    for (std::size_t r = 1; r < rings; ++r)
    {
      double3 p00 = getPoint(r, i);
      double3 p01 = getPoint(r, next);
      double3 p10 = getPoint(r + 1, i);
      double3 p11 = getPoint(r + 1, next);
      emitTriangle(p00, p10, p11);
      emitTriangle(p00, p11, p01);
    }
  }
}

struct ImageData
{
  std::size_t atomIndex;
  int3 shift;
  double3 position;
  double radius;
};

struct CandidateVertex
{
  double3 position;
  double clearance;
  std::set<GeneratorID, GeneratorIDLess> touchers;
};

struct TripletKey
{
  GeneratorID g0;
  GeneratorID g1;
  GeneratorID g2;

  bool operator<(const TripletKey &other) const
  {
    GeneratorIDLess cmp;
    if (cmp(g0, other.g0)) return true;
    if (cmp(other.g0, g0)) return false;
    if (cmp(g1, other.g1)) return true;
    if (cmp(other.g1, g1)) return false;
    return cmp(g2, other.g2);
  }
};

struct UnwrappedSegment
{
  std::size_t edgeIndex;
  double3 startPos;
  double3 endPos;
  int3 shiftStart;
  int3 shiftEnd;
};

std::vector<double3> stitchFaceBoundaryLoop(const std::vector<UnwrappedSegment> &segments,
                                            const std::vector<SKApolloniusDiagram::ApolloniusEdge> &edges,
                                            const double3x3 &latticeVectors)
{
  if (segments.empty()) return {};

  std::vector<bool> used(segments.size(), false);
  std::vector<double3> loop;

  used[0] = true;
  const auto &firstSeg = segments[0];
  const auto &firstEdge = edges[firstSeg.edgeIndex];
  double3 firstShiftVec = latticeVectors * double3(firstSeg.shiftStart.x, firstSeg.shiftStart.y, firstSeg.shiftStart.z);
  for (std::size_t k = 0; k < firstEdge.polylinePoints.size(); ++k)
  {
    loop.push_back(firstEdge.polylinePoints[k] + firstShiftVec);
  }
  double3 currentEnd = firstSeg.endPos;

  for (std::size_t step = 1; step < segments.size(); ++step)
  {
    std::size_t bestIdx = std::numeric_limits<std::size_t>::max();
    bool reverseBest = false;
    double bestDistSq = 1e-2;

    for (std::size_t i = 0; i < segments.size(); ++i)
    {
      if (used[i]) continue;
      double dStart = (segments[i].startPos - currentEnd).length_squared();
      double dEnd = (segments[i].endPos - currentEnd).length_squared();
      if (dStart < bestDistSq)
      {
        bestDistSq = dStart;
        bestIdx = i;
        reverseBest = false;
      }
      if (dEnd < bestDistSq)
      {
        bestDistSq = dEnd;
        bestIdx = i;
        reverseBest = true;
      }
    }

    if (bestIdx == std::numeric_limits<std::size_t>::max())
    {
      break;
    }

    used[bestIdx] = true;
    const auto &seg = segments[bestIdx];
    const auto &edge = edges[seg.edgeIndex];
    double3 shiftVec = latticeVectors * double3(seg.shiftStart.x, seg.shiftStart.y, seg.shiftStart.z);

    if (!reverseBest)
    {
      for (std::size_t k = 1; k < edge.polylinePoints.size(); ++k)
      {
        loop.push_back(edge.polylinePoints[k] + shiftVec);
      }
      currentEnd = seg.endPos;
    }
    else
    {
      for (std::size_t k = edge.polylinePoints.size() - 1; k > 0; --k)
      {
        loop.push_back(edge.polylinePoints[k - 1] + shiftVec);
      }
      currentEnd = seg.startPos;
    }
  }

  if (loop.size() > 2 && (loop.front() - loop.back()).length_squared() < 1e-6)
  {
    loop.pop_back();
  }

  return loop;
}

double3 minImageVector(const double3 &cartDelta,
                       const double3x3 &latticeVectors,
                       const double3x3 &invLattice,
                       bool isPeriodic)
{
  if (!isPeriodic) return cartDelta;
  double3 df = invLattice * cartDelta;
  df.x -= std::rint(df.x);
  df.y -= std::rint(df.y);
  df.z -= std::rint(df.z);
  return latticeVectors * df;
}

struct ExactApolloniusSubface
{
  std::size_t neighborAtomIndex{0};
  int3 periodicShift{0, 0, 0};
  std::vector<double3> boundaryVertices;
  std::vector<GeneratorID> boundaryThirdGenerators;
  std::vector<std::vector<double3>> edgePolylines;
  std::vector<double3> triangles;
  std::vector<double3> triangleNormals;
  double area{0.0};
};

struct ExactApolloniusCellResult
{
  std::size_t atomIndex{0};
  std::vector<ExactApolloniusSubface> subfaces;
  std::vector<double3> uniqueVertices;
  double volume{0.0};
  double surfaceArea{0.0};
};

std::pair<double3, double> rayHyperboloidIntersection(const double3 &p_i, double r_i,
                                                      const double3 &p_j, double r_j,
                                                      const double3 &uDir,
                                                      double maxBound)
{
  double3 d = p_j - p_i;
  double dr = r_i - r_j;
  double rhs0 = 0.5 * (double3::dot(d, d) - dr * dr);
  if (rhs0 <= 1e-12)
  {
    return {p_i, 0.0};
  }

  double du = double3::dot(d, uDir);
  double A = du * du - dr * dr;
  double B = -2.0 * du * rhs0;
  double C = rhs0 * rhs0;

  double bestT = maxBound;
  if (std::abs(A) < 1e-12)
  {
    if (std::abs(B) > 1e-12)
    {
      double t = -C / B;
      if (t > 0.0 && du * t - rhs0 >= -1e-9 * rhs0)
      {
        bestT = std::min(bestT, t);
      }
    }
  }
  else
  {
    double disc = B * B - 4.0 * A * C;
    if (disc >= 0.0)
    {
      double sqrtDisc = std::sqrt(disc);
      double t1 = (-B - sqrtDisc) / (2.0 * A);
      double t2 = (-B + sqrtDisc) / (2.0 * A);
      for (double t : {t1, t2})
      {
        if (t > 1e-12 && du * t - rhs0 >= -1e-7 * std::max(1.0, rhs0))
        {
          if (t < bestT)
          {
            bestT = t;
          }
        }
      }
    }
  }

  return {p_i + bestT * uDir, bestT};
}

struct NeighborCand
{
  std::size_t atomIdx;
  int3 shift;
  double3 pos;
  double radius;
  double surfDist;
};

struct ActiveSheet
{
  std::size_t atomIdx;
  int3 shift;
  double3 pos;
  double radius;
};

struct TripletArcSegment
{
  std::size_t sA;
  std::size_t sB;
  double sMin;
  double sMax;
  int branchSign; // +1 or -1 for normal side
  double3 pStart;
  double3 pEnd;
  bool isClosedLoop;
  TripletConicParam conic;
  std::array<double3, 3> centers;
  std::array<double, 3> radii;
  double3 nUnit;
};

bool evalTripletBranch(const TripletConicParam &param,
                       const double3 &nUnit,
                       int branchSign,
                       double s,
                       double3 &outPos)
{
  if (!param.valid || s < 0.0) return false;
  double hSq = s * s - (param.a * s * s + 2.0 * param.b * s + param.c);
  if (hSq < -1e-8) return false;
  double h = std::sqrt(std::max(0.0, hSq));
  outPos = param.cRef + param.p0 + s * param.vDir + (static_cast<double>(branchSign) * h) * nUnit;
  return true;
}

bool computeValidSInterval(const TripletConicParam &param, double maxS, double &sLo, double &sHi)
{
  if (!param.valid) return false;
  double A = 1.0 - param.a;
  double B = -2.0 * param.b;
  double C = -param.c;

  sLo = 0.0;
  sHi = maxS;

  if (std::abs(A) < 1e-12)
  {
    if (std::abs(B) < 1e-12)
    {
      return (C >= -1e-9);
    }
    double root = -C / B;
    if (B > 0.0)
    {
      sLo = std::max(sLo, root);
    }
    else
    {
      sHi = std::min(sHi, root);
    }
  }
  else
  {
    double disc = B * B - 4.0 * A * C;
    if (disc < -1e-9)
    {
      return (A > 0.0);
    }
    double sqrtDisc = std::sqrt(std::max(0.0, disc));
    double r1 = (-B - sqrtDisc) / (2.0 * A);
    double r2 = (-B + sqrtDisc) / (2.0 * A);
    if (r1 > r2) std::swap(r1, r2);

    if (A < 0.0)
    {
      sLo = std::max(sLo, r1);
      sHi = std::min(sHi, r2);
    }
    else
    {
      if (r2 >= 0.0)
      {
        sLo = std::max(sLo, r2);
      }
    }
  }

  return (sHi >= sLo - 1e-9);
}

void buildExactCellFromRayCast(std::size_t atomIndex,
                               const double3 &p_i,
                               double r_i,
                               const std::vector<NeighborCand> &candidates,
                               double maxBound,
                               ExactApolloniusCellResult &result)
{
  (void)atomIndex;
  const std::size_t numFibonacciRays = 600;
  const double goldenRatio = (1.0 + std::sqrt(5.0)) / 2.0;

  std::set<GeneratorID, GeneratorIDLess> activeSet;
  for (std::size_t k = 0; k < numFibonacciRays; ++k)
  {
    double z = 1.0 - (2.0 * static_cast<double>(k) + 1.0) / static_cast<double>(numFibonacciRays);
    double r = std::sqrt(std::max(0.0, 1.0 - z * z));
    double phi = 2.0 * M_PI * static_cast<double>(k) / goldenRatio;
    double3 uDir(r * std::cos(phi), r * std::sin(phi), z);

    double bestT = maxBound;
    std::size_t bestCand = std::numeric_limits<std::size_t>::max();
    for (std::size_t c = 0; c < candidates.size(); ++c)
    {
      auto [pt, t] = rayHyperboloidIntersection(p_i, r_i, candidates[c].pos, candidates[c].radius, uDir, bestT);
      if (t < bestT - 1e-9)
      {
        bestT = t;
        bestCand = c;
      }
    }
    if (bestCand != std::numeric_limits<std::size_t>::max())
    {
      activeSet.insert({candidates[bestCand].atomIdx, candidates[bestCand].shift});
    }
  }

  for (std::size_t c = 0; c < std::min(candidates.size(), std::size_t{28}); ++c)
  {
    activeSet.insert({candidates[c].atomIdx, candidates[c].shift});
  }

  std::vector<ActiveSheet> sheets;
  for (const auto &cand : candidates)
  {
    if (activeSet.find({cand.atomIdx, cand.shift}) != activeSet.end())
    {
      sheets.push_back(ActiveSheet{cand.atomIdx, cand.shift, cand.pos, cand.radius});
    }
  }

  const std::size_t nTheta = 28;
  const std::size_t nPhi = 56;
  const std::size_t totalVerts = (nTheta + 1) * nPhi;

  std::vector<double3> spherePos(totalVerts);
  std::vector<std::size_t> winnerSheet(totalVerts, 0);

  auto evalBisectorRay = [&](const ActiveSheet &sh, const double3 &uDir) -> double
  {
    auto [pt, t] = rayHyperboloidIntersection(p_i, r_i, sh.pos, sh.radius, uDir, maxBound);
    return t;
  };

  for (std::size_t it = 0; it <= nTheta; ++it)
  {
    double theta = M_PI * static_cast<double>(it) / static_cast<double>(nTheta);
    double sinT = std::sin(theta);
    double cosT = std::cos(theta);
    for (std::size_t ip = 0; ip < nPhi; ++ip)
    {
      double phi = 2.0 * M_PI * static_cast<double>(ip) / static_cast<double>(nPhi);
      double3 uDir(sinT * std::cos(phi), sinT * std::sin(phi), cosT);

      double bestT = maxBound;
      std::size_t bestS = 0;
      for (std::size_t s = 0; s < sheets.size(); ++s)
      {
        double t = evalBisectorRay(sheets[s], uDir);
        if (t < bestT)
        {
          bestT = t;
          bestS = s;
        }
      }
      std::size_t idx = it * nPhi + ip;
      spherePos[idx] = p_i + bestT * uDir;
      winnerSheet[idx] = bestS;
    }
  }

  auto refineEdgePoint = [&](const double3 &u0, const double3 &u1, std::size_t s0, std::size_t s1) -> double3
  {
    double lo = 0.0, hi = 1.0;
    for (int iter = 0; iter < 18; ++iter)
    {
      double mid = 0.5 * (lo + hi);
      double3 uMid = double3::normalize((1.0 - mid) * u0 + mid * u1);
      double t0 = evalBisectorRay(sheets[s0], uMid);
      double t1 = evalBisectorRay(sheets[s1], uMid);
      if (t0 <= t1) lo = mid;
      else hi = mid;
    }
    double mid = 0.5 * (lo + hi);
    double3 uMid = double3::normalize((1.0 - mid) * u0 + mid * u1);
    double t = std::min(evalBisectorRay(sheets[s0], uMid), evalBisectorRay(sheets[s1], uMid));
    double3 pos = p_i + t * uMid;
    double cl = t - r_i;
    refineOnTripletBisector(pos, cl,
                            {p_i, sheets[s0].pos, sheets[s1].pos},
                            {r_i, sheets[s0].radius, sheets[s1].radius});
    return pos;
  };

  auto refineCornerPoint = [&](const double3 &u0, const double3 &u1, const double3 &u2,
                               std::size_t s0, std::size_t s1, std::size_t s2) -> double3
  {
    std::array<double3, 4> c4 = {p_i, sheets[s0].pos, sheets[s1].pos, sheets[s2].pos};
    std::array<double, 4> r4 = {r_i, sheets[s0].radius, sheets[s1].radius, sheets[s2].radius};
    double3 approxU = double3::normalize(u0 + u1 + u2);
    double approxT = std::min({evalBisectorRay(sheets[s0], approxU),
                               evalBisectorRay(sheets[s1], approxU),
                               evalBisectorRay(sheets[s2], approxU)});
    double3 approxPos = p_i + approxT * approxU;

    auto sols = SKApolloniusDiagram::solveApolloniusQuadruplet(c4, r4);
    double bestDistSq = std::numeric_limits<double>::max();
    double3 bestPos = approxPos;
    for (const auto &[solPos, solR] : sols)
    {
      double dSq = (solPos - approxPos).length_squared();
      if (dSq < bestDistSq)
      {
        bestDistSq = dSq;
        bestPos = solPos;
      }
    }
    return bestPos;
  };

  std::map<std::size_t, ExactApolloniusSubface> subfaceMap;

  auto addSubfaceTri = [&](std::size_t sIdx, const double3 &a, const double3 &b, const double3 &c)
  {
    if ((b - a).length_squared() < 1e-14 || (c - b).length_squared() < 1e-14 || (a - c).length_squared() < 1e-14)
    {
      return;
    }
    auto &sf = subfaceMap[sIdx];
    sf.neighborAtomIndex = sheets[sIdx].atomIdx;
    sf.periodicShift = sheets[sIdx].shift;

    CurvedSurfaceFrame frame = makeCurvedSurfaceFrame(p_i, r_i, sheets[sIdx].pos, sheets[sIdx].radius);
    sf.triangles.push_back(a);
    sf.triangles.push_back(b);
    sf.triangles.push_back(c);
    sf.triangleNormals.push_back(bisectorNormalAt(frame, a));
    sf.triangleNormals.push_back(bisectorNormalAt(frame, b));
    sf.triangleNormals.push_back(bisectorNormalAt(frame, c));
  };

  auto processSphericalTri = [&](std::size_t i0, std::size_t i1, std::size_t i2)
  {
    std::size_t s0 = winnerSheet[i0];
    std::size_t s1 = winnerSheet[i1];
    std::size_t s2 = winnerSheet[i2];
    const double3 &p0 = spherePos[i0];
    const double3 &p1 = spherePos[i1];
    const double3 &p2 = spherePos[i2];

    if (s0 == s1 && s1 == s2)
    {
      addSubfaceTri(s0, p0, p1, p2);
      return;
    }

    double3 u0 = double3::normalize(p0 - p_i);
    double3 u1 = double3::normalize(p1 - p_i);
    double3 u2 = double3::normalize(p2 - p_i);

    if (s0 != s1 && s1 != s2 && s0 != s2)
    {
      double3 e01 = refineEdgePoint(u0, u1, s0, s1);
      double3 e12 = refineEdgePoint(u1, u2, s1, s2);
      double3 e20 = refineEdgePoint(u2, u0, s2, s0);
      double3 center = refineCornerPoint(u0, u1, u2, s0, s1, s2);

      addSubfaceTri(s0, p0, e01, center);
      addSubfaceTri(s0, p0, center, e20);
      addSubfaceTri(s1, p1, e12, center);
      addSubfaceTri(s1, p1, center, e01);
      addSubfaceTri(s2, p2, e20, center);
      addSubfaceTri(s2, p2, center, e12);

      subfaceMap[s0].edgePolylines.push_back({e01, center, e20});
      subfaceMap[s1].edgePolylines.push_back({e12, center, e01});
      subfaceMap[s2].edgePolylines.push_back({e20, center, e12});
      result.uniqueVertices.push_back(center);
    }
    else
    {
      std::size_t idxSolo = i0, idxA = i1, idxB = i2;
      if (s0 == s1) { idxSolo = i2; idxA = i0; idxB = i1; }
      else if (s0 == s2) { idxSolo = i1; idxA = i2; idxB = i0; }

      std::size_t sSolo = winnerSheet[idxSolo];
      std::size_t sPair = winnerSheet[idxA];
      double3 pSolo = spherePos[idxSolo];
      double3 pA = spherePos[idxA];
      double3 pB = spherePos[idxB];
      double3 uSolo = double3::normalize(pSolo - p_i);
      double3 uA = double3::normalize(pA - p_i);
      double3 uB = double3::normalize(pB - p_i);

      double3 eA = refineEdgePoint(uSolo, uA, sSolo, sPair);
      double3 eB = refineEdgePoint(uSolo, uB, sSolo, sPair);

      addSubfaceTri(sSolo, pSolo, eA, eB);
      addSubfaceTri(sPair, eA, pA, pB);
      addSubfaceTri(sPair, eA, pB, eB);

      subfaceMap[sSolo].edgePolylines.push_back({eA, eB});
      subfaceMap[sPair].edgePolylines.push_back({eB, eA});
    }
  };

  for (std::size_t it = 0; it < nTheta; ++it)
  {
    for (std::size_t ip = 0; ip < nPhi; ++ip)
    {
      std::size_t nextIp = (ip + 1) % nPhi;
      std::size_t v00 = it * nPhi + ip;
      std::size_t v01 = it * nPhi + nextIp;
      std::size_t v10 = (it + 1) * nPhi + ip;
      std::size_t v11 = (it + 1) * nPhi + nextIp;

      if (it > 0) processSphericalTri(v00, v10, v11);
      if (it + 1 < nTheta) processSphericalTri(v00, v11, v01);
    }
  }

  double totalVolume = 0.0;
  double totalArea = 0.0;
  for (auto &[sIdx, sf] : subfaceMap)
  {
    double sfArea = 0.0;
    for (std::size_t t = 0; t + 2 < sf.triangles.size(); t += 3)
    {
      const double3 &v0 = sf.triangles[t];
      const double3 &v1 = sf.triangles[t + 1];
      const double3 &v2 = sf.triangles[t + 2];
      double3 crossVec = double3::cross(v1 - v0, v2 - v0);
      sfArea += 0.5 * crossVec.length();
      totalVolume += std::abs(double3::dot(v0 - p_i, crossVec)) / 6.0;
    }
    sf.area = sfArea;
    totalArea += sfArea;
    result.subfaces.push_back(std::move(sf));
  }
  result.volume = totalVolume;
  result.surfaceArea = totalArea;
}
ExactApolloniusCellResult computeExactApolloniusCell(std::size_t atomIndex,
                                                     const std::vector<std::pair<double3, double>> &atoms,
                                                     const double3x3 &latticeVectors,
                                                     bool isPeriodic,
                                                     std::size_t polylineResolution)
{
  ExactApolloniusCellResult result;
  result.atomIndex = atomIndex;
  if (atomIndex >= atoms.size()) return result;

  const double3 &p_i = atoms[atomIndex].first;
  double r_i = atoms[atomIndex].second;

  double maxBound = 15.0;
  if (isPeriodic)
  {
    maxBound = std::max({latticeVectors[0].length(), latticeVectors[1].length(), latticeVectors[2].length()}) * 2.5;
  }
  else
  {
    for (const auto &a : atoms)
    {
      maxBound = std::max(maxBound, (a.first - p_i).length() + 15.0);
    }
  }

  std::vector<NeighborCand> candidates;
  int maxShell = isPeriodic ? 2 : 0;
  for (int nx = -maxShell; nx <= maxShell; ++nx)
  {
    for (int ny = -maxShell; ny <= maxShell; ++ny)
    {
      for (int nz = -maxShell; nz <= maxShell; ++nz)
      {
        int3 shift(nx, ny, nz);
        double3 shiftVec = latticeVectors * double3(nx, ny, nz);
        for (std::size_t j = 0; j < atoms.size(); ++j)
        {
          if (j == atomIndex && nx == 0 && ny == 0 && nz == 0) continue;
          double3 p_j = atoms[j].first + shiftVec;
          double r_j = atoms[j].second;
          double dist = (p_j - p_i).length();
          if (dist < 1e-8) continue;
          double surfDist = 0.5 * (dist - r_i - r_j);
          candidates.push_back(NeighborCand{j, shift, p_j, r_j, surfDist});
        }
      }
    }
  }

  if (!isPeriodic)
  {
    double boxDist = maxBound;
    const double3 dirs[6] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };
    const int3 shifts[6] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };
    for (int b = 0; b < 6; ++b)
    {
      double3 p_b = p_i + (2.0 * boxDist) * dirs[b];
      candidates.push_back(NeighborCand{atomIndex, shifts[b], p_b, r_i, boxDist - r_i});
    }
  }

  std::sort(candidates.begin(), candidates.end(),
            [](const NeighborCand &a, const NeighborCand &b) { return a.surfDist < b.surfDist; });

  std::set<GeneratorID, GeneratorIDLess> activeSet;
  const std::size_t numFibonacciRays = 500;
  const double goldenRatio = (1.0 + std::sqrt(5.0)) / 2.0;
  for (std::size_t k = 0; k < numFibonacciRays; ++k)
  {
    double z = 1.0 - (2.0 * static_cast<double>(k) + 1.0) / static_cast<double>(numFibonacciRays);
    double r = std::sqrt(std::max(0.0, 1.0 - z * z));
    double phi = 2.0 * M_PI * static_cast<double>(k) / goldenRatio;
    double3 uDir(r * std::cos(phi), r * std::sin(phi), z);

    double bestT = maxBound * 2.0;
    std::size_t bestCand = std::numeric_limits<std::size_t>::max();
    for (std::size_t c = 0; c < candidates.size(); ++c)
    {
      auto [pt, t] = rayHyperboloidIntersection(p_i, r_i, candidates[c].pos, candidates[c].radius, uDir, bestT);
      if (t < bestT - 1e-9)
      {
        bestT = t;
        bestCand = c;
      }
    }
    if (bestCand != std::numeric_limits<std::size_t>::max())
    {
      activeSet.insert({candidates[bestCand].atomIdx, candidates[bestCand].shift});
    }
  }
  for (std::size_t c = 0; c < std::min(candidates.size(), std::size_t{36}); ++c)
  {
    activeSet.insert({candidates[c].atomIdx, candidates[c].shift});
  }

  std::vector<ActiveSheet> sheets;
  for (const auto &cand : candidates)
  {
    if (activeSet.find({cand.atomIdx, cand.shift}) != activeSet.end())
    {
      sheets.push_back(ActiveSheet{cand.atomIdx, cand.shift, cand.pos, cand.radius});
    }
  }

  auto isValidOnCell = [&](const double3 &pos, std::size_t ignoreA, std::size_t ignoreB, std::size_t ignoreC, double tol) -> bool
  {
    double cl_i = (pos - p_i).length() - r_i;
    for (std::size_t m = 0; m < sheets.size(); ++m)
    {
      if (m == ignoreA || m == ignoreB || m == ignoreC) continue;
      double cl_m = (pos - sheets[m].pos).length() - sheets[m].radius;
      if (cl_m < cl_i - tol)
      {
        return false;
      }
    }
    return true;
  };

  std::vector<TripletArcSegment> validSegments;
  double maxS = maxBound * 2.0 + r_i;

  for (std::size_t a = 0; a < sheets.size(); ++a)
  {
    for (std::size_t b = a + 1; b < sheets.size(); ++b)
    {
      std::array<double3, 3> c3 = {p_i, sheets[a].pos, sheets[b].pos};
      std::array<double, 3> r3 = {r_i, sheets[a].radius, sheets[b].radius};
      TripletConicParam conic = buildTripletConicParam(c3, r3);
      if (!conic.valid) continue;

      double sLo = 0.0, sHi = maxS;
      if (!computeValidSInterval(conic, maxS, sLo, sHi)) continue;

      double3 d1 = c3[static_cast<std::size_t>((conic.refIdx + 1) % 3)] - conic.cRef;
      double3 d2 = c3[static_cast<std::size_t>((conic.refIdx + 2) % 3)] - conic.cRef;
      double3 crossD = double3::cross(d1, d2);
      double crossLen = crossD.length();
      if (crossLen < 1e-12) continue;
      double3 nUnit = crossD / crossLen;

      for (int branchSign : {+1, -1})
      {
        std::vector<double> cutS = {sLo, sHi};

        for (std::size_t k = 0; k < sheets.size(); ++k)
        {
          if (k == a || k == b) continue;
          std::array<double3, 4> c4 = {p_i, sheets[a].pos, sheets[b].pos, sheets[k].pos};
          std::array<double, 4> r4 = {r_i, sheets[a].radius, sheets[b].radius, sheets[k].radius};
          auto sols = SKApolloniusDiagram::solveApolloniusQuadruplet(c4, r4);
          for (const auto &[solPos, solR] : sols)
          {
            double sVal = solR + conic.rRef;
            if (sVal < sLo - 1e-6 || sVal > sHi + 1e-6) continue;
            sVal = std::clamp(sVal, sLo, sHi);

            double3 rel = solPos - (conic.cRef + conic.p0 + sVal * conic.vDir);
            double signedH = double3::dot(rel, nUnit);
            if ((branchSign > 0 && signedH >= -1e-5) || (branchSign < 0 && signedH <= 1e-5))
            {
              cutS.push_back(sVal);
            }
          }
        }

        std::sort(cutS.begin(), cutS.end());
        std::vector<double> uniqueS;
        for (double sVal : cutS)
        {
          if (uniqueS.empty() || std::abs(sVal - uniqueS.back()) > 1e-6)
          {
            uniqueS.push_back(sVal);
          }
        }

        for (std::size_t idx = 0; idx + 1 < uniqueS.size(); ++idx)
        {
          double s0 = uniqueS[idx];
          double s1 = uniqueS[idx + 1];
          if (s1 - s0 < 1e-6) continue;
          double sMid = 0.5 * (s0 + s1);
          double3 pMid;
          if (!evalTripletBranch(conic, nUnit, branchSign, sMid, pMid)) continue;

          double cl = (pMid - p_i).length() - r_i;
          refineOnTripletBisector(pMid, cl, c3, r3);

          if (isValidOnCell(pMid, a, b, std::numeric_limits<std::size_t>::max(), 1e-4))
          {
            double3 pStart, pEnd;
            evalTripletBranch(conic, nUnit, branchSign, s0, pStart);
            evalTripletBranch(conic, nUnit, branchSign, s1, pEnd);
            double cl0 = 0.0, cl1 = 0.0;
            refineOnTripletBisector(pStart, cl0, c3, r3);
            refineOnTripletBisector(pEnd, cl1, c3, r3);

            TripletArcSegment seg;
            seg.sA = a;
            seg.sB = b;
            seg.sMin = s0;
            seg.sMax = s1;
            seg.branchSign = branchSign;
            seg.pStart = pStart;
            seg.pEnd = pEnd;
            seg.isClosedLoop = false;
            seg.conic = conic;
            seg.centers = c3;
            seg.radii = r3;
            seg.nUnit = nUnit;
            validSegments.push_back(std::move(seg));
          }
        }
      }
    }
  }

  std::map<std::size_t, std::vector<std::size_t>> sheetToSegs;
  for (std::size_t segIdx = 0; segIdx < validSegments.size(); ++segIdx)
  {
    sheetToSegs[validSegments[segIdx].sA].push_back(segIdx);
    sheetToSegs[validSegments[segIdx].sB].push_back(segIdx);

    auto addUniqueVert = [&](const double3 &v)
    {
      for (const auto &ev : result.uniqueVertices)
      {
        if ((ev - v).length_squared() < 1e-6) return;
      }
      result.uniqueVertices.push_back(v);
    };
    addUniqueVert(validSegments[segIdx].pStart);
    addUniqueVert(validSegments[segIdx].pEnd);
  }

  double totalVolume = 0.0;
  double totalArea = 0.0;
  std::size_t numArcSamples = std::max<std::size_t>(4, polylineResolution / 2);

  for (const auto &[sIdx, segIndices] : sheetToSegs)
  {
    if (segIndices.empty()) continue;

    ExactApolloniusSubface sf;
    sf.neighborAtomIndex = sheets[sIdx].atomIdx;
    sf.periodicShift = sheets[sIdx].shift;

    std::vector<std::vector<double3>> rawPolylines;
    for (std::size_t segIdx : segIndices)
    {
      const auto &seg = validSegments[segIdx];
      std::vector<double3> arcPts;
      arcPts.reserve(numArcSamples + 1);
      for (std::size_t step = 0; step <= numArcSamples; ++step)
      {
        double frac = static_cast<double>(step) / static_cast<double>(numArcSamples);
        double sVal = seg.sMin + frac * (seg.sMax - seg.sMin);
        double3 pt;
        if (evalTripletBranch(seg.conic, seg.nUnit, seg.branchSign, sVal, pt))
        {
          double cl = 0.0;
          refineOnTripletBisector(pt, cl, seg.centers, seg.radii);
          arcPts.push_back(pt);
        }
      }
      if (arcPts.size() >= 2)
      {
        rawPolylines.push_back(arcPts);
        sf.edgePolylines.push_back(std::move(arcPts));
      }
    }

    if (rawPolylines.empty()) continue;

    struct MergedArc
    {
      std::vector<double3> pts;
      std::size_t otherSheet;
    };
    std::vector<MergedArc> mergedArcs;
    std::vector<bool> rawUsed(rawPolylines.size(), false);

    for (std::size_t i = 0; i < rawPolylines.size(); ++i)
    {
      if (rawUsed[i]) continue;
      rawUsed[i] = true;
      const auto &segI = validSegments[segIndices[i]];
      std::size_t otherI = (segI.sA == sIdx) ? segI.sB : segI.sA;
      std::vector<double3> chain = rawPolylines[i];

      bool extended = true;
      while (extended)
      {
        extended = false;
        for (std::size_t j = 0; j < rawPolylines.size(); ++j)
        {
          if (rawUsed[j]) continue;
          const auto &segJ = validSegments[segIndices[j]];
          std::size_t otherJ = (segJ.sA == sIdx) ? segJ.sB : segJ.sA;
          if (otherJ != otherI) continue;

          const auto &polyJ = rawPolylines[j];
          double tolSq = 1e-4;
          if ((chain.back() - polyJ.front()).length_squared() < tolSq)
          {
            for (std::size_t k = 1; k < polyJ.size(); ++k) chain.push_back(polyJ[k]);
            rawUsed[j] = true;
            extended = true;
          }
          else if ((chain.back() - polyJ.back()).length_squared() < tolSq)
          {
            for (std::size_t k = polyJ.size() - 1; k > 0; --k) chain.push_back(polyJ[k - 1]);
            rawUsed[j] = true;
            extended = true;
          }
          else if ((chain.front() - polyJ.back()).length_squared() < tolSq)
          {
            std::vector<double3> newChain = polyJ;
            for (std::size_t k = 1; k < chain.size(); ++k) newChain.push_back(chain[k]);
            chain = std::move(newChain);
            rawUsed[j] = true;
            extended = true;
          }
          else if ((chain.front() - polyJ.front()).length_squared() < tolSq)
          {
            std::vector<double3> newChain(polyJ.rbegin(), polyJ.rend());
            for (std::size_t k = 1; k < chain.size(); ++k) newChain.push_back(chain[k]);
            chain = std::move(newChain);
            rawUsed[j] = true;
            extended = true;
          }
        }
      }
      mergedArcs.push_back(MergedArc{std::move(chain), otherI});
    }

    std::vector<bool> used(mergedArcs.size(), false);
    std::vector<double3> loopPoints;
    used[0] = true;
    loopPoints = mergedArcs[0].pts;
    sf.boundaryVertices.push_back(mergedArcs[0].pts.front());
    sf.boundaryThirdGenerators.push_back({sheets[mergedArcs[0].otherSheet].atomIdx,
                                          sheets[mergedArcs[0].otherSheet].shift});

    for (std::size_t step = 1; step < mergedArcs.size(); ++step)
    {
      double3 currEnd = loopPoints.back();
      std::size_t bestIdx = std::numeric_limits<std::size_t>::max();
      bool rev = false;
      double bestDistSq = 1e-1;

      for (std::size_t i = 0; i < mergedArcs.size(); ++i)
      {
        if (used[i]) continue;
        double dFront = (mergedArcs[i].pts.front() - currEnd).length_squared();
        double dBack = (mergedArcs[i].pts.back() - currEnd).length_squared();
        if (dFront < bestDistSq)
        {
          bestDistSq = dFront;
          bestIdx = i;
          rev = false;
        }
        if (dBack < bestDistSq)
        {
          bestDistSq = dBack;
          bestIdx = i;
          rev = true;
        }
      }

      if (bestIdx == std::numeric_limits<std::size_t>::max()) break;
      used[bestIdx] = true;
      const auto &arc = mergedArcs[bestIdx];
      sf.boundaryVertices.push_back(currEnd);
      sf.boundaryThirdGenerators.push_back({sheets[arc.otherSheet].atomIdx,
                                            sheets[arc.otherSheet].shift});
      if (!rev)
      {
        for (std::size_t k = 1; k < arc.pts.size(); ++k)
        {
          loopPoints.push_back(arc.pts[k]);
        }
      }
      else
      {
        for (std::size_t k = arc.pts.size() - 1; k > 0; --k)
        {
          loopPoints.push_back(arc.pts[k - 1]);
        }
      }
    }

    if (loopPoints.size() > 2 && (loopPoints.front() - loopPoints.back()).length_squared() < 1e-4)
    {
      loopPoints.pop_back();
    }

    if (loopPoints.size() >= 3)
    {
      SKApolloniusDiagram::ApolloniusFace tempFace;
      triangulateCurvedFace(tempFace, loopPoints,
                            p_i, r_i,
                            sheets[sIdx].pos, sheets[sIdx].radius,
                            polylineResolution);
      sf.triangles = std::move(tempFace.triangles);
      sf.triangleNormals = std::move(tempFace.triangleNormals);

      double sfArea = 0.0;
      for (std::size_t t = 0; t + 2 < sf.triangles.size(); t += 3)
      {
        const double3 &v0 = sf.triangles[t];
        const double3 &v1 = sf.triangles[t + 1];
        const double3 &v2 = sf.triangles[t + 2];
        double3 crossVec = double3::cross(v1 - v0, v2 - v0);
        sfArea += 0.5 * crossVec.length();
        totalVolume += std::abs(double3::dot(v0 - p_i, crossVec)) / 6.0;
      }
      sf.area = sfArea;
      totalArea += sfArea;
      result.subfaces.push_back(std::move(sf));
    }
  }

  result.volume = totalVolume;
  result.surfaceArea = totalArea;

  bool needFallback = result.subfaces.size() < 4 || result.volume < 1e-6;
  if (!needFallback)
  {
    std::size_t closedTestCount = 32;
    const double goldenRatio2 = (1.0 + std::sqrt(5.0)) / 2.0;
    std::size_t hits = 0;
    for (std::size_t k = 0; k < closedTestCount; ++k)
    {
      double z = 1.0 - (2.0 * static_cast<double>(k) + 1.0) / static_cast<double>(closedTestCount);
      double r = std::sqrt(std::max(0.0, 1.0 - z * z));
      double phi = 2.0 * M_PI * static_cast<double>(k) / goldenRatio2;
      double3 rayDir(r * std::cos(phi), r * std::sin(phi), z);
      bool hitAny = false;
      for (const auto &sf : result.subfaces)
      {
        for (std::size_t t = 0; t + 2 < sf.triangles.size(); t += 3)
        {
          double3 v0 = sf.triangles[t] - p_i;
          double3 v1 = sf.triangles[t + 1] - p_i;
          double3 v2 = sf.triangles[t + 2] - p_i;
          double3 e1 = v1 - v0;
          double3 e2 = v2 - v0;
          double3 hVec = double3::cross(rayDir, e2);
          double det = double3::dot(e1, hVec);
          if (std::abs(det) < 1e-12) continue;
          double invDet = 1.0 / det;
          double3 sVec = -v0;
          double u = invDet * double3::dot(sVec, hVec);
          if (u < -1e-4 || u > 1.0 + 1e-4) continue;
          double3 qVec = double3::cross(sVec, e1);
          double v = invDet * double3::dot(rayDir, qVec);
          if (v < -1e-4 || u + v > 1.0 + 2e-4) continue;
          double tRay = invDet * double3::dot(e2, qVec);
          if (tRay > 1e-5)
          {
            hitAny = true;
            break;
          }
        }
        if (hitAny) break;
      }
      if (hitAny) ++hits;
    }
    if (hits < closedTestCount * 3 / 4)
    {
      needFallback = true;
    }
  }

  if (needFallback)
  {
    result.subfaces.clear();
    result.uniqueVertices.clear();
    result.volume = 0.0;
    result.surfaceArea = 0.0;
    buildExactCellFromRayCast(atomIndex, p_i, r_i, candidates, maxBound, result);
  }

  return result;
}
} // namespace

SKApolloniusDiagram::ApolloniusNetwork SKApolloniusDiagram::computeNetwork(
    const std::vector<std::pair<double3, double>> &atoms,
    const double3x3 &latticeVectors,
    bool isPeriodic,
    std::size_t polylineResolution)
{
  ApolloniusNetwork net;
  if (atoms.empty()) return net;

  double minR = atoms[0].second;
  double maxR = atoms[0].second;
  for (const auto &a : atoms)
  {
    minR = std::min(minR, a.second);
    maxR = std::max(maxR, a.second);
  }
  bool equalRadii = (maxR - minR) < 1e-7;

  double3x3 invLattice = isPeriodic ? latticeVectors.inverse() : double3x3::identity();

  int maxShell = isPeriodic ? 2 : 0;
  std::vector<ImageData> images;
  for (int nx = -maxShell; nx <= maxShell; ++nx)
  {
    for (int ny = -maxShell; ny <= maxShell; ++ny)
    {
      for (int nz = -maxShell; nz <= maxShell; ++nz)
      {
        int3 shift(nx, ny, nz);
        double3 shiftVec = latticeVectors * double3(nx, ny, nz);
        for (std::size_t i = 0; i < atoms.size(); ++i)
        {
          images.push_back(ImageData{i, shift, atoms[i].first + shiftVec, atoms[i].second});
        }
      }
    }
  }

  double3 cellCenter{0.0, 0.0, 0.0};
  double maxRadiusFromCenter = 0.0;
  if (!isPeriodic)
  {
    for (const auto &a : atoms) cellCenter += a.first;
    cellCenter = cellCenter / static_cast<double>(atoms.size());
    for (const auto &a : atoms)
    {
      maxRadiusFromCenter = std::max(maxRadiusFromCenter, (a.first - cellCenter).length() + a.second);
    }
  }

  auto lookupGeneratorPositionAndRadius = [&](const GeneratorID &gen) -> std::pair<double3, double>
  {
    double3 shiftVec = latticeVectors * double3(gen.second.x, gen.second.y, gen.second.z);
    return {atoms[gen.first].first + shiftVec, atoms[gen.first].second};
  };

  auto checkEmptySphere = [&](const double3 &pos, double clearance) -> std::pair<bool, std::set<GeneratorID, GeneratorIDLess>>
  {
    std::set<GeneratorID, GeneratorIDLess> touchers;
    constexpr double VIOLATION_TOL = 1e-4;
    constexpr double TOUCH_TOL = 1e-3;

    for (const auto &img : images)
    {
      double dist = (pos - img.position).length() - img.radius;
      if (dist < clearance - VIOLATION_TOL)
      {
        return {false, {}};
      }
      if (std::abs(dist - clearance) <= TOUCH_TOL)
      {
        touchers.insert({img.atomIndex, img.shift});
      }
    }
    return {true, touchers};
  };

  std::vector<CandidateVertex> candidateVertices;

  auto addCandidateVertex = [&](const double3 &rawPos, double rawClearance)
  {
    if (!isPeriodic)
    {
      if ((rawPos - cellCenter).length() > maxRadiusFromCenter * 3.0 + 15.0)
      {
        return;
      }
    }

    double3 wrappedCart = rawPos;
    double3 frac = invLattice * rawPos;
    int3 shift(0, 0, 0);
    if (isPeriodic)
    {
      shift = int3(static_cast<int>(std::floor(frac.x + 1e-6)),
                   static_cast<int>(std::floor(frac.y + 1e-6)),
                   static_cast<int>(std::floor(frac.z + 1e-6)));
      frac = frac - double3(shift.x, shift.y, shift.z);
      wrappedCart = latticeVectors * frac;
    }

    auto [isValid, touchers] = checkEmptySphere(wrappedCart, rawClearance);
    if (!isValid || touchers.size() < 4)
    {
      return;
    }

    constexpr double MERGE_TOL = 1e-3;
    for (auto &existing : candidateVertices)
    {
      double3 delta = minImageVector(existing.position - wrappedCart, latticeVectors, invLattice, isPeriodic);
      if (delta.length_squared() < MERGE_TOL * MERGE_TOL)
      {
        for (const auto &t : touchers)
        {
          existing.touchers.insert(t);
        }
        return;
      }
    }

    candidateVertices.push_back(CandidateVertex{wrappedCart, rawClearance, std::move(touchers)});
  };

  SKVoronoi::VoronoiNetwork radNet = SKVoronoi::computeNetwork(atoms, latticeVectors, isPeriodic,
                                                               SKVoronoi::VoronoiType::Radical, 10.0);

  if (equalRadii && isPeriodic)
  {
    for (const auto &vn : radNet.nodes)
    {
      auto [isValid, touchers] = checkEmptySphere(vn.cartesianPosition, vn.maxFreeSphereRadius);
      if (touchers.size() >= 4)
      {
        candidateVertices.push_back(CandidateVertex{vn.cartesianPosition, vn.maxFreeSphereRadius, std::move(touchers)});
      }
    }
  }
  else
  {
    std::set<std::array<GeneratorID, 4>> testedQuadruplets;

    auto testQuadruplet = [&](std::array<GeneratorID, 4> quad)
    {
      std::sort(quad.begin(), quad.end(), GeneratorIDLess{});
      for (int k = 0; k < 3; ++k)
      {
        if (quad[static_cast<std::size_t>(k)] == quad[static_cast<std::size_t>(k + 1)]) return;
      }

      int3 baseShift = quad[0].second;
      for (auto &g : quad)
      {
        g.second = g.second - baseShift;
      }
      std::sort(quad.begin(), quad.end(), GeneratorIDLess{});

      if (testedQuadruplets.find(quad) != testedQuadruplets.end()) return;
      testedQuadruplets.insert(quad);

      std::array<double3, 4> c4;
      std::array<double, 4> r4;
      for (std::size_t k = 0; k < 4; ++k)
      {
        auto [p, r] = lookupGeneratorPositionAndRadius(quad[k]);
        c4[k] = p;
        r4[k] = r;
      }

      auto sols = solveApolloniusQuadruplet(c4, r4);
      for (const auto &[pos, r] : sols)
      {
        addCandidateVertex(pos, r);
      }
    };

    for (std::size_t i = 0; i < radNet.cells.size(); ++i)
    {
      const auto &cell = radNet.cells[i];
      GeneratorID g0{i, int3(0, 0, 0)};

      for (std::size_t vIdx = 0; vIdx < cell.vertices.size(); ++vIdx)
      {
        std::vector<GeneratorID> incidentFaces;
        for (const auto &face : cell.faces)
        {
          if (std::find(face.vertexIndices.begin(), face.vertexIndices.end(), vIdx) != face.vertexIndices.end())
          {
            incidentFaces.push_back({face.neighborAtomIndex, face.periodicShift});
          }
        }
        if (incidentFaces.size() >= 3)
        {
          for (std::size_t a = 0; a < incidentFaces.size(); ++a)
          {
            for (std::size_t b = a + 1; b < incidentFaces.size(); ++b)
            {
              for (std::size_t c = b + 1; c < incidentFaces.size(); ++c)
              {
                testQuadruplet({g0, incidentFaces[a], incidentFaces[b], incidentFaces[c]});
              }
            }
          }
        }
      }
    }

    const std::size_t maxNeighbors = std::min<std::size_t>(images.size(), 14);
    for (std::size_t i = 0; i < atoms.size(); ++i)
    {
      GeneratorID g0{i, int3(0, 0, 0)};
      double3 p0 = atoms[i].first;
      double r0 = atoms[i].second;

      std::vector<std::pair<double, GeneratorID>> nearest;
      nearest.reserve(images.size());
      for (const auto &img : images)
      {
        if (img.atomIndex == i && img.shift == int3(0, 0, 0)) continue;
        double surfDist = (img.position - p0).length() - r0 - img.radius;
        nearest.push_back({surfDist, {img.atomIndex, img.shift}});
      }
      std::partial_sort(nearest.begin(),
                        nearest.begin() + static_cast<std::ptrdiff_t>(std::min(maxNeighbors, nearest.size())),
                        nearest.end(),
                        [](const auto &a, const auto &b) { return a.first < b.first; });

      std::size_t limit = std::min(maxNeighbors, nearest.size());
      for (std::size_t a = 0; a < limit; ++a)
      {
        for (std::size_t b = a + 1; b < limit; ++b)
        {
          for (std::size_t c = b + 1; c < limit; ++c)
          {
            testQuadruplet({g0, nearest[a].second, nearest[b].second, nearest[c].second});
          }
        }
      }
    }
  }

  std::vector<std::set<GeneratorID, GeneratorIDLess>> vertexTouchers;
  net.vertices.reserve(candidateVertices.size());
  vertexTouchers.reserve(candidateVertices.size());

  for (const auto &cv : candidateVertices)
  {
    ApolloniusVertex av;
    av.cartesianPosition = cv.position;
    av.fractionalPosition = isPeriodic ? (invLattice * cv.position) : cv.position;
    av.clearanceRadius = cv.clearance;

    std::size_t count = 0;
    for (const auto &g : cv.touchers)
    {
      if (count < 4)
      {
        av.generatorAtoms[count] = g.first;
        av.generatorShifts[count] = g.second;
        ++count;
      }
    }
    net.vertices.push_back(av);
    vertexTouchers.push_back(cv.touchers);
  }

  std::set<std::tuple<std::size_t, std::size_t, int, int, int, TripletKey>> constructedEdges;

  auto getShiftedTouchers = [&](std::size_t vIdx, const int3 &shift) -> std::set<GeneratorID, GeneratorIDLess>
  {
    std::set<GeneratorID, GeneratorIDLess> shifted;
    for (const auto &g : vertexTouchers[vIdx])
    {
      shifted.insert({g.first, g.second + shift});
    }
    return shifted;
  };

  int edgeShell = isPeriodic ? 1 : 0;
  for (std::size_t u = 0; u < net.vertices.size(); ++u)
  {
    const auto &touchU = vertexTouchers[u];
    const double3 &posU = net.vertices[u].cartesianPosition;

    for (std::size_t v = u; v < net.vertices.size(); ++v)
    {
      for (int sx = -edgeShell; sx <= edgeShell; ++sx)
      {
        for (int sy = -edgeShell; sy <= edgeShell; ++sy)
        {
          for (int sz = -edgeShell; sz <= edgeShell; ++sz)
          {
            int3 shiftV(sx, sy, sz);
            if (u == v && std::tie(sx, sy, sz) <= std::make_tuple(0, 0, 0)) continue;

            double3 posV = net.vertices[v].cartesianPosition + latticeVectors * double3(sx, sy, sz);
            double chordLen = (posV - posU).length();
            if (chordLen < 1e-5) continue;

            auto touchV = getShiftedTouchers(v, shiftV);
            std::vector<GeneratorID> shared;
            for (const auto &g : touchU)
            {
              if (touchV.find(g) != touchV.end())
              {
                shared.push_back(g);
              }
            }

            if (shared.size() < 3) continue;

            for (std::size_t i = 0; i < shared.size(); ++i)
            {
              for (std::size_t j = i + 1; j < shared.size(); ++j)
              {
                for (std::size_t k = j + 1; k < shared.size(); ++k)
                {
                  std::array<GeneratorID, 3> trip = {shared[i], shared[j], shared[k]};
                  std::sort(trip.begin(), trip.end(), GeneratorIDLess{});

                  std::array<double3, 3> c3;
                  std::array<double, 3> r3;
                  for (std::size_t m = 0; m < 3; ++m)
                  {
                    auto [p, r] = lookupGeneratorPositionAndRadius(trip[m]);
                    c3[m] = p;
                    r3[m] = r;
                  }

                  std::size_t steps = std::max<std::size_t>(4, polylineResolution);
                  std::vector<double3> pts(steps + 1);
                  std::vector<double> clearances(steps + 1);
                  pts[0] = posU;
                  clearances[0] = net.vertices[u].clearanceRadius;
                  pts[steps] = posV;
                  clearances[steps] = net.vertices[v].clearanceRadius;

                  double3 chordDir = (posV - posU) / chordLen;
                  TripletConicParam conicParam = buildTripletConicParam(c3, r3);
                  double sStart = clearances[0] + conicParam.rRef;
                  double sEnd = clearances[steps] + conicParam.rRef;
                  bool monotonicS = conicParam.valid && (std::abs(sEnd - sStart) > 1e-3 * chordLen);

                  bool traceValid = true;
                  for (std::size_t s = 1; s < steps; ++s)
                  {
                    double t = static_cast<double>(s) / static_cast<double>(steps);
                    double3 linearHint = (1.0 - t) * posU + t * posV;
                    double3 p = linearHint;
                    double cl = 0.0;

                    if (monotonicS)
                    {
                      double sTarget = (1.0 - t) * sStart + t * sEnd;
                      if (!evaluateTripletConicAtS(conicParam, c3, r3, sTarget, linearHint, p, cl))
                      {
                        p = linearHint;
                        double planeD = double3::dot(chordDir, linearHint);
                        if (!solvePointOnBisectorWithPlane(c3, r3, chordDir, planeD, p, cl))
                        {
                          traceValid = false;
                          break;
                        }
                      }
                    }
                    else
                    {
                      double planeD = double3::dot(chordDir, linearHint);
                      if (!solvePointOnBisectorWithPlane(c3, r3, chordDir, planeD, p, cl))
                      {
                        traceValid = false;
                        break;
                      }
                    }

                    for (const auto &img : images)
                    {
                      GeneratorID imgGen{img.atomIndex, img.shift};
                      if (imgGen == trip[0] || imgGen == trip[1] || imgGen == trip[2]) continue;
                      double dImg = (p - img.position).length() - img.radius;
                      if (dImg < cl - 5e-4)
                      {
                        traceValid = false;
                        break;
                      }
                    }
                    if (!traceValid) break;

                    pts[s] = p;
                    clearances[s] = cl;
                  }

                  if (!traceValid) continue;

                  std::size_t minIdx = 0;
                  for (std::size_t s = 1; s <= steps; ++s)
                  {
                    if (clearances[s] < clearances[minIdx])
                    {
                      minIdx = s;
                    }
                  }
                  double bottleneckR = clearances[minIdx];
                  double3 bottleneckPos = pts[minIdx];

                  if (minIdx > 0 && minIdx < steps)
                  {
                    double cPrev = clearances[minIdx - 1];
                    double cCurr = clearances[minIdx];
                    double cNext = clearances[minIdx + 1];
                    double denom = 2.0 * (cPrev - 2.0 * cCurr + cNext);
                    if (std::abs(denom) > 1e-12)
                    {
                      double offset = std::clamp((cPrev - cNext) / denom, -0.5, 0.5);
                      double tMin = (static_cast<double>(minIdx) + offset) / static_cast<double>(steps);
                      double3 pCand = (1.0 - tMin) * posU + tMin * posV;
                      double clCand = cCurr;
                      double planeD = double3::dot(chordDir, pCand);
                      if (solvePointOnBisectorWithPlane(c3, r3, chordDir, planeD, pCand, clCand) && clCand < bottleneckR)
                      {
                        bottleneckR = clCand;
                        bottleneckPos = pCand;
                      }
                    }
                  }

                  int3 normBase = trip[0].second;
                  std::array<GeneratorID, 3> normTrip = trip;
                  for (auto &g : normTrip) g.second = g.second - normBase;
                  std::sort(normTrip.begin(), normTrip.end(), GeneratorIDLess{});
                  TripletKey tKey{normTrip[0], normTrip[1], normTrip[2]};

                  auto edgeKey = std::make_tuple(u, v, sx, sy, sz, tKey);
                  if (constructedEdges.find(edgeKey) != constructedEdges.end()) continue;
                  constructedEdges.insert(edgeKey);

                  ApolloniusEdge edge;
                  edge.startVertex = u;
                  edge.endVertex = v;
                  edge.periodicShift = shiftV;
                  edge.bottleneckRadius = bottleneckR;
                  edge.bottleneckPosition = bottleneckPos;
                  for (std::size_t m = 0; m < 3; ++m)
                  {
                    edge.generatorAtoms[m] = trip[m].first;
                    edge.generatorShifts[m] = trip[m].second;
                  }
                  edge.curvatureType = classifyTripletEdge(c3, r3);
                  edge.polylinePoints = std::move(pts);
                  edge.polylineClearances = std::move(clearances);

                  net.edges.push_back(std::move(edge));
                }
              }
            }
          }
        }
      }
    }
  }

  {
    std::map<std::tuple<std::size_t, std::size_t, int, int, int>, std::vector<std::size_t>> endpointGroups;
    for (std::size_t eIdx = 0; eIdx < net.edges.size(); ++eIdx)
    {
      const auto &e = net.edges[eIdx];
      endpointGroups[{e.startVertex, e.endVertex, e.periodicShift.x, e.periodicShift.y, e.periodicShift.z}].push_back(eIdx);
    }

    std::vector<bool> keepEdge(net.edges.size(), true);
    for (const auto &[key, group] : endpointGroups)
    {
      if (group.size() <= 1) continue;
      for (std::size_t i = 0; i < group.size(); ++i)
      {
        std::size_t eA = group[i];
        if (!keepEdge[eA]) continue;
        double3 midA = net.edges[eA].polylinePoints[net.edges[eA].polylinePoints.size() / 2];
        for (std::size_t j = i + 1; j < group.size(); ++j)
        {
          std::size_t eB = group[j];
          if (!keepEdge[eB]) continue;
          double3 midB = net.edges[eB].polylinePoints[net.edges[eB].polylinePoints.size() / 2];
          if ((midA - midB).length_squared() < 1e-4)
          {
            keepEdge[eB] = false;
          }
        }
      }
    }

    std::vector<ApolloniusEdge> filteredEdges;
    filteredEdges.reserve(net.edges.size());
    for (std::size_t eIdx = 0; eIdx < net.edges.size(); ++eIdx)
    {
      if (keepEdge[eIdx])
      {
        filteredEdges.push_back(std::move(net.edges[eIdx]));
      }
    }
    net.edges = std::move(filteredEdges);
  }

  net.cells.resize(atoms.size());
  for (std::size_t i = 0; i < atoms.size(); ++i)
  {
    net.cells[i].atomIndex = i;
  }

  if (equalRadii)
  {
    using PairKey = std::tuple<std::size_t, std::size_t, int, int, int>;
    std::map<PairKey, std::vector<UnwrappedSegment>> pairToSegments;

    for (std::size_t eIdx = 0; eIdx < net.edges.size(); ++eIdx)
    {
      const auto &edge = net.edges[eIdx];
      for (int a = 0; a < 3; ++a)
      {
        for (int b = a + 1; b < 3; ++b)
        {
          GeneratorID gA{edge.generatorAtoms[static_cast<std::size_t>(a)], edge.generatorShifts[static_cast<std::size_t>(a)]};
          GeneratorID gB{edge.generatorAtoms[static_cast<std::size_t>(b)], edge.generatorShifts[static_cast<std::size_t>(b)]};

          if (GeneratorIDLess{}(gB, gA))
          {
            std::swap(gA, gB);
          }

          int3 shiftAnchor = gA.second;
          int3 relShiftB = gB.second - shiftAnchor;
          PairKey key{gA.first, gB.first, relShiftB.x, relShiftB.y, relShiftB.z};

          int3 shiftStart = -shiftAnchor;
          int3 shiftEnd = edge.periodicShift - shiftAnchor;
          double3 startPos = net.vertices[edge.startVertex].cartesianPosition +
                             latticeVectors * double3(shiftStart.x, shiftStart.y, shiftStart.z);
          double3 endPos = net.vertices[edge.endVertex].cartesianPosition +
                           latticeVectors * double3(shiftEnd.x, shiftEnd.y, shiftEnd.z);

          pairToSegments[key].push_back(UnwrappedSegment{eIdx, startPos, endPos, shiftStart, shiftEnd});
        }
      }
    }

    for (auto &[key, segments] : pairToSegments)
    {
      auto [atomA, atomB, sx, sy, sz] = key;
      int3 shiftB(sx, sy, sz);

      ApolloniusFace face;
      face.atomA = atomA;
      face.atomB = atomB;
      face.periodicShiftB = shiftB;

      std::set<std::size_t> uniqueEdges;
      std::set<std::size_t> uniqueVerts;
      for (const auto &seg : segments)
      {
        uniqueEdges.insert(seg.edgeIndex);
        uniqueVerts.insert(net.edges[seg.edgeIndex].startVertex);
        uniqueVerts.insert(net.edges[seg.edgeIndex].endVertex);
      }
      face.edgeIndices.assign(uniqueEdges.begin(), uniqueEdges.end());
      face.vertexIndices.assign(uniqueVerts.begin(), uniqueVerts.end());

      std::vector<double3> loopPoints = stitchFaceBoundaryLoop(segments, net.edges, latticeVectors);
      auto [cA, rA] = lookupGeneratorPositionAndRadius({atomA, int3(0, 0, 0)});
      auto [cB, rB] = lookupGeneratorPositionAndRadius({atomB, shiftB});

      triangulateCurvedFace(face, loopPoints, cA, rA, cB, rB, polylineResolution);

      std::size_t faceIdx = net.faces.size();
      net.faces.push_back(std::move(face));

      if (atomA < net.cells.size())
      {
        net.cells[atomA].faceIndices.push_back(faceIdx);
      }
      if (atomB < net.cells.size() && (atomB != atomA || shiftB != int3(0, 0, 0)))
      {
        net.cells[atomB].faceIndices.push_back(faceIdx);
      }
    }

    for (std::size_t i = 0; i < atoms.size(); ++i)
    {
      auto &cell = net.cells[i];
      std::set<std::size_t> cellVerts;
      double totalArea = 0.0;
      for (std::size_t fIdx : cell.faceIndices)
      {
        const auto &face = net.faces[fIdx];
        for (std::size_t vIdx : face.vertexIndices) cellVerts.insert(vIdx);
        for (std::size_t t = 0; t + 2 < face.triangles.size(); t += 3)
        {
          double3 e1 = face.triangles[t + 1] - face.triangles[t];
          double3 e2 = face.triangles[t + 2] - face.triangles[t];
          totalArea += 0.5 * double3::cross(e1, e2).length();
        }
      }
      cell.vertexIndices.assign(cellVerts.begin(), cellVerts.end());
      cell.surfaceArea = totalArea;
      if (i < radNet.cells.size())
      {
        cell.volume = radNet.cells[i].volume;
      }
    }
  }
  else
  {
    net.vertices.clear();
    net.edges.clear();
    net.faces.clear();

    auto findOrAddExactVertex = [&](const double3 &rawPos,
                                    const std::array<GeneratorID, 4> &gens) -> std::pair<std::size_t, int3>
    {
      double3 wrappedCart = rawPos;
      double3 frac = invLattice * rawPos;
      int3 shift(0, 0, 0);
      if (isPeriodic)
      {
        shift = int3(static_cast<int>(std::floor(frac.x + 1e-6)),
                     static_cast<int>(std::floor(frac.y + 1e-6)),
                     static_cast<int>(std::floor(frac.z + 1e-6)));
        frac = frac - double3(shift.x, shift.y, shift.z);
        wrappedCart = latticeVectors * frac;
      }

      constexpr double MERGE_TOL = 2e-2;
      for (std::size_t idx = 0; idx < net.vertices.size(); ++idx)
      {
        double3 delta = minImageVector(net.vertices[idx].cartesianPosition - wrappedCart,
                                       latticeVectors, invLattice, isPeriodic);
        if (delta.length_squared() < MERGE_TOL * MERGE_TOL)
        {
          return {idx, shift};
        }
      }

      ApolloniusVertex av;
      av.cartesianPosition = wrappedCart;
      av.fractionalPosition = frac;
      auto [c0, r0] = lookupGeneratorPositionAndRadius(gens[0]);
      av.clearanceRadius = (rawPos - c0).length() - r0;
      for (std::size_t k = 0; k < 4; ++k)
      {
        av.generatorAtoms[k] = gens[k].first;
        av.generatorShifts[k] = gens[k].second - shift;
      }
      std::size_t newIdx = net.vertices.size();
      net.vertices.push_back(av);
      return {newIdx, shift};
    };

    struct AddedEdgeKey
    {
      std::size_t u;
      std::size_t v;
      int3 shift;
      double3 midWrapped;
    };
    std::vector<AddedEdgeKey> addedEdges;

    for (std::size_t i = 0; i < atoms.size(); ++i)
    {
      ExactApolloniusCellResult exactCell = computeExactApolloniusCell(i, atoms, latticeVectors, isPeriodic, polylineResolution);
      auto &cell = net.cells[i];
      cell.volume = exactCell.volume;
      cell.surfaceArea = exactCell.surfaceArea;

      std::set<std::size_t> cellVertSet;
      GeneratorID gA{i, int3(0, 0, 0)};

      for (auto &sf : exactCell.subfaces)
      {
        GeneratorID gB{sf.neighborAtomIndex, sf.periodicShift};

        ApolloniusFace face;
        face.atomA = i;
        face.atomB = sf.neighborAtomIndex;
        face.periodicShiftB = sf.periodicShift;
        face.triangles = std::move(sf.triangles);
        face.triangleNormals = std::move(sf.triangleNormals);

        std::size_t m = sf.boundaryVertices.size();
        std::vector<std::pair<std::size_t, int3>> loopVertIds(m);
        for (std::size_t k = 0; k < m; ++k)
        {
          GeneratorID gPrev = sf.boundaryThirdGenerators[(k + m - 1) % m];
          GeneratorID gCurr = sf.boundaryThirdGenerators[k];
          std::array<GeneratorID, 4> gens = {gA, gB, gPrev, gCurr};
          loopVertIds[k] = findOrAddExactVertex(sf.boundaryVertices[k], gens);
          face.vertexIndices.push_back(loopVertIds[k].first);
          cellVertSet.insert(loopVertIds[k].first);
        }

        for (const auto &poly : sf.edgePolylines)
        {
          if (poly.size() < 2) continue;
          std::array<GeneratorID, 4> dummyGens = {gA, gB, gA, gB};
          auto [uIdx, shiftU] = findOrAddExactVertex(poly.front(), dummyGens);
          auto [vIdx, shiftV] = findOrAddExactVertex(poly.back(), dummyGens);
          cellVertSet.insert(uIdx);
          cellVertSet.insert(vIdx);

          int3 edgeShift = shiftV - shiftU;
          double3 shiftVecU = latticeVectors * double3(shiftU.x, shiftU.y, shiftU.z);
          double3 midRaw = poly[poly.size() / 2] - shiftVecU;
          double3 midFrac = invLattice * midRaw;
          if (isPeriodic)
          {
            midFrac.x -= std::floor(midFrac.x + 1e-6);
            midFrac.y -= std::floor(midFrac.y + 1e-6);
            midFrac.z -= std::floor(midFrac.z + 1e-6);
          }
          double3 midWrapped = isPeriodic ? (latticeVectors * midFrac) : midRaw;

          std::size_t existingEdgeIdx = std::numeric_limits<std::size_t>::max();
          for (std::size_t eIdx = 0; eIdx < addedEdges.size(); ++eIdx)
          {
            double3 dMid = minImageVector(addedEdges[eIdx].midWrapped - midWrapped,
                                          latticeVectors, invLattice, isPeriodic);
            if (dMid.length_squared() < 4e-2)
            {
              bool matchDirect = (addedEdges[eIdx].u == uIdx && addedEdges[eIdx].v == vIdx && addedEdges[eIdx].shift == edgeShift);
              bool matchRev = (addedEdges[eIdx].u == vIdx && addedEdges[eIdx].v == uIdx && addedEdges[eIdx].shift == -edgeShift);
              if (matchDirect || matchRev)
              {
                existingEdgeIdx = eIdx;
                break;
              }
            }
          }

          if (existingEdgeIdx != std::numeric_limits<std::size_t>::max())
          {
            face.edgeIndices.push_back(existingEdgeIdx);
            continue;
          }

          ApolloniusEdge edge;
          edge.startVertex = uIdx;
          edge.endVertex = vIdx;
          edge.periodicShift = edgeShift;
          edge.curvatureType = EdgeCurvatureType::Hyperbola;

          double minCl = std::numeric_limits<double>::max();
          double3 minPos = poly.front() - shiftVecU;
          for (const auto &pt : poly)
          {
            double3 shiftedPt = pt - shiftVecU;
            double cl = (pt - atoms[i].first).length() - atoms[i].second;
            edge.polylinePoints.push_back(shiftedPt);
            edge.polylineClearances.push_back(cl);
            if (cl < minCl)
            {
              minCl = cl;
              minPos = shiftedPt;
            }
          }
          edge.bottleneckRadius = minCl;
          edge.bottleneckPosition = minPos;
          edge.generatorAtoms = {i, sf.neighborAtomIndex, i};
          edge.generatorShifts = {int3(0, 0, 0) - shiftU, sf.periodicShift - shiftU, int3(0, 0, 0) - shiftU};

          std::size_t newEdgeIdx = net.edges.size();
          net.edges.push_back(std::move(edge));
          addedEdges.push_back(AddedEdgeKey{uIdx, vIdx, edgeShift, midWrapped});
          face.edgeIndices.push_back(newEdgeIdx);
        }

        std::size_t faceIdx = net.faces.size();
        net.faces.push_back(std::move(face));
        cell.faceIndices.push_back(faceIdx);
      }

      cell.vertexIndices.assign(cellVertSet.begin(), cellVertSet.end());
    }
  }

  return net;
}
