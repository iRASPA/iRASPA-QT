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

#include "float3.h"

float3 float3::normalise()
{
  float magnitude = sqrt((x * x) + (y * y) + (z * z)) ;

  if (magnitude != 0)
  {
    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
  }
  return *this;
}

float3 float3::fract()
{
  float3 s = float3(x,y,z);
  s.x -= rint(x);
  s.y -= rint(y);
  s.z -= rint(z);

  if(s.x<0.0)
  {
    s.x += 1.0;
  }
  if(s.x>1.0)
  {
    s.x -= 1.0;
  }

  if(s.y<0.0)
  {
    s.y += 1.0;
  }
  if(s.y>1.0)
  {
    s.y -= 1.0;
  }

  if(s.z<0.0)
  {
    s.z += 1.0;
  }
  if(s.z>1.0)
  {
    s.z -= 1.0;
  }
  return s;
}

std::ostream& operator<<(std::ostream& out, const float3& vec) // output
{
  out << vec.x;
  out << vec.y;
  out << vec.z;
  return out;
}

