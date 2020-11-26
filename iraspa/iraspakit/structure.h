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

#include <QString>
#include <QColor>
#include <QDate>
#include <utility>
#include <type_traits>
#include <mathkit.h>
#include <symmetrykit.h>
#include <simulationkit.h>
#include <renderkit.h>
#include "iraspakitprotocols.h"

enum class iRASPAStructureType : qint64
{ 
  none = 0, structure = 1, crystal = 2, molecularCrystal = 3, molecule = 4, protein = 5, proteinCrystal = 6,
  proteinCrystalSolvent = 7, crystalSolvent = 8, molecularCrystalSolvent = 9,
  crystalEllipsoidPrimitive = 10, crystalCylinderPrimitive = 11, crystalPolygonalPrismPrimitive = 12,
  ellipsoidPrimitive = 13, cylinderPrimitive = 14, polygonalPrismPrimitive = 15
};


struct enum_hash
{
    template <typename T>
    inline
    typename std::enable_if<std::is_enum<T>::value, std::size_t>::type
    operator ()(T const value) const
    {
        return static_cast<std::size_t>(value);
    }
};



class Structure: public RKRenderStructure, public DisplayableProtocol
{
public:
  Structure();
  Structure(std::shared_ptr<SKAtomTreeController> atomTreeController);
  Structure(std::shared_ptr<SKStructure> structure);
  Structure(std::shared_ptr<Structure> clone);
  Structure(const Structure &structure);
  virtual std::shared_ptr<Structure> clone() {return nullptr;}
  virtual ~Structure() {}

  virtual bool hasSymmetry() {return false;}
  virtual std::shared_ptr<Structure> superCell() const {return nullptr;}
  virtual std::shared_ptr<Structure> removeSymmetry() const {return nullptr;}
  virtual std::shared_ptr<Structure> wrapAtomsToCell() const {return nullptr;}
  virtual std::shared_ptr<Structure> flattenHierarchy() const {return nullptr;}
  virtual std::shared_ptr<Structure> appliedCellContentShift() const {return nullptr;}
  virtual std::pair<std::vector<int>, std::vector<double3>> atomSymmetryData() const {return {};}
  virtual void setAtomSymmetryData(double3x3 unitCell, std::vector<std::pair<int, double3>> atomData) {};


  enum class StructureType: qint64
  {
    framework = 0, adsorbate = 1, cation = 2, ionicLiquid = 3, solvent = 4
  };

  enum class TemperatureScale: qint64
  {
    Kelvin = 0, Celsius = 1, multiple_values = 2
  };

  enum class PressureScale: qint64
  {
    Pascal = 0, bar = 1, multiple_values = 2
  };

  enum class CreationMethod: qint64
  {
    unknown = 0, simulation = 1, experimental = 2, multiple_values = 3
  };

  enum class UnitCellRelaxationMethod: qint64
  {
    unknown = 0, allFree = 1, fixedAnglesIsotropic = 2, fixedAnglesAnistropic = 3, betaAnglefixed = 4, fixedVolumeFreeAngles = 5, allFixed = 6, multiple_values = 7
  };

  enum class IonsRelaxationAlgorithm: qint64
  {
    unknown = 0, none = 1, simplex = 2, simulatedAnnealing = 3, geneticAlgorithm = 4, steepestDescent = 5, conjugateGradient = 6,
    quasiNewton = 7, NewtonRaphson = 8, BakersMinimization = 9, multiple_values = 10
  };

  enum class IonsRelaxationCheck: qint64
  {
    unknown = 0, none = 1, allPositiveEigenvalues = 2, someSmallNegativeEigenvalues = 3, someSignificantNegativeEigenvalues = 4,
    manyNegativeEigenvalues = 5, multiple_values = 6
  };


  enum class RepresentationType: qint64
  {
    sticks_and_balls = 0, vdw = 1, unity = 2, multiple_values = 3
  };

  enum class RepresentationStyle: qint64
  {
    custom = -1, defaultStyle = 0, fancy = 1, licorice = 2, objects = 3, multiple_values = 4
  };

  enum class ProbeMolecule: qint64
  {
    helium = 0, methane = 1, nitrogen = 2, hydrogen = 3, water = 4, co2 = 5, xenon = 6, krypton = 7, argon = 8, multiple_values = 9
  };

  virtual iRASPAStructureType structureType() {return iRASPAStructureType::structure;}

  virtual std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForFractionalPositions();
  virtual std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForCartesianPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToFractionalPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToCartesianPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToFractionalPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToCartesianPositions();


  QString displayName() const override {return _displayName;}
  void setDisplayName(QString name) {_displayName = name;}

  bool isVisible() const  override {return _isVisible;}
  void setVisibility(bool visibility) {_isVisible = visibility;}
  bool hasSelectedAtoms() const override final;

  std::vector<double3> atomPositions() const override {return std::vector<double3>();}
  std::vector<double3> bondPositions() const override {return std::vector<double3>();}

  std::vector<double2> potentialParameters() const override {return std::vector<double2>();}

  virtual std::vector<RKInPerInstanceAttributesAtoms> renderAtoms() const override;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderInternalBonds() const override;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderExternalBonds() const override;
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const override;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const override;

  virtual std::vector<RKInPerInstanceAttributesAtoms> renderSelectedAtoms() const override;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderSelectedInternalBonds() const override;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderSelectedExternalBonds() const override;

  virtual std::set<int> filterCartesianAtomPositions(std::function<bool(double3)> &);
  virtual std::set<int> filterCartesianBondPositions(std::function<bool(double3)> &);

  virtual std::vector<RKInPerInstanceAttributesText> atomTextData(RKFontAtlas *fontAtlas) const override;

  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const;

  SKBoundingBox boundingBox() const override;
  SKBoundingBox transformedBoundingBox() const final override;
  void reComputeBoundingBox() final override;

  void expandSymmetry() override;
  virtual double3 centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const;
  virtual double3x3 matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const;
  void recomputeSelectionBodyFixedBasis();

  virtual double bondLength(std::shared_ptr<SKBond> bond) const;
  virtual double3 bondVector(std::shared_ptr<SKBond> bond) const;
  virtual std::pair<double3, double3> computeChangedBondLength(std::shared_ptr<SKBond>, double) const;

  bool clipAtomsAtUnitCell() const override {return false;}
  bool clipBondsAtUnitCell() const override {return false;}
  bool colorAtomsWithBondColor() const override;

  std::vector<RKInPerInstanceAttributesText> renderTextData() const override {return std::vector<RKInPerInstanceAttributesText>();}
  RKTextType renderTextType() const override {return _atomTextType;}
  void setRenderTextType(RKTextType type) {_atomTextType = type;}
  void setRenderTextFont(QString value) {_atomTextFont = value;}
  QString renderTextFont() const override {return _atomTextFont;}
  RKTextAlignment renderTextAlignment() const override {return _atomTextAlignment;}
  void setRenderTextAlignment(RKTextAlignment alignment) {_atomTextAlignment = alignment;}
  RKTextStyle renderTextStyle() const override {return _atomTextStyle;}
  void setRenderTextStyle(RKTextStyle style) {_atomTextStyle = style;}
  QColor renderTextColor() const override {return _atomTextColor;}
  void setRenderTextColor(QColor color) {_atomTextColor = color;}
  double renderTextScaling() const override {return _atomTextScaling;}
  void setRenderTextScaling(double scaling)  {_atomTextScaling = scaling;}
  double3 renderTextOffset() const override {return _atomTextOffset;}
  void setRenderTextOffsetX(double value) {_atomTextOffset.x = value;}
  void setRenderTextOffsetY(double value) {_atomTextOffset.y = value;}
  void setRenderTextOffsetZ(double value) {_atomTextOffset.z = value;}

