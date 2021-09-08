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

#include "skintegersymmetryoperationset.h"
#include "sksymmetrycell.h"
#include "skspacegroup.h"

SKIntegerSymmetryOperationSet::SKIntegerSymmetryOperationSet()
{

}

SKIntegerSymmetryOperationSet::SKIntegerSymmetryOperationSet(std::unordered_set<SKSeitzIntegerMatrix> operations)
{
  this->operations = operations;
  this->centring = Centring::primitive;
}

SKIntegerSymmetryOperationSet::SKIntegerSymmetryOperationSet(std::vector<SKSeitzIntegerMatrix> operations)
{
  this->operations = std::unordered_set<SKSeitzIntegerMatrix>(operations.begin(),operations.end());
}

SKIntegerSymmetryOperationSet SKIntegerSymmetryOperationSet::fullSeitzMatrices()
{
  return SKIntegerSymmetryOperationSet();
}

std::vector<std::tuple<double3, int, double>>  SKIntegerSymmetryOperationSet::symmetrize(double3x3 lattice, std::vector<std::tuple<double3, int, double>> atoms, double symmetryPrecision = 1e-2)
{
  std::vector<std::tuple<double3, int, double>> symmetrizedAtoms{};
  symmetrizedAtoms.reserve(atoms.size());

  for(size_t i=0;i<atoms.size();i++)
  {
    SKRotationMatrix averageRotation = SKRotationMatrix::zero;
    double3 averageTranslation = double3(0.0,0.0,0.0);
    int count = 0;

    for(const SKSeitzIntegerMatrix& operation: this->operations)
    {
      double3 translation = double3(double(operation.translation.x)/24.0, double(operation.translation.y)/24.0, double(operation.translation.z)/24.0);
      double3 position = operation.rotation * std::get<0>(atoms[i]) + translation;

      if(SKSymmetryCell::isOverlap(position, std::get<0>(atoms[i]), lattice, symmetryPrecision))
      {
        averageRotation = averageRotation + operation.rotation;
        averageTranslation += translation - double3::rint(position - std::get<0>(atoms[i]));
        count = count + 1;
      }
    }

    double3x3 averagedRotation = double3x3(averageRotation.int3x3) / double(count);
    double3 averagedTranslation = double3(double(averageTranslation.x), double(averageTranslation.y), double(averageTranslation.z)) / double(count);

    std::tuple<double3, int, double> symmetrizedAtom = std::make_tuple(averagedRotation * std::get<0>(atoms[i]) + averagedTranslation, std::get<1>(atoms[i]), std::get<2>(atoms[i]));
    symmetrizedAtoms.push_back(symmetrizedAtom);
  }

  return symmetrizedAtoms;
}

std::vector<std::tuple<double3, int, double>> SKIntegerSymmetryOperationSet::asymmetricAtoms(int HallNumber, std::vector<std::tuple<double3, int, double>> &atoms, double3x3 lattice, bool allowPartialOccupancies, double symmetryPrecision = 1e-2)
{
  std::vector<std::tuple<double3,int,double,int>> atomData{};
  std::transform(atoms.begin(), atoms.end(), std::back_inserter(atomData),
            [](const std::tuple<double3, int, double> &atom) {return std::make_tuple(std::get<0>(atom),std::get<1>(atom),std::get<2>(atom),-1);});

  if(atoms.empty()) return {};

  std::vector<std::tuple<double3, int, double>> asymmetricAtoms = {};
  //std::get<3>(atomData[0]) = 0;

  // loop over all atoms
  loop: for(size_t i=0;i<atoms.size();i++)
  {
    // skip if already tagged
    if(std::get<3>(atomData[i]) == -1)
    {
      // loop over all current asymmetric atoms, and see if one of the symmetry-copies matches with an asymmetric atom
      for(size_t j=0;j<asymmetricAtoms.size();j++)
      {
        if(std::get<1>(atomData[i]) == std::get<1>(asymmetricAtoms[j]))
        {
          for(const SKSeitzIntegerMatrix& operation: this->operations)
          {
            double3 position = operation.rotation * std::get<0>(atomData[i]) + double3(double(operation.translation.x)/24.0, double(operation.translation.y)/24.0, double(operation.translation.z)/24.0);
            if(SKSymmetryCell::isOverlap(position, std::get<0>(asymmetricAtoms[j]), lattice, symmetryPrecision))
            {
              // overlap and of the same type: the atom is therefore a copy of the asymmetric atom 'j'
              std::get<3>(atomData[i]) = j;
              break;
            }
          }
        }
      }

      // not typed yet
      if(std::get<3>(atomData[i]) == -1)
      {
        asymmetricAtoms.push_back(std::make_tuple(std::get<0>(atomData[i]), std::get<1>(atomData[i]), std::get<2>(atomData[i])));
        std::get<3>(atomData[i]) = asymmetricAtoms.size() - 1;
        goto loop;
      }
    }
  }


  for(size_t i=0;i<asymmetricAtoms.size();i++)
  {
    bool found = false;
    for(const SKSeitzIntegerMatrix& operation: operations)
    {
      double3 position = operation.rotation * std::get<0>(asymmetricAtoms[i]) + double3(double(operation.translation.x)/24.0, double(operation.translation.y)/24.0, double(operation.translation.z)/24.0);

      // if directly inside the asymmetric unit cell, overwrite the position and break
      if(SKSpaceGroup(HallNumber).spaceGroupSetting().asymmetricUnit().contains(position))
      {
        std::get<0>(asymmetricAtoms[i]) = double3::fract(position);
        found = true;
        break;
      }
    }

    // if directly inside the asymmetric unit cell including a small epsilon, overwrite the position and break
    if(!found)
    {
      for(const SKSeitzIntegerMatrix& operation: operations)
      {
        double3 position = operation.rotation * std::get<0>(asymmetricAtoms[i]) + double3(double(operation.translation.x)/24.0, double(operation.translation.y)/24.0, double(operation.translation.z)/24.0);

        //let spaceGroupNumber = SKSpacegroup.spaceGroupData[HallNumber].spaceGroupNumber
        //if SKAsymmetricUnit.isInsideIUCAsymmetricUnitCell(number: spaceGroupNumber, point: position, precision: symmetryPrecision)
        if(SKSpaceGroup(HallNumber).spaceGroupSetting().asymmetricUnit().contains(position))
        {
          std::get<0>(asymmetricAtoms[i]) = double3::fract(position);
          break;
        }
      }
    }
  }

  return asymmetricAtoms;
}
