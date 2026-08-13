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

#include "skdsssecondarystructureassigner.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr int kBreakSize = 5;
constexpr int kMaxHydrogenBondPartners = 6;
constexpr double kConnectedAlphaCarbonDistance = 4.5;
constexpr int kMinimumBackboneSequenceSeparation = 3;

constexpr double kHelixPsiTarget = -48.0;
constexpr double kHelixPsiInclude = 55.0;
constexpr double kHelixPsiExclude = 85.0;
constexpr double kHelixPhiTarget = -57.0;
constexpr double kHelixPhiInclude = 55.0;
constexpr double kHelixPhiExclude = 85.0;
constexpr double kStrandPsiTarget = 124.0;
constexpr double kStrandPsiInclude = 40.0;
constexpr double kStrandPsiExclude = 90.0;
constexpr double kStrandPhiTarget = -129.0;
constexpr double kStrandPhiInclude = 40.0;
constexpr double kStrandPhiExclude = 100.0;

constexpr uint16_t kHelix3HBond = 0x0001;
constexpr uint16_t kHelix4HBond = 0x0002;
constexpr uint16_t kHelix5HBond = 0x0004;
constexpr uint16_t kGotPhiPsi = 0x0008;
constexpr uint16_t kPhiPsiHelix = 0x0010;
constexpr uint16_t kPhiPsiNotHelix = 0x0020;
constexpr uint16_t kPhiPsiStrand = 0x0040;
constexpr uint16_t kPhiPsiNotStrand = 0x0080;
constexpr uint16_t kAntiStrandSingleHB = 0x0100;
constexpr uint16_t kAntiStrandDoubleHB = 0x0200;
constexpr uint16_t kAntiStrandBulgeHB = 0x0400;
constexpr uint16_t kAntiStrandSkip = 0x0800;
constexpr uint16_t kParaStrandSingleHB = 0x1000;
constexpr uint16_t kParaStrandDoubleHB = 0x2000;
constexpr uint16_t kParaStrandSkip = 0x4000;
constexpr uint16_t kHelixHBond = kHelix3HBond | kHelix4HBond | kHelix5HBond;
constexpr uint16_t kAntiStrandHB = kAntiStrandSingleHB | kAntiStrandDoubleHB;
constexpr uint16_t kParaStrandHB = kParaStrandSingleHB | kParaStrandDoubleHB;

struct DssResidue
{
  bool isReal = false;
  QString residueName;
  std::optional<double3> nitrogen;
  std::optional<double3> alphaCarbon;
  std::optional<double3> carbonylCarbon;
  std::optional<double3> carbonylOxygen;
  std::optional<double3> implicitHydrogen;
  double phiDegrees = 0.0;
  double psiDegrees = 0.0;
  char assignment = 'L';
  uint16_t flags = 0;
  std::vector<int> acceptorPartners;
  std::vector<int> donorPartners;
};

struct HBondCriteria
{
  double maxAngle = 63.0;
  double maxDistAtMaxAngle = 3.2;
  double maxDistAtZero = 4.0;
  double powerA = 1.6;
  double powerB = 5.0;
  double coneCosine = 0.0;
  double factorA = 0.0;
  double factorB = 0.0;
  double cutoff = 0.0;

  HBondCriteria()
  {
    factorA = 0.5 / std::pow(maxAngle, powerA);
    factorB = 0.5 / std::pow(maxAngle, powerB);
    cutoff = std::max(maxDistAtMaxAngle, maxDistAtZero);
  }
};

double distance(const double3 &a, const double3 &b)
{
  return (a - b).length();
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
    return 0.0;
  }
  normal0 = normal0 / length0;
  normal1 = normal1 / length1;
  double scalar = double3::dot(normal0, normal1);
  scalar = std::min(std::max(scalar, -1.0 + 1.0e-6), 1.0 - 1.0e-6);
  const double absoluteAngle = std::acos(scalar) * 180.0 / kPi;
  return double3::dot(bond0, normal1) > 0.0 ? absoluteAngle : -absoluteAngle;
}

