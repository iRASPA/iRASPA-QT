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
#include <utility>
#include "skpointgroup.h"
#include "skspacegroup.h"
#include <assert.h>

SKSymmetryCell::SKSymmetryCell()
{
}

SKSymmetryCell::SKSymmetryCell(double a, double b, double c, double alpha, double beta, double gamma)
{
  _a = a;
  _b = b;
  _c = c;
  _alpha = alpha * M_PI/180.0;
  _beta = beta * M_PI/180.0;
  _gamma = gamma * M_PI/180.0;
}

SKSymmetryCell SKSymmetryCell::createFromMetricTensor(double3x3 metricTensor)
{
  double A = metricTensor[0][0];
  double B = metricTensor[1][1];
  double C = metricTensor[2][2];

  SKSymmetryCell cell;
  cell._a = sqrt(A);
  cell._b = sqrt(B);
  cell._c = sqrt(C);
  cell._alpha = acos(metricTensor[1][2]/(sqrt(B)*sqrt(C)));
  cell._beta = acos(metricTensor[0][2]/(sqrt(A)*sqrt(C)));
  cell._gamma = acos(metricTensor[0][1]/(sqrt(A)*sqrt(B)));
  return cell;
}

SKSymmetryCell SKSymmetryCell::createFromUnitCell(double3x3 unitCell)
{
  double3 column1 = unitCell[0];
  double3 column2 = unitCell[1];
  double3 column3 = unitCell[2];
  double length1 = column1.length();
  double length2 = column2.length();
  double length3 = column3.length();

  SKSymmetryCell cell;
  cell._a = length1;
  cell._b = length2;
  cell._c = length3;
  cell._alpha = acos(double3::dot(column2, column3) / (length2 * length3));
  cell._beta = acos(double3::dot(column1, column3) / (length1 * length3));
  cell._gamma = acos(double3::dot(column1, column2) / (length1 * length2));
  return cell;
}

SKSymmetryCell SKSymmetryCell::idealized(int pointGroupNumber, QString qualifier)
{
  Holohedry holohedry = SKPointGroup::pointGroupData[pointGroupNumber].holohedry();
  switch(holohedry)
  {
  case Holohedry::none:
    return *this;
  case Holohedry::triclinic:
  {
    double cg = cos(_gamma);
    double cb = cos(_beta);
    double ca = cos(_alpha);
    double sg = sin(_gamma);
    double temp = _c * sqrt(1.0 - ca * ca - cb * cb - cg * cg + 2.0 * ca * cb * cg) / sg;
    return SKSymmetryCell::createFromUnitCell(double3x3(double3(_a,0.0,0.0),double3(_b * cg, _b * sg,0.0),double3(_c * cb, _c * (ca - cb * cg) / sg, temp)));
  }
  case Holohedry::monoclinic:
    return SKSymmetryCell::createFromUnitCell(double3x3(double3(_a,0.0,0.0),double3(0.0,_b,0.0),double3(_c * cos(_beta),0.0, _c * sin(_beta))));
  case Holohedry::orthorhombic:
    return SKSymmetryCell::createFromUnitCell(double3x3(double3(_a,0.0,0.0),double3(0.0,_b,0.0),double3(0.0,0.0,_c)));
  case Holohedry::tetragonal:
    return SKSymmetryCell::createFromUnitCell(double3x3(double3(0.5*(_a+_b),0.0,0.0),double3(0.0,0.5*(_a+_b),0.0),double3(0.0,0.0,_c)));
  case Holohedry::trigonal:
    if(qualifier == "R")
    {
      double avg = (_a+_b+_c)/3.0;
      double angle = acos((cos(_gamma) + cos(_beta) + cos(_alpha)) / 3.0);
      // Reference, https://homepage.univie.ac.at/michael.leitner/lattice/struk/rgr.html
      double ahex = 2.0 * avg * sin(0.5 * angle);
      double chex = (_a+_b+_c)/3.0 * sqrt(3.0 * (1.0 + 2.0 * cos(angle)));
      return  SKSymmetryCell::createFromUnitCell(double3x3(double3(ahex / 2.0,-ahex / (2.0 * sqrt(3.0)),chex / 3.0),double3(0.0,ahex / sqrt(3.0),chex / 3.0),double3(-ahex / 2.0,-ahex / (2.0 * sqrt(3.0)),chex / 3.0)));
    }
    return SKSymmetryCell::createFromUnitCell(double3x3(double3(0.5*(_a+_b),0.0,0.0),double3(-(_a+_b)/4.0,(_a+_b)/4.0*sqrt(3.0),0.0),double3(0.0,0.0,_c)));
    case Holohedry::hexagonal:
    return SKSymmetryCell::createFromUnitCell(double3x3(double3(0.5*(_a+_b),0.0,0.0),double3(-(_a+_b)/4.0,(_a+_b)/4.0*sqrt(3.0),0.0),double3(0.0,0.0,_c)));
  case Holohedry::cubic:
  {
    double edge = (_a + _b + _c)/3.0;
    return SKSymmetryCell::createFromUnitCell(double3x3(double3(edge,0.0,0.0),double3(0.0,edge,0.0),double3(0.0,0.0,edge)));
  }
  default:
    return SKSymmetryCell::createFromUnitCell(double3x3());
  }
}

