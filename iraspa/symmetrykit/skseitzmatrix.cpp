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

#include "skseitzmatrix.h"
#include <iostream>
#include <cassert>
#include <QDebug>

SKSeitzMatrix::SKSeitzMatrix()
{

}

SKSeitzMatrix::SKSeitzMatrix(SKRotationMatrix rotation, double3 translation)
{
  _rotation = rotation;
  _translation = translation;
}

bool operator==(const SKSeitzMatrix& lhs, const SKSeitzMatrix& rhs)
{
  double3 dr = (lhs._translation - rhs._translation);
  dr.x -= rint(dr.x);
  dr.y -= rint(dr.y);
  dr.z -= rint(dr.z);

  return (lhs._rotation.int3x3.m11 == rhs._rotation.int3x3.m11) &&
         (lhs._rotation.int3x3.m12 == rhs._rotation.int3x3.m12) &&
         (lhs._rotation.int3x3.m13 == rhs._rotation.int3x3.m13) &&
         (lhs._rotation.int3x3.m21 == rhs._rotation.int3x3.m21) &&
         (lhs._rotation.int3x3.m22 == rhs._rotation.int3x3.m22) &&
         (lhs._rotation.int3x3.m23 == rhs._rotation.int3x3.m23) &&
         (lhs._rotation.int3x3.m31 == rhs._rotation.int3x3.m31) &&
         (lhs._rotation.int3x3.m32 == rhs._rotation.int3x3.m32) &&
         (lhs._rotation.int3x3.m33 == rhs._rotation.int3x3.m33) &&
         (dr.length_squared() < 1e-5);

}