  void clearSelection();
  void setAtomSelection(int asymmetricAtomId);
  void setBondSelection(int asymmetricBondId);
  void addAtomToSelection(int asymmetricAtomId);
  void addBondToSelection(int asymmetricBondId);
  void toggleAtomSelection(int asymmetricAtomId);
  void toggleBondSelection(int asymmetricAtomId);
  void setAtomSelection(std::set<int>& atomIds);
  void addToAtomSelection(std::set<int>& atomIds);

  RKSelectionStyle atomSelectionStyle() const override {return _atomSelectionStyle;}
  void setAtomSelectionStyle(RKSelectionStyle style) {_atomSelectionStyle = style;}
  double atomSelectionScaling() const override {return _atomSelectionScaling;}
  void setAtomSelectionScaling(double scaling) {_atomSelectionScaling = scaling;}
  double atomSelectionIntensity() const override {return _atomSelectionIntensity;}
  void setSelectionIntensity(double scaling) {_atomSelectionIntensity = scaling;}

  double atomSelectionStripesDensity() const override {return _atomSelectionStripesDensity;}
  void setAtomSelectionStripesDensity(double value) {_atomSelectionStripesDensity = value;}
  double atomSelectionStripesFrequency() const override {return _atomSelectionStripesFrequency;}
  void setAtomSelectionStripesFrequency(double value) {_atomSelectionStripesFrequency = value;}
  double atomSelectionWorleyNoise3DFrequency() const override {return _atomSelectionWorleyNoise3DFrequency;}
  void setAtomSelectionWorleyNoise3DFrequency(double value) {_atomSelectionWorleyNoise3DFrequency = value;}
  double atomSelectionWorleyNoise3DJitter() const override {return _atomSelectionWorleyNoise3DJitter;}
  void setAtomSelectionWorleyNoise3DJitter(double value) {_atomSelectionWorleyNoise3DJitter = value;}

  double atomSelectionFrequency() const override;
  void setAtomSelectionFrequency(double value);
  double atomSelectionDensity() const override;
  void setAtomSelectionDensity(double value);

  virtual double3 CartesianPosition(double3 position, int3 replicaPosition) const {Q_UNUSED(position); Q_UNUSED(replicaPosition); return double(); }
  virtual double3 FractionalPosition(double3 position, int3 replicaPosition) const {Q_UNUSED(position); Q_UNUSED(replicaPosition); return double(); }
  void convertAsymmetricAtomsToCartesian();
  void convertAsymmetricAtomsToFractional();

  int numberOfAtoms() const override {return 0;}
  int numberOfInternalBonds() const override {return 0;}
  int numberOfExternalBonds() const override {return 0;}

  void setAtomTreeController(std::shared_ptr<SKAtomTreeController> controller) {_atomsTreeController = controller;}
  std::shared_ptr<SKAtomTreeController> &atomsTreeController() {return _atomsTreeController;}
  std::shared_ptr<SKBondSetController> bondSetController() {return _bondSetController;}

  bool drawUnitCell() const override {return _drawUnitCell;}
  void setDrawUnitCell(bool state) {_drawUnitCell = state;}
  bool drawAtoms() const override {return _drawAtoms;}
  void setDrawAtoms(bool drawAtoms) {_drawAtoms = drawAtoms;}
  bool drawBonds() const override {return _drawBonds;}
  void setDrawBonds(bool drawBonds) {_drawBonds = drawBonds;}

  double rotationDelta() {return _rotationDelta;}
  void setRotationDelta(double angle) {_rotationDelta = angle;}
  void setOrientation(simd_quatd orientation) {_orientation = orientation;}
  simd_quatd orientation() const override final {return _orientation;}
  double3 origin() const override final {return _origin;}
  void setOrigin(double3 value) {_origin = value;}
  void setOriginX(double value) {_origin.x = value;}
  void setOriginY(double value) {_origin.y = value;}
  void setOriginZ(double value) {_origin.z = value;}

  // primitive

  simd_quatd primitiveOrientation() {return _primitiveOrientation;}
  void setPrimitiveOrientation(simd_quatd orientation) {_primitiveOrientation = orientation;}
  double primitiveRotationDelta() {return _primitiveRotationDelta;}
  void setPrimitiveRotationDelta(double angle) {_primitiveRotationDelta = angle;}
  double3x3 transformationMatrix() {return _primitiveTransformationMatrix;}
  void setTransformationMatrix(double3x3 matrix) {_primitiveTransformationMatrix = matrix;}
  double primitiveOpacity() {return _primitiveOpacity;}
  void setPrimitiveOpacity(double opacity) {_primitiveOpacity = opacity;}
  int primitiveNumberOfSides() {return _primitiveNumberOfSides;}
  void setPrimitiveNumberOfSides(int numberOfSides) {_primitiveNumberOfSides = numberOfSides;}
  bool primitiveIsCapped() {return _primitiveIsCapped;}
  void setPrimitiveIsCapped(bool isCapped) {_primitiveIsCapped = isCapped;}

  RKSelectionStyle primitiveSelectionStyle() const {return _primitiveSelectionStyle;}
  void setPrimitiveSelectionStyle(RKSelectionStyle style) {_primitiveSelectionStyle = style;}
  double primitiveSelectionScaling() {return _primitiveSelectionScaling;}
  void setPrimitiveSelectionScaling(double scaling) {_primitiveSelectionScaling = scaling;}
  double primitiveSelectionIntensity()  {return _primitiveSelectionIntensity;}
  void setPrimitiveSelectionIntensity(double value) {_primitiveSelectionIntensity = value;}

  double primitiveSelectionStripesDensity()  {return _primitiveSelectionStripesDensity;}
  double primitiveSelectionStripesFrequency()   {return _primitiveSelectionStripesFrequency;}
  double primitiveSelectionWorleyNoise3DFrequency()  {return _primitiveSelectionWorleyNoise3DFrequency;}
  double primitiveSelectionWorleyNoise3DJitter()   {return _primitiveSelectionWorleyNoise3DJitter;}

  double primitiveSelectionFrequency() const;
  void setPrimitiveSelectionFrequency(double value);
  double primitiveSelectionDensity() const;
  void setPrimitiveSelectionDensity(double value);

  double primitiveHue() const {return _primitiveHue;}
  void setPrimitiveHue(double value) {_primitiveHue = value;}
  double primitiveSaturation() const {return _primitiveSaturation;}
  void setPrimitiveSaturation(double value) {_primitiveSaturation = value;}
  double primitiveValue() const {return _primitiveValue;}
  void setPrimitiveValue(double value) {_primitiveValue = value;}

