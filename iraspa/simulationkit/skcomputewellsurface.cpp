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

#include "skcomputewellsurface.h"
#include "skcomputeisosurface.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <unordered_map>

namespace
{
constexpr double overlapEnergy = 10000000.0;
constexpr double blockedEnergyPerAngstrom = 1000.0;
constexpr double twoToOneSixth = 1.122462048309373;

double smoothstep(double edge0, double edge1, double x)
{
  double t = std::clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  return t * t * (3.0 - 2.0 * t);
}

double blockingPocketDistance(const double3 &point,
                              const std::vector<double4> &blockingPockets,
                              const double3x3 &replicaCell,
                              const double3 &replicaCorrection)
{
  double nearest = 1.0e10;
  for (const auto &pocket : blockingPockets)
  {
    double3 pocketReplicaFrac(pocket.x * replicaCorrection.x,
                              pocket.y * replicaCorrection.y,
                              pocket.z * replicaCorrection.z);
    double3 df = point - pocketReplicaFrac;
    df.x -= std::rint(df.x);
    df.y -= std::rint(df.y);
    df.z -= std::rint(df.z);
    double3 dr = replicaCell * df;
    nearest = std::min(nearest, dr.length() - pocket.w);
  }
  return nearest;
}

double wellPotentialEnergy(const double3 &point,
                           const std::vector<double3> &atomPositions,
                           const std::vector<double2> &potParameters,
                           const double3x3 &replicaCell,
                           const std::vector<double3> &replicas)
{
  double value = 0.0;
  std::size_t numberOfAtoms = atomPositions.size();
  for (const auto &replica : replicas)
  {
    for (std::size_t iatom = 0; iatom < numberOfAtoms; ++iatom)
    {
      double3 dr = (point - atomPositions[iatom]) - replica;
      dr.x -= std::rint(dr.x);
      dr.y -= std::rint(dr.y);
      dr.z -= std::rint(dr.z);
      dr = replicaCell * dr;
      double rr = std::max(double3::dot(dr, dr), 1.0e-8);
      if (rr < 12.0 * 12.0)
      {
        double sigma = potParameters[iatom].y;
        double temp = sigma * sigma / rr;
        double rri3 = temp * temp * temp;
        value += potParameters[iatom].x * (rri3 * (rri3 - 1.0));
      }
    }
  }
  return value;
}

std::vector<bool> dilateBinary(const std::vector<bool> &mask, int nx, int ny, int nz)
{
  std::vector<bool> out = mask;
  int nxy = nx * ny;
  for (int z = 0; z < nz; ++z)
  {
    int zm = (z == 0) ? nz - 1 : z - 1;
    int zp = (z + 1 == nz) ? 0 : z + 1;
    int zz[3] = {zm, z, zp};
    for (int y = 0; y < ny; ++y)
    {
      int ym = (y == 0) ? ny - 1 : y - 1;
      int yp = (y + 1 == ny) ? 0 : y + 1;
      int yz[3] = {ym, y, yp};
      for (int x = 0; x < nx; ++x)
      {
        int i = x + nx * y + nxy * z;
        if (mask[std::size_t(i)]) continue;
        int xm = (x == 0) ? nx - 1 : x - 1;
        int xp = (x + 1 == nx) ? 0 : x + 1;
        bool found = false;
        for (int zb : zz)
        {
          for (int yb : yz)
          {
            int row = nx * yb + nxy * zb;
            if (mask[std::size_t(xm + row)] || mask[std::size_t(x + row)] || mask[std::size_t(xp + row)])
            {
              found = true;
              break;
            }
          }
          if (found) break;
        }
        if (found) out[std::size_t(i)] = true;
      }
    }
  }
  return out;
}

std::vector<bool> erodeBinary(const std::vector<bool> &mask, int nx, int ny, int nz)
{
  std::vector<bool> out = mask;
  int nxy = nx * ny;
  for (int z = 0; z < nz; ++z)
  {
    int zm = (z == 0) ? nz - 1 : z - 1;
    int zp = (z + 1 == nz) ? 0 : z + 1;
    int zz[3] = {zm, z, zp};
    for (int y = 0; y < ny; ++y)
    {
      int ym = (y == 0) ? ny - 1 : y - 1;
      int yp = (y + 1 == ny) ? 0 : y + 1;
      int yz[3] = {ym, y, yp};
      for (int x = 0; x < nx; ++x)
      {
        int i = x + nx * y + nxy * z;
        if (!mask[std::size_t(i)]) continue;
        int xm = (x == 0) ? nx - 1 : x - 1;
        int xp = (x + 1 == nx) ? 0 : x + 1;
        bool keep = true;
        for (int zb : zz)
        {
          for (int yb : yz)
          {
            int row = nx * yb + nxy * zb;
            if (!mask[std::size_t(xm + row)] || !mask[std::size_t(x + row)] || !mask[std::size_t(xp + row)])
            {
              keep = false;
              break;
            }
          }
          if (!keep) break;
        }
        if (!keep) out[std::size_t(i)] = false;
      }
    }
  }
  return out;
}

void fillEnclosedCavities(std::vector<bool> &closed, const std::vector<bool> &candidate, int nx, int ny, int nz)
{
  int n = nx * ny * nz;
  int nxy = nx * ny;
  std::vector<bool> exterior(std::size_t(n), false);
  std::vector<int> stack;
  stack.reserve(std::size_t(n / 8));
  for (int i = 0; i < n; ++i)
  {
    if (!closed[std::size_t(i)] && !candidate[std::size_t(i)])
    {
      exterior[std::size_t(i)] = true;
      stack.push_back(i);
    }
  }
  while (!stack.empty())
  {
    int i = stack.back();
    stack.pop_back();
    int x = i % nx;
    int y = (i / nx) % ny;
    int z = i / nxy;
    int xm = (x == 0) ? nx - 1 : x - 1;
    int xp = (x + 1 == nx) ? 0 : x + 1;
    int ym = (y == 0) ? ny - 1 : y - 1;
    int yp = (y + 1 == ny) ? 0 : y + 1;
    int zm = (z == 0) ? nz - 1 : z - 1;
    int zp = (z + 1 == nz) ? 0 : z + 1;
    int neighbors[6] = {
        xm + nx * y + nxy * z, xp + nx * y + nxy * z,
        x + nx * ym + nxy * z, x + nx * yp + nxy * z,
        x + nx * y + nxy * zm, x + nx * y + nxy * zp
    };
    for (int nb : neighbors)
    {
      if (!closed[std::size_t(nb)] && !exterior[std::size_t(nb)])
      {
        exterior[std::size_t(nb)] = true;
        stack.push_back(nb);
      }
    }
  }
  for (int i = 0; i < n; ++i)
  {
    if (!closed[std::size_t(i)] && !exterior[std::size_t(i)])
    {
      closed[std::size_t(i)] = true;
    }
  }
}

std::vector<float4> removeFilamentSpecks(const std::vector<float4> &vertices, const double3x3 &unitCell)
{
  int triangles = int(vertices.size() / 9);
  if (triangles <= 0) return {};

  std::vector<int> parent(static_cast<std::size_t>(triangles));
  for (int i = 0; i < triangles; ++i) parent[std::size_t(i)] = i;

  auto findRoot = [&](int i) -> int
  {
    int r = i;
    while (parent[std::size_t(r)] != r) r = parent[std::size_t(r)];
    int w = i;
    while (parent[std::size_t(w)] != r)
    {
      int nx = parent[std::size_t(w)];
      parent[std::size_t(w)] = r;
      w = nx;
    }
    return r;
  };

  auto quantize = [](float x) -> int32_t
  {
    int32_t r = int32_t(std::rint(x * 1048576.0f));
    return r == 1048576 ? 0 : r;
  };

  std::map<std::tuple<int32_t, int32_t, int32_t>, int> seen;
  for (int t = 0; t < triangles; ++t)
  {
    for (int v = 0; v < 3; ++v)
    {
      const float4 &p = vertices[std::size_t(9 * t + 3 * v)];
      auto key = std::make_tuple(quantize(p.x), quantize(p.y), quantize(p.z));
      auto it = seen.find(key);
      if (it != seen.end())
      {
        int a = findRoot(t);
        int b = findRoot(it->second);
        if (a != b) parent[std::size_t(std::max(a, b))] = std::min(a, b);
      }
      else
      {
        seen[key] = t;
      }
    }
  }

  std::unordered_map<int, double> area;
  for (int t = 0; t < triangles; ++t)
  {
    double3 corners[3];
    for (int v = 0; v < 3; ++v)
    {
      const float4 &p = vertices[std::size_t(9 * t + 3 * v)];
      corners[v] = unitCell * double3(double(p.x), double(p.y), double(p.z));
    }
    double triArea = 0.5 * double3::cross(corners[1] - corners[0], corners[2] - corners[0]).length();
    area[findRoot(t)] += triArea;
  }

  std::vector<float4> kept;
  kept.reserve(vertices.size());
  for (int t = 0; t < triangles; ++t)
  {
    if (area[findRoot(t)] >= SKComputeWellSurface::filamentMinimumArea)
    {
      for (int k = 0; k < 9; ++k)
      {
        kept.push_back(vertices[std::size_t(9 * t + k)]);
      }
    }
  }
  return kept;
}
} // namespace

