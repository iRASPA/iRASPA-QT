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
#include "skdefinitions.h"


struct SKRotationMatrix: public int3x3
{
  SKRotationMatrix();
  SKRotationMatrix(int3 v1, int3 v2, int3 v3);

 // SKRotationMatrix proper();
 friend std::ostream& operator<<(std::ostream& os, const SKRotationMatrix& setting);

  SKRotationMatrix operator * (const SKRotationMatrix& right) const;
  int3 operator * (const int3& right) const;
  double3 operator * (const double3& right) const;
  SKRotationMatrix operator-() const;

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
};
