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

#include "sksecondarystructuregeometry.h"
#include <algorithm>
#include <cmath>

static constexpr double kPi = 3.14159265358979323846;

std::vector<std::optional<double3>> SKSecondaryStructureGeometry::alphaCarbonPositions(const SKStrideBackboneChain &chain)
{
  std::vector<std::optional<double3>> positions;
  for (const SKStrideBackboneResidue &residue : chain.residues)
  {
    if (residue.alphaCarbon.has_value())
    {
      positions.push_back(residue.alphaCarbon);
    }
  }
  return positions;
}

std::vector<std::optional<double3>> SKSecondaryStructureGeometry::alphaCarbonPositionsWithBreaks(const SKStrideBackboneChain &chain,
                                                                                                    double breakDistance)
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

  std::vector<std::optional<double3>> positions;
  for (size_t index = 0; index < residues.size(); ++index)
  {
    if (index > 0)
    {
      const double3 &previous = residues[index - 1].alphaCarbon.value();
      const double3 &current = residues[index].alphaCarbon.value();
      if (distance(previous, current) > breakDistance)
      {
        positions.push_back(std::nullopt);
      }
    }
    positions.push_back(residues[index].alphaCarbon);
  }
  return positions;
}

std::vector<SKSecondaryStructureGeometry::PhiPsiDegrees> SKSecondaryStructureGeometry::phiPsiDegrees(const SKStrideBackboneChain &chain)
{
  std::vector<SKStrideBackboneResidue> residues;
  for (const SKStrideBackboneResidue &residue : chain.residues)
  {
    if (residue.alphaCarbon.has_value())
    {
      residues.push_back(residue);
    }
  }

  std::vector<PhiPsiDegrees> angles(residues.size());
  for (size_t index = 0; index < residues.size(); ++index)
  {
    if (!residues[index].nitrogen.has_value() || !residues[index].alphaCarbon.has_value() ||
        !residues[index].carbonylCarbon.has_value())
    {
      continue;
    }
    const double3 &nitrogen = residues[index].nitrogen.value();
    const double3 &alphaCarbon = residues[index].alphaCarbon.value();
    const double3 &carbonylCarbon = residues[index].carbonylCarbon.value();

    if (index > 0 && residues[index - 1].carbonylCarbon.has_value())
    {
      angles[index].phi = torsionAngle(residues[index - 1].carbonylCarbon.value(), nitrogen, alphaCarbon, carbonylCarbon);
    }
    if (index + 1 < residues.size() && residues[index + 1].nitrogen.has_value())
    {
      angles[index].psi = torsionAngle(nitrogen, alphaCarbon, carbonylCarbon, residues[index + 1].nitrogen.value());
    }
  }
  return angles;
}

std::optional<SKSecondaryStructureGeometry::LocalAxisFrame> SKSecondaryStructureGeometry::localAxisFrame(int index,
                                                                                                          const std::vector<double3> &positions)
{
  const int windowStart = index - 1;
  const int windowEnd = index + 2;
  if (windowStart < 0 || windowEnd >= static_cast<int>(positions.size()))
  {
    return std::nullopt;
  }

  double3 centroid(0.0, 0.0, 0.0);
  for (int positionIndex = windowStart; positionIndex <= windowEnd; ++positionIndex)
  {
    centroid += positions[static_cast<size_t>(positionIndex)];
  }
  centroid = centroid / 4.0;

  double3 direction = positions[static_cast<size_t>(windowEnd)] - positions[static_cast<size_t>(windowStart)];
  const double directionLength = direction.length();
  if (directionLength <= 1.0e-12)
  {
    return std::nullopt;
  }

  LocalAxisFrame frame;
  frame.origin = centroid;
  frame.direction = direction / directionLength;
  return frame;
}

double SKSecondaryStructureGeometry::axisRadius(const double3 &position, const LocalAxisFrame &frame)
{
  const double3 vector = position - frame.origin;
  const double3 projected = vector - frame.direction * double3::dot(vector, frame.direction);
  return projected.length();
}

double SKSecondaryStructureGeometry::axisTau(const double3 &position, const LocalAxisFrame &frame)
{
  const double3 vector = position - frame.origin;
  const double vectorLength = vector.length();
  if (vectorLength <= 1.0e-12)
  {
    return 0.0;
  }
  double scalar = double3::dot(normalize(vector), frame.direction);
  scalar = std::min(std::max(scalar, -1.0), 1.0);
  return std::acos(scalar) * 180.0 / kPi;
}

std::vector<bool> SKSecondaryStructureGeometry::maskConsecutive(const std::vector<bool> &mask, int minimumLength)
{
  if (minimumLength <= 0 || mask.empty())
  {
    return {};
  }
  std::vector<bool> output(mask.size(), false);
  std::optional<int> startIndex;

  for (size_t index = 0; index < mask.size(); ++index)
  {
    if (mask[index])
    {
      if (!startIndex.has_value())
      {
        startIndex = static_cast<int>(index);
      }
    }
    else if (startIndex.has_value())
    {
      const int start = startIndex.value();
      if (static_cast<int>(index) - start >= minimumLength)
      {
        for (int fillIndex = start; fillIndex < static_cast<int>(index); ++fillIndex)
        {
          output[static_cast<size_t>(fillIndex)] = true;
        }
      }
      startIndex = std::nullopt;
    }
  }
  if (startIndex.has_value())
  {
    const int start = startIndex.value();
    if (static_cast<int>(mask.size()) - start >= minimumLength)
    {
      for (size_t fillIndex = static_cast<size_t>(start); fillIndex < mask.size(); ++fillIndex)
      {
        output[fillIndex] = true;
      }
    }
  }
  return output;
}

