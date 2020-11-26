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


class SKSeitzMatrix
{
public:
  SKSeitzMatrix();
  SKSeitzMatrix(SKRotationMatrix rotation, int3 translation);
  SKSeitzMatrix(char xvalue, char yvalue, char zvalue);
  static std::vector<SKSeitzMatrix> SeitzMatrices(std::string encoding);

  SKRotationMatrix rotation() const {return _rotation;}
  int3 translation() const {return _translation;}
  void setTranslation(int3 t) {_translation = t;}
  int3 normalizedTranslation() const {return int3(_translation.x%12, _translation.y%12, _translation.z%12);}

  double3 operator * (const double3& right) const;
  SKSeitzMatrix operator * (const SKSeitzMatrix &right) const;
  bool operator==(SKSeitzMatrix const& rhs) const;
private:
  SKRotationMatrix _rotation;
  int3 _translation;   // denominator = 12

  static std::vector<SKOneThirdSeitzMatrix> _SeitzData;
};

namespace std
{
  template <> struct hash<SKSeitzMatrix>
  {
    size_t operator()(const SKSeitzMatrix& k) const
    {
      // Compute individual hash values for two data members and combine them using XOR and bit shifting
      int3 normalizedTranslation = k.normalizedTranslation();
      int r1 = (k.rotation().m11+1);
      int r2 = (3 * (k.rotation().m12+1));
      int r3 = ((3*3) * (k.rotation().m13+1));
      int r4 = ((3*3*3) * (k.rotation().m12+1));
      int r5 = ((3*3*3*3) * (k.rotation().m22+1));
      int r6 = ((3*3*3*3*3) * (k.rotation().m32+1));
      int r7 = ((3*3*3*3*3*3) * (k.rotation().m31+1));
      int r8 = ((3*3*3*3*3*3*3) * (k.rotation().m23+1));
      int r9 = ((3*3*3*3*3*3*3*3) * (k.rotation().m33+1));
      int v1 = ((3*3*3*3*3*3*3*3*12) * normalizedTranslation.x);
      int v2 = ((3*3*3*3*3*3*3*3*12*12) * normalizedTranslation.y);
      int v3 = ((3*3*3*3*3*3*3*3*12*12*12) * normalizedTranslation.z);
      return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + v1 + v2 + v3;
    }
  };
}
