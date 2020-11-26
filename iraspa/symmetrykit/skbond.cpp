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

#include "skbond.h"
#include "skasymmetricatom.h"
#include "skatomcopy.h"

SKBond::SKBond()
{

}

SKBond::SKBond(std::shared_ptr<SKAtomCopy> a, std::shared_ptr<SKAtomCopy> b, BoundaryType type)
{
  if(a->parent()->tag() < b->parent()->tag())
  {
      _atom1 = a;
      _atom2 = b;
  }
  else
  {
      _atom1 = b;
      _atom2 = a;
  }
  _boundaryType = type;
}

void SKBond::setAtoms(std::shared_ptr<SKAtomCopy> a, std::shared_ptr<SKAtomCopy> b)
{
  if(a->tag() < b->tag())
  {
      _atom1 = a;
      _atom2 = b;
  }
  else
  {
      _atom1 = b;
      _atom2 = a;
  }
}

bool SKBond::operator==(SKBond const& rhs) const
{
  return this == &rhs;
}

double SKBond::bondLength()
{
  return (this->atom1()->position()-this->atom2()->position()).length();
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<SKBond> &bond)
{
  stream << bond->atom1()->tag();
  stream << bond->atom2()->tag();
  stream << static_cast<typename std::underlying_type<SKBond::BoundaryType>::type>(bond->_boundaryType);
  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<SKBond> &bond)
{
  stream >> bond->_tag1;
  stream >> bond->_tag2;
  qint64 boundaryType;
  stream >> boundaryType;
  bond->_boundaryType = SKBond::BoundaryType(boundaryType);
  return stream;
}

