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
#include <ostream>

#include "int3.h"

union float3
{
  float v[3];
  struct {float x, y, z;};

  float3(float x=0,float y=0, float z=0):x(x),y(y),z(z) {}
  float3(int3 a):x(float(a.x)),y(float(a.y)),z(float(a.z)) {}
  inline float & operator [] (int i) { return v[i]; }
  inline const float & operator [] (int i) const { return v[i]; }
  float3 operator + (const float3& right) const;
  float3 operator - (const float3& right) const;
  float length() {return sqrt(x*x+y*y+z*z);}
  float length() const {return sqrt(x*x+y*y+z*z);}
  float3 normalise();
  float3 fract();

  static float dot(const float3 &v1, const float3 &v2);
  static float3 max(const float3 &v1, const float3 &v2);
  static float3 min(const float3 &v1, const float3 &v2);
  static float3 cross(const float3 &v1, const float3 &v2);


  friend float3 operator*(const float3 &v1, const float3 &v2);
  friend float3 operator*(const float3 &v, float value);
  friend float3 operator*(float value, const float3 &v);
  friend std::ostream& operator<<(std::ostream& out, const float3& vec) ;
};