  bool frontPrimitiveHDR() {return _primitiveFrontSideHDR;}
  void setFrontPrimitiveHDR(bool isHDR) {_primitiveFrontSideHDR = isHDR;}
  double frontPrimitiveHDRExposure() {return _primitiveFrontSideHDRExposure;}
  void setFrontPrimitiveHDRExposure(double exposure) {_primitiveFrontSideHDRExposure = exposure;}
  double frontPrimitiveAmbientIntensity() {return _primitiveFrontSideAmbientIntensity;}
  void setFrontPrimitiveAmbientIntensity(double intensity) {_primitiveFrontSideAmbientIntensity = intensity;}
  double frontPrimitiveDiffuseIntensity() {return _primitiveFrontSideDiffuseIntensity;}
  void setFrontPrimitiveDiffuseIntensity(double intensity) {_primitiveFrontSideDiffuseIntensity = intensity;}
  double frontPrimitiveSpecularIntensity() {return _primitiveFrontSideSpecularIntensity;}
  void setFrontPrimitiveSpecularIntensity(double intensity) {_primitiveFrontSideSpecularIntensity = intensity;}
  QColor frontPrimitiveAmbientColor() {return _primitiveFrontSideAmbientColor;}
  void setFrontPrimitiveAmbientColor(QColor color) {_primitiveFrontSideAmbientColor = color;}
  QColor frontPrimitiveDiffuseColor() {return _primitiveFrontSideDiffuseColor;}
  void setFrontPrimitiveDiffuseColor(QColor color) {_primitiveFrontSideDiffuseColor = color;}
  QColor frontPrimitiveSpecularColor() {return _primitiveFrontSideSpecularColor;}
  void setFrontPrimitiveSpecularColor(QColor color) {_primitiveFrontSideSpecularColor = color;}
  double frontPrimitiveShininess() {return _primitiveFrontSideShininess;}
  void setFrontPrimitiveShininess(double value) {_primitiveFrontSideShininess = value;}

  bool backPrimitiveHDR() {return _primitiveBackSideHDR;}
  void setBackPrimitiveHDR(bool isHDR) {_primitiveBackSideHDR = isHDR;}
  double backPrimitiveHDRExposure() {return _primitiveBackSideHDRExposure;}
  void setBackPrimitiveHDRExposure(double exposure) {_primitiveBackSideHDRExposure = exposure;}
  double backPrimitiveAmbientIntensity() {return _primitiveBackSideAmbientIntensity;}
  void setBackPrimitiveAmbientIntensity(double intensity) {_primitiveBackSideAmbientIntensity = intensity;}
  double backPrimitiveDiffuseIntensity() {return _primitiveBackSideDiffuseIntensity;}
  void setBackPrimitiveDiffuseIntensity(double intensity) {_primitiveBackSideDiffuseIntensity = intensity;}
  double backPrimitiveSpecularIntensity() {return _primitiveBackSideSpecularIntensity;}
  void setBackPrimitiveSpecularIntensity(double intensity) {_primitiveBackSideSpecularIntensity = intensity;}
  QColor backPrimitiveAmbientColor() {return _primitiveBackSideAmbientColor;}
  void setBackPrimitiveAmbientColor(QColor color) {_primitiveBackSideAmbientColor = color;}
  QColor backPrimitiveDiffuseColor() {return _primitiveBackSideDiffuseColor;}
  void setBackPrimitiveDiffuseColor(QColor color) {_primitiveBackSideDiffuseColor = color;}
  QColor backPrimitiveSpecularColor() {return _primitiveBackSideSpecularColor;}
  void setBackPrimitiveSpecularColor(QColor color) {_primitiveBackSideSpecularColor = color;}
  double backPrimitiveShininess() {return _primitiveBackSideShininess;}
  void setBackPrimitiveShininess(double value) {_primitiveBackSideShininess = value;}


  // material properties
  QColor atomAmbientColor() const override {return _atomAmbientColor;}
  void setAtomAmbientColor(QColor color) {_atomAmbientColor = color;}
  QColor atomDiffuseColor() const override {return _atomDiffuseColor;}
  void setAtomDiffuseColor(QColor color) {_atomDiffuseColor = color;}
  QColor atomSpecularColor() const override {return _atomSpecularColor;}
  void setAtomSpecularColor(QColor color) {_atomSpecularColor = color;}

  double atomAmbientIntensity() const override {return _atomAmbientIntensity;}
  void setAtomAmbientIntensity(double value) {_atomAmbientIntensity = value;}
  double atomDiffuseIntensity() const override {return _atomDiffuseIntensity;}
  void setAtomDiffuseIntensity(double value) {_atomDiffuseIntensity = value;}
  double atomSpecularIntensity() const override {return _atomSpecularIntensity;}
  void setAtomSpecularIntensity(double value) {_atomSpecularIntensity = value;}
  double atomShininess() const override {return _atomShininess;}
  void setAtomShininess(double value) {_atomShininess = value;}


  void setCell(std::shared_ptr<SKCell> cell) {_cell = cell;}
  std::shared_ptr<SKCell> cell() const override {return _cell;}

  //virtual atomCacheAmbientOcclusionTexture: [CUnsignedChar] {get set}

  bool hasExternalBonds() const override {return true;}

  double atomHue() const override {return _atomHue;}
  void setAtomHue(double value) {_atomHue = value;}
  double atomSaturation() const override {return _atomSaturation;}
  void setAtomSaturation(double value) {_atomSaturation = value;}
  double atomValue() const override {return _atomValue;}
  void setAtomValue(double value) {_atomValue = value;}

  double atomScaleFactor() const override {return _atomScaleFactor;}
  void setAtomScaleFactor(double size);

  bool atomAmbientOcclusion() const override {return _atomAmbientOcclusion;}
  void setAtomAmbientOcclusion(bool value) {_atomAmbientOcclusion = value;}
  int atomAmbientOcclusionPatchNumber() const override {return _atomAmbientOcclusionPatchNumber;}
  void setAtomAmbientOcclusionPatchNumber(int patchNumber) override {_atomAmbientOcclusionPatchNumber=patchNumber;}
  int atomAmbientOcclusionPatchSize() const override {return _atomAmbientOcclusionPatchSize;}
  void setAtomAmbientOcclusionPatchSize(int patchSize) override {_atomAmbientOcclusionPatchSize=patchSize;}
  int atomAmbientOcclusionTextureSize() const override {return _atomAmbientOcclusionTextureSize;}
  void setAtomAmbientOcclusionTextureSize(int size) override {_atomAmbientOcclusionTextureSize=size;}

  bool atomHDR() const  override{return _atomHDR;}
  void setAtomHDR(bool value) {_atomHDR = value;}
  double atomHDRExposure() const override {return _atomHDRExposure;}
  void setAtomHDRExposure(double value) {_atomHDRExposure = value;}


  bool bondAmbientOcclusion() const override {return _bondAmbientOcclusion;}
  void setBondAmbientOcclusion(bool state) {_bondAmbientOcclusion = state;}
  QColor bondAmbientColor() const override {return _bondAmbientColor;}
  void setBondAmbientColor(QColor color) {_bondAmbientColor = color;}
  QColor bondDiffuseColor() const override {return _bondDiffuseColor;}
  void setBondDiffuseColor(QColor color) {_bondDiffuseColor = color;}
  QColor bondSpecularColor() const override {return _bondSpecularColor;}
  void setBondSpecularColor(QColor color) {_bondSpecularColor = color;}
  double bondAmbientIntensity() const override {return _bondAmbientIntensity;}
  void setBondAmbientIntensity(double value) {_bondAmbientIntensity = value;}
  double bondDiffuseIntensity() const override {return _bondDiffuseIntensity;}
  void setBondDiffuseIntensity(double value) {_bondDiffuseIntensity = value;}
  double bondSpecularIntensity() const override {return _bondSpecularIntensity;}
  void setBondSpecularIntensity(double value) {_bondSpecularIntensity = value;}
  double bondShininess() const override {return _bondShininess;}
  void setBondShininess(double value) {_bondShininess = value;}