double wrappedDelta(double angle, double target)
{
  double delta = std::abs(angle - target);
  if (delta > 180.0)
  {
    delta = 360.0 - delta;
  }
  return delta;
}

SKSecondaryStructureType structureTypeForCode(char code)
{
  switch (code)
  {
  case 'H':
  case 'h':
  case 'G':
  case 'g':
  case 'I':
  case 'i':
    return SKSecondaryStructureType::helix;
  case 'S':
  case 's':
  case 'E':
  case 'e':
  case 'B':
  case 'b':
    return SKSecondaryStructureType::sheet;
  default:
    return SKSecondaryStructureType::coil;
  }
}

bool isCompleteBackbone(const SKStrideBackboneResidue &residue)
{
  return residue.nitrogen.has_value() && residue.alphaCarbon.has_value() &&
         residue.carbonylCarbon.has_value() && residue.carbonylOxygen.has_value();
}

bool needsChainBreak(const SKStrideBackboneResidue &previous, const SKStrideBackboneResidue &current)
{
  if (!previous.alphaCarbon.has_value() || !current.alphaCarbon.has_value())
  {
    return true;
  }
  return distance(previous.alphaCarbon.value(), current.alphaCarbon.value()) > kConnectedAlphaCarbonDistance;
}

void appendBreakPadding(std::vector<DssResidue> &residues, std::vector<std::optional<int>> &sourceIndices)
{
  for (int index = 0; index < kBreakSize; ++index)
  {
    residues.push_back(DssResidue());
    sourceIndices.push_back(std::nullopt);
  }
}

bool isProline(const DssResidue &residue)
{
  return residue.residueName.toUpper() == QStringLiteral("PRO");
}

void appendPartner(std::vector<int> &partners, int index)
{
  if (static_cast<int>(partners.size()) < kMaxHydrogenBondPartners &&
      std::find(partners.begin(), partners.end(), index) == partners.end())
  {
    partners.push_back(index);
  }
}

std::optional<double3> averageAcceptorPlane(const double3 &acceptorOxygen,
                                            const double3 &acceptorCarbon,
                                            const double3 &acceptorAlphaCarbon,
                                            const double3 &incoming)
{
  double3 average = double3::normalize(acceptorOxygen - acceptorCarbon);
  average += double3::normalize(acceptorOxygen - acceptorAlphaCarbon);
  const double averageLength = average.length();
  if (averageLength <= 0.1)
  {
    return std::nullopt;
  }
  double3 plane = average / averageLength;
  const double incomingLength = incoming.length();
  if (incomingLength > 1.0e-12)
  {
    const double3 incomingNormalized = incoming / incomingLength;
    if (std::abs(double3::dot(plane, incomingNormalized)) < 0.99)
    {
      double3 perpendicular = incomingNormalized - plane * double3::dot(incomingNormalized, plane);
      const double perpendicularLength = perpendicular.length();
      if (perpendicularLength > 1.0e-12)
      {
        const double3 adjusted = 0.333644 * plane + 0.942699 * (perpendicular / perpendicularLength);
        plane = double3::normalize(plane - adjusted);
      }
    }
  }
  return plane;
}

bool checkHydrogenBond(const double3 &donorNitrogen,
                       const double3 &hydrogen,
                       const double3 &acceptorOxygen,
                       const std::optional<double3> &acceptorPlane,
                       const HBondCriteria &criteria)
{
  const double3 donorToAcceptor = acceptorOxygen - donorNitrogen;
  const double3 donorToHydrogen = hydrogen - donorNitrogen;
  const double3 hydrogenToAcceptor = acceptorOxygen - hydrogen;
  const double3 normalizedHydrogenToAcceptor = double3::normalize(hydrogenToAcceptor);
  if (acceptorPlane.has_value() && double3::dot(normalizedHydrogenToAcceptor, acceptorPlane.value()) > (-criteria.coneCosine))
  {
    return false;
  }

  const double adhCosine = double3::dot(double3::normalize(donorToHydrogen), double3::normalize(donorToAcceptor));
  double angle = 0.0;
  if (adhCosine < 1.0 && adhCosine > 0.0)
  {
    angle = std::acos(std::min(std::max(adhCosine, -1.0), 1.0)) * 180.0 / kPi;
  }
  else if (adhCosine > 0.0)
  {
    angle = 0.0;
  }
  else
  {
    angle = 90.0;
  }
  if (angle > criteria.maxAngle)
  {
    return false;
  }
  const double curve = std::pow(angle, criteria.powerA) * criteria.factorA + std::pow(angle, criteria.powerB) * criteria.factorB;
  const double cutoff = criteria.maxDistAtMaxAngle * curve + criteria.maxDistAtZero * (1.0 - curve);
  return donorToAcceptor.length() <= cutoff;
}

