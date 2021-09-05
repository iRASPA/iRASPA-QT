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

#include <ostream>
#include <mathkit.h>
#include "skdefinitions.h"


struct SKTransformationMatrix
{
  SKTransformationMatrix();
  SKTransformationMatrix(int3x3 m) {this->int3x3 = m;}
  SKTransformationMatrix(int3 v1, int3 v2, int3 v3);

  int3x3 int3x3;
  int3 translation;

  SKTransformationMatrix adjugate() const;
  SKTransformationMatrix operator * (const SKTransformationMatrix& right) const;

  static SKTransformationMatrix zero;
  static SKTransformationMatrix identity;
  static SKTransformationMatrix inversionIdentity;

  // based on the centering, convert conventional cell to primitive using conventionally used transformation matrices
  // Taken from: Table 2.C.1, page 141, Fundamentals of Crystallography, 2nd edition, C. Giacovazzo et al. 2002
  // Tranformation matrices M, conventionally used to generate centered from primitive lattices, and vice versa, accoording to: A' = M A

  static SKTransformationMatrix primitiveToPrimitive;           // P -> P
  static SKTransformationMatrix primitiveToBodyCentered;        // P -> I
  static SKTransformationMatrix primitiveToFaceCentered;        // P -> F
  static SKTransformationMatrix primitiveToACentered;           // P -> A
  static SKTransformationMatrix primitiveToBCentered;           // P -> B
  static SKTransformationMatrix primitiveToCCentered;           // P -> C
  static SKTransformationMatrix primitiveToRhombohedral;        // P -> R
  static SKTransformationMatrix primitiveToHexagonal;           // P -> H
  static SKTransformationMatrix rhombohedralObverseHexagonal;   // Robv -> Rh
  static SKTransformationMatrix rhombohedralHexagonalToReverse; // Rh -> Rrev
  static SKTransformationMatrix rhombohedralReverseToHexagonal; // Rrev -> Rh

  static SKTransformationMatrix monoclinicAtoC;
  static SKTransformationMatrix AtoC;
  static SKTransformationMatrix monoclinicItoC;
  static SKTransformationMatrix BtoC;
  static SKTransformationMatrix primitiveRhombohedralToTripleHexagonalCell_R2;
  static SKTransformationMatrix primitiveRhombohedralToTripleHexagonalCell_R1_Obverse;

  static double3x3 bodyCenteredToPrimitive;  // I -> P
  static double3x3 faceCenteredToPrimitive;  // F -> P
  static double3x3 ACenteredToPrimitive;     // A -> P
  static double3x3 BCenteredToPrimitive;     // B -> P
  static double3x3 CCenteredToPrimitive3;    // C -> P
  static double3x3 rhombohedralToPrimitive;  // R -> P

  // CHECK
  static double3x3 rhombohedralReverseToPrimitive;  // R -> P

  static double3x3 hexagonalToPrimitive;            // H -> P
  static double3x3 rhombohedralHexagonalToObverse;  // Rh -> Robv

};


namespace std
{
  template <> struct hash<SKTransformationMatrix>
  {
    size_t operator()(const SKTransformationMatrix& k) const
    {
      std::hash<int3x3> hash_fn;
      return hash_fn(k.int3x3);
    }
  };
}
