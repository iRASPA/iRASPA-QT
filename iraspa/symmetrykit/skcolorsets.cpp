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

#include "skcolorsets.h"
#include <iostream>

SKColorSets::SKColorSets(): _colorSets{SKColorSet(SKColorSet::ColorScheme::jmol), SKColorSet(SKColorSet::ColorScheme::rasmol_modern),
                                       SKColorSet(SKColorSet::ColorScheme::rasmol),SKColorSet(SKColorSet::ColorScheme::vesta)}
{

}

SKColorSet* SKColorSets::operator[] (QString name)
{
  for(SKColorSet& colorSet: _colorSets)
  {
    if(QString::compare(colorSet.displayName(), name, Qt::CaseInsensitive) == 0)
    {
      return &colorSet;
    }
  }

  return nullptr;
}

const SKColorSet* SKColorSets::operator[] (QString name) const
{
  for(const SKColorSet& colorSet: _colorSets)
  {
    if(QString::compare(colorSet.displayName(), name, Qt::CaseInsensitive) == 0)
    {
      return &colorSet;
    }
  }

  return nullptr;
}

QDataStream &operator<<(QDataStream& stream, const std::vector<SKColorSet>& val)
{
  stream << static_cast<int32_t>(val.size());
  for(const SKColorSet& singleVal : val)
    stream << singleVal;
  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::vector<SKColorSet>& val)
{
  int32_t vecSize;
  val.clear();
  stream >> vecSize;
  val.reserve(vecSize);
  SKColorSet tempVal;
  while(vecSize--)
  {
    stream >> tempVal;
    val.push_back(tempVal);
  }
  return stream;
}

QDataStream &operator<<(QDataStream &stream, const SKColorSets &colorSets)
{
  stream << colorSets._versionNumber;
  stream << colorSets._colorSets;
  return stream;
}

QDataStream &operator>>(QDataStream &stream, SKColorSets &colorSets)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > colorSets._versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "SKColorSets");
  }

  stream >> colorSets._colorSets;

  return stream;
}


