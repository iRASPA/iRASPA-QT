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

#include "skapolloniusporeanalysis.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace
{
struct DirectedAdj
{
  std::size_t neighbor;
  int3 shift;
  double bottleneckRadius;
  std::size_t edgeIndex;
};

int latticeRank(const std::vector<int3> &vectors)
{
  std::vector<std::array<double, 3>> rows;
  rows.reserve(vectors.size());
  for (const auto &v : vectors)
  {
    if (v.x != 0 || v.y != 0 || v.z != 0)
    {
      rows.push_back({double(v.x), double(v.y), double(v.z)});
    }
  }
  if (rows.empty()) return 0;

  int rank = 0;
  int nRows = int(rows.size());
  for (int col = 0; col < 3; ++col)
  {
    if (rank >= nRows) break;
    int pivot = -1;
    double best = 1e-9;
    for (int r = rank; r < nRows; ++r)
    {
      double val = std::abs(rows[std::size_t(r)][std::size_t(col)]);
      if (val > best)
      {
        best = val;
        pivot = r;
      }
    }
    if (pivot < 0) continue;
    std::swap(rows[std::size_t(rank)], rows[std::size_t(pivot)]);
    double pv = rows[std::size_t(rank)][std::size_t(col)];
    for (int c = col; c < 3; ++c)
    {
      rows[std::size_t(rank)][std::size_t(c)] /= pv;
    }
    for (int r = 0; r < nRows; ++r)
    {
      if (r == rank) continue;
      double factor = rows[std::size_t(r)][std::size_t(col)];
      if (std::abs(factor) > 1e-12)
      {
        for (int c = col; c < 3; ++c)
        {
          rows[std::size_t(r)][std::size_t(c)] -= factor * rows[std::size_t(rank)][std::size_t(c)];
        }
      }
    }
    rank += 1;
  }
  return rank;
}

std::tuple<bool, bool, bool> latticeSpansAxes(const std::vector<int3> &vectors)
{
  bool hasX = false;
  bool hasY = false;
  bool hasZ = false;
  for (const auto &v : vectors)
  {
    if (v.x != 0) hasX = true;
    if (v.y != 0) hasY = true;
    if (v.z != 0) hasZ = true;
  }
  return {hasX, hasY, hasZ};
}

struct ComponentAnalysis
{
  std::vector<std::size_t> vertices;
  int dimensionality{0};
  std::vector<int3> cycleVectors;
  std::unordered_map<std::size_t, int3> referenceShifts;
};

std::vector<ComponentAnalysis> extractConnectedComponents(
    std::size_t numVertices,
    const std::vector<bool> &vertexActive,
    const std::vector<std::vector<DirectedAdj>> &adj)
{
  std::vector<bool> visited(numVertices, false);
  std::vector<int3> shiftToRoot(numVertices, int3(0, 0, 0));
  std::vector<ComponentAnalysis> results;

  for (std::size_t start = 0; start < numVertices; ++start)
  {
    if (!vertexActive[start] || visited[start]) continue;

    ComponentAnalysis comp;
    std::vector<std::size_t> queue = {start};
    std::size_t head = 0;
    visited[start] = true;
    shiftToRoot[start] = int3(0, 0, 0);
    comp.referenceShifts[start] = int3(0, 0, 0);

    while (head < queue.size())
    {
      std::size_t u = queue[head];
      head += 1;
      comp.vertices.push_back(u);
      int3 su = shiftToRoot[u];

      for (const auto &edge : adj[u])
      {
        std::size_t v = edge.neighbor;
        if (!vertexActive[v]) continue;
        int3 candidateShift = su + edge.shift;
        if (!visited[v])
        {
          visited[v] = true;
          shiftToRoot[v] = candidateShift;
          comp.referenceShifts[v] = candidateShift;
          queue.push_back(v);
        }
        else
        {
          int3 diff = candidateShift - shiftToRoot[v];
          if (diff.x != 0 || diff.y != 0 || diff.z != 0)
          {
            comp.cycleVectors.push_back(diff);
          }
        }
      }
    }

    comp.dimensionality = latticeRank(comp.cycleVectors);
    results.push_back(std::move(comp));
  }

  return results;
}

std::tuple<double, double, double, double> computeLimitingDiameters(
    const SKApolloniusDiagram::ApolloniusNetwork &network)
{
  std::size_t n = network.vertices.size();
  if (n == 0 || network.edges.empty())
  {
    return {0.0, 0.0, 0.0, 0.0};
  }

  std::set<double> rSet;
  for (const auto &v : network.vertices)
  {
    if (v.clearanceRadius > 0.0)
    {
      rSet.insert(v.clearanceRadius);
    }
  }
  for (const auto &e : network.edges)
  {
    if (e.bottleneckRadius > 0.0)
    {
      rSet.insert(e.bottleneckRadius);
    }
  }
  std::vector<double> thresholds(rSet.rbegin(), rSet.rend());
  if (thresholds.empty())
  {
    return {0.0, 0.0, 0.0, 0.0};
  }

  auto evaluateAtRadius = [&](double rProbe) -> std::tuple<bool, bool, bool>
  {
    std::vector<bool> active(n, false);
    for (std::size_t i = 0; i < n; ++i)
    {
      if (network.vertices[i].clearanceRadius >= rProbe - 1e-9)
      {
        active[i] = true;
      }
    }
    std::vector<std::vector<DirectedAdj>> adj(n);
    for (std::size_t eIdx = 0; eIdx < network.edges.size(); ++eIdx)
    {
      const auto &e = network.edges[eIdx];
      if (e.bottleneckRadius >= rProbe - 1e-9 && active[e.startVertex] && active[e.endVertex])
      {
        adj[e.startVertex].push_back(DirectedAdj{e.endVertex, e.periodicShift, e.bottleneckRadius, eIdx});
        adj[e.endVertex].push_back(DirectedAdj{e.startVertex, -e.periodicShift, e.bottleneckRadius, eIdx});
      }
    }
    auto comps = extractConnectedComponents(n, active, adj);
    bool spanA = false;
    bool spanB = false;
    bool spanC = false;
    for (const auto &c : comps)
    {
      if (c.dimensionality >= 1)
      {
        auto [ax, ay, az] = latticeSpansAxes(c.cycleVectors);
        if (ax) spanA = true;
        if (ay) spanB = true;
        if (az) spanC = true;
      }
    }
    return {spanA, spanB, spanC};
  };

  auto binarySearchThreshold = [&](auto predicate) -> double
  {
    int lo = 0;
    int hi = int(thresholds.size()) - 1;
    int bestIdx = -1;
    while (lo <= hi)
    {
      int mid = (lo + hi) / 2;
      if (predicate(thresholds[std::size_t(mid)]))
      {
        bestIdx = mid;
        hi = mid - 1;
      }
      else
      {
        lo = mid + 1;
      }
    }
    return bestIdx >= 0 ? 2.0 * thresholds[std::size_t(bestIdx)] : 0.0;
  };

  double dfA = binarySearchThreshold([&](double r) { return std::get<0>(evaluateAtRadius(r)); });
  double dfB = binarySearchThreshold([&](double r) { return std::get<1>(evaluateAtRadius(r)); });
  double dfC = binarySearchThreshold([&](double r) { return std::get<2>(evaluateAtRadius(r)); });
  double globalDf = std::max({dfA, dfB, dfC});

  return {globalDf, dfA, dfB, dfC};
}

double computeIncludedAlongFreePath(
    const SKApolloniusDiagram::ApolloniusNetwork &network,
    double globalDf)
{
  double rLim = 0.5 * globalDf;
  if (rLim <= 0.0) return 0.0;

  std::size_t n = network.vertices.size();
  std::vector<bool> active(n, false);
  for (std::size_t i = 0; i < n; ++i)
  {
    if (network.vertices[i].clearanceRadius >= rLim - 1e-7)
    {
      active[i] = true;
    }
  }

  std::vector<std::vector<DirectedAdj>> adj(n);
  for (std::size_t eIdx = 0; eIdx < network.edges.size(); ++eIdx)
  {
    const auto &e = network.edges[eIdx];
    if (e.bottleneckRadius >= rLim - 1e-7 && active[e.startVertex] && active[e.endVertex])
    {
      adj[e.startVertex].push_back(DirectedAdj{e.endVertex, e.periodicShift, e.bottleneckRadius, eIdx});
      adj[e.endVertex].push_back(DirectedAdj{e.startVertex, -e.periodicShift, e.bottleneckRadius, eIdx});
    }
  }

  auto comps = extractConnectedComponents(n, active, adj);
  double maxR = 0.0;
  for (const auto &c : comps)
  {
    if (c.dimensionality >= 1)
    {
      for (std::size_t vIdx : c.vertices)
      {
        maxR = std::max(maxR, network.vertices[vIdx].clearanceRadius);
      }
    }
  }
  return 2.0 * maxR;
}

std::vector<std::pair<double3, double>> deduplicateAndPruneSpheres(
    const std::vector<std::pair<double3, double>> &spheres,
    const double3x3 &latticeVectors,
    const double3x3 &invLattice,
    bool isPeriodic,
    double maxPocketSpheres = 256.0)
{
  if (spheres.empty()) return {};

  struct WrappedSphere
  {
    double3 frac;
    double3 cart;
    double radius;
  };
  std::vector<WrappedSphere> wrapped;
  wrapped.reserve(spheres.size());
  for (const auto &sp : spheres)
  {
    if (sp.second <= 1e-4) continue;
    double3 f = isPeriodic ? (invLattice * sp.first) : sp.first;
    if (isPeriodic)
    {
      f.x -= std::floor(f.x);
      f.y -= std::floor(f.y);
      f.z -= std::floor(f.z);
    }
    double3 c = isPeriodic ? (latticeVectors * f) : sp.first;
    wrapped.push_back(WrappedSphere{f, c, sp.second});
  }

  std::sort(wrapped.begin(), wrapped.end(), [](const WrappedSphere &a, const WrappedSphere &b) {
    return a.radius > b.radius;
  });

  std::vector<WrappedSphere> kept;
  for (const auto &cand : wrapped)
  {
    bool redundant = false;
    for (const auto &existing : kept)
    {
      double3 dCart;
      if (isPeriodic)
      {
        double3 df = cand.frac - existing.frac;
        df.x -= std::rint(df.x);
        df.y -= std::rint(df.y);
        df.z -= std::rint(df.z);
        dCart = latticeVectors * df;
      }
      else
      {
        dCart = cand.cart - existing.cart;
      }
      double dist = dCart.length();
      if (dist + cand.radius <= existing.radius + 0.05)
      {
        redundant = true;
        break;
      }
      if (dist < 0.15 && std::abs(cand.radius - existing.radius) < 0.15)
      {
        redundant = true;
        break;
      }
    }
    if (!redundant)
    {
      kept.push_back(cand);
      if (double(kept.size()) >= maxPocketSpheres)
      {
        break;
      }
    }
  }

  std::vector<std::pair<double3, double>> result;
  result.reserve(kept.size());
  for (const auto &k : kept)
  {
    result.push_back({k.frac, k.radius});
  }
  return result;
}

struct GridVoidAnalysis
{
  std::vector<SKApolloniusPoreAnalysis::InaccessiblePocket> pockets;
  std::vector<std::pair<double3, double>> blockingSpheres;
  int channelDimensionality{0};
  double gridDi{0.0};
  double gridDf{0.0};
  double gridDif{0.0};
  double dfAlongA{0.0};
  double dfAlongB{0.0};
  double dfAlongC{0.0};
};

GridVoidAnalysis computeGridVoidAnalysis(
    const std::vector<std::pair<double3, double>> &atoms,
    const double3x3 &latticeVectors,
    double probeRadius,
    bool isPeriodic,
    double gridSpacing = 0.2,
    bool computeMetrics = false)
{
  GridVoidAnalysis emptyResult;
  if (atoms.empty()) return emptyResult;

  double3 aVec = latticeVectors[0];
  double3 bVec = latticeVectors[1];
  double3 cVec = latticeVectors[2];
  double lenA = aVec.length();
  double lenB = bVec.length();
  double lenC = cVec.length();
  if (lenA < 1e-6 || lenB < 1e-6 || lenC < 1e-6) return emptyResult;

  int nx = std::clamp(int(std::ceil(lenA / gridSpacing)), 24, 128);
  int ny = std::clamp(int(std::ceil(lenB / gridSpacing)), 24, 128);
  int nz = std::clamp(int(std::ceil(lenC / gridSpacing)), 24, 128);
  int totalVoxels = nx * ny * nz;

  double3x3 invLattice = latticeVectors.inverse();
  double voxelStepA = lenA / double(nx);
  double voxelStepB = lenB / double(ny);
  double voxelStepC = lenC / double(nz);
  double voxelHalfDiag = 0.5 * std::sqrt(voxelStepA * voxelStepA + voxelStepB * voxelStepB + voxelStepC * voxelStepC);

  struct AtomFrac
  {
    double3 frac;
    double3 cart;
    double vdwRadius;
    double expandedRadius;
  };

  double maxExpandedR = 0.0;
  std::vector<AtomFrac> atomList;
  atomList.reserve(atoms.size());
  for (const auto &at : atoms)
  {
    double3 f = invLattice * at.first;
    if (isPeriodic)
    {
      f.x -= std::floor(f.x);
      f.y -= std::floor(f.y);
      f.z -= std::floor(f.z);
    }
    double3 c = isPeriodic ? (latticeVectors * f) : at.first;
    double expR = at.second + probeRadius;
    maxExpandedR = std::max(maxExpandedR, expR);
    atomList.push_back(AtomFrac{f, c, at.second, expR});
  }

  std::vector<float> clearanceGrid(std::size_t(totalVoxels), computeMetrics ? 1e6f : float(probeRadius + 2.0 * voxelHalfDiag));

  double3 crossBC = double3::cross(bVec, cVec);
  double3 crossCA = double3::cross(cVec, aVec);
  double3 crossAB = double3::cross(aVec, bVec);
  double vol = std::abs(double3::dot(aVec, crossBC));
  double perpA = vol / std::max(1e-12, crossBC.length());
  double perpB = vol / std::max(1e-12, crossCA.length());
  double perpC = vol / std::max(1e-12, crossAB.length());

  double maxInfluenceRadius = maxExpandedR + (computeMetrics ? 0.5 * std::max({perpA, perpB, perpC}) : 2.0 * voxelHalfDiag);

  for (const auto &at : atomList)
  {
    double rInf = computeMetrics ? maxInfluenceRadius : (at.expandedRadius + 2.0 * voxelHalfDiag);
    int dnx = int(std::ceil(rInf / perpA)) + 1;
    int dny = int(std::ceil(rInf / perpB)) + 1;
    int dnz = int(std::ceil(rInf / perpC)) + 1;

    int cx = int(std::floor(at.frac.x * double(nx)));
    int cy = int(std::floor(at.frac.y * double(ny)));
    int cz = int(std::floor(at.frac.z * double(nz)));

    int rxMax = isPeriodic ? std::min(dnx, nx) : dnx;
    int ryMax = isPeriodic ? std::min(dny, ny) : dny;
    int rzMax = isPeriodic ? std::min(dnz, nz) : dnz;

    for (int dz = -rzMax; dz <= rzMax; ++dz)
    {
      int izRaw = cz + dz;
      if (!isPeriodic && (izRaw < 0 || izRaw >= nz)) continue;
      int iz = ((izRaw % nz) + nz) % nz;
      double fz = (double(izRaw) + 0.5) / double(nz);

      for (int dy = -ryMax; dy <= ryMax; ++dy)
      {
        int iyRaw = cy + dy;
        if (!isPeriodic && (iyRaw < 0 || iyRaw >= ny)) continue;
        int iy = ((iyRaw % ny) + ny) % ny;
        double fy = (double(iyRaw) + 0.5) / double(ny);

        int baseIdx = (iz * ny + iy) * nx;
        for (int dx = -rxMax; dx <= rxMax; ++dx)
        {
          int ixRaw = cx + dx;
          if (!isPeriodic && (ixRaw < 0 || ixRaw >= nx)) continue;
          int ix = ((ixRaw % nx) + nx) % nx;
          double fx = (double(ixRaw) + 0.5) / double(nx);

          double3 df(fx - at.frac.x, fy - at.frac.y, fz - at.frac.z);
          double3 dc = latticeVectors * df;
          double dist = dc.length() - at.vdwRadius;
          int vIdx = baseIdx + ix;
          if ( float(dist) < clearanceGrid[std::size_t(vIdx)])
          {
            clearanceGrid[std::size_t(vIdx)] = float(dist);
          }
        }
      }
    }
  }

  float rProbeF = float(probeRadius);
  std::vector<uint8_t> accessible(std::size_t(totalVoxels), 0);
  double globalMaxClearance = 0.0;
  for (int i = 0; i < totalVoxels; ++i)
  {
    double cl = double(clearanceGrid[std::size_t(i)]);
    if (cl > globalMaxClearance)
    {
      globalMaxClearance = cl;
    }
    if (clearanceGrid[std::size_t(i)] >= rProbeF)
    {
      accessible[std::size_t(i)] = 1;
    }
  }

  const int3 neighborOffsets[6] = {
      {1, 0, 0}, {-1, 0, 0},
      {0, 1, 0}, {0, -1, 0},
      {0, 0, 1}, {0, 0, -1}
  };

  std::vector<uint8_t> visited(std::size_t(totalVoxels), 0);
  std::vector<int3> shiftToRoot(std::size_t(totalVoxels), int3(0, 0, 0));

  std::vector<SKApolloniusPoreAnalysis::InaccessiblePocket> pockets;
  std::vector<std::pair<double3, double>> allBlockingSpheres;
  int maxChannelDim = 0;

  for (int startIdx = 0; startIdx < totalVoxels; ++startIdx)
  {
    if (accessible[std::size_t(startIdx)] == 0 || visited[std::size_t(startIdx)] != 0) continue;

    std::vector<int> compVoxels;
    std::vector<int3> cycleVecs;
    std::vector<int> queue = {startIdx};
    std::size_t head = 0;
    visited[std::size_t(startIdx)] = 1;
    shiftToRoot[std::size_t(startIdx)] = int3(0, 0, 0);

    while (head < queue.size())
    {
      int u = queue[head];
      head += 1;
      compVoxels.push_back(u);

      int uz = u / (nx * ny);
      int rem = u % (nx * ny);
      int uy = rem / nx;
      int ux = rem % nx;
      int3 su = shiftToRoot[std::size_t(u)];

      for (const auto &off : neighborOffsets)
      {
        int vx = ux + off.x;
        int vy = uy + off.y;
        int vz = uz + off.z;
        int3 stepShift(0, 0, 0);

        if (isPeriodic)
        {
          if (vx < 0) { vx += nx; stepShift.x = -1; }
          else if (vx >= nx) { vx -= nx; stepShift.x = 1; }
          if (vy < 0) { vy += ny; stepShift.y = -1; }
          else if (vy >= ny) { vy -= ny; stepShift.y = 1; }
          if (vz < 0) { vz += nz; stepShift.z = -1; }
          else if (vz >= nz) { vz -= nz; stepShift.z = 1; }
        }
        else
        {
          if (vx < 0 || vx >= nx || vy < 0 || vy >= ny || vz < 0 || vz >= nz) continue;
        }

        int v = (vz * ny + vy) * nx + vx;
        if (accessible[std::size_t(v)] == 0) continue;

        int3 candShift = su + stepShift;
        if (visited[std::size_t(v)] == 0)
        {
          visited[std::size_t(v)] = 1;
          shiftToRoot[std::size_t(v)] = candShift;
          queue.push_back(v);
        }
        else
        {
          int3 diff = candShift - shiftToRoot[std::size_t(v)];
          if (diff.x != 0 || diff.y != 0 || diff.z != 0)
          {
            cycleVecs.push_back(diff);
          }
        }
      }
    }

    int dim = latticeRank(cycleVecs);
    if (dim >= 1)
    {
      maxChannelDim = std::max(maxChannelDim, dim);
    }
    else
    {
      int rootVoxel = compVoxels[0];
      int rz0 = rootVoxel / (nx * ny);
      int rrem0 = rootVoxel % (nx * ny);
      int ry0 = rrem0 / nx;
      int rx0 = rrem0 % nx;
      double3 rootFrac((double(rx0) + 0.5) / double(nx),
                       (double(ry0) + 0.5) / double(ny),
                       (double(rz0) + 0.5) / double(nz));

      double3 sumUnwrappedFrac(0.0, 0.0, 0.0);
      double maxCl = 0.0;
      std::vector<std::pair<double3, double>> rawPocketSpheres;
      rawPocketSpheres.reserve(compVoxels.size());

      for (int v : compVoxels)
      {
        int vz = v / (nx * ny);
        int vrem = v % (nx * ny);
        int vy = vrem / nx;
        int vx = vrem % nx;
        double3 vFrac((double(vx) + 0.5) / double(nx),
                      (double(vy) + 0.5) / double(ny),
                      (double(vz) + 0.5) / double(nz));
        double3 df = vFrac - rootFrac;
        if (isPeriodic)
        {
          df.x -= std::rint(df.x);
          df.y -= std::rint(df.y);
          df.z -= std::rint(df.z);
        }
        double3 unwrappedF = rootFrac + df;
        sumUnwrappedFrac += unwrappedF;

        double cl = double(clearanceGrid[std::size_t(v)]);
        maxCl = std::max(maxCl, cl);
        double3 cart = latticeVectors * unwrappedF;
        double coverRadius = std::max(cl, probeRadius) + voxelHalfDiag;
        rawPocketSpheres.push_back({cart, coverRadius});
      }

      double3 meanFrac = sumUnwrappedFrac / double(compVoxels.size());
      if (isPeriodic)
      {
        meanFrac.x -= std::floor(meanFrac.x);
        meanFrac.y -= std::floor(meanFrac.y);
        meanFrac.z -= std::floor(meanFrac.z);
      }
      double3 meanCart = latticeVectors * meanFrac;

      double boundR = 0.0;
      for (const auto &sp : rawPocketSpheres)
      {
        double3 dCart;
        if (isPeriodic)
        {
          double3 df = (invLattice * sp.first) - meanFrac;
          df.x -= std::rint(df.x);
          df.y -= std::rint(df.y);
          df.z -= std::rint(df.z);
          dCart = latticeVectors * df;
        }
        else
        {
          dCart = sp.first - meanCart;
        }
        boundR = std::max(boundR, dCart.length() + sp.second);
      }

      auto prunedSpheres = deduplicateAndPruneSpheres(rawPocketSpheres, latticeVectors, invLattice, isPeriodic, 64.0);
      for (const auto &s : prunedSpheres)
      {
        allBlockingSpheres.push_back(s);
      }

      SKApolloniusPoreAnalysis::InaccessiblePocket pocket;
      pocket.fractionalCenter = meanFrac;
      pocket.cartesianCenter = meanCart;
      pocket.maxIncludedSphereDiameter = 2.0 * maxCl;
      pocket.boundingRadius = boundR;
      pocket.sphereUnion = std::move(prunedSpheres);
      pockets.push_back(std::move(pocket));
    }
  }

  double gridDf = 0.0;
  double gridDif = 0.0;
  double dfA = 0.0;
  double dfB = 0.0;
  double dfC = 0.0;

  if (computeMetrics && globalMaxClearance > 0.0)
  {
    auto evaluatePercolation = [&](double rTest) -> std::tuple<bool, bool, bool, double>
    {
      float rTestF = float(rTest);
      std::vector<uint8_t> vis(std::size_t(totalVoxels), 0);
      std::vector<int3> sRoot(std::size_t(totalVoxels), int3(0, 0, 0));
      bool spanA = false;
      bool spanB = false;
      bool spanC = false;
      double maxChCl = 0.0;

      for (int sIdx = 0; sIdx < totalVoxels; ++sIdx)
      {
        if (clearanceGrid[std::size_t(sIdx)] < rTestF || vis[std::size_t(sIdx)] != 0) continue;
        std::vector<int> q = {sIdx};
        std::size_t hd = 0;
        vis[std::size_t(sIdx)] = 1;
        sRoot[std::size_t(sIdx)] = int3(0, 0, 0);
        std::vector<int3> cyc;
        double compMaxCl = 0.0;

        while (hd < q.size())
        {
          int u = q[hd];
          hd += 1;
          double clU = double(clearanceGrid[std::size_t(u)]);
          if (clU > compMaxCl) compMaxCl = clU;

          int uz = u / (nx * ny);
          int rem = u % (nx * ny);
          int uy = rem / nx;
          int ux = rem % nx;
          int3 su = sRoot[std::size_t(u)];

          for (const auto &off : neighborOffsets)
          {
            int vx = ux + off.x;
            int vy = uy + off.y;
            int vz = uz + off.z;
            int3 stepShift(0, 0, 0);
            if (isPeriodic)
            {
              if (vx < 0) { vx += nx; stepShift.x = -1; }
              else if (vx >= nx) { vx -= nx; stepShift.x = 1; }
              if (vy < 0) { vy += ny; stepShift.y = -1; }
              else if (vy >= ny) { vy -= ny; stepShift.y = 1; }
              if (vz < 0) { vz += nz; stepShift.z = -1; }
              else if (vz >= nz) { vz -= nz; stepShift.z = 1; }
            }
            else
            {
              if (vx < 0 || vx >= nx || vy < 0 || vy >= ny || vz < 0 || vz >= nz) continue;
            }
            int v = (vz * ny + vy) * nx + vx;
            if (clearanceGrid[std::size_t(v)] < rTestF) continue;
            int3 candShift = su + stepShift;
            if (vis[std::size_t(v)] == 0)
            {
              vis[std::size_t(v)] = 1;
              sRoot[std::size_t(v)] = candShift;
              q.push_back(v);
            }
            else
            {
              int3 diff = candShift - sRoot[std::size_t(v)];
              if (diff.x != 0 || diff.y != 0 || diff.z != 0)
              {
                cyc.push_back(diff);
              }
            }
          }
        }
        if (!cyc.empty())
        {
          auto [ax, ay, az] = latticeSpansAxes(cyc);
          if (ax) spanA = true;
          if (ay) spanB = true;
          if (az) spanC = true;
          if (ax || ay || az)
          {
            maxChCl = std::max(maxChCl, compMaxCl);
          }
        }
      }
      return {spanA, spanB, spanC, maxChCl};
    };

    auto binarySearchAxis = [&](int axisIndex) -> double
    {
      double lo = 0.0;
      double hi = globalMaxClearance;
      for (int iter = 0; iter < 18; ++iter)
      {
        double mid = 0.5 * (lo + hi);
        auto [sa, sb, sc, mcl] = evaluatePercolation(mid);
        bool ok = (axisIndex == 0) ? sa : ((axisIndex == 1) ? sb : sc);
        if (ok) lo = mid;
        else hi = mid;
      }
      return 2.0 * lo;
    };

    dfA = binarySearchAxis(0);
    dfB = binarySearchAxis(1);
    dfC = binarySearchAxis(2);
    gridDf = std::max({dfA, dfB, dfC});
    if (gridDf > 0.0)
    {
      auto [sa, sb, sc, maxChCl] = evaluatePercolation(0.5 * gridDf - 1e-4);
      gridDif = std::max(gridDf, 2.0 * maxChCl);
    }
  }

  GridVoidAnalysis out;
  out.pockets = std::move(pockets);
  out.blockingSpheres = std::move(allBlockingSpheres);
  out.channelDimensionality = maxChannelDim;
  out.gridDi = 2.0 * globalMaxClearance;
  out.gridDf = gridDf;
  out.gridDif = gridDif;
  out.dfAlongA = dfA;
  out.dfAlongB = dfB;
  out.dfAlongC = dfC;
  return out;
}
std::vector<std::pair<double3, double>> refineGridBlockingSpheres(
    const std::vector<std::pair<double3, double>> &atoms,
    const double3x3 &latticeVectors,
    double probeRadius,
    bool isPeriodic,
    double gridSpacing = 0.2)
{
  if (atoms.empty()) return {};

  double3 aVec = latticeVectors[0];
  double3 bVec = latticeVectors[1];
  double3 cVec = latticeVectors[2];
  double lenA = aVec.length();
  double lenB = bVec.length();
  double lenC = cVec.length();
  if (lenA < 1e-6 || lenB < 1e-6 || lenC < 1e-6) return {};

  int nx = std::clamp(int(std::ceil(lenA / gridSpacing)), 24, 128);
  int ny = std::clamp(int(std::ceil(lenB / gridSpacing)), 24, 128);
  int nz = std::clamp(int(std::ceil(lenC / gridSpacing)), 24, 128);
  int totalVoxels = nx * ny * nz;

  double3x3 invLattice = latticeVectors.inverse();
  double voxelStepA = lenA / double(nx);
  double voxelStepB = lenB / double(ny);
  double voxelStepC = lenC / double(nz);
  double voxelHalfDiag = 0.5 * std::sqrt(voxelStepA * voxelStepA + voxelStepB * voxelStepB + voxelStepC * voxelStepC);

  struct AtomFrac
  {
    double3 frac;
    double3 cart;
    double vdwRadius;
    double expandedRadius;
  };

  std::vector<AtomFrac> atomList;
  atomList.reserve(atoms.size());
  for (const auto &at : atoms)
  {
    double3 f = invLattice * at.first;
    if (isPeriodic)
    {
      f.x -= std::floor(f.x);
      f.y -= std::floor(f.y);
      f.z -= std::floor(f.z);
    }
    double3 c = isPeriodic ? (latticeVectors * f) : at.first;
    double expR = at.second + probeRadius;
    atomList.push_back(AtomFrac{f, c, at.second, expR});
  }

  double3 crossBC = double3::cross(bVec, cVec);
  double3 crossCA = double3::cross(cVec, aVec);
  double3 crossAB = double3::cross(aVec, bVec);
  double vol = std::abs(double3::dot(aVec, crossBC));
  double perpA = vol / std::max(1e-12, crossBC.length());
  double perpB = vol / std::max(1e-12, crossCA.length());
  double perpC = vol / std::max(1e-12, crossAB.length());

  float maxStoreR = float(probeRadius + 4.0 * voxelHalfDiag);
  std::vector<float> clearanceGrid(std::size_t(totalVoxels), maxStoreR);

  for (const auto &at : atomList)
  {
    double rInf = at.expandedRadius + 3.0 * voxelHalfDiag;
    int dnx = int(std::ceil(rInf / perpA)) + 1;
    int dny = int(std::ceil(rInf / perpB)) + 1;
    int dnz = int(std::ceil(rInf / perpC)) + 1;

    int cx = int(std::floor(at.frac.x * double(nx)));
    int cy = int(std::floor(at.frac.y * double(ny)));
    int cz = int(std::floor(at.frac.z * double(nz)));

    int rxMax = isPeriodic ? std::min(dnx, nx) : dnx;
    int ryMax = isPeriodic ? std::min(dny, ny) : dny;
    int rzMax = isPeriodic ? std::min(dnz, nz) : dnz;

    for (int dz = -rzMax; dz <= rzMax; ++dz)
    {
      int izRaw = cz + dz;
      if (!isPeriodic && (izRaw < 0 || izRaw >= nz)) continue;
      int iz = ((izRaw % nz) + nz) % nz;
      double fz = (double(izRaw) + 0.5) / double(nz);

      for (int dy = -ryMax; dy <= ryMax; ++dy)
      {
        int iyRaw = cy + dy;
        if (!isPeriodic && (iyRaw < 0 || iyRaw >= ny)) continue;
        int iy = ((iyRaw % ny) + ny) % ny;
        double fy = (double(iyRaw) + 0.5) / double(ny);

        int baseIdx = (iz * ny + iy) * nx;
        for (int dx = -rxMax; dx <= rxMax; ++dx)
        {
          int ixRaw = cx + dx;
          if (!isPeriodic && (ixRaw < 0 || ixRaw >= nx)) continue;
          int ix = ((ixRaw % nx) + nx) % nx;
          double fx = (double(ixRaw) + 0.5) / double(nx);

          double3 df(fx - at.frac.x, fy - at.frac.y, fz - at.frac.z);
          double3 dc = latticeVectors * df;
          double dist = dc.length() - at.vdwRadius;
          int vIdx = baseIdx + ix;
          if (float(dist) < clearanceGrid[std::size_t(vIdx)])
          {
            clearanceGrid[std::size_t(vIdx)] = float(dist);
          }
        }
      }
    }
  }

  float rCoreThreshold = float(probeRadius);
  float rMarginThreshold = float(std::max(0.0, probeRadius - voxelHalfDiag));

  std::vector<uint8_t> isCore(std::size_t(totalVoxels), 0);
  for (int i = 0; i < totalVoxels; ++i)
  {
    if (clearanceGrid[std::size_t(i)] >= rCoreThreshold)
    {
      isCore[std::size_t(i)] = 1;
    }
  }

  const int3 neighborOffsets6[6] = {
      {1, 0, 0}, {-1, 0, 0},
      {0, 1, 0}, {0, -1, 0},
      {0, 0, 1}, {0, 0, -1}
  };

  std::vector<int3> neighborOffsets26;
  for (int dz = -1; dz <= 1; ++dz)
  {
    for (int dy = -1; dy <= 1; ++dy)
    {
      for (int dx = -1; dx <= 1; ++dx)
      {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        neighborOffsets26.push_back(int3(dx, dy, dz));
      }
    }
  }

  std::vector<uint8_t> visitedCore(std::size_t(totalVoxels), 0);
  std::vector<int3> shiftToRoot(std::size_t(totalVoxels), int3(0, 0, 0));
  std::vector<uint8_t> isChannelCore(std::size_t(totalVoxels), 0);

  for (int startIdx = 0; startIdx < totalVoxels; ++startIdx)
  {
    if (isCore[std::size_t(startIdx)] == 0 || visitedCore[std::size_t(startIdx)] != 0) continue;

    std::vector<int> compVoxels;
    std::vector<int3> cycleVecs;
    std::vector<int> queue = {startIdx};
    std::size_t head = 0;
    visitedCore[std::size_t(startIdx)] = 1;
    shiftToRoot[std::size_t(startIdx)] = int3(0, 0, 0);

    while (head < queue.size())
    {
      int u = queue[head];
      head += 1;
      compVoxels.push_back(u);

      int uz = u / (nx * ny);
      int rem = u % (nx * ny);
      int uy = rem / nx;
      int ux = rem % nx;
      int3 su = shiftToRoot[std::size_t(u)];

      for (const auto &off : neighborOffsets6)
      {
        int vx = ux + off.x;
        int vy = uy + off.y;
        int vz = uz + off.z;
        int3 stepShift(0, 0, 0);

        if (isPeriodic)
        {
          if (vx < 0) { vx += nx; stepShift.x = -1; }
          else if (vx >= nx) { vx -= nx; stepShift.x = 1; }
          if (vy < 0) { vy += ny; stepShift.y = -1; }
          else if (vy >= ny) { vy -= ny; stepShift.y = 1; }
          if (vz < 0) { vz += nz; stepShift.z = -1; }
          else if (vz >= nz) { vz -= nz; stepShift.z = 1; }
        }
        else
        {
          if (vx < 0 || vx >= nx || vy < 0 || vy >= ny || vz < 0 || vz >= nz) continue;
        }

        int v = (vz * ny + vy) * nx + vx;
        if (isCore[std::size_t(v)] == 0) continue;

        int3 candShift = su + stepShift;
        if (visitedCore[std::size_t(v)] == 0)
        {
          visitedCore[std::size_t(v)] = 1;
          shiftToRoot[std::size_t(v)] = candShift;
          queue.push_back(v);
        }
        else
        {
          int3 diff = candShift - shiftToRoot[std::size_t(v)];
          if (diff.x != 0 || diff.y != 0 || diff.z != 0)
          {
            cycleVecs.push_back(diff);
          }
        }
      }
    }

    int dim = latticeRank(cycleVecs);
    if (dim >= 1)
    {
      for (int v : compVoxels)
      {
        isChannelCore[std::size_t(v)] = 1;
      }
    }
  }

  std::vector<uint8_t> isAccessible(std::size_t(totalVoxels), 0);
  std::vector<int> accQueue;
  for (int i = 0; i < totalVoxels; ++i)
  {
    if (isChannelCore[std::size_t(i)] == 1)
    {
      isAccessible[std::size_t(i)] = 1;
      accQueue.push_back(i);
    }
  }

  std::size_t accHead = 0;
  while (accHead < accQueue.size())
  {
    int u = accQueue[accHead];
    accHead += 1;

    int uz = u / (nx * ny);
    int rem = u % (nx * ny);
    int uy = rem / nx;
    int ux = rem % nx;

    for (const auto &off : neighborOffsets26)
    {
      int vx = ux + off.x;
      int vy = uy + off.y;
      int vz = uz + off.z;
      if (isPeriodic)
      {
        if (vx < 0) vx += nx;
        else if (vx >= nx) vx -= nx;
        if (vy < 0) vy += ny;
        else if (vy >= ny) vy -= ny;
        if (vz < 0) vz += nz;
        else if (vz >= nz) vz -= nz;
      }
      else
      {
        if (vx < 0 || vx >= nx || vy < 0 || vy >= ny || vz < 0 || vz >= nz) continue;
      }
      int v = (vz * ny + vy) * nx + vx;
      if (isAccessible[std::size_t(v)] == 0 && clearanceGrid[std::size_t(v)] >= rMarginThreshold)
      {
        isAccessible[std::size_t(v)] = 1;
        accQueue.push_back(v);
      }
    }
  }

  float maxIntDist = float(nx + ny + nz);
  std::vector<float> distToChannel(std::size_t(totalVoxels), maxIntDist);
  std::vector<int> bfsQueue;
  for (int i = 0; i < totalVoxels; ++i)
  {
    if (isAccessible[std::size_t(i)] == 1)
    {
      distToChannel[std::size_t(i)] = 0.0f;
      bfsQueue.push_back(i);
    }
  }

  if (!bfsQueue.empty())
  {
    std::size_t head = 0;
    while (head < bfsQueue.size())
    {
      int u = bfsQueue[head];
      head += 1;
      float dNext = distToChannel[std::size_t(u)] + 1.0f;

      int uz = u / (nx * ny);
      int rem = u % (nx * ny);
      int uy = rem / nx;
      int ux = rem % nx;

      for (const auto &off : neighborOffsets6)
      {
        int vx = ux + off.x;
        int vy = uy + off.y;
        int vz = uz + off.z;
        if (isPeriodic)
        {
          if (vx < 0) vx += nx;
          else if (vx >= nx) vx -= nx;
          if (vy < 0) vy += ny;
          else if (vy >= ny) vy -= ny;
          if (vz < 0) vz += nz;
          else if (vz >= nz) vz -= nz;
        }
        else
        {
          if (vx < 0 || vx >= nx || vy < 0 || vy >= ny || vz < 0 || vz >= nz) continue;
        }
        int v = (vz * ny + vy) * nx + vx;
        if (dNext < distToChannel[std::size_t(v)])
        {
          distToChannel[std::size_t(v)] = dNext;
          bfsQueue.push_back(v);
        }
      }
    }
  }

  double minStep = std::min({voxelStepA, voxelStepB, voxelStepC});

  std::vector<int> pocketVoxels;
  for (int i = 0; i < totalVoxels; ++i)
  {
    if (isAccessible[std::size_t(i)] == 0 && clearanceGrid[std::size_t(i)] >= rMarginThreshold)
    {
      pocketVoxels.push_back(i);
    }
  }
  if (pocketVoxels.empty()) return {};

  struct CandidateSphere
  {
    int voxelIdx;
    double3 frac;
    double3 cart;
    double radius;
  };
  std::vector<CandidateSphere> candidates;
  candidates.reserve(pocketVoxels.size());

  for (int v : pocketVoxels)
  {
    int vz = v / (nx * ny);
    int vrem = v % (nx * ny);
    int vy = vrem / nx;
    int vx = vrem % nx;
    double3 f((double(vx) + 0.5) / double(nx),
              (double(vy) + 0.5) / double(ny),
              (double(vz) + 0.5) / double(nz));
    double3 c = latticeVectors * f;

    double rClearance = double(clearanceGrid[std::size_t(v)]) + voxelHalfDiag;
    double safeRadius = rClearance;
    if (!bfsQueue.empty())
    {
      double dSteps = double(distToChannel[std::size_t(v)]);
      double maxSafeToChannel = std::max(0.0, (dSteps - 1.0) * minStep - voxelHalfDiag);
      safeRadius = std::min(rClearance, maxSafeToChannel);
    }
    double finalR = std::max(voxelHalfDiag * 1.05, safeRadius);
    candidates.push_back(CandidateSphere{v, f, c, finalR});
  }

  std::sort(candidates.begin(), candidates.end(), [](const CandidateSphere &a, const CandidateSphere &b) {
    return a.radius > b.radius;
  });

  std::vector<uint8_t> covered(std::size_t(totalVoxels), 0);
  std::vector<std::pair<double3, double>> selectedSpheres;

  for (const auto &cand : candidates)
  {
    if (covered[std::size_t(cand.voxelIdx)] == 1) continue;

    selectedSpheres.push_back({cand.frac, cand.radius});
    covered[std::size_t(cand.voxelIdx)] = 1;

    double rCover = std::max(0.0, cand.radius - voxelHalfDiag);
    if (rCover <= 0.0) continue;

    int dnx = int(std::ceil(rCover / perpA)) + 1;
    int dny = int(std::ceil(rCover / perpB)) + 1;
    int dnz = int(std::ceil(rCover / perpC)) + 1;

    int vz = cand.voxelIdx / (nx * ny);
    int vrem = cand.voxelIdx % (nx * ny);
    int vy = vrem / nx;
    int vx = vrem % nx;

    int rxMax = isPeriodic ? std::min(dnx, nx / 2) : dnx;
    int ryMax = isPeriodic ? std::min(dny, ny / 2) : dny;
    int rzMax = isPeriodic ? std::min(dnz, nz / 2) : dnz;
    double rCoverSq = rCover * rCover;

    for (int dz = -rzMax; dz <= rzMax; ++dz)
    {
      int izRaw = vz + dz;
      if (!isPeriodic && (izRaw < 0 || izRaw >= nz)) continue;
      int iz = ((izRaw % nz) + nz) % nz;
      double fz = (double(izRaw) + 0.5) / double(nz);

      for (int dy = -ryMax; dy <= ryMax; ++dy)
      {
        int iyRaw = vy + dy;
        if (!isPeriodic && (iyRaw < 0 || iyRaw >= ny)) continue;
        int iy = ((iyRaw % ny) + ny) % ny;
        double fy = (double(iyRaw) + 0.5) / double(ny);

        int baseIdx = (iz * ny + iy) * nx;
        for (int dx = -rxMax; dx <= rxMax; ++dx)
        {
          int ixRaw = vx + dx;
          if (!isPeriodic && (ixRaw < 0 || ixRaw >= nx)) continue;
          int ix = ((ixRaw % nx) + nx) % nx;
          int nIdx = baseIdx + ix;
          if (covered[std::size_t(nIdx)] == 1) continue;

          double fx = (double(ixRaw) + 0.5) / double(nx);
          double3 df(fx - cand.frac.x, fy - cand.frac.y, fz - cand.frac.z);
          double3 dc = latticeVectors * df;
          if (double3::dot(dc, dc) <= rCoverSq)
          {
            covered[std::size_t(nIdx)] = 1;
          }
        }
      }
    }
  }

  return selectedSpheres;
}
} // namespace

