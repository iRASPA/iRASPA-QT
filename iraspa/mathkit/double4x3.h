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

#include "double3.h"
#include "double4.h"

union double4x3
{
  double m[12];
  double mm[4][3];
  double3 v[4];
  struct {double m11, m21, m31,
                 m12, m22, m32,
                 m13, m23, m33,
                 m14, m24, m34;
  };
  struct {double ax, ay, az,
                 bx, by, bz,
                 cx, cy, cz,
                 wx, wy, wz;
  };

  double4x3(double3 v1, double3 v2, double3 v3, double3 v4): v{v1,v2,v3,v4} {}

  inline double3 & operator [] (int i) { return v[i]; }
  inline const double3 & operator [] (int i) const { return v[i]; }
};
