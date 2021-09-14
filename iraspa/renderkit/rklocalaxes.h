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

#pragma once

#include <cmath>
#include <QRect>
#include <QColor>
#include <QSize>
#include <vector>
#include <type_traits>
#include <foundationkit.h>
#include "mathkit.h"
#include "axessystemdefaultgeometry.h"

class RKLocalAxes
{
public:
  RKLocalAxes();

  enum class Style: qint64
  {
    defaultStyle = 0, defaultStyleRGB = 1, cylinder = 2, cylinderRGB = 3, multiple_values = 4
  };

  enum class Position: qint64
  {
    none = 0, origin = 1, originBoundingBox = 2, center = 3, centerBoundingBox = 4, multiple_values = 5
  };

  enum class ScalingType: qint64
  {
    absolute = 0, relative = 1, multiple_values = 2
  };
  RKLocalAxes::Style style() const {return _style;}
  void setStyle(RKLocalAxes::Style style) {_style = style;}
  RKLocalAxes::Position position() const {return _position;}
  void setPosition(RKLocalAxes::Position position) {_position = position;}
  RKLocalAxes::ScalingType scalingType() const {return _scalingType;}
  void setScalingType(RKLocalAxes::ScalingType scalingType) {_scalingType = scalingType;}

  double length() const {return _length;}
  void setLength(double length) {_length = length;}
  double width() const {return _width;}
  void setWidth(double width) {_width = width;}
  double3 offset() const {return _offset;}
  void setOffset(double3 offset) {_offset = offset;}
  double offsetX() const {return _offset.x;}
  void setOffsetX(double offsetX) {_offset.x = offsetX;}
  double offsetY() const {return _offset.y;}
  void setOffsetY(double offsetY) {_offset.y = offsetY;}
  double offsetZ() const {return _offset.z;}
  void setOffsetZ(double offsetZ) {_offset.z = offsetZ;}
private:
  qint64 _versionNumber{1};

  RKLocalAxes::Style _style  = RKLocalAxes::Style::defaultStyle;
  RKLocalAxes::Position _position  = RKLocalAxes::Position::none;
  RKLocalAxes::ScalingType _scalingType = RKLocalAxes::ScalingType::absolute;
  double3 _offset = double3(0,0,0);
  double _length = 5.0;
  double _width = 0.5;

  friend QDataStream &operator<<(QDataStream &, const RKLocalAxes &);
  friend QDataStream &operator>>(QDataStream &, RKLocalAxes &);
};