void placeImplicitHydrogens(std::vector<DssResidue> &residues)
{
  for (size_t index = 1; index < residues.size(); ++index)
  {
    if (!residues[index].isReal || isProline(residues[index]) || !residues[index].nitrogen.has_value() ||
        !residues[index].alphaCarbon.has_value() || !residues[index - 1].carbonylCarbon.has_value())
    {
      continue;
    }
    double3 direction = double3::normalize(-(residues[index - 1].carbonylCarbon.value() - residues[index].nitrogen.value()));
    direction += double3::normalize(-(residues[index].alphaCarbon.value() - residues[index].nitrogen.value()));
    const double directionLength = direction.length();
    if (directionLength <= 1.0e-12)
    {
      continue;
    }
    residues[index].implicitHydrogen = residues[index].nitrogen.value() + direction / directionLength;
  }
}

void findHydrogenBonds(std::vector<DssResidue> &residues)
{
  const HBondCriteria criteria;
  const int count = static_cast<int>(residues.size());
  for (int acceptorIndex = 0; acceptorIndex < count; ++acceptorIndex)
  {
    if (!residues[static_cast<size_t>(acceptorIndex)].isReal || !residues[static_cast<size_t>(acceptorIndex)].carbonylOxygen.has_value() ||
        !residues[static_cast<size_t>(acceptorIndex)].carbonylCarbon.has_value() ||
        !residues[static_cast<size_t>(acceptorIndex)].alphaCarbon.has_value())
    {
      continue;
    }
    const double3 acceptorOxygen = residues[static_cast<size_t>(acceptorIndex)].carbonylOxygen.value();
    const double3 acceptorCarbon = residues[static_cast<size_t>(acceptorIndex)].carbonylCarbon.value();
    const double3 acceptorAlphaCarbon = residues[static_cast<size_t>(acceptorIndex)].alphaCarbon.value();

    for (int donorIndex = 0; donorIndex < count; ++donorIndex)
    {
      if (donorIndex == acceptorIndex || !residues[static_cast<size_t>(donorIndex)].isReal ||
          isProline(residues[static_cast<size_t>(donorIndex)]) ||
          std::abs(donorIndex - acceptorIndex) < kMinimumBackboneSequenceSeparation ||
          !residues[static_cast<size_t>(donorIndex)].nitrogen.has_value() ||
          !residues[static_cast<size_t>(donorIndex)].implicitHydrogen.has_value())
      {
        continue;
      }
      const double3 donorNitrogen = residues[static_cast<size_t>(donorIndex)].nitrogen.value();
      const double3 hydrogen = residues[static_cast<size_t>(donorIndex)].implicitHydrogen.value();
      if ((acceptorOxygen - donorNitrogen).length() > criteria.cutoff)
      {
        continue;
      }
      const std::optional<double3> acceptorPlane = averageAcceptorPlane(acceptorOxygen, acceptorCarbon, acceptorAlphaCarbon, acceptorOxygen - hydrogen);
      if (checkHydrogenBond(donorNitrogen, hydrogen, acceptorOxygen, acceptorPlane, criteria))
      {
        appendPartner(residues[static_cast<size_t>(acceptorIndex)].acceptorPartners, donorIndex);
        appendPartner(residues[static_cast<size_t>(donorIndex)].donorPartners, acceptorIndex);
      }
    }
  }
}

