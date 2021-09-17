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
#include "iraspakitprotocols.h"

class CrystalPolygonalPrismPrimitive: public Structure, public RKRenderCrystalPrimitivePolygonalPrimsObjectsSource
{
public:
  CrystalPolygonalPrismPrimitive();
  CrystalPolygonalPrismPrimitive(const CrystalPolygonalPrismPrimitive &crystalPolygonalPrismPrimitive);
  CrystalPolygonalPrismPrimitive(std::shared_ptr<Structure> s);
  ~CrystalPolygonalPrismPrimitive() {}

  std::shared_ptr<Structure> clone() override final;
  std::shared_ptr<Structure> appliedCellContentShift() const override final;

  bool hasSymmetry() override final {return true;}
  std::shared_ptr<Structure> superCell() const override final;
  std::shared_ptr<Structure> flattenHierarchy() const override final;

  iRASPAStructureType structureType() override final { return iRASPAStructureType::crystalPolygonalPrismPrimitive; }

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

  simd_quatd primitiveOrientation() const override {return _primitiveOrientation;}
  double3x3 primitiveTransformationMatrix() const override  {return _primitiveTransformationMatrix;}
  double primitiveOpacity() const override  {return _primitiveOpacity;}
  bool primitiveIsCapped() const override  {return _primitiveIsCapped;}
  bool primitiveIsFractional() const override  {return _primitiveIsFractional;}
  int primitiveNumberOfSides() const override  {return _primitiveNumberOfSides;}
  double primitiveThickness() const override  {return _primitiveThickness;}

  RKSelectionStyle primitiveSelectionStyle() const override {return _primitiveSelectionStyle;}
  double primitiveSelectionIntensity() const override {return _primitiveSelectionIntensity;}
  double primitiveSelectionScaling() const override {return _primitiveSelectionScaling;}
  double primitiveSelectionStripesDensity() const override {return _primitiveSelectionStripesDensity;}
  double primitiveSelectionStripesFrequency() const override {return _primitiveSelectionStripesFrequency;}
  double primitiveSelectionWorleyNoise3DFrequency() const override {return _primitiveSelectionWorleyNoise3DFrequency;}
  double primitiveSelectionWorleyNoise3DJitter() const override {return _primitiveSelectionWorleyNoise3DJitter;}

  double primitiveHue() const override {return _primitiveHue;}
  double primitiveSaturation() const override {return _primitiveSaturation;}
  double primitiveValue() const override {return _primitiveValue;}

  bool primitiveFrontSideHDR() const override  {return _primitiveFrontSideHDR;}
  double primitiveFrontSideHDRExposure() const override  {return _primitiveFrontSideHDRExposure;}
  QColor primitiveFrontSideAmbientColor() const override  {return _primitiveFrontSideAmbientColor;}
  QColor primitiveFrontSideDiffuseColor() const override  {return _primitiveFrontSideDiffuseColor;}
  QColor primitiveFrontSideSpecularColor() const override  {return _primitiveFrontSideSpecularColor;}
  double primitiveFrontSideAmbientIntensity() const override  {return _primitiveFrontSideAmbientIntensity;}
  double primitiveFrontSideDiffuseIntensity() const override  {return _primitiveFrontSideDiffuseIntensity;}
  double primitiveFrontSideSpecularIntensity() const override  {return _primitiveFrontSideSpecularIntensity;}
  double primitiveFrontSideShininess() const override  {return _primitiveFrontSideShininess;}

  bool primitiveBackSideHDR() const override  {return _primitiveBackSideHDR;}
  double primitiveBackSideHDRExposure() const override  {return _primitiveBackSideHDRExposure;}
  QColor primitiveBackSideAmbientColor() const override  {return _primitiveBackSideAmbientColor;}
  QColor primitiveBackSideDiffuseColor() const override  {return _primitiveBackSideDiffuseColor;}
  QColor primitiveBackSideSpecularColor() const override  {return _primitiveBackSideSpecularColor;}
  double primitiveBackSideAmbientIntensity() const override  {return _primitiveBackSideAmbientIntensity;}
  double primitiveBackSideDiffuseIntensity() const override  {return _primitiveBackSideDiffuseIntensity;}
  double primitiveBackSideSpecularIntensity() const override  {return _primitiveBackSideSpecularIntensity;}
  double primitiveBackSideShininess() const override  {return _primitiveBackSideShininess;}

  double3 centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const override final;
  double3x3 matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;

private:
  qint64 _versionNumber{1};
  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<CrystalPolygonalPrismPrimitive> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<CrystalPolygonalPrismPrimitive> &);
};

