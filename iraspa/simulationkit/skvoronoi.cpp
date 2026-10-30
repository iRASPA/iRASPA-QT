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

#include "skvoronoi.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <tuple>
#include <unordered_set>

namespace
{
constexpr double EPSILON = 1e-8;

struct Plane
{
  double3 normal;
  double d; // plane equation: dot(normal, x) <= d
  std::size_t neighborAtomIndex;
  int3 periodicShift;
  double distSq;
};

SKVoronoi::VoronoiCell createInitialBox(const double3 &center, double halfExtent, std::size_t selfIndex)
{
  SKVoronoi::VoronoiCell cell;
  double minX = center.x - halfExtent, maxX = center.x + halfExtent;
  double minY = center.y - halfExtent, maxY = center.y + halfExtent;
  double minZ = center.z - halfExtent, maxZ = center.z + halfExtent;

  cell.vertices = {
      {minX, minY, minZ}, // 0
      {maxX, minY, minZ}, // 1
      {maxX, maxY, minZ}, // 2
      {minX, maxY, minZ}, // 3
      {minX, minY, maxZ}, // 4
      {maxX, minY, maxZ}, // 5
      {maxX, maxY, maxZ}, // 6
      {minX, maxY, maxZ}  // 7
  };

  cell.faces = {
      {{0, 3, 2, 1}, selfIndex, {0, 0, -1}, {0, 0, -1}}, // -Z
      {{4, 5, 6, 7}, selfIndex, {0, 0, 1}, {0, 0, 1}},   // +Z
      {{0, 4, 7, 3}, selfIndex, {-1, 0, 0}, {-1, 0, 0}}, // -X
      {{1, 2, 6, 5}, selfIndex, {1, 0, 0}, {1, 0, 0}},   // +X
      {{0, 1, 5, 4}, selfIndex, {0, -1, 0}, {0, -1, 0}}, // -Y
      {{3, 7, 6, 2}, selfIndex, {0, 1, 0}, {0, 1, 0}}    // +Y
  };

  return cell;
}

void computeCellVolume(SKVoronoi::VoronoiCell &cell)
{
  if (cell.vertices.empty() || cell.faces.empty())
  {
    cell.volume = 0.0;
    return;
  }

  double3 center{0.0, 0.0, 0.0};
  for (const auto &v : cell.vertices)
  {
    center += v;
  }
  center = center / static_cast<double>(cell.vertices.size());

  double totalVolume = 0.0;
  for (const auto &face : cell.faces)
  {
    if (face.vertexIndices.size() < 3) continue;
    const double3 &v0 = cell.vertices[face.vertexIndices[0]];
    for (std::size_t i = 1; i + 1 < face.vertexIndices.size(); ++i)
    {
      const double3 &v1 = cell.vertices[face.vertexIndices[i]];
      const double3 &v2 = cell.vertices[face.vertexIndices[i + 1]];
      double3 a = v0 - center;
      double3 b = v1 - center;
      double3 c = v2 - center;
      totalVolume += std::abs(double3::dot(a, double3::cross(b, c))) / 6.0;
    }
  }
  cell.volume = totalVolume;
}

double maxVertexRadiusSq(const SKVoronoi::VoronoiCell &cell, const double3 &origin)
{
  double maxRSq = 0.0;
  for (const auto &v : cell.vertices)
  {
    double3 diff = v - origin;
    maxRSq = std::max(maxRSq, double3::dot(diff, diff));
  }
  return maxRSq;
}

void clipCellByPlane(SKVoronoi::VoronoiCell &cell, const Plane &plane)
{
  std::size_t numVerts = cell.vertices.size();
  if (numVerts == 0) return;

  std::vector<double> dists(numVerts);
  bool hasOutside = false;
  bool hasInside = false;

  for (std::size_t i = 0; i < numVerts; ++i)
  {
    dists[i] = double3::dot(plane.normal, cell.vertices[i]) - plane.d;
    if (dists[i] > EPSILON) hasOutside = true;
    if (dists[i] < -EPSILON) hasInside = true;
  }

  if (!hasOutside) return;
  if (!hasInside)
  {
    cell.vertices.clear();
    cell.faces.clear();
    cell.volume = 0.0;
    return;
  }

  std::map<std::pair<std::size_t, std::size_t>, std::size_t> edgeIntersectionMap;
  auto getIntersectionVertex = [&](std::size_t v1, std::size_t v2) -> std::size_t
  {
    auto key = std::minmax(v1, v2);
    auto it = edgeIntersectionMap.find(key);
    if (it != edgeIntersectionMap.end())
    {
      return it->second;
    }

    double d1 = dists[v1];
    double d2 = dists[v2];
    double t = d1 / (d1 - d2);
    t = std::clamp(t, 0.0, 1.0);
    double3 p = cell.vertices[v1] + t * (cell.vertices[v2] - cell.vertices[v1]);

    for (std::size_t i = 0; i < numVerts; ++i)
    {
      if (std::abs(dists[i]) <= EPSILON)
      {
        double3 diff = cell.vertices[i] - p;
        if (double3::dot(diff, diff) < EPSILON * EPSILON)
        {
          edgeIntersectionMap[key] = i;
          return i;
        }
      }
    }

    std::size_t newIdx = cell.vertices.size();
    cell.vertices.push_back(p);
    dists.push_back(0.0);
    edgeIntersectionMap[key] = newIdx;
    return newIdx;
  };

  std::vector<SKVoronoi::VoronoiFace> newFaces;
  std::unordered_set<std::size_t> capVerticesSet;

  for (const auto &face : cell.faces)
  {
    std::vector<std::size_t> clippedIndices;
    std::size_t n = face.vertexIndices.size();
    for (std::size_t i = 0; i < n; ++i)
    {
      std::size_t curr = face.vertexIndices[i];
      std::size_t next = face.vertexIndices[(i + 1) % n];
      double dCurr = dists[curr];
      double dNext = dists[next];

      bool currIn = (dCurr <= EPSILON);
      bool nextIn = (dNext <= EPSILON);

      if (currIn && nextIn)
      {
        clippedIndices.push_back(next);
        if (std::abs(dNext) <= EPSILON)
        {
          capVerticesSet.insert(next);
        }
      }
      else if (currIn && !nextIn)
      {
        if (std::abs(dCurr) > EPSILON)
        {
          std::size_t inter = getIntersectionVertex(curr, next);
          if (clippedIndices.empty() || clippedIndices.back() != inter)
          {
            clippedIndices.push_back(inter);
          }
          capVerticesSet.insert(inter);
        }
        else
        {
          capVerticesSet.insert(curr);
        }
      }
      else if (!currIn && nextIn)
      {
        if (std::abs(dNext) > EPSILON)
        {
          std::size_t inter = getIntersectionVertex(curr, next);
          clippedIndices.push_back(inter);
          capVerticesSet.insert(inter);
        }
        else
        {
          capVerticesSet.insert(next);
        }
        clippedIndices.push_back(next);
      }
    }

    std::vector<std::size_t> uniqueIndices;
    for (std::size_t idx : clippedIndices)
    {
      if (uniqueIndices.empty() || uniqueIndices.back() != idx)
      {
        uniqueIndices.push_back(idx);
      }
    }
    if (uniqueIndices.size() > 1 && uniqueIndices.front() == uniqueIndices.back())
    {
      uniqueIndices.pop_back();
    }

    if (uniqueIndices.size() >= 3)
    {
      SKVoronoi::VoronoiFace nf = face;
      nf.vertexIndices = std::move(uniqueIndices);
      newFaces.push_back(std::move(nf));
    }
  }

  if (capVerticesSet.size() >= 3)
  {
    std::vector<std::size_t> capVerts(capVerticesSet.begin(), capVerticesSet.end());
    double3 centroid{0.0, 0.0, 0.0};
    for (std::size_t idx : capVerts)
    {
      centroid += cell.vertices[idx];
    }
    centroid = centroid / static_cast<double>(capVerts.size());

    double3 n = plane.normal;
    double3 ref = (std::abs(n.x) < 0.9) ? double3{1.0, 0.0, 0.0} : double3{0.0, 1.0, 0.0};
    double3 u = double3::normalize(double3::cross(n, ref));
    double3 v = double3::cross(n, u);

    std::sort(capVerts.begin(), capVerts.end(), [&](std::size_t a, std::size_t b) {
      double3 da = cell.vertices[a] - centroid;
      double3 db = cell.vertices[b] - centroid;
      double angleA = std::atan2(double3::dot(da, v), double3::dot(da, u));
      double angleB = std::atan2(double3::dot(db, v), double3::dot(db, u));
      return angleA < angleB;
    });

    std::vector<std::size_t> filteredCapVerts;
    for (std::size_t idx : capVerts)
    {
      if (filteredCapVerts.empty())
      {
        filteredCapVerts.push_back(idx);
      }
      else
      {
        double3 diff = cell.vertices[idx] - cell.vertices[filteredCapVerts.back()];
        if (double3::dot(diff, diff) > EPSILON * EPSILON)
        {
          filteredCapVerts.push_back(idx);
        }
      }
    }
    if (filteredCapVerts.size() > 1)
    {
      double3 diff = cell.vertices[filteredCapVerts.front()] - cell.vertices[filteredCapVerts.back()];
      if (double3::dot(diff, diff) <= EPSILON * EPSILON)
      {
        filteredCapVerts.pop_back();
      }
    }

    if (filteredCapVerts.size() >= 3)
    {
      SKVoronoi::VoronoiFace capFace;
      capFace.vertexIndices = std::move(filteredCapVerts);
      capFace.neighborAtomIndex = plane.neighborAtomIndex;
      capFace.periodicShift = plane.periodicShift;
      capFace.normal = plane.normal;
      newFaces.push_back(std::move(capFace));
    }
  }

  std::vector<std::size_t> oldToNew(cell.vertices.size(), std::numeric_limits<std::size_t>::max());
  std::vector<double3> compactVertices;
  for (auto &face : newFaces)
  {
    for (auto &idx : face.vertexIndices)
    {
      if (oldToNew[idx] == std::numeric_limits<std::size_t>::max())
      {
        oldToNew[idx] = compactVertices.size();
        compactVertices.push_back(cell.vertices[idx]);
      }
      idx = oldToNew[idx];
    }
  }

  cell.vertices = std::move(compactVertices);
  cell.faces = std::move(newFaces);
}

} // namespace