double3x3 SKSymmetryCell::unitCell() const
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

bool SKSymmetryCell::isOverlap(double3 a, double3 b, double3x3 lattice, double symmetryPrecision = 1e-2)
{
  double3 dr = double3::abs(a - b);
  dr.x -= std::rint(dr.x);
  dr.y -= std::rint(dr.y);
  dr.z -= std::rint(dr.z);
  if((lattice * dr).length_squared() < symmetryPrecision * symmetryPrecision)
  {
    return true;
  }
  return false;
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
            double3x3 temp = relativeLattice.inverse();
            return unitCell * double3x3(temp.toInt3x3()).inverse();
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

std::optional<double3x3> SKSymmetryCell::computeDelaunayReducedCell2D(double3x3 unitCell, double symmetryPrecision = 1e-2)
{
  double3x3 lattice2D = double3x3();
  lattice2D[0] = unitCell[0];
  lattice2D[1] = unitCell[2];

  double3x3 extendedBasis = double3x3(lattice2D[0], lattice2D[1], -(lattice2D[0] + lattice2D[1]));

  bool somePositive = false;
  do
  {
    somePositive = false;
    // (i,j) in (0,1), (0,2), (1,2); k denote the other two vectors
    std::vector<std::tuple<int, int, int>> basisIndices = {std::make_tuple(0,1,2), std::make_tuple(0,2,1), std::make_tuple(1,2,0)};
    for(auto &[i,j,k] : basisIndices)
    {
      if (double3::dot(extendedBasis[i], extendedBasis[j]) > symmetryPrecision)
      {
        extendedBasis[k] += 2.0 * extendedBasis[i];
        extendedBasis[i] = -extendedBasis[i];

        // start over (until all dotproducts are negative or zero)
        somePositive = true;
        break;
      }
    }
  }while(somePositive);

  // Search in the set {b1, b2, b3, b1+b2}
  std::vector<double3> b = {extendedBasis[0], extendedBasis[1], extendedBasis[2], extendedBasis[0]+extendedBasis[1]};
  sort(b.begin(), b.end(),
      [](const double3 & a, const double3 & b) -> bool
  {
      return a.length_squared() + 1e-10 < b.length_squared();
  });

  for(int i=1;i<4;i++)
  {
    double3x3 tmpmat = double3x3(b[0], unitCell[1], b[i]);

    if (fabs(tmpmat.determinant()) > symmetryPrecision)
    {
      extendedBasis[0] = b[0];
      extendedBasis[1] = b[i];
      break;
    }
  }

  double3x3 basis = double3x3();
  basis[0] = extendedBasis[0];
  basis[1] = unitCell[1];
  basis[2] = extendedBasis[1];

  double volume = basis.determinant();

  if (fabs(volume) < symmetryPrecision)
  {
    return std::nullopt;
  }

  if (volume < 0.0)
  {
    basis[1] = -basis[1];
  }

  return basis;
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
          int determinant = axes.determinant();

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
      double3x3 temp = transform.inverse() * double3x3(pointSymmetry.int3x3) * transform;
      SKRotationMatrix mat = SKRotationMatrix(temp.toInt3x3());

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
  SKSymmetryCell metricCell = SKSymmetryCell::createFromMetricTensor(metricTensor);
  SKSymmetryCell transformedMetricCell = SKSymmetryCell::createFromMetricTensor(transformedMetricTensor);

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

const double epsilon = 1.0e-5;

bool SKSymmetryCell::isSmallerThen(double x, double y)
{
  return x < (y - epsilon);
}

bool SKSymmetryCell::isLargerThen(double x, double y)
{
  return isSmallerThen(y, x);
}

bool SKSymmetryCell::isSmallerEqualThen(double x, double y)
{
  return !(y < (x - epsilon));
}

bool SKSymmetryCell::isLargerEqualThen(double x, double y)
{
  return !(x < (y - epsilon));
}

bool  SKSymmetryCell::isEqualTo(double x, double y)
{
  return !((x < (y - epsilon)) || (y < (x - epsilon)));
}

bool SKSymmetryCell::isLargerThenZeroXiEtaZeta(double xi, double eta, double zeta)
{
  int n_positive = 0;
  int n_zero = 0;

  if(isSmallerThen(0, xi)) {n_positive += 1;}
  else if(!isSmallerThen(xi, 0.0)) {n_zero+=1;}

  if(isSmallerThen(0.0, eta)) {n_positive += 1;}
  else if(!isSmallerThen(eta, 0.0)) {n_zero+=1;}

  if(isSmallerThen(0.0, zeta)) {n_positive += 1;}
  else if(!isSmallerThen(zeta, 0.0)) {n_zero+=1;}

  return ((n_positive == 3) || (n_zero == 0 && n_positive == 1));
}

template <typename T> int sign(T val)
{
  return (T(0) < val) - (val < T(0));
}

/// Ref: I. Krivy, B. Gruber,  "A Unified Algorithm for Determining the Reduced (Niggli) Cell",  Acta Cryst. (1976). A32, 297-298
  ///    R. W. Grosse-Kunstleve, N. K. Sauter and P. D. Adams, "Numerically stable algorithms for the computation of reduced unit cells", Acta Cryst. (2004). A60, 1-6
std::optional<std::pair<SKSymmetryCell, SKTransformationMatrix >> SKSymmetryCell::computeReducedNiggliCellAndChangeOfBasisMatrix()
{
  int counter = 0;

  // step 0:
  double A = (_a*_a);
  double B = (_b*_b);
  double C = (_c*_c);
  double xi = (2.0*_b*_c*cos(_alpha));
  double eta = (2.0*_a*_c*cos(_beta));
  double zeta = (2.0*_a*_b*cos(_gamma));

  SKTransformationMatrix changeOfBasisMatrix = SKTransformationMatrix::identity;

  algorithmStart:
  {
    counter += 1;
    if(counter>10000) {return std::nullopt;}

    // step 1
    if(SKSymmetryCell::isLargerThen(A, B)||(SKSymmetryCell::isEqualTo(A, B)&&(SKSymmetryCell::isLargerThen(abs(xi), abs(eta)))))
    {
      SKTransformationMatrix matrixC = SKTransformationMatrix(int3(0,-1,0),int3(-1,0,0),int3(0,0,-1));
      assert(matrixC.determinant() == 1);
      changeOfBasisMatrix = changeOfBasisMatrix * matrixC;

      // Swap x, y and ensures proper sign of determinant
      std::swap(A,B);
      std::swap(xi,eta);
    }

    // step 2
    if(SKSymmetryCell::isLargerThen(B,C)||(SKSymmetryCell::isEqualTo(B, C)&&(SKSymmetryCell::isLargerThen(abs(eta), abs(zeta)))))
    {
      SKTransformationMatrix matrixC = SKTransformationMatrix(int3(-1,0,0),int3(0,0,-1),int3(0,-1,0));
      assert(matrixC.determinant() == 1);
      changeOfBasisMatrix = changeOfBasisMatrix * matrixC;

      // Swap y, z and ensures proper sign of determinant
      std::swap(B,C);
      std::swap(eta,zeta);

      goto algorithmStart;
    }

    // step 3
    if(SKSymmetryCell::isLargerThenZeroXiEtaZeta(xi, eta, zeta))
    {
      std::array<int,3> f = {1,1,1};
      if (SKSymmetryCell::isSmallerThen(xi, 0.0)) {f[0] = -1;}
      if (SKSymmetryCell::isSmallerThen(eta, 0.0)) {f[1] = -1;}
      if (SKSymmetryCell::isSmallerThen(zeta, 0.0)) {f[2] = -1;}
      SKTransformationMatrix matrixC = SKTransformationMatrix(int3(f[0],0,0),int3(0,f[1],0),int3(0,0,f[2]));
      assert(matrixC.determinant() == 1);
      changeOfBasisMatrix = changeOfBasisMatrix * matrixC;

      xi = abs(xi);
      eta = abs(eta);
      zeta = abs(zeta);
    }
    else // step 4:
    {
      int p = -1;
      std::array<int,3> f = {1,1,1};
      if (SKSymmetryCell::isLargerThen(xi, 0.0)) {f[0] = -1;}
      else if (!SKSymmetryCell::isSmallerThen(xi, 0.0)) {p=0;}
      if (SKSymmetryCell::isLargerThen(eta, 0.0)) {f[1] = -1;}
      else if (!SKSymmetryCell::isSmallerThen(eta, 0.0)) {p=1;}
      if (SKSymmetryCell::isLargerThen(zeta, 0.0)) {f[2] = -1;}
      else if (!SKSymmetryCell::isSmallerThen(zeta, 0.0)) {p=2;}
      if (f[0]*f[1]*f[2] < 0)
      {
        f[p] = -1;
      }
      SKTransformationMatrix matrixC = SKTransformationMatrix(int3(f[0],0,0),int3(0,f[1],0),int3(0,0,f[2]));
      assert(matrixC.determinant() == 1);
      changeOfBasisMatrix = changeOfBasisMatrix * matrixC;

      xi = -abs(xi);
      eta = -abs(eta);
      zeta = -abs(zeta);
    }

    // step 5
    if((SKSymmetryCell::isLargerThen(abs(xi), B))||(SKSymmetryCell::isEqualTo(xi, B)&&SKSymmetryCell::isSmallerThen(2.0*eta, zeta))||
       (SKSymmetryCell::isEqualTo(xi, -B)&&SKSymmetryCell::isSmallerThen(zeta, 0.0)))
    {
      SKTransformationMatrix matrixC = SKTransformationMatrix(int3(1,0,0),int3(0,1,0),int3(0,-int(sign(xi)),1));
      assert(matrixC.determinant() == 1);
      changeOfBasisMatrix = changeOfBasisMatrix * matrixC;

      C = B + C - xi * sign(xi);
      eta = eta - zeta * sign(xi);
      xi = xi -  2.0 * B * sign(xi);

      goto algorithmStart;
    }

    // step 6
    if((SKSymmetryCell::isLargerThen(abs(eta), A))||(SKSymmetryCell::isEqualTo(eta, A)&&SKSymmetryCell::isSmallerThen(2.0*xi, zeta))||
            (SKSymmetryCell::isEqualTo(eta, -A)&&SKSymmetryCell::isSmallerThen(zeta, 0.0)))
    {
      SKTransformationMatrix matrixC = SKTransformationMatrix(int3(1,0,0),int3(0,1,0),int3(-int(sign(eta)),0,1));
      assert(matrixC.determinant() == 1);
      changeOfBasisMatrix = changeOfBasisMatrix * matrixC;

      C = A + C - eta * sign(eta);
      xi = xi - zeta * sign(eta);
      eta = eta - 2.0*A * sign(eta);

      goto algorithmStart;
    }

    // step7
    if((SKSymmetryCell::isLargerThen(abs(zeta), A))||(SKSymmetryCell::isEqualTo(zeta, A)&&SKSymmetryCell::isSmallerThen(2.0*xi, eta))||
            (SKSymmetryCell::isEqualTo(zeta, -A)&&SKSymmetryCell::isSmallerThen(eta, 0.0)))
    {
      SKTransformationMatrix matrixC = SKTransformationMatrix(int3(1,0,0),int3(-int(sign(zeta)),1,0),int3(0,0,1));
      assert(matrixC.determinant() == 1);
      changeOfBasisMatrix = changeOfBasisMatrix * matrixC;

      B = A + B - zeta * sign(zeta);
      xi = xi - eta * sign(zeta);
      zeta = zeta - 2.0*A * sign(zeta);

      goto algorithmStart;
    }

    // step 8
    if(SKSymmetryCell::isSmallerThen(xi+eta+zeta+A+B, 0.0)||(SKSymmetryCell::isEqualTo(xi+eta+zeta+A+B, 0.0)&&SKSymmetryCell::isLargerThen(2.0*(A+eta)+zeta, 0.0)))
    {
      SKTransformationMatrix matrixC = SKTransformationMatrix(int3(1,0,0),int3(0,1,0),int3(1,1,1));
      assert(matrixC.determinant() == 1);
      changeOfBasisMatrix = changeOfBasisMatrix * matrixC;

      C = A + B + C + xi + eta + zeta;
      xi = 2.0*B + xi + zeta;
      eta =  2.0*A + eta + zeta;

      goto algorithmStart;
    }
  }

  SKSymmetryCell cell = SKSymmetryCell(sqrt(A), sqrt(B), sqrt(C), acos(xi/(2.0*sqrt(B)*sqrt(C))) * 180.0/M_PI, acos(eta/(2.0*sqrt(A)*sqrt(C))) * 180.0/M_PI, acos(zeta/(2.0*sqrt(A)*sqrt(B))) * 180.0/M_PI);

  return std::make_pair(cell, changeOfBasisMatrix);
}

QDebug operator<<(QDebug debug, const SKSymmetryCell &m)
{
  debug << "(" << m._a << ", " << m._b << ", " << m._c << m._alpha * 180.0/M_PI << ", " << m._beta *180.0/M_PI << ", " << m._gamma * 180.0/M_PI <<")";
  return debug;
}
