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

#include "skseitzintegermatrix.h"
#include <iostream>
#include <cassert>
#include <QDebug>

SKSeitzIntegerMatrix::SKSeitzIntegerMatrix()
{

}

SKSeitzIntegerMatrix::SKSeitzIntegerMatrix(SKRotationMatrix rotation, int3 translation)
{
  _rotation = rotation;
  _translation = translation;  // denominator = 24
}

SKSeitzIntegerMatrix::SKSeitzIntegerMatrix(char xvalue, char yvalue, char zvalue)
{
  char referenceValue = '0';

  size_t x = size_t(xvalue - referenceValue);
  size_t y = size_t(yvalue - referenceValue);
  size_t z = size_t(zvalue - referenceValue);

  assert(x<SKSeitzIntegerMatrix::_SeitzData.size());
  assert(y<SKSeitzIntegerMatrix::_SeitzData.size());
  assert(z<SKSeitzIntegerMatrix::_SeitzData.size());

  int3 r1 = int3(SKSeitzIntegerMatrix::_SeitzData[x].r1(), SKSeitzIntegerMatrix::_SeitzData[y].r1(), SKSeitzIntegerMatrix::_SeitzData[z].r1());
  int3 r2 = int3(SKSeitzIntegerMatrix::_SeitzData[x].r2(), SKSeitzIntegerMatrix::_SeitzData[y].r2(), SKSeitzIntegerMatrix::_SeitzData[z].r2());
  int3 r3 = int3(SKSeitzIntegerMatrix::_SeitzData[x].r3(), SKSeitzIntegerMatrix::_SeitzData[y].r3(), SKSeitzIntegerMatrix::_SeitzData[z].r3());
   _rotation = SKRotationMatrix(r1,r2,r3);
   _translation = int3(SKSeitzIntegerMatrix::_SeitzData[x].t(), SKSeitzIntegerMatrix::_SeitzData[y].t(), SKSeitzIntegerMatrix::_SeitzData[z].t());
}

std::vector<SKSeitzIntegerMatrix> SKSeitzIntegerMatrix::SeitzMatrices(std::string encoding)
{
  size_t m = encoding.size()/3;

  std::vector<SKSeitzIntegerMatrix> matrices = std::vector<SKSeitzIntegerMatrix>();
  matrices.reserve(m);

  for(size_t i=0;i<m;i++)
  {
    char x = encoding[3 * i];
    char y = encoding[3 * i + 1];
    char z = encoding[3 * i + 2];

    matrices[i] = SKSeitzIntegerMatrix(x,y,z);
  }

  return matrices;
}

bool SKSeitzIntegerMatrix::operator==(SKSeitzIntegerMatrix const& rhs) const
{
    return (_rotation.int3x3.m11 == rhs.rotation().int3x3.m11) &&
           (_rotation.int3x3.m21 == rhs.rotation().int3x3.m21) &&
           (_rotation.int3x3.m31 == rhs.rotation().int3x3.m31) &&
           (_rotation.int3x3.m12 == rhs.rotation().int3x3.m12) &&
           (_rotation.int3x3.m22 == rhs.rotation().int3x3.m22) &&
           (_rotation.int3x3.m32 == rhs.rotation().int3x3.m32) &&
           (_rotation.int3x3.m13 == rhs.rotation().int3x3.m13) &&
           (_rotation.int3x3.m23 == rhs.rotation().int3x3.m23) &&
           (_rotation.int3x3.m33 == rhs.rotation().int3x3.m33) &&
           ((_translation.x%24) == (rhs.translation().x%24)) &&
           ((_translation.y%24) == (rhs.translation().y%24)) &&
           ((_translation.z%24) == (rhs.translation().z%24));
}

double3 SKSeitzIntegerMatrix::operator * (const double3& right) const
{
  double3 v = _rotation * right;
  return v + double3(_translation.x/24.0, _translation.y/24.0, _translation.z/24.0);
}

SKSeitzIntegerMatrix SKSeitzIntegerMatrix::operator * (const SKSeitzIntegerMatrix& right) const
{
  SKRotationMatrix rotationMatrix = _rotation * right.rotation();
  int3 translation = _translation + _rotation * right.translation();
  return SKSeitzIntegerMatrix(rotationMatrix, translation);
}

