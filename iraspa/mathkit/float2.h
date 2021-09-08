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

union float2
{
  float v[2];
  struct {float x, y;};

  inline float2(float x=0,float y=0):x(x),y(y) {};

  inline float & operator [] (int i) { return v[i]; }
  inline const float & operator [] (int i) const { return v[i]; }

  inline float2& operator += (const float2& rhs) { x += rhs.x; y += rhs.y; return *this; }
  inline float2& operator -= (const float2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }

  inline static float dot(const float2 &v1, const float2 &v2)
  {
    return v1.x*v2.x+v1.y*v2.y;
  }
};


inline float2 operator+(const float2& a, const float2& b)
{
  return float2(a.x+b.x,a.y+b.y);
}

inline float2 operator-(const float2& a, const float2& b)
{
  return float2(a.x-b.x,a.y-b.y);
}

inline float2 operator*(const float2& a, const float2& b)
{
  return float2(a.x*b.x, a.y*b.y);
}
