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
#include "forcefieldtype.h"
#include <iostream>



class ForceFieldSet
{
public:
  enum class ForceFieldSchemeOrder: qint64
  {
    elementOnly = 0, forceFieldFirst = 1, forceFieldOnly = 2, multiple_values = 3
  };

  ForceFieldSet();
  ForceFieldSet(QString name, ForceFieldSet& forcefieldset, bool editable=false);
  QString displayName() const {return _displayName;}

  ForceFieldType* operator[] (const QString name);
  ForceFieldType& operator[] (size_t sortNumber) {return _atomTypeList[sortNumber];}

  std::vector<ForceFieldType>& atomTypeList() {return _atomTypeList;}
  void insert(int index, ForceFieldType& forceFieldType) {_atomTypeList.insert(_atomTypeList.begin()+index, forceFieldType);}
  void duplicate(size_t index);
  bool editable() {return _editable;}
  QString uniqueName(int atomicNumber);
private:
  qint64 _versionNumber{1};
  QString _displayName = "Default";
  bool _editable = false;
  std::vector<ForceFieldType> _atomTypeList{};

  static std::vector<ForceFieldType> _defaultForceField;

  friend QDataStream &operator<<(QDataStream &, const ForceFieldSet &);
  friend QDataStream &operator>>(QDataStream &, ForceFieldSet &);
};