void computePhiPsiFlags(std::vector<DssResidue> &residues)
{
  const int count = static_cast<int>(residues.size());
  for (int index = 1; index < count; ++index)
  {
    if (!residues[static_cast<size_t>(index)].isReal || !residues[static_cast<size_t>(index - 1)].isReal ||
        !residues[static_cast<size_t>(index)].carbonylCarbon.has_value() ||
        !residues[static_cast<size_t>(index)].alphaCarbon.has_value() ||
        !residues[static_cast<size_t>(index)].nitrogen.has_value() ||
        !residues[static_cast<size_t>(index - 1)].carbonylCarbon.has_value())
    {
      continue;
    }
    residues[static_cast<size_t>(index)].phiDegrees = torsionAngle(residues[static_cast<size_t>(index)].carbonylCarbon.value(),
                                                                   residues[static_cast<size_t>(index)].alphaCarbon.value(),
                                                                   residues[static_cast<size_t>(index)].nitrogen.value(),
                                                                   residues[static_cast<size_t>(index - 1)].carbonylCarbon.value());
    if (index + 1 < count && residues[static_cast<size_t>(index + 1)].isReal &&
        residues[static_cast<size_t>(index + 1)].nitrogen.has_value())
    {
      residues[static_cast<size_t>(index)].psiDegrees = torsionAngle(residues[static_cast<size_t>(index + 1)].nitrogen.value(),
                                                                     residues[static_cast<size_t>(index)].carbonylCarbon.value(),
                                                                     residues[static_cast<size_t>(index)].alphaCarbon.value(),
                                                                     residues[static_cast<size_t>(index)].nitrogen.value());
    }
    residues[static_cast<size_t>(index)].flags |= kGotPhiPsi;

    const double helixPsiDelta = wrappedDelta(residues[static_cast<size_t>(index)].psiDegrees, kHelixPsiTarget);
    const double helixPhiDelta = wrappedDelta(residues[static_cast<size_t>(index)].phiDegrees, kHelixPhiTarget);
    const double strandPsiDelta = wrappedDelta(residues[static_cast<size_t>(index)].psiDegrees, kStrandPsiTarget);
    const double strandPhiDelta = wrappedDelta(residues[static_cast<size_t>(index)].phiDegrees, kStrandPhiTarget);

    if (helixPsiDelta > kHelixPsiExclude || helixPhiDelta > kHelixPhiExclude)
    {
      residues[static_cast<size_t>(index)].flags |= kPhiPsiNotHelix;
    }
    else if (helixPsiDelta < kHelixPsiInclude && helixPhiDelta < kHelixPhiInclude)
    {
      residues[static_cast<size_t>(index)].flags |= kPhiPsiHelix;
    }

    if (strandPsiDelta > kStrandPsiExclude || strandPhiDelta > kStrandPhiExclude)
    {
      residues[static_cast<size_t>(index)].flags |= kPhiPsiNotStrand;
    }
    else if (strandPsiDelta < kStrandPsiInclude && strandPhiDelta < kStrandPhiInclude)
    {
      residues[static_cast<size_t>(index)].flags |= kPhiPsiStrand;
    }
  }
}

