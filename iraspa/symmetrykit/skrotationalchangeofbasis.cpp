#include "skrotationalchangeofbasis.h"


SKRotationalChangeOfBasis::SKRotationalChangeOfBasis(SKRotationMatrix rotationMatrix)
{
  this->_rotationMatrix = rotationMatrix;
  this->_inverseRotationMatrix = SKRotationMatrix(rotationMatrix.inverse());
}

SKRotationalChangeOfBasis::SKRotationalChangeOfBasis(SKRotationMatrix rotationMatrix, SKRotationMatrix inverseRotationMatrix)
{
  _rotationMatrix = rotationMatrix;
  _inverseRotationMatrix = inverseRotationMatrix;
}

SKRotationalChangeOfBasis SKRotationalChangeOfBasis::identity = SKRotationalChangeOfBasis(SKRotationMatrix(int3( 1, 0, 0),int3( 0, 1, 0), int3( 0, 0, 1)));

  // Table 2 from R. W. Grosse-Kunstleve, Acta Cryst. (1999). A55, 383-395
std::vector<SKRotationalChangeOfBasis> SKRotationalChangeOfBasis::changeOfMonoclinicCentering =
{
  // Note: multiples matches are possible
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 1, 0, 0), int3( 0, 1, 0), int3( 0, 0, 1))), //  1 : I           a,  b,    c
  SKRotationalChangeOfBasis(SKRotationMatrix(int3(-1, 0,-1), int3( 0, 1, 0), int3( 1, 0, 0))), //  2 : R3       -a-c,  b,    a
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 0, 0, 1), int3( 0, 1, 0), int3(-1, 0,-1))), //  3 : R3.R3       c,  b, -a-c
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 0, 0, 1), int3( 0,-1, 0), int3( 1, 0, 0))), //  4 : R2          c, -b,    a
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 1, 0, 0), int3( 0,-1, 0), int3(-1, 0,-1))), //  5 : R2.R3       a, -b, -a-c
  SKRotationalChangeOfBasis(SKRotationMatrix(int3(-1, 0,-1), int3( 0,-1, 0), int3( 0, 0, 1)))  //  6 : R2.R3.R3 -a-c, -b,    c
};


  // Table 2 from R. W. Grosse-Kunstleve, Acta Cryst. (1999). A55, 383-395
std::vector<SKRotationalChangeOfBasis> SKRotationalChangeOfBasis::changeOfOrthorhombicCentering =
{
  // Note: multiples matches are possible
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 1, 0, 0), int3( 0, 1, 0), int3( 0, 0, 1))), // 1 : I           a,  b,  c
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 0, 1, 0), int3( 0, 0, 1), int3( 1, 0, 0))), // 2 : R3          b,  c,  a
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 0, 0, 1), int3( 1, 0, 0), int3( 0, 1, 0))), // 3 : R3.R3       c,  a,  b
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 0, 1, 0), int3( 1, 0, 0), int3( 0, 0,-1))), // 4 : R2          b,  a, -c
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 1, 0, 0), int3( 0, 0,-1), int3( 0, 1, 0))), // 5 : R2.R3       a, -c,  b
  SKRotationalChangeOfBasis(SKRotationMatrix(int3( 0, 0,-1), int3( 0, 1, 0), int3( 1, 0, 0)))  // 6 : R2.R3.R3   -c,  b,  a
};

SKRotationalChangeOfBasis SKRotationalChangeOfBasis::inverse()
{
  return SKRotationalChangeOfBasis(_inverseRotationMatrix, _rotationMatrix);
}

SKSeitzIntegerMatrix SKRotationalChangeOfBasis::operator *(const SKSeitzIntegerMatrix& right) const
{
  SKRotationMatrix rotation  = SKRotationMatrix(this->_inverseRotationMatrix.int3x3 * right.rotation().int3x3 * this->_rotationMatrix.int3x3);
  int3 translation = this->_inverseRotationMatrix.int3x3 * right.translation();
  return SKSeitzIntegerMatrix(rotation, translation);
}

double3 SKRotationalChangeOfBasis::operator *(const double3& right) const
{
  return this->_inverseRotationMatrix.int3x3 * right;
}
int3 SKRotationalChangeOfBasis::operator *(const int3& right) const
{
  return this->_inverseRotationMatrix.int3x3 * right;
}





