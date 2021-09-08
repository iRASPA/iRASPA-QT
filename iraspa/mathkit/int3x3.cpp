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

#include "int3x3.h"
#include "ring.h"
#include "double3x3.h"

int3x3::int3x3()
{

}

int3x3::int3x3(int value)
{
  m11 = value;
  m22 = value;
  m33 = value;
}

int3x3::int3x3(int3 v1, int3 v2, int3 v3)
{
  m11 = v1.x; m21 = v1.y; m31 = v1.z;
  m12 = v2.x; m22 = v2.y; m32 = v2.z;
  m13 = v3.x; m23 = v3.y; m33 = v3.z;
}

int3x3::int3x3(double3x3 m)
{
  m11 = int(rint(m.m11)); m21 = int(rint(m.m21)); m31 = int(rint(m.m31));
  m12 = int(rint(m.m12)); m22 = int(rint(m.m22)); m32 = int(rint(m.m32));
  m13 = int(rint(m.m13)); m23 = int(rint(m.m23)); m33 = int(rint(m.m33));
}


int int3x3::determinant(void) const
{
  int determinant =
   + m11 * (m22 * m33 - m23 * m32)
   - m12 * (m21 * m33 - m23 * m31)
   + m13 * (m21 * m32 - m22 * m31);

   return determinant;
}

int3x3 int3x3::adjugate() const
{
  int3x3 result{};
  result.m11 = this->m22 * this->m33 - this->m32 * this->m23;
  result.m12 = this->m13 * this->m32 - this->m12 * this->m33;
  result.m13 = this->m12 * this->m23 - this->m13 * this->m22;
  result.m21 = this->m23 * this->m31 - this->m21 * this->m33;
  result.m22 = this->m11 * this->m33 - this->m13 * this->m31;
  result.m23 = this->m21 * this->m13 - this->m11 * this->m23;
  result.m31 = this->m21 * this->m32 - this->m31 * this->m22;
  result.m32 = this->m31 * this->m12 - this->m11 * this->m32;
  result.m33 = this->m11 * this->m22 - this->m21 * this->m12;

  return result;
}

int int3x3::greatestCommonDivisor()
{
  std::vector<int> v1{m11,m12,m13,m21,m22,m23,m31,m32,m33};
  return std::accumulate(v1.begin(), v1.end(), 0, [&](double a, double b){return Ring::greatestCommonDivisor(a,b);});
}

int int3x3::trace() const
{
  return m11+m22+m33;
}


