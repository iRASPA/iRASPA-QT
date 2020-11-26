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

#include "int3.h"

int3 int3::operator + (const int3& right) const
{
  int3 a(0,0,0);

  a.x=x+right.x;
  a.y=y+right.y;
  a.z=z+right.z;

  return a;
}

int3 int3::operator - (const int3& right) const
{
  int3 a(0,0,0);

  a.x=x-right.x;
  a.y=y-right.y;
  a.z=z-right.z;

  return a;
}

int modulo(int a, int b)
{
  return a - b * int(floor(double(a) / double(b)));
}

int greatestCommonDivisor(int arg1, int arg2)
{
  int a = arg1;
  int b = arg2;
  while( b != 0 )
  {
    int tempa = b;
    int tempb = modulo(a, b);
    a = tempa;
    b = tempb;
  }
  return abs(a);
}

int3 greatestCommonDivisor(int3 a, int b)
{
  return int3(greatestCommonDivisor(a.x,b), greatestCommonDivisor(a.y,b), greatestCommonDivisor(a.z,b));
}

QDataStream &operator<<(QDataStream &stream, const int3 &v)
{
  stream << v.x;
  stream << v.y;
  stream << v.z;
  return stream;
}

QDataStream &operator>>(QDataStream &stream, int3 &v)
{
  stream >> v.x;
  stream >> v.y;
  stream >> v.z;
  return stream;
}
