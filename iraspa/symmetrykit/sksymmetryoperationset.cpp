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

#include "sksymmetryoperationset.h"
#include <algorithm>
#include <set>

SKSymmetryOperationSet::SKSymmetryOperationSet()
{

}


SKSymmetryOperationSet::SKSymmetryOperationSet(std::vector<SKSeitzMatrix> operations)
{
  _operations = operations;
/*
  std::unordered_set<SKSeitzMatrix> seen;

  // remove doubles but leave order intact
  auto newEnd = remove_if(_operations.begin(), _operations.end(), [&seen](const SKSeitzMatrix& value)
  {
      if (seen.find(value) != end(seen))
          return true;

      seen.insert(value);
      return false;
  });
  _operations.erase(newEnd, _operations.end());
*/
}

const std::vector<SKRotationMatrix> SKSymmetryOperationSet::rotations() const
{
  std::vector<SKRotationMatrix> rotationMatrices{};

  std::transform(_operations.begin(), _operations.end(), std::back_inserter(rotationMatrices), [](SKSeitzMatrix m){return m.rotation();} );

  return rotationMatrices;
}
