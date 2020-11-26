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

#include "skatomcopy.h"

SKAtomCopy::SKAtomCopy(const SKAtomCopy &atomCopy)
{
  this->_position = atomCopy._position;
  this->_type = atomCopy._type;
  this->_tag = atomCopy._tag;
  this->_asymmetricIndex = atomCopy._asymmetricIndex;
  //this->_bonds = {};
}



QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<SKAtomCopy> &copy)
{
  stream << copy->_versionNumber;
  stream << copy->_position;
  stream << static_cast<typename std::underlying_type<SKAtomCopy::AtomCopyType>::type>(copy->_type);
  stream << copy->_tag;
  stream << copy->_asymmetricIndex;
  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<SKAtomCopy> &copy)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > copy->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "SKAsymmetricAtom");
  }

  stream >> copy->_position;
  qint64 type;
  stream >> type;
  copy->_type = SKAtomCopy::AtomCopyType(type);
  stream >> copy->_tag;
  stream >> copy->_asymmetricIndex;
  return stream;
}