void assignPatternFlags(std::vector<DssResidue> &residues)
{
  const int count = static_cast<int>(residues.size());
  for (int index = kBreakSize; index < count - kBreakSize; ++index)
  {
    if (!residues[static_cast<size_t>(index)].isReal)
    {
      continue;
    }
    for (int partner : residues[static_cast<size_t>(index)].acceptorPartners)
    {
      if (partner == index + 3) residues[static_cast<size_t>(index)].flags |= kHelix3HBond;
      if (partner == index + 4) residues[static_cast<size_t>(index)].flags |= kHelix4HBond;
      if (partner == index + 5) residues[static_cast<size_t>(index)].flags |= kHelix5HBond;
    }
    for (int partner : residues[static_cast<size_t>(index)].donorPartners)
    {
      if (partner == index - 3) residues[static_cast<size_t>(index)].flags |= kHelix3HBond;
      if (partner == index - 4) residues[static_cast<size_t>(index)].flags |= kHelix4HBond;
      if (partner == index - 5) residues[static_cast<size_t>(index)].flags |= kHelix5HBond;
    }

    for (int acceptorPartner : residues[static_cast<size_t>(index)].acceptorPartners)
    {
      if (!residues[static_cast<size_t>(acceptorPartner)].isReal)
      {
        continue;
      }
      for (int nestedPartner : residues[static_cast<size_t>(acceptorPartner)].acceptorPartners)
      {
        if (nestedPartner == index)
        {
          residues[static_cast<size_t>(index)].flags |= kAntiStrandDoubleHB;
          residues[static_cast<size_t>(acceptorPartner)].flags |= kAntiStrandDoubleHB;
        }
      }
      if (acceptorPartner + 1 < count && residues[static_cast<size_t>(acceptorPartner + 1)].isReal)
      {
        for (int nestedPartner : residues[static_cast<size_t>(acceptorPartner + 1)].acceptorPartners)
        {
          if (nestedPartner == index)
          {
            residues[static_cast<size_t>(index)].flags |= kAntiStrandDoubleHB;
            residues[static_cast<size_t>(acceptorPartner + 1)].flags |= kAntiStrandBulgeHB;
            residues[static_cast<size_t>(acceptorPartner)].flags |= kAntiStrandBulgeHB;
          }
        }
      }
    }

    if (index + 2 < count && residues[static_cast<size_t>(index + 1)].isReal && residues[static_cast<size_t>(index + 2)].isReal)
    {
      for (int acceptorPartner : residues[static_cast<size_t>(index)].acceptorPartners)
      {
        const int partnerIndex = acceptorPartner - 2;
        if (partnerIndex < 0 || !residues[static_cast<size_t>(partnerIndex)].isReal)
        {
          continue;
        }
        for (int nestedPartner : residues[static_cast<size_t>(partnerIndex)].acceptorPartners)
        {
          if (nestedPartner == index + 2)
          {
            residues[static_cast<size_t>(index)].flags |= kAntiStrandSingleHB;
            residues[static_cast<size_t>(index + 1)].flags |= kAntiStrandSkip;
            residues[static_cast<size_t>(index + 2)].flags |= kAntiStrandSingleHB;
            residues[static_cast<size_t>(partnerIndex)].flags |= kAntiStrandSingleHB;
            if (partnerIndex + 1 < count && residues[static_cast<size_t>(partnerIndex + 1)].isReal)
            {
              residues[static_cast<size_t>(partnerIndex + 1)].flags |= kAntiStrandSkip;
            }
            if (partnerIndex + 2 < count && residues[static_cast<size_t>(partnerIndex + 2)].isReal)
            {
              residues[static_cast<size_t>(partnerIndex + 2)].flags |= kAntiStrandSingleHB;
            }
          }
        }
      }

      for (int acceptorPartner : residues[static_cast<size_t>(index)].acceptorPartners)
      {
        if (!residues[static_cast<size_t>(acceptorPartner)].isReal)
        {
          continue;
        }
        for (int nestedPartner : residues[static_cast<size_t>(acceptorPartner)].acceptorPartners)
        {
          if (nestedPartner == index + 2)
          {
            residues[static_cast<size_t>(index)].flags |= kParaStrandSingleHB;
            residues[static_cast<size_t>(index + 1)].flags |= kParaStrandSkip;
            residues[static_cast<size_t>(index + 2)].flags |= kParaStrandSingleHB;
            residues[static_cast<size_t>(acceptorPartner)].flags |= kParaStrandDoubleHB;
          }
        }
      }
    }
  }
}

