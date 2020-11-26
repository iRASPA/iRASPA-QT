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

class SKOneThirdSeitzMatrix
{
public:
  SKOneThirdSeitzMatrix(std::string text, uint8_t encoding, int8_t r1, int8_t r2, int8_t r3, int8_t t);
  std::string text() {return _text;}
  uint8_t encoding() {return _encoding;}
  int8_t r1() {return _r1;}
  int8_t r2() {return _r2;}
  int8_t r3() {return _r3;}
  int8_t t() {return _t;}
private:
  std::string _text;
  uint8_t _encoding;
  int8_t _r1;
  int8_t _r2;
  int8_t _r3;
  int8_t _t;
};