std::vector<float> SKComputeWellSurface::computeWellFieldGrid(int3 size,
                                                              double2 probeParameter,
                                                              const std::vector<double3> &positions,
                                                              const std::vector<double2> &potentialParameters,
                                                              double3x3 unitCell,
                                                              int3 numberOfReplicas,
                                                              const std::vector<double4> &blockingPockets) noexcept
{
  int numberOfGridPoints = size.x * size.y * size.z;
  if (positions.empty() || numberOfGridPoints <= 0)
  {
    return {};
  }

  std::size_t numberOfAtoms = positions.size();
  double3 correction(1.0 / double(numberOfReplicas.x),
                     1.0 / double(numberOfReplicas.y),
                     1.0 / double(numberOfReplicas.z));

  double3x3 replicaCell(double(numberOfReplicas.x) * unitCell[0],
                        double(numberOfReplicas.y) * unitCell[1],
                        double(numberOfReplicas.z) * unitCell[2]);

  std::vector<double3> atomPos(numberOfAtoms);
  std::vector<double2> params(numberOfAtoms);
  for (std::size_t i = 0; i < numberOfAtoms; ++i)
  {
    atomPos[i] = correction * positions[i];
    params[i] = double2(4.0 * std::sqrt(potentialParameters[i].x * probeParameter.x),
                        0.5 * (potentialParameters[i].y + probeParameter.y));
  }

  int totalNumberOfReplicas = numberOfReplicas.x * numberOfReplicas.y * numberOfReplicas.z;
  std::vector<double3> replicaVector(static_cast<std::size_t>(totalNumberOfReplicas));
  int index = 0;
  for (int i = 0; i < numberOfReplicas.x; ++i)
  {
    for (int j = 0; j < numberOfReplicas.y; ++j)
    {
      for (int k = 0; k < numberOfReplicas.z; ++k)
      {
        replicaVector[std::size_t(index)] = double3(double(i) / double(numberOfReplicas.x),
                                                    double(j) / double(numberOfReplicas.y),
                                                    double(k) / double(numberOfReplicas.z));
        ++index;
      }
    }
  }

  std::vector<float> output(std::size_t(3 * numberOfGridPoints), 0.0f);
  constexpr double tau = 0.4;

  for (int z = 0; z < size.z; ++z)
  {
    for (int y = 0; y < size.y; ++y)
    {
      for (int x = 0; x < size.x; ++x)
      {
        int igrid = x + y * size.x + z * size.x * size.y;
        double3 gridpos = correction * double3(double(x) / double(size.x),
                                               double(y) / double(size.y),
                                               double(z) / double(size.z));

        double pocket = blockingPocketDistance(gridpos, blockingPockets, replicaCell, correction);
        double value = 0.0;
        double distance = 1.0e10;
        double3 directionSum(0.0, 0.0, 0.0);
        double weightSum = 0.0;

        for (int j = 0; j < totalNumberOfReplicas; ++j)
        {
          const double3 &replica = replicaVector[std::size_t(j)];
          for (std::size_t iatom = 0; iatom < numberOfAtoms; ++iatom)
          {
            double3 dr = (gridpos - atomPos[iatom]) - replica;
            dr.x -= std::rint(dr.x);
            dr.y -= std::rint(dr.y);
            dr.z -= std::rint(dr.z);
            dr = replicaCell * dr;

            double rr = std::max(double3::dot(dr, dr), 1.0e-8);
            if (rr < 12.0 * 12.0)
            {
              double eps = params[iatom].x;
              double sigma = params[iatom].y;
              double temp = sigma * sigma / rr;
              double rri3 = temp * temp * temp;
              value += eps * (rri3 * (rri3 - 1.0));

              double r = std::sqrt(rr);
              double contact = r - twoToOneSixth * sigma;
              distance = std::min(distance, contact);
              double w = std::exp(-contact / tau);
              directionSum += w * (-dr / r);
              weightSum += w;
            }
          }
        }

        double energy = (pocket < 0.0) ? std::min(-pocket * blockedEnergyPerAngstrom, overlapEnergy)
                                       : std::min(value, overlapEnergy);
        double finalDist = std::min(distance, pocket);
        double rel = (weightSum > 0.0) ? (directionSum.length() / weightSum) : 1.0;

        output[std::size_t(3 * igrid + 0)] = float(energy);
        output[std::size_t(3 * igrid + 1)] = float(finalDist);
        output[std::size_t(3 * igrid + 2)] = float(rel);
      }
    }
  }

  return output;
}

