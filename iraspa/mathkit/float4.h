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

#include <QColor>
#include "double4.h"
#include "double3.h"
#include "float3.h"

union float4
{
  float v[4];
  struct {float x, y, z, w;};


  float4(float x=0,float y=0, float z=0, float w=0):x(x),y(y),z(z),w(w) {}
  float4(double4 a):x(float(a.x)),y(float(a.y)),z(float(a.z)),w(float(a.w)) {}
  float4(double3 a, double b):x(float(a.x)),y(float(a.y)),z(float(a.z)),w(float(b)) {}
  float4(QColor a, double b):x(float(a.redF())),y(float(a.greenF())),z(float(a.blueF())),w(float(b)) {}
  float4(float3 a, float b):x(a.x),y(a.y),z(a.z),w(b) {}
  inline float & operator [] (int i) { return v[i]; }
  inline const float & operator [] (int i) const { return v[i]; }

  void normalise();
  static float dot(const float4 &v1, const float4 &v2);
  static float4 cross(const float4 &v1, const float4 &v2);
  static float4 cross(const float4 &v1, const float4 &v2, const float4 &v3);

  friend float4 operator*(const float4 &v1, const float4 &v2);
  friend float4 operator*(const float4 &v, double value);
  friend float4 operator*(double value, float4 const &v);
  friend float4 operator*(const float4 &v, float value);
  friend float4 operator*(float value, float4 const &v);

  friend float4 operator+(const float4 &v1, const float4 &v2);
  friend float4 operator-(const float4 &v1, const float4 &v2);

};
