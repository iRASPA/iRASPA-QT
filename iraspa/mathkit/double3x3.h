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
#include "int3x3.h"

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

  double3x3(double m11=0.0, double m21=0.0, double m31=0.0,
            double m12=0.0, double m22=0.0, double m32=0.0,
            double m13=0.0, double m23=0.0, double m33=0.0)
        : m11(m11), m21(m21), m31(m31),
          m12(m12), m22(m22), m32(m32),
          m13(m13), m23(m23), m33(m33)
          {

          };
  double3x3(double3 v1, double3 v2, double3 v3):
          m11(v1.x), m21(v1.y), m31(v1.z),
          m12(v2.x), m22(v2.y), m32(v2.z),
          m13(v3.x), m23(v3.y), m33(v3.z)

          {

          };
  double3x3(simd_quatd q);

  double3x3(int3x3 m);

  double3x3(double lattice[3][3]);

  inline double3 & operator [] (int i) { return v[i]; }
  inline const double3 & operator [] (int i) const { return v[i]; }

  static double3x3 identity();

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

  inline double3x3 operator-() const {return double3x3(-this->v[0], -this->v[1], -this->v[2]);}

  inline int3x3 toInt3x3() const
  {
      int3x3 w;
      w.m11 = int(rint(this->m11)); w.m21 = int(rint(this->m21)); w.m31 = int(rint(this->m31));
      w.m12 = int(rint(this->m12)); w.m22 = int(rint(this->m22)); w.m32 = int(rint(this->m32));
      w.m13 = int(rint(this->m13)); w.m23 = int(rint(this->m23)); w.m33 = int(rint(this->m33));
      return w;
  };
  inline bool operator==(const double3x3& b) const
  {
    return ( (abs(this->m11-b.m11) < 1e-5) && (abs(this->m12-b.m12) < 1e-5) &&  (abs(this->m13-b.m13) < 1e-5) &&
             (abs(this->m21-b.m21) < 1e-5) && (abs(this->m22-b.m22) < 1e-5) &&  (abs(this->m23-b.m23) < 1e-5) &&
             (abs(this->m31-b.m31) < 1e-5) && (abs(this->m32-b.m32) < 1e-5) &&  (abs(this->m33-b.m33) < 1e-5) );
  }

  friend std::ostream& operator<<(std::ostream& out, const double3x3& vec);

  friend QDataStream &operator<<(QDataStream &, const double3x3 &);
  friend QDataStream &operator>>(QDataStream &, double3x3 &);

  friend QDebug operator<<(QDebug debug, const double3x3 &m);
};

inline double3x3 operator*(const double3x3& a, const double3x3& b)
{
  double3x3 r;

  r.m11 = a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31;
  r.m21 = a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31;
  r.m31 = a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31;

  r.m12 = a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32;
  r.m22 = a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32;
  r.m32 = a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32;

  r.m13 = a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33;
  r.m23 = a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33;
  r.m33 = a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33;

  return r;
}

inline double3 operator*(const double3x3& a, const double3& b)
{
  double3 r;

  r.x = a.m11 * b.x + a.m12 * b.y + a.m13 * b.z;
  r.y = a.m21 * b.x + a.m22 * b.y + a.m23 * b.z;
  r.z = a.m31 * b.x + a.m32 * b.y + a.m33 * b.z;

  return r;
}

inline double3x3 operator/(const double3x3& a, const double& b)
{
  double3x3 r;
  r.m11 = a.m11 / b; r.m12 = a.m12 / b; r.m13 = a.m13 / b;
  r.m21 = a.m21 / b; r.m22 = a.m22 / b; r.m23 = a.m23 / b;
  r.m31 = a.m31 / b; r.m32 = a.m32 / b; r.m33 = a.m33 / b;
  return r;
}