float SKComputeWellSurface::effectiveTrimIsovalue(const std::vector<float> &field,
                                                  double isovalue,
                                                  int3 dimensions) noexcept
{
  int numberOfGridPoints = dimensions.x * dimensions.y * dimensions.z;
  float minimumEnergy = std::numeric_limits<float>::max();
  for (int i = 0; i < numberOfGridPoints; ++i)
  {
    minimumEnergy = std::min(minimumEnergy, field[std::size_t(3 * i)]);
  }
  float iso = float(isovalue);
  if (!(iso > minimumEnergy))
  {
    iso = 0.25f * minimumEnergy;
  }
  return iso;
}

std::vector<float4> SKComputeWellSurface::constructWellSurfaceVertexBuffer(const std::vector<float> &field,
                                                                           double isovalue,
                                                                           int3 dimensions)
{
  int numberOfGridPoints = dimensions.x * dimensions.y * dimensions.z;
  if (numberOfGridPoints <= 0 || field.size() < std::size_t(3 * numberOfGridPoints))
  {
    return {};
  }

  float iso = effectiveTrimIsovalue(field, isovalue, dimensions);

  std::vector<float> combined(std::size_t(numberOfGridPoints), 0.0f);
  float minC = std::numeric_limits<float>::max();
  float maxC = std::numeric_limits<float>::lowest();
  for (int i = 0; i < numberOfGridPoints; ++i)
  {
    float val = std::max(-field[std::size_t(3 * i + 1)], energyScale * (field[std::size_t(3 * i)] - iso));
    combined[std::size_t(i)] = val;
    minC = std::min(minC, val);
    maxC = std::max(maxC, val);
  }

  if (!(minC < 0.0f && maxC > 0.0f))
  {
    return {};
  }

  return SKComputeIsosurface::computeIsosurface(dimensions, &combined, 0.0);
}

