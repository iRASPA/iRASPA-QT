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

#include "skrotationmatrix.h"

SKRotationMatrix::SKRotationMatrix()
{

}

SKRotationMatrix::SKRotationMatrix(int3 v1, int3 v2, int3 v3)
{
  m11 = v1.x; m21 = v1.y; m31 = v1.z;
  m12 = v2.x; m22 = v2.y; m32 = v2.z;
  m13 = v3.x; m23 = v3.y; m33 = v3.z;
}

std::ostream& operator<<(std::ostream& os, const SKRotationMatrix& m)
{
    os << "SKRotationMatrix: " << m.m11 << '/' << m.m12 << '/' << m.m13 << '/' << m.m21 << '/' << m.m22 << '/' << m.m23 << '/' << m.m31 << '/' << m.m32 << '/' << m.m33 << '/';
    return os;
}

/*
SKRotationMatrix SKRotationMatrix::proper()
{
  if(determinant() == 1)
  {
    return *this;
  }
  else
  {
    return -(*this);
  }
}*/

SKRotationMatrix SKRotationMatrix::operator * (const SKRotationMatrix& b) const
{
  SKRotationMatrix r;


  r.m11 = m11 * b.m11 + m12 * b.m21 + m13 * b.m31;
  r.m21 = m21 * b.m11 + m22 * b.m21 + m23 * b.m31;
  r.m31 = m31 * b.m11 + m32 * b.m21 + m33 * b.m31;

  r.m12 = m11 * b.m12 + m12 * b.m22 + m13 * b.m32;
  r.m22 = m21 * b.m12 + m22 * b.m22 + m23 * b.m32;
  r.m32 = m31 * b.m12 + m32 * b.m22 + m33 * b.m32;

  r.m13 = m11 * b.m13 + m12 * b.m23 + m13 * b.m33;
  r.m23 = m21 * b.m13 + m22 * b.m23 + m23 * b.m33;
  r.m33 = m31 * b.m13 + m32 * b.m23 + m33 * b.m33;


  return r;
}

int3 SKRotationMatrix::operator * (const int3& b) const
{
  int3 r(0,0,0);

  r.x = m11 * b.x + m12 * b.y + m13 * b.z;
  r.y = m21 * b.x + m22 * b.y + m23 * b.z;
  r.z = m31 * b.x + m32 * b.y + m33 * b.z;

  return r;
}

double3 SKRotationMatrix::operator * (const double3& b) const
{
  double3 r;

  r.x = m11 * b.x + m12 * b.y + m13 * b.z;
  r.y = m21 * b.x + m22 * b.y + m23 * b.z;
  r.z = m31 * b.x + m32 * b.y + m33 * b.z;

  return r;
}

SKRotationMatrix SKRotationMatrix::operator-() const
{
  SKRotationMatrix r;
  r.m11 = -m11; r.m21 = -m21; r.m31 = -m31;
  r.m12 = -m12; r.m22 = -m22; r.m32 = -m32;
  r.m13 = -m13; r.m23 = -m23; r.m33 = -m33;
  return r;
}

SKRotationMatrix SKRotationMatrix::zero = SKRotationMatrix(int3(0,0,0),int3(0,0,0),int3(0,0,0));
SKRotationMatrix SKRotationMatrix::identity = SKRotationMatrix(int3(1,0,0),int3(0,1,0),int3(0,0,1));
SKRotationMatrix SKRotationMatrix::inversionIdentity = SKRotationMatrix(int3(-1,0,0),int3(0,-1,0),int3(0,0,-1));

// rotations for principle axes
SKRotationMatrix SKRotationMatrix::r_2_100 = SKRotationMatrix(int3(1,0,0),int3(0,-1,0),int3(0,0,-1));
SKRotationMatrix SKRotationMatrix::r_2i_100 = r_2_100;
SKRotationMatrix SKRotationMatrix::r_3_100 = SKRotationMatrix(int3(1,0,0),int3(0,0,1),int3(0,-1,-1));
SKRotationMatrix SKRotationMatrix::r_3i_100 = SKRotationMatrix(int3(1,0,0),int3(0,-1,-1),int3(0,1,0));
SKRotationMatrix SKRotationMatrix::r_4_100 = SKRotationMatrix(int3(1,0,0),int3(0,0,1),int3(0,-1,0));
SKRotationMatrix SKRotationMatrix::r_4i_100 = SKRotationMatrix(int3(1,0,0),int3(0,0,-1),int3(0,1,0));
SKRotationMatrix SKRotationMatrix::r_6_100 = SKRotationMatrix(int3(1,0,0),int3(0,1,1),int3(0,-1,0));
SKRotationMatrix SKRotationMatrix::r_6i_100 = SKRotationMatrix(int3(1,0,0),int3(0,0,-1),int3(0,1,1));

