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

#include "sksymmetrycell.h"
#include "skrotationmatrix.h"
#include <cmath>
#include <cfloat>
#include <algorithm>

SKSymmetryCell::SKSymmetryCell()
{

}

SKSymmetryCell::SKSymmetryCell(double3x3 metricTensor)
{
  double A = metricTensor[0][0];
  double B = metricTensor[1][1];
  double C = metricTensor[2][2];

  this->_a = sqrt(A);
  this->_b = sqrt(B);
  this->_c = sqrt(C);
  this->_alpha = acos(metricTensor[1][2]/(sqrt(B)*sqrt(C)));
  this->_beta = acos(metricTensor[0][2]/(sqrt(A)*sqrt(C)));
  this->_gamma = acos(metricTensor[0][1]/(sqrt(A)*sqrt(B)));
}

double3x3 SKSymmetryCell::cell()
{
  double temp = (cos(_alpha) - cos(_gamma) * cos(_beta)) / sin(_gamma);

  double3 v1 = double3(_a, 0.0, 0.0);
  double3 v2 = double3(_b * cos(_gamma), _b * sin(_gamma), 0.0);
  double3 v3 = double3(_c * cos(_beta), _c * temp, _c * sqrt(1.0 - cos(_beta)*cos(_beta)-temp*temp));
  return double3x3(v1, v2, v3);
}

double3x3 SKSymmetryCell::metricTensor()
{
  double half_xi = _b*_c*cos(_alpha);
  double half_eta = _a*_c*cos(_beta);
  double half_zeta = _a*_b*cos(_gamma);

  double3 v1 = double3(_a*_a, half_zeta, half_eta);
  double3 v2 = double3(half_zeta, _b*_b, half_xi);
  double3 v3 = double3(half_eta, half_xi, _c*_c);
  return double3x3(v1, v2, v3);
}

double SKSymmetryCell::volume()
{
  double cosAlpha = cos(_alpha);
  double cosBeta = cos(_beta);
  double cosGamma = cos(_gamma);
  double temp = 1.0 - cosAlpha*cosAlpha - cosBeta*cosBeta - cosGamma*cosGamma + 2.0 * cosAlpha * cosBeta * cosGamma;
  return  _a * _b * _c * sqrt(temp);
}

double3x3 SKSymmetryCell::findSmallestPrimitiveCell(std::vector<std::tuple<double3, int, double>> reducedAtoms, std::vector<std::tuple<double3, int, double>> atoms,
                                                    double3x3 unitCell, bool allowPartialOccupancies, double symmetryPrecision = 1e-2)
{
  std::vector<double3> translationVectors{};

  if (reducedAtoms.size()>0)
  {
    double3 origin = std::get<0>(reducedAtoms[0]);

    for(size_t i=1;i<reducedAtoms.size();i++)
    {
      double3 vec = std::get<0>(reducedAtoms[i]) - origin;

      if(SKSymmetryCell::testTranslationalSymmetry(vec, atoms, unitCell, allowPartialOccupancies, symmetryPrecision))
      {
        double3 a = double3(vec.x-rint(vec.x), vec.y-rint(vec.y), vec.z-rint(vec.z));
        if (a.x < 0.0 - 1e-10) {a.x += 1.0;}
        if (a.y < 0.0 - 1e-10) {a.y += 1.0;}
        if (a.z < 0.0 - 1e-10) {a.z += 1.0;}
        translationVectors.push_back(a);
      }
    }

    translationVectors.push_back(double3(1.0,0.0,0.0));
    translationVectors.push_back(double3(0.0,1.0,0.0));
    translationVectors.push_back(double3(0.0,0.0,1.0));
  }

  size_t size = translationVectors.size();

  double3x3 smallestCell = unitCell;
  double initialVolume = unitCell.determinant();
  double minimumVolume = initialVolume;

  for(size_t i=0;i<size;i++)
  {
    for(size_t j=i+1;j<size;j++)
    {
      for(size_t k=j+1;k<size;k++)
      {
        double3 tmpv1 = unitCell * translationVectors[i];
        double3 tmpv2 = unitCell * translationVectors[j];
        double3 tmpv3 = unitCell * translationVectors[k];
        double3x3 cell = double3x3(tmpv1,tmpv2,tmpv3);
        double volume = fabs(cell.determinant());

        if((volume>1.0) && (volume<minimumVolume))
        {
          minimumVolume=volume;
          smallestCell = double3x3(tmpv1,tmpv2,tmpv3);

          // initialVolume/volume is nearly integer
          if(size_t(rint(initialVolume/volume)) == (size - 2))
          {
            double3x3 relativeLattice = double3x3(translationVectors[i],translationVectors[j],translationVectors[k]);

            // inverse is nearly integer
            return unitCell * double3x3(int3x3(relativeLattice.inverse())).inverse();
          }
        }
      }
    }
  }
  return smallestCell;
}