void SKComputeWellSurface::refineWellSurfaceVertices(std::vector<float4> &vertexBuffer,
                                                     double2 probeParameter,
                                                     const std::vector<double3> &positions,
                                                     const std::vector<double2> &potentialParameters,
                                                     double3x3 unitCell,
                                                     int3 numberOfReplicas,
                                                     float isovalue,
                                                     const std::vector<double4> &blockingPockets) noexcept
{
  std::size_t numberOfVertices = vertexBuffer.size() / 3;
  std::size_t numberOfAtoms = positions.size();
  if (numberOfAtoms == 0 || numberOfVertices == 0)
  {
    return;
  }

  double3 correction(1.0 / double(numberOfReplicas.x),
                     1.0 / double(numberOfReplicas.y),
                     1.0 / double(numberOfReplicas.z));

  double3x3 replicaCell(double(numberOfReplicas.x) * unitCell[0],
                        double(numberOfReplicas.y) * unitCell[1],
                        double(numberOfReplicas.z) * unitCell[2]);
  double3x3 inverseCell = replicaCell.inverse();

  std::vector<double3> atomPos(numberOfAtoms);
  std::vector<double2> params(numberOfAtoms);
  for (std::size_t i = 0; i < numberOfAtoms; ++i)
  {
    atomPos[i] = correction * positions[i];
    params[i] = double2(4.0 * std::sqrt(potentialParameters[i].x * probeParameter.x),
                        0.5 * (potentialParameters[i].y + probeParameter.y));
  }

  int totalNumberOfReplicas = numberOfReplicas.x * numberOfReplicas.y * numberOfReplicas.z;
  std::vector<double3> replicas(static_cast<std::size_t>(totalNumberOfReplicas));
  int index = 0;
  for (int i = 0; i < numberOfReplicas.x; ++i)
  {
    for (int j = 0; j < numberOfReplicas.y; ++j)
    {
      for (int k = 0; k < numberOfReplicas.z; ++k)
      {
        replicas[std::size_t(index)] = double3(double(i) / double(numberOfReplicas.x),
                                               double(j) / double(numberOfReplicas.y),
                                               double(k) / double(numberOfReplicas.z));
        ++index;
      }
    }
  }

  constexpr double tau = 0.4;
  constexpr double invphi = 0.6180339887498949;
  constexpr int coarse = 14;

  for (std::size_t vertexId = 0; vertexId < numberOfVertices; ++vertexId)
  {
    float4 &vPos = vertexBuffer[3 * vertexId];
    double3 quantized(std::rint(double(vPos.x) * 1048576.0) / 1048576.0,
                      std::rint(double(vPos.y) * 1048576.0) / 1048576.0,
                      std::rint(double(vPos.z) * 1048576.0) / 1048576.0);
    double3 point = quantized * correction;

    double energyHere = wellPotentialEnergy(point, atomPos, params, replicaCell, replicas);
    if (energyHere > double(isovalue) - 0.02 * std::abs(double(isovalue)))
    {
      continue;
    }

    double nearest = 1.0e10;
    for (const auto &replica : replicas)
    {
      for (std::size_t iatom = 0; iatom < numberOfAtoms; ++iatom)
      {
        double3 dr = (point - atomPos[iatom]) - replica;
        dr.x -= std::rint(dr.x);
        dr.y -= std::rint(dr.y);
        dr.z -= std::rint(dr.z);
        dr = replicaCell * dr;
        nearest = std::min(nearest, dr.length() - twoToOneSixth * params[iatom].y);
      }
    }

    if (blockingPocketDistance(point, blockingPockets, replicaCell, correction) < nearest)
    {
      continue;
    }

    double3 directionSum(0.0, 0.0, 0.0);
    double weightSum = 0.0;
    for (const auto &replica : replicas)
    {
      for (std::size_t iatom = 0; iatom < numberOfAtoms; ++iatom)
      {
        double3 dr = (point - atomPos[iatom]) - replica;
        dr.x -= std::rint(dr.x);
        dr.y -= std::rint(dr.y);
        dr.z -= std::rint(dr.z);
        dr = replicaCell * dr;
        double r = dr.length();
        if (!(r > 1.0e-6)) continue;
        double weighted = r - twoToOneSixth * params[iatom].y;
        if (weighted - nearest > 6.0 * tau) continue;
        double w = std::exp(-(weighted - nearest) / tau);
        directionSum += w * (-dr / r);
        weightSum += w;
      }
    }
    if (!(weightSum > 0.0)) continue;
    double dirLen = directionSum.length();
    if (!(dirLen > 1.0e-12)) continue;
    double reliability = dirLen / weightSum;
    double span = 0.7 * smoothstep(0.25, 0.6, reliability);
    if (span < 0.05) continue;

    double3 rayFractional = inverseCell * (directionSum / dirLen);

    double sBest = 0.0;
    double uBest = energyHere;
    for (int i = -coarse; i <= coarse; ++i)
    {
      double s = span * double(i) / double(coarse);
      double u = wellPotentialEnergy(point + s * rayFractional, atomPos, params, replicaCell, replicas);
      if (u < uBest)
      {
        uBest = u;
        sBest = s;
      }
    }
    if (std::abs(sBest) >= span - 0.5 * span / double(coarse))
    {
      continue;
    }

    double a = sBest - span / double(coarse);
    double b = sBest + span / double(coarse);
    double x1 = b - invphi * (b - a);
    double x2 = a + invphi * (b - a);
    double f1 = wellPotentialEnergy(point + x1 * rayFractional, atomPos, params, replicaCell, replicas);
    double f2 = wellPotentialEnergy(point + x2 * rayFractional, atomPos, params, replicaCell, replicas);
    for (int iteration = 0; iteration < 20; ++iteration)
    {
      if (f1 < f2)
      {
        b = x2;
        x2 = x1;
        f2 = f1;
        x1 = b - invphi * (b - a);
        f1 = wellPotentialEnergy(point + x1 * rayFractional, atomPos, params, replicaCell, replicas);
      }
      else
      {
        a = x1;
        x1 = x2;
        f1 = f2;
        x2 = a + invphi * (b - a);
        f2 = wellPotentialEnergy(point + x2 * rayFractional, atomPos, params, replicaCell, replicas);
      }
    }
    double s = 0.5 * (a + b);
    double3 refined = (point + s * rayFractional) / correction;
    vPos = float4(float(refined.x), float(refined.y), float(refined.z), 1.0f);
  }
}

