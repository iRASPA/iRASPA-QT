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

class PolygonalPrismPrimitive: public Primitive, public RKRenderPrimitivePolygonalPrimsObjectsSource
{
public:
  PolygonalPrismPrimitive();
  PolygonalPrismPrimitive(const PolygonalPrismPrimitive &polygonalPrismPrimitive);

  PolygonalPrismPrimitive(const std::shared_ptr<const class Crystal> structure);
  PolygonalPrismPrimitive(const std::shared_ptr<const class MolecularCrystal> structure);
  PolygonalPrismPrimitive(const std::shared_ptr<const class Molecule> structure);
  PolygonalPrismPrimitive(const std::shared_ptr<const class ProteinCrystal> structure);
  PolygonalPrismPrimitive(const std::shared_ptr<const class Protein> structure);
  PolygonalPrismPrimitive(const std::shared_ptr<const class CrystalCylinderPrimitive> primitive);
  PolygonalPrismPrimitive(const std::shared_ptr<const class CrystalEllipsoidPrimitive> primitive);
  PolygonalPrismPrimitive(const std::shared_ptr<const class CrystalPolygonalPrismPrimitive> primitive);
  PolygonalPrismPrimitive(const std::shared_ptr<const class CylinderPrimitive> primitive);
  PolygonalPrismPrimitive(const std::shared_ptr<const class EllipsoidPrimitive> primitive);
  PolygonalPrismPrimitive(const std::shared_ptr<const class PolygonalPrismPrimitive> primitive);
  PolygonalPrismPrimitive(const std::shared_ptr<const class GridVolume> volume);

  ~PolygonalPrismPrimitive() {}

  std::shared_ptr<Object> shallowClone() override final;

  bool drawAtoms() const override {return _drawAtoms;}

  ObjectType structureType() override final { return ObjectType::polygonalPrismPrimitive; }

  std::shared_ptr<Primitive> flattenHierarchy() const override final;

  std::vector<RKInPerInstanceAttributesAtoms> renderPrimitivePolygonalPrismObjects() const override final;
  std::vector<RKInPerInstanceAttributesAtoms> renderSelectedPrimitivePolygonalPrismObjects() const override final;

  std::set<int> filterCartesianAtomPositions(std::function<bool(double3)> &) override final;

  SKBoundingBox boundingBox() const final override;
  void reComputeBoundingBox() final override;

  void expandSymmetry() override final;
  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForFractionalPositions() override final;
  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToFractionalPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToFractionalPositions() override final;

  double3 centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms) const override final;
  double3x3 matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom> > atoms) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;

private:
  qint64 _versionNumber{2};
  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<PolygonalPrismPrimitive> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<PolygonalPrismPrimitive> &);
};

