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

#include <string>
#include <vector>
#include <mathkit.h>
#include "skrotationmatrix.h"
#include "skonethirdseitzmatrix.h"
#include "skdefinitions.h"


struct SKSeitzIntegerMatrix
{
  SKRotationMatrix rotation;
  int3 translation;   // denominator = 24

  SKSeitzIntegerMatrix();
  SKSeitzIntegerMatrix(SKRotationMatrix rotation, int3 translation);
  SKSeitzIntegerMatrix(char xvalue, char yvalue, char zvalue);
  static std::vector<SKSeitzIntegerMatrix> SeitzMatrices(std::string encoding);

  int3 normalizedTranslation() const {return int3(translation.x%24, translation.y%24, translation.z%24);}

  bool operator==(SKSeitzIntegerMatrix const& rhs) const
  {
      return (this->rotation == rhs.rotation) &&
             ((this->translation.x%24) == (rhs.translation.x%24)) &&
             ((this->translation.y%24) == (rhs.translation.y%24)) &&
             ((this->translation.z%24) == (rhs.translation.z%24));
  }

  static std::vector<SKOneThirdSeitzMatrix> SeitzData;
};

inline double3 operator*(const SKSeitzIntegerMatrix &a, const double3& b)
{
  double3 v = a.rotation * b;
  return v + double3(a.translation.x/24.0, a.translation.y/24.0, a.translation.z/24.0);
}

inline SKSeitzIntegerMatrix operator*(const SKSeitzIntegerMatrix &a,const SKSeitzIntegerMatrix &b)
{
  SKRotationMatrix rotationMatrix = a.rotation * b.rotation;
  int3 translationVector = a.translation + a.rotation * b.translation;
  return SKSeitzIntegerMatrix(rotationMatrix, translationVector);
}


namespace std
{
  template <> struct hash<SKSeitzIntegerMatrix>
  {
    size_t operator()(const SKSeitzIntegerMatrix& k) const
    {
      std::size_t h=0;
      hash_combine(h, k.rotation);
      hash_combine(h, k.translation);
      return h;
    }
  };
}