std::vector<float4> SKComputeWellSurface::constructWellFilamentVertexBuffer(const std::vector<float> &field,
                                                                            double isovalue,
                                                                            int3 dimensions,
                                                                            double3x3 unitCell)
{
  int nx = dimensions.x;
  int ny = dimensions.y;
  int nz = dimensions.z;
  int numberOfGridPoints = nx * ny * nz;
  if (nx <= 0 || ny <= 0 || nz <= 0 || field.size() < std::size_t(3 * numberOfGridPoints))
  {
    return {};
  }

  float iso = effectiveTrimIsovalue(field, isovalue, dimensions);

  std::vector<bool> candidate(std::size_t(numberOfGridPoints), false);
  std::vector<bool> seeds(std::size_t(numberOfGridPoints), false);
  bool anySeed = false;
  for (int i = 0; i < numberOfGridPoints; ++i)
  {
    float energyTerm = energyScale * (field[std::size_t(3 * i)] - iso);
    float distance = field[std::size_t(3 * i + 1)];
    if (distance < 0.0f && energyTerm < 0.0f)
    {
      candidate[std::size_t(i)] = true;
      if (field[std::size_t(3 * i + 2)] < filamentReliabilityThreshold)
      {
        seeds[std::size_t(i)] = true;
        anySeed = true;
      }
    }
  }
  if (!anySeed) return {};

  std::vector<bool> closed = seeds;
  for (int step = 0; step < filamentClosingRadius; ++step)
  {
    closed = dilateBinary(closed, nx, ny, nz);
    for (int i = 0; i < numberOfGridPoints; ++i)
    {
      if (!candidate[std::size_t(i)]) closed[std::size_t(i)] = false;
    }
  }
  for (int step = 0; step < filamentClosingRadius; ++step)
  {
    closed = erodeBinary(closed, nx, ny, nz);
  }
  for (int i = 0; i < numberOfGridPoints; ++i)
  {
    if (!candidate[std::size_t(i)]) closed[std::size_t(i)] = false;
    else if (seeds[std::size_t(i)]) closed[std::size_t(i)] = true;
  }
  fillEnclosedCavities(closed, candidate, nx, ny, nz);

  std::vector<float> combined(std::size_t(numberOfGridPoints), 0.0f);
  int interiorPoints = 0;
  for (int i = 0; i < numberOfGridPoints; ++i)
  {
    float energyTerm = energyScale * (field[std::size_t(3 * i)] - iso);
    float distance = field[std::size_t(3 * i + 1)];
    float raw = std::max(field[std::size_t(3 * i + 2)] - filamentReliabilityThreshold, std::max(distance, energyTerm));
    float value = (closed[std::size_t(i)] && raw >= 0.0f) ? std::min(-1.0e-4f, std::max(distance, energyTerm)) : raw;
    combined[std::size_t(i)] = value;
    if (value < 0.0f) ++interiorPoints;
  }
  if (interiorPoints == 0) return {};

  std::vector<float4> mesh = SKComputeIsosurface::computeIsosurface(dimensions, &combined, 0.0);
  if (mesh.empty()) return {};
  return removeFilamentSpecks(mesh, unitCell);
}

double SKComputeWellSurface::areaOfVertexBuffer(const std::vector<float4> &vertexBuffer,
                                                const double3x3 &unitCell) noexcept
{
  std::size_t triangles = vertexBuffer.size() / 9;
  double totalArea = 0.0;
  for (std::size_t t = 0; t < triangles; ++t)
  {
    const float4 &v0 = vertexBuffer[9 * t + 0];
    const float4 &v1 = vertexBuffer[9 * t + 3];
    const float4 &v2 = vertexBuffer[9 * t + 6];
    double3 p0 = unitCell * double3(double(v0.x), double(v0.y), double(v0.z));
    double3 p1 = unitCell * double3(double(v1.x), double(v1.y), double(v1.z));
    double3 p2 = unitCell * double3(double(v2.x), double(v2.y), double(v2.z));
    totalArea += 0.5 * double3::cross(p1 - p0, p2 - p0).length();
  }
  return totalArea;
}