void assignHelices(std::vector<DssResidue> &residues)
{
  const int count = static_cast<int>(residues.size());
  for (int index = kBreakSize; index < count - kBreakSize; ++index)
  {
    if (!residues[static_cast<size_t>(index)].isReal)
    {
      continue;
    }
    if ((residues[static_cast<size_t>(index - 1)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiNotHelix) == 0)
    {
      residues[static_cast<size_t>(index)].assignment = 'H';
    }
    if ((residues[static_cast<size_t>(index - 2)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index - 1)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index - 1)].flags & kPhiPsiHelix) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiHelix) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kPhiPsiHelix) != 0 &&
        (residues[static_cast<size_t>(index + 2)].flags & kHelixHBond) != 0)
    {
      residues[static_cast<size_t>(index)].assignment = 'h';
    }
  }

  for (int index = kBreakSize; index < count - kBreakSize; ++index)
  {
    if (residues[static_cast<size_t>(index)].isReal && residues[static_cast<size_t>(index)].assignment == 'h')
    {
      residues[static_cast<size_t>(index)].flags |= kHelixHBond;
      residues[static_cast<size_t>(index)].assignment = 'H';
    }
  }

  for (int index = kBreakSize; index < count - kBreakSize; ++index)
  {
    if (!residues[static_cast<size_t>(index)].isReal)
    {
      continue;
    }
    if ((residues[static_cast<size_t>(index)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiHelix) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kPhiPsiHelix) != 0 &&
        (residues[static_cast<size_t>(index + 2)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index + 2)].flags & kPhiPsiHelix) != 0 &&
        residues[static_cast<size_t>(index + 1)].assignment == 'H')
    {
      residues[static_cast<size_t>(index)].assignment = 'H';
    }
    if ((residues[static_cast<size_t>(index)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiHelix) != 0 &&
        (residues[static_cast<size_t>(index - 1)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index - 1)].flags & kPhiPsiHelix) != 0 &&
        (residues[static_cast<size_t>(index - 2)].flags & kHelixHBond) != 0 &&
        (residues[static_cast<size_t>(index - 2)].flags & kPhiPsiHelix) != 0 &&
        residues[static_cast<size_t>(index - 1)].assignment == 'H')
    {
      residues[static_cast<size_t>(index)].assignment = 'H';
    }
  }
}

void assignSheets(std::vector<DssResidue> &residues)
{
  const int count = static_cast<int>(residues.size());
  for (int index = kBreakSize; index < count - kBreakSize; ++index)
  {
    if (!residues[static_cast<size_t>(index)].isReal)
    {
      continue;
    }
    if ((residues[static_cast<size_t>(index)].flags & kAntiStrandDoubleHB) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiNotStrand) == 0)
    {
      residues[static_cast<size_t>(index)].assignment = 'S';
    }
    if ((residues[static_cast<size_t>(index)].flags & kAntiStrandBulgeHB) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kAntiStrandBulgeHB) != 0)
    {
      residues[static_cast<size_t>(index)].assignment = 'S';
      residues[static_cast<size_t>(index + 1)].assignment = 'S';
    }
    if ((residues[static_cast<size_t>(index - 1)].flags & kAntiStrandDoubleHB) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kAntiStrandSkip) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiNotStrand) == 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kAntiStrandHB) != 0)
    {
      residues[static_cast<size_t>(index)].assignment = 'S';
    }
    if ((residues[static_cast<size_t>(index - 1)].flags & kAntiStrandHB) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kAntiStrandSkip) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiNotStrand) == 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kAntiStrandDoubleHB) != 0)
    {
      residues[static_cast<size_t>(index)].assignment = 'S';
    }
    if ((residues[static_cast<size_t>(index - 1)].flags & kAntiStrandHB) != 0 &&
        (residues[static_cast<size_t>(index - 1)].flags & kPhiPsiStrand) != 0 &&
        (residues[static_cast<size_t>(index - 1)].flags & kPhiPsiNotStrand) == 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiStrand) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiNotStrand) == 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kAntiStrandHB) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kPhiPsiStrand) != 0)
    {
      residues[static_cast<size_t>(index - 1)].assignment = 'S';
      residues[static_cast<size_t>(index)].assignment = 'S';
      residues[static_cast<size_t>(index + 1)].assignment = 'S';
    }
    if ((residues[static_cast<size_t>(index)].flags & kParaStrandDoubleHB) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiNotStrand) == 0)
    {
      residues[static_cast<size_t>(index)].assignment = 'S';
    }
    if ((residues[static_cast<size_t>(index - 1)].flags & kParaStrandDoubleHB) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kParaStrandSkip) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiNotStrand) == 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kParaStrandHB) != 0)
    {
      residues[static_cast<size_t>(index)].assignment = 'S';
    }
    if ((residues[static_cast<size_t>(index - 1)].flags & kParaStrandHB) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kParaStrandSkip) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiStrand) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kParaStrandHB) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kPhiPsiStrand) != 0)
    {
      residues[static_cast<size_t>(index - 1)].assignment = 'S';
      residues[static_cast<size_t>(index)].assignment = 'S';
      residues[static_cast<size_t>(index + 1)].assignment = 'S';
    }
    if ((residues[static_cast<size_t>(index - 1)].flags & kParaStrandHB) != 0 &&
        (residues[static_cast<size_t>(index - 1)].flags & kPhiPsiStrand) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kParaStrandSkip) != 0 &&
        (residues[static_cast<size_t>(index)].flags & kPhiPsiStrand) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kParaStrandHB) != 0 &&
        (residues[static_cast<size_t>(index + 1)].flags & kPhiPsiStrand) != 0)
    {
      residues[static_cast<size_t>(index - 1)].assignment = 'S';
      residues[static_cast<size_t>(index)].assignment = 'S';
      residues[static_cast<size_t>(index + 1)].assignment = 'S';
    }
  }
}

