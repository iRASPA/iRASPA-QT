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

#include "skdsspsecondarystructureassigner.h"
#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kCoulombFactor = 0.084 * 332.0;
constexpr double kEnergyCutoff = -0.5;
constexpr double kEnergyMargin = 1.0;
constexpr double kHydrogenBondSearchCutoff = 5.0;
constexpr double kNitrogenHydrogenBondLength = 1.01;

struct ResidueState
{
  QString residueName;
  double3 nitrogen;
  double3 alphaCarbon;
  double3 carbonylCarbon;
  double3 carbonylOxygen;
  std::optional<double3> implicitHydrogen;
  bool isDonor = false;
};

double distance(const double3 &a, const double3 &b)
{
  return (a - b).length();
}

double3 normalizeVector(const double3 &vector)
{
  const double lengthValue = vector.length();
  if (lengthValue <= 1.0e-12)
  {
    return double3(0.0, 0.0, 0.0);
  }
  return vector / lengthValue;
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

std::vector<ResidueState> buildStates(const std::vector<SKStrideBackboneResidue> &residues)
{
  std::vector<ResidueState> states;
  states.reserve(residues.size());
  for (const SKStrideBackboneResidue &residue : residues)
  {
    if (!residue.nitrogen.has_value() || !residue.alphaCarbon.has_value() ||
        !residue.carbonylCarbon.has_value() || !residue.carbonylOxygen.has_value())
    {
      continue;
    }
    ResidueState state;
    state.residueName = residue.residueName;
    state.nitrogen = residue.nitrogen.value();
    state.alphaCarbon = residue.alphaCarbon.value();
    state.carbonylCarbon = residue.carbonylCarbon.value();
    state.carbonylOxygen = residue.carbonylOxygen.value();
    state.isDonor = residue.residueName.toUpper() != QStringLiteral("PRO");
    states.push_back(state);
  }

  for (size_t index = 1; index < states.size(); ++index)
  {
    if (!states[index].isDonor)
    {
      continue;
    }
    const double3 previousCarbon = states[index - 1].carbonylCarbon;
    const double3 nitrogen = states[index].nitrogen;
    const double3 alphaCarbon = states[index].alphaCarbon;
    const double3 carbonToNitrogen = normalizeVector(nitrogen - previousCarbon);
    const double3 alphaCarbonToNitrogen = normalizeVector(nitrogen - alphaCarbon);
    const double3 hydrogenDirection = normalizeVector(carbonToNitrogen + alphaCarbonToNitrogen);
    states[index].implicitHydrogen = nitrogen + kNitrogenHydrogenBondLength * hydrogenDirection;
  }
  return states;
}

std::vector<std::vector<double>> hydrogenBondMap(const std::vector<ResidueState> &states)
{
  const int count = static_cast<int>(states.size());
  std::vector<std::vector<double>> energyMap(static_cast<size_t>(count), std::vector<double>(static_cast<size_t>(count), 0.0));

  for (int donorIndex = 1; donorIndex < count; ++donorIndex)
  {
    if (!states[static_cast<size_t>(donorIndex)].isDonor || !states[static_cast<size_t>(donorIndex)].implicitHydrogen.has_value())
    {
      continue;
    }
    const double3 hydrogen = states[static_cast<size_t>(donorIndex)].implicitHydrogen.value();
    const double3 donorNitrogen = states[static_cast<size_t>(donorIndex)].nitrogen;

    for (int acceptorIndex = 0; acceptorIndex < count - 1; ++acceptorIndex)
    {
      if (std::abs(acceptorIndex - donorIndex) < 2)
      {
        continue;
      }
      const double3 acceptorOxygen = states[static_cast<size_t>(acceptorIndex)].carbonylOxygen;
      const double3 acceptorCarbon = states[static_cast<size_t>(acceptorIndex)].carbonylCarbon;
      const double nitrogenOxygenDistance = distance(donorNitrogen, acceptorOxygen);
      if (nitrogenOxygenDistance > kHydrogenBondSearchCutoff)
      {
        continue;
      }
      const double carbonHydrogenDistance = distance(acceptorCarbon, hydrogen);
      const double oxygenHydrogenDistance = distance(acceptorOxygen, hydrogen);
      const double carbonNitrogenDistance = distance(acceptorCarbon, donorNitrogen);
      if (carbonHydrogenDistance <= 1.0e-6 || oxygenHydrogenDistance <= 1.0e-6 || carbonNitrogenDistance <= 1.0e-6)
      {
        continue;
      }
      const double energy = kCoulombFactor * (1.0 / nitrogenOxygenDistance + 1.0 / carbonHydrogenDistance -
                                              1.0 / oxygenHydrogenDistance - 1.0 / carbonNitrogenDistance);
      energyMap[static_cast<size_t>(donorIndex)][static_cast<size_t>(acceptorIndex)] = energy;
    }
  }

  std::vector<std::vector<double>> transformed(static_cast<size_t>(count), std::vector<double>(static_cast<size_t>(count), 0.0));
  for (int rowIndex = 0; rowIndex < count; ++rowIndex)
  {
    for (int columnIndex = 0; columnIndex < count; ++columnIndex)
    {
      const double energy = energyMap[static_cast<size_t>(rowIndex)][static_cast<size_t>(columnIndex)];
      const double clipped = std::min(std::max(kEnergyCutoff - kEnergyMargin - energy, -kEnergyMargin), kEnergyMargin);
      transformed[static_cast<size_t>(rowIndex)][static_cast<size_t>(columnIndex)] =
          (std::sin(clipped / kEnergyMargin * kPi / 2.0) + 1.0) / 2.0;
    }
  }
  return transformed;
}

std::vector<bool> diagonalGreaterThanZero(const std::vector<std::vector<double>> &matrix, int offset)
{
  const int count = static_cast<int>(matrix.size());
  std::vector<bool> values;
  values.reserve(static_cast<size_t>(std::max(0, count - offset)));
  for (int index = 0; index < count - offset; ++index)
  {
    values.push_back(matrix[static_cast<size_t>(index)][static_cast<size_t>(index + offset)] > 0.0);
  }
  return values;
}

std::vector<bool> paddedPairProduct(const std::vector<bool> &values, int leadingPadding, int trailingPadding)
{
  std::vector<bool> padded(static_cast<size_t>(leadingPadding), false);
  if (values.size() >= 2)
  {
    for (size_t index = 0; index + 1 < values.size(); ++index)
    {
      padded.push_back(values[index] && values[index + 1]);
    }
  }
  padded.insert(padded.end(), static_cast<size_t>(trailingPadding), false);
  return padded;
}

std::vector<bool> shift(const std::vector<bool> &values, int amount)
{
  if (values.empty())
  {
    return {};
  }
  if (amount == 0)
  {
    return values;
  }
  if (amount > 0)
  {
    std::vector<bool> shifted(static_cast<size_t>(amount), false);
    shifted.insert(shifted.end(), values.begin(), values.end() - static_cast<size_t>(amount));
    return shifted;
  }
  const int positiveAmount = -amount;
  std::vector<bool> shifted(values.begin() + positiveAmount, values.end());
  shifted.insert(shifted.end(), static_cast<size_t>(positiveAmount), false);
  return shifted;
}

std::vector<bool> rollingSum(const std::vector<bool> &values, int window)
{
  if (values.empty())
  {
    return {};
  }
  std::vector<bool> summed = values;
  for (int offset = 1; offset < window; ++offset)
  {
    const std::vector<bool> shifted = shift(values, offset);
    for (size_t index = 0; index < summed.size(); ++index)
    {
      summed[index] = summed[index] || shifted[index];
    }
  }
  return summed;
}

std::vector<std::vector<std::vector<double>>> unfoldAlongRows(const std::vector<std::vector<double>> &matrix, int window)
{
  const int rowCount = static_cast<int>(matrix.size());
  if (rowCount < window)
  {
    return {};
  }
  std::vector<std::vector<std::vector<double>>> result;
  for (int startRow = 0; startRow <= rowCount - window; ++startRow)
  {
    std::vector<std::vector<double>> slab;
    for (int offset = 0; offset < window; ++offset)
    {
      slab.push_back(matrix[static_cast<size_t>(startRow + offset)]);
    }
    result.push_back(slab);
  }
  return result;
}

std::vector<std::vector<std::vector<double>>> unfoldAlongColumns(const std::vector<std::vector<double>> &matrix, int window)
{
  if (matrix.empty())
  {
    return {};
  }
  const int columnCount = static_cast<int>(matrix[0].size());
  if (columnCount < window)
  {
    return {};
  }
  std::vector<std::vector<std::vector<double>>> result;
  for (int startColumn = 0; startColumn <= columnCount - window; ++startColumn)
  {
    std::vector<std::vector<double>> block;
    for (const std::vector<double> &row : matrix)
    {
      std::vector<double> slice;
      for (int offset = 0; offset < window; ++offset)
      {
        slice.push_back(row[static_cast<size_t>(startColumn + offset)]);
      }
      block.push_back(slice);
    }
    result.push_back(block);
  }
  return result;
}

std::vector<std::vector<std::vector<std::vector<bool>>>> upsampleBooleanMap(const std::vector<std::vector<double>> &matrix, int window)
{
  const auto rowWindows = unfoldAlongRows(matrix, window);
  std::vector<std::vector<std::vector<std::vector<bool>>>> result;
  for (const auto &slab : rowWindows)
  {
    const auto columnWindows = unfoldAlongColumns(slab, window);
    std::vector<std::vector<std::vector<bool>>> converted;
    for (const auto &windowBlock : columnWindows)
    {
      std::vector<std::vector<bool>> boolBlock;
      for (const std::vector<double> &row : windowBlock)
      {
        std::vector<bool> boolRow;
        for (double value : row)
        {
          boolRow.push_back(value > 0.0);
        }
        boolBlock.push_back(boolRow);
      }
      converted.push_back(boolBlock);
    }
    result.push_back(converted);
  }
  return result;
}

std::vector<std::vector<bool>> padBridgeMap(const std::vector<std::vector<bool>> &map)
{
  const size_t columnCount = map.empty() ? 0 : map[0].size();
  std::vector<std::vector<bool>> padded;
  padded.push_back(std::vector<bool>(columnCount, false));
  padded.insert(padded.end(), map.begin(), map.end());
  padded.push_back(std::vector<bool>(columnCount, false));
  return padded;
}

std::vector<bool> detectBetaLadder(const std::vector<std::vector<double>> &hbmap)
{
  const int count = static_cast<int>(hbmap.size());
  if (count < 3)
  {
    return std::vector<bool>(static_cast<size_t>(count), false);
  }

  const auto unfolded = upsampleBooleanMap(hbmap, 3);
  const int rowCount = static_cast<int>(unfolded.size());
  const int columnCount = rowCount > 0 ? static_cast<int>(unfolded[0].size()) : 0;

  std::vector<std::vector<bool>> parallelBridge(static_cast<size_t>(rowCount), std::vector<bool>(static_cast<size_t>(columnCount), false));
  std::vector<std::vector<bool>> antiparallelBridge(static_cast<size_t>(rowCount), std::vector<bool>(static_cast<size_t>(columnCount), false));

  for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
  {
    for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex)
    {
      const std::vector<std::vector<bool>> &block = unfolded[static_cast<size_t>(rowIndex)][static_cast<size_t>(columnIndex)];
      if (block.size() != 3 || block[0].size() != 3)
      {
        continue;
      }
      std::vector<std::vector<bool>> reversedBlock = block;
      std::reverse(reversedBlock.begin(), reversedBlock.end());
      for (std::vector<bool> &row : reversedBlock)
      {
        std::reverse(row.begin(), row.end());
      }
      parallelBridge[static_cast<size_t>(rowIndex)][static_cast<size_t>(columnIndex)] =
          (block[0][1] && reversedBlock[1][2]) || (reversedBlock[0][1] && block[1][2]);
      antiparallelBridge[static_cast<size_t>(rowIndex)][static_cast<size_t>(columnIndex)] =
          (block[1][1] && reversedBlock[1][1]) || (block[0][2] && reversedBlock[0][2]);
    }
  }

  parallelBridge = padBridgeMap(parallelBridge);
  antiparallelBridge = padBridgeMap(antiparallelBridge);

  std::vector<bool> ladder(static_cast<size_t>(count), false);
  for (size_t rowIndex = 0; rowIndex < parallelBridge.size(); ++rowIndex)
  {
    bool hasBridge = false;
    for (size_t columnIndex = 0; columnIndex < parallelBridge[rowIndex].size(); ++columnIndex)
    {
      if (parallelBridge[rowIndex][columnIndex] || antiparallelBridge[rowIndex][columnIndex])
      {
        hasBridge = true;
        break;
      }
    }
    if (rowIndex < static_cast<size_t>(count))
    {
      ladder[rowIndex] = hasBridge;
    }
  }
  return ladder;
}

