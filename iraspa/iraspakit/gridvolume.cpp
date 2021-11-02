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

#include "gridvolume.h"
#include <symmetrykit.h>
#include <qmath.h>
#include <QRegularExpression>
#include <math.h>
#include <cmath>
#include "structure.h"
#include "primitive.h"

GridVolume::GridVolume(): Object()
{

}

std::shared_ptr<Object> GridVolume::shallowClone()
{
  return std::make_shared<GridVolume>(static_cast<const GridVolume&>(*this));
}

GridVolume::GridVolume(const std::shared_ptr<const Structure> structure): Object(structure)
{

}

GridVolume::GridVolume(const std::shared_ptr<const Primitive> primitive): Object(primitive)
{

}

GridVolume::GridVolume(const std::shared_ptr<const GridVolume> volume): Object(volume)
{

}


GridVolume::GridVolume(std::shared_ptr<SKStructure> frame)
{

}

//double3 GridVolume::aspectRatio()
//{
//  double max = std::max({_dimensions.x * _spacing.x, _dimensions.y*  _spacing.y, _dimensions.z * _spacing.z});
//  return _spacing/max;
//}

SKBoundingBox GridVolume::boundingBox() const
{
  return SKBoundingBox(double3(0,0,0), double3(_dimensions.x * _spacing.x,_dimensions.y * _spacing.y,_dimensions.z * _spacing.z));
}



void GridVolume::reComputeBoundingBox()
{
  //SKBoundingBox boundingBox = this->boundingBox();

  // store in the cell datastructure
  //_cell->setBoundingBox(boundingBox);
}


std::vector<RKInPerInstanceAttributesAtoms> GridVolume::renderUnitCellSpheres() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  std::vector<RKInPerInstanceAttributesAtoms> data = std::vector<RKInPerInstanceAttributesAtoms>();

  uint32_t asymmetricBondIndex=0;
  for(int k1=minimumReplicaX;k1<=maximumReplicaX+1;k1++)
  {
    for(int k2=minimumReplicaY;k2<=maximumReplicaY+1;k2++)
    {
      for(int k3=minimumReplicaZ;k3<=maximumReplicaZ+1;k3++)
      {
        double3 position = _cell->unitCell() * double3(k1,k2,k3); // + origin();
        float4 ambient = float4(1.0f,1.0f,1.0f,1.0f);
        float4 diffuse = float4(1.0f,1.0f,1.0f,1.0f);
        float4 specular = float4(1.0f,1.0f,1.0f,1.0f);
        float4 scale = float4(0.1f,0.1f,0.1f,1.0f);
        RKInPerInstanceAttributesAtoms sphere = RKInPerInstanceAttributesAtoms(float4(position,1.0), ambient, diffuse, specular, scale, asymmetricBondIndex);
        data.push_back(sphere);
      }
    }
    asymmetricBondIndex++;
  }

  return data;
}

std::vector<RKInPerInstanceAttributesBonds> GridVolume::renderUnitCellCylinders() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  std::vector<RKInPerInstanceAttributesBonds> data = std::vector<RKInPerInstanceAttributesBonds>();

  for(int k1=minimumReplicaX;k1<=maximumReplicaX+1;k1++)
  {
    for(int k2=minimumReplicaY;k2<=maximumReplicaY+1;k2++)
    {
      for(int k3=minimumReplicaZ;k3<=maximumReplicaZ+1;k3++)
      {
        if(k1<=maximumReplicaX)
        {
          double3 position1 = _cell->unitCell() * double3(k1,k2,k3); // + origin();
          double3 position2 = _cell->unitCell() * double3(k1+1,k2,k3); // + origin();
          float4 scale = float4(0.1f,1.0f,0.1f,1.0f);
          RKInPerInstanceAttributesBonds cylinder =
                  RKInPerInstanceAttributesBonds(float4(position1,1.0),
                                                 float4(position2,1.0),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 scale,0,0);
          data.push_back(cylinder);
        }

        if(k2<=maximumReplicaY)
        {
          double3 position1 = _cell->unitCell() * double3(k1,k2,k3); // + origin();
          double3 position2 = _cell->unitCell() * double3(k1,k2+1,k3); // + origin();
          float4 scale = float4(0.1f,1.0f,0.1f,1.0f);
          RKInPerInstanceAttributesBonds cylinder =
                  RKInPerInstanceAttributesBonds(float4(position1,1.0),
                                                 float4(position2,1.0),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 scale,0,0);
          data.push_back(cylinder);
        }

        if(k3<=maximumReplicaZ)
        {
          double3 position1 = _cell->unitCell() * double3(k1,k2,k3); // + origin();
          double3 position2 = _cell->unitCell() * double3(k1,k2,k3+1); // + origin();
          float4 scale = float4(0.1f,1.0f,0.1f,1.0f);
          RKInPerInstanceAttributesBonds cylinder =
                  RKInPerInstanceAttributesBonds(float4(position1,1.0),
                                                 float4(position2,1.0),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 scale,0,0);
          data.push_back(cylinder);
        }
      }
    }
  }

  return data;
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<GridVolume> &volume)
{
     qDebug() << "WRITE GRIDVOLUME";
  stream << volume->_versionNumber;

  stream << volume->_dimensions;
  stream << volume->_spacing;
  stream << volume->_range.first;
  stream << volume->_range.second;
  stream << volume->_data;
  qDebug() << "SIZE: " << volume->_data.size();

  stream << qint64(0x6f6b6195);

  // handle super class
  stream << std::static_pointer_cast<Object>(volume);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<GridVolume> &volume)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > volume->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "DensityVolume");
  }

  stream >> volume->_dimensions;
  stream >> volume->_spacing;
  stream >> volume->_range.first;
  stream >> volume->_range.second;
  stream >> volume->_data;

  qint64 magicNumber;
  stream >> magicNumber;
  if(magicNumber != qint64(0x6f6b6195))
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "GridVolume invalid magic-number");
  }

  std::shared_ptr<Object> object = std::static_pointer_cast<Object>(volume);
  stream >> object;

  return stream;
}
