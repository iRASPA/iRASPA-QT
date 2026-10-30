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
#include "skseitzintegermatrix.h"

class SKStructure
{
public:
  enum class DataType {Uint8, Int8, Uint16, Int16, Uint32, Int32, Uint64, Int64, Float, Double};

  SKStructure(): cell(std::make_shared<SKCell>()) {};

  enum class Kind: qint64
  {
    none = -1, object = 0, structure = 1,
    crystal = 2, molecularCrystal = 3, molecule = 4, protein = 5, proteinCrystal = 6,
    proteinCrystalSolvent = 7, crystalSolvent = 8, molecularCrystalSolvent = 9,
    crystalEllipsoidPrimitive = 10, crystalCylinderPrimitive = 11, crystalPolygonalPrismPrimitive = 12,
    ellipsoidPrimitive = 13, cylinderPrimitive = 14, polygonalPrismPrimitive = 15,
    gridVolume = 16, RASPADensityVolume = 17, VTKDensityVolume = 18, VASPDensityVolume = 19, GaussianCubeVolume = 20,
    dna = 21, dnaCrystal = 22
  };

  enum class MaterialType: qint64
  {
    unspecified = 0,
    silica = 1,
    aluminosilicate = 2,
    aluminophosphate = 3,
    metallophosphate = 4,
    silicoaluminophosphate = 5,
    zeolite = 6,
    mof = 7,
    cof = 8,
    zif = 9,
    molecule = 10,
    protein = 11,
    dnaRna = 12,
    molecularCrystal = 13,
    carbon = 14,
    oxide = 15,
    hof = 16,
    paf = 17,
    pim = 18,
    polymer = 19,
    ionicLiquid = 20,
    clay = 21,
    perovskite = 22,
    alloy = 23,
    glass = 24
  };

  static QString materialTypeDisplayName(MaterialType type);
  static std::vector<QString> allMaterialTypeDisplayNames();
  static std::optional<MaterialType> materialTypeFromDisplayName(const QString &name);
  static bool usesAluminosilicateForceField(MaterialType type);
  static MaterialType inferMaterialType(const std::vector<int> &elementIdentifiers, Kind kind, const std::vector<QString> &names = {});
  void applyInferredMaterialType(const std::vector<QString> &extraNames = {}, std::optional<Kind> kindOverride = std::nullopt);

  Kind kind = Kind::crystal;
  MaterialType materialType = MaterialType::unspecified;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms;
  std::set<QString> unknownAtoms;

  std::optional<QString> displayName;
  std::shared_ptr<SKCell> cell;
  std::optional<int> spaceGroupHallNumber;
  std::optional<std::vector<SKSeitzIntegerMatrix>> cifSymmetryOperations = std::nullopt;
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

  int3 dimensions;
  double3 origin;
  double3 spacing;
  DataType dataType;
  std::pair<double,double> range;
  double average;
  double variance;
  QByteArray byteData;
};
