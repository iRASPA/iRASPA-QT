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
#include "iraspamainwindowconsumerprotocol.h"
#include <simulationkit.h>
#include <optional>
#include "appearanceprimitiveform.h"
#include "appearanceatomsform.h"
#include "appearancebondsform.h"
#include "appearanceunitcellform.h"
#include "appearanceadsorptionsurfaceform.h"
#include "appearanceannotationform.h"

class AppearanceTreeWidgetController: public QTreeWidget, public MainWindowConsumer, public ProjectConsumer, public Reloadable
{
  Q_OBJECT

public:
  AppearanceTreeWidgetController(QWidget* parent = nullptr);
  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override final;
  void setMainWindow(MainWindow *mainWindow) override final {_mainWindow = mainWindow;}
  void setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAStructure>> iraspa_structures);
  void resetData();
private:
  AppearancePrimitiveForm* _appearancePrimitiveForm;
  AppearanceAtomsForm* _appearanceAtomsForm;
  AppearanceBondsForm* _appearanceBondsForm;
  AppearanceUnitCellForm* _appearanceUnitCellForm;
  AppearanceAdsorptionSurfaceForm* _appearanceAdsorptionSurfaceForm;
  AppearanceAnnotationForm* _appearanceAnnotationForm;

  QPushButton* pushButtonPrimitive;
  QPushButton* pushButtonAtoms;
  QPushButton* pushButtonBonds;
  QPushButton* pushButtonUnitCell;
  QPushButton* pushButtonAdsorptionSurface;
  QPushButton* pushButtonAnnotation;

  MainWindow *_mainWindow;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<ProjectStructure> _projectStructure;
  std::vector<std::shared_ptr<iRASPAStructure>> _iraspa_structures{};
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

  std::optional<double> rotationAngle();
  void setRotationAngle(double angle);
  void rotateYawPlus();
  void rotateYawMinus();
  void rotatePitchPlus();
  void rotatePitchMinus();
  void rotateRollPlus();
  void rotateRollMinus();

  void setEulerAngleX(double angle);
  std::optional<double> EulerAngleX();
  void setEulerAngleY(double angle);
  std::optional<double> EulerAngleY();
  void setEulerAngleZ(double angle);
  std::optional<double> EulerAngleZ();


  std::optional<double> transformationMatrixAX();
  void setTransformationMatrixAX(double value);
  std::optional<double> transformationMatrixAY();
  void setTransformationMatrixAY(double value);
  std::optional<double> transformationMatrixAZ();
  void setTransformationMatrixAZ(double value);

  std::optional<double> transformationMatrixBX();
  void setTransformationMatrixBX(double value);
  std::optional<double> transformationMatrixBY();
  void setTransformationMatrixBY(double value);
  std::optional<double> transformationMatrixBZ();
  void setTransformationMatrixBZ(double value);

  std::optional<double> transformationMatrixCX();
  void setTransformationMatrixCX(double value);
  std::optional<double> transformationMatrixCY();
  void setTransformationMatrixCY(double value);
  std::optional<double> transformationMatrixCZ();
  void setTransformationMatrixCZ(double value);

  std::optional<double> primitiveOpacity();
  void setPrimitiveOpacity(double opacity);
  std::optional<int> primitiveNumberOfSides();
  void setPrimitiveNumberOfSides(int numberOfSides);

  std::optional<bool> primitiveIsCapped();
  void setPrimitiveIsCapped(bool capped);

  void setPrimitiveHue(double value);
  std::optional<double> primitiveHue();
  void setPrimitiveSaturation(double value);
  std::optional<double> primitiveSaturation();
  void setPrimitiveValue(double value);
  std::optional<double> primitiveValue();

  void setPrimitiveSelectionStyle(int value);
  std::optional<RKSelectionStyle> primitiveSelectionStyle();
  void setPrimitiveSelectionStyleNu(double value);
  std::optional<double> primitiveSelectionStyleNu();
  void setPrimitiveSelectionStyleRho(double value);
  std::optional<double> primitiveSelectionStyleRho();
  void setPrimitiveSelectionIntensity(double value);
  std::optional<double> primitiveSelectionIntensity();
  void setPrimitiveSelectionScaling(double value);
  std::optional<double> primitiveSelectionScaling();

  void setFrontPrimitiveHighDynamicRange(int value);
  std::optional<bool> frontPrimitiveHighDynamicRange();
  void setFrontPrimitiveHDRExposure(double value);
  std::optional<double> frontPrimitiveHDRExposure();
  void setFrontPrimitiveAmbientLightIntensity(double value);
  std::optional<double> frontPrimitiveAmbientLightIntensity();
  void setFrontPrimitiveAmbientLightColor();
  std::optional<QColor> frontPrimitiveAmbientLightColor();
  void setFrontPrimitiveDiffuseLightIntensity(double value);
  std::optional<double> frontPrimitiveDiffuseLightIntensity();
  void setFrontPrimitiveDiffuseLightColor();
  std::optional<QColor> frontPrimitiveDiffuseLightColor();
  void setFrontPrimitiveSpecularLightIntensity(double value);
  std::optional<double> frontPrimitiveSpecularLightIntensity();
  void setFrontPrimitiveSpecularLightColor();
  std::optional<QColor> frontPrimitiveSpecularLightColor();
  void setFrontPrimitiveShininess(double value);
  std::optional<double> frontPrimitiveShininess();

  void setBackPrimitiveHighDynamicRange(int value);
  std::optional<bool> backPrimitiveHighDynamicRange();
  void setBackPrimitiveHDRExposure(double value);
  std::optional<double> backPrimitiveHDRExposure();
  void setBackPrimitiveAmbientLightIntensity(double value);
  std::optional<double> backPrimitiveAmbientLightIntensity();
  void setBackPrimitiveAmbientLightColor();
  std::optional<QColor> backPrimitiveAmbientLightColor();
  void setBackPrimitiveDiffuseLightIntensity(double value);
  std::optional<double> backPrimitiveDiffuseLightIntensity();
  void setBackPrimitiveDiffuseLightColor();
  std::optional<QColor> backPrimitiveDiffuseLightColor();
  void setBackPrimitiveSpecularLightIntensity(double value);
  std::optional<double> backPrimitiveSpecularLightIntensity();
  void setBackPrimitiveSpecularLightColor();
  std::optional<QColor> backPrimitiveSpecularLightColor();
  void setBackPrimitiveShininess(double value);
  std::optional<double> backPrimitiveShininess();

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
  std::optional<Structure::RepresentationType> representationType();
  void setRepresentationStyle(int value);
  std::optional<Structure::RepresentationStyle> representationStyle();
  void setColorSchemeComboBoxIndex(int value);
  std::optional<QString> colorSchemeIdentifier();
  void setColorSchemeOrder(int value);
  std::optional<SKColorSet::ColorSchemeOrder> colorSchemeOrder();
  void setForcefieldSchemeComboBoxIndex(int value);
  std::optional<QString> forceFieldSchemeIdentifier();
  void setForceFieldSchemeOrder(int value);
  std::optional<ForceFieldSet::ForceFieldSchemeOrder> forceFieldSchemeOrder();
  void setAtomSelectionStyle(int value);
  std::optional<RKSelectionStyle> atomSelectionStyle();
  void setAtomSelectionStyleNu(double value);
  std::optional<double> atomSelectionStyleNu();
  void setAtomSelectionStyleRho(double value);
  std::optional<double> atomSelectionStyleRho();
  void setAtomSelectionIntensity(double value);
  std::optional<double> atomSelectionIntensity();
  void setAtomSelectionScaling(double value);
  std::optional<double> atomSelectionScaling();
  void setAtomDrawAtoms(int state);
  std::optional<bool> atomDrawAtoms();
  void setAtomSizeScalingDoubleSpinBox(double size);
  void setAtomSizeScalingDoubleSliderBegin();
  void setAtomSizeScalingDoubleSliderIntermediate(double size);
  void setAtomSizeScalingDoubleSliderFinal();
  std::optional<double> atomSizeScaling();
  void setAtomHighDynamicRange(int value);
  std::optional<bool> atomHighDynamicRange();
  void setAtomHDRExposure(double value);
  std::optional<double> atomHDRExposure();
  void setAtomHue(double value);
  std::optional<double> atomHue();
  void setAtomSaturation(double value);
  std::optional<double> atomSaturation();
  void setAtomValue(double value);
  std::optional<double> atomValue();
  void setAtomAmbientOcclusion(int value);
  std::optional<bool> atomAmbientOcclusion();
  void setAtomAmbientLightIntensity(double value);
  std::optional<double> atomAmbientLightIntensity();
  void setAtomAmbientLightColor();
  std::optional<QColor> atomAmbientLightColor();
  void setAtomDiffuseLightIntensity(double value);
  std::optional<double> atomDiffuseLightIntensity();
  void setAtomDiffuseLightColor();
  std::optional<QColor> atomDiffuseLightColor();
  void setAtomSpecularLightIntensity(double value);
  std::optional<double> atomSpecularLightIntensity();
  void setAtomSpecularLightColor();
  std::optional<QColor> atomSpecularLightColor();
  void setAtomShininess(double value);
  std::optional<double> atomShininess();

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
  std::optional<bool> bondDrawBonds();
  void setBondSizeScaling(double size);
  std::optional<double> bondSizeScaling();
  void setBondColorMode(int value);
  std::optional<RKBondColorMode> bondColorMode();
  void setBondSelectionStyle(int value);
  std::optional<RKSelectionStyle> bondSelectionStyle();
  void setBondSelectionStyleNu(double value);
  std::optional<double> bondSelectionStyleNu();
  void setBondSelectionStyleRho(double value);
  std::optional<double> bondSelectionStyleRho();
  void setBondSelectionIntensity(double value);
  std::optional<double> bondSelectionIntensity();
  void setBondSelectionScaling(double value);
  std::optional<double> bondSelectionScaling();
  void setBondHighDynamicRange(int value);
  std::optional<bool> bondHighDynamicRange();
  void setBondHDRExposure(double value);
  std::optional<double> bondHDRExposure();
  void setBondHue(double value);
  std::optional<double> bondHue();
  void setBondSaturation(double value);
  std::optional<double> bondSaturation();
  void setBondValue(double value);
  std::optional<double> bondValue();
  void setBondAmbientOcclusion(int value);
  std::optional<bool> bondAmbientOcclusion();
  void setBondAmbientLightIntensity(double value);
  std::optional<double> bondAmbientLightIntensity();
  void setBondAmbientLightColor();
  std::optional<QColor> bondAmbientLightColor();
  void setBondDiffuseLightIntensity(double value);
  std::optional<double> bondDiffuseLightIntensity();
  void setBondDiffuseLightColor();
  std::optional<QColor> bondDiffuseLightColor();
  void setBondSpecularLightIntensity(double value);
  std::optional<double> bondSpecularLightIntensity();
  void setBondSpecularLightColor();
  std::optional<QColor> bondSpecularLightColor();
  void setBondShininess(double value);
  std::optional<double> bondShininessy();

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

  // adsorption surface properties

  void reloadDrawAdsorptionSurfaceCheckBox();
  void reloadAdsorptionSurfaceProbeMoleculePopupBox();
  void reloadAdsorptionSurfaceIsovalue();
  void reloadAdsorptionSurfaceOpacity();
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
  std::optional<bool> drawAdsorptionSurface();
  void setAdsorptionSurfaceProbeMolecule(int value);
  std::optional<Structure::ProbeMolecule> adsorptionSurfaceProbeMolecule();
  void setAdsorptionSurfaceIsovalue(double value);
  std::optional<double> adsorptionSurfaceIsovalue();
  std::optional<double> adsorptionSurfaceMinimumValue();
  void setAdsorptionSurfaceOpacity(double value);
  std::optional<double> adsorptionSurfaceOpacity();

  void setAdsorptionSurfaceHue(double value);
  std::optional<double> adsorptionSurfaceHue();
  void setAdsorptionSurfaceSaturation(double value);
  std::optional<double> adsorptionSurfaceSaturation();
  void setAdsorptionSurfaceValue(double value);
  std::optional<double> adsorptionSurfaceValue();

  void setAdsorptionSurfaceInsideHighDynamicRange(int value);
  std::optional<bool> adsorptionSurfaceInsideHighDynamicRange();
  void setAdsorptionSurfaceInsideHDRExposure(double value);
  std::optional<double> adsorptionSurfaceInsideHDRExposure();
  void setAdsorptionSurfaceInsideAmbientLightIntensity(double value);
  std::optional<double> adsorptionSurfaceInsideAmbientLightIntensity();
  void setAdsorptionSurfaceInsideAmbientLightColor();
  std::optional<QColor> adsorptionSurfaceInsideAmbientLightColor();
  void setAdsorptionSurfaceInsideDiffuseLightIntensity(double value);
  std::optional<double> adsorptionSurfaceInsideDiffuseLightIntensity();
  void setAdsorptionSurfaceInsideDiffuseLightColor();
  std::optional<QColor> adsorptionSurfaceInsideDiffuseLightColor();
  void setAdsorptionSurfaceInsideSpecularLightIntensity(double value);
  std::optional<double> adsorptionSurfaceInsideSpecularLightIntensity();
  void setAdsorptionSurfaceInsideSpecularLightColor();
  std::optional<QColor> adsorptionSurfaceInsideSpecularLightColor();
  void setAdsorptionSurfaceInsideShininess(double value);
  std::optional<double> adsorptionSurfaceInsideShininessy();

  void setAdsorptionSurfaceOutsideHighDynamicRange(int value);
  std::optional<bool> adsorptionSurfaceOutsideHighDynamicRange();
  void setAdsorptionSurfaceOutsideHDRExposure(double value);
  std::optional<double> adsorptionSurfaceOutsideHDRExposure();
  void setAdsorptionSurfaceOutsideAmbientLightIntensity(double value);
  std::optional<double> adsorptionSurfaceOutsideAmbientLightIntensity();
  void setAdsorptionSurfaceOutsideAmbientLightColor();
  std::optional<QColor> adsorptionSurfaceOutsideAmbientLightColor();
  void setAdsorptionSurfaceOutsideDiffuseLightIntensity(double value);
  std::optional<double> adsorptionSurfaceOutsideDiffuseLightIntensity();
  void setAdsorptionSurfaceOutsideDiffuseLightColor();
  std::optional<QColor> adsorptionSurfaceOutsideDiffuseLightColor();
  void setAdsorptionSurfaceOutsideSpecularLightIntensity(double value);
  std::optional<double> adsorptionSurfaceOutsideSpecularLightIntensity();
  void setAdsorptionSurfaceOutsideSpecularLightColor();
  std::optional<QColor> adsorptionSurfaceOutsideSpecularLightColor();
  void setAdsorptionSurfaceOutsideShininess(double value);
  std::optional<double> adsorptionSurfaceOutsideShininessy();


  void reloadAnnotationType();
  void reloadAnnotationFont();
  void reloadAnnotationColor();
  void reloadAnnotationAlignment();
  void reloadAnnotationStyle();
  void reloadAnnotationScaling();
  void reloadAnnotationOffset();
  void setAnnotationType(int value);
  std::optional<RKTextType> annotationType();
  void setAnnotationFontFamily(QString value);
  void setAnnotationFontMember(QString value);
  std::optional<QString> annotationFont();
  void setAnnotationTextColor();
  std::optional<QColor> annotationTextColor();
  void setAnnotationAlignment(int value);
  std::optional<RKTextAlignment> annotationAlignment();
  void setAnnotationStyle(int value);
  std::optional<RKTextStyle> annotationStyle();
  void setAnnotationScaling(double value);
  std::optional<double> annotationScaling();
  void setAnnotationOffsetX(double value);
  std::optional<double> annotationOffsetX();
  void setAnnotationOffsetY(double value);
  std::optional<double> annotationOffsetY();
  void setAnnotationOffsetZ(double value);
  std::optional<double> annotationOffsetZ();

private slots:
  void expandPrimitiveItem();
  void expandAtomsItem();
  void expandBondsItem();
  void expandUnitCellItem();
  void expandAdsorptionSurfaceItem();
  void expandAnnotationItem();
signals:
  void rendererReloadData();
  void redrawRenderer();
  void redrawWithQuality(RKRenderQuality quality);

  void invalidateCachedAmbientOcclusionTextures(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> structures);
  void invalidateCachedIsoSurfaces(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> structures);
};