  double bondScaleFactor() const override {return _bondScaleFactor;}
  void setBondScaleFactor(double value);
  RKBondColorMode bondColorMode() const override {return _bondColorMode;}
  void setBondColorMode(RKBondColorMode value) {_bondColorMode = value;}

  bool bondHDR() const override {return _bondHDR;}
  void setBondHDR(bool value) {_bondHDR = value;}
  double bondHDRExposure() const override {return _bondHDRExposure;}
  void setBondHDRExposure(double value) {_bondHDRExposure = value;}

  RKSelectionStyle bondSelectionStyle() const override {return _bondSelectionStyle;}
  void setBondSelectionStyle(RKSelectionStyle style) {_bondSelectionStyle = style;}
  double bondSelectionScaling() const override {return _bondSelectionScaling;}
  void setBondSelectionScaling(double scaling) {_bondSelectionScaling = scaling;}
  double bondSelectionIntensity() const override {return _bondSelectionIntensity;}
  void setBondSelectionIntensity(double value) {_bondSelectionIntensity = value;}

  double bondSelectionStripesDensity() const override {return _bondSelectionStripesDensity;}
  double bondSelectionStripesFrequency() const override  {return _bondSelectionStripesFrequency;}
  double bondSelectionWorleyNoise3DFrequency() const override {return _bondSelectionWorleyNoise3DFrequency;}
  double bondSelectionWorleyNoise3DJitter() const override  {return _bondSelectionWorleyNoise3DJitter;}

  double bondSelectionFrequency() const override;
  void setBondSelectionFrequency(double value);
  double bondSelectionDensity() const override;
  void setBondSelectionDensity(double value);

  double bondHue() const override {return _bondHue;}
  void setBondHue(double value) {_bondHue = value;}
  double bondSaturation() const override {return _bondSaturation;}
  void setBondSaturation(double value) {_bondSaturation = value;}
  double bondValue() const override {return _bondValue;}
  void setBondValue(double value) {_bondValue = value;}

  // unit cell
  double unitCellScaleFactor() const override {return _unitCellScaleFactor;}
  void setUnitCellScaleFactor(double value) {_unitCellScaleFactor = value;}
  QColor unitCellDiffuseColor() const override {return _unitCellDiffuseColor;}
  void setUnitCellDiffuseColor(QColor color) {_unitCellDiffuseColor = color;}
  double unitCellDiffuseIntensity() const override {return _unitCellDiffuseIntensity;}
  void setUnitCellDiffuseIntensity(double value) {_unitCellDiffuseIntensity = value;}


  // adsorption surface
  std::vector<double3> atomUnitCellPositions() const override {return std::vector<double3>();}
  bool drawAdsorptionSurface() const override {return _drawAdsorptionSurface;}
  void setDrawAdsorptionSurface(bool state) {_drawAdsorptionSurface = state;}
  double adsorptionSurfaceOpacity() const override {return _adsorptionSurfaceOpacity;}
  void setAdsorptionSurfaceOpacity(double value) {_adsorptionSurfaceOpacity = value;}
  double adsorptionSurfaceIsoValue() const override {return _adsorptionSurfaceIsoValue;}
  void setAdsorptionSurfaceIsoValue(double value)  {_adsorptionSurfaceIsoValue = value;}
  double adsorptionSurfaceMinimumValue() const override {return _adsorptionSurfaceMinimumValue;}
  void setAdsorptionSurfaceMinimumValue(double value) override {_adsorptionSurfaceMinimumValue = value;}

  int adsorptionSurfaceSize() const override {return _adsorptionSurfaceSize;}
  double2 adsorptionSurfaceProbeParameters() const override final;
 // int adsorptionSurfaceNumberOfTriangles() const override {return 1;}

  ProbeMolecule adsorptionSurfaceProbeMolecule() const {return _adsorptionSurfaceProbeMolecule;}
  void setAdsorptionSurfaceProbeMolecule(ProbeMolecule value) {_adsorptionSurfaceProbeMolecule = value;}

  bool adsorptionSurfaceFrontSideHDR() const override {return _adsorptionSurfaceFrontSideHDR;}
  void setAdsorptionSurfaceFrontSideHDR(bool state) {_adsorptionSurfaceFrontSideHDR = state;}
  double adsorptionSurfaceFrontSideHDRExposure() const override {return _adsorptionSurfaceFrontSideHDRExposure;}
  void setAdsorptionSurfaceFrontSideHDRExposure(double value) {_adsorptionSurfaceFrontSideHDRExposure = value;}
  QColor adsorptionSurfaceFrontSideAmbientColor() const override {return _adsorptionSurfaceFrontSideAmbientColor;}
  void setAdsorptionSurfaceFrontSideAmbientColor(QColor color) {_adsorptionSurfaceFrontSideAmbientColor = color;}
  QColor adsorptionSurfaceFrontSideDiffuseColor() const override {return _adsorptionSurfaceFrontSideDiffuseColor;}
  void setAdsorptionSurfaceFrontSideDiffuseColor(QColor color) {_adsorptionSurfaceFrontSideDiffuseColor = color;}
  QColor adsorptionSurfaceFrontSideSpecularColor() const override {return _adsorptionSurfaceFrontSideSpecularColor;}
  void setAdsorptionSurfaceFrontSideSpecularColor(QColor color) {_adsorptionSurfaceFrontSideSpecularColor = color;}
  double adsorptionSurfaceFrontSideDiffuseIntensity() const override {return _adsorptionSurfaceFrontSideDiffuseIntensity;}
  void setAdsorptionSurfaceFrontSideDiffuseIntensity(double value) {_adsorptionSurfaceFrontSideDiffuseIntensity = value;}
  double adsorptionSurfaceFrontSideAmbientIntensity() const override {return _adsorptionSurfaceFrontSideAmbientIntensity;}
  void setAdsorptionSurfaceFrontSideAmbientIntensity(double value) {_adsorptionSurfaceFrontSideAmbientIntensity = value;}
  double adsorptionSurfaceFrontSideSpecularIntensity() const override {return _adsorptionSurfaceFrontSideSpecularIntensity;}
  void setAdsorptionSurfaceFrontSideSpecularIntensity(double value) {_adsorptionSurfaceFrontSideSpecularIntensity = value;}
  double adsorptionSurfaceFrontSideShininess() const override {return _adsorptionSurfaceFrontSideShininess;}
  void setAdsorptionSurfaceFrontSideShininess(double value) {_adsorptionSurfaceFrontSideShininess = value;}

