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

#include "skpseasecondarystructureassigner.h"
#include "sksecondarystructuregeometry.h"
#include <optional>
#include <vector>

namespace {

struct Metrics
{
  std::vector<std::optional<double>> distance2;
  std::vector<std::optional<double>> distance3;
  std::vector<std::optional<double>> distance4;
  std::vector<std::optional<double>> angle;
  std::vector<std::optional<double>> dihedral;
};

bool inRange(const std::optional<double> &value, double lower, double upper)
{
  if (!value.has_value())
  {
    return false;
  }
  return value.value() >= lower && value.value() <= upper;
}

bool strandDihedralInRange(const std::optional<double> &value)
{
  if (!value.has_value())
  {
    return false;
  }
  const double v = value.value();
  return (v >= -180.0 && v <= -125.0) || (v >= 145.0 && v <= 180.0);
}

Metrics computeMetrics(const std::vector<std::optional<double3>> &positions)
{
  const size_t count = positions.size();
  Metrics metrics;
  metrics.distance2.assign(count, std::nullopt);
  metrics.distance3.assign(count, std::nullopt);
  metrics.distance4.assign(count, std::nullopt);
  metrics.angle.assign(count, std::nullopt);
  metrics.dihedral.assign(count, std::nullopt);

  for (size_t index = 1; index + 1 < count; ++index)
  {
    if (positions[index - 1].has_value() && positions[index + 1].has_value())
    {
      metrics.distance2[index] = SKSecondaryStructureGeometry::distance(positions[index - 1].value(), positions[index + 1].value());
    }
    if (positions[index - 1].has_value() && positions[index].has_value() && positions[index + 1].has_value())
    {
      metrics.angle[index] = SKSecondaryStructureGeometry::angle(positions[index - 1].value(), positions[index].value(), positions[index + 1].value());
    }
  }

  for (size_t index = 1; index + 2 < count; ++index)
  {
    if (positions[index - 1].has_value() && positions[index + 2].has_value())
    {
      metrics.distance3[index] = SKSecondaryStructureGeometry::distance(positions[index - 1].value(), positions[index + 2].value());
    }
    if (positions[index - 1].has_value() && positions[index].has_value() && positions[index + 1].has_value() && positions[index + 2].has_value())
    {
      metrics.dihedral[index] = SKSecondaryStructureGeometry::torsionAngle(positions[index - 1].value(), positions[index].value(),
                                                                           positions[index + 1].value(), positions[index + 2].value());
    }
  }

  for (size_t index = 1; index + 3 < count; ++index)
  {
    if (positions[index - 1].has_value() && positions[index + 3].has_value())
    {
      metrics.distance4[index] = SKSecondaryStructureGeometry::distance(positions[index - 1].value(), positions[index + 3].value());
    }
  }
  return metrics;
}

std::vector<bool> assignHelices(const Metrics &metrics)
{
  const size_t count = metrics.distance3.size();
  std::vector<bool> relaxedHelix(count, false);
  std::vector<bool> strictHelix(count, false);

  for (size_t index = 0; index < count; ++index)
  {
    const bool d3InRange = inRange(metrics.distance3[index], 4.8, 5.8);
    const bool d4InRange = inRange(metrics.distance4[index], 5.8, 7.0);
    const bool angleInRange = inRange(metrics.angle[index], 77.0, 101.0);
    const bool dihedralInRange = inRange(metrics.dihedral[index], 30.0, 70.0);
    relaxedHelix[index] = d3InRange || angleInRange;
    strictHelix[index] = (d3InRange && d4InRange) || (angleInRange && dihedralInRange);
  }

  std::vector<bool> helixMask = SKSecondaryStructureGeometry::maskConsecutive(strictHelix, 5);
  return SKSecondaryStructureGeometry::extendRegion(helixMask, relaxedHelix);
}

std::vector<bool> maskRegionsWithContacts(const std::vector<std::optional<double3>> &positions,
                                          const std::vector<bool> &candidateMask,
                                          int minimumContacts,
                                          double minimumDistance,
                                          double maximumDistance)
{
  if (positions.empty())
  {
    return {};
  }

  std::vector<double3> validPositions;
  for (const std::optional<double3> &position : positions)
  {
    if (position.has_value())
    {
      validPositions.push_back(position.value());
    }
  }
  if (validPositions.empty())
  {
    return std::vector<bool>(positions.size(), false);
  }

  std::vector<int> contacts(positions.size(), 0);
  for (size_t index = 0; index < positions.size(); ++index)
  {
    if (!candidateMask[index] || !positions[index].has_value())
    {
      continue;
    }
    int contactCount = 0;
    for (const double3 &other : validPositions)
    {
      const double separation = SKSecondaryStructureGeometry::distance(positions[index].value(), other);
      if (separation > minimumDistance && separation <= maximumDistance)
      {
        ++contactCount;
      }
    }
    contacts[index] = contactCount;
  }

  std::vector<bool> output(positions.size(), false);
  std::optional<size_t> regionStart;
  for (size_t index = 0; index <= positions.size(); ++index)
  {
    const bool inRegion = index < positions.size() && candidateMask[index];
    if (inRegion)
    {
      if (!regionStart.has_value())
      {
        regionStart = index;
      }
    }
    else if (regionStart.has_value())
    {
      const size_t start = regionStart.value();
      int totalContacts = 0;
      for (size_t fillIndex = start; fillIndex < index; ++fillIndex)
      {
        totalContacts += contacts[fillIndex];
      }
      if (totalContacts >= minimumContacts)
      {
        for (size_t fillIndex = start; fillIndex < index; ++fillIndex)
        {
          output[fillIndex] = true;
        }
      }
      regionStart = std::nullopt;
    }
  }
  return output;
}

std::vector<bool> assignStrands(const Metrics &metrics, const std::vector<std::optional<double3>> &positions)
{
  const size_t count = metrics.distance3.size();
  std::vector<bool> relaxedStrand(count, false);
  std::vector<bool> strictStrand(count, false);

  for (size_t index = 0; index < count; ++index)
  {
    const bool d2InRange = inRange(metrics.distance2[index], 6.1, 7.3);
    const bool d3InRange = inRange(metrics.distance3[index], 9.0, 10.8);
    const bool d4InRange = inRange(metrics.distance4[index], 10.3, 13.5);
    const bool angleInRange = inRange(metrics.angle[index], 110.0, 138.0);
    const bool dihedralInRange = strandDihedralInRange(metrics.dihedral[index]);
    relaxedStrand[index] = d3InRange;
    strictStrand[index] = (d2InRange && d3InRange && d4InRange) || (angleInRange && dihedralInRange);
  }

  std::vector<bool> strandMask = SKSecondaryStructureGeometry::maskConsecutive(strictStrand, 4);
  const std::vector<bool> shortStrandSeed = SKSecondaryStructureGeometry::maskConsecutive(strictStrand, 3);
  const std::vector<bool> shortStrandMask = maskRegionsWithContacts(positions, shortStrandSeed, 5, 4.2, 5.2);
  std::vector<bool> combinedStrand(count);
  for (size_t index = 0; index < count; ++index)
  {
    combinedStrand[index] = strandMask[index] || shortStrandMask[index];
  }
  return SKSecondaryStructureGeometry::extendRegion(combinedStrand, relaxedStrand);
}

std::vector<SKSecondaryStructureType> mapToTypes(size_t count, const std::vector<bool> &helix, const std::vector<bool> &sheet)
{
  std::vector<SKSecondaryStructureType> assignments(count, SKSecondaryStructureType::coil);
  for (size_t index = 0; index < count; ++index)
  {
    if (helix[index])
    {
      assignments[index] = SKSecondaryStructureType::helix;
    }
    if (sheet[index])
    {
      assignments[index] = SKSecondaryStructureType::sheet;
    }
  }
  return assignments;
}

} // namespace

std::vector<SKSecondaryStructureType> SKPSeaSecondaryStructureAssigner::assign(const SKStrideBackboneChain &chain)
{
  const std::vector<std::optional<double3>> positions = SKSecondaryStructureGeometry::alphaCarbonPositionsWithBreaks(chain);
  if (positions.size() <= 5)
  {
    return std::vector<SKSecondaryStructureType>(positions.size(), SKSecondaryStructureType::coil);
  }
  const Metrics metrics = computeMetrics(positions);
  const std::vector<bool> helixMask = assignHelices(metrics);
  const std::vector<bool> sheetMask = assignStrands(metrics, positions);
  return mapToTypes(positions.size(), helixMask, sheetMask);
}
