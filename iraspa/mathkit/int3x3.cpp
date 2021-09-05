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

int3x3 int3x3::operator * (const int3x3& b) const
{
  int3x3 r;


  r.m11 = m11 * b.m11 + m12 * b.m21 + m13 * b.m31;
  r.m21 = m21 * b.m11 + m22 * b.m21 + m23 * b.m31;
  r.m31 = m31 * b.m11 + m32 * b.m21 + m33 * b.m31;

  r.m12 = m11 * b.m12 + m12 * b.m22 + m13 * b.m32;
  r.m22 = m21 * b.m12 + m22 * b.m22 + m23 * b.m32;
  r.m32 = m31 * b.m12 + m32 * b.m22 + m33 * b.m32;

  r.m13 = m11 * b.m13 + m12 * b.m23 + m13 * b.m33;
  r.m23 = m21 * b.m13 + m22 * b.m23 + m23 * b.m33;
  r.m33 = m31 * b.m13 + m32 * b.m23 + m33 * b.m33;


  return r;
}

int3 int3x3::operator * (const int3& b) const
{
  int3 r(0,0,0);

  r.x = m11 * b.x + m12 * b.y + m13 * b.z;
  r.y = m21 * b.x + m22 * b.y + m23 * b.z;
  r.z = m31 * b.x + m32 * b.y + m33 * b.z;

  return r;
}

double3 int3x3::operator * (const double3& b) const
{
  double3 r(0,0,0);

  r.x = m11 * b.x + m12 * b.y + m13 * b.z;
  r.y = m21 * b.x + m22 * b.y + m23 * b.z;
  r.z = m31 * b.x + m32 * b.y + m33 * b.z;

  return r;
}

int3x3 int3x3::operator-() const
{
  int3x3 r;
  r.m11 = -m11; r.m21 = -m21; r.m31 = -m31;
  r.m12 = -m12; r.m22 = -m22; r.m32 = -m32;
  r.m13 = -m13; r.m23 = -m23; r.m33 = -m33;
  return r;
}

int3x3 int3x3::operator + (const int3x3& b) const
{
  int3x3 r;

  r.m11 = this->m11 + b.m11;  r.m21 = this->m21 + b.m21;  r.m31 = this->m31 + b.m31;
  r.m12 = this->m12 + b.m12;  r.m22 = this->m22 + b.m22;  r.m32 = this->m32 + b.m32;
  r.m13 = this->m13 + b.m13;  r.m23 = this->m23 + b.m23;  r.m33 = this->m33 + b.m33;

  return r;
}

int3x3 int3x3::operator / (const int& b) const
{
  int3x3 r;
  r.m11 = this->m11 / b; r.m21 = this->m21 / b; r.m31 = this->m31 / b;
  r.m12 = this->m12 / b; r.m22 = this->m22 / b; r.m32 = this->m32 / b;
  r.m13 = this->m13 / b; r.m23 = this->m23 / b; r.m33 = this->m33 / b;
  return r;
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
  result.mm[0][0] = this->mm[1][1] * this->mm[2][2] - this->mm[2][1] * this->mm[1][2];
  result.mm[0][1] = this->mm[0][2] * this->mm[2][1] - this->mm[0][1] * this->mm[2][2];
  result.mm[0][2] = this->mm[0][1] * this->mm[1][2] - this->mm[0][2] * this->mm[1][1];
  result.mm[1][0] = this->mm[1][2] * this->mm[2][0] - this->mm[1][0] * this->mm[2][2];
  result.mm[1][1] = this->mm[0][0] * this->mm[2][2] - this->mm[0][2] * this->mm[2][0];
  result.mm[1][2] = this->mm[1][0] * this->mm[0][2] - this->mm[0][0] * this->mm[1][2];
  result.mm[2][0] = this->mm[1][0] * this->mm[2][1] - this->mm[2][0] * this->mm[1][1];
  result.mm[2][1] = this->mm[2][0] * this->mm[0][1] - this->mm[0][0] * this->mm[2][1];
  result.mm[2][2] = this->mm[0][0] * this->mm[1][1] - this->mm[1][0] * this->mm[0][1];

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