std::vector<bool> SKSecondaryStructureGeometry::extendRegion(const std::vector<bool> &base, const std::vector<bool> &extensionMask)
{
  if (base.empty())
  {
    return {};
  }
  std::vector<bool> output = base;
  for (size_t index = 0; index < base.size(); ++index)
  {
    if (!base[index])
    {
      const bool leftCandidate = index > 0 && base[index - 1];
      const bool rightCandidate = index + 1 < base.size() && base[index + 1];
      if ((leftCandidate || rightCandidate) && extensionMask[index])
      {
        output[index] = true;
      }
    }
  }
  return output;
}

double SKSecondaryStructureGeometry::distance(const double3 &a, const double3 &b)
{
  return (a - b).length();
}

double SKSecondaryStructureGeometry::angle(const double3 &p0, const double3 &p1, const double3 &p2)
{
  double3 vector0 = p0 - p1;
  double3 vector1 = p2 - p1;
  const double length0 = vector0.length();
  const double length1 = vector1.length();
  if (length0 < 1.0e-12 || length1 < 1.0e-12)
  {
    return 0.0;
  }
  double scalar = double3::dot(vector0, vector1) / (length0 * length1);
  scalar = std::min(std::max(scalar, -1.0), 1.0);
  return std::acos(scalar) * 180.0 / kPi;
}

double SKSecondaryStructureGeometry::torsionAngle(const double3 &p0, const double3 &p1, const double3 &p2, const double3 &p3)
{
  const double3 bond0 = p1 - p0;
  const double3 bond1 = p2 - p1;
  const double3 bond2 = p3 - p2;

  double3 normal0 = double3::cross(bond0, bond1);
  double3 normal1 = double3::cross(bond1, bond2);
  const double length0 = normal0.length();
  const double length1 = normal1.length();
  if (length0 < 1.0e-12 || length1 < 1.0e-12)
  {
    return 0.0;
  }
  normal0 = normal0 / length0;
  normal1 = normal1 / length1;

  double scalar = double3::dot(normal0, normal1);
  scalar = std::min(std::max(scalar, -1.0 + 1.0e-6), 1.0 - 1.0e-6);
  const double absoluteAngle = std::acos(scalar) * 180.0 / kPi;
  return double3::dot(bond0, normal1) > 0.0 ? absoluteAngle : -absoluteAngle;
}

double3 SKSecondaryStructureGeometry::normalize(const double3 &vector)
{
  const double lengthValue = vector.length();
  if (lengthValue <= 1.0e-12)
  {
    return double3(0.0, 0.0, 0.0);
  }
  return vector / lengthValue;
}

std::optional<double3> SKSecondaryStructureGeometry::localHelixAxis(int index, const std::vector<double3> &positions)
{
  const int start = std::max(0, index - 1);
  const int end = std::min(static_cast<int>(positions.size()) - 1, index + 2);
  if (end <= start)
  {
    return std::nullopt;
  }

  double3 average(0.0, 0.0, 0.0);
  int count = 0;
  for (int positionIndex = start; positionIndex <= end; ++positionIndex)
  {
    average += positions[static_cast<size_t>(positionIndex)];
    count += 1;
  }
  if (count < 3)
  {
    return std::nullopt;
  }
  average = average / static_cast<double>(count);

  const double3 direction = positions[static_cast<size_t>(end)] - positions[static_cast<size_t>(start)];
  const double directionLength = direction.length();
  if (directionLength <= 1.0e-12)
  {
    return std::nullopt;
  }
  return normalize(direction);
}

double SKSecondaryStructureGeometry::helixRadius(int index, const std::vector<double3> &positions,
                                                  const double3 &axisOrigin, const double3 &axis)
{
  const double3 vector = positions[static_cast<size_t>(index)] - axisOrigin;
  const double3 projected = vector - axis * double3::dot(vector, axis);
  return projected.length();
}

double SKSecondaryStructureGeometry::tauAngle(const double3 &position, const double3 &axisOrigin, const double3 &axis)
{
  const double3 vector = position - axisOrigin;
  const double3 projected = vector - axis * double3::dot(vector, axis);
  if (projected.length() < 1.0e-12 || vector.length() < 1.0e-12)
  {
    return 0.0;
  }
  double scalar = double3::dot(normalize(projected), normalize(vector));
  scalar = std::min(std::max(scalar, -1.0), 1.0);
  return std::acos(scalar) * 180.0 / kPi;
}

bool SKSecondaryStructureGeometry::inRange(double value, double lower, double upper)
{
  return value >= lower && value <= upper;
}

bool SKSecondaryStructureGeometry::inWrappedRange(double value, double lower, double upper)
{
  if (upper > 180.0 && value < 0.0)
  {
    return value + 360.0 >= lower && value + 360.0 <= upper;
  }
  return value >= lower && value <= upper;
}

double SKSecondaryStructureGeometry::acutePeptidePlaneAngle(const double3 &firstNitrogen,
                                                               const double3 &firstAlphaCarbon,
                                                               const double3 &firstCarbonylCarbon,
                                                               const double3 &secondNitrogen,
                                                               const double3 &secondAlphaCarbon,
                                                               const double3 &secondCarbonylCarbon)
{
  const double3 firstNormal = normalize(double3::cross(firstAlphaCarbon - firstNitrogen,
                                                        firstCarbonylCarbon - firstNitrogen));
  const double3 secondNormal = normalize(double3::cross(secondAlphaCarbon - secondNitrogen,
                                                         secondCarbonylCarbon - secondNitrogen));
  double scalar = std::abs(double3::dot(firstNormal, secondNormal));
  scalar = std::min(std::max(scalar, -1.0), 1.0);
  return std::acos(scalar) * 180.0 / kPi;
}
