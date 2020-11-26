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

#include <qdebug.h>
#include "skasymmetricbond.h"

SKAsymmetricBond::SKAsymmetricBond(std::shared_ptr<SKAsymmetricAtom> a, std::shared_ptr<SKAsymmetricAtom> b): _isVisible(true), _bondType(SKBondType::singleBond)
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

bool SKAsymmetricBond::operator==(SKAsymmetricBond const& rhs) const
{
  return (this->atom1().get() == rhs.atom1().get() && this->atom2().get() == rhs.atom2().get()) ||
         (this->atom1().get() == rhs.atom2().get() && this->atom2().get() == rhs.atom1().get());
}


QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<SKAsymmetricBond> &asymmetricBond)
{
  stream << asymmetricBond->atom1()->tag();
  stream << asymmetricBond->atom2()->tag();
  stream << asymmetricBond->_copies;
  stream << static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(asymmetricBond->_bondType);
  stream << asymmetricBond->_isVisible;
  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<SKAsymmetricBond> &asymmetricBond)
{
  stream >> asymmetricBond->_tag1;
  stream >> asymmetricBond->_tag2;
  stream >> asymmetricBond->_copies;
  qint64 bondType;
  stream >> bondType;
  asymmetricBond->_bondType = SKAsymmetricBond::SKBondType(bondType);
  stream >> asymmetricBond->_isVisible;

  return stream;
}
