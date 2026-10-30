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

#include "skspheresweep.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <tuple>

namespace
{
struct CircleOnSphere
{
  int otherBall{0};
  int3 otherImage{0, 0, 0};
  double3 otherCenter;
  double3 axisUnit;
  double3 center3D;
  double circleRadius{0.0};
  double cosAlpha{0.0};
  double sinAlpha{0.0};
  double planeOffset{0.0};
  double3 uAxis;
  double3 vAxis;
};

struct ArcVertex
{
  double3 position;
};

struct DirectedArc
{
  int circleIndex{0};
  int otherBall{0};
  int3 otherImage{0, 0, 0};
  double3 otherCenter;
  double theta0{0.0};
  double theta1{0.0};
  bool reversed{false};
  int startVertex{-1};
  int endVertex{-1};
  bool visited{false};
  int concavePatchIndex{-1};
  int saddlePatchIndex{-1};
};

double clampUnit(double x)
{
  return std::clamp(x, -1.0, 1.0);
}

double3 circlePoint(const CircleOnSphere &c, double theta)
{
  return c.center3D + c.circleRadius * (std::cos(theta) * c.uAxis + std::sin(theta) * c.vAxis);
}

bool pointCoveredByBall(const double3 &p, const SKSphereSweep::Ball &b, double eps = 1e-9)
{
  double3 d = p - b.center;
  return double3::dot(d, d) < (b.radius - eps) * (b.radius - eps);
}

double sphericalExcessTriangle(const double3 &u1, const double3 &u2, const double3 &u3)
{
  double det = double3::dot(u1, double3::cross(u2, u3));
  double denom = 1.0 + double3::dot(u1, u2) + double3::dot(u2, u3) + double3::dot(u3, u1);
  return 2.0 * std::atan2(det, denom);
}

double geodesicTurnAngle(const double3 &pUnit, const double3 &tIn, const double3 &tOut)
{
  double3 pIn = tIn - double3::dot(tIn, pUnit) * pUnit;
  double3 pOut = tOut - double3::dot(tOut, pUnit) * pUnit;
  double nIn = pIn.length();
  double nOut = pOut.length();
  if (nIn < 1e-14 || nOut < 1e-14)
  {
    return 0.0;
  }
  pIn /= nIn;
  pOut /= nOut;
  double sinTurn = double3::dot(pUnit, double3::cross(pIn, pOut));
  double cosTurn = clampUnit(double3::dot(pIn, pOut));
  return std::atan2(sinTurn, cosTurn);
}

std::vector<SKSphereSweep::Ball> buildActiveBalls(
    const std::vector<std::pair<double3, double>> &atoms,
    const double3x3 &lattice,
    bool periodic,
    double probeRadius)
{
  std::vector<SKSphereSweep::Ball> baseBalls;
  baseBalls.reserve(atoms.size());
  double maxR = 0.0;
  for (std::size_t idx = 0; idx < atoms.size(); ++idx)
  {
    double r = atoms[idx].second + probeRadius;
    if (r <= 0.0) continue;
    maxR = std::max(maxR, r);
    baseBalls.push_back(SKSphereSweep::Ball{atoms[idx].first, r, int3{0, 0, 0}, static_cast<int>(idx)});
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

  std::vector<SKSphereSweep::Ball> allBalls;
  for (const auto &ball : baseBalls)
  {
    if (ball.imageShift == int3{0, 0, 0})
    {
      allBalls.push_back(ball);
    }
  }
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
          SKSphereSweep::Ball shifted = ball;
          shifted.center = ball.center + shift;
          shifted.imageShift = int3{ix, iy, iz};
          bool touchesPrimary = false;
          for (const auto &primary : baseBalls)
          {
            double3 diff = shifted.center - primary.center;
            double sumR = shifted.radius + primary.radius;
            if (double3::dot(diff, diff) < sumR * sumR + 1e-9)
            {
              touchesPrimary = true;
              break;
            }
          }
          if (touchesPrimary)
          {
            allBalls.push_back(shifted);
          }
        }
      }
    }
  }

  return allBalls;
}
} // namespace

void SKSphereSweep::makeCircleFrame(const double3 &axisUnit, double3 &uOut, double3 &vOut)
{
  double3 ref = (std::abs(axisUnit.z) < 0.9) ? double3{0.0, 0.0, 1.0} : double3{1.0, 0.0, 0.0};
  uOut = double3::cross(axisUnit, ref);
  double normU = uOut.length();
  if (normU < 1e-15)
  {
    ref = double3{0.0, 1.0, 0.0};
    uOut = double3::cross(axisUnit, ref);
    normU = uOut.length();
  }
  uOut /= normU;
  vOut = double3::cross(axisUnit, uOut);
}

double SKSphereSweep::wrapAngle0To2Pi(double angle)
{
  constexpr double twoPi = 2.0 * M_PI;
  angle = std::fmod(angle, twoPi);
  if (angle < 0.0)
  {
    angle += twoPi;
  }
  return angle;
}

double SKSphereSweep::positiveAngleDifference(double fromAngle, double toAngle)
{
  constexpr double twoPi = 2.0 * M_PI;
  double d = wrapAngle0To2Pi(toAngle - fromAngle);
  if (d <= 1e-13)
  {
    d += twoPi;
  }
  return d;
}