  double adsorptionSurfaceHue() const override {return _adsorptionSurfaceHue;}
  void setAdsorptionSurfaceHue(double value) {_adsorptionSurfaceHue = value;}
  double adsorptionSurfaceSaturation() const override {return _adsorptionSurfaceSaturation;}
  void setAdsorptionSurfaceSaturation(double value) {_adsorptionSurfaceSaturation = value;}
  double adsorptionSurfaceValue() const override {return _adsorptionSurfaceValue;}
  void setAdsorptionSurfaceValue(double value) {_adsorptionSurfaceValue = value;}

  bool adsorptionSurfaceBackSideHDR() const override {return _adsorptionSurfaceBackSideHDR;}
  void setAdsorptionSurfaceBackSideHDR(bool state) {_adsorptionSurfaceBackSideHDR = state;}
  double adsorptionSurfaceBackSideHDRExposure() const override {return _adsorptionSurfaceBackSideHDRExposure;}
  void setAdsorptionSurfaceBackSideHDRExposure(double value) {_adsorptionSurfaceBackSideHDRExposure = value;}
  QColor adsorptionSurfaceBackSideAmbientColor() const override {return _adsorptionSurfaceBackSideAmbientColor;}
  void setAdsorptionSurfaceBackSideAmbientColor(QColor color) {_adsorptionSurfaceBackSideAmbientColor = color;}
  QColor adsorptionSurfaceBackSideDiffuseColor() const override {return _adsorptionSurfaceBackSideDiffuseColor;}
  void setAdsorptionSurfaceBackSideDiffuseColor(QColor color) {_adsorptionSurfaceBackSideDiffuseColor = color;}
  QColor adsorptionSurfaceBackSideSpecularColor() const override {return _adsorptionSurfaceBackSideSpecularColor;}
  void setAdsorptionSurfaceBackSideSpecularColor(QColor color) {_adsorptionSurfaceBackSideSpecularColor = color;}
  double adsorptionSurfaceBackSideDiffuseIntensity() const override {return _adsorptionSurfaceBackSideDiffuseIntensity;}
  void setAdsorptionSurfaceBackSideDiffuseIntensity(double value) {_adsorptionSurfaceBackSideDiffuseIntensity = value;}
  double adsorptionSurfaceBackSideAmbientIntensity() const override {return _adsorptionSurfaceBackSideAmbientIntensity;}
  void setAdsorptionSurfaceBackSideAmbientIntensity(double value) {_adsorptionSurfaceBackSideAmbientIntensity = value;}
  double adsorptionSurfaceBackSideSpecularIntensity() const override {return _adsorptionSurfaceBackSideSpecularIntensity;}
  void setAdsorptionSurfaceBackSideSpecularIntensity(double value) {_adsorptionSurfaceBackSideSpecularIntensity = value;}
  double adsorptionSurfaceBackSideShininess() const override {return _adsorptionSurfaceBackSideShininess;}
  void setAdsorptionSurfaceBackSideShininess(double value) {_adsorptionSurfaceBackSideShininess = value;}

  void setRepresentationStyle(RepresentationStyle style);
  void setRepresentationStyle(RepresentationStyle style, const SKColorSets &colorSets);
  RepresentationStyle atomRepresentationStyle() {return _atomRepresentationStyle;}
  void setRepresentationType(RepresentationType);
  RepresentationType atomRepresentationType() {return _atomRepresentationType;}
  bool isUnity() const override final {return _atomRepresentationType == RepresentationType::unity;}
  void recheckRepresentationStyle();

  void setRepresentationColorSchemeIdentifier(const QString colorSchemeName, const SKColorSets &colorSets);
  QString atomColorSchemeIdentifier() {return _atomColorSchemeIdentifier;}
  void setColorSchemeOrder(SKColorSet::ColorSchemeOrder order) {_atomColorSchemeOrder = order;}
  SKColorSet::ColorSchemeOrder colorSchemeOrder() {return _atomColorSchemeOrder;}

  QString atomForceFieldIdentifier() {return _atomForceFieldIdentifier;}
  void setAtomForceFieldIdentifier(QString identifier, ForceFieldSets &forceFieldSets);
  void updateForceField(ForceFieldSets &forceFieldSets);
  void setForceFieldSchemeOrder(ForceFieldSet::ForceFieldSchemeOrder order) {_atomForceFieldOrder = order;}
  ForceFieldSet::ForceFieldSchemeOrder forceFieldSchemeOrder() {return _atomForceFieldOrder;}

  QString authorFirstName() {return _authorFirstName;}
  void setAuthorFirstName(QString name) {_authorFirstName = name;}
  QString authorMiddleName() {return _authorMiddleName;}
  void setAuthorMiddleName(QString name) {_authorMiddleName = name;}
  QString authorLastName() {return _authorLastName;}
  void setAuthorLastName(QString name) {_authorLastName = name;}
  QString authorOrchidID() {return _authorOrchidID;}
  void setAuthorOrchidID(QString name) {_authorOrchidID = name;}
  QString authorResearcherID() {return _authorResearcherID;}
  void setAuthorResearcherID(QString name) {_authorResearcherID = name;}
  QString authorAffiliationUniversityName() {return _authorAffiliationUniversityName;}
  void setAuthorAffiliationUniversityName(QString name) {_authorAffiliationUniversityName = name;}
  QString authorAffiliationFacultyName() {return _authorAffiliationFacultyName;}
  void setAuthorAffiliationFacultyName(QString name) {_authorAffiliationFacultyName = name;}
  QString authorAffiliationInstituteName() {return _authorAffiliationInstituteName;}
  void setAuthorAffiliationInstituteName(QString name) {_authorAffiliationInstituteName = name;}
  QString authorAffiliationCityName() {return _authorAffiliationCityName;}
  void setAuthorAffiliationCityName(QString name) {_authorAffiliationCityName = name;}
  QString authorAffiliationCountryName() {return _authorAffiliationCountryName;}
  void setAuthorAffiliationCountryName(QString name) {_authorAffiliationCountryName = name;}

