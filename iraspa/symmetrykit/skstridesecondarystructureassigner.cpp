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

#include "skstridesecondarystructureassigner.h"
#include "skstrideramachandranmaps.h"
#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kUndefinedAngle = 360.0;
constexpr double kDistanceCutoff = 6.0;
constexpr double kGridMinimumDistance = 3.0;
constexpr double kPolarEnergyThreshold = -10.0;
constexpr double kHelixScoreThreshold = -230.0;
constexpr double kHelixBoundaryProbability = 0.12;
constexpr double kHelixTrailingProbability = 0.06;
constexpr double kSheetScoreThreshold = -240.0;
constexpr double kHelixTorsionWeight = 1.0;
constexpr double kSheetTorsionWeight = 0.2;
constexpr double kSheetTorsionOffset = -0.2;
constexpr int kMinimumSegmentLength = 3;
constexpr double kHelixTorsionFallback = 0.15;
constexpr double kSheetTorsionFallback = 0.08;
constexpr double kBakerHydrogenBondEnergy = -450.0;
constexpr double kHelixBakerScoreThreshold = -150.0;
constexpr double kSheetBridgeProbability = 0.05;
const double kGridConstantsC = -3.0 * (-2.8) * std::pow(3.0, 8.0);
const double kGridConstantsD = -4.0 * (-2.8) * std::pow(3.0, 6.0);
const double kGridK1 = 0.9 / std::pow(std::cos(110.0 * kPi / 180.0), 6.0);
const double kGridK2 = std::pow(std::cos(110.0 * kPi / 180.0), 2.0);

struct BackboneResidueState
{
  std::optional<double3> nitrogen;
  std::optional<double3> alphaCarbon;
  std::optional<double3> carbonylCarbon;
  std::optional<double3> carbonylOxygen;
  std::optional<double3> hydrogen;
  double phiDegrees = 360.0;
  double psiDegrees = 360.0;
  char assignmentCode = 'C';
};

struct HydrogenBond
{
  int donorIndex = 0;
  int acceptorIndex = 0;
  double energy = 0.0;
  bool isPolarInteraction = false;
  bool isBakerHydrogenBond = false;
};

double distance(const double3 &a, const double3 &b)
{
  return (a - b).length();
}

double angle(const double3 &p0, const double3 &p1, const double3 &p2)
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
  scalar = std::min(std::max(scalar, -1.0 + 1.0e-6), 1.0 - 1.0e-6);
  return std::acos(scalar) * 180.0 / kPi;
}

double torsionAngle(const double3 &p0, const double3 &p1, const double3 &p2, const double3 &p3)
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
    return kUndefinedAngle;
  }
  normal0 = normal0 / length0;
  normal1 = normal1 / length1;
  double scalar = double3::dot(normal0, normal1);
  scalar = std::min(std::max(scalar, -1.0 + 1.0e-6), 1.0 - 1.0e-6);
  const double absoluteAngle = std::acos(scalar) * 180.0 / kPi;
  return double3::dot(bond0, normal1) > 0.0 ? absoluteAngle : -absoluteAngle;
}

double3 projectOntoPlane(const double3 &point, const double3 &planePoint0, const double3 &planePoint1, const double3 &planePoint2)
{
  const double3 normal = double3::normalize(double3::cross(planePoint1 - planePoint0, planePoint2 - planePoint1));
  const double3 vector = point - planePoint0;
  return point - normal * double3::dot(vector, normal);
}

struct GridHydrogenBondComponents
{
  double energy = 0.0;
  double tangential = 0.0;
  double planar = 0.0;
};

