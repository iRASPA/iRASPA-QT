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


#include <algorithm>
#include "rklocalaxes.h"

RKLocalAxes::RKLocalAxes()
{

}

QDataStream &operator<<(QDataStream &stream, const RKLocalAxes &axes)
{
  stream << axes._versionNumber;
  stream << static_cast<typename std::underlying_type<RKLocalAxes::Style>::type>(axes._style);
  stream << static_cast<typename std::underlying_type<RKLocalAxes::Position>::type>(axes._position);
  stream << static_cast<typename std::underlying_type<RKLocalAxes::ScalingType>::type>(axes._scalingType);

  stream << axes._offset;
  stream << axes._length;
  stream << axes._width;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, RKLocalAxes &axes)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > axes._versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "RKLocalAxes");
  }

  qint64 style;
  stream >> style;
  axes._style = RKLocalAxes::Style(style);
  qint64 position;
  stream >> position;
  axes._position = RKLocalAxes::Position(position);
  qint64 scalingType;
  stream >> scalingType;
  axes._scalingType = RKLocalAxes::ScalingType(scalingType);

  stream >> axes._offset;
  stream >> axes._length;
  stream >> axes._width;

  return stream;
}