  QDate creationDate() {return _creationDate;}
  void setCreationDate(QDate date) {_creationDate = date;}
  QString creationTemperature() {return _creationTemperature;}
  void setCreationTemperature(QString name) {_creationTemperature = name;}
  TemperatureScale creationTemperatureScale() {return _creationTemperatureScale;}
  void setCreationTemperatureScale(TemperatureScale scale) {_creationTemperatureScale = scale;}
  QString creationPressure() {return _creationPressure;}
  void setCreationPressure(QString pressure) {_creationPressure = pressure;}
  PressureScale creationPressureScale() {return _creationPressureScale;}
  void setCreationPressureScale(PressureScale scale) {_creationPressureScale = scale;}
  CreationMethod creationMethod() {return _creationMethod;}
  void setCreationMethod(CreationMethod method) {_creationMethod = method;}
  UnitCellRelaxationMethod creationUnitCellRelaxationMethod() {return _creationUnitCellRelaxationMethod;}
  void setCreationUnitCellRelaxationMethod(UnitCellRelaxationMethod method) {_creationUnitCellRelaxationMethod = method;}
  QString creationAtomicPositionsSoftwarePackage() {return _creationAtomicPositionsSoftwarePackage;}
  void setCreationAtomicPositionsSoftwarePackage(QString name) {_creationAtomicPositionsSoftwarePackage = name;}
  IonsRelaxationAlgorithm creationAtomicPositionsIonsRelaxationAlgorithm() {return _creationAtomicPositionsIonsRelaxationAlgorithm;}
  void setCreationAtomicPositionsIonsRelaxationAlgorithm(IonsRelaxationAlgorithm algorithm) {_creationAtomicPositionsIonsRelaxationAlgorithm = algorithm;}
  IonsRelaxationCheck creationAtomicPositionsIonsRelaxationCheck() {return _creationAtomicPositionsIonsRelaxationCheck;}
  void setCreationAtomicPositionsIonsRelaxationCheck(IonsRelaxationCheck check) {_creationAtomicPositionsIonsRelaxationCheck = check;}
  QString creationAtomicPositionsForcefield() {return _creationAtomicPositionsForcefield;}
  void setCreationAtomicPositionsForcefield(QString name) {_creationAtomicPositionsForcefield = name;}
  QString creationAtomicPositionsForcefieldDetails() {return _creationAtomicPositionsForcefieldDetails;}
  void setCreationAtomicPositionsForcefieldDetails(QString name) {_creationAtomicPositionsForcefieldDetails = name;}
  QString creationAtomicChargesSoftwarePackage() {return _creationAtomicChargesSoftwarePackage;}
  void setCreationAtomicChargesSoftwarePackage(QString name) {_creationAtomicChargesSoftwarePackage = name;}
  QString creationAtomicChargesAlgorithms() {return _creationAtomicChargesAlgorithms;}
  void setCreationAtomicChargesAlgorithms(QString name) {_creationAtomicChargesAlgorithms = name;}
  QString creationAtomicChargesForcefield() {return _creationAtomicChargesForcefield;}
  void setCreationAtomicChargesForcefield(QString name) {_creationAtomicChargesForcefield = name;}
  QString creationAtomicChargesForcefieldDetails() {return _creationAtomicChargesForcefieldDetails;}
  void setCreationAtomicChargesForcefieldDetails(QString name) {_creationAtomicChargesForcefieldDetails = name;}

  QString experimentalMeasurementRadiation() {return _experimentalMeasurementRadiation;}
  void setExperimentalMeasurementRadiation(QString name) {_experimentalMeasurementRadiation = name;}
  QString experimentalMeasurementWaveLength() {return _experimentalMeasurementWaveLength;}
  void setExperimentalMeasurementWaveLength(QString name) {_experimentalMeasurementWaveLength = name;}
  QString experimentalMeasurementThetaMin() {return _experimentalMeasurementThetaMin;}
  void setExperimentalMeasurementThetaMin(QString name) {_experimentalMeasurementThetaMin = name;}
  QString experimentalMeasurementThetaMax() {return _experimentalMeasurementThetaMax;}
  void setExperimentalMeasurementThetaMax(QString name) {_experimentalMeasurementThetaMax = name;}
  QString experimentalMeasurementIndexLimitsHmin() {return _experimentalMeasurementIndexLimitsHmin;}
  void setExperimentalMeasurementIndexLimitsHmin(QString name) {_experimentalMeasurementIndexLimitsHmin = name;}
  QString experimentalMeasurementIndexLimitsHmax() {return _experimentalMeasurementIndexLimitsHmax;}
  void setExperimentalMeasurementIndexLimitsHmax(QString name) {_experimentalMeasurementIndexLimitsHmax = name;}
  QString experimentalMeasurementIndexLimitsKmin() {return _experimentalMeasurementIndexLimitsKmin;}
  void setExperimentalMeasurementIndexLimitsKmin(QString name) {_experimentalMeasurementIndexLimitsKmin = name;}
  QString experimentalMeasurementIndexLimitsKmax() {return _experimentalMeasurementIndexLimitsKmax;}
  void setExperimentalMeasurementIndexLimitsKmax(QString name) {_experimentalMeasurementIndexLimitsKmax = name;}
  QString experimentalMeasurementIndexLimitsLmin() {return _experimentalMeasurementIndexLimitsLmin;}
  void setExperimentalMeasurementIndexLimitsLmin(QString name) {_experimentalMeasurementIndexLimitsLmin = name;}
  QString experimentalMeasurementIndexLimitsLmax() {return _experimentalMeasurementIndexLimitsLmax;}
  void setExperimentalMeasurementIndexLimitsLmax(QString name) {_experimentalMeasurementIndexLimitsLmax = name;}
  QString experimentalMeasurementNumberOfSymmetryIndependentReflections() {return _experimentalMeasurementNumberOfSymmetryIndependentReflections;}
  void setExperimentalMeasurementNumberOfSymmetryIndependentReflections(QString name) {_experimentalMeasurementNumberOfSymmetryIndependentReflections = name;}
  QString experimentalMeasurementSoftware() {return _experimentalMeasurementSoftware;}
  void setExperimentalMeasurementSoftware(QString name) {_experimentalMeasurementSoftware = name;}
  QString experimentalMeasurementRefinementDetails() {return _experimentalMeasurementRefinementDetails;}
  QString experimentalMeasurementGoodnessOfFit() {return _experimentalMeasurementGoodnessOfFit;}
  void setExperimentalMeasurementRefinementDetails(QString name) {_experimentalMeasurementGoodnessOfFit = name;}
  QString experimentalMeasurementRFactorGt() {return _experimentalMeasurementRFactorGt;}
  void setExperimentalMeasurementRFactorGt(QString name) {_experimentalMeasurementRFactorGt = name;}
  QString experimentalMeasurementRFactorAll() {return _experimentalMeasurementRFactorAll;}
  void setExperimentalMeasurementRFactorAll(QString name) {_experimentalMeasurementRFactorAll = name;}

  QString chemicalFormulaMoiety() {return _chemicalFormulaMoiety;}
  void setChemicalFormulaMoiety(QString name) {_chemicalFormulaMoiety = name;}
  QString chemicalFormulaSum() {return _chemicalFormulaSum;}
  void setChemicalFormulaSum(QString name) {_chemicalFormulaSum = name;}
  QString chemicalNameSystematic() {return _chemicalNameSystematic;}
  void setChemicalNameSystematic(QString name) {_chemicalNameSystematic = name;}

  QString citationArticleTitle() {return _citationArticleTitle;}
  void setCitationArticleTitle(QString name) {_citationArticleTitle = name;}
  QString citationJournalTitle() {return _citationJournalTitle;}
  void setCitationJournalTitle(QString name) {_citationJournalTitle = name;}
  QString citationAuthors() {return _citationAuthors;}
  void setCitationAuthors(QString name) {_citationAuthors = name;}
  QString citationJournalVolume() {return _citationJournalVolume;}
  void setCitationJournalVolume(QString name) {_citationJournalVolume = name;}
  QString citationJournalNumber() {return _citationJournalNumber;}
  void setCitationJournalNumber(QString name) {_citationJournalNumber = name;}
  QString citationJournalPageNumbers() {return _citationJournalPageNumbers;}
  void setCitationJournalPageNumbers(QString name) {_citationJournalPageNumbers = name;}
  QString citationDOI() {return _citationDOI;}
  void setCitationDOI(QString name) {_citationDOI = name;}
  QDate citationPublicationDate() {return _citationPublicationDate;}
  void setCitationPublicationDate(QDate date) {_citationPublicationDate = date;}
  QString citationDatebaseCodes() {return _citationDatebaseCodes;}
  void setCitationDatebaseCodes(QString name) {_citationDatebaseCodes = name;}

