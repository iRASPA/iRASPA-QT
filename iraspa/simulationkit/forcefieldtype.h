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

#include "mathkit.h"
#include <foundationkit.h>
#include <QDataStream>

class ForceFieldType
{
public:
  ForceFieldType();
  ForceFieldType(const ForceFieldType& forceFieldType);
  ForceFieldType& operator=(const ForceFieldType& rhs) = default;
  ForceFieldType(QString forceFieldStringIdentifier, qint64 atomicNumber, double2 potentialParameters, double mass, double userDefinedRadius, bool editable);
  ForceFieldType(QString forceFieldStringIdentifier, qint64 atomicNumber, qint64 sortIndex, double2 potentialParameters, double mass, double userDefinedRadius, bool editable, double charge = 0.0);
  QString forceFieldStringIdentifier() const {return _forceFieldStringIdentifier;}
  void setForceFieldStringIdentifier(QString forceFieldStringIdentifier) {_forceFieldStringIdentifier = forceFieldStringIdentifier;}
  double2 potentialParameters() const {return _potentialParameters;}
  qint64 atomicNumber() const {return _atomicNumber;}
  void setAtomicNumber(int atomicNumber) {_atomicNumber = atomicNumber;}
  qint64 sortIndex() const {return _sortIndex;}
  void setSortIndex(qint64 sortIndex) {_sortIndex = sortIndex;}
  double userDefinedRadius() const {return _userDefinedRadius;}
  double mass() const {return _mass;}
  double charge() const {return _charge;}
  void setCharge(double charge) {_charge = charge;}
  void setEpsilonPotentialParameter(double epsilon)  {_potentialParameters = double2(epsilon,_potentialParameters.y);}
  void setSigmaPotentialParameter(double sigma)  {_potentialParameters = double2(_potentialParameters.x, sigma);}
  void setMass(double mass) {_mass = mass;}
  void setUserDefinedRadius(double radius) {_userDefinedRadius = radius;}
  bool editable() const {return _editable;}
  void setEditable(bool value) {_editable = value;}
  bool isVisible() const {return _isVisible;}
  void setIsVisible(bool value) {_isVisible = value;}
private:
  qint64 _versionNumber{4};
  bool _editable = false;
  qint64 _atomicNumber{6};
  qint64 _sortIndex{6};
  QString _forceFieldStringIdentifier;
  double2 _potentialParameters = double2(0.0,0.0);
  double _mass = 0.0;
  double _userDefinedRadius = 0.0;
  double _charge = 0.0;
  bool _isVisible = true;

  friend QDataStream &operator<<(QDataStream &, const ForceFieldType &);
  friend QDataStream &operator>>(QDataStream &, ForceFieldType &);
};
