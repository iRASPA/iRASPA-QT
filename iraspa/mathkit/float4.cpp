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

#include "float4.h"

#include <cmath>

/*
float4::float4()
{
  _x=0.0;
  _y=0.0;
  _z=0.0;
  _r=0.0;
}
*/

/*
float4::float4(float v1=0.0, float v2=0.0, float v3=0.0, float v4=0.0)
{
  _x=v1;
  _y=v2;
  _z=v3;
  _z=v4;
}
*/



void float4::normalise()
{
  float magnitude = sqrt((x * x) + (y * y) + (z * z) * (w * w));

  if (magnitude != 0)
  {
    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
    w /= magnitude;
  }
}

float float4::dot(const float4 &v1, const float4 &v2)
{
  return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z+v1.w*v2.w;
}

// http://www.gamedev.net/topic/456301-cross-product-vector-4d/
float4 float4::cross(const float4 &v1, const float4 &v2)
{
  return float4(v1.y*v2.z - v1.z* v2.y,
                 -v1.x*v2.z + v1.z*v2.x,
                 v1.x*v2.y - v1.y*v2.x,
                 0.0);
}


float4 float4::cross(const float4 &v1, const float4 &v2, const float4 &v3)
{
  return float4(v1.y*(v2.z*v3.w - v3.z*v2.w) - v1.z*(v2.y*v3.w - v3.y*v2.w) + v1.w*(v2.y*v3.z - v3.y*v2.z),
                 -v1.x*(v2.z*v3.w - v3.z*v2.w) + v1.z*(v2.x*v3.w - v3.x*v2.w) - v1.w*(v2.x*v3.z - v3.x*v2.z),
                 v1.x*(v2.y*v3.w - v3.y*v2.w) - v1.y*(v2.x*v3.w - v3.x*v2.w) + v1.w*(v2.x*v3.y - v3.x*v2.y),
                 -v1.x*(v2.y*v3.z - v3.y*v2.z) + v1.y*(v2.x*v3.z - v3.x*v2.z) - v1.z*(v2.x*v3.y - v3.x*v2.y));
}

float4 operator*(const float4 &v1, const float4 &v2)
{
   return float4(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z, v1.w*v2.w);
}

float4 operator*(const float4 &v, double value)
{
  return float4(value*v.x, value*v.y, value*v.z, value*v.w);
}

float4 operator*(double value, float4 const &v)
{
  return float4(value*v.x, value*v.y, value*v.z, value*v.w);
}

float4 operator*(const float4 &v, float value)
{
  return float4(value*v.x, value*v.y, value*v.z, value*v.w);
}

float4 operator*(float value, float4 const &v)
{
  return float4(value*v.x, value*v.y, value*v.z, value*v.w);
}


float4 operator+(const float4 &v1, const float4 &v2)
{
   return float4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

float4 operator-(const float4 &v1, const float4 &v2)
{
   return float4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}
