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

#include <map>
#include <QString>
#include <tuple>
#include <vector>
#include <set>
#include <optional>
#include <functional>
#include "forcefieldtype.h"
#include <iostream>



class ForceFieldSet
{
public:
  enum class ForceFieldSchemeOrder: qint64
  {
    elementOnly = 0, forceFieldFirst = 1, forceFieldOnly = 2, multiple_values = 3
  };

  static const QString defaultDisplayName;
  static const QString aluminosilicateDisplayName;
  static const QString zeoliteAtlasDisplayName;
  static const QString aluminosilicateZeoPlusPlusDisplayName;
  static constexpr double zeoliteAtlasOxygenSigma = 2.7;
  static const QString bridgingAluminumOxygenIdentifier;
  static constexpr double oxygenAluminumBondCutoff = 2.1;
  static const std::set<int> trivalentTAtomAtomicNumbers;

  ForceFieldSet();
  ForceFieldSet(QString name, const ForceFieldSet& forcefieldset, bool editable=false);
  QString displayName() const {return _displayName;}
  void setDisplayName(QString name) {_displayName = name;}

  ForceFieldType* operator[] (const QString name);
  const ForceFieldType* operator[] (const QString name) const;
  ForceFieldType& operator[] (size_t sortNumber) {return _atomTypeList[sortNumber];}
  const ForceFieldType& operator[] (size_t sortNumber) const {return _atomTypeList[sortNumber];}

  std::vector<ForceFieldType>& atomTypeList() {return _atomTypeList;}
  const std::vector<ForceFieldType>& atomTypeList() const {return _atomTypeList;}
  void insert(int index, ForceFieldType& forceFieldType) {_atomTypeList.insert(_atomTypeList.begin()+index, forceFieldType);}
  void duplicate(size_t index);
  bool editable() const {return _editable;}
  void setEditable(bool editable) {_editable = editable;}
  QString uniqueName(int atomicNumber);

  static bool isDefaultForceFieldType(const QString &uniqueForceFieldName);
  static std::optional<ForceFieldType> defaultType(const QString &symbol);
  static bool isAluminosilicateFamily(const QString &displayName);
  static ForceFieldSet predefined(const QString &name);
  static ForceFieldSet aluminosilicate();
  static ForceFieldSet zeoliteAtlas();
  static ForceFieldSet aluminosilicateZeoPlusPlus();
  static bool isBridgingAluminumOxygen(double3 oxygenFractional, const std::vector<double3> &tAtomFractionals, double3x3 unitCell);
  std::optional<ForceFieldType> resolvedType(const QString &forceFieldName, int atomicNumber) const;
private:
  qint64 _versionNumber{1};
  QString _displayName = "Default";
  bool _editable = false;
  std::vector<ForceFieldType> _atomTypeList{};

  static std::vector<ForceFieldType> _defaultForceField;
  static ForceFieldType cation(const QString &symbol, double charge);
  static ForceFieldType tAtom(const QString &symbol, int atomicNumber, double charge, double radius, double mass);
  static std::vector<ForceFieldType> makeAluminosilicateForceField();
  static std::vector<ForceFieldType> makeAluminosilicateDerivedForceField(const std::function<double2(const QString &, int)> &parameters);
  static double zeoPlusPlusRadius(const QString &symbol, int atomicNumber);

  friend QDataStream &operator<<(QDataStream &, const ForceFieldSet &);
  friend QDataStream &operator>>(QDataStream &, ForceFieldSet &);

  friend QDataStream &operator<<(QDataStream& stream, const std::vector<ForceFieldType>& val);
  friend QDataStream &operator>>(QDataStream& stream, std::vector<ForceFieldType>& val);
};