  ProbeMolecule frameworkProbeMolecule() const {return _frameworkProbeMolecule;}
  void setFrameworkProbeMolecule(ProbeMolecule value) {_frameworkProbeMolecule = value;}
  void recomputeDensityProperties() override;
  QString structureMaterialType() {return _structureMaterialType;}
  double structureMass() {return _structureMass;}
  double structureDensity() {return _structureDensity;}
  double structureHeliumVoidFraction() {return _structureHeliumVoidFraction;}
  void setStructureHeliumVoidFraction(double value) override final {_structureHeliumVoidFraction = value;}
  void setStructureNitrogenSurfaceArea(double value) override final;
  double2 frameworkProbeParameters() const override final;
  double structureSpecificVolume() {return _structureSpecificVolume;}
  double structureAccessiblePoreVolume () {return _structureAccessiblePoreVolume;}
  double structureVolumetricNitrogenSurfaceArea() {return _structureVolumetricNitrogenSurfaceArea;}
  double structureGravimetricNitrogenSurfaceArea() {return _structureGravimetricNitrogenSurfaceArea;}
  int structureNumberOfChannelSystems() {return _structureNumberOfChannelSystems;}
  void setStructureNumberOfChannelSystems(int value) {_structureNumberOfChannelSystems = value;}
  int structureNumberOfInaccessiblePockets() {return _structureNumberOfInaccessiblePockets;}
  void setStructureNumberOfInaccessiblePockets(int value) {_structureNumberOfInaccessiblePockets = value;}
  int structureDimensionalityOfPoreSystem() {return _structureDimensionalityOfPoreSystem;}
  void setStructureDimensionalityOfPoreSystem(int value) {_structureDimensionalityOfPoreSystem = value;}
  double structureLargestCavityDiameter() {return _structureLargestCavityDiameter;}
  void setStructureLargestCavityDiameter(double value) {_structureLargestCavityDiameter = value;}
  double structureRestrictingPoreLimitingDiameter() {return _structureRestrictingPoreLimitingDiameter;}
  void setStructureRestrictingPoreLimitingDiameter(double value) {_structureRestrictingPoreLimitingDiameter = value;}
  double structureLargestCavityDiameterAlongAviablePath() {return _structureLargestCavityDiameterAlongAViablePath;}
  void setStructureLargestCavityDiameterAlongAviablePath(double value) {_structureLargestCavityDiameterAlongAViablePath = value;}

  virtual void setSpaceGroupHallNumber(int HallNumber)  {_spaceGroup = SKSpaceGroup(HallNumber); std::cout << "BASECLASS" << std::endl;}
  SKSpaceGroup& spaceGroup()  {return _spaceGroup;}

  void computeBonds() override {;}
protected:
  qint64 _versionNumber{7};
  QString _displayName = QString("test123");

  std::shared_ptr<SKAtomTreeController> _atomsTreeController;
  std::shared_ptr<SKBondSetController> _bondSetController;

  SKSpaceGroup _spaceGroup;
  std::shared_ptr<SKCell> _cell;

  double3 _origin = double3(0.0, 0.0, 0.0);
  double3 _scaling = double3(1.0, 1.0, 1.0);
  simd_quatd _orientation = simd_quatd(1.0, double3(0.0, 0.0, 0.0));
  double _rotationDelta = 5.0;

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

  bool _periodic = false;
  bool _isVisible = true;

  double _minimumGridEnergyValue = 0.0f;

  bool _drawAtoms =  true;

  RepresentationType _atomRepresentationType = RepresentationType::sticks_and_balls;
  RepresentationStyle _atomRepresentationStyle = RepresentationStyle::defaultStyle;
  QString _atomForceFieldIdentifier = QString("Default");
  ForceFieldSet::ForceFieldSchemeOrder _atomForceFieldOrder = ForceFieldSet::ForceFieldSchemeOrder::elementOnly;
  QString _atomColorSchemeIdentifier = QString("Jmol");
  SKColorSet::ColorSchemeOrder _atomColorSchemeOrder = SKColorSet::ColorSchemeOrder::elementOnly;

  RKSelectionStyle _atomSelectionStyle = RKSelectionStyle::WorleyNoise3D;
  double _atomSelectionStripesDensity = 0.25;
  double _atomSelectionStripesFrequency = 12.0;
  double _atomSelectionWorleyNoise3DFrequency = 2.0;
  double _atomSelectionWorleyNoise3DJitter = 1.0;
  double _atomSelectionScaling = 1.2;
  double _selectionIntensity = 1.0;


  double _atomHue = 1.0;
  double _atomSaturation = 1.0;
  double _atomValue = 1.0;
  double _atomScaleFactor = 1.0;

  bool _atomAmbientOcclusion = false;
  qint64 _atomAmbientOcclusionPatchNumber = 256;
  qint64 _atomAmbientOcclusionTextureSize = 1024;
  qint64 _atomAmbientOcclusionPatchSize = 16;
  //  public var _atomCacheAmbientOcclusionTexture: [CUnsignedChar] = [CUnsignedChar]();

  bool _atomHDR = true;
  double _atomHDRExposure = 1.5;
  double _atomSelectionIntensity = 0.5;

  QColor _atomAmbientColor = QColor(255, 255, 255, 255);
  QColor _atomDiffuseColor = QColor(255, 255, 255, 255);
  QColor _atomSpecularColor = QColor(255, 255, 255, 255);
  double _atomAmbientIntensity= 0.2;
  double _atomDiffuseIntensity = 1.0;
  double _atomSpecularIntensity = 1.0;
  double _atomShininess = 4.0;

  bool _drawBonds = true;

  double _bondScaleFactor = 1.0;
  RKBondColorMode _bondColorMode = RKBondColorMode::uniform;

  QColor _bondAmbientColor = QColor(255, 255, 255, 255);
  QColor _bondDiffuseColor = QColor(200, 200, 200, 255);
  QColor _bondSpecularColor = QColor(255, 255, 255, 255);
  double _bondAmbientIntensity = 0.1;
  double _bondDiffuseIntensity = 1.0;
  double _bondSpecularIntensity = 1.0;
  double _bondShininess = 4.0;

  bool _bondHDR = true;
  double _bondHDRExposure = 1.5;

  double _bondHue = 1.0;
  double _bondSaturation = 1.0;
  double _bondValue = 1.0;

  RKSelectionStyle _bondSelectionStyle = RKSelectionStyle::striped;
  double _bondSelectionScaling = 1.2;
  double _bondSelectionStripesDensity = 0.25;
  double _bondSelectionStripesFrequency = 12.0;
  double _bondSelectionWorleyNoise3DFrequency = 2.0;
  double _bondSelectionWorleyNoise3DJitter = 1.0;
  double _bondSelectionIntensity = 0.5;

  bool _bondAmbientOcclusion = false;


  RKTextType _atomTextType = RKTextType::none;
  QString _atomTextFont = QString("Helvetica");
  double _atomTextScaling = 1.0;
  QColor _atomTextColor = QColor(0, 0, 0, 255);
  QColor _atomTextGlowColor = QColor(0, 255, 0, 255);
  RKTextStyle _atomTextStyle = RKTextStyle::flatBillboard;
  RKTextEffect _atomTextEffect = RKTextEffect::none;
  RKTextAlignment _atomTextAlignment = RKTextAlignment::center;
  double3 _atomTextOffset = double3();