GridHydrogenBondComponents gridHydrogenBondEnergy(const double3 &acceptorOxygen,
                                                  const double3 &acceptorCarbon,
                                                  const double3 &acceptorAlphaCarbon,
                                                  const double3 &hydrogen,
                                                  const double3 &donorNitrogen)
{
  const double acceptorHydrogenDistance = std::max(distance(acceptorOxygen, hydrogen), kGridMinimumDistance);
  const double radialEnergy = kGridConstantsC / std::pow(acceptorHydrogenDistance, 8.0) - kGridConstantsD / std::pow(acceptorHydrogenDistance, 6.0);
  const double3 projectedHydrogen = projectOntoPlane(hydrogen, acceptorOxygen, acceptorCarbon, acceptorAlphaCarbon);
  const double ti = std::abs(180.0 - angle(projectedHydrogen, acceptorOxygen, acceptorCarbon));
  const double to = angle(hydrogen, acceptorOxygen, projectedHydrogen);
  const double planarAngle = angle(donorNitrogen, hydrogen, acceptorOxygen);
  double tangentialComponent = 0.0;
  if (ti < 90.0)
  {
    tangentialComponent = std::cos(to * kPi / 180.0) * (0.9 + 0.1 * std::sin(2.0 * ti * kPi / 180.0));
  }
  else if (ti < 110.0)
  {
    tangentialComponent = kGridK1 * std::cos(to * kPi / 180.0) * std::pow(kGridK2 - std::pow(std::cos(ti * kPi / 180.0), 2.0), 3.0);
  }
  const double planarComponent = (planarAngle > 90.0 && planarAngle < 270.0) ? std::pow(std::cos(planarAngle * kPi / 180.0), 2.0) : 0.0;
  GridHydrogenBondComponents result;
  result.energy = 1000.0 * radialEnergy * tangentialComponent * planarComponent;
  result.tangential = tangentialComponent;
  result.planar = planarComponent;
  return result;
}

SKSecondaryStructureType structureTypeForCode(char code)
{
  switch (code)
  {
  case 'H':
  case 'G':
  case 'I':
    return SKSecondaryStructureType::helix;
  case 'E':
  case 'B':
  case 'b':
    return SKSecondaryStructureType::sheet;
  default:
    return SKSecondaryStructureType::coil;
  }
}

double helixProbability(const BackboneResidueState &state)
{
  return SKStrideRamachandranMaps::probability(SKStrideRamachandranMaps::helixProbabilityMap(), state.phiDegrees, state.psiDegrees);
}

double sheetProbability(const BackboneResidueState &state)
{
  return SKStrideRamachandranMaps::probability(SKStrideRamachandranMaps::sheetProbabilityMap(), state.phiDegrees, state.psiDegrees);
}

std::vector<BackboneResidueState> buildStates(const std::vector<SKStrideBackboneResidue> &residues)
{
  std::vector<BackboneResidueState> states;
  states.reserve(residues.size());
  for (const SKStrideBackboneResidue &residue : residues)
  {
    BackboneResidueState state;
    state.nitrogen = residue.nitrogen;
    state.alphaCarbon = residue.alphaCarbon;
    state.carbonylCarbon = residue.carbonylCarbon;
    state.carbonylOxygen = residue.carbonylOxygen;
    states.push_back(state);
  }
  return states;
}

void computeBackboneAngles(std::vector<BackboneResidueState> &states)
{
  const int count = static_cast<int>(states.size());
  for (int index = 0; index < count; ++index)
  {
    if (index > 0 && states[index - 1].carbonylCarbon.has_value() && states[index].nitrogen.has_value() &&
        states[index].alphaCarbon.has_value() && states[index].carbonylCarbon.has_value())
    {
      states[index].phiDegrees = torsionAngle(states[index - 1].carbonylCarbon.value(), states[index].nitrogen.value(),
                                              states[index].alphaCarbon.value(), states[index].carbonylCarbon.value());
    }
    if (index + 1 < count && states[index].nitrogen.has_value() && states[index].alphaCarbon.has_value() &&
        states[index].carbonylCarbon.has_value() && states[index + 1].nitrogen.has_value())
    {
      states[index].psiDegrees = torsionAngle(states[index].nitrogen.value(), states[index].alphaCarbon.value(),
                                              states[index].carbonylCarbon.value(), states[index + 1].nitrogen.value());
    }
  }
  for (int index = 0; index < count; ++index)
  {
    if (index > 0 && states[index].psiDegrees >= kUndefinedAngle)
    {
      states[index].psiDegrees = states[index - 1].psiDegrees;
    }
    if (index + 1 < count && states[index].phiDegrees >= kUndefinedAngle)
    {
      states[index].phiDegrees = states[index + 1].phiDegrees;
    }
  }
}

