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
#include "iraspakitprotocols.h"
#include "object.h"
#include "atomviewer.h"
#include "primitivevisualappearanceviewer.h"
#include "rkrenderkitprotocols.h"

class Primitive: public Object, public AtomViewer, public PrimitiveVisualAppearanceViewer, public RKRenderPrimitiveObjectsSource
{
public:
  Primitive();
  Primitive(const Primitive &primitive);

  Primitive(const std::shared_ptr<const class Structure> structure);
  Primitive(const std::shared_ptr<const class Primitive> primitive);
  Primitive(const std::shared_ptr<const class GridVolume> volume);

  std::shared_ptr<Object> shallowClone() override;

  bool drawAtoms() const override {return _drawAtoms;}
  void setDrawAtoms(bool draw) { _drawAtoms = draw;}

  void clearSelection() override final;
  void setAtomSelection(int asymmetricAtomId) override final;
  void addAtomToSelection(int asymmetricAtomId) override final;
  void toggleAtomSelection(int asymmetricAtomId) override final;
  void setAtomSelection(std::set<int>& atomIds) override final;
  void addToAtomSelection(std::set<int>& atomIds) override final;

  std::shared_ptr<SKAtomTreeController> &atomsTreeController() override final {return _atomsTreeController;}
  void setAtomTreeController(std::shared_ptr<SKAtomTreeController> controller) {_atomsTreeController = controller;}

  simd_quatd primitiveOrientation() const override final {return _primitiveOrientation;}
  void setPrimitiveOrientation(simd_quatd orientation) override final  {_primitiveOrientation = orientation;}
  double primitiveRotationDelta() const override final  {return _primitiveRotationDelta;}
  void setPrimitiveRotationDelta(double angle) override final  {_primitiveRotationDelta = angle;}
  void setPrimitiveTransformationMatrix(double3x3 matrix) override final  {_primitiveTransformationMatrix = matrix;}
  double3x3 primitiveTransformationMatrix() const override final  {return _primitiveTransformationMatrix;}
  double primitiveOpacity() const override final   {return _primitiveOpacity;}
  void setPrimitiveOpacity(double opacity) override final  {_primitiveOpacity = opacity;}
  int primitiveNumberOfSides() const override final  {return _primitiveNumberOfSides;}
  void setPrimitiveNumberOfSides(int numberOfSides) override final  {_primitiveNumberOfSides = numberOfSides;}
  bool primitiveIsCapped() const  override final  {return _primitiveIsCapped;}
  void setPrimitiveIsCapped(bool isCapped) override final  {_primitiveIsCapped = isCapped;}

  bool primitiveIsFractional() const override {return true;}
  double primitiveThickness() const override final {return 0.05;}

  RKSelectionStyle primitiveSelectionStyle() const override final  {return _primitiveSelectionStyle;}
  void setPrimitiveSelectionStyle(RKSelectionStyle style) override final  {_primitiveSelectionStyle = style;}
  double primitiveSelectionScaling() const override final  {return _primitiveSelectionScaling;}
  void setPrimitiveSelectionScaling(double scaling) override final  {_primitiveSelectionScaling = scaling;}
  double primitiveSelectionIntensity() const override final  {return _primitiveSelectionIntensity;}
  void setPrimitiveSelectionIntensity(double value) override final   {_primitiveSelectionIntensity = value;}

  double primitiveSelectionStripesDensity() const override final  {return _primitiveSelectionStripesDensity;}
  double primitiveSelectionStripesFrequency() const  override final  {return _primitiveSelectionStripesFrequency;}
  double primitiveSelectionWorleyNoise3DFrequency() const override final  {return _primitiveSelectionWorleyNoise3DFrequency;}
  double primitiveSelectionWorleyNoise3DJitter() const override final  {return _primitiveSelectionWorleyNoise3DJitter;}

  double primitiveSelectionFrequency() const  override final;
  void setPrimitiveSelectionFrequency(double value) override final;
  double primitiveSelectionDensity() const override final ;
  void setPrimitiveSelectionDensity(double value) override final;