bool strandHasPartner(int index, const std::vector<DssResidue> &residues)
{
  const DssResidue &residue = residues[static_cast<size_t>(index)];
  const char assignment = residue.assignment;
  for (int partner : residue.acceptorPartners)
  {
    if (residues[static_cast<size_t>(partner)].assignment == assignment)
    {
      return true;
    }
  }
  for (int partner : residue.donorPartners)
  {
    if (residues[static_cast<size_t>(partner)].assignment == assignment)
    {
      return true;
    }
  }
  if ((residue.flags & kAntiStrandSkip) != 0 || (residue.flags & kParaStrandSkip) != 0)
  {
    if (index + 1 < static_cast<int>(residues.size()) && residues[static_cast<size_t>(index + 1)].assignment == assignment)
    {
      for (int partner : residues[static_cast<size_t>(index + 1)].acceptorPartners)
      {
        if (residues[static_cast<size_t>(partner)].assignment == assignment)
        {
          return true;
        }
      }
    }
    if (index > 0 && residues[static_cast<size_t>(index - 1)].assignment == assignment)
    {
      for (int partner : residues[static_cast<size_t>(index - 1)].donorPartners)
      {
        if (residues[static_cast<size_t>(partner)].assignment == assignment)
        {
          return true;
        }
      }
    }
  }
  return false;
}

void filterShortSegments(std::vector<DssResidue> &residues)
{
  const int count = static_cast<int>(residues.size());
  bool repeatFiltering = true;
  while (repeatFiltering)
  {
    repeatFiltering = false;
    for (int index = kBreakSize; index < count - kBreakSize; ++index)
    {
      if (!residues[static_cast<size_t>(index)].isReal)
      {
        continue;
      }
      if (residues[static_cast<size_t>(index)].assignment == 'S' &&
          residues[static_cast<size_t>(index + 1)].assignment == 'S' &&
          residues[static_cast<size_t>(index - 1)].assignment != 'S' &&
          residues[static_cast<size_t>(index + 2)].assignment != 'S')
      {
        residues[static_cast<size_t>(index)].assignment = 'L';
        residues[static_cast<size_t>(index + 1)].assignment = 'L';
        repeatFiltering = true;
      }
      if (residues[static_cast<size_t>(index)].assignment == 'H' &&
          residues[static_cast<size_t>(index + 1)].assignment == 'H' &&
          residues[static_cast<size_t>(index - 1)].assignment != 'H' &&
          residues[static_cast<size_t>(index + 2)].assignment != 'H')
      {
        residues[static_cast<size_t>(index)].assignment = 'L';
        residues[static_cast<size_t>(index + 1)].assignment = 'L';
        repeatFiltering = true;
      }
      if (residues[static_cast<size_t>(index)].assignment == 'S' &&
          residues[static_cast<size_t>(index - 1)].assignment != 'S' &&
          residues[static_cast<size_t>(index + 1)].assignment != 'S')
      {
        residues[static_cast<size_t>(index)].assignment = 'L';
        repeatFiltering = true;
      }
      if (residues[static_cast<size_t>(index)].assignment == 'H' &&
          residues[static_cast<size_t>(index - 1)].assignment != 'H' &&
          residues[static_cast<size_t>(index + 1)].assignment != 'H')
      {
        residues[static_cast<size_t>(index)].assignment = 'L';
        repeatFiltering = true;
      }
      if (residues[static_cast<size_t>(index)].assignment == 'S' &&
          (residues[static_cast<size_t>(index - 1)].assignment != 'S' || residues[static_cast<size_t>(index + 1)].assignment != 'S') &&
          !strandHasPartner(index, residues))
      {
        residues[static_cast<size_t>(index)].assignment = 'L';
        repeatFiltering = true;
      }
    }
  }
}

} // namespace

