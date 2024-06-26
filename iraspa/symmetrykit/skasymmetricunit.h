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

#include <tuple>
#include "mathkit.h"

struct SKAsymmetricUnit
{
  std::pair<int,int> a;
  std::pair<int,int> b;
  std::pair<int,int> c;

  SKAsymmetricUnit();
  SKAsymmetricUnit(std::pair<int,int>  p1, std::pair<int,int> p2, std::pair<int,int> p3): a(p1), b(p2), c(p3) {};

  SKAsymmetricUnit(const SKAsymmetricUnit& t)
  {
    this->a = t.a;
    this->b = t.b;
    this->c = t.c;
  }

  SKAsymmetricUnit& operator=(const SKAsymmetricUnit &other)
  {
    this->a = other.a;
    this->b = other.b;
    this->c = other.c;
    return *this;
  }

  bool contains(double3 point);
  bool isInsideRange(double point, double leftBoundary, int equalLeft, double rightBoundary, int equalRight);
};