  bool _drawUnitCell = true;
  double _unitCellScaleFactor = 1.0;
  QColor _unitCellDiffuseColor = QColor(255, 255, 255, 255);
  double _unitCellDiffuseIntensity = 1.0;

  bool _drawAdsorptionSurface = false;
  double _adsorptionSurfaceOpacity = 1.0;
  double _adsorptionSurfaceIsoValue = 0.0;
  double _adsorptionSurfaceMinimumValue = -1000.0;

  qint64 _adsorptionSurfaceSize = 128;
  qint64 _adsorptionSurfaceNumberOfTriangles = 0;

  ProbeMolecule _adsorptionSurfaceProbeMolecule = ProbeMolecule::helium;

  double _adsorptionSurfaceHue = 1.0;
  double _adsorptionSurfaceSaturation = 1.0;
  double _adsorptionSurfaceValue = 1.0;

  bool _adsorptionSurfaceFrontSideHDR = true;
  double _adsorptionSurfaceFrontSideHDRExposure = 1.5;
  QColor _adsorptionSurfaceFrontSideAmbientColor = QColor(0, 0, 0, 255);
  QColor _adsorptionSurfaceFrontSideDiffuseColor = QColor(150, 171, 186, 255);
  QColor _adsorptionSurfaceFrontSideSpecularColor = QColor(230, 230, 230, 1.0);
  double _adsorptionSurfaceFrontSideDiffuseIntensity = 1.0;
  double _adsorptionSurfaceFrontSideAmbientIntensity = 0.2;
  double _adsorptionSurfaceFrontSideSpecularIntensity = 1.0;
  double _adsorptionSurfaceFrontSideShininess = 4.0;

  bool _adsorptionSurfaceBackSideHDR = true;
  double _adsorptionSurfaceBackSideHDRExposure = 1.5;
  QColor _adsorptionSurfaceBackSideAmbientColor = QColor(0, 0, 0, 255);
  QColor _adsorptionSurfaceBackSideDiffuseColor = QColor(150, 171, 186, 255);
  QColor _adsorptionSurfaceBackSideSpecularColor = QColor(230, 230, 230, 255);
  double _adsorptionSurfaceBackSideDiffuseIntensity = 1.0;
  double _adsorptionSurfaceBackSideAmbientIntensity = 0.2;
  double _adsorptionSurfaceBackSideSpecularIntensity = 1.0;
  double _adsorptionSurfaceBackSideShininess = 4.0;

  double3 _selectionCOMTranslation = double3(0.0, 0.0, 0.0);
  int _selectionRotationIndex = 0;
  double3x3 _selectionBodyFixedBasis = double3x3();

  StructureType _structureType = StructureType::framework;
  ProbeMolecule _frameworkProbeMolecule = ProbeMolecule::nitrogen;
  QString _structureMaterialType = QString("Unspecified");
  double _structureMass = 0.0;
  double _structureDensity = 0.0;
  double _structureHeliumVoidFraction = 0.0;
  double _structureSpecificVolume = 0.0;
  double _structureAccessiblePoreVolume = 0.0;
  double _structureVolumetricNitrogenSurfaceArea = 0.0;
  double _structureGravimetricNitrogenSurfaceArea = 0.0;
  qint64 _structureNumberOfChannelSystems = 0;
  qint64 _structureNumberOfInaccessiblePockets = 0;
  qint64 _structureDimensionalityOfPoreSystem = 0;
  double _structureLargestCavityDiameter = 0.0;
  double _structureRestrictingPoreLimitingDiameter = 0.0;
  double _structureLargestCavityDiameterAlongAViablePath = 0.0;

  QString _authorFirstName = QString("");
  QString _authorMiddleName = QString("");
  QString _authorLastName = QString("");
  QString _authorOrchidID = QString("");
  QString _authorResearcherID = QString("");
  QString _authorAffiliationUniversityName = QString("");
  QString _authorAffiliationFacultyName = QString("");
  QString _authorAffiliationInstituteName = QString("");
  QString _authorAffiliationCityName = QString("");
  QString _authorAffiliationCountryName = QString("Netherlands");

  QDate _creationDate = QDate().currentDate();
  QString _creationTemperature = QString();
  TemperatureScale _creationTemperatureScale = TemperatureScale::Kelvin;
  QString _creationPressure = QString("");
  PressureScale _creationPressureScale = PressureScale::Pascal;
  CreationMethod  _creationMethod = CreationMethod::unknown;
  UnitCellRelaxationMethod _creationUnitCellRelaxationMethod = UnitCellRelaxationMethod::unknown;
  QString _creationAtomicPositionsSoftwarePackage = QString("");
  IonsRelaxationAlgorithm _creationAtomicPositionsIonsRelaxationAlgorithm = IonsRelaxationAlgorithm::unknown;
  IonsRelaxationCheck _creationAtomicPositionsIonsRelaxationCheck = IonsRelaxationCheck::unknown;
  QString _creationAtomicPositionsForcefield = QString("");
  QString _creationAtomicPositionsForcefieldDetails = QString("");
  QString _creationAtomicChargesSoftwarePackage = QString("");
  QString _creationAtomicChargesAlgorithms = QString("");
  QString _creationAtomicChargesForcefield = QString("");
  QString _creationAtomicChargesForcefieldDetails = QString();

  QString _experimentalMeasurementRadiation = QString("");
  QString _experimentalMeasurementWaveLength = QString("");
  QString _experimentalMeasurementThetaMin = QString("");
  QString _experimentalMeasurementThetaMax = QString("");
  QString _experimentalMeasurementIndexLimitsHmin = QString("");
  QString _experimentalMeasurementIndexLimitsHmax = QString("");
  QString _experimentalMeasurementIndexLimitsKmin = QString("");
  QString _experimentalMeasurementIndexLimitsKmax = QString("");
  QString _experimentalMeasurementIndexLimitsLmin = QString("");
  QString _experimentalMeasurementIndexLimitsLmax = QString("");
  QString _experimentalMeasurementNumberOfSymmetryIndependentReflections = QString("");
  QString _experimentalMeasurementSoftware = QString("");
  QString _experimentalMeasurementRefinementDetails = QString("");
  QString _experimentalMeasurementGoodnessOfFit = QString("");
  QString _experimentalMeasurementRFactorGt = QString("");
  QString _experimentalMeasurementRFactorAll = QString();

  QString _chemicalFormulaMoiety = QString("");
  QString _chemicalFormulaSum = QString("");
  QString _chemicalNameSystematic = QString("");
  qint64 _cellFormulaUnitsZ = 0;

  QString _citationArticleTitle = QString("");
  QString _citationJournalTitle = QString("");
  QString _citationAuthors = QString("");
  QString _citationJournalVolume = QString("");
  QString _citationJournalNumber = QString("");
  QString _citationJournalPageNumbers = QString("");
  QString _citationDOI = QString("");
  QDate _citationPublicationDate = QDate().currentDate();
  QString _citationDatebaseCodes = QString();

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<Structure> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<Structure> &);
};

struct FrameConsumer
{
  virtual void setFrame(std::shared_ptr<Structure> structure) = 0;
};
