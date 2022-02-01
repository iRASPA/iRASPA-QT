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

#include <ostream>
#include <mathkit.h>

struct SKRotationMatrix
{
  union int3x3 int3x3;

  enum class RotationType: qint64
  {
    axis_6m = -6, axis_4m = -4, axis_3m = -3, axis_2m = -2, axis_1m = -1, none = 0, axis_1 = 1, axis_2 = 2, axis_3 = 3, axis_4 = 4, axis_6 = 6
  };

  enum class SymmetryType: qint64
  {
    unknown = 0, identity = 1, translation = 2, inversion = 3, pure_rotation = 4, pure_reflection = 5, screw_rotation = 6, glide_reflection = 7
  };


  SKRotationMatrix();
  SKRotationMatrix(union int3x3 m) {this->int3x3 = m;}
  SKRotationMatrix(int3 v1, int3 v2, int3 v3);


  SKRotationMatrix inverse();
  const SKRotationMatrix proper() const;
  SKRotationMatrix::RotationType type() const;
  int3 rotationAxis() const;
  std::vector<int3> orthogonalToAxisDirection(int rotationOrder);

  inline int determinant() const {return this->int3x3.determinant();}
  inline SKRotationMatrix operator-() const {return -this->int3x3;}
  inline bool operator==(const SKRotationMatrix& b) const {return this->int3x3 == b.int3x3;}

  friend std::ostream& operator<<(std::ostream& os, const SKRotationMatrix& setting);

  static SKRotationMatrix zero;
  static SKRotationMatrix identity;
  static SKRotationMatrix inversionIdentity;

  // rotations for principle axes
  static SKRotationMatrix r_2_100;
  static SKRotationMatrix r_2i_100;
  static SKRotationMatrix r_3_100;
  static SKRotationMatrix r_3i_100;
  static SKRotationMatrix r_4_100;
  static SKRotationMatrix r_4i_100;
  static SKRotationMatrix r_6_100;
  static SKRotationMatrix r_6i_100;

  static SKRotationMatrix r_2_010;
  static SKRotationMatrix r_2i_010;
  static SKRotationMatrix r_3_010;
  static SKRotationMatrix r_3i_010;
  static SKRotationMatrix r_4_010;
  static SKRotationMatrix r_4i_010;
  static SKRotationMatrix r_6_010;
  static SKRotationMatrix r_6i_010;

  static SKRotationMatrix r_2_001;
  static SKRotationMatrix r_2i_001;
  static SKRotationMatrix r_3_001;
  static SKRotationMatrix r_3i_001;
  static SKRotationMatrix r_4_001;
  static SKRotationMatrix r_4i_001;
  static SKRotationMatrix r_6_001;
  static SKRotationMatrix r_6i_001;

  static SKRotationMatrix r_3_111;
  static SKRotationMatrix r_3i_111;

  static SKRotationMatrix r_2prime_100;
  static SKRotationMatrix r_2iprime_100;
  static SKRotationMatrix r_2doubleprime_100;
  static SKRotationMatrix r_2idoubleprime_100;

  static SKRotationMatrix r_2prime_010;
  static SKRotationMatrix r_2iprime_010;
  static SKRotationMatrix r_2doubleprime_010;
  static SKRotationMatrix r_2idoubleprime_010;

  static SKRotationMatrix r_2prime_001;
  static SKRotationMatrix r_2iprime_001;
  static SKRotationMatrix r_2doubleprime_001;
  static SKRotationMatrix r_2idoubleprime_001;

  static SKRotationMatrix monoclinicB1toA1;
  static SKRotationMatrix monoclinicB1toA2;
  static SKRotationMatrix monoclinicB1toA3;
  static SKRotationMatrix monoclinicB1toB2;
  static SKRotationMatrix monoclinicB1toB3;
  static SKRotationMatrix monoclinicB1toC1;
  static SKRotationMatrix monoclinicB1toC2;
  static SKRotationMatrix monoclinicB1toC3;

  static SKRotationMatrix orthorhombicCABtoABC;
  static SKRotationMatrix orthorhombicBCAtoABC;
  static SKRotationMatrix orthorhombicBAmCtoABC;
  static SKRotationMatrix orthorhombicAmCBtoABC;
  static SKRotationMatrix orthorhombicmCBAtoABC;

  static std::vector<std::tuple<SKRotationMatrix, int3, int3>> twoFoldSymmetryOperations;
  static std::vector<int3> allPossibleRotationAxes;
};

inline SKRotationMatrix operator * (const SKRotationMatrix& a, const SKRotationMatrix& b)
{
  return SKRotationMatrix(a.int3x3 * b.int3x3);
}

inline int3 operator * (const SKRotationMatrix& a, const int3& b)
{
  return a.int3x3 * b;
}

inline double3 operator * (const SKRotationMatrix& a, const double3& b)
{
  return a.int3x3 * b;
}

inline SKRotationMatrix operator + (const SKRotationMatrix& a, const SKRotationMatrix& b)
{
  return a.int3x3 + b.int3x3;
}

inline double3x3 operator*(const double3x3 &v1, const SKRotationMatrix &v2)
{
  return v1 * double3x3(v2.int3x3);
}

inline SKRotationMatrix operator-(const SKRotationMatrix &v1, const SKRotationMatrix &v2)
{
  return SKRotationMatrix(v1.int3x3-v2.int3x3);
}

namespace std
{
  template <> struct hash<SKRotationMatrix>
  {
    size_t operator()(const SKRotationMatrix& k) const
    {
      std::hash<int3x3> hash_fn;
      return hash_fn(k.int3x3);
    }
  };
}
