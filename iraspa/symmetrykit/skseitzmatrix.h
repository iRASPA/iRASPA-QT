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
  SKSeitzMatrix(SKRotationMatrix rotation, double3 translation);
  const SKRotationMatrix &rotation() const {return _rotation;}
  const double3 &translation() const {return _translation;}
private:
  SKRotationMatrix _rotation;
  double3 _translation;

  friend bool operator==(const SKSeitzMatrix& lhs, const SKSeitzMatrix& rhs);
};

/*
namespace std
{
  template <> struct hash<SKSeitzMatrix>
  {
    size_t operator()(const SKSeitzMatrix& k) const
    {
      std::size_t h=0;
      double3 translation = double3::fract(k.translation());
      hash_combine(h, k.rotation(), int(translation.x*100), int(translation.y*100), int(translation.z*100));
      return h;
    }
  };
}*/
