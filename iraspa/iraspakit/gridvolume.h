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

#pragma  once

#include <vector>
#include <tuple>
#include <mathkit.h>
#include <renderkit.h>
#include "object.h"
#include "infoviewer.h"

class GridVolume: public Object
{
public:
  GridVolume();
  GridVolume(std::shared_ptr<SKStructure> frame);

  GridVolume(const std::shared_ptr<const class Structure> structure);
  GridVolume(const std::shared_ptr<const class Primitive> primitive);
  GridVolume(const std::shared_ptr<const class GridVolume> volume);

  std::shared_ptr<Object> shallowClone() override final;
  ObjectType structureType() override  { return ObjectType::gridVolume; }

  SKBoundingBox boundingBox()  const override;
  void reComputeBoundingBox() final override;

  std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const override final;

protected:
  int3 _dimensions;
  double3 _spacing;
  double3 _aspectRatio;
  std::pair<double, double> _range;
  QByteArray _data;

  inline unsigned modulo( int value, unsigned m) {
      int mod = value % (int)m;
      if (mod < 0) {
          mod += m;
      }
      return mod;
  }

  inline float Images(const std::vector<float4> &data, size_t x,size_t y,size_t z)
  {
    size_t index = modulo(x,_dimensions.x)+modulo(y,_dimensions.y)*_dimensions.y+modulo(z,_dimensions.z)*_dimensions.x*_dimensions.y;

    return data[index].x;
  }

private:
  qint64 _versionNumber{1};

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<GridVolume> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<GridVolume> &);
};
