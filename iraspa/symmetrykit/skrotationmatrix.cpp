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
  int3x3.m11 = v1.x; int3x3.m21 = v1.y; int3x3.m31 = v1.z;
  int3x3.m12 = v2.x; int3x3.m22 = v2.y; int3x3.m32 = v2.z;
  int3x3.m13 = v3.x; int3x3.m23 = v3.y; int3x3.m33 = v3.z;
}

std::ostream& operator<<(std::ostream& os, const SKRotationMatrix& m)
{
    os << "SKRotationMatrix: " << m.int3x3.m11 << '/' << m.int3x3.m12 << '/' << m.int3x3.m13 << '/' << m.int3x3.m21 << '/' << m.int3x3.m22 << '/' << m.int3x3.m23 << '/' << m.int3x3.m31 << '/' << m.int3x3.m32 << '/' << m.int3x3.m33 << '/';
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
  return SKRotationMatrix(this->int3x3 * b.int3x3);
}

int3 SKRotationMatrix::operator * (const int3& b) const
{
  return this->int3x3 * b;
}

double3 SKRotationMatrix::operator * (const double3& b) const
{
  return double3x3(this->int3x3) * b;
}

SKRotationMatrix SKRotationMatrix::operator-() const
{
  return -this->int3x3;
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
