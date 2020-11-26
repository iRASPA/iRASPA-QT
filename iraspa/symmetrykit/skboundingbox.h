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

#include <array>
#include <mathkit.h>
#include <foundationkit.h>

class SKBoundingBox
{
public:
  SKBoundingBox();
  SKBoundingBox(double3 minimum, double3 maximum);
  SKBoundingBox(const double3 center, const double3 width, const double scale);
  double3 widths() const;
  std::array<double3,8> const corners() const;
  std::array<std::pair<double3, double3>, 12> const sides() const;
  double3 center();
  double boundingSphereRadius();
  double3 maximum() const {return _maximum;}
  double3 minimum() const {return _minimum;}
  SKBoundingBox adjustForTransformation(double4x4 transformation);
  friend SKBoundingBox operator+(const SKBoundingBox left, const SKBoundingBox right);
  friend SKBoundingBox operator+(const SKBoundingBox left, double3 right);
  friend SKBoundingBox operator-(const SKBoundingBox left, double3 right);
private:
  qint64 _versionNumber{1};
  double3 _minimum = double3(0.0, 0.0, 0.0);
  double3 _maximum = double3(0.0, 0.0, 0.0);

  friend QDataStream &operator<<(QDataStream &, const SKBoundingBox &);
  friend QDataStream &operator>>(QDataStream &, SKBoundingBox &);
};