void placeBackboneHydrogens(std::vector<BackboneResidueState> &states, const std::vector<SKStrideBackboneResidue> &residues)
{
  for (size_t index = 1; index < states.size(); ++index)
  {
    if (residues[index].residueName.toUpper() == QStringLiteral("PRO"))
    {
      continue;
    }
    if (!states[index].nitrogen.has_value() || !states[index].alphaCarbon.has_value() || !states[index - 1].carbonylCarbon.has_value())
    {
      continue;
    }
    const double3 &nitrogen = states[index].nitrogen.value();
    const double3 &alphaCarbon = states[index].alphaCarbon.value();
    const double3 &previousCarbon = states[index - 1].carbonylCarbon.value();
    double3 direction = -(previousCarbon - nitrogen) / (previousCarbon - nitrogen).length();
    direction += -(alphaCarbon - nitrogen) / (alphaCarbon - nitrogen).length();
    const double directionLength = direction.length();
    if (directionLength <= 1.0e-12)
    {
      continue;
    }
    states[index].hydrogen = nitrogen + direction / directionLength;
  }
}

std::vector<HydrogenBond> findMainChainHydrogenBonds(const std::vector<BackboneResidueState> &states)
{
  std::vector<HydrogenBond> bonds;
  const int count = static_cast<int>(states.size());
  for (int donorIndex = 0; donorIndex < count; ++donorIndex)
  {
    if (!states[donorIndex].hydrogen.has_value() || !states[donorIndex].nitrogen.has_value())
    {
      continue;
    }
    const double3 &hydrogen = states[donorIndex].hydrogen.value();
    const double3 &donorNitrogen = states[donorIndex].nitrogen.value();
    const std::optional<double3> donorPreviousCarbon = donorIndex > 0 ? states[donorIndex - 1].carbonylCarbon : std::nullopt;
    if (!donorPreviousCarbon.has_value())
    {
      continue;
    }
    for (int acceptorIndex = 0; acceptorIndex < count; ++acceptorIndex)
    {
      if (std::abs(donorIndex - acceptorIndex) < 2)
      {
        continue;
      }
      if (!states[acceptorIndex].carbonylOxygen.has_value() || !states[acceptorIndex].carbonylCarbon.has_value() ||
          !states[acceptorIndex].alphaCarbon.has_value())
      {
        continue;
      }
      const double3 &acceptorOxygen = states[acceptorIndex].carbonylOxygen.value();
      const double3 &acceptorCarbon = states[acceptorIndex].carbonylCarbon.value();
      const double3 &acceptorAlphaCarbon = states[acceptorIndex].alphaCarbon.value();
      if (distance(hydrogen, acceptorOxygen) > kDistanceCutoff)
      {
        continue;
      }
      const GridHydrogenBondComponents components = gridHydrogenBondEnergy(acceptorOxygen, acceptorCarbon, acceptorAlphaCarbon, hydrogen, donorNitrogen);
      const bool isPolar = components.energy < kPolarEnergyThreshold && std::abs(components.tangential) > 1.0e-6 && std::abs(components.planar) > 1.0e-6;
      const bool isGeometric = distance(hydrogen, acceptorOxygen) <= 2.5 &&
                               angle(donorNitrogen, hydrogen, acceptorOxygen) >= 90.0 &&
                               angle(acceptorCarbon, acceptorOxygen, hydrogen) >= 90.0 &&
                               angle(acceptorOxygen, donorNitrogen, donorPreviousCarbon.value()) >= 90.0 &&
                               angle(acceptorOxygen, donorNitrogen, donorPreviousCarbon.value()) <= 180.0;
      if (isPolar || isGeometric)
      {
        HydrogenBond bond;
        bond.donorIndex = donorIndex;
        bond.acceptorIndex = acceptorIndex;
        bond.energy = components.energy;
        bond.isPolarInteraction = isPolar;
        bond.isBakerHydrogenBond = isGeometric;
        bonds.push_back(bond);
      }
    }
  }
  return bonds;
}

