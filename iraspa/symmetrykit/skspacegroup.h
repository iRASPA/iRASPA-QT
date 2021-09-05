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

#include <vector>
#include <array>
#include <QString>
#include <optional>
#include "skpointsymmetryset.h"
#include "sksymmetryoperationset.h"
#include "skspacegroupsetting.h"
#include "skrotationalchangeofbasis.h"
#include "skspacegroupdatabase.h"

class SKSpaceGroup
{
public:
  SKSpaceGroup(int HallNumber);
  std::vector<double3> listOfSymmetricPositions(double3 pos);
  const SKSpaceGroupSetting &spaceGroupSetting() const {return _spaceGroupSetting;}

  static std::vector<QString> latticeTranslationStrings(int HallNumber);
  static QString inversionCenterString(int HallNumber);
  static std::optional<int> HallNumberFromHMString(QString inputString);
  static std::optional<int> HallNumberFromSpaceGroupNumber(int);
  static std::optional<int> HallNumber(QString inputString);
  static SKSymmetryOperationSet findSpaceGroupSymmetry(double3x3 unitCell, std::vector<std::tuple<double3, int, double>> reducedAtoms, std::vector<std::tuple<double3, int, double>> atoms, SKPointSymmetrySet latticeSymmetries, bool allowPartialOccupancies, double symmetryPrecision);
  static std::optional<int> findSpaceGroupGroup(double3x3 unitCell, std::vector<std::tuple<double3, int, double> > atoms, bool allowPartialOccupancies, double symmetryPrecision);
  static std::optional<std::pair<double3, SKRotationalChangeOfBasis>> matchSpaceGroup(int HallNumber, double3x3 lattice, Centring entering, std::vector<SKSeitzMatrix> seitzMatrices, double symmetryPrecision);
  static std::optional<double3> getOriginShift(int HallNumber, Centring centering, SKRotationalChangeOfBasis changeOfBasis, std::vector<SKSeitzMatrix> seitzMatrices, double symmetryPrecision);
private:
  SKSpaceGroupSetting _spaceGroupSetting = SKSpaceGroupDataBase::spaceGroupData[1];
  
  static bool matchSpacegroup(QString spaceSearchGroupString, QString storedSpaceGroupString);

  friend QDataStream &operator<<(QDataStream &, const SKSpaceGroup &);
  friend QDataStream &operator>>(QDataStream &, SKSpaceGroup &);
};
