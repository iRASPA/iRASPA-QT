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

#include <iostream>
#include <QtGlobal>

enum class Symmorphicity: qint64
{
  asymmorphic = 0, symmorphic = 1, hemisymmorphic = 2
};

enum class Centring: qint64
{
  none = 0, primitive = 1, body = 2, a_face = 3, b_face = 4, c_face = 5, face = 6, base = 7, r = 8, h = 9, d = 10
};

enum class Holohedry: qint64
{
  none = 0, triclinic = 1, monoclinic = 2, orthorhombic = 3, tetragonal = 4, trigonal = 5, hexagonal = 6, cubic = 7
};