SKApolloniusPoreAnalysis::PoreTopologyResult SKApolloniusPoreAnalysis::analyze(
    const SKApolloniusDiagram::ApolloniusNetwork &network,
    const double3x3 &latticeVectors,
    double probeRadius,
    bool isPeriodic,
    const std::vector<std::pair<double3, double>> &atoms)
{
  std::size_t numVertices = network.vertices.size();
  double3x3 invLattice = isPeriodic ? latticeVectors.inverse() : double3x3::identity();

  double globalDi = 0.0;
  for (const auto &v : network.vertices)
  {
    if (v.clearanceRadius > 0.0)
    {
      globalDi = std::max(globalDi, 2.0 * v.clearanceRadius);
    }
  }

  std::vector<bool> vertexActive(numVertices, false);
  for (std::size_t i = 0; i < numVertices; ++i)
  {
    if (network.vertices[i].clearanceRadius >= probeRadius - 1e-7)
    {
      vertexActive[i] = true;
    }
  }

  std::vector<std::vector<DirectedAdj>> adj(numVertices);
  for (std::size_t eIdx = 0; eIdx < network.edges.size(); ++eIdx)
  {
    const auto &e = network.edges[eIdx];
    if (e.bottleneckRadius >= probeRadius - 1e-7 &&
        vertexActive[e.startVertex] &&
        vertexActive[e.endVertex])
    {
      adj[e.startVertex].push_back(DirectedAdj{
          e.endVertex,
          e.periodicShift,
          e.bottleneckRadius,
          eIdx
      });
      adj[e.endVertex].push_back(DirectedAdj{
          e.startVertex,
          -e.periodicShift,
          e.bottleneckRadius,
          eIdx
      });
    }
  }

  auto components = extractConnectedComponents(numVertices, vertexActive, adj);

  std::vector<ChannelComponent> channels;
  std::vector<InaccessiblePocket> pockets;
  std::vector<std::pair<double3, double>> allBlockingSpheres;
  int maxChannelDim = 0;

  for (const auto &comp : components)
  {
    double compMaxR = 0.0;
    for (std::size_t vIdx : comp.vertices)
    {
      compMaxR = std::max(compMaxR, network.vertices[vIdx].clearanceRadius);
    }

    if (comp.dimensionality >= 1)
    {
      maxChannelDim = std::max(maxChannelDim, comp.dimensionality);
      double minBottleneck = compMaxR;
      for (std::size_t u : comp.vertices)
      {
        for (const auto &edge : adj[u])
        {
          minBottleneck = std::min(minBottleneck, edge.bottleneckRadius);
        }
      }
      ChannelComponent ch;
      ch.vertexIndices = comp.vertices;
      ch.dimensionality = comp.dimensionality;
      ch.maxIncludedSphereDiameter = 2.0 * compMaxR;
      ch.limitingFreeSphereDiameter = 2.0 * minBottleneck;
      channels.push_back(std::move(ch));
    }
    else
    {
      double3 sumUnwrappedCart(0.0, 0.0, 0.0);
      std::vector<std::pair<double3, double>> rawPocketSpheres;
      std::unordered_set<std::size_t> compVertexSet(comp.vertices.begin(), comp.vertices.end());

      for (std::size_t vIdx : comp.vertices)
      {
        int3 shift = int3(0, 0, 0);
        auto it = comp.referenceShifts.find(vIdx);
        if (it != comp.referenceShifts.end())
        {
          shift = it->second;
        }
        double3 shiftCart = latticeVectors * double3(double(shift.x), double(shift.y), double(shift.z));
        double3 unwrappedPos = network.vertices[vIdx].cartesianPosition + shiftCart;
        sumUnwrappedCart += unwrappedPos;

        double rClear = std::max(0.0, network.vertices[vIdx].clearanceRadius);
        if (rClear > 1e-4)
        {
          rawPocketSpheres.push_back({unwrappedPos, rClear});
        }
      }

      for (const auto &e : network.edges)
      {
        if (compVertexSet.find(e.startVertex) != compVertexSet.end() ||
            compVertexSet.find(e.endVertex) != compVertexSet.end())
        {
          if (e.bottleneckRadius < probeRadius - 1e-7 &&
              !(compVertexSet.find(e.startVertex) != compVertexSet.end() &&
                compVertexSet.find(e.endVertex) != compVertexSet.end()))
          {
            continue;
          }
          int3 baseShift = int3(0, 0, 0);
          auto itS = comp.referenceShifts.find(e.startVertex);
          if (itS != comp.referenceShifts.end())
          {
            baseShift = itS->second;
          }
          else
          {
            auto itE = comp.referenceShifts.find(e.endVertex);
            if (itE != comp.referenceShifts.end())
            {
              baseShift = itE->second - e.periodicShift;
            }
          }
          double3 shiftCart = latticeVectors * double3(double(baseShift.x), double(baseShift.y), double(baseShift.z));
          std::size_t count = std::min(e.polylinePoints.size(), e.polylineClearances.size());
          for (std::size_t k = 0; k < count; ++k)
          {
            double cl = e.polylineClearances[k];
            if (cl > 1e-4)
            {
              rawPocketSpheres.push_back({e.polylinePoints[k] + shiftCart, cl});
            }
          }
        }
      }

      double countD = double(std::max<std::size_t>(1, comp.vertices.size()));
      double3 meanUnwrappedCart = sumUnwrappedCart / countD;
      double3 meanFrac = isPeriodic ? (invLattice * meanUnwrappedCart) : meanUnwrappedCart;
      if (isPeriodic)
      {
        meanFrac.x -= std::floor(meanFrac.x);
        meanFrac.y -= std::floor(meanFrac.y);
        meanFrac.z -= std::floor(meanFrac.z);
      }
      double3 wrappedCenterCart = isPeriodic ? (latticeVectors * meanFrac) : meanUnwrappedCart;

      double boundR = 0.0;
      for (const auto &sp : rawPocketSpheres)
      {
        double dist = (sp.first - meanUnwrappedCart).length() + sp.second;
        boundR = std::max(boundR, dist);
      }

      auto prunedSpheres = deduplicateAndPruneSpheres(rawPocketSpheres, latticeVectors, invLattice, isPeriodic);
      for (const auto &s : prunedSpheres)
      {
        allBlockingSpheres.push_back(s);
      }

      InaccessiblePocket pocket;
      pocket.vertexIndices = comp.vertices;
      pocket.fractionalCenter = meanFrac;
      pocket.cartesianCenter = wrappedCenterCart;
      pocket.maxIncludedSphereDiameter = 2.0 * compMaxR;
      pocket.boundingRadius = boundR;
      pocket.sphereUnion = std::move(prunedSpheres);
      pockets.push_back(std::move(pocket));
    }
  }

  auto [globalDf, dfA, dfB, dfC] = computeLimitingDiameters(network);
  double globalDif = computeIncludedAlongFreePath(network, globalDf);

  if (!atoms.empty())
  {
    bool needGridMetrics = (globalDf <= 1e-5);
    GridVoidAnalysis gridResult = computeGridVoidAnalysis(
        atoms,
        latticeVectors,
        probeRadius,
        isPeriodic,
        0.2,
        needGridMetrics);

    if (gridResult.gridDi > globalDi)
    {
      globalDi = gridResult.gridDi;
    }
    if (needGridMetrics && gridResult.gridDf > globalDf)
    {
      globalDf = gridResult.gridDf;
      globalDif = std::max(globalDif, gridResult.gridDif);
      dfA = gridResult.dfAlongA;
      dfB = gridResult.dfAlongB;
      dfC = gridResult.dfAlongC;
    }
    if (gridResult.channelDimensionality > maxChannelDim)
    {
      maxChannelDim = gridResult.channelDimensionality;
    }

    auto refinedSpheres = refineGridBlockingSpheres(
        atoms,
        latticeVectors,
        probeRadius,
        isPeriodic,
        0.2);

    pockets = std::move(gridResult.pockets);
    allBlockingSpheres = std::move(refinedSpheres);
  }

  PoreTopologyResult res;
  res.probeRadius = probeRadius;
  res.globalDi = globalDi;
  res.globalDf = globalDf;
  res.globalDif = globalDif;
  res.dfAlongA = dfA;
  res.dfAlongB = dfB;
  res.dfAlongC = dfC;
  res.channelDimensionality = maxChannelDim;
  res.channels = std::move(channels);
  res.inaccessiblePockets = std::move(pockets);
  res.blockingSpheres = std::move(allBlockingSpheres);
  return res;
}

std::vector<std::pair<double3, double>> SKApolloniusPoreAnalysis::computeBlockingSpheres(
    const std::vector<std::pair<double3, double>> &atoms,
    const double3x3 &latticeVectors,
    double probeRadius,
    bool isPeriodic,
    double gridSpacing)
{
  return refineGridBlockingSpheres(
      atoms,
      latticeVectors,
      probeRadius,
      isPeriodic,
      gridSpacing);
}
