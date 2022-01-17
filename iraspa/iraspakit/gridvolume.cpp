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

GridVolume::GridVolume(const std::shared_ptr<Object> object): Object(object)
{

}

std::shared_ptr<Object> GridVolume::shallowClone()
{
  return std::make_shared<GridVolume>(static_cast<const GridVolume&>(*this));
}

GridVolume::GridVolume(std::shared_ptr<SKStructure> frame)
{

}

//double3 GridVolume::aspectRatio()
//{
//  double max = std::max({_dimensions.x * _spacing.x, _dimensions.y*  _spacing.y, _dimensions.z * _spacing.z});
//  return _spacing/max;
//}

// MARK: Bounding box
// =====================================================================

SKBoundingBox GridVolume::boundingBox() const
{
  double3 minimumReplica = _cell->minimumReplicas();
  double3 maximumReplica = _cell->maximumReplicas();

  double3 c0 = _cell->unitCell() * double3(minimumReplica);
  double3 c1 = _cell->unitCell() * double3(maximumReplica.x+1, minimumReplica.y,   minimumReplica.z);
  double3 c2 = _cell->unitCell() * double3(maximumReplica.x+1, maximumReplica.y+1, minimumReplica.z);
  double3 c3 = _cell->unitCell() * double3(minimumReplica.x,   maximumReplica.y+1, minimumReplica.z);
  double3 c4 = _cell->unitCell() * double3(minimumReplica.x,   minimumReplica.y,   maximumReplica.z+1);
  double3 c5 = _cell->unitCell() * double3(maximumReplica.x+1, minimumReplica.y,   maximumReplica.z+1);
  double3 c6 = _cell->unitCell() * double3(maximumReplica.x+1, maximumReplica.y+1, maximumReplica.z+1);
  double3 c7 = _cell->unitCell() * double3(minimumReplica.x,   maximumReplica.y+1, maximumReplica.z+1);

  double valuesX[8] = {c0.x, c1.x, c2.x, c3.x, c4.x, c5.x, c6.x, c7.x};
  double valuesY[8] = {c0.y, c1.y, c2.y, c3.y, c4.y, c5.y, c6.y, c7.y};
  double valuesZ[8] = {c0.z, c1.z, c2.z, c3.z, c4.z, c5.z, c6.z, c7.z};

  double3 minimum = double3(*std::min_element(valuesX,valuesX+8),
                            *std::min_element(valuesY,valuesY+8),
                            *std::min_element(valuesZ,valuesZ+8));

  double3 maximum = double3(*std::max_element(valuesX,valuesX+8),
                            *std::max_element(valuesY,valuesY+8),
                            *std::max_element(valuesZ,valuesZ+8));

  return SKBoundingBox(minimum,maximum);
}

void GridVolume::reComputeBoundingBox()
{
  SKBoundingBox boundingBox = this->boundingBox();

  // store in the cell datastructure
  _cell->setBoundingBox(boundingBox);
}

std::vector<RKInPerInstanceAttributesAtoms> GridVolume::renderUnitCellSpheres() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  double3 boundingBoxWidths = _cell->boundingBox().widths();
  double scaleFactor = 0.0025 * std::max({boundingBoxWidths.x,boundingBoxWidths.y,boundingBoxWidths.z});

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
        float4 scale = float4(scaleFactor,scaleFactor,scaleFactor,1.0f);
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

  double3 boundingBoxWidths = _cell->boundingBox().widths();
  double scaleFactor = 0.0025 * std::max({boundingBoxWidths.x,boundingBoxWidths.y,boundingBoxWidths.z});

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
          float4 scale = float4(scaleFactor,1.0f,scaleFactor,1.0f);
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
          float4 scale = float4(scaleFactor,1.0f,scaleFactor,1.0f);
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
          float4 scale = float4(scaleFactor,1.0f,scaleFactor,1.0f);
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

