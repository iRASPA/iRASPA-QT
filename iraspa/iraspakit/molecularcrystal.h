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

#include <symmetrykit.h>
#include "structure.h"
#include "spacegroupviewer.h"
#include "molecularcrystal.h"
#include "structuralpropertyviewer.h"
#include "unitcellviewer.h"

class MolecularCrystal: public Structure, public SpaceGroupViewer,
                        public UnitCellEditor,
                        public VolumetricDataEditor, public StructuralPropertyEditor,
                        public RKRenderUnitCellSource, public RKRenderVolumetricDataSource
{
public:
  MolecularCrystal();
  MolecularCrystal(const MolecularCrystal &molecularCrystal);
  MolecularCrystal(std::shared_ptr<SKStructure> frame);

  ~MolecularCrystal() {}

  // Object
  // ===============================================================================================
  MolecularCrystal(const std::shared_ptr<Object> object);
  ObjectType structureType() override final { return ObjectType::molecularCrystal; }
  std::shared_ptr<Object> shallowClone()  override final;
  SKBoundingBox boundingBox() const override final;
  void reComputeBoundingBox() override final;

  // RKRenderAtomSource: overwritten from Structure
  // ===============================================================================================
  std::vector<RKInPerInstanceAttributesAtoms> renderAtoms() const override final;
  std::vector<RKInPerInstanceAttributesAtoms> renderSelectedAtoms() const override final;
  std::vector<RKInPerInstanceAttributesText> atomTextData(RKFontAtlas *fontAtlas) const override final;

  // RKRenderBondSource: overwritten from Structure
  // ===============================================================================================
  std::vector<RKInPerInstanceAttributesBonds> renderInternalBonds() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderSelectedInternalBonds() const override final;

  // AtomViewer: overwritten from Structure
  // ===============================================================================================
  bool isFractional() override final {return false;}
  void expandSymmetry() override final;
  void expandSymmetry(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom);
  std::set<int> filterCartesianAtomPositions(std::function<bool(double3)> &) override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;

  // BondViewer: overwritten from Structure
  // ===============================================================================================
  BondSelectionIndexSet filterCartesianBondPositions(std::function<bool(double3)> &) override final;
  void computeBonds() override final;

  // Structure: reimplemented
  // ===============================================================================================
  bool hasSymmetry() override final {return true;}
  std::shared_ptr<Structure> superCell() const override final;
  std::shared_ptr<Structure> removeSymmetry() const override final;
  std::shared_ptr<Structure> wrapAtomsToCell() const override final;
  std::shared_ptr<Structure> flattenHierarchy() const override final;
  std::shared_ptr<Structure> appliedCellContentShift() const override final;
  std::vector<std::tuple<double3, int, double>> atomSymmetryData() const override final;
  void setAtomSymmetryData(double3x3 unitCell, std::vector<std::tuple<double3, int, double>> atomData) override final;

  std::vector<double2> potentialParameters() const override final;
  std::vector<double3> atomPositions() const override final;

  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForFractionalPositions() override final;
  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToFractionalPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToFractionalPositions() override final;

  double3 centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const override final;
  double3x3 matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const override final;

  double bondLength(std::shared_ptr<SKBond> bond) const override final;
  double3 bondVector(std::shared_ptr<SKBond> bond) const override final;
  std::pair<double3, double3> computeChangedBondLength(std::shared_ptr<SKBond> bond, double bondlength) const override final;

  // Protocol:  SpaceGroupViewer
  // ===============================================================================================
  SKSpaceGroup& spaceGroup() override final {return _spaceGroup;}
  void setSpaceGroupHallNumber(int HallNumber)  override final {_spaceGroup = SKSpaceGroup(HallNumber);}

  // Protocol: RKRenderUnitCellSource and UnitCeViewer
  // ===============================================================================================
  bool drawUnitCell() const override final {return _drawUnitCell;}
  void setDrawUnitCell(bool value) override final {_drawUnitCell = value;}
  double unitCellScaleFactor() const override final {return _unitCellScaleFactor;}
  QColor unitCellDiffuseColor() const override final {return _unitCellDiffuseColor;}
  double unitCellDiffuseIntensity() const override final {return _unitCellDiffuseIntensity;}
  std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const override final;

  // Protocol: UnitCellEditor
  // ===============================================================================================

  void setUnitCellScaleFactor(double value) override final {_unitCellScaleFactor = value;}
  void setUnitCellDiffuseColor(QColor value) override final {_unitCellDiffuseColor = value;}
  void setUnitCellDiffuseIntensity(double value) override final {_unitCellDiffuseIntensity = value;}

  // Protocol: RKRenderAdsorptionSurfaceSource
  // ===============================================================================================
  bool drawAdsorptionSurface() const override final {return _drawAdsorptionSurface;}
  int3 dimensions() const override final {return _dimensions;}
  std::vector<float> gridData() override final;
  std::vector<float4> gridValueAndGradientData() override final;
  bool isImmutable() const override final {return false;}

  RKEnergySurfaceType adsorptionSurfaceRenderingMethod() const override final {return _adsorptionSurfaceRenderingMethod;}
  RKPredefinedVolumeRenderingTransferFunction adsorptionVolumeTransferFunction() const override final {return _adsorptionVolumeTransferFunction;}
  double adsorptionVolumeStepLength() const override final {return _adsorptionVolumeStepLength;}

  double adsorptionSurfaceOpacity() const override final {return _adsorptionSurfaceOpacity;}
  double adsorptionTransparencyThreshold() const override final {return _adsorptionTransparencyThreshold;}
  double adsorptionSurfaceIsoValue() const override final {return _adsorptionSurfaceIsoValue;}
  int encompassingPowerOfTwoCubicGridSize() const override final {return _encompassingPowerOfTwoCubicGridSize;}

  double adsorptionSurfaceHue() const override final {return _adsorptionSurfaceHue;}
  double adsorptionSurfaceSaturation() const override final {return _adsorptionSurfaceSaturation;}
  double adsorptionSurfaceValue() const override final {return _adsorptionSurfaceValue;}

  bool adsorptionSurfaceFrontSideHDR() const override final {return _adsorptionSurfaceFrontSideHDR;}
  double adsorptionSurfaceFrontSideHDRExposure() const override final {return _adsorptionSurfaceFrontSideHDRExposure;}
  QColor adsorptionSurfaceFrontSideAmbientColor() const override final {return _adsorptionSurfaceFrontSideAmbientColor;}
  QColor adsorptionSurfaceFrontSideDiffuseColor() const override final {return _adsorptionSurfaceFrontSideDiffuseColor;}
  QColor adsorptionSurfaceFrontSideSpecularColor() const override final {return _adsorptionSurfaceFrontSideSpecularColor;}
  double adsorptionSurfaceFrontSideDiffuseIntensity() const override final {return _adsorptionSurfaceFrontSideDiffuseIntensity;}
  double adsorptionSurfaceFrontSideAmbientIntensity() const override final {return _adsorptionSurfaceFrontSideAmbientIntensity;}
  double adsorptionSurfaceFrontSideSpecularIntensity() const override final {return _adsorptionSurfaceFrontSideSpecularIntensity;}
  double adsorptionSurfaceFrontSideShininess() const override final {return _adsorptionSurfaceFrontSideShininess;}

  bool adsorptionSurfaceBackSideHDR() const override final {return _adsorptionSurfaceBackSideHDR;}
  double adsorptionSurfaceBackSideHDRExposure() const override final {return _adsorptionSurfaceBackSideHDRExposure;}
  QColor adsorptionSurfaceBackSideAmbientColor() const override final {return _adsorptionSurfaceBackSideAmbientColor;}
  QColor adsorptionSurfaceBackSideDiffuseColor() const override final {return _adsorptionSurfaceBackSideDiffuseColor;}
  QColor adsorptionSurfaceBackSideSpecularColor() const override final {return _adsorptionSurfaceBackSideSpecularColor;}
  double adsorptionSurfaceBackSideDiffuseIntensity() const override final {return _adsorptionSurfaceBackSideDiffuseIntensity;}
  double adsorptionSurfaceBackSideAmbientIntensity() const override final {return _adsorptionSurfaceBackSideAmbientIntensity;}
  double adsorptionSurfaceBackSideSpecularIntensity() const override final {return _adsorptionSurfaceBackSideSpecularIntensity;}
  double adsorptionSurfaceBackSideShininess() const override final {return _adsorptionSurfaceBackSideShininess;}

  // Protocol: VolumetricDataViewer (many already defined in RKRenderAdsorptionSurfaceSource)
  // ===============================================================================================

  void setDrawAdsorptionSurface(bool state) override final {_drawAdsorptionSurface = state;}
  std::pair<double,double> range() const override final {return _range;}
  double3 spacing() const override final {return _spacing;}
  QByteArray data() const override final {return _data;}
  double average() const override final {return _average;}
  double variance() const override final {return _variance;}

  void setAdsorptionSurfaceOpacity(double value) override final {_adsorptionSurfaceOpacity = value;}
  void setAdsorptionTransparencyThreshold(double value) override final {_adsorptionTransparencyThreshold = value;}
  void setAdsorptionSurfaceIsoValue(double value) override final {_adsorptionSurfaceIsoValue = value;}
  ProbeMolecule adsorptionSurfaceProbeMolecule() const override final {return Structure::adsorptionSurfaceProbeMolecule();}
  void setAdsorptionSurfaceProbeMolecule(ProbeMolecule value) override final {_adsorptionSurfaceProbeMolecule = value;}

  void setAdsorptionSurfaceRenderingMethod(RKEnergySurfaceType type) override final {_adsorptionSurfaceRenderingMethod = type;}
  void setAdsorptionVolumeTransferFunction(RKPredefinedVolumeRenderingTransferFunction function) override final {_adsorptionVolumeTransferFunction = function;}
  void setAdsorptionVolumeStepLength(double value) override final {_adsorptionVolumeStepLength = value;}

  void setAdsorptionSurfaceHue(double value) override final {_adsorptionSurfaceHue = value;}
  void setAdsorptionSurfaceSaturation(double value) override final {_adsorptionSurfaceSaturation = value;}
  void setAdsorptionSurfaceValue(double value) override final {_adsorptionSurfaceValue = value;}

  void setAdsorptionSurfaceFrontSideHDR(bool state) override final {_adsorptionSurfaceFrontSideHDR = state;}
  void setAdsorptionSurfaceFrontSideHDRExposure(double value) override final {_adsorptionSurfaceFrontSideHDRExposure = value;}
  void setAdsorptionSurfaceFrontSideAmbientColor(QColor color) override final {_adsorptionSurfaceFrontSideAmbientColor = color;}
  void setAdsorptionSurfaceFrontSideDiffuseColor(QColor color) override final {_adsorptionSurfaceFrontSideDiffuseColor = color;}
  void setAdsorptionSurfaceFrontSideSpecularColor(QColor color) override final {_adsorptionSurfaceFrontSideSpecularColor = color;}
  void setAdsorptionSurfaceFrontSideDiffuseIntensity(double value) override final {_adsorptionSurfaceFrontSideDiffuseIntensity = value;}
  void setAdsorptionSurfaceFrontSideAmbientIntensity(double value) override final {_adsorptionSurfaceFrontSideAmbientIntensity = value;}
  void setAdsorptionSurfaceFrontSideSpecularIntensity(double value) override final {_adsorptionSurfaceFrontSideSpecularIntensity = value;}
  void setAdsorptionSurfaceFrontSideShininess(double value) override final {_adsorptionSurfaceFrontSideShininess = value;}

  void setAdsorptionSurfaceBackSideHDR(bool state) override final {_adsorptionSurfaceBackSideHDR = state;}
  void setAdsorptionSurfaceBackSideHDRExposure(double value) override final {_adsorptionSurfaceBackSideHDRExposure = value;}
  void setAdsorptionSurfaceBackSideAmbientColor(QColor color) override final {_adsorptionSurfaceBackSideAmbientColor = color;}
  void setAdsorptionSurfaceBackSideDiffuseColor(QColor color) override final {_adsorptionSurfaceBackSideDiffuseColor = color;}
  void setAdsorptionSurfaceBackSideSpecularColor(QColor color) override final {_adsorptionSurfaceBackSideSpecularColor = color;}
  void setAdsorptionSurfaceBackSideDiffuseIntensity(double value) override final {_adsorptionSurfaceBackSideDiffuseIntensity = value;}
  void setAdsorptionSurfaceBackSideAmbientIntensity(double value) override final {_adsorptionSurfaceBackSideAmbientIntensity = value;}
  void setAdsorptionSurfaceBackSideSpecularIntensity(double value) override final {_adsorptionSurfaceBackSideSpecularIntensity = value;}
  void setAdsorptionSurfaceBackSideShininess(double value) override final {_adsorptionSurfaceBackSideShininess = value;}

  // Protocol: VolumetricDataEditor
  // ===============================================================================================

  void setEncompassingPowerOfTwoCubicGridSize(int value) override final;

  // Protocol: StructuralPropertyViewer
  // ===============================================================================================
  double computeVoidFraction() const override final;
  double computeNitrogenSurfaceArea() const override final;
  void recomputeDensityProperties() override final {Structure::recomputeDensityProperties();}

  //   var structureType: Structure.StructureType {get}
  QString structureMaterialType() const override final {return _structureMaterialType;}
  ProbeMolecule frameworkProbeMolecule() const override final {return _frameworkProbeMolecule;}
  double structureMass() const override final {return _structureMass;}
  double structureDensity() const override final {return _structureDensity;}
  double structureHeliumVoidFraction() const override final {return _structureHeliumVoidFraction;}
  double structureSpecificVolume() const override final {return _structureSpecificVolume;}
  double structureAccessiblePoreVolume() const override final {return _structureAccessiblePoreVolume;}
  double structureVolumetricNitrogenSurfaceArea() const override final {return _structureVolumetricNitrogenSurfaceArea;}
  double structureGravimetricNitrogenSurfaceArea() const override final {return _structureGravimetricNitrogenSurfaceArea;}
  int structureNumberOfChannelSystems() const override final {return _structureNumberOfChannelSystems;}
  int structureNumberOfInaccessiblePockets() const override final {return _structureNumberOfInaccessiblePockets;}
  int structureDimensionalityOfPoreSystem() const override final {return _structureDimensionalityOfPoreSystem;}
  double structureLargestCavityDiameter() const override final {return _structureLargestCavityDiameter;}
  double structureRestrictingPoreLimitingDiameter() const override final {return _structureRestrictingPoreLimitingDiameter;}
  double structureLargestCavityDiameterAlongAViablePath() const override final {return _structureLargestCavityDiameterAlongAViablePath;}


  // Protocol: StructuralPropertyEditor
  // ===============================================================================================

  void setStructureMaterialType(QString value) override final {_structureMaterialType = value;}
  void setFrameworkProbeMolecule(ProbeMolecule molecule) override final {_frameworkProbeMolecule = molecule;}
  void setStructureMass(double value) override final {_structureMass = value;}
  void setStructureDensity(double value) override final {_structureDensity = value;}
  void setStructureHeliumVoidFraction(double value) override final {_structureHeliumVoidFraction = value;}
  void setStructureSpecificVolume(double value) override final {_structureSpecificVolume = value;}
  void setStructureAccessiblePoreVolume(double value) override final {_structureAccessiblePoreVolume = value;}
  void setStructureVolumetricNitrogenSurfaceArea(double value) override final;
  void setStructureGravimetricNitrogenSurfaceArea(double value) override final;
  void setStructureNumberOfChannelSystems(int value) override final {_structureNumberOfChannelSystems = value;}
  void setStructureNumberOfInaccessiblePockets(int value) override final {_structureNumberOfInaccessiblePockets = value;}
  void setStructureDimensionalityOfPoreSystem(int value) override final {_structureDimensionalityOfPoreSystem = value;}
  void setStructureLargestCavityDiameter(double value) override final {_structureLargestCavityDiameter = value;}
  void setStructureRestrictingPoreLimitingDiameter(double value) override final {_structureRestrictingPoreLimitingDiameter = value;}
  void setStructureLargestCavityDiameterAlongAViablePath(double value) override final {_structureLargestCavityDiameterAlongAViablePath = value;}
private:
  qint64 _versionNumber{2};

  SKSpaceGroup _spaceGroup = SKSpaceGroup(1);

  int3 _dimensions{128,128,128};
  std::pair<double,double> _range{};
  double3 _spacing = double3(0.1,0.1,0.1);
  QByteArray _data = QByteArray();
  double _average = 0.0;
  double _variance = 0.0;

  std::vector<double3> atomUnitCellPositions() const;

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<MolecularCrystal> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<MolecularCrystal> &);
};