std::vector<SKSecondaryStructureType> SKDssSecondaryStructureAssigner::assign(const SKStrideBackboneChain &chain)
{
  std::vector<SKStrideBackboneResidue> sourceResidues;
  for (const SKStrideBackboneResidue &residue : chain.residues)
  {
    if (residue.alphaCarbon.has_value())
    {
      sourceResidues.push_back(residue);
    }
  }
  if (sourceResidues.empty())
  {
    return {};
  }

  std::vector<DssResidue> paddedResidues(static_cast<size_t>(kBreakSize));
  std::vector<std::optional<int>> sourceIndices(static_cast<size_t>(kBreakSize));
  std::optional<int> lastPackedSourceIndex;

  for (size_t index = 0; index < sourceResidues.size(); ++index)
  {
    const SKStrideBackboneResidue &residue = sourceResidues[index];
    if (!isCompleteBackbone(residue))
    {
      continue;
    }
    if (lastPackedSourceIndex.has_value() && needsChainBreak(sourceResidues[static_cast<size_t>(lastPackedSourceIndex.value())], residue))
    {
      appendBreakPadding(paddedResidues, sourceIndices);
    }
    DssResidue dssResidue;
    dssResidue.isReal = true;
    dssResidue.residueName = residue.residueName;
    dssResidue.nitrogen = residue.nitrogen;
    dssResidue.alphaCarbon = residue.alphaCarbon;
    dssResidue.carbonylCarbon = residue.carbonylCarbon;
    dssResidue.carbonylOxygen = residue.carbonylOxygen;
    paddedResidues.push_back(dssResidue);
    sourceIndices.push_back(static_cast<int>(index));
    lastPackedSourceIndex = static_cast<int>(index);
  }
  appendBreakPadding(paddedResidues, sourceIndices);

  if (paddedResidues.size() <= static_cast<size_t>(2 * kBreakSize))
  {
    return std::vector<SKSecondaryStructureType>(sourceResidues.size(), SKSecondaryStructureType::coil);
  }

  placeImplicitHydrogens(paddedResidues);
  findHydrogenBonds(paddedResidues);
  computePhiPsiFlags(paddedResidues);

  for (size_t index = static_cast<size_t>(kBreakSize); index + static_cast<size_t>(kBreakSize) < paddedResidues.size(); ++index)
  {
    if (paddedResidues[index].isReal)
    {
      paddedResidues[index].assignment = 'L';
    }
  }

  assignPatternFlags(paddedResidues);
  assignHelices(paddedResidues);
  assignSheets(paddedResidues);
  filterShortSegments(paddedResidues);

  std::vector<SKSecondaryStructureType> assignments(sourceResidues.size(), SKSecondaryStructureType::coil);
  for (size_t paddedIndex = 0; paddedIndex < sourceIndices.size(); ++paddedIndex)
  {
    if (!sourceIndices[paddedIndex].has_value() || !paddedResidues[paddedIndex].isReal)
    {
      continue;
    }
    assignments[static_cast<size_t>(sourceIndices[paddedIndex].value())] = structureTypeForCode(paddedResidues[paddedIndex].assignment);
  }
  return assignments;
}