std::vector<SKOneThirdSeitzMatrix> SKSeitzIntegerMatrix::_SeitzData = std::vector<SKOneThirdSeitzMatrix>{
  SKOneThirdSeitzMatrix(" x", '0', 1, 0, 0, 0),
  SKOneThirdSeitzMatrix(" y", '1', 0, 1, 0, 0),
  SKOneThirdSeitzMatrix(" z", '2', 0, 0, 1, 0),
  SKOneThirdSeitzMatrix("-x", '3', -1, 0, 0, 0),
  SKOneThirdSeitzMatrix("-y", '4', 0, -1, 0, 0),
  SKOneThirdSeitzMatrix("-z", '5', 0, 0, -1, 0),
  SKOneThirdSeitzMatrix("x-y", '6',  1, -1, 0, 0),
  SKOneThirdSeitzMatrix("-x+y", '7', -1, 1, 0, 0),
  SKOneThirdSeitzMatrix("x+1/2", '8', 1, 0, 0, 12),
  SKOneThirdSeitzMatrix("x+1/3", '9', 1, 0, 0, 8),
  SKOneThirdSeitzMatrix("x+1/4", ':', 1, 0, 0, 6),
  SKOneThirdSeitzMatrix("x+2/3", ';', 1, 0, 0, 16),
  SKOneThirdSeitzMatrix("x+3/4", '<', 1, 0, 0, 18),
  SKOneThirdSeitzMatrix("y+1/2", '=', 0, 1, 0, 12),
  SKOneThirdSeitzMatrix("y+1/3", '>', 0, 1, 0, 8),
  SKOneThirdSeitzMatrix("y+1/4", '?', 0, 1, 0, 6),
  SKOneThirdSeitzMatrix("y+2/3", '@', 0, 1, 0, 16),
  SKOneThirdSeitzMatrix("y+3/4", 'A', 0, 1, 0, 18),
  SKOneThirdSeitzMatrix("z+1/2", 'B', 0, 0, 1, 12),
  SKOneThirdSeitzMatrix("z+1/3", 'C', 0, 0, 1, 8),
  SKOneThirdSeitzMatrix("z+1/4", 'D', 0, 0, 1, 6),
  SKOneThirdSeitzMatrix("z+1/6", 'E', 0, 0, 1, 4),
  SKOneThirdSeitzMatrix("z+2/3", 'F', 0, 0, 1, 16),
  SKOneThirdSeitzMatrix("z+3/4", 'G', 0, 0, 1, 18),
  SKOneThirdSeitzMatrix("z+5/6", 'H', 0, 0, 1, 20),
  SKOneThirdSeitzMatrix("-x+1/2", 'I', -1,  0, 0, 12),
  SKOneThirdSeitzMatrix("-x+1/3", 'J', -1,  0, 0, 8),
  SKOneThirdSeitzMatrix("-x+1/4", 'K', -1,  0, 0, 6),
  SKOneThirdSeitzMatrix("-x+2/3", 'L', -1,  0, 0, 16),
  SKOneThirdSeitzMatrix("-x+3/4", 'M', -1,  0, 0, 18),
  SKOneThirdSeitzMatrix("-y+1/2", 'N', 0, -1, 0, 12),
  SKOneThirdSeitzMatrix("-y+1/3", 'O', 0, -1, 0, 8),
  SKOneThirdSeitzMatrix("-y+1/4", 'P', 0, -1, 0, 6),
  SKOneThirdSeitzMatrix("-y+2/3", 'Q', 0, -1, 0, 16),
  SKOneThirdSeitzMatrix("-y+3/4", 'R', 0, -1, 0, 18),
  SKOneThirdSeitzMatrix("-z+1/2", 'S', 0,  0, -1, 12),
  SKOneThirdSeitzMatrix("-z+1/3", 'T', 0,  0, -1, 8),
  SKOneThirdSeitzMatrix("-z+1/4", 'U', 0,  0, -1, 6),
  SKOneThirdSeitzMatrix("-z+1/6", 'V', 0,  0, -1, 4),
  SKOneThirdSeitzMatrix("-z+2/3", 'W', 0,  0, -1, 16),
  SKOneThirdSeitzMatrix("-z+3/4", 'X', 0,  0, -1, 18),
  SKOneThirdSeitzMatrix("-z+5/6", 'Y', 0,  0, -1, 20),
  SKOneThirdSeitzMatrix("x-y+1/3", 'Z', 1, -1, 0, 8),
  SKOneThirdSeitzMatrix("x-y+2/3", '[', 1, -1, 0, 16),
  SKOneThirdSeitzMatrix("-x+y+1/3", '\\', -1, 1, 0, 8),
  SKOneThirdSeitzMatrix("-x+y+2/3", ']', -1, 1, 0, 16)
};