std::vector<float> GridVolume::gridData()
{
  int encompassingcubicsize = pow(2, _encompassingPowerOfTwoCubicGridSize);
  auto *p = reinterpret_cast<float*>(_data.data());
  std::vector<float> copiedData(encompassingcubicsize*encompassingcubicsize*encompassingcubicsize);
  for(int x=0;x<_dimensions.x;x++)
  {
    for(int y=0;y<_dimensions.y;y++)
    {
      for(int z=0;z<_dimensions.z;z++)
      {
        int index = int(x+encompassingcubicsize*y+z*encompassingcubicsize*encompassingcubicsize);
        copiedData[index] = p[x + _dimensions.x*y + z*_dimensions.x*_dimensions.y];
      }
    }
  }

  return copiedData;
}
std::vector<float4> GridVolume::gridValueAndGradientData()
{
   auto *p = reinterpret_cast<float*>(_data.data());
   std::vector<float> energyData(_dimensions.x*_dimensions.y*_dimensions.z);

   // normalize data
   for(int i=0;i<_dimensions.x*_dimensions.y*_dimensions.z;i++)
   {
     energyData[i] = (p[i] - _range.first) / (_range.second - _range.first);
   };

  int encompassingcubicsize = pow(2,_encompassingPowerOfTwoCubicGridSize);
  std::vector<float4> gradientData(encompassingcubicsize*encompassingcubicsize*encompassingcubicsize);

  for(int x=0;x<_dimensions.x;x++)
  {
    for(int y=0;y<_dimensions.y;y++)
    {
      for(int z=0;z<_dimensions.z;z++)
      {
        float value = energyData[x+_dimensions.x*y+z*_dimensions.x*_dimensions.y];

        // x
        int xi = (int)(x + 0.5f);
        float xf = x + 0.5f - xi;
        float xd0 = energyData[int(((xi-1 + _dimensions.x) % _dimensions.x)+y*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float xd1 = energyData[int((xi)+y*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float xd2 = energyData[int(((xi+1 + _dimensions.x) % _dimensions.x)+y*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float gx = (xd1 - xd0) * (1.0f - xf) + (xd2 - xd1) * xf; // lerp

        // y
        int yi = (int)(y + 0.5f);
        float yf = y + 0.5f - yi;
        float yd0 = energyData[int(x + ((yi-1+_dimensions.y) % _dimensions.y)*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float yd1 = energyData[int(x + (yi)*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float yd2 = energyData[int(x + ((yi+1+_dimensions.y) % _dimensions.y)*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float gy = (yd1 - yd0) * (1.0f - yf) + (yd2 - yd1) * yf; // lerp

        // z
        int zi = (int)(z + 0.5f);
        float zf = z + 0.5f - zi;
        float zd0 = energyData[int(x+y*_dimensions.x+((zi-1+_dimensions.z) % _dimensions.z)*_dimensions.x*_dimensions.y)];
        float zd1 = energyData[int(x+y*_dimensions.x+(zi)*_dimensions.x*_dimensions.y)];
        float zd2 = energyData[int(x+y*_dimensions.x+((zi+1+_dimensions.z) % _dimensions.z)*_dimensions.x*_dimensions.y)];
        float gz =  (zd1 - zd0) * (1.0f - zf) + (zd2 - zd1) * zf; // lerp

        int index = int(x+encompassingcubicsize*y+z*encompassingcubicsize*encompassingcubicsize);
        gradientData[index] = float4(value, gx, gy, gz);
      }
    }
  }
  return gradientData;
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<GridVolume> &volume)
{
  stream << volume->_versionNumber;

  stream << volume->_dimensions;
  stream << volume->_spacing;
  stream << volume->_range.first;
  stream << volume->_range.second;
  stream << volume->_data;

  stream << volume->_average;
  stream << volume->_variance;

  stream << volume->_drawAdsorptionSurface;

  stream << volume->_adsorptionSurfaceOpacity;
  stream << volume->_adsorptionTransparencyThreshold;
  stream << volume->_adsorptionSurfaceIsoValue;
  stream << volume->_encompassingPowerOfTwoCubicGridSize;

  stream << static_cast<typename std::underlying_type<ProbeMolecule>::type>(volume->_adsorptionSurfaceProbeMolecule);
  stream << static_cast<typename std::underlying_type<RKEnergySurfaceType>::type>(volume->_adsorptionSurfaceRenderingMethod);
  stream << static_cast<typename std::underlying_type<RKPredefinedVolumeRenderingTransferFunction>::type>(volume->_adsorptionVolumeTransferFunction);
  stream << volume->_adsorptionVolumeStepLength;

  stream << volume->_adsorptionSurfaceHue;
  stream << volume->_adsorptionSurfaceSaturation;
  stream << volume->_adsorptionSurfaceValue;

  stream << volume->_adsorptionSurfaceFrontSideHDR;
  stream << volume->_adsorptionSurfaceFrontSideHDRExposure;
  stream << volume->_adsorptionSurfaceFrontSideAmbientColor;
  stream << volume->_adsorptionSurfaceFrontSideDiffuseColor;
  stream << volume->_adsorptionSurfaceFrontSideSpecularColor;
  stream << volume->_adsorptionSurfaceFrontSideAmbientIntensity;
  stream << volume->_adsorptionSurfaceFrontSideDiffuseIntensity;
  stream << volume->_adsorptionSurfaceFrontSideSpecularIntensity;
  stream << volume->_adsorptionSurfaceFrontSideShininess;

  stream << volume->_adsorptionSurfaceBackSideHDR;
  stream << volume->_adsorptionSurfaceBackSideHDRExposure;
  stream << volume->_adsorptionSurfaceBackSideAmbientColor;
  stream << volume->_adsorptionSurfaceBackSideDiffuseColor;
  stream << volume->_adsorptionSurfaceBackSideSpecularColor;
  stream << volume->_adsorptionSurfaceBackSideAmbientIntensity;
  stream << volume->_adsorptionSurfaceBackSideDiffuseIntensity;
  stream << volume->_adsorptionSurfaceBackSideSpecularIntensity;
  stream << volume->_adsorptionSurfaceBackSideShininess;

  stream << qint64(0x6f6b6195);

  // handle super class
  stream << std::static_pointer_cast<Object>(volume);

  return stream;
}



QDataStream &operator>>(QDataStream &stream, std::shared_ptr<GridVolume> &volume)
{
     qDebug() << "BEGIN check";
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > volume->_versionNumber)
  {
    qDebug() << "version" << versionNumber << volume->_versionNumber;
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "GridVolume");
  }

  stream >> volume->_dimensions;
  stream >> volume->_spacing;
  stream >> volume->_range.first;
  stream >> volume->_range.second;
  stream >> volume->_data;

  if(versionNumber >= 2) // introduced in version 2
  {
    stream >> volume->_average;
    stream >> volume->_variance;

    stream >> volume->_drawAdsorptionSurface;

    stream >> volume->_adsorptionSurfaceOpacity;
    stream >> volume->_adsorptionTransparencyThreshold;
    stream >> volume->_adsorptionSurfaceIsoValue;
    stream >> volume->_encompassingPowerOfTwoCubicGridSize;

    qint64 adsorptionSurfaceProbeMolecule;
    stream >> adsorptionSurfaceProbeMolecule;
    volume->_adsorptionSurfaceProbeMolecule = ProbeMolecule(adsorptionSurfaceProbeMolecule);

    qint64 adsorptionSurfaceRenderingMethod;
    stream >> adsorptionSurfaceRenderingMethod;
    volume->_adsorptionSurfaceRenderingMethod = RKEnergySurfaceType(adsorptionSurfaceRenderingMethod);
    qint64 adsorptionVolumeTransferFunction;
    stream >> adsorptionVolumeTransferFunction;
    volume->_adsorptionVolumeTransferFunction = RKPredefinedVolumeRenderingTransferFunction(adsorptionVolumeTransferFunction);
    stream >> volume->_adsorptionVolumeStepLength;

    stream >> volume->_adsorptionSurfaceHue;
    stream >> volume->_adsorptionSurfaceSaturation;
    stream >> volume->_adsorptionSurfaceValue;

    stream >> volume->_adsorptionSurfaceFrontSideHDR;
    stream >> volume->_adsorptionSurfaceFrontSideHDRExposure;
    stream >> volume->_adsorptionSurfaceFrontSideAmbientColor;
    stream >> volume->_adsorptionSurfaceFrontSideDiffuseColor;
    stream >> volume->_adsorptionSurfaceFrontSideSpecularColor;
    stream >> volume->_adsorptionSurfaceFrontSideAmbientIntensity;
    stream >> volume->_adsorptionSurfaceFrontSideDiffuseIntensity;
    stream >> volume->_adsorptionSurfaceFrontSideSpecularIntensity;
    stream >> volume->_adsorptionSurfaceFrontSideShininess;

    stream >> volume->_adsorptionSurfaceBackSideHDR;
    stream >> volume->_adsorptionSurfaceBackSideHDRExposure;
    stream >> volume->_adsorptionSurfaceBackSideAmbientColor;
    stream >> volume->_adsorptionSurfaceBackSideDiffuseColor;
    stream >> volume->_adsorptionSurfaceBackSideSpecularColor;
    stream >> volume->_adsorptionSurfaceBackSideAmbientIntensity;
    stream >> volume->_adsorptionSurfaceBackSideDiffuseIntensity;
    stream >> volume->_adsorptionSurfaceBackSideSpecularIntensity;
    stream >> volume->_adsorptionSurfaceBackSideShininess;
  }

  qint64 magicNumber;
  stream >> magicNumber;
  if(magicNumber != qint64(0x6f6b6195))
  {
      qDebug() << "HERE check";
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "GridVolume invalid magic-number");
  }

  std::shared_ptr<Object> object = std::static_pointer_cast<Object>(volume);
  stream >> object;

  return stream;
}
