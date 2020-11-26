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

#include "int3.h"

struct int3x3
{
  int m[9];
  int mm[3][3];
  int v[3];
  int denominator = 1;
  struct {int m11, m21, m31,     // 1st column
              m12, m22, m32,     // 2nd column
              m13, m23, m33;     // 3rd column
  };
  struct {int ax, ay, az,     // 1st column
              bx, by, bz,     // 2nd column
              cx, cy, cz;     // 3rd column
  };

  int3x3();
  int3x3(int value);
  int3x3(int3 v1, int3 v2, int3 v3);

  int3x3 operator * (const int3x3& right) const;
  int3 operator * (const int3& right) const;
  int3x3 operator-() const;
};
