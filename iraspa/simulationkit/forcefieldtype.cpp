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

#include "forcefieldtype.h"


ForceFieldType::ForceFieldType()
{

}

ForceFieldType::ForceFieldType(const ForceFieldType& forceFieldType): _editable(forceFieldType._editable), _atomicNumber(forceFieldType._atomicNumber),
    _forceFieldStringIdentifier(forceFieldType._forceFieldStringIdentifier), _potentialParameters(forceFieldType._potentialParameters),
    _mass(forceFieldType._mass), _userDefinedRadius(forceFieldType._userDefinedRadius)
{

}

ForceFieldType::ForceFieldType(QString forceFieldStringIdentifier, qint64 atomicNumber, double2 potentialParameters, double mass, double userDefinedRadius, bool editable)
{
  _forceFieldStringIdentifier = forceFieldStringIdentifier;
  _editable = editable;
  _atomicNumber = atomicNumber;
  _potentialParameters = potentialParameters;
  _mass = mass;
  _userDefinedRadius = userDefinedRadius;
}

QDataStream &operator<<(QDataStream &stream, const ForceFieldType &forcefieldType)
{
  stream << forcefieldType._versionNumber;
  stream << forcefieldType._editable;
  stream << forcefieldType._atomicNumber;
  stream << forcefieldType._forceFieldStringIdentifier;
  stream << forcefieldType._potentialParameters;
  stream << forcefieldType._mass;
  stream << forcefieldType._userDefinedRadius;
  stream << forcefieldType._isVisible;
  return stream;
}

QDataStream &operator>>(QDataStream &stream, ForceFieldType &forcefieldType)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > forcefieldType._versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "ForceFieldType");
  }

  stream >> forcefieldType._editable;
  stream >> forcefieldType._atomicNumber;
  stream >> forcefieldType._forceFieldStringIdentifier;
  stream >> forcefieldType._potentialParameters;
  stream >> forcefieldType._mass;
  stream >> forcefieldType._userDefinedRadius;

  if(versionNumber >= 2)
  {
    stream >> forcefieldType._isVisible;
  }

  return stream;
}