bool SKSymmetryCell::testTranslationalSymmetry(double3 translationVector, std::vector<std::tuple<double3, int, double>> atoms, double3x3 unitCell, bool allowPartialOccupancies, double precision = 1e-2)
{
  double squared_precision = precision * precision;

  for(size_t i=0;i<atoms.size();i++)
  {
    double3 translatedPosition = std::get<0>(atoms[i]) + translationVector;

    bool isFound = false;
    for(size_t j=0;j<atoms.size();j++)
    {
      if(allowPartialOccupancies || std::get<1>(atoms[i]) == std::get<1>(atoms[j]))
      {
        double3 dr = translatedPosition - std::get<0>(atoms[j]);
        dr.x -= rint(dr.x);
        dr.y -= rint(dr.y);
        dr.z -= rint(dr.z);
        if ((unitCell * dr).length_squared() < squared_precision)
        {
          isFound = true;
          break;
        }
      }
    }

    // if no overlap is found then we can immediately return 'false'
    if(!isFound)
    {
      return false;
    }
  }
  return true;
}

bool SKSymmetryCell::testSymmetry(double3 translationVector, SKRotationMatrix rotationMatrix, std::vector<std::tuple<double3, int, double>> atoms, double3x3 unitCell, bool allowPartialOccupancies, double precision = 1e-2)
{
  for(size_t i=0;i<atoms.size();i++)
  {
    double3 rotatedAndTranslatedPosition = rotationMatrix * std::get<0>(atoms[i]) + translationVector;

    bool isFound = false;
    for(size_t j=0;j<atoms.size();j++)
    {
      if(allowPartialOccupancies || std::get<1>(atoms[i]) == std::get<1>(atoms[j]))
      {
        double3 dr = rotatedAndTranslatedPosition - std::get<0>(atoms[j]);
        dr.x -= rint(dr.x);
        dr.y -= rint(dr.y);
        dr.z -= rint(dr.z);
        if ((unitCell * dr).length_squared() < precision * precision)
        {
          isFound = true;
          break;
        }
      }
    }

    // if no overlap is found then we can immediately return 'false'
    if(!isFound)
    {
      return false;
    }
  }
  return true;
}


  /// Computes translation vectors for symmetry operations
  ///
  /// - parameter unitCell:            the unit cell
  /// - parameter fractionalPositions: the fractional positions of the atomic configuration
  /// - parameter rotationMatrix:      the symmetry elements
  /// - parameter symmetryPrecision:   the precision of the search (default: 1e-2)
  ///
  /// - returns: the list of translation vectors, including (0,0,0)
