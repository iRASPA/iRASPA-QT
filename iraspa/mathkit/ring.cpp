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

#include "ring.h"
#include <cmath>

Ring::Ring()
{

}

int Ring::floorDivision(int a, int b)
{
  return int(floor(double(a) / double(b)));
}

int Ring::modulo(int a, int b)
{
  return a - b * int(floor(double(a) / double(b)));
}

int Ring::greatestCommonDivisor(int arg1, int arg2)
{
  int a = arg1;
  int b = arg2;
  while(b != 0)
  {
    int stored_a = a;
    a = b;
    b = stored_a % b;
  }
  return abs(a);
}

std::tuple<int,int,int> Ring::extendedGreatestCommonDivisor(int a, int b)
{
  int ai = b;   // ai stands for: a with index i
  int aim1 = a; // aim1 stands for: a with index i-1
  int bim1 = 0;
  int cim1 = 0;

  // We can accelerate the first step
  if(ai != 0)
  {
    // compute both quotient and remainder
    int q = aim1 / ai;
    int r = aim1 % ai;

    aim1 = ai;
    ai = r;
    bim1 = 0; // before: bi = 0, bim1 = 1
    int bi = 1;
    cim1 = 1; // before: ci = 1, cim1 = 0
    int ci = -q;
    // Now continue
    while(ai != 0)
    {
      // compute both quotient and remainder
      int q = aim1 / ai;
      int r = aim1 % ai;

      aim1 = ai;
      ai = r;

      int stored_bim1 = bim1;
      bim1 = bi;
      bi = stored_bim1 - q * bi;

      int stored_cim1 = cim1;
      cim1 = ci;
      ci = stored_cim1 - q * ci;
    }
  }
  else
  {
    bim1 = 1;
    cim1 = 0;
  }
  if(aim1 < 0)
  {
    // Make sure that the GCD is non-negative
    aim1 = -aim1;
    bim1 = -bim1;
    cim1 = -cim1;
  }

  return std::make_tuple(aim1, bim1, cim1);
}

std::pair<int,int> Ring::divisionModulo(int a, int b)
{
  //guard (b != 0) else {throw NumericalError.divisionByZero}
  if (b==0)
  {
    return std::make_pair(0,0);
  }
  int temp = int(floor(double(a) / double(b)));
  return std::make_pair(temp, a - b * temp);
}
