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
#include "simd_quatd.h"
#include <QDebug>

union double4x4;

union double3x3
{
  double m[9];
  double mm[3][3];
  double3 v[3];
  struct {double m11, m21, m31,     // 1st column
                 m12, m22, m32,     // 2nd column
                 m13, m23, m33;     // 3rd column
  };
  struct {double ax, ay, az,     // 1st column
                 bx, by, bz,     // 2nd column
                 cx, cy, cz;     // 3rd column
  };

  double3x3 operator * (const double3x3& right) const;
  inline double3 & operator [] (int i) { return v[i]; }
  inline const double3 & operator [] (int i) const { return v[i]; }

 double3 operator * (const double3& right) const;

   double3x3(double m11=0.0, double m21=0.0, double m31=0.0,
             double m12=0.0, double m22=0.0, double m32=0.0,
             double m13=0.0, double m23=0.0, double m33=0.0)
        : m11(m11), m21(m21), m31(m31),
          m12(m12), m22(m22), m32(m32),
          m13(m13), m23(m23), m33(m33)
          {

          };
  double3x3(double4x4);
  double3x3(double3 v1, double3 v2, double3 v3):
          m11(v1.x), m21(v1.y), m31(v1.z),
          m12(v2.x), m22(v2.y), m32(v2.z),
          m13(v3.x), m23(v3.y), m33(v3.z)

          {

          };
  double3x3(simd_quatd q);

  double3x3(double lattice[3][3]);
  double* data() {return m;}

  double determinant(void);
  double trace(void);
  double3x3 const inverse();
  static double3x3 const inverse(const double3x3& right);
  static double3x3 transpose(const double3x3& right);
  double3x3 const transpose(void) const;
  double3x3 inversetranpose(void);
  void solve_lambdas(double3 &res);
  void solve_angles_0( double3 &res, double3 lambdas );
  void solve_angles_1( double3 &res, double3 lambdas );
  void solve_angles_2( double3 &res, double3 lambdas );
  void EigenSystemSymmetric(double3 &eigenvalues,double3x3 &eigenvectors);


  friend std::ostream& operator<<(std::ostream& out, const double3x3& vec);

  friend QDataStream &operator<<(QDataStream &, const double3x3 &);
  friend QDataStream &operator>>(QDataStream &, double3x3 &);

  friend QDebug operator<<(QDebug debug, const double3x3 &m);
};