SKVoronoi::VoronoiCell SKVoronoi::computeCell(std::size_t atomIndex,
                                              const std::vector<std::pair<double3, double>> &atoms,
                                              const double3x3 &latticeVectors,
                                              bool isPeriodic,
                                              VoronoiType type,
                                              double boundingBoxPadding)
{
  if (atomIndex >= atoms.size())
  {
    return {};
  }

  const double3 &p_i = atoms[atomIndex].first;
  double r_i = (type == VoronoiType::Radical) ? atoms[atomIndex].second : 0.0;

  double maxBoxExtent = boundingBoxPadding;
  if (isPeriodic)
  {
    double lenA = latticeVectors[0].length();
    double lenB = latticeVectors[1].length();
    double lenC = latticeVectors[2].length();
    maxBoxExtent = std::max({lenA, lenB, lenC}) * 2.5;
  }
  else
  {
    for (const auto &atom : atoms)
    {
      double dist = (atom.first - p_i).length() + boundingBoxPadding;
      maxBoxExtent = std::max(maxBoxExtent, dist);
    }
  }

  VoronoiCell cell = createInitialBox(p_i, maxBoxExtent, atomIndex);

  std::vector<Plane> candidatePlanes;
  int maxShell = isPeriodic ? 2 : 0;

  for (int nx = -maxShell; nx <= maxShell; ++nx)
  {
    for (int ny = -maxShell; ny <= maxShell; ++ny)
    {
      for (int nz = -maxShell; nz <= maxShell; ++nz)
      {
        int3 shift{nx, ny, nz};
        double3 shiftVec = latticeVectors * double3(nx, ny, nz);

        for (std::size_t j = 0; j < atoms.size(); ++j)
        {
          if (j == atomIndex && nx == 0 && ny == 0 && nz == 0) continue;

          double3 p_j = atoms[j].first + shiftVec;
          double r_j = (type == VoronoiType::Radical) ? atoms[j].second : 0.0;

          double3 diff = p_j - p_i;
          double distSq = double3::dot(diff, diff);
          if (distSq < EPSILON * EPSILON) continue;

          double dist = std::sqrt(distSq);
          double3 normal = diff / dist;

          double alpha = 0.5 * (1.0 + (r_i * r_i - r_j * r_j) / distSq);
          double3 planePoint = p_i + alpha * diff;
          double d = double3::dot(normal, planePoint);

          candidatePlanes.push_back(Plane{normal, d, j, shift, distSq});
        }
      }
    }
  }

  std::sort(candidatePlanes.begin(), candidatePlanes.end(),
            [](const Plane &a, const Plane &b) { return a.distSq < b.distSq; });

  for (const auto &plane : candidatePlanes)
  {
    double distFromOriginToPlane = double3::dot(plane.normal, p_i) - plane.d;
    if (distFromOriginToPlane < 0.0)
    {
      double planeDistSq = distFromOriginToPlane * distFromOriginToPlane;
      if (planeDistSq > maxVertexRadiusSq(cell, p_i) + EPSILON)
      {
        continue;
      }
    }

    clipCellByPlane(cell, plane);
    if (cell.vertices.empty()) break;
  }

  computeCellVolume(cell);
  return cell;
}