SKSphereSweep::Decomposition SKSphereSweep::compute(
    const std::vector<std::pair<double3, double>> &atoms,
    const double3x3 &latticeVectors,
    bool isPeriodic,
    double probeRadius)
{
  Decomposition result;
  result.probeRadius = probeRadius;

  std::vector<Ball> balls = buildActiveBalls(atoms, latticeVectors, isPeriodic, probeRadius);

  int numPrimary = 0;
  for (const auto &b : balls)
  {
    if (b.imageShift == int3{0, 0, 0})
    {
      ++numPrimary;
    }
  }

  for (int i = 0; i < numPrimary; ++i)
  {
    const Ball &bi = balls[static_cast<std::size_t>(i)];
    double ri = bi.radius;

    AtomFace face;
    face.atomIndex = bi.atomIndex;
    face.center = bi.center;
    face.radius = ri;

    bool completelySubmerged = false;
    std::vector<CircleOnSphere> circles;
    std::vector<int> neighborBalls;

    for (std::size_t j = 0; j < balls.size(); ++j)
    {
      if (static_cast<int>(j) == i) continue;
      const Ball &bj = balls[j];
      double rj = bj.radius;
      double3 diff = bj.center - bi.center;
      double d2 = double3::dot(diff, diff);
      double d = std::sqrt(d2);

      if (d >= ri + rj - 1e-12) continue;

      if (d <= rj - ri + 1e-12)
      {
        completelySubmerged = true;
        break;
      }
      if (d <= ri - rj + 1e-12)
      {
        continue;
      }

      neighborBalls.push_back(static_cast<int>(j));

      double cosAlpha = clampUnit((ri * ri + d2 - rj * rj) / (2.0 * ri * d));
      double sinAlpha = std::sqrt(std::max(0.0, 1.0 - cosAlpha * cosAlpha));
      double3 axisUnit = diff / d;
      double h = ri * cosAlpha;
      double rho = ri * sinAlpha;

      CircleOnSphere c;
      c.otherBall = bj.atomIndex;
      c.otherImage = bj.imageShift;
      c.otherCenter = bj.center;
      c.axisUnit = axisUnit;
      c.center3D = bi.center + h * axisUnit;
      c.circleRadius = rho;
      c.cosAlpha = cosAlpha;
      c.sinAlpha = sinAlpha;
      c.planeOffset = h;
      makeCircleFrame(axisUnit, c.uAxis, c.vAxis);
      circles.push_back(c);
    }

    if (completelySubmerged)
    {
      result.atomFaces.push_back(std::move(face));
      continue;
    }

    if (circles.empty())
    {
      face.solidAngle = 4.0 * M_PI;
      face.area = 4.0 * M_PI * ri * ri;
      face.volumeWedge = (4.0 * M_PI / 3.0) * ri * ri * ri;
      face.volumeDirect = face.volumeWedge;
      result.totalArea += face.area;
      result.totalVolumeWedge += face.volumeWedge;
      result.totalVolumeDirect += face.volumeDirect;
      if (probeRadius > 0.0)
      {
        double rAtom = std::max(0.0, ri - probeRadius);
        result.totalContactArea += 4.0 * M_PI * rAtom * rAtom;
        result.totalSESArea += 4.0 * M_PI * rAtom * rAtom;
      }
      result.atomFaces.push_back(std::move(face));
      continue;
    }

    auto coveredByNeighbors = [&](const double3 &p, int ignoreCircleA = -1, int ignoreCircleB = -1) -> bool
    {
      for (std::size_t idx = 0; idx < neighborBalls.size(); ++idx)
      {
        if (static_cast<int>(idx) == ignoreCircleA || static_cast<int>(idx) == ignoreCircleB)
        {
          continue;
        }
        const Ball &bj = balls[static_cast<std::size_t>(neighborBalls[idx])];
        if (pointCoveredByBall(p, bj, 1e-8))
        {
          return true;
        }
      }
      return false;
    };

    std::vector<ArcVertex> vertices;
    double vertexMergeTol = 1e-6 * ri;
    auto findOrAddVertex = [&](const double3 &pos) -> int
    {
      for (std::size_t v = 0; v < vertices.size(); ++v)
      {
        double3 d = vertices[v].position - pos;
        if (double3::dot(d, d) <= vertexMergeTol * vertexMergeTol)
        {
          return static_cast<int>(v);
        }
      }
      vertices.push_back(ArcVertex{pos});
      return static_cast<int>(vertices.size() - 1);
    };

    std::vector<DirectedArc> arcs;

    for (std::size_t ci = 0; ci < circles.size(); ++ci)
    {
      const CircleOnSphere &c1 = circles[ci];
      std::vector<double> cutAngles;

      for (std::size_t cj = 0; cj < circles.size(); ++cj)
      {
        if (ci == cj) continue;
        const CircleOnSphere &c2 = circles[cj];

        double3 n1 = c1.axisUnit;
        double3 n2 = c2.axisUnit;
        double h1 = c1.planeOffset;
        double h2 = c2.planeOffset;

        double3 crossN = double3::cross(n1, n2);
        double sinGamma2 = double3::dot(crossN, crossN);
        if (sinGamma2 < 1e-18)
        {
          continue;
        }

        double dotN = double3::dot(n1, n2);
        double a = (h1 - h2 * dotN) / sinGamma2;
        double b = (h2 - h1 * dotN) / sinGamma2;
        double3 p0Rel = a * n1 + b * n2;
        double dist2 = double3::dot(p0Rel, p0Rel);
        double rad2 = ri * ri - dist2;
        if (rad2 <= 1e-14 * ri * ri)
        {
          continue;
        }
        double tLen = std::sqrt(std::max(0.0, rad2));
        double3 lineDir = crossN / std::sqrt(sinGamma2);

        for (int sign : {-1, +1})
        {
          double3 pRel = p0Rel + double(sign) * tLen * lineDir;
          double3 p = bi.center + pRel;
          double3 relToCircle = p - c1.center3D;
          double x = double3::dot(relToCircle, c1.uAxis);
          double y = double3::dot(relToCircle, c1.vAxis);
          double theta = wrapAngle0To2Pi(std::atan2(y, x));
          cutAngles.push_back(theta);
        }
      }

      if (cutAngles.empty())
      {
        double3 sample = circlePoint(c1, 0.0);
        if (!coveredByNeighbors(sample, static_cast<int>(ci), -1))
        {
          DirectedArc arc;
          arc.circleIndex = static_cast<int>(ci);
          arc.otherBall = c1.otherBall;
          arc.otherImage = c1.otherImage;
          arc.otherCenter = c1.otherCenter;
          arc.theta0 = 0.0;
          arc.theta1 = 2.0 * M_PI;
          arc.reversed = true;
          arc.startVertex = -1;
          arc.endVertex = -1;
          arcs.push_back(arc);
        }
        continue;
      }

      std::sort(cutAngles.begin(), cutAngles.end());
      std::vector<double> uniqueAngles;
      double angleMergeTol = 1e-7;
      for (double ang : cutAngles)
      {
        if (uniqueAngles.empty() || std::abs(ang - uniqueAngles.back()) > angleMergeTol)
        {
          uniqueAngles.push_back(ang);
        }
      }
      if (uniqueAngles.size() >= 2 && (2.0 * M_PI - uniqueAngles.back() + uniqueAngles.front()) <= angleMergeTol)
      {
        uniqueAngles.pop_back();
      }

      if (uniqueAngles.size() < 2)
      {
        double3 sample = circlePoint(c1, uniqueAngles.empty() ? 0.0 : wrapAngle0To2Pi(uniqueAngles[0] + M_PI));
        if (!coveredByNeighbors(sample, static_cast<int>(ci), -1))
        {
          DirectedArc arc;
          arc.circleIndex = static_cast<int>(ci);
          arc.otherBall = c1.otherBall;
          arc.otherImage = c1.otherImage;
          arc.otherCenter = c1.otherCenter;
          arc.theta0 = 0.0;
          arc.theta1 = 2.0 * M_PI;
          arc.reversed = true;
          arc.startVertex = -1;
          arc.endVertex = -1;
          arcs.push_back(arc);
        }
        continue;
      }

      std::size_t m = uniqueAngles.size();
      for (std::size_t k = 0; k < m; ++k)
      {
        double a0 = uniqueAngles[k];
        double a1 = uniqueAngles[(k + 1) % m];
        double dTheta = positiveAngleDifference(a0, a1);
        if (dTheta <= 1e-9) continue;

        double midTheta = wrapAngle0To2Pi(a0 + 0.5 * dTheta);
        double3 midPt = circlePoint(c1, midTheta);

        if (!coveredByNeighbors(midPt, static_cast<int>(ci), -1))
        {
          double3 pStart = circlePoint(c1, a1);
          double3 pEnd = circlePoint(c1, a0);
          int vStart = findOrAddVertex(pStart);
          int vEnd = findOrAddVertex(pEnd);
          if (vStart == vEnd && dTheta < M_PI)
          {
            continue;
          }

          DirectedArc arc;
          arc.circleIndex = static_cast<int>(ci);
          arc.otherBall = c1.otherBall;
          arc.otherImage = c1.otherImage;
          arc.otherCenter = c1.otherCenter;
          arc.theta0 = a1;
          arc.theta1 = a0;
          arc.reversed = true;
          arc.startVertex = vStart;
          arc.endVertex = vEnd;
          arcs.push_back(arc);
        }
      }
    }

    auto arcTangentStart = [&](const DirectedArc &arc) -> double3
    {
      const CircleOnSphere &c = circles[static_cast<std::size_t>(arc.circleIndex)];
      double th = arc.theta0;
      double3 dPdTheta = -std::sin(th) * c.uAxis + std::cos(th) * c.vAxis;
      return arc.reversed ? -dPdTheta : dPdTheta;
    };

    auto arcTangentEnd = [&](const DirectedArc &arc) -> double3
    {
      const CircleOnSphere &c = circles[static_cast<std::size_t>(arc.circleIndex)];
      double th = arc.theta1;
      double3 dPdTheta = -std::sin(th) * c.uAxis + std::cos(th) * c.vAxis;
      return arc.reversed ? -dPdTheta : dPdTheta;
    };

    auto arcSweepAngle = [](const DirectedArc &arc) -> double
    {
      if (arc.startVertex == -1 && arc.endVertex == -1)
      {
        return 2.0 * M_PI;
      }
      return arc.reversed ? positiveAngleDifference(arc.theta1, arc.theta0)
                          : positiveAngleDifference(arc.theta0, arc.theta1);
    };

    std::vector<std::vector<int>> outgoing(vertices.size());
    for (std::size_t ai = 0; ai < arcs.size(); ++ai)
    {
      if (arcs[ai].startVertex >= 0)
      {
        outgoing[static_cast<std::size_t>(arcs[ai].startVertex)].push_back(static_cast<int>(ai));
      }
    }

    std::vector<std::vector<int>> cycleArcIndices;

    for (std::size_t ai = 0; ai < arcs.size(); ++ai)
    {
      if (arcs[ai].visited) continue;

      if (arcs[ai].startVertex == -1)
      {
        arcs[ai].visited = true;
        cycleArcIndices.push_back({static_cast<int>(ai)});
        continue;
      }

      std::vector<int> currentCycle;
      int curArc = static_cast<int>(ai);
      int startVert = arcs[ai].startVertex;
      bool closed = false;

      while (curArc >= 0 && !arcs[static_cast<std::size_t>(curArc)].visited)
      {
        arcs[static_cast<std::size_t>(curArc)].visited = true;
        currentCycle.push_back(curArc);
        int vNext = arcs[static_cast<std::size_t>(curArc)].endVertex;
        if (vNext == startVert)
        {
          closed = true;
          break;
        }
        if (vNext < 0 || outgoing[static_cast<std::size_t>(vNext)].empty())
        {
          break;
        }

        double3 pUnit = (vertices[static_cast<std::size_t>(vNext)].position - bi.center) / ri;
        double3 tIn = arcTangentEnd(arcs[static_cast<std::size_t>(curArc)]);

        int bestNext = -1;
        double bestTurn = -1e100;
        for (int cand : outgoing[static_cast<std::size_t>(vNext)])
        {
          if (arcs[static_cast<std::size_t>(cand)].visited) continue;
          double3 tOut = arcTangentStart(arcs[static_cast<std::size_t>(cand)]);
          double turn = geodesicTurnAngle(pUnit, tIn, tOut);
          if (turn > bestTurn)
          {
            bestTurn = turn;
            bestNext = cand;
          }
        }
        curArc = bestNext;
      }

      if (closed && !currentCycle.empty())
      {
        cycleArcIndices.push_back(std::move(currentCycle));
      }
    }

    auto sampleInteriorPointNearArc = [&](const DirectedArc &arc) -> double3
    {
      const CircleOnSphere &c = circles[static_cast<std::size_t>(arc.circleIndex)];
      double dTheta = arcSweepAngle(arc);
      double midTheta = (arc.startVertex == -1) ? 0.0
                        : (arc.reversed ? wrapAngle0To2Pi(arc.theta0 - 0.5 * dTheta)
                                        : wrapAngle0To2Pi(arc.theta0 + 0.5 * dTheta));
      double3 pArc = circlePoint(c, midTheta);
      double3 uArc = (pArc - bi.center) / ri;
      double3 tArc = arc.reversed ? (-std::sin(midTheta) * c.uAxis + std::cos(midTheta) * c.vAxis) * (-1.0)
                                  : (-std::sin(midTheta) * c.uAxis + std::cos(midTheta) * c.vAxis);
      tArc /= std::max(1e-15, tArc.length());
      double3 inward = double3::cross(uArc, tArc);
      inward /= std::max(1e-15, inward.length());

      for (double eps : {1e-3, 1e-4, 1e-2, 5e-2, 1e-1})
      {
        double3 uTest = std::cos(eps) * uArc + std::sin(eps) * inward;
        uTest /= uTest.length();
        double3 pTest = bi.center + ri * uTest;
        if (!coveredByNeighbors(pTest, -1, -1))
        {
          return uTest;
        }
      }
      return uArc;
    };

    double sumCurvatureIntegral = 0.0;
    double sumWedgeCircles = 0.0;
    double sumCapVolumeTerms = 0.0;

    for (const auto &cyc : cycleArcIndices)
    {
      Cycle cycleOut;
      cycleOut.interiorPoint = bi.center + ri * sampleInteriorPointNearArc(arcs[static_cast<std::size_t>(cyc[0])]);

      std::size_t m = cyc.size();
      for (std::size_t k = 0; k < m; ++k)
      {
        const DirectedArc &da = arcs[static_cast<std::size_t>(cyc[k])];
        const CircleOnSphere &c = circles[static_cast<std::size_t>(da.circleIndex)];
        double dTheta = arcSweepAngle(da);

        double kG = -c.cosAlpha / std::max(1e-15, ri * c.sinAlpha);
        double ds = (ri * c.sinAlpha) * dTheta;
        sumCurvatureIntegral += kG * ds;

        if (m >= 2)
        {
          const DirectedArc &daNext = arcs[static_cast<std::size_t>(cyc[(k + 1) % m])];
          int vCorner = da.endVertex;
          double3 pUnit = (vertices[static_cast<std::size_t>(vCorner)].position - bi.center) / ri;
          double3 tIn = arcTangentEnd(da);
          double3 tOut = arcTangentStart(daNext);
          double turn = geodesicTurnAngle(pUnit, tIn, tOut);
          sumCurvatureIntegral += turn;
        }

        double h = c.planeOffset;
        double rho = c.circleRadius;
        double capSliceArea = 0.5 * rho * rho * dTheta;
        if (da.startVertex >= 0 && da.endVertex >= 0 && m >= 2)
        {
          double3 pStart = vertices[static_cast<std::size_t>(da.startVertex)].position - c.center3D;
          double3 pEnd = vertices[static_cast<std::size_t>(da.endVertex)].position - c.center3D;
          double triSigned = 0.5 * double3::dot(c.axisUnit, double3::cross(pEnd, pStart));
          capSliceArea -= triSigned;
        }
        double capSliceVolume = (1.0 / 3.0) * h * capSliceArea;
        sumCapVolumeTerms += capSliceVolume;

        double d = (c.otherCenter - bi.center).length();
        double wedgeFactor = 0.5 * d * h * (rho * rho) * dTheta / 3.0;
        sumWedgeCircles += wedgeFactor;

        Arc arcOut;
        arcOut.otherBall = da.otherBall;
        arcOut.otherImage = da.otherImage;
        arcOut.otherCenter = da.otherCenter;
        arcOut.theta0 = da.theta0;
        arcOut.theta1 = da.theta1;
        arcOut.reversed = da.reversed;
        cycleOut.arcs.push_back(arcOut);
      }

      face.cycles.push_back(std::move(cycleOut));
    }

    double baseOmega = std::fmod(sumCurvatureIntegral, 2.0 * M_PI);
    if (baseOmega < 0.0) baseOmega += 2.0 * M_PI;

    double omegaCandidateA = baseOmega;
    double omegaCandidateB = baseOmega + 2.0 * M_PI;

    double numericOmega = 0.0;
    {
      constexpr int nSamples = 256;
      const double goldenAngle = M_PI * (3.0 - std::sqrt(5.0));
      int exposedCount = 0;
      for (int s = 0; s < nSamples; ++s)
      {
        double z = 1.0 - (2.0 * (s + 0.5)) / double(nSamples);
        double rad = std::sqrt(std::max(0.0, 1.0 - z * z));
        double phi = double(s) * goldenAngle;
        double3 u{rad * std::cos(phi), rad * std::sin(phi), z};
        double3 p = bi.center + ri * u;
        if (!coveredByNeighbors(p, -1, -1))
        {
          ++exposedCount;
        }
      }
      numericOmega = (4.0 * M_PI * double(exposedCount)) / double(nSamples);
    }

    double omega = (std::abs(omegaCandidateA - numericOmega) <= std::abs(omegaCandidateB - numericOmega))
                       ? omegaCandidateA
                       : omegaCandidateB;
    omega = std::clamp(omega, 0.0, 4.0 * M_PI);

    face.solidAngle = omega;
    face.area = ri * ri * omega;
    face.volumeWedge = (ri * face.area) / 3.0 - sumWedgeCircles;

    double polygonPyramidTerm = 0.0;
    for (const auto &cyc : cycleArcIndices)
    {
      if (cyc.size() < 3) continue;
      int v0 = arcs[static_cast<std::size_t>(cyc[0])].startVertex;
      double3 u0 = (vertices[static_cast<std::size_t>(v0)].position - bi.center) / ri;
      for (std::size_t k = 1; k + 1 < cyc.size(); ++k)
      {
        int v1 = arcs[static_cast<std::size_t>(cyc[k])].startVertex;
        int v2 = arcs[static_cast<std::size_t>(cyc[k + 1])].startVertex;
        double3 u1 = (vertices[static_cast<std::size_t>(v1)].position - bi.center) / ri;
        double3 u2 = (vertices[static_cast<std::size_t>(v2)].position - bi.center) / ri;
        polygonPyramidTerm += (ri * ri * ri / 6.0) * double3::dot(u0, double3::cross(u1, u2));
      }
    }
    face.volumeDirect = (ri * face.area) / 3.0 - sumCapVolumeTerms + polygonPyramidTerm;
    if (std::abs(face.volumeDirect - face.volumeWedge) > 0.1 * ri * ri * ri)
    {
      face.volumeDirect = face.volumeWedge;
    }

    result.totalArea += face.area;
    result.totalVolumeWedge += face.volumeWedge;
    result.totalVolumeDirect += face.volumeDirect;

    if (probeRadius > 0.0)
    {
      double rAtom = std::max(0.0, ri - probeRadius);
      double contactA = rAtom * rAtom * omega;
      result.totalContactArea += contactA;
    }

    result.atomFaces.push_back(std::move(face));
  }

  if (probeRadius > 0.0)
  {
    using PairKey = std::tuple<int, int, int, int, int>;
    std::map<PairKey, SaddlePatch> saddleMap;

    for (std::size_t fi = 0; fi < result.atomFaces.size(); ++fi)
    {
      const AtomFace &face = result.atomFaces[fi];
      int atomI = face.atomIndex;
      double rI = face.radius;
      double rAtomI = std::max(0.0, rI - probeRadius);

      for (const auto &cyc : face.cycles)
      {
        for (const auto &arc : cyc.arcs)
        {
          int atomJ = arc.otherBall;
          int3 shiftJ = arc.otherImage;
          if (atomI > atomJ || (atomI == atomJ && (shiftJ.x < 0 || (shiftJ.x == 0 && (shiftJ.y < 0 || (shiftJ.y == 0 && shiftJ.z <= 0))))))
          {
            continue;
          }

          double rJ = atoms[static_cast<std::size_t>(atomJ)].second + probeRadius;
          double rAtomJ = atoms[static_cast<std::size_t>(atomJ)].second;
          double3 cI = face.center;
          double3 cJ = arc.otherCenter;
          double3 diff = cJ - cI;
          double d = diff.length();
          if (d <= 1e-12 || d >= rI + rJ) continue;

          double cosPsiI = clampUnit((rI * rI + d * d - rJ * rJ) / (2.0 * rI * d));
          double cosPsiJ = clampUnit((rJ * rJ + d * d - rI * rI) / (2.0 * rJ * d));
          double psiI = std::acos(cosPsiI);
          double psiJ = std::acos(cosPsiJ);
          double sinPsiI = std::sqrt(std::max(0.0, 1.0 - cosPsiI * cosPsiI));
          double Rtorus = rI * sinPsiI;
          double3 axis = diff / d;
          double3 torusCenter = cI + (rI * cosPsiI) * axis;

          double dPhi = (arc.theta0 == 0.0 && arc.theta1 == 2.0 * M_PI)
                            ? 2.0 * M_PI
                            : (arc.reversed ? positiveAngleDifference(arc.theta1, arc.theta0)
                                            : positiveAngleDifference(arc.theta0, arc.theta1));

          double thetaMax = 0.5 * M_PI - psiI;
          double thetaMin = -(0.5 * M_PI - psiJ);
          if (Rtorus < probeRadius)
          {
            double cosLim = clampUnit(Rtorus / probeRadius);
            double thetaSelf = std::acos(cosLim);
            thetaMax = std::min(thetaMax, thetaSelf);
            thetaMin = std::max(thetaMin, -thetaSelf);
          }

          double patchArea = 0.0;
          if (thetaMax > thetaMin)
          {
            double integralTerm = Rtorus * (thetaMax - thetaMin) - probeRadius * (std::sin(thetaMax) - std::sin(thetaMin));
            patchArea = std::max(0.0, probeRadius * dPhi * integralTerm);
          }

          SaddlePatch sp;
          sp.atomI = atomI;
          sp.atomJ = atomJ;
          sp.imageShiftJ = shiftJ;
          sp.centerI = cI;
          sp.radiusI = rAtomI;
          sp.centerJ = cJ;
          sp.radiusJ = rAtomJ;
          sp.torusCenter = torusCenter;
          sp.axis = axis;
          sp.majorRadius = Rtorus;
          sp.phiMin = arc.theta1;
          sp.phiMax = arc.theta0;
          sp.psiI = psiI;
          sp.psiJ = psiJ;
          sp.arcs.push_back({arc.theta1, arc.theta0});
          sp.area = patchArea;

          result.totalTorusArea += patchArea;
          result.saddlePatches.push_back(sp);
        }
      }
    }

    double cornerMergeTol = 1e-5 * probeRadius;
    struct RawCorner
    {
      double3 probePos;
      int atomI;
      int atomJ;
      int atomK;
      int3 shiftJ;
      int3 shiftK;
      double3 centerI;
      double3 centerJ;
      double3 centerK;
    };
    std::vector<RawCorner> uniqueProbes;

    for (const auto &face : result.atomFaces)
    {
      double rI = face.radius;
      for (const auto &cyc : face.cycles)
      {
        std::size_t m = cyc.arcs.size();
        if (m < 2) continue;
        for (std::size_t k = 0; k < m; ++k)
        {
          const Arc &a1 = cyc.arcs[k];
          const Arc &a2 = cyc.arcs[(k + 1) % m];
          if (a1.theta0 == 0.0 && a1.theta1 == 2.0 * M_PI) continue;

          double3 diff = a1.otherCenter - face.center;
          double d = diff.length();
          if (d <= 1e-12) continue;
          double rJ = atoms[static_cast<std::size_t>(a1.otherBall)].second + probeRadius;
          double cosAlpha = clampUnit((rI * rI + d * d - rJ * rJ) / (2.0 * rI * d));
          double sinAlpha = std::sqrt(std::max(0.0, 1.0 - cosAlpha * cosAlpha));
          double3 axis = diff / d;
          double3 u, v;
          makeCircleFrame(axis, u, v);
          double3 c3D = face.center + (rI * cosAlpha) * axis;
          double rho = rI * sinAlpha;
          double th = a1.theta1;
          double3 probePos = c3D + rho * (std::cos(th) * u + std::sin(th) * v);

          bool alreadyHave = false;
          for (const auto &existing : uniqueProbes)
          {
            double3 dp = existing.probePos - probePos;
            if (double3::dot(dp, dp) <= cornerMergeTol * cornerMergeTol)
            {
              alreadyHave = true;
              break;
            }
          }
          if (!alreadyHave)
          {
            uniqueProbes.push_back(RawCorner{
                probePos,
                face.atomIndex,
                a1.otherBall,
                a2.otherBall,
                a1.otherImage,
                a2.otherImage,
                face.center,
                a1.otherCenter,
                a2.otherCenter});
          }
        }
      }
    }

    for (const auto &rc : uniqueProbes)
    {
      double3 uI = (rc.centerI - rc.probePos);
      double3 uJ = (rc.centerJ - rc.probePos);
      double3 uK = (rc.centerK - rc.probePos);
      double lenI = uI.length();
      double lenJ = uJ.length();
      double lenK = uK.length();
      if (lenI < 1e-12 || lenJ < 1e-12 || lenK < 1e-12) continue;
      uI /= lenI;
      uJ /= lenJ;
      uK /= lenK;

      double excess = std::abs(sphericalExcessTriangle(uI, uJ, uK));
      double area = probeRadius * probeRadius * excess;

      int primaryCount = 1;
      if (rc.shiftJ == int3{0, 0, 0}) ++primaryCount;
      if (rc.shiftK == int3{0, 0, 0}) ++primaryCount;
      double weight = double(primaryCount) / 3.0;

      ConcaveCorner cc;
      cc.atomI = rc.atomI;
      cc.atomJ = rc.atomJ;
      cc.atomK = rc.atomK;
      cc.imageShiftJ = rc.shiftJ;
      cc.imageShiftK = rc.shiftK;
      cc.probeCenter = rc.probePos;
      cc.touchI = rc.probePos + probeRadius * uI;
      cc.touchJ = rc.probePos + probeRadius * uJ;
      cc.touchK = rc.probePos + probeRadius * uK;
      cc.area = area * weight;
      cc.multiplicity = primaryCount;

      result.totalConcaveArea += cc.area;
      result.concaveCorners.push_back(cc);
    }

    result.totalSESArea = result.totalContactArea + result.totalTorusArea + result.totalConcaveArea;
  }
  else
  {
    result.totalContactArea = result.totalArea;
    result.totalSESArea = result.totalArea;
  }

  return result;
}