std::vector<double3> SKSymmetryCell::primitiveTranslationVectors(double3x3 unitCell, std::vector<std::tuple<double3, int, double>> reducedAtoms, std::vector<std::tuple<double3, int, double>> atoms, SKRotationMatrix rotationMatrix, bool allowPartialOccupancies, double symmetryPrecision = 1e-2)
{
    std::vector<double3> translationVectors{};

  if(reducedAtoms.size()>0)
  {
    double3 origin = rotationMatrix * std::get<0>(reducedAtoms[0]);

    for(size_t i=0;i<reducedAtoms.size();i++)
    {
      double3 vec = std::get<0>(reducedAtoms[i]) - origin;

      if(SKSymmetryCell::testSymmetry(vec, rotationMatrix, atoms, unitCell, allowPartialOccupancies, symmetryPrecision))
      {
        translationVectors.push_back(vec);
      }
    }
  }
  return translationVectors;
}

  /// Compute the Delaunay reduced cell
  ///
  /// - parameter unitCell:          the original unit cell
  /// - parameter symmetryPrecision: the precision of the cell
  ///
  /// - returns: the Delaunay cell
  ///
  ///   We start with a lattice basis (b_i) 1≤ i ≤ n (n=2,3). This basis is extended by a factor b_n+1 = -(b_1 + ... + b_n )
  ///   All scalar products  b_i . b_k (1 ≤ i < k ≤ n+1) are considered. The reduction is performed by mnimizing the sum: sum = b_1^2 + ... + b_n+1^2.
  ///   It can be shown that this sum can be reduced as long as one of the scalar products is still positive.
  ///   If e.g. the scalar product b_1 . b_2 is still positive, a transformation can be performed such that the sum sum' of the transformed b_i^2 is smaller than sum:
  ///   b_1' = -b_1
  ///   b_2' = b_2
  ///   b_3' = b_1 + b_3
  ///   b_4' = b+1 + b_4
  ///
  ///   If all the scalar products are less than or equal to zero, the three shortest vectors forming the reduced basis are contained in the set
  ///   V = {b_1, b_2, b_3, b_4, b_1 + b_2, b_2 + b_3, b_3 + b_1}
  ///   which corresponds to the maximal set of faces of the Dirichlet domain (14 faces).
  ///
  ///   Reference: International Tables for Crystallography, Vol.A: Space Group Symmetry, page 747
  std::optional<double3x3> SKSymmetryCell::computeDelaunayReducedCell(double3x3 unitCell, double symmetryPrecision = 1e-2)
  {
    double3 additionalBasisVector =  -(unitCell[0] + unitCell[1] + unitCell[2]);
    double4x3 extendedBasis = double4x3(unitCell[0],unitCell[1],unitCell[2],additionalBasisVector);

    bool somePositive = false;
    do
    {
      somePositive = false;
      // (i,j) in (0,1), (0,2), (0,3), (1,2), (1,3), (2,3); k,l denote the other two vectors
      std::vector<std::tuple<int, int, int, int>> basisIndices = {std::make_tuple(0,1,2,3), std::make_tuple(0,2,1,3), std::make_tuple(0,3,1,2), std::make_tuple(1,2,0,3), std::make_tuple(1,3,0,2), std::make_tuple(2,3,0,1)};
      for(auto &[i,j,k,l] : basisIndices)
      {
        if (double3::dot(extendedBasis[i], extendedBasis[j]) > symmetryPrecision)
        {
          extendedBasis[k] += extendedBasis[i];
          extendedBasis[l] += extendedBasis[i];

          extendedBasis[i] = -extendedBasis[i];

          // start over (until all dotproducts are negative or zero)
          somePositive = true;
          break;
        }
      }
    }while(somePositive);

    // Search in the array {b1, b2, b3, b4, b1+b2, b2+b3, b3+b1}, sorted by length (using a small epsilon to amke sure they are really different)
    std::vector<double3> b = {extendedBasis[0], extendedBasis[1], extendedBasis[2], extendedBasis[3],
                              extendedBasis[0] + extendedBasis[1], extendedBasis[1] + extendedBasis[2],
                              extendedBasis[2] + extendedBasis[0]};

    sort(b.begin(), b.end(),
        [](const double3 & a, const double3 & b) -> bool
    {
        return a.length_squared() + 1e-10 < b.length_squared();
    });

    // take the first two vectors, combined with a vector that has a non-zero, positive volume
    for(int i=2;i<7;i++)
    {
      double3x3 trialUnitCell = double3x3(b[0], b[1], b[i]);
      double volume = trialUnitCell.determinant();

      if (abs(volume) > symmetryPrecision)
      {
        return (volume > 0) ? trialUnitCell: -trialUnitCell;
      }
    }

    return std::nullopt;
  }

  // Determining the lattice symmetry is equivalent to determining the Bravais type.
