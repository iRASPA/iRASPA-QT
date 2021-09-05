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

#include "sktransformationmatrix.h"

SKTransformationMatrix::SKTransformationMatrix()
{

}

SKTransformationMatrix::SKTransformationMatrix(int3 v1, int3 v2, int3 v3)
{
  int3x3.m11 = v1.x; int3x3.m21 = v1.y; int3x3.m31 = v1.z;
  int3x3.m12 = v2.x; int3x3.m22 = v2.y; int3x3.m32 = v2.z;
  int3x3.m13 = v3.x; int3x3.m23 = v3.y; int3x3.m33 = v3.z;
}

SKTransformationMatrix SKTransformationMatrix::adjugate() const
{
  return SKTransformationMatrix(this->int3x3.adjugate());
}

SKTransformationMatrix SKTransformationMatrix::operator * (const SKTransformationMatrix& right) const
{
  return SKTransformationMatrix(this->int3x3 * right.int3x3);
}

SKTransformationMatrix SKTransformationMatrix::zero = SKTransformationMatrix(int3(0,0,0),int3(0,0,0),int3(0,0,0));
SKTransformationMatrix SKTransformationMatrix::identity = SKTransformationMatrix(int3(1,0,0),int3(0,1,0),int3(0,0,1));
SKTransformationMatrix SKTransformationMatrix::inversionIdentity = SKTransformationMatrix(int3(-1,0,0),int3(0,-1,0),int3(0,0,-1));

// based on the centering, convert conventional cell to primitive using conventionally used transformation matrices
// Taken from: Table 2.C.1, page 141, Fundamentals of Crystallography, 2nd edition, C. Giacovazzo et al. 2002
// Tranformation matrices M, conventionally used to generate centered from primitive lattices, and vice versa, accoording to: A' = M A

SKTransformationMatrix SKTransformationMatrix::primitiveToPrimitive = SKTransformationMatrix(int3( 1, 0, 0), int3( 0, 1, 0), int3( 0, 0, 1));           // P -> P
SKTransformationMatrix SKTransformationMatrix::primitiveToBodyCentered = SKTransformationMatrix(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0));           // P -> I
SKTransformationMatrix SKTransformationMatrix::primitiveToFaceCentered = SKTransformationMatrix(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1));        // P -> F
SKTransformationMatrix SKTransformationMatrix::primitiveToACentered = SKTransformationMatrix(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1));            // P -> A
SKTransformationMatrix SKTransformationMatrix::primitiveToBCentered = SKTransformationMatrix(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1));            // P -> B
SKTransformationMatrix SKTransformationMatrix::primitiveToCCentered = SKTransformationMatrix(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1));            // P -> C
SKTransformationMatrix SKTransformationMatrix::primitiveToRhombohedral = SKTransformationMatrix(int3( 1, -1, 0), int3( 0, 1,-1), int3( 1, 1, 1));       // P -> R
SKTransformationMatrix SKTransformationMatrix::primitiveToHexagonal = SKTransformationMatrix(int3( 1, -1, 0), int3( 1, 2, 0), int3( 0, 0, 3));          // P -> H
SKTransformationMatrix SKTransformationMatrix::rhombohedralObverseHexagonal = SKTransformationMatrix(int3(1, 0, 1), int3(-1, 1, 1), int3(0, -1, 1));    // Robv -> Rh
SKTransformationMatrix SKTransformationMatrix::rhombohedralHexagonalToReverse = SKTransformationMatrix(int3(1, 1, -2),int3(-1, 0, 1),int3(1, 1, -1));   // Rh -> Rrev
SKTransformationMatrix SKTransformationMatrix::rhombohedralReverseToHexagonal = SKTransformationMatrix(int3(-1, -1, 1), int3(0, 1, 1), int3(-1, 0, 1)); // Rrev -> Rh

SKTransformationMatrix SKTransformationMatrix::monoclinicAtoC = SKTransformationMatrix(int3(0,0,1),int3(0,-1,0),int3(1,0,0));
SKTransformationMatrix SKTransformationMatrix::AtoC = SKTransformationMatrix(int3(0,1,0),int3(0,0,1),int3(1,0,0));
SKTransformationMatrix SKTransformationMatrix::monoclinicItoC = SKTransformationMatrix(int3(1,0,1),int3(0, 1,0),int3(-1,0,0));
SKTransformationMatrix SKTransformationMatrix::BtoC = SKTransformationMatrix(int3(0,0,1),int3(1,0,0),int3(0,1,0));
SKTransformationMatrix SKTransformationMatrix::primitiveRhombohedralToTripleHexagonalCell_R2 = SKTransformationMatrix(int3( 0,-1, 1), int3( 1, 0,-1), int3( 1, 1, 1));
SKTransformationMatrix SKTransformationMatrix::primitiveRhombohedralToTripleHexagonalCell_R1_Obverse = SKTransformationMatrix(int3( 1,-1, 0), int3( 0, 1,-1), int3( 1, 1, 1));

double3x3 SKTransformationMatrix::bodyCenteredToPrimitive = double3x3(double3(-0.5,0.5,0.5), double3(0.5,-0.5,0.5), double3(0.5,0.5,-0.5));  // I -> P
double3x3 SKTransformationMatrix::faceCenteredToPrimitive = double3x3(double3(0,0.5,0.5), double3(0.5,0,0.5), double3(0.5,0.5,0));           // F -> P
double3x3 SKTransformationMatrix::ACenteredToPrimitive = double3x3(double3(-1.0,0,0), double3(0,-0.5,0.5), double3(0,0.5,0.5));              // A -> P
double3x3 SKTransformationMatrix::BCenteredToPrimitive = double3x3(double3(-0.5,0,0.5), double3(0,-1.0,0), double3(0.5,0,0.5));              // B -> P
double3x3 SKTransformationMatrix::CCenteredToPrimitive3 = double3x3(double3(0.5,0.5,0), double3(0.5,-0.5,0), double3(0,0,-1.0));             // C -> P
double3x3 SKTransformationMatrix::rhombohedralToPrimitive = double3x3(double3(2.0/3.0,1.0/3.0,1.0/3.0), double3(-1.0/3.0, 1.0/3.0, 1.0/3.0), double3(-1.0/3.0,-2.0/3.0, 1.0/3.0));  // R -> P

// CHECK
double3x3 SKTransformationMatrix::rhombohedralReverseToPrimitive = double3x3(double3(1.0/3.0,2.0/3.0,1.0/3.0), double3(-2.0/3.0, -1.0/3.0, 1.0/3.0), double3(1.0/3.0,-1.0/3.0, 1.0/3.0));  // R -> P

double3x3 SKTransformationMatrix::hexagonalToPrimitive = double3x3(double3(2.0/3.0,1.0/3.0, 0), double3(-1.0/3.0, 1.0/3.0, 0), double3( 0, 0, 1.0/3.0));  // H -> P
double3x3 SKTransformationMatrix::rhombohedralHexagonalToObverse = double3x3(double3(2.0/3.0,-1.0/3.0,-1.0/3.0),double3(1.0/3.0,1.0/3.0,-2.0/3.0),double3(1.0/3.0,1.0/3.0,1.0/3.0));   // Rh -> Robv
