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

#include "float3.h"

float3 float3::operator + (const float3& right) const
{
  float3 a;

  a.x=x+right.x;
  a.y=y+right.y;
  a.z=z+right.z;

  return a;
}

float3 float3::operator - (const float3& right) const
{
  float3 a;

  a.x=x-right.x;
  a.y=y-right.y;
  a.z=z-right.z;

  return a;
}


float3 float3::normalise()
{
  float magnitude = sqrt((x * x) + (y * y) + (z * z)) ;

  if (magnitude != 0)
  {
    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
  }
  return *this;
}

float3 float3::fract()
{
  float3 s = float3(x,y,z);
  s.x -= rint(x);
  s.y -= rint(y);
  s.z -= rint(z);

  if(s.x<0.0)
  {
    s.x += 1.0;
  }
  if(s.x>1.0)
  {
    s.x -= 1.0;
  }

  if(s.y<0.0)
  {
    s.y += 1.0;
  }
  if(s.y>1.0)
  {
    s.y -= 1.0;
  }

  if(s.z<0.0)
  {
    s.z += 1.0;
  }
  if(s.z>1.0)
  {
    s.z -= 1.0;
  }
  return s;
}

float float3::dot(const float3 &v1, const float3 &v2)
{
  return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
}

float3 float3::max(const float3 &v1, const float3 &v2)
{
  return float3(std::max(v1.x,v2.x),std::max(v1.y,v2.y),std::max(v1.z,v2.z));
}

float3 float3::min(const float3 &v1, const float3 &v2)
{
  return float3(std::min(v1.x,v2.x),std::min(v1.y,v2.y),std::min(v1.z,v2.z));
}

float3 float3::cross(const float3 &v1, const float3 &v2)
{
  return float3(v1.y*v2.z - v1.z*v2.y,
                -v1.x*v2.z + v1.z*v2.x,
                 v1.x*v2.y - v1.y*v2.x);
}

float3 operator*(const float3 &v1, const float3 &v2)
{
   return float3(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z);
}

float3 operator*(const float3 &v, float value)
{
  return float3(value*v.x, value*v.y, value*v.z);
}

float3 operator*(float value, const float3 &v)
{
  return float3(value*v.x, value*v.y, value*v.z);
}


std::ostream& operator<<(std::ostream& out, const float3& vec) // output
{
  out << vec.x;
  out << vec.y;
  out << vec.z;
  return out;
}

