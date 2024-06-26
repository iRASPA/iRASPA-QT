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

#include "documentdata.h"

DocumentData::DocumentData(): _projectData(std::make_shared<ProjectTreeController>())
{

}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<DocumentData>& data)
{
  stream << data->_versionNumber;
  stream << data->_projectData;
  stream << static_cast<qint64>(0x6f6b6179); // magic number 'okay' in hex
  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<DocumentData>& data)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > data->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "DocumentData");
  }

  stream >> data->_projectData;

  qint64 magicNumber;
  stream >> magicNumber;
  if(magicNumber != static_cast<qint64>(0x6f6b6179))
  {
    throw InconsistentArchiveException(__FILE__, __LINE__, "DocumentData");
  }
  qDebug() << "Magic number read correctly: " << magicNumber << static_cast<qint64>(0x6f6b6179);
  return stream;
}