std::vector<SKVoronoi::VoronoiCell> SKVoronoi::computeAllCells(const std::vector<std::pair<double3, double>> &atoms,
                                                               const double3x3 &latticeVectors,
                                                               bool isPeriodic,
                                                               VoronoiType type,
                                                               double boundingBoxPadding)
{
  std::vector<VoronoiCell> cells;
  cells.reserve(atoms.size());
  for (std::size_t i = 0; i < atoms.size(); ++i)
  {
    cells.push_back(computeCell(i, atoms, latticeVectors, isPeriodic, type, boundingBoxPadding));
  }
  return cells;
}

SKVoronoi::VoronoiNetwork SKVoronoi::computeNetwork(const std::vector<std::pair<double3, double>> &atoms,
                                                    const double3x3 &latticeVectors,
                                                    bool isPeriodic,
                                                    VoronoiType type,
                                                    double boundingBoxPadding)
{
  VoronoiNetwork net;
  net.cells = computeAllCells(atoms, latticeVectors, isPeriodic, type, boundingBoxPadding);

  if (atoms.empty()) return net;

  double3x3 invLattice = isPeriodic ? latticeVectors.inverse() : double3x3::identity();

  auto calcMaxFreeSphereRadius = [&](const double3 &pos) -> double
  {
    double minRadius = std::numeric_limits<double>::max();
    int maxShell = isPeriodic ? 1 : 0;
    for (int nx = -maxShell; nx <= maxShell; ++nx)
    {
      for (int ny = -maxShell; ny <= maxShell; ++ny)
      {
        for (int nz = -maxShell; nz <= maxShell; ++nz)
        {
          double3 shiftVec = latticeVectors * double3(nx, ny, nz);
          for (const auto &atom : atoms)
          {
            double dist = (pos - (atom.first + shiftVec)).length() - atom.second;
            minRadius = std::min(minRadius, dist);
          }
        }
      }
    }
    return minRadius;
  };

  constexpr double NODE_MERGE_TOL = 1e-4;
  auto findOrAddNode = [&](const double3 &cartPos) -> std::pair<std::size_t, int3>
  {
    double3 wrappedCart = cartPos;
    double3 frac = invLattice * cartPos;
    int3 shift{0, 0, 0};

    if (isPeriodic)
    {
      shift = int3(static_cast<int>(std::floor(frac.x + 1e-6)),
                   static_cast<int>(std::floor(frac.y + 1e-6)),
                   static_cast<int>(std::floor(frac.z + 1e-6)));
      frac = frac - double3(shift.x, shift.y, shift.z);
      wrappedCart = latticeVectors * frac;
    }

    for (std::size_t idx = 0; idx < net.nodes.size(); ++idx)
    {
      if (isPeriodic)
      {
        double3 df = net.nodes[idx].fractionalPosition - frac;
        df.x -= std::rint(df.x);
        df.y -= std::rint(df.y);
        df.z -= std::rint(df.z);
        double3 dc = latticeVectors * df;
        if (double3::dot(dc, dc) < NODE_MERGE_TOL * NODE_MERGE_TOL)
        {
          return {idx, shift};
        }
      }
      else
      {
        double3 dc = net.nodes[idx].cartesianPosition - wrappedCart;
        if (double3::dot(dc, dc) < NODE_MERGE_TOL * NODE_MERGE_TOL)
        {
          return {idx, shift};
        }
      }
    }

    VoronoiNode node;
    node.cartesianPosition = wrappedCart;
    node.fractionalPosition = frac;
    node.maxFreeSphereRadius = calcMaxFreeSphereRadius(wrappedCart);
    std::size_t newIdx = net.nodes.size();
    net.nodes.push_back(node);
    return {newIdx, shift};
  };

  std::set<std::tuple<std::size_t, std::size_t, int, int, int>> uniqueEdges;

  for (const auto &cell : net.cells)
  {
    std::vector<std::pair<std::size_t, int3>> cellVertToNode(cell.vertices.size());
    for (std::size_t v = 0; v < cell.vertices.size(); ++v)
    {
      cellVertToNode[v] = findOrAddNode(cell.vertices[v]);
    }

    for (const auto &face : cell.faces)
    {
      std::size_t n = face.vertexIndices.size();
      for (std::size_t i = 0; i < n; ++i)
      {
        std::size_t v1 = face.vertexIndices[i];
        std::size_t v2 = face.vertexIndices[(i + 1) % n];

        auto [n1, s1] = cellVertToNode[v1];
        auto [n2, s2] = cellVertToNode[v2];
        int3 edgeShift = s2 - s1;

        if (n1 == n2 && edgeShift == int3(0, 0, 0)) continue;

        std::size_t u = n1, w = n2;
        int3 canonicalShift = edgeShift;
        if (u > w || (u == w && std::tie(canonicalShift.x, canonicalShift.y, canonicalShift.z) < std::make_tuple(0, 0, 0)))
        {
          std::swap(u, w);
          canonicalShift = -canonicalShift;
        }

        auto edgeKey = std::make_tuple(u, w, canonicalShift.x, canonicalShift.y, canonicalShift.z);
        if (uniqueEdges.find(edgeKey) == uniqueEdges.end())
        {
          uniqueEdges.insert(edgeKey);
          double3 midPoint = 0.5 * (cell.vertices[v1] + cell.vertices[v2]);
          double edgeRadius = std::min({net.nodes[u].maxFreeSphereRadius,
                                        net.nodes[w].maxFreeSphereRadius,
                                        calcMaxFreeSphereRadius(midPoint)});
          net.edges.push_back(VoronoiEdge{u, w, canonicalShift, edgeRadius});
        }
      }
    }
  }

  return net;
}
