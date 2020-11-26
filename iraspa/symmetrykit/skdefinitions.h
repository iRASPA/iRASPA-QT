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

enum class Symmorphicity: qint64
{
  asymmorphic = 0, symmorphic = 1, hemisymmorphic = 2
};

enum class Centring: qint64
{
  none = 0, primitive = 1, body = 2, a_face = 3, b_face = 4, c_face = 5, face = 6, base = 7, r = 8, h = 9, d = 10
};

enum class RotationType: qint64
{
  axis_6m = -6, axis_4m = -4, axis_3m = -3, axis_2m = -2, axis_1m = -1, none = 0, axis_1 = 1, axis_2 = 2, axis_3 = 3, axis_4 = 4, axis_6 = 6
};

enum class SymmetryType: qint64
{
  unknown = 0, identity = 1, translation = 2, inversion = 3, pure_rotation = 4, pure_reflection = 5, screw_rotation = 6, glide_reflection = 7
};

enum class Holohedry: qint64
{
  none = 0, triclinic = 1, monoclinic = 2, orthorhombic = 3, tetragonal = 4, trigonal = 5, hexagonal = 6, cubic = 7
};

enum class Laue: qint64
{
  none = 0, laue_1 = 1, laue_2m = 2, laue_mmm = 3, laue_4m = 4, laue_4mmm = 5, laue_3 = 6, laue_3m = 7, laue_6m = 8, laue_6mmm = 9, laue_m3 = 10, laue_m3m = 11
};
