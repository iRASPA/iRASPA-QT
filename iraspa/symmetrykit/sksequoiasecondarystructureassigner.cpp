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

#include "sksequoiasecondarystructureassigner.h"
#include "sksecondarystructuregeometry.h"
#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace {

constexpr int kNearestNeighborCount = 2;
constexpr double kHelixScoreThreshold = 0.55;
constexpr double kSheetScoreThreshold = 0.55;
constexpr double kHelixMinimumLength = 4;
constexpr double kSheetMinimumLength = 3;
constexpr double kPi = 3.14159265358979323846;

struct SegmentMetrics
{
  std::vector<std::optional<double>> sequentialDistance2;
  std::vector<std::optional<double>> sequentialDistance3;
  std::vector<std::optional<double>> sequentialDistance4;
  std::vector<std::optional<double>> sequentialCosPhi;
  std::vector<std::optional<double>> neighborCosPhiAverage;
  std::vector<std::optional<double>> neighborDistanceAverage;
};

struct NeighborEntry
{
  int partner = 0;
  double distance = 0.0;
};

double gaussianScore(const std::optional<double> &value, double center, double width, double weight)
{
  if (!value.has_value())
  {
    return 0.0;
  }
  const double normalized = (value.value() - center) / width;
  return weight * std::exp(-0.5 * normalized * normalized);
}

std::optional<double> pseudoDihedralCosine(const std::vector<double3> &positions, const std::vector<int> &indices)
{
  if (indices.size() != 4)
  {
    return std::nullopt;
  }
  for (int index : indices)
  {
    if (index < 0 || index >= static_cast<int>(positions.size()))
    {
      return std::nullopt;
    }
  }
  const double angleDegrees = SKSecondaryStructureGeometry::torsionAngle(positions[static_cast<size_t>(indices[0])],
                                                                         positions[static_cast<size_t>(indices[1])],
                                                                         positions[static_cast<size_t>(indices[2])],
                                                                         positions[static_cast<size_t>(indices[3])]);
  return std::cos(angleDegrees * kPi / 180.0);
}

std::vector<NeighborEntry> nearestNeighbors(int index, const std::vector<double3> &positions)
{
  std::vector<NeighborEntry> neighbors;
  for (size_t partnerIndex = 0; partnerIndex < positions.size(); ++partnerIndex)
  {
    if (static_cast<int>(partnerIndex) == index)
    {
      continue;
    }
    NeighborEntry entry;
    entry.partner = static_cast<int>(partnerIndex);
    entry.distance = SKSecondaryStructureGeometry::distance(positions[static_cast<size_t>(index)], positions[partnerIndex]);
    neighbors.push_back(entry);
  }
  std::sort(neighbors.begin(), neighbors.end(), [](const NeighborEntry &lhs, const NeighborEntry &rhs) {
    return lhs.distance < rhs.distance;
  });
  if (static_cast<int>(neighbors.size()) > kNearestNeighborCount)
  {
    neighbors.resize(static_cast<size_t>(kNearestNeighborCount));
  }
  return neighbors;
}

std::optional<double> neighborPseudoDihedralCosine(const std::vector<double3> &positions, int sourceIndex, int partnerIndex)
{
  const std::vector<NeighborEntry> sortedBySource = nearestNeighbors(sourceIndex, positions);
  if (sortedBySource.size() < 2)
  {
    return std::nullopt;
  }
  const int sourcePrimeIndex = sortedBySource[1].partner;
  const std::vector<NeighborEntry> sortedByPartner = nearestNeighbors(partnerIndex, positions);
  int partnerPrimeIndex = -1;
  for (const NeighborEntry &entry : sortedByPartner)
  {
    if (entry.partner != sourceIndex && entry.partner != sourcePrimeIndex)
    {
      partnerPrimeIndex = entry.partner;
      break;
    }
  }
  if (partnerPrimeIndex < 0)
  {
    return std::nullopt;
  }
  return pseudoDihedralCosine(positions, {sourcePrimeIndex, sourceIndex, partnerIndex, partnerPrimeIndex});
}

SegmentMetrics computeMetrics(const std::vector<double3> &positions)
{
  const size_t count = positions.size();
  SegmentMetrics metrics;
  metrics.sequentialDistance2.assign(count, std::nullopt);
  metrics.sequentialDistance3.assign(count, std::nullopt);
  metrics.sequentialDistance4.assign(count, std::nullopt);
  metrics.sequentialCosPhi.assign(count, std::nullopt);
  metrics.neighborCosPhiAverage.assign(count, std::nullopt);
  metrics.neighborDistanceAverage.assign(count, std::nullopt);

  for (size_t index = 1; index + 1 < count; ++index)
  {
    metrics.sequentialDistance2[index] = SKSecondaryStructureGeometry::distance(positions[index - 1], positions[index + 1]);
  }
  for (size_t index = 1; index + 2 < count; ++index)
  {
    metrics.sequentialDistance3[index] = SKSecondaryStructureGeometry::distance(positions[index - 1], positions[index + 2]);
    metrics.sequentialCosPhi[index] = pseudoDihedralCosine(positions, {static_cast<int>(index - 1), static_cast<int>(index),
                                                                       static_cast<int>(index + 1), static_cast<int>(index + 2)});
  }
  for (size_t index = 1; index + 3 < count; ++index)
  {
    metrics.sequentialDistance4[index] = SKSecondaryStructureGeometry::distance(positions[index - 1], positions[index + 3]);
  }

  for (size_t index = 0; index < count; ++index)
  {
    const std::vector<NeighborEntry> neighbors = nearestNeighbors(static_cast<int>(index), positions);
    if (neighbors.empty())
    {
      continue;
    }
    std::vector<double> cosValues;
    std::vector<double> distances;
    for (const NeighborEntry &neighbor : neighbors)
    {
      distances.push_back(neighbor.distance);
      if (const std::optional<double> cosPhi = neighborPseudoDihedralCosine(positions, static_cast<int>(index), neighbor.partner))
      {
        cosValues.push_back(cosPhi.value());
      }
    }
    if (!cosValues.empty())
    {
      double sum = 0.0;
      for (double value : cosValues)
      {
        sum += value;
      }
      metrics.neighborCosPhiAverage[index] = sum / static_cast<double>(cosValues.size());
    }
    double distanceSum = 0.0;
    for (double value : distances)
    {
      distanceSum += value;
    }
    metrics.neighborDistanceAverage[index] = distanceSum / static_cast<double>(distances.size());
  }
  return metrics;
}

