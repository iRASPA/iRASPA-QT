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

#include "sksegnosecondarystructureassigner.h"
#include "sksecondarystructuregeometry.h"
#include <vector>

namespace {

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

bool betaRamachandran(double phi, double psi)
{
  const double referencePhi = phi < 0.0 ? phi + 360.0 : phi;
  const double referencePsi = psi < 0.0 ? psi + 360.0 : psi;
  return SKSecondaryStructureGeometry::inRange(referencePhi, 170.0, 290.0) &&
         SKSecondaryStructureGeometry::inRange(referencePsi, 60.0, 185.0);
}

double peptidePlaneDihedral(int firstIndex, int secondIndex, const std::vector<SKStrideBackboneResidue> &residues)
{
  if (firstIndex < 0 || secondIndex >= static_cast<int>(residues.size()))
  {
    return 0.0;
  }
  if (!residues[static_cast<size_t>(firstIndex)].alphaCarbon.has_value() ||
      !residues[static_cast<size_t>(firstIndex)].carbonylCarbon.has_value() ||
      !residues[static_cast<size_t>(secondIndex)].nitrogen.has_value() ||
      !residues[static_cast<size_t>(secondIndex)].alphaCarbon.has_value())
  {
    return 0.0;
  }
  return SKSecondaryStructureGeometry::torsionAngle(residues[static_cast<size_t>(firstIndex)].alphaCarbon.value(),
                                                    residues[static_cast<size_t>(firstIndex)].carbonylCarbon.value(),
                                                    residues[static_cast<size_t>(secondIndex)].nitrogen.value(),
                                                    residues[static_cast<size_t>(secondIndex)].alphaCarbon.value());
}

double carbonylPlaneDihedral(int currentIndex, const std::vector<SKStrideBackboneResidue> &residues)
{
  const int previousIndex = currentIndex - 1;
  if (previousIndex < 0)
  {
    return 0.0;
  }
  if (!residues[static_cast<size_t>(previousIndex)].carbonylCarbon.has_value() ||
      !residues[static_cast<size_t>(previousIndex)].carbonylOxygen.has_value() ||
      !residues[static_cast<size_t>(currentIndex)].carbonylCarbon.has_value() ||
      !residues[static_cast<size_t>(currentIndex)].carbonylOxygen.has_value())
  {
    return 0.0;
  }
  return SKSecondaryStructureGeometry::torsionAngle(residues[static_cast<size_t>(previousIndex)].carbonylCarbon.value(),
                                                    residues[static_cast<size_t>(previousIndex)].carbonylOxygen.value(),
                                                    residues[static_cast<size_t>(currentIndex)].carbonylCarbon.value(),
                                                    residues[static_cast<size_t>(currentIndex)].carbonylOxygen.value());
}

bool forwardBetaPair(int currentIndex, const std::vector<SKStrideBackboneResidue> &residues,
                     const std::vector<SKSecondaryStructureGeometry::PhiPsiDegrees> &angles)
{
  const int nextIndex = currentIndex + 1;
  if (nextIndex >= static_cast<int>(residues.size()))
  {
    return false;
  }
  const double omega1 = peptidePlaneDihedral(currentIndex, nextIndex, residues);
  if (!SKSecondaryStructureGeometry::inWrappedRange(omega1, 123.0, 210.0))
  {
    return false;
  }
  return betaRamachandran(angles[static_cast<size_t>(nextIndex)].phi, angles[static_cast<size_t>(currentIndex)].psi);
}

bool backwardBetaPair(int currentIndex, const std::vector<SKStrideBackboneResidue> &residues,
                      const std::vector<SKSecondaryStructureGeometry::PhiPsiDegrees> &angles)
{
  const int previousIndex = currentIndex - 1;
  if (previousIndex < 0)
  {
    return false;
  }
  if (!residues[static_cast<size_t>(previousIndex)].nitrogen.has_value() ||
      !residues[static_cast<size_t>(previousIndex)].alphaCarbon.has_value() ||
      !residues[static_cast<size_t>(previousIndex)].carbonylCarbon.has_value() ||
      !residues[static_cast<size_t>(currentIndex)].nitrogen.has_value() ||
      !residues[static_cast<size_t>(currentIndex)].alphaCarbon.has_value() ||
      !residues[static_cast<size_t>(currentIndex)].carbonylCarbon.has_value())
  {
    return false;
  }
  const double acutePlaneAngle = SKSecondaryStructureGeometry::acutePeptidePlaneAngle(
      residues[static_cast<size_t>(previousIndex)].nitrogen.value(),
      residues[static_cast<size_t>(previousIndex)].alphaCarbon.value(),
      residues[static_cast<size_t>(previousIndex)].carbonylCarbon.value(),
      residues[static_cast<size_t>(currentIndex)].nitrogen.value(),
      residues[static_cast<size_t>(currentIndex)].alphaCarbon.value(),
      residues[static_cast<size_t>(currentIndex)].carbonylCarbon.value());
  const double carbonylDihedral = carbonylPlaneDihedral(currentIndex, residues);
  if (!(acutePlaneAngle < 80.0 && SKSecondaryStructureGeometry::inWrappedRange(carbonylDihedral, 125.0, 210.0)))
  {
    return false;
  }
  return betaRamachandran(angles[static_cast<size_t>(currentIndex)].phi, angles[static_cast<size_t>(currentIndex)].psi);
}

std::vector<bool> helixMask(const std::vector<SKStrideBackboneResidue> &residues,
                            const std::vector<double3> &positions,
                            const std::vector<SKSecondaryStructureGeometry::PhiPsiDegrees> &angles)
{
  std::vector<bool> mask(residues.size(), false);
  for (size_t index = 0; index < residues.size(); ++index)
  {
    const std::optional<SKSecondaryStructureGeometry::LocalAxisFrame> frame =
        SKSecondaryStructureGeometry::localAxisFrame(static_cast<int>(index), positions);
    if (!frame.has_value() || !residues[index].alphaCarbon.has_value())
    {
      continue;
    }
    const double radius = SKSecondaryStructureGeometry::axisRadius(residues[index].alphaCarbon.value(), frame.value());
    const double tau = SKSecondaryStructureGeometry::axisTau(residues[index].alphaCarbon.value(), frame.value());
    const double phi = angles[index].phi;
    const double psi = angles[index].psi;
    const bool tauValid = SKSecondaryStructureGeometry::inRange(tau, 68.0, 130.0);
    mask[index] = SKSecondaryStructureGeometry::inRange(radius, 1.7, 3.0) && tauValid &&
                  SKSecondaryStructureGeometry::inRange(phi, -95.0, -35.0) &&
                  SKSecondaryStructureGeometry::inRange(psi, -70.0, -10.0);
  }
  return SKSecondaryStructureGeometry::maskConsecutive(mask, 4);
}

std::vector<bool> sheetMask(const std::vector<SKStrideBackboneResidue> &residues,
                            const std::vector<double3> &positions,
                            const std::vector<SKSecondaryStructureGeometry::PhiPsiDegrees> &angles)
{
  std::vector<bool> mask(residues.size(), false);
  for (size_t index = 0; index < residues.size(); ++index)
  {
    const std::optional<SKSecondaryStructureGeometry::LocalAxisFrame> frame =
        SKSecondaryStructureGeometry::localAxisFrame(static_cast<int>(index), positions);
    if (!frame.has_value() || !residues[index].alphaCarbon.has_value())
    {
      continue;
    }
    const double tau = SKSecondaryStructureGeometry::axisTau(residues[index].alphaCarbon.value(), frame.value());
    if (!(tau > 110.0))
    {
      continue;
    }
    const bool pairedWithNext = index + 1 < residues.size() &&
                                forwardBetaPair(static_cast<int>(index), residues, angles);
    const bool pairedWithPrevious = index > 0 &&
                                    backwardBetaPair(static_cast<int>(index), residues, angles);
    mask[index] = pairedWithNext || pairedWithPrevious;
  }
  return SKSecondaryStructureGeometry::maskConsecutive(mask, 3);
}

} // namespace

std::vector<SKSecondaryStructureType> SKSegnoSecondaryStructureAssigner::assign(const SKStrideBackboneChain &chain)
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

  std::vector<double3> positions;
  for (const SKStrideBackboneResidue &residue : residues)
  {
    if (residue.alphaCarbon.has_value())
    {
      positions.push_back(residue.alphaCarbon.value());
    }
  }
  const std::vector<SKSecondaryStructureGeometry::PhiPsiDegrees> angles = SKSecondaryStructureGeometry::phiPsiDegrees(chain);
  const std::vector<bool> helix = helixMask(residues, positions, angles);
  const std::vector<bool> sheet = sheetMask(residues, positions, angles);
  return mapToTypes(residues.size(), helix, sheet);
}
