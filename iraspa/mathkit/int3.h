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
#include <cstdlib>
#include <QDataStream>

union double3x3;
union double3;

union int3
{
  int32_t v[3];
  struct {int32_t x, y, z;};

  int3(): x(0), y(0), z(0) {}
  int3(int x,int y, int z): x(x), y(y), z(z) {}
  inline int & operator [] (int i) { return v[i]; }
  inline const int & operator [] (int i) const { return v[i]; }

  int3 operator + (const int3& right) const;
  int3 operator - (const int3& right) const;
  double3 operator *(const double3x3& right) const;

  friend QDataStream &operator<<(QDataStream &, const int3 &);
  friend QDataStream &operator>>(QDataStream &, int3 &);
};

int3 greatestCommonDivisor(int3 a, int b);

