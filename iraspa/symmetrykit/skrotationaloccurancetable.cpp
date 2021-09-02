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

#include "skrotationaloccurancetable.h"
#include "skrotationmatrix.h"

SKRotationalOccuranceTable::SKRotationalOccuranceTable(int axis_6m, int axis_4m, int axis_3m,
              int axis_2m, int axis_1m, int axis_1,
              int axis_2, int axis_3, int axis_4, int axis_6)
{
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_6m)] = axis_6m;
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_4m)] = axis_4m;
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_3m)] = axis_3m;
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_2m)] = axis_2m;
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_1m)] = axis_1m;
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_1)] = axis_1;
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_2)] = axis_2;
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_3)] = axis_3;
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_4)] = axis_4;
   occurance[static_cast<typename std::underlying_type<SKRotationMatrix::RotationType>::type>(SKRotationMatrix::RotationType::axis_6)] = axis_6;
}

bool operator== (const SKRotationalOccuranceTable& c1, const SKRotationalOccuranceTable& c2)
{
  return (c1.occurance == c2.occurance);
}
