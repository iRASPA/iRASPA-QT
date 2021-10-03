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

#pragma  once

#include <symmetrykit.h>
#include "structure.h"
#include "primitive.h"
#include "iraspakitprotocols.h"

class CrystalPolygonalPrismPrimitive: public Primitive, public RKRenderCrystalPrimitivePolygonalPrimsObjectsSource
{
public:
  CrystalPolygonalPrismPrimitive();
  CrystalPolygonalPrismPrimitive(const CrystalPolygonalPrismPrimitive &crystalPolygonalPrismPrimitive);
  CrystalPolygonalPrismPrimitive(std::shared_ptr<Structure> s);
  ~CrystalPolygonalPrismPrimitive() {}

  bool drawAtoms() const override {return _drawAtoms;}

  std::shared_ptr<Structure> clone() override final;
  std::shared_ptr<Structure> appliedCellContentShift() const override final;

  bool hasSymmetry() override final {return true;}
  std::shared_ptr<Structure> superCell() const override final;
  std::shared_ptr<Structure> flattenHierarchy() const override final;

  ObjectType structureType() override final { return ObjectType::crystalPolygonalPrismPrimitive; }

  std::vector<RKInPerInstanceAttributesAtoms> renderCrystalPrimitivePolygonalPrismObjects() const override;
  std::vector<RKInPerInstanceAttributesAtoms> renderSelectedCrystalPrimitivePolygonalPrismObjects() const override;
  std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const override final;

  std::set<int> filterCartesianAtomPositions(std::function<bool(double3)> &) override final;

  SKBoundingBox boundingBox() const override final;
  void reComputeBoundingBox() final override;

  void expandSymmetry() override final;
  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForFractionalPositions() override final;
  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToFractionalPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToFractionalPositions() override final;

  double3 centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const override final;
  double3x3 matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;

private:
  qint64 _versionNumber{2};
  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<CrystalPolygonalPrismPrimitive> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<CrystalPolygonalPrismPrimitive> &);
};

