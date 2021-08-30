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

#include "float3x4.h"

#include <cmath>

#include <iostream>
#include <ostream>



float3x4::float3x4(const float3x4 &a)  // copy constructor
{
  m11=a.m11; m21=a.m21; m31=a.m31; m41=a.m41;
  m12=a.m12; m22=a.m22; m32=a.m32; m42=a.m42;
  m13=a.m13; m23=a.m23; m33=a.m33; m43=a.m43;
}

float3x4& float3x4::operator=(const float3x4& a)  //assignment operator
{
  if (this != &a) // protect against self-assingment
  {
    m11=a.m11; m21=a.m21; m31=a.m31; m41=a.m41;
    m12=a.m12; m22=a.m22; m32=a.m32; m42=a.m42;
    m13=a.m13; m23=a.m23; m33=a.m33; m43=a.m43;
  }
  return *this;
}