  double primitiveHue() const override final {return _primitiveHue;}
  void setPrimitiveHue(double value) override final {_primitiveHue = value;}
  double primitiveSaturation() const override final {return _primitiveSaturation;}
  void setPrimitiveSaturation(double value)  override final {_primitiveSaturation = value;}
  double primitiveValue() const override final {return _primitiveValue;}
  void setPrimitiveValue(double value) override final {_primitiveValue = value;}

  bool primitiveFrontSideHDR() const override final {return _primitiveFrontSideHDR;}
  void setPrimitiveFrontSideHDR(bool isHDR) override final  {_primitiveFrontSideHDR = isHDR;}
  double primitiveFrontSideHDRExposure() const override final {return _primitiveFrontSideHDRExposure;}
  void setPrimitiveFrontSideHDRExposure(double exposure) override final {_primitiveFrontSideHDRExposure = exposure;}
  double primitiveFrontSideAmbientIntensity() const override final {return _primitiveFrontSideAmbientIntensity;}
  void setPrimitiveFrontSideAmbientIntensity(double intensity) override final {_primitiveFrontSideAmbientIntensity = intensity;}
  double primitiveFrontSideDiffuseIntensity() const override final {return _primitiveFrontSideDiffuseIntensity;}
  void setPrimitiveFrontSideDiffuseIntensity(double intensity) override final {_primitiveFrontSideDiffuseIntensity = intensity;}
  double primitiveFrontSideSpecularIntensity() const override final {return _primitiveFrontSideSpecularIntensity;}
  void setPrimitiveFrontSideSpecularIntensity(double intensity) override final {_primitiveFrontSideSpecularIntensity = intensity;}
  QColor primitiveFrontSideAmbientColor() const override final {return _primitiveFrontSideAmbientColor;}
  void setPrimitiveFrontSideAmbientColor(QColor color) override final {_primitiveFrontSideAmbientColor = color;}
  QColor primitiveFrontSideDiffuseColor() const override final {return _primitiveFrontSideDiffuseColor;}
  void setPrimitiveFrontSideDiffuseColor(QColor color) override final {_primitiveFrontSideDiffuseColor = color;}
  QColor primitiveFrontSideSpecularColor() const override final {return _primitiveFrontSideSpecularColor;}
  void setPrimitiveFrontSideSpecularColor(QColor color) override final {_primitiveFrontSideSpecularColor = color;}
  double primitiveFrontSideShininess() const override final  {return _primitiveFrontSideShininess;}
  void setPrimitiveFrontSideShininess(double value) override final {_primitiveFrontSideShininess = value;}

  bool primitiveBackSideHDR() const override final {return _primitiveBackSideHDR;}
  void setPrimitiveBackSideHDR(bool isHDR)  override final {_primitiveBackSideHDR = isHDR;}
  double primitiveBackSideHDRExposure() const  override final {return _primitiveBackSideHDRExposure;}
  void setPrimitiveBackSideHDRExposure(double exposure) override final {_primitiveBackSideHDRExposure = exposure;}
  double primitiveBackSideAmbientIntensity() const override final {return _primitiveBackSideAmbientIntensity;}
  void setPrimitiveBackSideAmbientIntensity(double intensity) override final {_primitiveBackSideAmbientIntensity = intensity;}
  double primitiveBackSideDiffuseIntensity() const override final {return _primitiveBackSideDiffuseIntensity;}
  void setPrimitiveBackSideDiffuseIntensity(double intensity) override final {_primitiveBackSideDiffuseIntensity = intensity;}
  double primitiveBackSideSpecularIntensity() const override final {return _primitiveBackSideSpecularIntensity;}
  void setPrimitiveBackSideSpecularIntensity(double intensity) override final {_primitiveBackSideSpecularIntensity = intensity;}
  QColor primitiveBackSideAmbientColor() const override final {return _primitiveBackSideAmbientColor;}
  void setPrimitiveBackSideAmbientColor(QColor color) override final{_primitiveBackSideAmbientColor = color;}
  QColor primitiveBackSideDiffuseColor() const override final {return _primitiveBackSideDiffuseColor;}
  void setPrimitiveBackSideDiffuseColor(QColor color) override final {_primitiveBackSideDiffuseColor = color;}
  QColor primitiveBackSideSpecularColor() const override final {return _primitiveBackSideSpecularColor;}
  void setPrimitiveBackSideSpecularColor(QColor color) override final {_primitiveBackSideSpecularColor = color;}
  double primitiveBackSideShininess() const override final {return _primitiveBackSideShininess;}
  void setPrimitiveBackSideShininess(double value) override final {_primitiveBackSideShininess = value;}

