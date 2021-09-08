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

#include "float3.h"
#include "float4.h"
#include "double3x3.h"
#include "double4x4.h"
#include "simd_quatd.h"

union float3x4
{
  float m[12];
  float mm[3][4];
  float4 v[3];
  struct {float m11, m21, m31, m41,   // 1st column
                m12, m22, m32, m42,   // 2nd column
                m13, m23, m33, m43;   // 3rd column
  };
  struct {float ax, ay, az, aw,    // 1st column
                bx, by, bz, bw,    // 2nd column
                cx, cy, cz, cw;    // 3rd column
  };

  inline float4 & operator [] (int i) { return v[i]; }
  inline const float4 & operator [] (int i) const { return v[i]; }

  float3x4(float m11=0.0, float m21=0.0, float m31=0.0, float m41=0.0,
           float m12=0.0, float m22=0.0, float m32=0.0, float m42=0.0,
           float m13=0.0, float m23=0.0, float m33=0.0, float m43=0.0)
        : m11(m11), m21(m21), m31(m31), m41(m41),
          m12(m12), m22(m22), m32(m32), m42(m42),
          m13(m13), m23(m23), m33(m33), m43(m43)
          {

          };

  float3x4(const float3x4 &);  // copy constructor
  float3x4& operator=(const float3x4&);  //assignment operator
  float3x4(float4 v1, float4 v2, float4 v3): v{v1,v2,v3} {}
};
