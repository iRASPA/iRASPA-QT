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
#include "unitcellviewer.h"
#include "atomviewer.h"
#include "bondviewer.h"
#include "primitivestructureviewer.h"
#include "atomstructureviewer.h"
#include "bondstructureviewer.h"
#include "volumetricdataviewer.h"
#include "annotationviewer.h"
#include "primitive.h"
#include "gridvolume.h"

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


class Structure: public Object, public InfoEditor,  public AtomViewer, public BondViewer,
                 public AtomStructureEditor, public BondStructureEditor,
                 public AnnotationEditor,
                 public RKRenderAtomSource, public RKRenderBondSource
{
public:
  Structure();
  Structure(std::shared_ptr<SKAtomTreeController> atomTreeController);
  Structure(std::shared_ptr<SKStructure> structure);
  Structure(const Structure &structure);

  virtual ~Structure() {}

  // Object
  // ===============================================================================================
  Structure(const std::shared_ptr<Object> object);
  ObjectType structureType() override {return ObjectType::structure;}
  std::shared_ptr<Object> shallowClone() override;
  SKBoundingBox boundingBox() const override;
  void reComputeBoundingBox() override;
  SKBoundingBox transformedBoundingBox() const;

  enum class StructureType: qint64
  {
    framework = 0, adsorbate = 1, cation = 2, ionicLiquid = 3, solvent = 4
  };

  // Protocol:  AtomViewer
  // ===============================================================================================
  std::shared_ptr<SKAtomTreeController> &atomsTreeController() override final {return _atomsTreeController;}
  const std::shared_ptr<SKAtomTreeController> &atomsTreeController() const {return _atomsTreeController;}
  void expandSymmetry() override;

  void clearSelection() override final;
  void setAtomSelection(int asymmetricAtomId) override final;
  void addAtomToSelection(int asymmetricAtomId) override final;
  void toggleAtomSelection(int asymmetricAtomId) override final;
  void setAtomSelection(std::set<int>& atomIds) override final;
  void addToAtomSelection(std::set<int>& atomIds) override final;

  std::set<int> filterCartesianAtomPositions(std::function<bool(double3)> &) override;

  void recomputeSelectionBodyFixedBasis() override;

  void convertAsymmetricAtomsToCartesian() override;
  void convertAsymmetricAtomsToFractional() override;

  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override;

  // Protocol:  AtomEditor
  // ===============================================================================================
  void setAtomTreeController(std::shared_ptr<SKAtomTreeController> controller) {_atomsTreeController = controller;}

  // Protocol:  RKRenderAtomSource
  // ===============================================================================================
  int numberOfAtoms() const override {return 0;}
  bool drawAtoms() const override {return _drawAtoms;}

  QColor atomAmbientColor() const override {return _atomAmbientColor;}
  QColor atomDiffuseColor() const override {return _atomDiffuseColor;}
  QColor atomSpecularColor() const override {return _atomSpecularColor;}
  double atomAmbientIntensity() const override {return _atomAmbientIntensity;}
  double atomDiffuseIntensity() const override {return _atomDiffuseIntensity;}
  double atomSpecularIntensity() const override {return _atomSpecularIntensity;}
  double atomShininess() const override {return _atomShininess;}

  double atomHue() const override {return _atomHue;}
  double atomSaturation() const override {return _atomSaturation;}
  double atomValue() const override {return _atomValue;}

  bool colorAtomsWithBondColor() const override;

  double atomScaleFactor() const override {return _atomScaleFactor;}
  bool atomAmbientOcclusion() const override {return _atomAmbientOcclusion;}
  int atomAmbientOcclusionPatchNumber() const override {return _atomAmbientOcclusionPatchNumber;}
  int atomAmbientOcclusionPatchSize() const override {return _atomAmbientOcclusionPatchSize;}
  int atomAmbientOcclusionTextureSize() const override {return _atomAmbientOcclusionTextureSize;}
  void setAtomAmbientOcclusionPatchNumber(int patchNumber) override  {_atomAmbientOcclusionPatchNumber=patchNumber;}
  void setAtomAmbientOcclusionPatchSize(int patchSize)  override {_atomAmbientOcclusionPatchSize=patchSize;}
  void setAtomAmbientOcclusionTextureSize(int size)  override {_atomAmbientOcclusionTextureSize=size;}

  bool atomHDR() const  override{return _atomHDR;}
  double atomHDRExposure() const override {return _atomHDRExposure;}
  bool clipAtomsAtUnitCell() const override {return false;}
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderAtoms() const override;

  virtual std::vector<RKInPerInstanceAttributesText> atomTextData(RKFontAtlas *fontAtlas) const override;
  std::vector<RKInPerInstanceAttributesText> renderTextData() const override {return std::vector<RKInPerInstanceAttributesText>();}
  RKTextType renderTextType() const override {return _atomTextType;}
  QString renderTextFont() const override {return _atomTextFont;}
  RKTextAlignment renderTextAlignment() const override {return _atomTextAlignment;}
  RKTextStyle renderTextStyle() const override {return _atomTextStyle;}
  QColor renderTextColor() const override {return _atomTextColor;}
  double renderTextScaling() const override {return _atomTextScaling;}
  double3 renderTextOffset() const override {return _atomTextOffset;}

  virtual std::vector<RKInPerInstanceAttributesAtoms> renderSelectedAtoms() const override;
  RKSelectionStyle atomSelectionStyle() const override {return _atomSelectionStyle;}
  double atomSelectionStripesDensity() const override {return _atomSelectionStripesDensity;}
  double atomSelectionStripesFrequency() const override {return _atomSelectionStripesFrequency;}
  double atomSelectionWorleyNoise3DFrequency() const override {return _atomSelectionWorleyNoise3DFrequency;}
  double atomSelectionWorleyNoise3DJitter() const override {return _atomSelectionWorleyNoise3DJitter;}
  double atomSelectionScaling() const override {return _atomSelectionScaling;}
  double atomSelectionIntensity() const override {return _atomSelectionIntensity;}

  // Protocol:  AtomStructureViewer (most are already in RKRenderAtomSource)
  // ===============================================================================================

  RepresentationType atomRepresentationType() override {return _atomRepresentationType;}
  RepresentationStyle atomRepresentationStyle() override {return _atomRepresentationStyle;}
  QString atomColorSchemeIdentifier() override {return _atomColorSchemeIdentifier;}
  SKColorSet::ColorSchemeOrder colorSchemeOrder() override {return _atomColorSchemeOrder;}
  QString atomForceFieldIdentifier() override {return _atomForceFieldIdentifier;}
  ForceFieldSet::ForceFieldSchemeOrder forceFieldSchemeOrder() override {return _atomForceFieldOrder;}

  double atomSelectionFrequency() const override;
  double atomSelectionDensity() const override;

  // Protocol:  AtomStructureEditor
  // ===============================================================================================

  void recheckRepresentationStyle() override;
  void setDrawAtoms(bool drawAtoms) override {_drawAtoms = drawAtoms;}

  void setRepresentationType(RepresentationType) override final;
  void setRepresentationStyle(RepresentationStyle style) override;
  void setRepresentationStyle(RepresentationStyle style, const SKColorSets &colorSets) override;
  void setRepresentationColorSchemeIdentifier(const QString colorSchemeName, const SKColorSets &colorSets) override;
  void setColorSchemeOrder(SKColorSet::ColorSchemeOrder order) override {_atomColorSchemeOrder = order;}
  void setAtomForceFieldIdentifier(QString identifier, ForceFieldSets &forceFieldSets) override;
  void updateForceField(ForceFieldSets &forceFieldSets) override;
  void setForceFieldSchemeOrder(ForceFieldSet::ForceFieldSchemeOrder order) override {_atomForceFieldOrder = order;}

  void setAtomHue(double value) override {_atomHue = value;}
  void setAtomSaturation(double value) override {_atomSaturation = value;}
  void setAtomValue(double value) override {_atomValue = value;}
  void setAtomScaleFactor(double size) override;

  void setAtomAmbientOcclusion(bool value) override {_atomAmbientOcclusion = value;}
  void setAtomHDR(bool value) override {_atomHDR = value;}
  void setAtomHDRExposure(double value) override {_atomHDRExposure = value;}

  void setAtomAmbientColor(QColor color) override {_atomAmbientColor = color;}
  void setAtomDiffuseColor(QColor color) override {_atomDiffuseColor = color;}
  void setAtomSpecularColor(QColor color) override {_atomSpecularColor = color;}
  void setAtomAmbientIntensity(double value) override {_atomAmbientIntensity = value;}
  void setAtomDiffuseIntensity(double value) override {_atomDiffuseIntensity = value;}
  void setAtomSpecularIntensity(double value) override {_atomSpecularIntensity = value;}
  void setAtomShininess(double value) override {_atomShininess = value;}

  void setAtomSelectionStyle(RKSelectionStyle style) override {_atomSelectionStyle = style;}
  void setAtomSelectionFrequency(double value) override;
  void setAtomSelectionDensity(double value) override;
  void setAtomSelectionScaling(double scaling) override {_atomSelectionScaling = scaling;}
  void setSelectionIntensity(double scaling) override {_atomSelectionIntensity = scaling;}

  void setAtomSelectionStripesDensity(double value)  {_atomSelectionStripesDensity = value;}
  void setAtomSelectionStripesFrequency(double value)  {_atomSelectionStripesFrequency = value;}
  void setAtomSelectionWorleyNoise3DFrequency(double value)  {_atomSelectionWorleyNoise3DFrequency = value;}
  void setAtomSelectionWorleyNoise3DJitter(double value)  {_atomSelectionWorleyNoise3DJitter = value;}

  // move
  void setRenderTextType(RKTextType type) override {_atomTextType = type;}
  void setRenderTextFont(QString value) override {_atomTextFont = value;}
  void setRenderTextAlignment(RKTextAlignment alignment) override {_atomTextAlignment = alignment;}
  void setRenderTextStyle(RKTextStyle style) override {_atomTextStyle = style;}
  void setRenderTextColor(QColor color) override {_atomTextColor = color;}
  void setRenderTextScaling(double scaling) override {_atomTextScaling = scaling;}
  void setRenderTextOffsetX(double value) override {_atomTextOffset.x = value;}
  void setRenderTextOffsetY(double value) override {_atomTextOffset.y = value;}
  void setRenderTextOffsetZ(double value) override {_atomTextOffset.z = value;}

  // Protocol:  BondViewer
  // ===============================================================================================

  std::shared_ptr<SKBondSetController> bondSetController() override final {return _bondSetController;}
  void computeBonds() override {;}

  void setBondSelection(int asymmetricBondId) override final;
  void addBondToSelection(int asymmetricBondId) override final;
  void toggleBondSelection(int asymmetricAtomId) override final;

   BondSelectionIndexSet filterCartesianBondPositions(std::function<bool(double3)> &) override;

  // Protocol:  BondEditor
  // ===============================================================================================

   // TODO

  // Protocol:  RKRenderBondSource
  // ===============================================================================================

  bool drawBonds() const override {return _drawBonds;}
  int numberOfInternalBonds() const override {return 0;}
  int numberOfExternalBonds() const override {return 0;}
  virtual std::vector<RKInPerInstanceAttributesBonds> renderInternalBonds() const override;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderExternalBonds() const override;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderSelectedInternalBonds() const override;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderSelectedExternalBonds() const override;

  bool bondAmbientOcclusion() const override {return _bondAmbientOcclusion;}
  QColor bondAmbientColor() const override {return _bondAmbientColor;}
  QColor bondDiffuseColor() const override {return _bondDiffuseColor;}
  QColor bondSpecularColor() const override {return _bondSpecularColor;}
  double bondAmbientIntensity() const override {return _bondAmbientIntensity;}
  double bondDiffuseIntensity() const override {return _bondDiffuseIntensity;}
  double bondSpecularIntensity() const override {return _bondSpecularIntensity;}
  double bondShininess() const override {return _bondShininess;}

  bool isUnity() const override final {return _atomRepresentationType == RepresentationType::unity;}
  bool hasExternalBonds() const override {return true;}

  double bondScaleFactor() const override {return _bondScaleFactor;}
  RKBondColorMode bondColorMode() const override {return _bondColorMode;}

  bool bondHDR() const override {return _bondHDR;}
  double bondHDRExposure() const override {return _bondHDRExposure;}

  bool clipBondsAtUnitCell() const override {return false;}

  double bondHue() const override {return _bondHue;}
  double bondSaturation() const override {return _bondSaturation;}
  double bondValue() const override {return _bondValue;}

  RKSelectionStyle bondSelectionStyle() const override {return _bondSelectionStyle;}
  double bondSelectionStripesDensity() const override {return _bondSelectionStripesDensity;}
  double bondSelectionStripesFrequency() const override  {return _bondSelectionStripesFrequency;}
  double bondSelectionWorleyNoise3DFrequency() const override {return _bondSelectionWorleyNoise3DFrequency;}
  double bondSelectionWorleyNoise3DJitter() const override  {return _bondSelectionWorleyNoise3DJitter;}
  double bondSelectionIntensity() const override {return _bondSelectionIntensity;}
  double bondSelectionScaling() const override {return _bondSelectionScaling;}


  // Protocol:  BondStructureViewer (most are already in RKRenderBondSource)
  // ===============================================================================================

  double bondSelectionFrequency() const override;
  double bondSelectionDensity() const override;

  // Protocol:  BondStructureEditor
  // ===============================================================================================

  void setDrawBonds(bool drawBonds) override {_drawBonds = drawBonds;}
  void setBondScaleFactor(double value) override;
  void setBondColorMode(RKBondColorMode value) override {_bondColorMode = value;}

  void setBondAmbientOcclusion(bool state) override {_bondAmbientOcclusion = state;}

  void setBondHDR(bool value) override {_bondHDR = value;}
  void setBondHDRExposure(double value) override {_bondHDRExposure = value;}

  void setBondHue(double value) override {_bondHue = value;}
  void setBondSaturation(double value) override {_bondSaturation = value;}
  void setBondValue(double value) override {_bondValue = value;}

  void setBondAmbientColor(QColor color) override {_bondAmbientColor = color;}
  void setBondDiffuseColor(QColor color) override {_bondDiffuseColor = color;}
  void setBondSpecularColor(QColor color) override {_bondSpecularColor = color;}
  void setBondAmbientIntensity(double value) override {_bondAmbientIntensity = value;}
  void setBondDiffuseIntensity(double value) override {_bondDiffuseIntensity = value;}
  void setBondSpecularIntensity(double value) override {_bondSpecularIntensity = value;}
  void setBondShininess(double value) override {_bondShininess = value;}

  void setBondSelectionStyle(RKSelectionStyle style) override {_bondSelectionStyle = style;}
  void setBondSelectionFrequency(double value) override;
  void setBondSelectionDensity(double value) override;
  void setBondSelectionIntensity(double value) override {_bondSelectionIntensity = value;}
  void setBondSelectionScaling(double scaling) override {_bondSelectionScaling = scaling;}


  // To be overwritten in subclasses of 'Structure'
  // ===============================================================================================
  bool isFractional() override  {return false;}
  virtual bool hasSymmetry() {return false;}
  virtual std::shared_ptr<Structure> superCell() const {return nullptr;}
  virtual std::shared_ptr<Structure> removeSymmetry() const {return nullptr;}
  virtual std::shared_ptr<Structure> wrapAtomsToCell() const {return nullptr;}
  virtual std::shared_ptr<Structure> flattenHierarchy() const {return nullptr;}
  virtual std::shared_ptr<Structure> appliedCellContentShift() const {return nullptr;}
  virtual std::vector<std::tuple<double3,int,double>> atomSymmetryData() const {return {};}
  virtual void setAtomSymmetryData(double3x3 unitCell, std::vector<std::tuple<double3,int,double>> atomData) {Q_UNUSED(unitCell); Q_UNUSED(atomData);};

  virtual bool hasSelectedAtoms() const;

  virtual std::vector<double3> atomPositions() const {return std::vector<double3>();}
  virtual std::vector<double3> bondPositions() const {return std::vector<double3>();}

  virtual std::vector<double2> potentialParameters() const {return std::vector<double2>();}

  virtual std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const override;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const override;

  virtual std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForFractionalPositions();
  virtual std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForCartesianPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToFractionalPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToCartesianPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToFractionalPositions();
  virtual std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToCartesianPositions();

  virtual double3 centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const;
  virtual double3x3 matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const;

  virtual double bondLength(std::shared_ptr<SKBond> bond) const;
  virtual double3 bondVector(std::shared_ptr<SKBond> bond) const;
  virtual std::pair<double3, double3> computeChangedBondLength(std::shared_ptr<SKBond>, double) const;

  virtual double3 CartesianPosition(double3 position, int3 replicaPosition) const {Q_UNUSED(position); Q_UNUSED(replicaPosition); return double(); }
  virtual double3 FractionalPosition(double3 position, int3 replicaPosition) const {Q_UNUSED(position); Q_UNUSED(replicaPosition); return double(); }



  double2 adsorptionSurfaceProbeParameters() const;
  ProbeMolecule adsorptionSurfaceProbeMolecule() const {return _adsorptionSurfaceProbeMolecule;}

  void recomputeDensityProperties();
  double2 frameworkProbeParameters() const;
  void setStructureNitrogenSurfaceArea(double value);

  SKSpaceGroup& legacySpaceGroup() {return _legacySpaceGroup;}

  // ==========================================================================================
  // Legacy for reading primitive from 'Structure' (Remove in the future)
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
  // ==========================================================================================



  // info
  //=================================================================================================
  // remove in future version
  QString authorFirstName()  {return _authorFirstName;}
  void setAuthorFirstName(QString name) {_authorFirstName = name;}
  QString authorMiddleName() {return _authorMiddleName;}
  void setAuthorMiddleName(QString name)  {_authorMiddleName = name;}
  QString authorLastName() {return _authorLastName;}
  void setAuthorLastName(QString name)  {_authorLastName = name;}
  QString authorOrchidID()  {return _authorOrchidID;}
  void setAuthorOrchidID(QString name)  {_authorOrchidID = name;}
  QString authorResearcherID()  {return _authorResearcherID;}
  void setAuthorResearcherID(QString name)  {_authorResearcherID = name;}
  QString authorAffiliationUniversityName()  {return _authorAffiliationUniversityName;}
  void setAuthorAffiliationUniversityName(QString name)  {_authorAffiliationUniversityName = name;}
  QString authorAffiliationFacultyName()  {return _authorAffiliationFacultyName;}
  void setAuthorAffiliationFacultyName(QString name) {_authorAffiliationFacultyName = name;}
  QString authorAffiliationInstituteName()  {return _authorAffiliationInstituteName;}
  void setAuthorAffiliationInstituteName(QString name)  {_authorAffiliationInstituteName = name;}
  QString authorAffiliationCityName()  {return _authorAffiliationCityName;}
  void setAuthorAffiliationCityName(QString name)  {_authorAffiliationCityName = name;}
  QString authorAffiliationCountryName()  {return _authorAffiliationCountryName;}
  void setAuthorAffiliationCountryName(QString name)  {_authorAffiliationCountryName = name;}


  QDate creationDate() {return _creationDate;}
  void setCreationDate(QDate date) {_creationDate = date;}
  //=================================================================================================

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

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<Structure> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<Structure> &);
protected:
  qint64 _versionNumber{10};

  std::shared_ptr<SKAtomTreeController> _atomsTreeController;
  std::shared_ptr<SKBondSetController> _bondSetController;

  SKSpaceGroup _legacySpaceGroup = SKSpaceGroup(1);

  // ==========================================================================================
  // Legacy for reading primitive from 'Structure' (Remove in the future)
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
  // ==========================================================================================

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
  double _adsorptionTransparencyThreshold = 0.0;
  double _adsorptionSurfaceIsoValue = 0.0;
  qint64 _encompassingPowerOfTwoCubicGridSize = 7;
  double _adsorptionSurfaceMinimumValue = -1000.0;

  //std::pair<double,double> _range{};
  //int3 _dimensions{128,128,128};
  RKEnergySurfaceType _adsorptionSurfaceRenderingMethod = RKEnergySurfaceType::isoSurface;
  RKPredefinedVolumeRenderingTransferFunction  _adsorptionVolumeTransferFunction = RKPredefinedVolumeRenderingTransferFunction::RASPA_PES;
  double _adsorptionVolumeStepLength = 0.0005;

  qint64 _adsorptionSurfaceSize = 128;
  qint64 _adsorptionSurfaceNumberOfTriangles = 0;

  ProbeMolecule _adsorptionSurfaceProbeMolecule = ProbeMolecule::helium;

  double _adsorptionSurfaceHue = 1.0;
  double _adsorptionSurfaceSaturation = 1.0;
  double _adsorptionSurfaceValue = 1.0;

  bool _adsorptionSurfaceFrontSideHDR = true;
  double _adsorptionSurfaceFrontSideHDRExposure = 2.0;
  QColor _adsorptionSurfaceFrontSideAmbientColor = QColor(0, 0, 0, 255);
  QColor _adsorptionSurfaceFrontSideDiffuseColor = QColor(255, 255, 255, 255);
  QColor _adsorptionSurfaceFrontSideSpecularColor = QColor(230, 230, 230, 1.0);
  double _adsorptionSurfaceFrontSideDiffuseIntensity = 1.0;
  double _adsorptionSurfaceFrontSideAmbientIntensity = 0.0;
  double _adsorptionSurfaceFrontSideSpecularIntensity = 0.5;
  double _adsorptionSurfaceFrontSideShininess = 4.0;

  bool _adsorptionSurfaceBackSideHDR = true;
  double _adsorptionSurfaceBackSideHDRExposure = 2.0;
  QColor _adsorptionSurfaceBackSideAmbientColor = QColor(0, 0, 0, 255);
  QColor _adsorptionSurfaceBackSideDiffuseColor = QColor(255, 255, 255, 255);
  QColor _adsorptionSurfaceBackSideSpecularColor = QColor(230, 230, 230, 255);
  double _adsorptionSurfaceBackSideDiffuseIntensity = 1.0;
  double _adsorptionSurfaceBackSideAmbientIntensity = 0.0;
  double _adsorptionSurfaceBackSideSpecularIntensity = 0.5;
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

  // ================================================================
  // remove in future version
  //QString _authorFirstName = QString("");
  //QString _authorMiddleName = QString("");
  //QString _authorLastName = QString("");
  //QString _authorOrchidID = QString("");
  //QString _authorResearcherID = QString("");
  //QString _authorAffiliationUniversityName = QString("");
  //QString _authorAffiliationFacultyName = QString("");
  //QString _authorAffiliationInstituteName = QString("");
  //QString _authorAffiliationCityName = QString("");
  //QString _authorAffiliationCountryName = QString("Netherlands");
  //
  //QDate _creationDate = QDate().currentDate();
  // ================================================================

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
