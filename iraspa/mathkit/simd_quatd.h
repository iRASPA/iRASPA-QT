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
#include "double3.h"

union simd_quatd
{
  double v[4];
  struct {double ix, iy, iz, r;};

  simd_quatd(double ix, double iy, double iz, double r);
  simd_quatd(double real, double3 imag);
  simd_quatd(double3 EulerAngles);
  static simd_quatd smallRandomQuaternion(double angleRange);
  static simd_quatd fromAxisAngle(double angle, double3 axis);
  double3 EulerAngles();
  simd_quatd normalized();
  static simd_quatd random();
  static simd_quatd yaw(double angle);
  static simd_quatd pitch(double angle);
  static simd_quatd roll(double angle);
  simd_quatd operator +(const simd_quatd& right) const;
  simd_quatd operator /(const double& right) const;
  simd_quatd operator *(const simd_quatd& right) const;
  static const simd_quatd data120[120];
  static const simd_quatd data60[60];
  static const simd_quatd data600[600];
  static const simd_quatd data300[300];
  static const simd_quatd data1992[1992];
  static const double weights1992[1992];
  static const simd_quatd data360[360];
  static const double weights360[360];

  friend QDataStream &operator<<(QDataStream &, const simd_quatd &);
  friend QDataStream &operator>>(QDataStream &, simd_quatd &);
};