std::vector<double> helixScores(const SegmentMetrics &metrics)
{
  std::vector<double> scores(metrics.sequentialDistance3.size(), 0.0);
  for (size_t index = 0; index < scores.size(); ++index)
  {
    double score = 0.0;
    score += gaussianScore(metrics.sequentialDistance3[index], 5.3, 0.5, 0.30);
    score += gaussianScore(metrics.sequentialDistance4[index], 6.4, 0.6, 0.25);
    score += gaussianScore(metrics.sequentialCosPhi[index], 0.64, 0.35, 0.20);
    score += gaussianScore(metrics.neighborCosPhiAverage[index], 0.55, 0.40, 0.15);
    score += gaussianScore(metrics.neighborDistanceAverage[index], 5.5, 1.5, 0.10);
    scores[index] = std::min(score, 1.0);
  }
  return scores;
}

std::vector<double> sheetScores(const SegmentMetrics &metrics)
{
  std::vector<double> scores(metrics.sequentialDistance3.size(), 0.0);
  for (size_t index = 0; index < scores.size(); ++index)
  {
    double score = 0.0;
    score += gaussianScore(metrics.sequentialDistance2[index], 6.7, 0.6, 0.20);
    score += gaussianScore(metrics.sequentialDistance3[index], 9.9, 0.9, 0.35);
    score += gaussianScore(metrics.sequentialDistance4[index], 12.4, 1.1, 0.20);
    if (metrics.sequentialCosPhi[index].has_value() && metrics.sequentialCosPhi[index].value() <= -0.5)
    {
      score += 0.10;
    }
    if (metrics.neighborCosPhiAverage[index].has_value() && metrics.neighborCosPhiAverage[index].value() <= -0.35)
    {
      score += 0.15;
    }
    scores[index] = std::min(score, 1.0);
  }
  return scores;
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

std::vector<SKSecondaryStructureType> assignSegment(const std::vector<double3> &positions)
{
  const size_t count = positions.size();
  if (count < 4)
  {
    return std::vector<SKSecondaryStructureType>(count, SKSecondaryStructureType::coil);
  }
  const SegmentMetrics metrics = computeMetrics(positions);
  const std::vector<double> helixScoreValues = helixScores(metrics);
  const std::vector<double> sheetScoreValues = sheetScores(metrics);

  std::vector<bool> helixCandidates(count);
  std::vector<bool> sheetCandidates(count);
  for (size_t index = 0; index < count; ++index)
  {
    helixCandidates[index] = helixScoreValues[index] >= kHelixScoreThreshold;
    sheetCandidates[index] = sheetScoreValues[index] >= kSheetScoreThreshold && helixScoreValues[index] < kHelixScoreThreshold - 0.05;
  }

  const std::vector<bool> helixMask = SKSecondaryStructureGeometry::maskConsecutive(helixCandidates, static_cast<int>(kHelixMinimumLength));
  const std::vector<bool> sheetMask = SKSecondaryStructureGeometry::maskConsecutive(sheetCandidates, static_cast<int>(kSheetMinimumLength));
  return mapToTypes(count, helixMask, sheetMask);
}

std::vector<std::vector<double3>> alphaCarbonSegments(const SKStrideBackboneChain &chain)
{
  std::vector<SKStrideBackboneResidue> residues;
  for (const SKStrideBackboneResidue &residue : chain.residues)
  {
    if (residue.alphaCarbon.has_value())
    {
      residues.push_back(residue);
    }
  }
  if (residues.empty())
  {
    return {};
  }

  std::vector<std::vector<double3>> segments;
  std::vector<double3> currentSegment;
  for (size_t index = 0; index < residues.size(); ++index)
  {
    if (!residues[index].alphaCarbon.has_value())
    {
      continue;
    }
    if (index > 0 && residues[index - 1].alphaCarbon.has_value() &&
        SKSecondaryStructureGeometry::distance(residues[index - 1].alphaCarbon.value(), residues[index].alphaCarbon.value()) > 4.5)
    {
      if (!currentSegment.empty())
      {
        segments.push_back(currentSegment);
        currentSegment.clear();
      }
    }
    currentSegment.push_back(residues[index].alphaCarbon.value());
  }
  if (!currentSegment.empty())
  {
    segments.push_back(currentSegment);
  }
  return segments;
}

} // namespace

std::vector<SKSecondaryStructureType> SKSequoiaSecondaryStructureAssigner::assign(const SKStrideBackboneChain &chain)
{
  const std::vector<std::vector<double3>> segments = alphaCarbonSegments(chain);
  if (segments.empty())
  {
    return {};
  }
  std::vector<SKSecondaryStructureType> assignments;
  for (const std::vector<double3> &segment : segments)
  {
    if (segment.empty())
    {
      continue;
    }
    const std::vector<SKSecondaryStructureType> segmentAssignments = assignSegment(segment);
    assignments.insert(assignments.end(), segmentAssignments.begin(), segmentAssignments.end());
  }
  return assignments;
}