bool hasValidHelixAngles(const std::vector<BackboneResidueState> &states, int startIndex)
{
  for (int offset = 0; offset <= 4; ++offset)
  {
    const int index = startIndex + offset;
    if (states[static_cast<size_t>(index)].phiDegrees >= kUndefinedAngle || states[static_cast<size_t>(index)].psiDegrees >= kUndefinedAngle)
    {
      return false;
    }
  }
  return true;
}

std::optional<double> helixBondEnergy(int donorIndex, int acceptorIndex, double torsionConfidence, const std::vector<HydrogenBond> &hydrogenBonds)
{
  if (torsionConfidence < kHelixTorsionFallback)
  {
    return std::nullopt;
  }
  for (const HydrogenBond &bond : hydrogenBonds)
  {
    if (bond.donorIndex != donorIndex || bond.acceptorIndex != acceptorIndex)
    {
      continue;
    }
    if (bond.isPolarInteraction)
    {
      return bond.energy * kHelixTorsionWeight * torsionConfidence;
    }
    if (bond.isBakerHydrogenBond)
    {
      return kBakerHydrogenBondEnergy * kHelixTorsionWeight * torsionConfidence;
    }
  }
  return std::nullopt;
}

void assignHelices(std::vector<BackboneResidueState> &states, const std::vector<HydrogenBond> &hydrogenBonds)
{
  const int count = static_cast<int>(states.size());
  if (count < 6)
  {
    return;
  }
  std::vector<double> helixScore(count, 0.0);
  for (int startIndex = 0; startIndex < count - 5; ++startIndex)
  {
    if (!hasValidHelixAngles(states, startIndex))
    {
      continue;
    }
    const double torsionConfidence = 0.5 * (helixProbability(states[static_cast<size_t>(startIndex)]) + helixProbability(states[static_cast<size_t>(startIndex + 4)]));
    if (const std::optional<double> bondEnergy = helixBondEnergy(startIndex + 4, startIndex, torsionConfidence, hydrogenBonds))
    {
      helixScore[static_cast<size_t>(startIndex)] = bondEnergy.value();
    }
  }
  for (int startIndex = 0; startIndex < count - 5; ++startIndex)
  {
    if (helixScore[static_cast<size_t>(startIndex)] < kHelixScoreThreshold && helixScore[static_cast<size_t>(startIndex + 1)] < kHelixScoreThreshold)
    {
      for (int offset = 1; offset <= 4; ++offset)
      {
        states[static_cast<size_t>(startIndex + offset)].assignmentCode = 'H';
      }
      if (helixProbability(states[static_cast<size_t>(startIndex)]) > kHelixBoundaryProbability)
      {
        states[static_cast<size_t>(startIndex)].assignmentCode = 'H';
      }
      if (startIndex + 5 < count && helixProbability(states[static_cast<size_t>(startIndex + 5)]) > kHelixTrailingProbability)
      {
        states[static_cast<size_t>(startIndex + 5)].assignmentCode = 'H';
      }
    }
    else if (helixScore[static_cast<size_t>(startIndex)] < kHelixBakerScoreThreshold &&
             helixScore[static_cast<size_t>(startIndex + 1)] < kHelixBakerScoreThreshold)
    {
      for (int offset = 1; offset <= 4; ++offset)
      {
        states[static_cast<size_t>(startIndex + offset)].assignmentCode = 'H';
      }
    }
  }
}

