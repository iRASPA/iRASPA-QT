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

#pragma once

#include <QString>
#include <string>
#include <mathkit.h>
#include <vector>
#include <set>
#include <optional>
#include "skasymmetricatom.h"
#include "skcell.h"

class SKStructure
{
public:
  SKStructure(): cell(std::make_shared<SKCell>()) {};

  enum class Kind: qint64
  {
    unknown = 0, structure = 1,
    crystal = 2, molecularCrystal = 3, molecule = 4, protein = 5, proteinCrystal = 6,
    proteinCrystalSolvent = 7, crystalSolvent = 8, molecularCrystalSolvent = 9,
    crystalEllipsoidPrimitive = 10, crystalCylinderPrimitive = 11, crystalPolygonalPrismPrimitive = 12,
    ellipsoidPrimitive = 13, cylinderPrimitive = 14, polygonalPrismPrimitive = 15
  };

  Kind kind = Kind::crystal;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms;
  std::set<QString> unknownAtoms;

  std::optional<QString> displayName;
  std::shared_ptr<SKCell> cell;
  std::optional<int> spaceGroupHallNumber;
  bool drawUnitCell = false;
  bool periodic = false;

  std::optional<QString> creationDate;
  std::optional<QString> creationMethod;
  std::optional<QString> chemicalFormulaSum;
  std::optional<QString> chemicalFormulaStructural;
  std::optional<int> cellFormulaUnitsZ;

  std::optional<int> numberOfChannels;
  std::optional<int> numberOfPockets;
  std::optional<int> dimensionality;
  std::optional<double> Di;
  std::optional<double> Df;
  std::optional<double> Dif;
};
