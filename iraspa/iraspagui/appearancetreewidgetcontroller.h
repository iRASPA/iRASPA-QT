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

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QColor>
#include <iraspakit.h>
#include <mathkit.h>
#include "iraspamainwindowconsumerprotocol.h"
#include <simulationkit.h>
#include <optional>
#include "appearanceprimitiveform.h"
#include "appearanceatomsform.h"
#include "appearancebondsform.h"
#include "appearanceunitcellform.h"
#include "appearancelocalaxesform.h"
#include "appearanceadsorptionsurfaceform.h"
#include "appearanceannotationform.h"

class AppearanceTreeWidgetController: public QTreeWidget, public MainWindowConsumer, public ProjectConsumer, public Reloadable, public LogReportingConsumer
{
  Q_OBJECT

public:
  AppearanceTreeWidgetController(QWidget* parent = nullptr);

  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override final;
  void setMainWindow(MainWindow *mainWindow) override final {_mainWindow = mainWindow;}
  void setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAObject>> iraspa_structures);
  void resetData();

  void setLogReportingWidget(LogReporting *logReporting) override final {_logReporter = logReporting;}
  LogReporting* logReporter() const override final {return _logReporter;};
private:
  AppearancePrimitiveForm* _appearancePrimitiveForm;
  AppearanceAtomsForm* _appearanceAtomsForm;
  AppearanceBondsForm* _appearanceBondsForm;
  AppearanceUnitCellForm* _appearanceUnitCellForm;
  AppearanceLocalAxesForm* _appearanceLocalAxesForm;
  AppearanceAdsorptionSurfaceForm* _appearanceAdsorptionSurfaceForm;
  AppearanceAnnotationForm* _appearanceAnnotationForm;

  QPushButton* pushButtonPrimitive;
  QPushButton* pushButtonAtoms;
  QPushButton* pushButtonBonds;
  QPushButton* pushButtonUnitCell;
  QPushButton* pushButtonLocalAxes;
  QPushButton* pushButtonAdsorptionSurface;
  QPushButton* pushButtonAnnotation;

  LogReporting* _logReporter = nullptr;
  MainWindow *_mainWindow;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<ProjectStructure> _projectStructure;
  std::vector<std::shared_ptr<iRASPAObject>> _iraspa_structures{};
  void reloadData() override final;
  void reloadSelection() override final;

  void reloadPrimitiveProperties();
  void reloadAtomProperties();
  void reloadBondProperties();
  void reloadUnitCellProperties();
  void reloadAdsorptionSurfaceProperties();
  void reloadAnnotationProperties();

  // primitive properties
  void reloadRotationAngle();
  void reloadEulerAngles();
  void reloadTransformationMatrix();
  void reloadOpacity();
  void reloadNumberOfSides();
  void reloadIsCapped();
  void reloadPrimitiveSelectionStyle();
  void reloadPrimitiveSelectionIntensity();
  void reloadPrimitiveSelectionScaling();
  void reloadPrimitiveHue();
  void reloadPrimitiveSaturation();
  void reloadPrimitiveValue();
  void reloadFrontPrimitiveHighDynamicRange();
  void reloadFrontPrimitiveHDRExposure();
  void reloadFrontPrimitiveAmbientLight();
  void reloadFrontPrimitiveDiffuseLight();
  void reloadFrontPrimitiveSpecularLight();
  void reloadFrontPrimitiveShininess();
  void reloadBackPrimitiveHighDynamicRange();
  void reloadBackPrimitiveHDRExposure();
  void reloadBackPrimitiveAmbientLight();
  void reloadBackPrimitiveDiffuseLight();
  void reloadBackPrimitiveSpecularLight();
  void reloadBackPrimitiveShininess();

  std::optional<std::unordered_set<double> > rotationAngle();
  void setRotationAngle(double angle);
  void rotateYawPlus();
  void rotateYawMinus();
  void rotatePitchPlus();
  void rotatePitchMinus();
  void rotateRollPlus();
  void rotateRollMinus();

  void setEulerAngleX(double angle);
  std::optional<std::unordered_set<double>> EulerAngleX();
  void setEulerAngleY(double angle);
  std::optional<std::unordered_set<double>> EulerAngleY();
  void setEulerAngleZ(double angle);
  std::optional<std::unordered_set<double>> EulerAngleZ();


  std::optional<std::unordered_set<double>> transformationMatrixAX();
  void setTransformationMatrixAX(double value);
  std::optional<std::unordered_set<double>> transformationMatrixAY();
  void setTransformationMatrixAY(double value);
  std::optional<std::unordered_set<double>> transformationMatrixAZ();
  void setTransformationMatrixAZ(double value);

  std::optional<std::unordered_set<double>> transformationMatrixBX();
  void setTransformationMatrixBX(double value);
  std::optional<std::unordered_set<double>> transformationMatrixBY();
  void setTransformationMatrixBY(double value);
  std::optional<std::unordered_set<double>> transformationMatrixBZ();
  void setTransformationMatrixBZ(double value);

  std::optional<std::unordered_set<double>> transformationMatrixCX();
  void setTransformationMatrixCX(double value);
  std::optional<std::unordered_set<double>> transformationMatrixCY();
  void setTransformationMatrixCY(double value);
  std::optional<std::unordered_set<double>> transformationMatrixCZ();
  void setTransformationMatrixCZ(double value);

  std::optional<std::unordered_set<double>> primitiveOpacity();
  void setPrimitiveOpacity(double opacity);
  std::optional<std::unordered_set<int> > primitiveNumberOfSides();
  void setPrimitiveNumberOfSides(int numberOfSides);

  std::optional<std::unordered_set<bool>> primitiveIsCapped();
  void setPrimitiveIsCapped(bool capped);

  void setPrimitiveHue(double value);
  std::optional<std::unordered_set<double> > primitiveHue();
  void setPrimitiveSaturation(double value);
  std::optional<std::unordered_set<double> > primitiveSaturation();
  void setPrimitiveValue(double value);
  std::optional<std::unordered_set<double> > primitiveValue();

  void setPrimitiveSelectionStyle(int value);
  std::optional<std::unordered_set<RKSelectionStyle, enum_hash> > primitiveSelectionStyle();
  void setPrimitiveSelectionStyleNu(double value);
  std::optional<std::unordered_set<double>> primitiveSelectionStyleNu();
  void setPrimitiveSelectionStyleRho(double value);
  std::optional<std::unordered_set<double> > primitiveSelectionStyleRho();
  void setPrimitiveSelectionIntensity(double value);
  std::optional<std::unordered_set<double> > primitiveSelectionIntensity();
  void setPrimitiveSelectionScaling(double value);
  std::optional<std::unordered_set<double> > primitiveSelectionScaling();

  void setFrontPrimitiveHighDynamicRange(int value);
  std::optional<std::unordered_set<bool> > frontPrimitiveHighDynamicRange();
  void setFrontPrimitiveHDRExposure(double value);
  std::optional<std::unordered_set<double> > frontPrimitiveHDRExposure();
  void setFrontPrimitiveAmbientLightIntensity(double value);
  std::optional<std::unordered_set<double> > frontPrimitiveAmbientLightIntensity();
  void setFrontPrimitiveAmbientLightColor();
  std::optional<std::unordered_set<QColor> > frontPrimitiveAmbientLightColor();
  void setFrontPrimitiveDiffuseLightIntensity(double value);
  std::optional<std::unordered_set<double> > frontPrimitiveDiffuseLightIntensity();
  void setFrontPrimitiveDiffuseLightColor();
  std::optional<std::unordered_set<QColor> > frontPrimitiveDiffuseLightColor();
  void setFrontPrimitiveSpecularLightIntensity(double value);
  std::optional<std::unordered_set<double> > frontPrimitiveSpecularLightIntensity();
  void setFrontPrimitiveSpecularLightColor();
  std::optional<std::unordered_set<QColor>> frontPrimitiveSpecularLightColor();
  void setFrontPrimitiveShininess(double value);
  std::optional<std::unordered_set<double> > frontPrimitiveShininess();

  void setBackPrimitiveHighDynamicRange(int value);
  std::optional<std::unordered_set<bool> > backPrimitiveHighDynamicRange();
  void setBackPrimitiveHDRExposure(double value);
  std::optional<std::unordered_set<double> > backPrimitiveHDRExposure();
  void setBackPrimitiveAmbientLightIntensity(double value);
  std::optional<std::unordered_set<double> > backPrimitiveAmbientLightIntensity();
  void setBackPrimitiveAmbientLightColor();
  std::optional<std::unordered_set<QColor> > backPrimitiveAmbientLightColor();
  void setBackPrimitiveDiffuseLightIntensity(double value);
  std::optional<std::unordered_set<double> > backPrimitiveDiffuseLightIntensity();
  void setBackPrimitiveDiffuseLightColor();
  std::optional<std::unordered_set<QColor> > backPrimitiveDiffuseLightColor();
  void setBackPrimitiveSpecularLightIntensity(double value);
  std::optional<std::unordered_set<double> > backPrimitiveSpecularLightIntensity();
  void setBackPrimitiveSpecularLightColor();
  std::optional<std::unordered_set<QColor>> backPrimitiveSpecularLightColor();
  void setBackPrimitiveShininess(double value);
  std::optional<std::unordered_set<double>> backPrimitiveShininess();

  // atom properties

  void reloadAtomRepresentationType();
  void reloadAtomRepresentationStyle();
  void reloadAtomColorSet();
  void reloadAtomColorSetOrder();
  void reloadAtomForceFieldSet();
  void reloadAtomForceFieldSetOrder();
  void reloadAtomDrawAtoms();
  void reloadAtomSelectionStyle();
  void reloadAtomSelectionIntensity();
  void reloadAtomSelectionScaling();
  void reloadAtomSizeScalingDoubleSpinBox();
  void reloadAtomSizeScalingDoubleSlider();
  void reloadAtomHighDynamicRange();
  void reloadAtomHDRExposure();
  void reloadAtomHue();
  void reloadAtomSaturation();
  void reloadAtomValue();
  void reloadAtomAmbientOcclusion();
  void reloadAtomAmbientLight();
  void reloadAtomDiffuseLight();
  void reloadAtomSpecularLight();
  void reloadAtomShininess();

  void setRepresentationType(int value);
  std::optional<std::unordered_set<AtomStructureViewer::RepresentationType, enum_hash> > representationType();
  void setRepresentationStyle(int value);
  std::optional<std::unordered_set<AtomStructureViewer::RepresentationStyle, enum_hash> > representationStyle();
  void setColorSchemeComboBoxIndex(int value);
  std::optional<std::set<QString> > colorSchemeIdentifier();
  void setColorSchemeOrder(int value);
  std::optional<std::unordered_set<SKColorSet::ColorSchemeOrder, enum_hash> > colorSchemeOrder();
  void setForcefieldSchemeComboBoxIndex(int value);
  std::optional<std::set<QString> > forceFieldSchemeIdentifier();
  void setForceFieldSchemeOrder(int value);
  std::optional<std::unordered_set<ForceFieldSet::ForceFieldSchemeOrder, enum_hash> > forceFieldSchemeOrder();
  void setAtomSelectionStyle(int value);
  std::optional<std::unordered_set<RKSelectionStyle, enum_hash> > atomSelectionStyle();
  void setAtomSelectionStyleNu(double value);
  std::optional<std::unordered_set<double> > atomSelectionStyleNu();
  void setAtomSelectionStyleRho(double value);
  std::optional<std::unordered_set<double> > atomSelectionStyleRho();
  void setAtomSelectionIntensity(double value);
  std::optional<std::unordered_set<double> > atomSelectionIntensity();
  void setAtomSelectionScaling(double value);
  std::optional<std::unordered_set<double> > atomSelectionScaling();
  void setAtomDrawAtoms(int state);
  std::optional<std::unordered_set<bool>> atomDrawAtoms();
  void setAtomSizeScalingDoubleSpinBox(double size);
  void setAtomSizeScalingDoubleSliderBegin();
  void setAtomSizeScalingDoubleSliderIntermediate(double size);
  void setAtomSizeScalingDoubleSliderFinal();
  std::optional<std::unordered_set<double> > atomSizeScaling();
  void setAtomHighDynamicRange(int value);
  std::optional<std::unordered_set<bool> > atomHighDynamicRange();
  void setAtomHDRExposure(double value);
  std::optional<std::unordered_set<double> > atomHDRExposure();
  void setAtomHue(double value);
  std::optional<std::unordered_set<double> > atomHue();
  void setAtomSaturation(double value);
  std::optional<std::unordered_set<double> > atomSaturation();
  void setAtomValue(double value);
  std::optional<std::unordered_set<double> > atomValue();
  void setAtomAmbientOcclusion(int value);
  std::optional<std::unordered_set<bool> > atomAmbientOcclusion();
  void setAtomAmbientLightIntensity(double value);
  std::optional<std::unordered_set<double> > atomAmbientLightIntensity();
  void setAtomAmbientLightColor();
  std::optional<std::unordered_set<QColor> > atomAmbientLightColor();
  void setAtomDiffuseLightIntensity(double value);
  std::optional<std::unordered_set<double> > atomDiffuseLightIntensity();
  void setAtomDiffuseLightColor();
  std::optional<std::unordered_set<QColor> > atomDiffuseLightColor();
  void setAtomSpecularLightIntensity(double value);
  std::optional<std::unordered_set<double> > atomSpecularLightIntensity();
  void setAtomSpecularLightColor();
  std::optional<std::unordered_set<QColor> > atomSpecularLightColor();
  void setAtomShininess(double value);
  std::optional<std::unordered_set<double> > atomShininess();

  // bond properties

  void reloadDrawBondsCheckBox();
  void reloadBondSizeScaling();
  void reloadBondColorMode();
  void reloadBondSelectionStyle();
  void reloadBondSelectionIntensity();
  void reloadBondSelectionScaling();
  void reloadBondHighDynamicRange();
  void reloadBondHDRExposure();
  void reloadBondHue();
  void reloadBondSaturation();
  void reloadBondValue();
  void reloadBondAmbientOcclusion();
  void reloadBondAmbientLight();
  void reloadBondDiffuseLight();
  void reloadBondSpecularLight();
  void reloadBondShininess();

  void setBondDrawBonds(int state);
  std::optional<std::unordered_set<bool> > bondDrawBonds();
  void setBondSizeScaling(double size);
  std::optional<std::unordered_set<double> > bondSizeScaling();
  void setBondColorMode(int value);
  std::optional<std::unordered_set<RKBondColorMode, enum_hash> > bondColorMode();
  void setBondSelectionStyle(int value);
  std::optional<std::unordered_set<RKSelectionStyle, enum_hash> > bondSelectionStyle();
  void setBondSelectionStyleNu(double value);
  std::optional<std::unordered_set<double> > bondSelectionStyleNu();
  void setBondSelectionStyleRho(double value);
  std::optional<std::unordered_set<double> > bondSelectionStyleRho();
  void setBondSelectionIntensity(double value);
  std::optional<std::unordered_set<double> > bondSelectionIntensity();
  void setBondSelectionScaling(double value);
  std::optional<std::unordered_set<double> > bondSelectionScaling();
  void setBondHighDynamicRange(int value);
  std::optional<std::unordered_set<bool> > bondHighDynamicRange();
  void setBondHDRExposure(double value);
  std::optional<std::unordered_set<double> > bondHDRExposure();
  void setBondHue(double value);
  std::optional<std::unordered_set<double> > bondHue();
  void setBondSaturation(double value);
  std::optional<std::unordered_set<double>> bondSaturation();
  void setBondValue(double value);
  std::optional<std::unordered_set<double> > bondValue();
  void setBondAmbientOcclusion(int value);
  std::optional<std::unordered_set<bool> > bondAmbientOcclusion();
  void setBondAmbientLightIntensity(double value);
  std::optional<std::unordered_set<double> > bondAmbientLightIntensity();
  void setBondAmbientLightColor();
  std::optional<std::unordered_set<QColor> > bondAmbientLightColor();
  void setBondDiffuseLightIntensity(double value);
  std::optional<std::unordered_set<double> > bondDiffuseLightIntensity();
  void setBondDiffuseLightColor();
  std::optional<std::unordered_set<QColor> > bondDiffuseLightColor();
  void setBondSpecularLightIntensity(double value);
  std::optional<std::unordered_set<double> > bondSpecularLightIntensity();
  void setBondSpecularLightColor();
  std::optional<std::unordered_set<QColor> > bondSpecularLightColor();
  void setBondShininess(double value);
  std::optional<std::unordered_set<double> > bondShininessy();

  // unit cell properties
  void reloadDrawUnitCell();
  void reloadUnitCellSizeScaling();
  void reloadUnitCellDiffuseLight();

  void setDrawUnitCell(int state);
  std::optional<bool> drawUnitCell();
  void setUnitCellSizeScaling(double size);
  std::optional<double> unitCellSizeScaling();
  void setUnitCellDiffuseLightIntensity(double value);
  std::optional<double> unitCellDiffuseLightIntensity();
  void setUnitCellDiffuseLightColor();
  std::optional<QColor> unitCellDiffuseLightColor();


  // local axes properties
  void reloadLocalAxes();
  void reloadLocalAxesPosition();
  void reloadLocalAxesStyle();
  void reloadLocalAxesScalingStyle();
  void reloadLocalAxesLength();
  void reloadLocalAxesWidth();
  void reloadLocalAxesOffset();

  std::optional<std::unordered_set<RKLocalAxes::Position, enum_hash> > localAxesPosition();
  void setLocalAxesPosition(int value);
  std::optional<std::unordered_set<RKLocalAxes::Style, enum_hash> > localAxesStyle();
  void setLocalAxesStyle(int value);
  std::optional<std::unordered_set<RKLocalAxes::ScalingType, enum_hash> > localAxesScalingType();
  void setLocalAxesScalingType(int value);

  std::optional<std::unordered_set<double> > localAxesLength();
  void setLocalAxesLength(double value);
  std::optional<std::unordered_set<double> > localAxesWidth();
  void setLocalAxesWidth(double value);
  std::optional<std::unordered_set<double> > localAxesOffsetX();
  void setLocalAxesOffsetX(double value);
  std::optional<std::unordered_set<double>> localAxesOffsetY();
  void setLocalAxesOffsetY(double value);
  std::optional<std::unordered_set<double>> localAxesOffsetZ();
  void setLocalAxesOffsetZ(double value);


  // adsorption surface properties

  void reloadDrawAdsorptionSurfaceCheckBox();
  void reloadAdsorptionSurfaceRenderingMethod();
  void reloadAdsorptionSurfaceProbeMolecule();
  void reloadAdsorptionVolumeTransferFunction();
  void reloadAdsorptionVolumeStepLength();
  void reloadAdsorptionSurfaceIsovalue();
  void reloadAdsorptionSurfaceOpacity();
  void reloadAdsorptionTransparencyThreshold();
  void reloadAdsorptionQuality();
  void reloadAdsorptionSurfaceHue();
  void reloadAdsorptionSurfaceSaturation();
  void reloadAdsorptionSurfaceValue();
  void reloadAdsorptionSurfaceInsideHighDynamicRange();
  void reloadAdsorptionSurfaceInsideHDRExposure();
  void reloadAdsorptionSurfaceInsideAmbientLight();
  void reloadAdsorptionSurfaceInsideDiffuseLight();
  void reloadAdsorptionSurfaceInsideSpecularLight();
  void reloadAdsorptionSurfaceInsideShininess();
  void reloadAdsorptionSurfaceOutsideHighDynamicRange();
  void reloadAdsorptionSurfaceOutsideHDRExposure();
  void reloadAdsorptionSurfaceOutsideAmbientLight();
  void reloadAdsorptionSurfaceOutsideDiffuseLight();
  void reloadAdsorptionSurfaceOutsideSpecularLight();
  void reloadAdsorptionSurfaceOutsideShininess();

  void setDrawAdsorptionSurface(int state);
  void setAdsorptionSurfaceRenderingMethod(int value);
  std::optional<std::unordered_set<RKEnergySurfaceType, enum_hash> > adsorptionSurfaceRenderingMethod();
  void setAdsorptionVolumeTransferFunction(int value);
  std::optional<std::unordered_set<RKPredefinedVolumeRenderingTransferFunction, enum_hash> > adsorptionVolumeTransferFunction();
  void setAdsorptionVolumeStepLength(double value);
  std::optional<std::unordered_set<double>> adsorptionVolumeStepLength();

  std::optional<std::unordered_set<bool>> drawAdsorptionSurface();
  void setAdsorptionSurfaceProbeMolecule(int value);
  std::optional<std::unordered_set<ProbeMolecule, enum_hash> > adsorptionSurfaceProbeMolecule();
  void setAdsorptionSurfaceIsovalue(double value);
  std::optional<std::set<double>> adsorptionSurfaceIsovalue();
  double adsorptionSurfaceMinimumValue();
  double adsorptionSurfaceMaximumValue();
  void setAdsorptionSurfaceOpacity(double value);
  std::optional<std::unordered_set<double>> adsorptionSurfaceOpacity();
  void setAdsorptionTransparencyThreshold(double value);
  std::optional<std::unordered_set<double>> adsorptionTransparencyThreshold();
  void setAdsorptionQualityIndex(int value);
  std::optional<std::unordered_set<int> > adsorptionQualityIndex();
  std::optional<std::unordered_set<bool> > adsorptionQualityIndexIsImmutable();

  void setAdsorptionSurfaceHue(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceHue();
  void setAdsorptionSurfaceSaturation(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceSaturation();
  void setAdsorptionSurfaceValue(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceValue();
  std::optional<std::unordered_set<int3>> adsorptionDimensions();

  void setAdsorptionSurfaceInsideHighDynamicRange(int value);
  std::optional<std::unordered_set<bool> > adsorptionSurfaceInsideHighDynamicRange();
  void setAdsorptionSurfaceInsideHDRExposure(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceInsideHDRExposure();
  void setAdsorptionSurfaceInsideAmbientLightIntensity(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceInsideAmbientLightIntensity();
  void setAdsorptionSurfaceInsideAmbientLightColor();
  std::optional<std::unordered_set<QColor> > adsorptionSurfaceInsideAmbientLightColor();
  void setAdsorptionSurfaceInsideDiffuseLightIntensity(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceInsideDiffuseLightIntensity();
  void setAdsorptionSurfaceInsideDiffuseLightColor();
  std::optional<std::unordered_set<QColor> > adsorptionSurfaceInsideDiffuseLightColor();
  void setAdsorptionSurfaceInsideSpecularLightIntensity(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceInsideSpecularLightIntensity();
  void setAdsorptionSurfaceInsideSpecularLightColor();
  std::optional<std::unordered_set<QColor> > adsorptionSurfaceInsideSpecularLightColor();
  void setAdsorptionSurfaceInsideShininess(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceInsideShininessy();

  void setAdsorptionSurfaceOutsideHighDynamicRange(int value);
  std::optional<std::unordered_set<bool> > adsorptionSurfaceOutsideHighDynamicRange();
  void setAdsorptionSurfaceOutsideHDRExposure(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceOutsideHDRExposure();
  void setAdsorptionSurfaceOutsideAmbientLightIntensity(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceOutsideAmbientLightIntensity();
  void setAdsorptionSurfaceOutsideAmbientLightColor();
  std::optional<std::unordered_set<QColor> > adsorptionSurfaceOutsideAmbientLightColor();
  void setAdsorptionSurfaceOutsideDiffuseLightIntensity(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceOutsideDiffuseLightIntensity();
  void setAdsorptionSurfaceOutsideDiffuseLightColor();
  std::optional<std::unordered_set<QColor> > adsorptionSurfaceOutsideDiffuseLightColor();
  void setAdsorptionSurfaceOutsideSpecularLightIntensity(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceOutsideSpecularLightIntensity();
  void setAdsorptionSurfaceOutsideSpecularLightColor();
  std::optional<std::unordered_set<QColor> > adsorptionSurfaceOutsideSpecularLightColor();
  void setAdsorptionSurfaceOutsideShininess(double value);
  std::optional<std::unordered_set<double> > adsorptionSurfaceOutsideShininessy();


  void reloadAnnotationType();
  void reloadAnnotationFont();
  void reloadAnnotationColor();
  void reloadAnnotationAlignment();
  void reloadAnnotationStyle();
  void reloadAnnotationScaling();
  void reloadAnnotationOffset();
  void setAnnotationType(int value);
  std::optional<std::set<RKTextType> > annotationType();
  void setAnnotationFontFamily(QString value);
  void setAnnotationFontMember(QString value);
  std::optional<std::set<QString> > annotationFont();
  void setAnnotationTextColor();
  std::optional<std::unordered_set<QColor> > annotationTextColor();
  void setAnnotationAlignment(int value);
  std::optional<std::set<RKTextAlignment> > annotationAlignment();
  void setAnnotationStyle(int value);
  std::optional<std::set<RKTextStyle> > annotationStyle();
  void setAnnotationScaling(double value);
  std::optional<std::unordered_set<double> > annotationScaling();
  void setAnnotationOffsetX(double value);
  std::optional<std::unordered_set<double>> annotationOffsetX();
  void setAnnotationOffsetY(double value);
  std::optional<std::unordered_set<double>> annotationOffsetY();
  void setAnnotationOffsetZ(double value);
  std::optional<std::unordered_set<double> > annotationOffsetZ();

private slots:
  void expandPrimitiveItem();
  void expandAtomsItem();
  void expandBondsItem();
  void expandUnitCellItem();
  void expandLocalAxesItem();
  void expandAdsorptionSurfaceItem();
  void expandAnnotationItem();
signals:
  void rendererReloadData();
  void rendererReloadStructureUniforms();
  void redrawRenderer();
  void redrawWithQuality(RKRenderQuality quality);

  void invalidateCachedAmbientOcclusionTextures(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
  void invalidateCachedIsoSurfaces(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
};