SKRotationMatrix SKRotationMatrix::r_2_010 = SKRotationMatrix(int3(-1,0,0),int3(0,1,0),int3(0,0,-1));
SKRotationMatrix SKRotationMatrix::r_2i_010 = r_2_010;
SKRotationMatrix SKRotationMatrix::r_3_010 = SKRotationMatrix(int3(-1,0,-1),int3(0,1,0),int3(1,0,0));
SKRotationMatrix SKRotationMatrix::r_3i_010 = SKRotationMatrix(int3(0,0,1),int3(0,1,0),int3(-1,0,-1));
SKRotationMatrix SKRotationMatrix::r_4_010 = SKRotationMatrix(int3(0,0,-1),int3(0,1,0),int3(1,0,0));
SKRotationMatrix SKRotationMatrix::r_4i_010 = SKRotationMatrix(int3(0,0,1),int3(0,1,0),int3(-1,0,0));
SKRotationMatrix SKRotationMatrix::r_6_010 = SKRotationMatrix(int3(0,0,-1),int3(0,1,0),int3(1,0,1));
SKRotationMatrix SKRotationMatrix::r_6i_010 = SKRotationMatrix(int3(1,0,1),int3(0,1,0),int3(-1,0,0));

SKRotationMatrix SKRotationMatrix::r_2_001 = SKRotationMatrix(int3(-1,0,0),int3(0,-1,0),int3(0,0,1));
SKRotationMatrix SKRotationMatrix::r_2i_001 = r_2_001;
SKRotationMatrix SKRotationMatrix::r_3_001 = SKRotationMatrix(int3(0,1,0),int3(-1,-1,0),int3(0,0,1));
SKRotationMatrix SKRotationMatrix::r_3i_001 = SKRotationMatrix(int3(-1,-1,0),int3(1,0,0),int3(0,0,1));
SKRotationMatrix SKRotationMatrix::r_4_001 = SKRotationMatrix(int3(0,1,0),int3(-1,0,0),int3(0,0,1));
SKRotationMatrix SKRotationMatrix::r_4i_001 = SKRotationMatrix(int3(0,-1,0),int3(1,0,0),int3(0,0,1));
SKRotationMatrix SKRotationMatrix::r_6_001 = SKRotationMatrix(int3(1,1,0),int3(-1,0,0),int3(0,0,1));
SKRotationMatrix SKRotationMatrix::r_6i_001 = SKRotationMatrix(int3(0,-1,0),int3(1,1,0),int3(0,0,1));

SKRotationMatrix SKRotationMatrix::r_3_111 = SKRotationMatrix(int3(0,1,0),int3(0,0,1),int3(1,0,0));
SKRotationMatrix SKRotationMatrix::r_3i_111 = SKRotationMatrix(int3(0,0,1),int3(1,0,0),int3(0,1,0));

SKRotationMatrix SKRotationMatrix::r_2prime_100 = SKRotationMatrix(int3(-1,0,0),int3(0,0,-1),int3(0,-1,0));   // b-c
SKRotationMatrix SKRotationMatrix::r_2iprime_100 = r_2prime_100;
SKRotationMatrix SKRotationMatrix::r_2doubleprime_100 = SKRotationMatrix(int3(-1,0,0),int3(0,0,1),int3(0,1,0)); // b+c
SKRotationMatrix SKRotationMatrix::r_2idoubleprime_100 = r_2doubleprime_100;

SKRotationMatrix SKRotationMatrix::r_2prime_010 = SKRotationMatrix(int3(0,0,-1),int3(0,-1,0),int3(-1,0,0)); // a-c
SKRotationMatrix SKRotationMatrix::r_2iprime_010 = r_2prime_010;
SKRotationMatrix SKRotationMatrix::r_2doubleprime_010 = SKRotationMatrix(int3(0,0,1),int3(0,-1,0),int3(1,0,0)); // a+c
SKRotationMatrix SKRotationMatrix::r_2idoubleprime_010 = r_2doubleprime_010;

SKRotationMatrix SKRotationMatrix::r_2prime_001 = SKRotationMatrix(int3(0,-1,0),int3(-1,0,0),int3(0,0,-1)); // a-b
SKRotationMatrix SKRotationMatrix::r_2iprime_001 = r_2prime_001;
SKRotationMatrix SKRotationMatrix::r_2doubleprime_001 = SKRotationMatrix(int3(0,1,0),int3(1,0,0),int3(0,0,-1)); // a+b
SKRotationMatrix SKRotationMatrix::r_2idoubleprime_001 = r_2doubleprime_001;
