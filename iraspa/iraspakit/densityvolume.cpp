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

#include "densityvolume.h"
#include <symmetrykit.h>
#include <qmath.h>
#include <QRegularExpression>
#include <math.h>
#include <cmath>

DensityVolume::DensityVolume()
{

}

//std::shared_ptr<Object> DensityVolume::clone()
//{
//  return std::make_shared<DensityVolume>(static_cast<const DensityVolume&>(*this));
//}


DensityVolume::DensityVolume(std::shared_ptr<SKStructure> frame)
{
  std::optional<QString> displayName = frame->displayName;
  if(displayName)
  {
    _displayName = *displayName;
  }
  _dimensions = frame->dimensions;
  _spacing = frame->spacing;
  _origin = frame->origin;
  _cell = frame->cell;

  auto *p = reinterpret_cast<uint16_t*>(frame->byteData.data());
  _data.clear();
  for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
  {
    _data.push_back(float4(float(p[i])/float(65535),0.0,0.0,0.0));
  }

  for(int z=0;z<_dimensions.z;z++)
  {
    for(int y=0;y<_dimensions.y;y++)
    {
      for(int x=0;x<_dimensions.x;x++)
      {
        float value = Images(x,y,z);

        // Sobel 3D gradient (finite difference leads to anisotropic gradients)
        float gx = Images(x+1,y+1,z+1)+2*Images(x,y+1,z+1)+Images(x-1,y+1,z+1) +2*Images(x+1,y+1,z)+4*Images(x,y+1,z)+ 2*Images(x-1,y+1,z) +
                   Images(x+1,y+1,z-1)+2*Images(x,y+1,z-1)+Images(x-1,y+1,z-1) - (Images(x+1,y-1,z+1)+2*Images(x,y-1,z+1)+Images(x-1,y-1,z+1) +
                   2*Images(x+1,y-1,z)+4*Images(x,y-1,z)+ 2*Images(x-1,y-1,z) +Images(x+1,y-1,z-1)+2*Images(x,y-1,z-1)+Images(x-1,y-1,z-1)) ;

        float gy = Images(x+1,y+1,z+1)+2*Images(x+1,y+1,z)+Images(x+1,y+1,z-1) +2*Images(x+1,y,z+1)+4*Images(x+1,y,z)+ 2*Images(x+1,y,z-1) +
                   Images(x+1,y-1,z+1)+2*Images(x+1,y-1,z)+Images(x+1,y-1,z-1) -(Images(x-1,y+1,z+1)+2*Images(x-1,y+1,z)+Images(x-1,y+1,z-1) +
                   2*Images(x-1,y,z+1)+4*Images(x-1,y,z)+ 2*Images(x-1,y,z-1) + Images(x-1,y-1,z+1)+2*Images(x-1,y-1,z)+Images(x-1,y-1,z-1));

        float gz = Images(x+1,y+1,z+1)+2*Images(x,y+1,z+1)+Images(x-1,y+1,z+1) +2*Images(x+1,y,z+1)+4*Images(x,y,z+1)+2*Images(x-1,y,z+1) +
                   Images(x+1,y-1,z+1)+2*Images(x,y-1,z+1)+Images(x-1,y-1,z+1) -(Images(x+1,y+1,z-1)+2*Images(x,y+1,z-1)+Images(x-1,y+1,z-1) +
                   2*Images(x+1,y,z-1)+4*Images(x,y,z-1)+ 2*Images(x-1,y,z-1) +Images(x+1,y-1,z-1)+2*Images(x,y-1,z-1)+Images(x-1,y-1,z-1));

        _data[x+y*_dimensions.y+z*_dimensions.x*_dimensions.y] = float4(value, gx, gy, gz);

      }
    }
  }
}

double3 DensityVolume::aspectRatio()
{
  double max = std::max({_dimensions.x * _spacing.x, _dimensions.y*  _spacing.y, _dimensions.z * _spacing.z});
  return _spacing/max;
}

SKBoundingBox DensityVolume::boundingBox() const
{
  return SKBoundingBox(double3(0,0,0), double3(_dimensions.x * _spacing.x,_dimensions.y * _spacing.y,_dimensions.z * _spacing.z));
}



void DensityVolume::reComputeBoundingBox()
{
  //SKBoundingBox boundingBox = this->boundingBox();

  // store in the cell datastructure
  //_cell->setBoundingBox(boundingBox);
}


std::vector<RKInPerInstanceAttributesAtoms> DensityVolume::renderUnitCellSpheres() const
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

std::vector<RKInPerInstanceAttributesBonds> DensityVolume::renderUnitCellCylinders() const
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

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<DensityVolume> &volume)
{
  stream << volume->_versionNumber;

  stream << volume->_dimensions;
  stream << volume->_spacing;
  stream << volume->_range;
 // stream << volume->_data;

  stream << volume->_authorFirstName;
  stream << volume->_authorMiddleName;
  stream << volume->_authorLastName;
  stream << volume->_authorOrchidID;
  stream << volume->_authorResearcherID;
  stream << volume->_authorAffiliationUniversityName;
  stream << volume->_authorAffiliationFacultyName;
  stream << volume->_authorAffiliationInstituteName;
  stream << volume->_authorAffiliationCityName;
  stream << volume->_authorAffiliationCountryName;

  // handle super class
  stream << std::static_pointer_cast<Object>(volume);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<DensityVolume> &volume)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > volume->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "DensityVolume");
  }

  stream >> volume->_dimensions;
  stream >> volume->_spacing;
  stream >> volume->_range;
  //stream >> volume->_data;

  stream >> volume->_authorFirstName;
  stream >> volume->_authorMiddleName;
  stream >> volume->_authorLastName;
  stream >> volume->_authorOrchidID;
  stream >> volume->_authorResearcherID;
  stream >> volume->_authorAffiliationUniversityName;
  stream >> volume->_authorAffiliationFacultyName;
  stream >> volume->_authorAffiliationInstituteName;
  stream >> volume->_authorAffiliationCityName;
  stream >> volume->_authorAffiliationCountryName;

  std::shared_ptr<Object> object = std::static_pointer_cast<Object>(volume);
  stream >> object;

  return stream;
}
