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
#include "simd_quatd.h"

union float4x4;

union float3x3
{
  float m[9];
  float mm[3][3];
  float3 v[3];
  struct {float m11, m21, m31,     // 1st column
                 m12, m22, m32,     // 2nd column
                 m13, m23, m33;     // 3rd column
  };
  struct {float ax, ay, az,     // 1st column
                 bx, by, bz,     // 2nd column
                 cx, cy, cz;     // 3rd column
  };

  float3x3 operator * (const float3x3& right) const;
  inline float3  operator [] (int i) { return v[i]; }
  inline const float3  operator [] (int i) const { return v[i]; }

 float3 operator * (const float3& right) const;

   float3x3(float m11=0.0, float m21=0.0, float m31=0.0,
             float m12=0.0, float m22=0.0, float m32=0.0,
             float m13=0.0, float m23=0.0, float m33=0.0)
        : m11(m11), m21(m21), m31(m31),
          m12(m12), m22(m22), m32(m32),
          m13(m13), m23(m23), m33(m33)
          {

          };
  float3x3(float4x4);
  float3x3(float3 v1, float3 v2, float3 v3):
          m11(v1.x), m21(v1.y), m31(v1.z),
          m12(v2.x), m22(v2.y), m32(v2.z),
          m13(v3.x), m23(v3.y), m33(v3.z)

          {

          };
   float3x3(simd_quatd q);

   float determinant(void);
   float trace(void);
   float3x3 const inverse();
   static float3x3 const inverse(const float3x3& right);
   float3x3 const transpose(void);
   float3x3 inversetranpose(void);
   void solve_lambdas(float3 &res);
   void solve_angles_0( float3 &res, float3 lambdas );
   void solve_angles_1( float3 &res, float3 lambdas );
   void solve_angles_2( float3 &res, float3 lambdas );
   void EigenSystemSymmetric(float3 &eigenvalues,float3x3 &eigenvectors);
   void SingularValueDecompositionSymmetric(float3x3 &U,float3 &D,float3x3 &V);
   void SingularValueDecomposition(float3x3 &a,float3 &w,float3x3 &v);



  friend std::ostream& operator<<(std::ostream& out, const float3x3& vec) ;
};
