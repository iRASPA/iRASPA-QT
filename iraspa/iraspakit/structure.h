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
#include "displayable.h"
#include "object.h"
#include "infoviewer.h"
#include "cellviewer.h"
#include "atomviewer.h"
#include "bondviewer.h"
#include "primitivevisualappearanceviewer.h"
#include "atomvisualappearanceviewer.h"
#include "bondvisualappearanceviewer.h"
#include "adsorptionsurfacevisualappearanceviewer.h"
#include "atomtextvisualappearanceviewer.h"

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


class Structure: public Object, public InfoViewer, public CellViewer, public AtomViewer, public BondViewer,
                 public AtomVisualAppearanceViewer, public BondVisualAppearanceViewer,
                 public AdsorptionSurfaceVisualAppearanceViewer, public AtomTextVisualAppearanceViewer,
                 public RKRenderAtomicStructureSource, public RKRenderAdsorptionSurfaceSource
{
public:
  Structure();
  Structure(std::shared_ptr<SKAtomTreeController> atomTreeController);
  Structure(std::shared_ptr<SKStructure> structure);
  Structure(std::shared_ptr<Structure> clone);
  Structure(const Structure &structure);
  virtual ~Structure() {}

  virtual std::shared_ptr<Structure> clone() override {return nullptr;}

  virtual bool hasSymmetry() {return false;}
  virtual std::shared_ptr<Structure> superCell() const {return nullptr;}
  virtual std::shared_ptr<Structure> removeSymmetry() const {return nullptr;}
  virtual std::shared_ptr<Structure> wrapAtomsToCell() const {return nullptr;}
  virtual std::shared_ptr<Structure> flattenHierarchy() const {return nullptr;}
  virtual std::shared_ptr<Structure> appliedCellContentShift() const {return nullptr;}
  virtual std::vector<std::tuple<double3,int,double>> atomSymmetryData() const {return {};}
  virtual void setAtomSymmetryData(double3x3 unitCell, std::vector<std::tuple<double3,int,double>> atomData) {Q_UNUSED(unitCell); Q_UNUSED(atomData);};


  enum class StructureType: qint64
  {
    framework = 0, adsorbate = 1, cation = 2, ionicLiquid = 3, solvent = 4
  };

  ObjectType structureType() override {return ObjectType::structure;}

  virtual std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForFractionalPositions();
  virtual std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForCartesianPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToFractionalPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToCartesianPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToFractionalPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToCartesianPositions();

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
  void reComputeBoundingBox() override;

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



  void clearSelection();
  void setAtomSelection(int asymmetricAtomId);
  void setBondSelection(int asymmetricBondId);
  void addAtomToSelection(int asymmetricAtomId);
  void addBondToSelection(int asymmetricBondId);
  void toggleAtomSelection(int asymmetricAtomId);
  void toggleBondSelection(int asymmetricAtomId);
  void setAtomSelection(std::set<int>& atomIds);
  void addToAtomSelection(std::set<int>& atomIds);



  virtual double3 CartesianPosition(double3 position, int3 replicaPosition) const {Q_UNUSED(position); Q_UNUSED(replicaPosition); return double(); }
  virtual double3 FractionalPosition(double3 position, int3 replicaPosition) const {Q_UNUSED(position); Q_UNUSED(replicaPosition); return double(); }
  void convertAsymmetricAtomsToCartesian();
  void convertAsymmetricAtomsToFractional();

  int numberOfAtoms() const override {return 0;}
  int numberOfInternalBonds() const override {return 0;}
  int numberOfExternalBonds() const override {return 0;}

  void setAtomTreeController(std::shared_ptr<SKAtomTreeController> controller) {_atomsTreeController = controller;}
  std::shared_ptr<SKAtomTreeController> &atomsTreeController() override final {return _atomsTreeController;}
  std::shared_ptr<SKBondSetController> bondSetController() override final {return _bondSetController;}


  // primitive
  /*
  simd_quatd primitiveOrientation() const {return _primitiveOrientation;}
  void setPrimitiveOrientation(simd_quatd orientation) {_primitiveOrientation = orientation;}
  double primitiveRotationDelta() const {return _primitiveRotationDelta;}
  void setPrimitiveRotationDelta(double angle) {_primitiveRotationDelta = angle;}
  double3x3 transformationMatrix() const {return _primitiveTransformationMatrix;}
  void setTransformationMatrix(double3x3 matrix) {_primitiveTransformationMatrix = matrix;}
  double primitiveOpacity() const  {return _primitiveOpacity;}
  void setPrimitiveOpacity(double opacity) {_primitiveOpacity = opacity;}
  int primitiveNumberOfSides() const {return _primitiveNumberOfSides;}
  void setPrimitiveNumberOfSides(int numberOfSides) {_primitiveNumberOfSides = numberOfSides;}
  bool primitiveIsCapped() const {return _primitiveIsCapped;}
  void setPrimitiveIsCapped(bool isCapped) {_primitiveIsCapped = isCapped;}

  RKSelectionStyle primitiveSelectionStyle() const {return _primitiveSelectionStyle;}
  void setPrimitiveSelectionStyle(RKSelectionStyle style) {_primitiveSelectionStyle = style;}
  double primitiveSelectionScaling() const {return _primitiveSelectionScaling;}
  void setPrimitiveSelectionScaling(double scaling) {_primitiveSelectionScaling = scaling;}
  double primitiveSelectionIntensity() const {return _primitiveSelectionIntensity;}
  void setPrimitiveSelectionIntensity(double value)  {_primitiveSelectionIntensity = value;}

  double primitiveSelectionStripesDensity() const {return _primitiveSelectionStripesDensity;}
  double primitiveSelectionStripesFrequency() const  {return _primitiveSelectionStripesFrequency;}
  double primitiveSelectionWorleyNoise3DFrequency() const {return _primitiveSelectionWorleyNoise3DFrequency;}
  double primitiveSelectionWorleyNoise3DJitter() const {return _primitiveSelectionWorleyNoise3DJitter;}

  double primitiveSelectionFrequency() const ;
  void setPrimitiveSelectionFrequency(double value) ;
  double primitiveSelectionDensity() const ;
  void setPrimitiveSelectionDensity(double value) ;

  double primitiveHue() const  {return _primitiveHue;}
  void setPrimitiveHue(double value)  {_primitiveHue = value;}
  double primitiveSaturation() const  {return _primitiveSaturation;}
  void setPrimitiveSaturation(double value)  {_primitiveSaturation = value;}
  double primitiveValue() const  {return _primitiveValue;}
  void setPrimitiveValue(double value)  {_primitiveValue = value;}

  bool frontPrimitiveHDR() const  {return _primitiveFrontSideHDR;}
  void setFrontPrimitiveHDR(bool isHDR)  {_primitiveFrontSideHDR = isHDR;}
  double frontPrimitiveHDRExposure() const  {return _primitiveFrontSideHDRExposure;}
  void setFrontPrimitiveHDRExposure(double exposure)  {_primitiveFrontSideHDRExposure = exposure;}
  double frontPrimitiveAmbientIntensity() const  {return _primitiveFrontSideAmbientIntensity;}
  void setFrontPrimitiveAmbientIntensity(double intensity)  {_primitiveFrontSideAmbientIntensity = intensity;}
  double frontPrimitiveDiffuseIntensity() const  {return _primitiveFrontSideDiffuseIntensity;}
  void setFrontPrimitiveDiffuseIntensity(double intensity)  {_primitiveFrontSideDiffuseIntensity = intensity;}
  double frontPrimitiveSpecularIntensity() const  {return _primitiveFrontSideSpecularIntensity;}
  void setFrontPrimitiveSpecularIntensity(double intensity)  {_primitiveFrontSideSpecularIntensity = intensity;}
  QColor frontPrimitiveAmbientColor() const  {return _primitiveFrontSideAmbientColor;}
  void setFrontPrimitiveAmbientColor(QColor color)  {_primitiveFrontSideAmbientColor = color;}
  QColor frontPrimitiveDiffuseColor() const  {return _primitiveFrontSideDiffuseColor;}
  void setFrontPrimitiveDiffuseColor(QColor color)  {_primitiveFrontSideDiffuseColor = color;}
  QColor frontPrimitiveSpecularColor() const  {return _primitiveFrontSideSpecularColor;}
  void setFrontPrimitiveSpecularColor(QColor color)  {_primitiveFrontSideSpecularColor = color;}
  double frontPrimitiveShininess() const  {return _primitiveFrontSideShininess;}
  void setFrontPrimitiveShininess(double value)  {_primitiveFrontSideShininess = value;}

  bool backPrimitiveHDR() const  {return _primitiveBackSideHDR;}
  void setBackPrimitiveHDR(bool isHDR)  {_primitiveBackSideHDR = isHDR;}
  double backPrimitiveHDRExposure() const  {return _primitiveBackSideHDRExposure;}
  void setBackPrimitiveHDRExposure(double exposure)  {_primitiveBackSideHDRExposure = exposure;}
  double backPrimitiveAmbientIntensity() const  {return _primitiveBackSideAmbientIntensity;}
  void setBackPrimitiveAmbientIntensity(double intensity)  {_primitiveBackSideAmbientIntensity = intensity;}
  double backPrimitiveDiffuseIntensity() const  {return _primitiveBackSideDiffuseIntensity;}
  void setBackPrimitiveDiffuseIntensity(double intensity)  {_primitiveBackSideDiffuseIntensity = intensity;}
  double backPrimitiveSpecularIntensity() const  {return _primitiveBackSideSpecularIntensity;}
  void setBackPrimitiveSpecularIntensity(double intensity)  {_primitiveBackSideSpecularIntensity = intensity;}
  QColor backPrimitiveAmbientColor() const  {return _primitiveBackSideAmbientColor;}
  void setBackPrimitiveAmbientColor(QColor color)  {_primitiveBackSideAmbientColor = color;}
  QColor backPrimitiveDiffuseColor() const  {return _primitiveBackSideDiffuseColor;}
  void setBackPrimitiveDiffuseColor(QColor color)  {_primitiveBackSideDiffuseColor = color;}
  QColor backPrimitiveSpecularColor() const  {return _primitiveBackSideSpecularColor;}
  void setBackPrimitiveSpecularColor(QColor color)  {_primitiveBackSideSpecularColor = color;}
  double backPrimitiveShininess() const  {return _primitiveBackSideShininess;}
  void setBackPrimitiveShininess(double value)  {_primitiveBackSideShininess = value;}
*/
  // atoms
  bool drawAtoms() const override {return _drawAtoms;}
  void setDrawAtoms(bool drawAtoms) override {_drawAtoms = drawAtoms;}

  void setRepresentationType(RepresentationType) override final;
  RepresentationType atomRepresentationType() override {return _atomRepresentationType;}
  void setRepresentationStyle(RepresentationStyle style) override;
  void setRepresentationStyle(RepresentationStyle style, const SKColorSets &colorSets) override;
  RepresentationStyle atomRepresentationStyle() override {return _atomRepresentationStyle;}
  bool isUnity() const override final {return _atomRepresentationType == RepresentationType::unity;}
  void recheckRepresentationStyle() override;

  void setRepresentationColorSchemeIdentifier(const QString colorSchemeName, const SKColorSets &colorSets) override;
  QString atomColorSchemeIdentifier() override {return _atomColorSchemeIdentifier;}
  void setColorSchemeOrder(SKColorSet::ColorSchemeOrder order) override {_atomColorSchemeOrder = order;}
  SKColorSet::ColorSchemeOrder colorSchemeOrder() override {return _atomColorSchemeOrder;}

  QString atomForceFieldIdentifier() override {return _atomForceFieldIdentifier;}
  void setAtomForceFieldIdentifier(QString identifier, ForceFieldSets &forceFieldSets) override;
  void updateForceField(ForceFieldSets &forceFieldSets) override;
  void setForceFieldSchemeOrder(ForceFieldSet::ForceFieldSchemeOrder order) override {_atomForceFieldOrder = order;}
  ForceFieldSet::ForceFieldSchemeOrder forceFieldSchemeOrder() override {return _atomForceFieldOrder;}

  QColor atomAmbientColor() const override {return _atomAmbientColor;}
  void setAtomAmbientColor(QColor color) override {_atomAmbientColor = color;}
  QColor atomDiffuseColor() const override {return _atomDiffuseColor;}
  void setAtomDiffuseColor(QColor color) override {_atomDiffuseColor = color;}
  QColor atomSpecularColor() const override {return _atomSpecularColor;}
  void setAtomSpecularColor(QColor color) override {_atomSpecularColor = color;}

  double atomAmbientIntensity() const override {return _atomAmbientIntensity;}
  void setAtomAmbientIntensity(double value) override {_atomAmbientIntensity = value;}
  double atomDiffuseIntensity() const override {return _atomDiffuseIntensity;}
  void setAtomDiffuseIntensity(double value) override {_atomDiffuseIntensity = value;}
  double atomSpecularIntensity() const override {return _atomSpecularIntensity;}
  void setAtomSpecularIntensity(double value) override {_atomSpecularIntensity = value;}
  double atomShininess() const override {return _atomShininess;}
  void setAtomShininess(double value) override {_atomShininess = value;}

  double atomHue() const override {return _atomHue;}
  void setAtomHue(double value) override {_atomHue = value;}
  double atomSaturation() const override {return _atomSaturation;}
  void setAtomSaturation(double value) override {_atomSaturation = value;}
  double atomValue() const override {return _atomValue;}
  void setAtomValue(double value) override {_atomValue = value;}

  double atomScaleFactor() const override {return _atomScaleFactor;}
  void setAtomScaleFactor(double size) override;

  bool atomAmbientOcclusion() const override {return _atomAmbientOcclusion;}
  void setAtomAmbientOcclusion(bool value) override {_atomAmbientOcclusion = value;}
  int atomAmbientOcclusionPatchNumber() const override {return _atomAmbientOcclusionPatchNumber;}
  void setAtomAmbientOcclusionPatchNumber(int patchNumber) override {_atomAmbientOcclusionPatchNumber=patchNumber;}
  int atomAmbientOcclusionPatchSize() const override {return _atomAmbientOcclusionPatchSize;}
  void setAtomAmbientOcclusionPatchSize(int patchSize) override {_atomAmbientOcclusionPatchSize=patchSize;}
  int atomAmbientOcclusionTextureSize() const override {return _atomAmbientOcclusionTextureSize;}
  void setAtomAmbientOcclusionTextureSize(int size) override {_atomAmbientOcclusionTextureSize=size;}

  bool atomHDR() const  override{return _atomHDR;}
  void setAtomHDR(bool value) override {_atomHDR = value;}
  double atomHDRExposure() const override {return _atomHDRExposure;}
  void setAtomHDRExposure(double value) override {_atomHDRExposure = value;}

  RKSelectionStyle atomSelectionStyle() const override {return _atomSelectionStyle;}
  void setAtomSelectionStyle(RKSelectionStyle style) override {_atomSelectionStyle = style;}
  double atomSelectionScaling() const override {return _atomSelectionScaling;}
  void setAtomSelectionScaling(double scaling) override {_atomSelectionScaling = scaling;}
  double atomSelectionIntensity() const override {return _atomSelectionIntensity;}
  void setSelectionIntensity(double scaling) override {_atomSelectionIntensity = scaling;}

  double atomSelectionStripesDensity() const override {return _atomSelectionStripesDensity;}
  void setAtomSelectionStripesDensity(double value) override {_atomSelectionStripesDensity = value;}
  double atomSelectionStripesFrequency() const override {return _atomSelectionStripesFrequency;}
  void setAtomSelectionStripesFrequency(double value) override {_atomSelectionStripesFrequency = value;}
  double atomSelectionWorleyNoise3DFrequency() const override {return _atomSelectionWorleyNoise3DFrequency;}
  void setAtomSelectionWorleyNoise3DFrequency(double value) override {_atomSelectionWorleyNoise3DFrequency = value;}
  double atomSelectionWorleyNoise3DJitter() const override {return _atomSelectionWorleyNoise3DJitter;}
  void setAtomSelectionWorleyNoise3DJitter(double value) override {_atomSelectionWorleyNoise3DJitter = value;}

  double atomSelectionFrequency() const override;
  void setAtomSelectionFrequency(double value) override;
  double atomSelectionDensity() const override;
  void setAtomSelectionDensity(double value) override;

  // bonds
  bool hasExternalBonds() const override {return true;}
  bool drawBonds() const override {return _drawBonds;}
  void setDrawBonds(bool drawBonds) override {_drawBonds = drawBonds;}

  bool bondAmbientOcclusion() const override {return _bondAmbientOcclusion;}
  void setBondAmbientOcclusion(bool state) override {_bondAmbientOcclusion = state;}
  QColor bondAmbientColor() const override {return _bondAmbientColor;}
  void setBondAmbientColor(QColor color) override {_bondAmbientColor = color;}
  QColor bondDiffuseColor() const override {return _bondDiffuseColor;}
  void setBondDiffuseColor(QColor color) override {_bondDiffuseColor = color;}
  QColor bondSpecularColor() const override {return _bondSpecularColor;}
  void setBondSpecularColor(QColor color) override {_bondSpecularColor = color;}
  double bondAmbientIntensity() const override {return _bondAmbientIntensity;}
  void setBondAmbientIntensity(double value) override {_bondAmbientIntensity = value;}
  double bondDiffuseIntensity() const override {return _bondDiffuseIntensity;}
  void setBondDiffuseIntensity(double value) override {_bondDiffuseIntensity = value;}
  double bondSpecularIntensity() const override {return _bondSpecularIntensity;}
  void setBondSpecularIntensity(double value) override {_bondSpecularIntensity = value;}
  double bondShininess() const override {return _bondShininess;}
  void setBondShininess(double value) override {_bondShininess = value;}

  double bondScaleFactor() const override {return _bondScaleFactor;}
  void setBondScaleFactor(double value) override;
  RKBondColorMode bondColorMode() const override {return _bondColorMode;}
  void setBondColorMode(RKBondColorMode value) override {_bondColorMode = value;}

  bool bondHDR() const override {return _bondHDR;}
  void setBondHDR(bool value) override {_bondHDR = value;}
  double bondHDRExposure() const override {return _bondHDRExposure;}
  void setBondHDRExposure(double value) override {_bondHDRExposure = value;}

  RKSelectionStyle bondSelectionStyle() const override {return _bondSelectionStyle;}
  void setBondSelectionStyle(RKSelectionStyle style) override {_bondSelectionStyle = style;}
  double bondSelectionScaling() const override {return _bondSelectionScaling;}
  void setBondSelectionScaling(double scaling) override {_bondSelectionScaling = scaling;}
  double bondSelectionIntensity() const override {return _bondSelectionIntensity;}
  void setBondSelectionIntensity(double value) override {_bondSelectionIntensity = value;}

  double bondSelectionStripesDensity() const override {return _bondSelectionStripesDensity;}
  double bondSelectionStripesFrequency() const override  {return _bondSelectionStripesFrequency;}
  double bondSelectionWorleyNoise3DFrequency() const override {return _bondSelectionWorleyNoise3DFrequency;}
  double bondSelectionWorleyNoise3DJitter() const override  {return _bondSelectionWorleyNoise3DJitter;}

  double bondSelectionFrequency() const override;
  void setBondSelectionFrequency(double value) override;
  double bondSelectionDensity() const override;
  void setBondSelectionDensity(double value) override;

  double bondHue() const override {return _bondHue;}
  void setBondHue(double value) override {_bondHue = value;}
  double bondSaturation() const override {return _bondSaturation;}
  void setBondSaturation(double value) override {_bondSaturation = value;}
  double bondValue() const override {return _bondValue;}
  void setBondValue(double value) override {_bondValue = value;}



  // adsorption surface
  RKEnergySurfaceType adsorptionSurfaceRenderingMethod() override final {return _adsorptionSurfaceRenderingMethod;}
  void setAdsorptionSurfaceRenderingMethod(RKEnergySurfaceType type) override final {_adsorptionSurfaceRenderingMethod = type;}
  RKPredefinedVolumeRenderingTransferFunction adsorptionVolumeTransferFunction() override final {return _adsorptionVolumeTransferFunction;}
  void setAdsorptionVolumeTransferFunction(RKPredefinedVolumeRenderingTransferFunction function) override final {_adsorptionVolumeTransferFunction = function;}
  double adsorptionVolumeStepLength() override final {return _adsorptionVolumeStepLength;}
  void setAdsorptionVolumeStepLength(double stepLength) override final {_adsorptionVolumeStepLength = stepLength;}
  std::vector<double3> atomUnitCellPositions() const override {return std::vector<double3>();}
  bool drawAdsorptionSurface() const override {return _drawAdsorptionSurface;}
  void setDrawAdsorptionSurface(bool state) override {_drawAdsorptionSurface = state;}
  double adsorptionSurfaceOpacity() const override {return _adsorptionSurfaceOpacity;}
  void setAdsorptionSurfaceOpacity(double value) override {_adsorptionSurfaceOpacity = value;}
  double adsorptionSurfaceIsoValue() const override {return _adsorptionSurfaceIsoValue;}
  void setAdsorptionSurfaceIsoValue(double value)  override {_adsorptionSurfaceIsoValue = value;}
  double adsorptionSurfaceMinimumValue() const override {return _adsorptionSurfaceMinimumValue;}
  void setAdsorptionSurfaceMinimumValue(double value) override {_adsorptionSurfaceMinimumValue = value;}

  int adsorptionSurfaceSize() const override {return _adsorptionSurfaceSize;}
  double2 adsorptionSurfaceProbeParameters() const override final;

  ProbeMolecule adsorptionSurfaceProbeMolecule() const override {return _adsorptionSurfaceProbeMolecule;}
  void setAdsorptionSurfaceProbeMolecule(ProbeMolecule value) override {_adsorptionSurfaceProbeMolecule = value;}

  bool adsorptionSurfaceFrontSideHDR() const override {return _adsorptionSurfaceFrontSideHDR;}
  void setAdsorptionSurfaceFrontSideHDR(bool state) override {_adsorptionSurfaceFrontSideHDR = state;}
  double adsorptionSurfaceFrontSideHDRExposure() const override {return _adsorptionSurfaceFrontSideHDRExposure;}
  void setAdsorptionSurfaceFrontSideHDRExposure(double value) override {_adsorptionSurfaceFrontSideHDRExposure = value;}
  QColor adsorptionSurfaceFrontSideAmbientColor() const override {return _adsorptionSurfaceFrontSideAmbientColor;}
  void setAdsorptionSurfaceFrontSideAmbientColor(QColor color) override {_adsorptionSurfaceFrontSideAmbientColor = color;}
  QColor adsorptionSurfaceFrontSideDiffuseColor() const override {return _adsorptionSurfaceFrontSideDiffuseColor;}
  void setAdsorptionSurfaceFrontSideDiffuseColor(QColor color) override {_adsorptionSurfaceFrontSideDiffuseColor = color;}
  QColor adsorptionSurfaceFrontSideSpecularColor() const override {return _adsorptionSurfaceFrontSideSpecularColor;}
  void setAdsorptionSurfaceFrontSideSpecularColor(QColor color) override {_adsorptionSurfaceFrontSideSpecularColor = color;}
  double adsorptionSurfaceFrontSideDiffuseIntensity() const override {return _adsorptionSurfaceFrontSideDiffuseIntensity;}
  void setAdsorptionSurfaceFrontSideDiffuseIntensity(double value) override {_adsorptionSurfaceFrontSideDiffuseIntensity = value;}
  double adsorptionSurfaceFrontSideAmbientIntensity() const override {return _adsorptionSurfaceFrontSideAmbientIntensity;}
  void setAdsorptionSurfaceFrontSideAmbientIntensity(double value) override {_adsorptionSurfaceFrontSideAmbientIntensity = value;}
  double adsorptionSurfaceFrontSideSpecularIntensity() const override {return _adsorptionSurfaceFrontSideSpecularIntensity;}
  void setAdsorptionSurfaceFrontSideSpecularIntensity(double value) override {_adsorptionSurfaceFrontSideSpecularIntensity = value;}
  double adsorptionSurfaceFrontSideShininess() const override {return _adsorptionSurfaceFrontSideShininess;}
  void setAdsorptionSurfaceFrontSideShininess(double value) override {_adsorptionSurfaceFrontSideShininess = value;}

  double adsorptionSurfaceHue() const override {return _adsorptionSurfaceHue;}
  void setAdsorptionSurfaceHue(double value) override {_adsorptionSurfaceHue = value;}
  double adsorptionSurfaceSaturation() const override {return _adsorptionSurfaceSaturation;}
  void setAdsorptionSurfaceSaturation(double value) override {_adsorptionSurfaceSaturation = value;}
  double adsorptionSurfaceValue() const override {return _adsorptionSurfaceValue;}
  void setAdsorptionSurfaceValue(double value) override {_adsorptionSurfaceValue = value;}

  bool adsorptionSurfaceBackSideHDR() const override {return _adsorptionSurfaceBackSideHDR;}
  void setAdsorptionSurfaceBackSideHDR(bool state) override {_adsorptionSurfaceBackSideHDR = state;}
  double adsorptionSurfaceBackSideHDRExposure() const override {return _adsorptionSurfaceBackSideHDRExposure;}
  void setAdsorptionSurfaceBackSideHDRExposure(double value) override {_adsorptionSurfaceBackSideHDRExposure = value;}
  QColor adsorptionSurfaceBackSideAmbientColor() const override {return _adsorptionSurfaceBackSideAmbientColor;}
  void setAdsorptionSurfaceBackSideAmbientColor(QColor color) override {_adsorptionSurfaceBackSideAmbientColor = color;}
  QColor adsorptionSurfaceBackSideDiffuseColor() const override {return _adsorptionSurfaceBackSideDiffuseColor;}
  void setAdsorptionSurfaceBackSideDiffuseColor(QColor color) override {_adsorptionSurfaceBackSideDiffuseColor = color;}
  QColor adsorptionSurfaceBackSideSpecularColor() const override {return _adsorptionSurfaceBackSideSpecularColor;}
  void setAdsorptionSurfaceBackSideSpecularColor(QColor color) override {_adsorptionSurfaceBackSideSpecularColor = color;}
  double adsorptionSurfaceBackSideDiffuseIntensity() const override {return _adsorptionSurfaceBackSideDiffuseIntensity;}
  void setAdsorptionSurfaceBackSideDiffuseIntensity(double value) override {_adsorptionSurfaceBackSideDiffuseIntensity = value;}
  double adsorptionSurfaceBackSideAmbientIntensity() const override {return _adsorptionSurfaceBackSideAmbientIntensity;}
  void setAdsorptionSurfaceBackSideAmbientIntensity(double value) override {_adsorptionSurfaceBackSideAmbientIntensity = value;}
  double adsorptionSurfaceBackSideSpecularIntensity() const override {return _adsorptionSurfaceBackSideSpecularIntensity;}
  void setAdsorptionSurfaceBackSideSpecularIntensity(double value) override {_adsorptionSurfaceBackSideSpecularIntensity = value;}
  double adsorptionSurfaceBackSideShininess() const override {return _adsorptionSurfaceBackSideShininess;}
  void setAdsorptionSurfaceBackSideShininess(double value) override{_adsorptionSurfaceBackSideShininess = value;}

 // text
  std::vector<RKInPerInstanceAttributesText> renderTextData() const override {return std::vector<RKInPerInstanceAttributesText>();}
  RKTextType renderTextType() const override {return _atomTextType;}
  void setRenderTextType(RKTextType type) override {_atomTextType = type;}
  void setRenderTextFont(QString value) override {_atomTextFont = value;}
  QString renderTextFont() const override {return _atomTextFont;}
  RKTextAlignment renderTextAlignment() const override {return _atomTextAlignment;}
  void setRenderTextAlignment(RKTextAlignment alignment) override {_atomTextAlignment = alignment;}
  RKTextStyle renderTextStyle() const override {return _atomTextStyle;}
  void setRenderTextStyle(RKTextStyle style) override {_atomTextStyle = style;}
  QColor renderTextColor() const override {return _atomTextColor;}
  void setRenderTextColor(QColor color) override {_atomTextColor = color;}
  double renderTextScaling() const override {return _atomTextScaling;}
  void setRenderTextScaling(double scaling) override {_atomTextScaling = scaling;}
  double3 renderTextOffset() const override {return _atomTextOffset;}
  void setRenderTextOffsetX(double value) override {_atomTextOffset.x = value;}
  void setRenderTextOffsetY(double value) override {_atomTextOffset.y = value;}
  void setRenderTextOffsetZ(double value) override {_atomTextOffset.z = value;}

  // info
  QString authorFirstName() override final {return _authorFirstName;}
  void setAuthorFirstName(QString name) override final {_authorFirstName = name;}
  QString authorMiddleName() override final {return _authorMiddleName;}
  void setAuthorMiddleName(QString name) override final {_authorMiddleName = name;}
  QString authorLastName() override final {return _authorLastName;}
  void setAuthorLastName(QString name) override final  {_authorLastName = name;}
  QString authorOrchidID() override final {return _authorOrchidID;}
  void setAuthorOrchidID(QString name) override final {_authorOrchidID = name;}
  QString authorResearcherID() override final {return _authorResearcherID;}
  void setAuthorResearcherID(QString name) override final {_authorResearcherID = name;}
  QString authorAffiliationUniversityName() override final {return _authorAffiliationUniversityName;}
  void setAuthorAffiliationUniversityName(QString name) override final {_authorAffiliationUniversityName = name;}
  QString authorAffiliationFacultyName() override final {return _authorAffiliationFacultyName;}
  void setAuthorAffiliationFacultyName(QString name) override final {_authorAffiliationFacultyName = name;}
  QString authorAffiliationInstituteName() override final {return _authorAffiliationInstituteName;}
  void setAuthorAffiliationInstituteName(QString name) override final {_authorAffiliationInstituteName = name;}
  QString authorAffiliationCityName() override final {return _authorAffiliationCityName;}
  void setAuthorAffiliationCityName(QString name) override final {_authorAffiliationCityName = name;}
  QString authorAffiliationCountryName() override final {return _authorAffiliationCountryName;}
  void setAuthorAffiliationCountryName(QString name) override final {_authorAffiliationCountryName = name;}

  QDate creationDate() override final {return _creationDate;}
  void setCreationDate(QDate date) override final {_creationDate = date;}
  QString creationTemperature() override final {return _creationTemperature;}
  void setCreationTemperature(QString name) override final {_creationTemperature = name;}
  TemperatureScale creationTemperatureScale() override final {return _creationTemperatureScale;}
  void setCreationTemperatureScale(TemperatureScale scale) override final {_creationTemperatureScale = scale;}
  QString creationPressure() override final {return _creationPressure;}
  void setCreationPressure(QString pressure) override final {_creationPressure = pressure;}
  PressureScale creationPressureScale() override final {return _creationPressureScale;}
  void setCreationPressureScale(PressureScale scale) override final {_creationPressureScale = scale;}
  CreationMethod creationMethod() override final {return _creationMethod;}
  void setCreationMethod(CreationMethod method) override final {_creationMethod = method;}
  UnitCellRelaxationMethod creationUnitCellRelaxationMethod() override final {return _creationUnitCellRelaxationMethod;}
  void setCreationUnitCellRelaxationMethod(UnitCellRelaxationMethod method) override final {_creationUnitCellRelaxationMethod = method;}
  QString creationAtomicPositionsSoftwarePackage() override final {return _creationAtomicPositionsSoftwarePackage;}
  void setCreationAtomicPositionsSoftwarePackage(QString name) override final {_creationAtomicPositionsSoftwarePackage = name;}
  IonsRelaxationAlgorithm creationAtomicPositionsIonsRelaxationAlgorithm() override final {return _creationAtomicPositionsIonsRelaxationAlgorithm;}
  void setCreationAtomicPositionsIonsRelaxationAlgorithm(IonsRelaxationAlgorithm algorithm) override final {_creationAtomicPositionsIonsRelaxationAlgorithm = algorithm;}
  IonsRelaxationCheck creationAtomicPositionsIonsRelaxationCheck() override final {return _creationAtomicPositionsIonsRelaxationCheck;}
  void setCreationAtomicPositionsIonsRelaxationCheck(IonsRelaxationCheck check) override final {_creationAtomicPositionsIonsRelaxationCheck = check;}
  QString creationAtomicPositionsForcefield() override final {return _creationAtomicPositionsForcefield;}
  void setCreationAtomicPositionsForcefield(QString name) override final {_creationAtomicPositionsForcefield = name;}
  QString creationAtomicPositionsForcefieldDetails() override final {return _creationAtomicPositionsForcefieldDetails;}
  void setCreationAtomicPositionsForcefieldDetails(QString name) override final {_creationAtomicPositionsForcefieldDetails = name;}
  QString creationAtomicChargesSoftwarePackage() override final {return _creationAtomicChargesSoftwarePackage;}
  void setCreationAtomicChargesSoftwarePackage(QString name) override final {_creationAtomicChargesSoftwarePackage = name;}
  QString creationAtomicChargesAlgorithms() override final {return _creationAtomicChargesAlgorithms;}
  void setCreationAtomicChargesAlgorithms(QString name) override final {_creationAtomicChargesAlgorithms = name;}
  QString creationAtomicChargesForcefield() override final {return _creationAtomicChargesForcefield;}
  void setCreationAtomicChargesForcefield(QString name) override final {_creationAtomicChargesForcefield = name;}
  QString creationAtomicChargesForcefieldDetails() override final {return _creationAtomicChargesForcefieldDetails;}
  void setCreationAtomicChargesForcefieldDetails(QString name) override final {_creationAtomicChargesForcefieldDetails = name;}

  QString experimentalMeasurementRadiation() override final {return _experimentalMeasurementRadiation;}
  void setExperimentalMeasurementRadiation(QString name) override final {_experimentalMeasurementRadiation = name;}
  QString experimentalMeasurementWaveLength() override final {return _experimentalMeasurementWaveLength;}
  void setExperimentalMeasurementWaveLength(QString name) override final {_experimentalMeasurementWaveLength = name;}
  QString experimentalMeasurementThetaMin() override final {return _experimentalMeasurementThetaMin;}
  void setExperimentalMeasurementThetaMin(QString name) override final {_experimentalMeasurementThetaMin = name;}
  QString experimentalMeasurementThetaMax() override final {return _experimentalMeasurementThetaMax;}
  void setExperimentalMeasurementThetaMax(QString name) override final {_experimentalMeasurementThetaMax = name;}
  QString experimentalMeasurementIndexLimitsHmin() override final {return _experimentalMeasurementIndexLimitsHmin;}
  void setExperimentalMeasurementIndexLimitsHmin(QString name) override final {_experimentalMeasurementIndexLimitsHmin = name;}
  QString experimentalMeasurementIndexLimitsHmax() override final {return _experimentalMeasurementIndexLimitsHmax;}
  void setExperimentalMeasurementIndexLimitsHmax(QString name) override final {_experimentalMeasurementIndexLimitsHmax = name;}
  QString experimentalMeasurementIndexLimitsKmin() override final {return _experimentalMeasurementIndexLimitsKmin;}
  void setExperimentalMeasurementIndexLimitsKmin(QString name) override final {_experimentalMeasurementIndexLimitsKmin = name;}
  QString experimentalMeasurementIndexLimitsKmax() override final {return _experimentalMeasurementIndexLimitsKmax;}
  void setExperimentalMeasurementIndexLimitsKmax(QString name) override final {_experimentalMeasurementIndexLimitsKmax = name;}
  QString experimentalMeasurementIndexLimitsLmin() override final {return _experimentalMeasurementIndexLimitsLmin;}
  void setExperimentalMeasurementIndexLimitsLmin(QString name) override final {_experimentalMeasurementIndexLimitsLmin = name;}
  QString experimentalMeasurementIndexLimitsLmax() override final {return _experimentalMeasurementIndexLimitsLmax;}
  void setExperimentalMeasurementIndexLimitsLmax(QString name) override final {_experimentalMeasurementIndexLimitsLmax = name;}
  QString experimentalMeasurementNumberOfSymmetryIndependentReflections() override final {return _experimentalMeasurementNumberOfSymmetryIndependentReflections;}
  void setExperimentalMeasurementNumberOfSymmetryIndependentReflections(QString name) override final {_experimentalMeasurementNumberOfSymmetryIndependentReflections = name;}
  QString experimentalMeasurementSoftware() override final {return _experimentalMeasurementSoftware;}
  void setExperimentalMeasurementSoftware(QString name) override final {_experimentalMeasurementSoftware = name;}
  QString experimentalMeasurementRefinementDetails() override final {return _experimentalMeasurementRefinementDetails;}
  void setExperimentalMeasurementGoodnessOfFit(QString goodness) override final {_experimentalMeasurementGoodnessOfFit = goodness;}
  QString experimentalMeasurementGoodnessOfFit() override final {return _experimentalMeasurementGoodnessOfFit;}
  void setExperimentalMeasurementRefinementDetails(QString name) override final {_experimentalMeasurementGoodnessOfFit = name;}
  QString experimentalMeasurementRFactorGt() override final {return _experimentalMeasurementRFactorGt;}
  void setExperimentalMeasurementRFactorGt(QString name) override final {_experimentalMeasurementRFactorGt = name;}
  QString experimentalMeasurementRFactorAll() override final {return _experimentalMeasurementRFactorAll;}
  void setExperimentalMeasurementRFactorAll(QString name) override final {_experimentalMeasurementRFactorAll = name;}

  QString chemicalFormulaMoiety() override final {return _chemicalFormulaMoiety;}
  void setChemicalFormulaMoiety(QString name) override final {_chemicalFormulaMoiety = name;}
  QString chemicalFormulaSum() override final {return _chemicalFormulaSum;}
  void setChemicalFormulaSum(QString name) override final {_chemicalFormulaSum = name;}
  QString chemicalNameSystematic() override final {return _chemicalNameSystematic;}
  void setChemicalNameSystematic(QString name) override final {_chemicalNameSystematic = name;}

  QString citationArticleTitle() override final {return _citationArticleTitle;}
  void setCitationArticleTitle(QString name) override final {_citationArticleTitle = name;}
  QString citationJournalTitle() override final {return _citationJournalTitle;}
  void setCitationJournalTitle(QString name) override final {_citationJournalTitle = name;}
  QString citationAuthors() override final {return _citationAuthors;}
  void setCitationAuthors(QString name) override final {_citationAuthors = name;}
  QString citationJournalVolume() override final {return _citationJournalVolume;}
  void setCitationJournalVolume(QString name) override final {_citationJournalVolume = name;}
  QString citationJournalNumber() override final {return _citationJournalNumber;}
  void setCitationJournalNumber(QString name) override final {_citationJournalNumber = name;}
  QString citationJournalPageNumbers() override final {return _citationJournalPageNumbers;}
  void setCitationJournalPageNumbers(QString name) override final {_citationJournalPageNumbers = name;}
  QString citationDOI() override final {return _citationDOI;}
  void setCitationDOI(QString name) override final {_citationDOI = name;}
  QDate citationPublicationDate() override final {return _citationPublicationDate;}
  void setCitationPublicationDate(QDate date) override final {_citationPublicationDate = date;}
  QString citationDatebaseCodes() override final {return _citationDatebaseCodes;}
  void setCitationDatebaseCodes(QString name) override final {_citationDatebaseCodes = name;}

  ProbeMolecule frameworkProbeMolecule() const override final  {return _frameworkProbeMolecule;}
  void setFrameworkProbeMolecule(ProbeMolecule value) override final {_frameworkProbeMolecule = value;}

  void recomputeDensityProperties() override;
  QString structureMaterialType() override {return _structureMaterialType;}
  double structureMass() override final {return _structureMass;}
  double structureDensity() override final {return _structureDensity;}
  double structureHeliumVoidFraction() override final {return _structureHeliumVoidFraction;}
  void setStructureHeliumVoidFraction(double value) override final {_structureHeliumVoidFraction = value;}
  void setStructureNitrogenSurfaceArea(double value) override final;
  double2 frameworkProbeParameters() const override final;
  double structureSpecificVolume() override final {return _structureSpecificVolume;}
  double structureAccessiblePoreVolume () override final {return _structureAccessiblePoreVolume;}
  double structureVolumetricNitrogenSurfaceArea() override final {return _structureVolumetricNitrogenSurfaceArea;}
  double structureGravimetricNitrogenSurfaceArea() override final {return _structureGravimetricNitrogenSurfaceArea;}
  int structureNumberOfChannelSystems() override final {return _structureNumberOfChannelSystems;}
  void setStructureNumberOfChannelSystems(int value) override final {_structureNumberOfChannelSystems = value;}
  int structureNumberOfInaccessiblePockets()  override final {return _structureNumberOfInaccessiblePockets;}
  void setStructureNumberOfInaccessiblePockets(int value)  override final {_structureNumberOfInaccessiblePockets = value;}
  int structureDimensionalityOfPoreSystem() override final {return _structureDimensionalityOfPoreSystem;}
  void setStructureDimensionalityOfPoreSystem(int value) override final {_structureDimensionalityOfPoreSystem = value;}
  double structureLargestCavityDiameter() override final {return _structureLargestCavityDiameter;}
  void setStructureLargestCavityDiameter(double value) override final {_structureLargestCavityDiameter = value;}
  double structureRestrictingPoreLimitingDiameter() override final {return _structureRestrictingPoreLimitingDiameter;}
  void setStructureRestrictingPoreLimitingDiameter(double value) override final {_structureRestrictingPoreLimitingDiameter = value;}
  double structureLargestCavityDiameterAlongAviablePath() override final {return _structureLargestCavityDiameterAlongAViablePath;}
  void setStructureLargestCavityDiameterAlongAviablePath(double value) override final {_structureLargestCavityDiameterAlongAViablePath = value;}

  // spacegroup
  void setSpaceGroupHallNumber(int HallNumber)  {_spaceGroup = SKSpaceGroup(HallNumber);}
  SKSpaceGroup& spaceGroup() {return _spaceGroup;}

  void computeBonds() override {;}

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<Structure> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<Structure> &);
protected:
  qint64 _versionNumber{9};

  std::shared_ptr<SKAtomTreeController> _atomsTreeController;
  std::shared_ptr<SKBondSetController> _bondSetController;

  SKSpaceGroup _spaceGroup;
  //std::shared_ptr<SKCell> _cell;

  //double3 _origin = double3(0.0, 0.0, 0.0);
  //double3 _scaling = double3(1.0, 1.0, 1.0);
  //simd_quatd _orientation = simd_quatd(1.0, double3(0.0, 0.0, 0.0));
  //double _rotationDelta = 5.0;

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


  bool _drawAdsorptionSurface = false;
  double _adsorptionSurfaceOpacity = 1.0;
  double _adsorptionSurfaceIsoValue = 0.0;
  double _adsorptionSurfaceMinimumValue = -1000.0;

  RKEnergySurfaceType _adsorptionSurfaceRenderingMethod = RKEnergySurfaceType::isoSurface;
  RKPredefinedVolumeRenderingTransferFunction  _adsorptionVolumeTransferFunction = RKPredefinedVolumeRenderingTransferFunction::standard;
  double _adsorptionVolumeStepLength = 0.0005;

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


};

struct FrameConsumer
{
  virtual void setFrame(std::shared_ptr<Structure> structure) = 0;
  virtual ~FrameConsumer() = 0;
};
