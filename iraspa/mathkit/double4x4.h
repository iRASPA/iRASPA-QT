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

#include "double3.h"
#include "double4.h"
#include "simd_quatd.h"

union double3x3;

union double4x4
{
  double m[16];
  double mm[4][4];
  double4 v[4];
  struct {double m11, m21, m31, m41,   // 1st column
                 m12, m22, m32, m42,   // 2nd column
                 m13, m23, m33, m43,   // 3rd column
                 m14, m24, m34, m44;   // 4th column, {tx,ty,tz,1}
  };
  struct {double ax, ay, az, aw,    // 1st column
                 bx, by, bz, bw,    // 2nd column
                 cx, cy, cz, cw,    // 3rd column
                 wx, wy, wz, ww;    // 4th column, {tx,ty,tz,1}
  };

  inline double4 & operator [] (int i) { return v[i]; }
  inline const double4 & operator [] (int i) const { return v[i]; }

  double4x4(double m11=0.0, double m21=0.0, double m31=0.0, double m41=0.0,
            double m12=0.0, double m22=0.0, double m32=0.0, double m42=0.0,
            double m13=0.0, double m23=0.0, double m33=0.0, double m43=0.0,
            double m14=0.0, double m24=0.0, double m34=0.0, double m44=0.0)
        : m11(m11), m21(m21), m31(m31), m41(m41),
          m12(m12), m22(m22), m32(m32), m42(m42),
          m13(m13), m23(m23), m33(m33), m43(m43),
          m14(m14), m24(m24), m34(m34), m44(m44)
          {

          }

  double4x4(double4 v1, double4 v2, double4 v3, double4 v4): v{v1,v2,v3,v4} {}
  double4x4(double3x3);


  double4x4 operator * (const double4x4& right) const;
  double4 operator * (const double4& right) const;
  inline bool operator==(const double4x4& rhs) const;
  inline bool operator!=(const double4x4& rhs) const;

  double4x4 const transpose(void);
  static double4x4 const inverse(const double4x4& right);
  void inverse();
  static double4x4 TransformationAroundArbitraryPoint(double4x4 m, double3 p);


  static double4x4 Identity(void);
  static double4x4 AffinityMatrixToTransformationAroundArbitraryPoint(double4x4 m, double3 p);
  static double4x4 AffinityMatrixToTransformationAroundArbitraryPointWithTranslation(double4x4 m, double3 p, double3 t);
  static double4x4 Rotationdouble4x4FromQuaternion(simd_quatd q);
};