std::vector<float4> SKSphereSweep::triangulate(
    const Decomposition &decomposition,
    const double3x3 &inverseCell,
    bool includeExtendedPatches,
    int maxSubdivisionDepth)
{
  std::vector<float4> vertices;
  double probeRadius = decomposition.probeRadius;
  int depth = std::clamp(maxSubdivisionDepth, 1, 5);
  int arcStepsPerRadian = 4 * (1 << (depth - 1));

  double3x3 unitCellTranspose = double3x3::transpose(inverseCell.inverse());

  auto emitTriangle = [&](const double3 &p0, const double3 &n0,
                          const double3 &p1, const double3 &n1,
                          const double3 &p2, const double3 &n2)
  {
    double3 u0 = inverseCell * p0;
    double3 u1 = inverseCell * p1;
    double3 u2 = inverseCell * p2;
    double3 tn0 = unitCellTranspose * n0;
    double3 tn1 = unitCellTranspose * n1;
    double3 tn2 = unitCellTranspose * n2;
    vertices.push_back(float4(float(u0.x), float(u0.y), float(u0.z), 1.0f));
    vertices.push_back(float4(float(tn0.x), float(tn0.y), float(tn0.z), 0.0f));
    vertices.push_back(float4(0.0f, 0.0f, 0.0f, 0.0f));
    vertices.push_back(float4(float(u1.x), float(u1.y), float(u1.z), 1.0f));
    vertices.push_back(float4(float(tn1.x), float(tn1.y), float(tn1.z), 0.0f));
    vertices.push_back(float4(0.0f, 0.0f, 0.0f, 0.0f));
    vertices.push_back(float4(float(u2.x), float(u2.y), float(u2.z), 1.0f));
    vertices.push_back(float4(float(tn2.x), float(tn2.y), float(tn2.z), 0.0f));
    vertices.push_back(float4(0.0f, 0.0f, 0.0f, 0.0f));
  };

  std::function<void(const double3&, double, const double3&, const double3&, const double3&, double, int)> emitSphericalTriangle;
  emitSphericalTriangle = [&](const double3 &center, double radius,
                              const double3 &u0, const double3 &u1, const double3 &u2,
                              double normalSign, int level)
  {
    if (level <= 0)
    {
      double3 p0 = center + radius * u0;
      double3 p1 = center + radius * u1;
      double3 p2 = center + radius * u2;
      double3 n0 = normalSign * u0;
      double3 n1 = normalSign * u1;
      double3 n2 = normalSign * u2;
      if (normalSign > 0.0)
      {
        emitTriangle(p0, n0, p1, n1, p2, n2);
      }
      else
      {
        emitTriangle(p0, n0, p2, n2, p1, n1);
      }
      return;
    }
    auto normVec = [](const double3 &v) -> double3
    {
      double len = v.length();
      return len > 1e-15 ? (v / len) : double3(0.0, 0.0, 1.0);
    };
    double3 m01 = normVec(u0 + u1);
    double3 m12 = normVec(u1 + u2);
    double3 m20 = normVec(u2 + u0);
    emitSphericalTriangle(center, radius, u0, m01, m20, normalSign, level - 1);
    emitSphericalTriangle(center, radius, m01, u1, m12, normalSign, level - 1);
    emitSphericalTriangle(center, radius, m20, m12, u2, normalSign, level - 1);
    emitSphericalTriangle(center, radius, m01, m12, m20, normalSign, level - 1);
  };

  auto emitFullSphere = [&](const double3 &center, double radius, int subdivDepth)
  {
    const double3 octV[6] = {
      double3( 1,  0,  0), double3(-1,  0,  0),
      double3( 0,  1,  0), double3( 0, -1,  0),
      double3( 0,  0,  1), double3( 0,  0, -1)
    };
    const int octFaces[8][3] = {
      {0, 2, 4}, {2, 1, 4}, {1, 3, 4}, {3, 0, 4},
      {2, 0, 5}, {1, 2, 5}, {3, 1, 5}, {0, 3, 5}
    };
    for (int f = 0; f < 8; ++f)
    {
      emitSphericalTriangle(center, radius, octV[octFaces[f][0]], octV[octFaces[f][1]], octV[octFaces[f][2]], 1.0, subdivDepth);
    }
  };

  for (const auto &face : decomposition.atomFaces)
  {
    if (face.area <= 1e-12) continue;
    double rExpanded = face.radius;
    double rRender = includeExtendedPatches ? rExpanded : std::max(1e-4, rExpanded - probeRadius);

    if (face.cycles.empty())
    {
      emitFullSphere(face.center, rRender, depth);
      continue;
    }

    for (const auto &cyc : face.cycles)
    {
      double3 uInt = cyc.interiorPoint - face.center;
      double lenInt = uInt.length();
      if (lenInt <= 1e-12) continue;
      uInt /= lenInt;

      std::vector<double3> polyUnit;
      for (const auto &arc : cyc.arcs)
      {
        double3 diff = arc.otherCenter - face.center;
        double d = diff.length();
        if (d <= 1e-12) continue;
        double3 axis = diff / d;
        double3 uAxis, vAxis;
        makeCircleFrame(axis, uAxis, vAxis);

        double dTheta = (arc.theta0 == 0.0 && arc.theta1 == 2.0 * M_PI)
                            ? 2.0 * M_PI
                            : (arc.reversed ? positiveAngleDifference(arc.theta1, arc.theta0)
                                            : positiveAngleDifference(arc.theta0, arc.theta1));

        double rJ = rExpanded;
        for (const auto &otherFace : decomposition.atomFaces)
        {
          if (otherFace.atomIndex == arc.otherBall)
          {
            rJ = otherFace.radius;
            break;
          }
        }
        double cosAlpha = clampUnit((rExpanded * rExpanded + d * d - rJ * rJ) / (2.0 * rExpanded * d));
        double sinAlpha = std::sqrt(std::max(0.0, 1.0 - cosAlpha * cosAlpha));

        int nSteps = std::max(3, int(std::ceil(dTheta * double(arcStepsPerRadian))));
        for (int s = 0; s < nSteps; ++s)
        {
          double frac = double(s) / double(nSteps);
          double th = arc.reversed ? wrapAngle0To2Pi(arc.theta0 - frac * dTheta)
                                   : wrapAngle0To2Pi(arc.theta0 + frac * dTheta);
          double3 uPt = cosAlpha * axis + sinAlpha * (std::cos(th) * uAxis + std::sin(th) * vAxis);
          double lenPt = uPt.length();
          if (lenPt > 1e-15)
          {
            polyUnit.push_back(uPt / lenPt);
          }
        }
      }

      if (polyUnit.size() < 3) continue;
      int m = int(polyUnit.size());
      for (int k = 0; k < m; ++k)
      {
        const double3 &uA = polyUnit[std::size_t(k)];
        const double3 &uB = polyUnit[std::size_t((k + 1) % m)];
        emitSphericalTriangle(face.center, rRender, uInt, uA, uB, 1.0, std::max(0, depth - 2));
      }
    }
  }

  if (!includeExtendedPatches && probeRadius > 0.0)
  {
    for (const auto &sp : decomposition.saddlePatches)
    {
      if (sp.area <= 1e-12 || sp.arcs.empty()) continue;
      double3 uAxis, vAxis;
      makeCircleFrame(sp.axis, uAxis, vAxis);

      double thetaMax = 0.5 * M_PI - sp.psiI;
      double thetaMin = -(0.5 * M_PI - sp.psiJ);
      if (sp.majorRadius < probeRadius)
      {
        double cosLim = clampUnit(sp.majorRadius / probeRadius);
        double thetaSelf = std::acos(cosLim);
        thetaMax = std::min(thetaMax, thetaSelf);
        thetaMin = std::max(thetaMin, -thetaSelf);
      }
      if (thetaMax <= thetaMin) continue;

      int nV = std::max(3, 2 * (1 << (depth - 1)));
      for (const auto &arcRange : sp.arcs)
      {
        double phiEnd = arcRange.first;
        double phiStart = arcRange.second;
        double dPhi = (phiStart == 0.0 && phiEnd == 2.0 * M_PI)
                          ? 2.0 * M_PI
                          : positiveAngleDifference(phiEnd, phiStart);
        int nU = std::max(3, int(std::ceil(dPhi * double(arcStepsPerRadian))));

        for (int iu = 0; iu < nU; ++iu)
        {
          double f0 = double(iu) / double(nU);
          double f1 = double(iu + 1) / double(nU);
          double phi0 = wrapAngle0To2Pi(phiStart - f0 * dPhi);
          double phi1 = wrapAngle0To2Pi(phiStart - f1 * dPhi);
          double3 rad0 = std::cos(phi0) * uAxis + std::sin(phi0) * vAxis;
          double3 rad1 = std::cos(phi1) * uAxis + std::sin(phi1) * vAxis;
          double3 pc0 = sp.torusCenter + sp.majorRadius * rad0;
          double3 pc1 = sp.torusCenter + sp.majorRadius * rad1;

          for (int iv = 0; iv < nV; ++iv)
          {
            double g0 = double(iv) / double(nV);
            double g1 = double(iv + 1) / double(nV);
            double th0 = thetaMin + g0 * (thetaMax - thetaMin);
            double th1 = thetaMin + g1 * (thetaMax - thetaMin);

            double3 n00 = std::cos(th0) * rad0 - std::sin(th0) * sp.axis;
            double3 n10 = std::cos(th0) * rad1 - std::sin(th0) * sp.axis;
            double3 n01 = std::cos(th1) * rad0 - std::sin(th1) * sp.axis;
            double3 n11 = std::cos(th1) * rad1 - std::sin(th1) * sp.axis;

            double3 p00 = pc0 - probeRadius * n00;
            double3 p10 = pc1 - probeRadius * n10;
            double3 p01 = pc0 - probeRadius * n01;
            double3 p11 = pc1 - probeRadius * n11;

            emitTriangle(p00, n00, p10, n10, p11, n11);
            emitTriangle(p00, n00, p11, n11, p01, n01);
          }
        }
      }
    }

    for (const auto &cc : decomposition.concaveCorners)
    {
      if (cc.area <= 1e-12) continue;
      double3 uI = (cc.touchI - cc.probeCenter) / probeRadius;
      double3 uJ = (cc.touchJ - cc.probeCenter) / probeRadius;
      double3 uK = (cc.touchK - cc.probeCenter) / probeRadius;
      double det = double3::dot(uI, double3::cross(uJ, uK));
      if (det > 0.0)
      {
        emitSphericalTriangle(cc.probeCenter, probeRadius, uI, uJ, uK, -1.0, std::max(1, depth - 1));
      }
      else
      {
        emitSphericalTriangle(cc.probeCenter, probeRadius, uI, uK, uJ, -1.0, std::max(1, depth - 1));
      }
    }
  }

  return vertices;
}