void assignSheets(std::vector<BackboneResidueState> &states, const std::vector<HydrogenBond> &hydrogenBonds)
{
  const int count = static_cast<int>(states.size());
  for (const HydrogenBond &bond : hydrogenBonds)
  {
    if (!(bond.isPolarInteraction || bond.isBakerHydrogenBond))
    {
      continue;
    }
    const int donorIndex = bond.donorIndex;
    const int acceptorIndex = bond.acceptorIndex;
    if (std::abs(donorIndex - acceptorIndex) < 3)
    {
      continue;
    }
    if (states[static_cast<size_t>(acceptorIndex)].assignmentCode == 'H' || states[static_cast<size_t>(donorIndex)].assignmentCode == 'H')
    {
      continue;
    }
    const double torsionConfidence = 0.5 * (sheetProbability(states[static_cast<size_t>(donorIndex)]) + sheetProbability(states[static_cast<size_t>(acceptorIndex)]));
    if (torsionConfidence < kSheetTorsionFallback)
    {
      continue;
    }
    const double bondEnergy = bond.isPolarInteraction ? bond.energy : kBakerHydrogenBondEnergy;
    const double score = bondEnergy * (1.0 + kSheetTorsionOffset + kSheetTorsionWeight * torsionConfidence);
    if (score >= kSheetScoreThreshold)
    {
      continue;
    }
    if (states[static_cast<size_t>(acceptorIndex)].assignmentCode == 'C')
    {
      states[static_cast<size_t>(acceptorIndex)].assignmentCode = 'E';
    }
    if (states[static_cast<size_t>(donorIndex)].assignmentCode == 'C')
    {
      states[static_cast<size_t>(donorIndex)].assignmentCode = 'E';
    }
  }
  for (int index = 1; index < count - 1; ++index)
  {
    if (states[static_cast<size_t>(index)].assignmentCode == 'C' &&
        states[static_cast<size_t>(index - 1)].assignmentCode == 'E' &&
        states[static_cast<size_t>(index + 1)].assignmentCode == 'E' &&
        sheetProbability(states[static_cast<size_t>(index)]) > kSheetBridgeProbability)
    {
      states[static_cast<size_t>(index)].assignmentCode = 'E';
    }
  }
}

void filterShortSegments(std::vector<BackboneResidueState> &states)
{
  int index = 0;
  const int count = static_cast<int>(states.size());
  while (index < count)
  {
    const char code = states[static_cast<size_t>(index)].assignmentCode;
    int end = index + 1;
    while (end < count && states[static_cast<size_t>(end)].assignmentCode == code)
    {
      ++end;
    }
    if (code != 'C' && end - index < kMinimumSegmentLength)
    {
      for (int position = index; position < end; ++position)
      {
        states[static_cast<size_t>(position)].assignmentCode = 'C';
      }
    }
    index = end;
  }
}

} // namespace

std::vector<SKSecondaryStructureType> SKStrideSecondaryStructureAssigner::assign(const SKStrideBackboneChain &chain)
{
  std::vector<SKStrideBackboneResidue> residues;
  for (const SKStrideBackboneResidue &residue : chain.residues)
  {
    if (residue.alphaCarbon.has_value())
    {
      residues.push_back(residue);
    }
  }
  if (residues.size() < 2)
  {
    return {};
  }
  std::vector<BackboneResidueState> states = buildStates(residues);
  computeBackboneAngles(states);
  placeBackboneHydrogens(states, residues);
  const std::vector<HydrogenBond> hydrogenBonds = findMainChainHydrogenBonds(states);
  assignHelices(states, hydrogenBonds);
  assignSheets(states, hydrogenBonds);
  filterShortSegments(states);
  std::vector<SKSecondaryStructureType> result;
  result.reserve(states.size());
  for (const BackboneResidueState &state : states)
  {
    result.push_back(structureTypeForCode(state.assignmentCode));
  }
  return result;
}