std::vector<char> assignSecondaryStructure(const std::vector<std::vector<double>> &hydrogenBondMapValues)
{
  const int count = static_cast<int>(hydrogenBondMapValues.size());
  std::vector<std::vector<double>> hbmap(static_cast<size_t>(count), std::vector<double>(static_cast<size_t>(count), 0.0));
  for (int rowIndex = 0; rowIndex < count; ++rowIndex)
  {
    for (int columnIndex = 0; columnIndex < count; ++columnIndex)
    {
      hbmap[static_cast<size_t>(rowIndex)][static_cast<size_t>(columnIndex)] =
          hydrogenBondMapValues[static_cast<size_t>(columnIndex)][static_cast<size_t>(rowIndex)];
    }
  }

  const std::vector<bool> turn3 = diagonalGreaterThanZero(hbmap, 3);
  const std::vector<bool> turn4 = diagonalGreaterThanZero(hbmap, 4);
  const std::vector<bool> turn5 = diagonalGreaterThanZero(hbmap, 5);

  std::vector<bool> h3 = paddedPairProduct(turn3, 1, 3);
  std::vector<bool> h4 = paddedPairProduct(turn4, 1, 4);
  std::vector<bool> h5 = paddedPairProduct(turn5, 1, 5);

  std::vector<bool> helix4 = rollingSum(h4, 4);
  const std::vector<bool> helix4Previous = shift(helix4, -1);
  for (size_t index = 0; index < h3.size(); ++index)
  {
    h3[index] = h3[index] && !helix4Previous[index] && !helix4[index];
    h5[index] = h5[index] && !helix4Previous[index] && !helix4[index];
  }

  const std::vector<bool> helix3 = rollingSum(h3, 3);
  helix4 = rollingSum(h4, 4);
  const std::vector<bool> helix5 = rollingSum(h5, 5);
  std::vector<bool> helix(h3.size());
  for (size_t index = 0; index < helix.size(); ++index)
  {
    helix[index] = helix3[index] || helix4[index] || helix5[index];
  }

  const std::vector<bool> strand = detectBetaLadder(hbmap);
  std::vector<char> labels(helix.size());
  for (size_t index = 0; index < labels.size(); ++index)
  {
    labels[index] = helix[index] ? 'H' : (strand[index] ? 'E' : '-');
  }
  return labels;
}

} // namespace

std::vector<SKSecondaryStructureType> SKDsspSecondaryStructureAssigner::assign(const SKStrideBackboneChain &chain)
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

  std::vector<SKSecondaryStructureType> assignments(sourceResidues.size(), SKSecondaryStructureType::coil);
  const std::vector<ResidueState> states = buildStates(sourceResidues);
  if (states.size() < 6)
  {
    return assignments;
  }

  const std::vector<std::vector<double>> bondMap = hydrogenBondMap(states);
  const std::vector<char> labels = assignSecondaryStructure(bondMap);

  size_t stateIndex = 0;
  for (size_t sourceIndex = 0; sourceIndex < sourceResidues.size(); ++sourceIndex)
  {
    if (!isCompleteBackbone(sourceResidues[sourceIndex]))
    {
      continue;
    }
    if (stateIndex < labels.size())
    {
      assignments[sourceIndex] = structureTypeForCode(labels[stateIndex]);
    }
    ++stateIndex;
  }
  return assignments;
}