SKPointSymmetrySet SKSymmetryCell::findLatticeSymmetry(double3x3 reducedLattice, double symmetryPrecision = 3.0)
{
  const std::vector<int3> latticeAxes = {
     int3( 1, 0, 0),
     int3( 0, 1, 0),
     int3( 0, 0, 1),
     int3(-1, 0, 0),
     int3( 0,-1, 0),
     int3( 0, 0,-1),
     int3( 0, 1, 1),
     int3( 1, 0, 1),
     int3( 1, 1, 0),
     int3( 0,-1,-1),
     int3(-1, 0,-1),
     int3(-1,-1, 0),
     int3( 0, 1,-1),
     int3(-1, 0, 1),
     int3( 1,-1, 0),
     int3( 0,-1, 1),
     int3( 1, 0,-1),
     int3(-1, 1, 0),
     int3( 1, 1, 1),
     int3(-1,-1,-1),
     int3(-1, 1, 1),
     int3( 1,-1, 1),
     int3( 1, 1,-1),
     int3( 1,-1,-1),
     int3(-1, 1,-1),
     int3(-1,-1, 1)
    };

    std::vector<SKRotationMatrix> pointSymmetries{};

    double3x3 latticeMetricMatrix = reducedLattice.transpose() * reducedLattice;

    // uses a stored list of all possible lattice vectors and loop over all possible permutations
    for(const int3 &firstAxis: latticeAxes)
    {
      for(const int3 &secondAxis: latticeAxes)
      {
        for(const int3 &thirdAxis: latticeAxes)
        {
          SKRotationMatrix axes = SKRotationMatrix(firstAxis, secondAxis, thirdAxis);
          int determinant = axes.int3x3.determinant();

          // if the determinant is 1 or -1 we have a (proper) rotation  (6960 proper rotations)
          if (determinant == 1 || determinant == -1)
          {
            double3x3 transformationMatrix = double3x3(axes.int3x3);

            // the inverse of a rotation matrix is its transpose, so we use the transpose here
            double3x3 newLattice = reducedLattice * transformationMatrix;
            double3x3 transformedLatticeMetricMatrix = newLattice.transpose() * newLattice;

            if (SKSymmetryCell::checkMetricSimilarity(transformedLatticeMetricMatrix, latticeMetricMatrix, symmetryPrecision))
            {
              pointSymmetries.push_back(axes);
            }
          }
        }
      }
    }

    double3x3 transform = (reducedLattice.inverse() * reducedLattice);
    std::vector<SKRotationMatrix> newpointSymmetries{};
    for(const SKRotationMatrix & pointSymmetry: pointSymmetries)
    {
      SKRotationMatrix mat = SKRotationMatrix(transform.inverse() * double3x3(pointSymmetry.int3x3) * transform);

      // avoid duplicate rotation matrices
      if(!(std::find(newpointSymmetries.begin(),newpointSymmetries.end(), mat) != newpointSymmetries.end()))
      {
        newpointSymmetries.push_back(mat);
      }
    }

    return SKPointSymmetrySet(newpointSymmetries);
}

bool SKSymmetryCell::checkMetricSimilarity(double3x3 transformedMetricTensor, double3x3 metricTensor, double symmetryPrecision = 1e-2)
{
  SKSymmetryCell metricCell = SKSymmetryCell(metricTensor);
  SKSymmetryCell transformedMetricCell = SKSymmetryCell(transformedMetricTensor);

  double3 lengthDifference = double3(abs(metricCell._a - transformedMetricCell._a),
                                     abs(metricCell._b - transformedMetricCell._b),
                                     abs(metricCell._c - transformedMetricCell._c));
  double3 angleDifference = double3(sin(abs(metricCell._alpha - transformedMetricCell._alpha)),
                                    sin(abs(metricCell._beta - transformedMetricCell._beta)),
                                    sin(abs(metricCell._gamma - transformedMetricCell._gamma)));

  // check on lengths
  if(lengthDifference.length() > symmetryPrecision)
  {
    return false;
  }

  // check on angles
  if(abs(std::max({angleDifference.x,angleDifference.y,angleDifference.z})) > symmetryPrecision)
  {
    return false;
  }

  return true;
}

std::vector<std::tuple<double3, int, double>> SKSymmetryCell::trim(std::vector<std::tuple<double3, int, double>> atoms, double3x3 from, double3x3 to, bool allowPartialOccupancies, double symmetryPrecision = 1e-2)
{
  double3x3 changeOfBasis = to.inverse() * from;

  std::vector<std::tuple<double3, int, double>> trimmedAtoms{};
  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(trimmedAtoms),
                   [changeOfBasis](const std::tuple<double3, int, double> &atom) -> std::tuple<double3, int, double>
                       {return std::make_tuple(double3::fract(changeOfBasis * std::get<0>(atom)), std::get<1>(atom),std::get<2>(atom));});

  std::vector<size_t> overlapTable = std::vector<size_t>(trimmedAtoms.size());
  std::fill(overlapTable.begin(), overlapTable.end(), -1);

  std::vector<std::tuple<double3, int, double>> result{};

  for(size_t i=0;i<trimmedAtoms.size();i++)
  {
    overlapTable[i] = i;
    for(size_t j=0;j<trimmedAtoms.size();j++)
    {
      if(std::get<1>(trimmedAtoms[i]) == std::get<1>(trimmedAtoms[j]))
      {
        double3 dr = double3::abs(std::get<0>(trimmedAtoms[i]) - std::get<0>(trimmedAtoms[j]));
        dr.x -= rint(dr.x);
        dr.y -= rint(dr.y);
        dr.z -= rint(dr.z);
        if ((to * dr).length_squared() < symmetryPrecision * symmetryPrecision)
        {
          if(overlapTable[j] == j)
          {
            overlapTable[i] = j;
            break;
          }
        }
      }
    }
  }

  for(size_t i=0;i<trimmedAtoms.size();i++)
  {
    if(overlapTable[i] == i)
    {
      result.push_back(trimmedAtoms[i]);
    }
  }

  return result;
}