  virtual bool hasSymmetry() {return false;}
  virtual std::shared_ptr<Primitive> superCell() const {return nullptr;}
  virtual std::shared_ptr<Primitive> flattenHierarchy() const {return nullptr;}
  virtual std::shared_ptr<Primitive> appliedCellContentShift() const {return nullptr;}
  virtual std::set<int> filterCartesianAtomPositions(std::function<bool(double3)> &) override;

  virtual std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForFractionalPositions();
  virtual std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForCartesianPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToFractionalPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToCartesianPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToFractionalPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToCartesianPositions();

  virtual void expandSymmetry() override;
  virtual double3 centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const;
  virtual double3x3 matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const;
  void recomputeSelectionBodyFixedBasis() override;

  void convertAsymmetricAtomsToFractional() override;
  void convertAsymmetricAtomsToCartesian() override;

  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override;

protected:
  qint64 _versionNumber{2};
  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<Primitive> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<Primitive> &);

  SKSpaceGroup _spaceGroup = SKSpaceGroup(1);
  double3 _selectionCOMTranslation = double3(0.0, 0.0, 0.0);
  int _selectionRotationIndex = 0;
  double3x3 _selectionBodyFixedBasis = double3x3();

  bool _drawAtoms =  true;
  std::shared_ptr<SKAtomTreeController> _atomsTreeController;

  double3x3 _primitiveTransformationMatrix = double3x3(1.0,0.0,0.0, 0.0,1.0,0.0, 0.0,0.0,1.0);
  simd_quatd _primitiveOrientation = simd_quatd(0.0,0.0,0.0,1.0);
  double _primitiveRotationDelta = 5.0;

  double _primitiveOpacity = 1.0;
  bool _primitiveIsCapped = false;
  bool _primitiveIsFractional = true;
  qint64 _primitiveNumberOfSides = 6;
  double _primitiveThickness = 0.05;

  double _primitiveHue = 1.0;
  double _primitiveSaturation = 1.0;
  double _primitiveValue = 1.0;

  RKSelectionStyle _primitiveSelectionStyle = RKSelectionStyle::striped;
  double _primitiveSelectionStripesDensity = 0.25;
  double _primitiveSelectionStripesFrequency = 12.0;
  double _primitiveSelectionWorleyNoise3DFrequency = 2.0;
  double _primitiveSelectionWorleyNoise3DJitter = 1.0;
  double _primitiveSelectionScaling = 1.0;
  double _primitiveSelectionIntensity = 1.0;


  bool _primitiveFrontSideHDR = true;
  double _primitiveFrontSideHDRExposure = 2.0;
  QColor _primitiveFrontSideAmbientColor = QColor(255, 255, 255, 255);
  QColor _primitiveFrontSideDiffuseColor = QColor(255, 255, 0, 255);
  QColor _primitiveFrontSideSpecularColor = QColor(255, 255, 255, 255);
  double _primitiveFrontSideAmbientIntensity = 0.1;
  double _primitiveFrontSideDiffuseIntensity = 1.0;
  double _primitiveFrontSideSpecularIntensity = 0.2;
  double _primitiveFrontSideShininess = 4.0;

  bool _primitiveBackSideHDR = true;
  double _primitiveBackSideHDRExposure = 2.0;
  QColor _primitiveBackSideAmbientColor = QColor(255, 255, 255, 255);
  QColor _primitiveBackSideDiffuseColor = QColor(0, 140, 255, 255);
  QColor _primitiveBackSideSpecularColor = QColor(255, 255, 255, 255);
  double _primitiveBackSideAmbientIntensity = 0.1;
  double _primitiveBackSideDiffuseIntensity = 1.0;
  double _primitiveBackSideSpecularIntensity = 0.2;
  double _primitiveBackSideShininess = 4.0;

};

