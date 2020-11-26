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

#include "appearancetreewidgetcontroller.h"

#include <QObject>
#include <QComboBox>
#include <QColorDialog>
#include <QColor>
#include <memory>
#include "qcolorhash.h"
#include "qdoubleslider.h"
#include <foundationkit.h>

AppearanceTreeWidgetController::AppearanceTreeWidgetController(QWidget* parent): QTreeWidget(parent),
    _appearancePrimitiveForm(new AppearancePrimitiveForm),
    _appearanceAtomsForm(new AppearanceAtomsForm),
    _appearanceBondsForm(new AppearanceBondsForm),
    _appearanceUnitCellForm(new AppearanceUnitCellForm),
    _appearanceAdsorptionSurfaceForm(new AppearanceAdsorptionSurfaceForm),
    _appearanceAnnotationForm(new AppearanceAnnotationForm)
{
  this->setHeaderHidden(true);
  this->setRootIsDecorated(true);
  this->setFrameStyle(QFrame::NoFrame);
  this->setAnimated(true);
  this->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  this->setExpandsOnDoubleClick(false);
  this->setIndentation(0);
  this->setSelectionMode(QAbstractItemView::NoSelection);

  // Primitives
  //=========================================================================
  QTreeWidgetItem* PrimitiveItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(PrimitiveItem);

  pushButtonPrimitive = new QPushButton(QString("Primitive"),this);
  pushButtonPrimitive->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonPrimitive->setStyleSheet("text-align:left;");
  setItemWidget(PrimitiveItem,0,pushButtonPrimitive);

  QTreeWidgetItem *childPrimitiveItem = new QTreeWidgetItem(PrimitiveItem);
  this->setItemWidget(childPrimitiveItem,0, _appearancePrimitiveForm);

  _appearancePrimitiveForm->EulerAngleXDoubleSpinBox->setMinimum(-180);
  _appearancePrimitiveForm->EulerAngleXDoubleSpinBox->setMaximum(180);
  _appearancePrimitiveForm->EulerAngleXDial->setDoubleMinimum(-180);
  _appearancePrimitiveForm->EulerAngleXDial->setDoubleMaximum(180);

  _appearancePrimitiveForm->EulerAngleYDoubleSpinBox->setMinimum(-90.0);
  _appearancePrimitiveForm->EulerAngleYDoubleSpinBox->setMaximum(90.0);
  _appearancePrimitiveForm->EulerAngleYSlider->setDoubleMinimum(-90.0);
  _appearancePrimitiveForm->EulerAngleYSlider->setDoubleMaximum(90.0);

  _appearancePrimitiveForm->EulerAngleZDoubleSpinBox->setMinimum(-180.0);
  _appearancePrimitiveForm->EulerAngleZDoubleSpinBox->setMaximum(180.0);
  _appearancePrimitiveForm->EulerAngleZDial->setDoubleMinimum(-180);
  _appearancePrimitiveForm->EulerAngleZDial->setDoubleMaximum(180);

  _appearancePrimitiveForm->transformationMatrixAXDoubleSpinBox->setMinimum(-1000.0);
  _appearancePrimitiveForm->transformationMatrixAXDoubleSpinBox->setMaximum(1000.0);
  _appearancePrimitiveForm->transformationMatrixAXDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->transformationMatrixAYDoubleSpinBox->setMinimum(-1000.0);
  _appearancePrimitiveForm->transformationMatrixAYDoubleSpinBox->setMaximum(1000.0);
  _appearancePrimitiveForm->transformationMatrixAYDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->transformationMatrixAZDoubleSpinBox->setMinimum(-1000.0);
  _appearancePrimitiveForm->transformationMatrixAZDoubleSpinBox->setMaximum(1000.0);
  _appearancePrimitiveForm->transformationMatrixAZDoubleSpinBox->setSingleStep(0.1);

  _appearancePrimitiveForm->transformationMatrixBXDoubleSpinBox->setMinimum(-1000.0);
  _appearancePrimitiveForm->transformationMatrixBXDoubleSpinBox->setMaximum(1000.0);
  _appearancePrimitiveForm->transformationMatrixBXDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->transformationMatrixBYDoubleSpinBox->setMinimum(-1000.0);
  _appearancePrimitiveForm->transformationMatrixBYDoubleSpinBox->setMaximum(1000.0);
  _appearancePrimitiveForm->transformationMatrixBYDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->transformationMatrixBZDoubleSpinBox->setMinimum(-1000.0);
  _appearancePrimitiveForm->transformationMatrixBZDoubleSpinBox->setMaximum(1000.0);
  _appearancePrimitiveForm->transformationMatrixBZDoubleSpinBox->setSingleStep(0.1);

  _appearancePrimitiveForm->transformationMatrixCXDoubleSpinBox->setMinimum(-1000.0);
  _appearancePrimitiveForm->transformationMatrixCXDoubleSpinBox->setMaximum(1000.0);
  _appearancePrimitiveForm->transformationMatrixCXDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->transformationMatrixCYDoubleSpinBox->setMinimum(-1000.0);
  _appearancePrimitiveForm->transformationMatrixCYDoubleSpinBox->setMaximum(1000.0);
  _appearancePrimitiveForm->transformationMatrixCYDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->transformationMatrixCZDoubleSpinBox->setMinimum(-1000.0);
  _appearancePrimitiveForm->transformationMatrixCZDoubleSpinBox->setMaximum(1000.0);
  _appearancePrimitiveForm->transformationMatrixCZDoubleSpinBox->setSingleStep(0.1);

  _appearancePrimitiveForm->primitiveOpacityDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->primitiveOpacityDoubleSpinBox->setMaximum(1.0);
  _appearancePrimitiveForm->primitiveOpacityDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->primitiveOpacitySlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->primitiveOpacitySlider->setDoubleMaximum(1.0);
  _appearancePrimitiveForm->primitiveNumberOfSidesSpinBox->setMinimum(2);
  _appearancePrimitiveForm->primitiveNumberOfSidesSpinBox->setMaximum(50);
  _appearancePrimitiveForm->primitiveNumberOfSidesSlider->setMinimum(2);
  _appearancePrimitiveForm->primitiveNumberOfSidesSlider->setMaximum(50);

  _appearancePrimitiveForm->primitiveSelectionStyleComboBox->insertItem(0,"None");
  _appearancePrimitiveForm->primitiveSelectionStyleComboBox->insertItem(1,"Worley Noise 3D");
  _appearancePrimitiveForm->primitiveSelectionStyleComboBox->insertItem(2,"Stripes");
  _appearancePrimitiveForm->primitiveSelectionStyleComboBox->insertItem(3,"Glow");


  _appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox->setDecimals(5);
  _appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox->setKeyboardTracking(false);
  _appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _appearancePrimitiveForm->primitiveSelectionItensityDoubleSlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->primitiveSelectionItensityDoubleSlider->setDoubleMaximum(2.0);

  _appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox->setDecimals(5);
  _appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox->setKeyboardTracking(false);
  _appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _appearancePrimitiveForm->primitiveSelectionScalingDoubleSlider->setDoubleMinimum(1.0);
  _appearancePrimitiveForm->primitiveSelectionScalingDoubleSlider->setDoubleMaximum(1.25);

  _appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox->setDecimals(3);
  _appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox->setSingleStep(1.0);
  _appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox->setKeyboardTracking(false);
  _appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox->setDecimals(3);
  _appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox->setSingleStep(0.05);
  _appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox->setKeyboardTracking(false);
  _appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _appearancePrimitiveForm->primitiveHueDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->primitiveHueDoubleSpinBox->setDecimals(5);
  _appearancePrimitiveForm->primitiveHueDoubleSpinBox->setSingleStep(0.05);
  _appearancePrimitiveForm->primitiveHueDoubleSpinBox->setKeyboardTracking(false);
  _appearancePrimitiveForm->primitiveHueDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _appearancePrimitiveForm->primitiveSaturationDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->primitiveSaturationDoubleSpinBox->setDecimals(5);
  _appearancePrimitiveForm->primitiveSaturationDoubleSpinBox->setSingleStep(0.05);
  _appearancePrimitiveForm->primitiveSaturationDoubleSpinBox->setKeyboardTracking(false);
  _appearancePrimitiveForm->primitiveSaturationDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _appearancePrimitiveForm->primitiveValueDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->primitiveValueDoubleSpinBox->setDecimals(5);
  _appearancePrimitiveForm->primitiveValueDoubleSpinBox->setSingleStep(0.05);
  _appearancePrimitiveForm->primitiveValueDoubleSpinBox->setKeyboardTracking(false);
  _appearancePrimitiveForm->primitiveValueDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _appearancePrimitiveForm->primitiveHueDoubleSlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->primitiveHueDoubleSlider->setDoubleMaximum(1.5);
  _appearancePrimitiveForm->primitiveSaturationDoubleSlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->primitiveSaturationDoubleSlider->setDoubleMaximum(1.5);
  _appearancePrimitiveForm->primitiveValueDoubleSlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->primitiveValueDoubleSlider->setDoubleMaximum(1.5);

  _appearancePrimitiveForm->frontExposureDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->frontExposureDoubleSpinBox->setMaximum(3.0);
  _appearancePrimitiveForm->frontExposureDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->frontExposureSlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->frontExposureSlider->setDoubleMaximum(3.0);
  _appearancePrimitiveForm->frontAmbientIntensityDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->frontAmbientIntensityDoubleSpinBox->setMaximum(1.0);
  _appearancePrimitiveForm->frontAmbientIntensityDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->frontAmbientIntensitySlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->frontAmbientIntensitySlider->setDoubleMaximum(1.0);
  _appearancePrimitiveForm->frontDiffuseIntensityDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->frontDiffuseIntensityDoubleSpinBox->setMaximum(1.0);
  _appearancePrimitiveForm->frontDiffuseIntensityDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->frontDiffuseIntensitySlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->frontDiffuseIntensitySlider->setDoubleMaximum(1.0);
  _appearancePrimitiveForm->frontSpecularIntensityDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->frontSpecularIntensityDoubleSpinBox->setMaximum(1.0);
  _appearancePrimitiveForm->frontSpecularIntensityDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->frontSpecularIntensitySlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->frontSpecularIntensitySlider->setDoubleMaximum(1.0);
  _appearancePrimitiveForm->frontShininessDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->frontShininessDoubleSpinBox->setMaximum(128.0);
  _appearancePrimitiveForm->frontShininessDoubleSpinBox->setSingleStep(1.0);
  _appearancePrimitiveForm->frontShininessSlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->frontShininessSlider->setDoubleMaximum(128.0);

  _appearancePrimitiveForm->backExposureDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->backExposureDoubleSpinBox->setMaximum(3.0);
  _appearancePrimitiveForm->backExposureDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->backExposureSlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->backExposureSlider->setDoubleMaximum(3.0);
  _appearancePrimitiveForm->backAmbientIntensityDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->backAmbientIntensityDoubleSpinBox->setMaximum(1.0);
  _appearancePrimitiveForm->backAmbientIntensityDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->backAmbientIntensitySlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->backAmbientIntensitySlider->setDoubleMaximum(1.0);
  _appearancePrimitiveForm->backDiffuseIntensityDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->backDiffuseIntensityDoubleSpinBox->setMaximum(1.0);
  _appearancePrimitiveForm->backDiffuseIntensityDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->backDiffuseIntensitySlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->backDiffuseIntensitySlider->setDoubleMaximum(1.0);
  _appearancePrimitiveForm->backSpecularIntensityDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->backSpecularIntensityDoubleSpinBox->setMaximum(1.0);
  _appearancePrimitiveForm->backSpecularIntensityDoubleSpinBox->setSingleStep(0.1);
  _appearancePrimitiveForm->backSpecularIntensitySlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->backSpecularIntensitySlider->setDoubleMaximum(1.0);
  _appearancePrimitiveForm->backShininessDoubleSpinBox->setMinimum(0.0);
  _appearancePrimitiveForm->backShininessDoubleSpinBox->setMaximum(128.0);
  _appearancePrimitiveForm->backShininessDoubleSpinBox->setSingleStep(1.0);
  _appearancePrimitiveForm->backShininessSlider->setDoubleMinimum(0.0);
  _appearancePrimitiveForm->backShininessSlider->setDoubleMaximum(128.0);


  QObject::connect(_appearancePrimitiveForm->rotationAngleDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setRotationAngle);
  QObject::connect(_appearancePrimitiveForm->rotatePlusXPushButton, &QPushButton::clicked, this, &AppearanceTreeWidgetController::rotateYawPlus);
  QObject::connect(_appearancePrimitiveForm->rotatePlusZPushButton, &QPushButton::clicked, this, &AppearanceTreeWidgetController::rotatePitchPlus);
  QObject::connect(_appearancePrimitiveForm->rotatePlusYPushButton, &QPushButton::clicked, this, &AppearanceTreeWidgetController::rotateRollPlus);
  QObject::connect(_appearancePrimitiveForm->rotateMinusXPushButton, &QPushButton::clicked, this, &AppearanceTreeWidgetController::rotateYawMinus);
  QObject::connect(_appearancePrimitiveForm->rotateMinusZPushButton, &QPushButton::clicked, this, &AppearanceTreeWidgetController::rotatePitchMinus);
  QObject::connect(_appearancePrimitiveForm->rotateMinusYPushButton, &QPushButton::clicked, this, &AppearanceTreeWidgetController::rotateRollMinus);


  QObject::connect(_appearancePrimitiveForm->EulerAngleXDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setEulerAngleX);
  QObject::connect(_appearancePrimitiveForm->EulerAngleYDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setEulerAngleY);
  QObject::connect(_appearancePrimitiveForm->EulerAngleZDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setEulerAngleZ);
  QObject::connect(_appearancePrimitiveForm->EulerAngleXDial,static_cast<void (QDoubleDial::*)(double)>(&QDoubleDial::sliderMoved),this,&AppearanceTreeWidgetController::setEulerAngleX);
  QObject::connect(_appearancePrimitiveForm->EulerAngleYSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setEulerAngleY);
  QObject::connect(_appearancePrimitiveForm->EulerAngleZDial,static_cast<void (QDoubleDial::*)(double)>(&QDoubleDial::sliderMoved),this,&AppearanceTreeWidgetController::setEulerAngleZ);

  QObject::connect(_appearancePrimitiveForm->transformationMatrixAXDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setTransformationMatrixAX);
  QObject::connect(_appearancePrimitiveForm->transformationMatrixAYDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setTransformationMatrixAY);
  QObject::connect(_appearancePrimitiveForm->transformationMatrixAZDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setTransformationMatrixAZ);
  QObject::connect(_appearancePrimitiveForm->transformationMatrixBXDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setTransformationMatrixBX);
  QObject::connect(_appearancePrimitiveForm->transformationMatrixBYDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setTransformationMatrixBY);
  QObject::connect(_appearancePrimitiveForm->transformationMatrixBZDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setTransformationMatrixBZ);
  QObject::connect(_appearancePrimitiveForm->transformationMatrixCXDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setTransformationMatrixCX);
  QObject::connect(_appearancePrimitiveForm->transformationMatrixCYDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setTransformationMatrixCY);
  QObject::connect(_appearancePrimitiveForm->transformationMatrixCZDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AppearanceTreeWidgetController::setTransformationMatrixCZ);

  QObject::connect(_appearancePrimitiveForm->primitiveOpacityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveOpacity);
  QObject::connect(_appearancePrimitiveForm->primitiveOpacitySlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setPrimitiveOpacity);
  QObject::connect(_appearancePrimitiveForm->primitiveNumberOfSidesSpinBox,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveNumberOfSides);
  QObject::connect(_appearancePrimitiveForm->primitiveNumberOfSidesSlider,static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveNumberOfSides);

  QObject::connect(_appearancePrimitiveForm->primitiveCappedCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setPrimitiveIsCapped);

  QObject::connect(_appearancePrimitiveForm->primitiveSelectionStyleComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setPrimitiveSelectionStyle);
  QObject::connect(_appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveSelectionStyleNu);
  QObject::connect(_appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveSelectionStyleRho);
  QObject::connect(_appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveSelectionIntensity);
  QObject::connect(_appearancePrimitiveForm->primitiveSelectionItensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setPrimitiveSelectionIntensity);
  QObject::connect(_appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveSelectionScaling);
  QObject::connect(_appearancePrimitiveForm->primitiveSelectionScalingDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setPrimitiveSelectionScaling);

  QObject::connect(_appearancePrimitiveForm->primitiveHueDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveHue);
  QObject::connect(_appearancePrimitiveForm->primitiveHueDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setPrimitiveHue);
  QObject::connect(_appearancePrimitiveForm->primitiveSaturationDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveSaturation);
  QObject::connect(_appearancePrimitiveForm->primitiveSaturationDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setPrimitiveSaturation);
  QObject::connect(_appearancePrimitiveForm->primitiveValueDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setPrimitiveValue);
  QObject::connect(_appearancePrimitiveForm->primitiveValueDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setPrimitiveValue);


  QObject::connect(_appearancePrimitiveForm->frontHDRCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setFrontPrimitiveHighDynamicRange);
  QObject::connect(_appearancePrimitiveForm->frontExposureDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setFrontPrimitiveHDRExposure);
  QObject::connect(_appearancePrimitiveForm->frontExposureSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setFrontPrimitiveHDRExposure);
  QObject::connect(_appearancePrimitiveForm->frontAmbientIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setFrontPrimitiveAmbientLightIntensity);
  QObject::connect(_appearancePrimitiveForm->frontAmbientIntensitySlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setFrontPrimitiveAmbientLightIntensity);
  QObject::connect(_appearancePrimitiveForm->frontDiffuseIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setFrontPrimitiveDiffuseLightIntensity);
  QObject::connect(_appearancePrimitiveForm->frontDiffuseIntensitySlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setFrontPrimitiveDiffuseLightIntensity);
  QObject::connect(_appearancePrimitiveForm->frontSpecularIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setFrontPrimitiveSpecularLightIntensity);
  QObject::connect(_appearancePrimitiveForm->frontSpecularIntensitySlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setFrontPrimitiveSpecularLightIntensity);
  QObject::connect(_appearancePrimitiveForm->frontShininessDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setFrontPrimitiveShininess);
  QObject::connect(_appearancePrimitiveForm->frontShininessSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setFrontPrimitiveShininess);

  QObject::connect(_appearancePrimitiveForm->frontAmbientColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setFrontPrimitiveAmbientLightColor);
  QObject::connect(_appearancePrimitiveForm->frontDiffuseColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setFrontPrimitiveDiffuseLightColor);
  QObject::connect(_appearancePrimitiveForm->frontSpecularColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setFrontPrimitiveSpecularLightColor);

  QObject::connect(_appearancePrimitiveForm->backHDRCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setBackPrimitiveHighDynamicRange);
  QObject::connect(_appearancePrimitiveForm->backExposureDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBackPrimitiveHDRExposure);
  QObject::connect(_appearancePrimitiveForm->backExposureSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBackPrimitiveHDRExposure);
  QObject::connect(_appearancePrimitiveForm->backAmbientIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBackPrimitiveAmbientLightIntensity);
  QObject::connect(_appearancePrimitiveForm->backAmbientIntensitySlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBackPrimitiveAmbientLightIntensity);
  QObject::connect(_appearancePrimitiveForm->backDiffuseIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBackPrimitiveDiffuseLightIntensity);
  QObject::connect(_appearancePrimitiveForm->backDiffuseIntensitySlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBackPrimitiveDiffuseLightIntensity);
  QObject::connect(_appearancePrimitiveForm->backSpecularIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBackPrimitiveSpecularLightIntensity);
  QObject::connect(_appearancePrimitiveForm->backSpecularIntensitySlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBackPrimitiveSpecularLightIntensity);
  QObject::connect(_appearancePrimitiveForm->backShininessDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBackPrimitiveShininess);
  QObject::connect(_appearancePrimitiveForm->backShininessSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBackPrimitiveShininess);

  QObject::connect(_appearancePrimitiveForm->backAmbientColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setBackPrimitiveAmbientLightColor);
  QObject::connect(_appearancePrimitiveForm->backDiffuseColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setBackPrimitiveDiffuseLightColor);
  QObject::connect(_appearancePrimitiveForm->backSpecularColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setBackPrimitiveSpecularLightColor);


  // Atoms
  //=========================================================================
  QTreeWidgetItem* AtomsItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(AtomsItem);

  pushButtonAtoms = new QPushButton(QString("Atoms"),this);
  pushButtonAtoms->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonAtoms->setStyleSheet("text-align:left;");
  setItemWidget(AtomsItem,0,pushButtonAtoms);

  QTreeWidgetItem *childAtomsItem = new QTreeWidgetItem(AtomsItem);
  this->setItemWidget(childAtomsItem,0, _appearanceAtomsForm);

  _appearanceAtomsForm->atomRepresentationType->insertItem(0,"Ball and stick");
  _appearanceAtomsForm->atomRepresentationType->insertItem(1,"Van der Waals");
  _appearanceAtomsForm->atomRepresentationType->insertItem(2, "Unity");

  _appearanceAtomsForm->atomRepresentationStyle->insertItem(0,"Default");
  _appearanceAtomsForm->atomRepresentationStyle->insertItem(1,"Fancy");
  _appearanceAtomsForm->atomRepresentationStyle->insertItem(2,"Licorice");
  _appearanceAtomsForm->atomRepresentationStyle->insertItem(3,"Objects");

  _appearanceAtomsForm->colorSchemeComboBox->insertItem(0,"Jmol");
  _appearanceAtomsForm->colorSchemeComboBox->insertItem(1,"Rasmol modern");
  _appearanceAtomsForm->colorSchemeComboBox->insertItem(2,"Rasmol");
  _appearanceAtomsForm->colorSchemeComboBox->insertItem(3,"Vesta");

  _appearanceAtomsForm->colorSchemeOrderComboBox->insertItem(0,"Element");
  _appearanceAtomsForm->colorSchemeOrderComboBox->insertItem(1,"Force field first");
  _appearanceAtomsForm->colorSchemeOrderComboBox->insertItem(2,"Force field only");

  _appearanceAtomsForm->forceFieldComboBox->insertItem(0,"Default");

  _appearanceAtomsForm->forceFieldSchemeOrderComboBox->insertItem(0,"Element");
  _appearanceAtomsForm->forceFieldSchemeOrderComboBox->insertItem(1,"Force field first");
  _appearanceAtomsForm->forceFieldSchemeOrderComboBox->insertItem(2,"Force field only");

  _appearanceAtomsForm->atomSelectionStyleComboBox->insertItem(0,"None");
  _appearanceAtomsForm->atomSelectionStyleComboBox->insertItem(1,"Worley Noise 3D");
  _appearanceAtomsForm->atomSelectionStyleComboBox->insertItem(2,"Stripes");
  _appearanceAtomsForm->atomSelectionStyleComboBox->insertItem(3,"Glow");

  _appearanceAtomsForm->atomAtomicSizeScalingSlider->setDoubleMinimum(0.0);
  _appearanceAtomsForm->atomAtomicSizeScalingSlider->setDoubleMaximum(2.0);
  _appearanceAtomsForm->atomHueDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAtomsForm->atomHueDoubleSlider->setDoubleMaximum(2.0);
  _appearanceAtomsForm->atomSaturationDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAtomsForm->atomSaturationDoubleSlider->setDoubleMaximum(2.0);
  _appearanceAtomsForm->atomValueDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAtomsForm->atomValueDoubleSlider->setDoubleMaximum(2.0);

  _appearanceAtomsForm->atomSelectionItensityDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAtomsForm->atomSelectionItensityDoubleSlider->setDoubleMaximum(3.0);

  _appearanceAtomsForm->atomSelectionScalingDoubleSlider->setDoubleMinimum(1.0);
  _appearanceAtomsForm->atomSelectionScalingDoubleSlider->setDoubleMaximum(3.0);

  _appearanceAtomsForm->atomHDRExposureDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAtomsForm->atomHDRExposureDoubleSlider->setDoubleMaximum(3.0);

  _appearanceAtomsForm->atomAmbientIntensityDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAtomsForm->atomAmbientIntensityDoubleSlider->setDoubleMaximum(1.0);
  _appearanceAtomsForm->atomDiffuseIntensityDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAtomsForm->atomDiffuseIntensityDoubleSlider->setDoubleMaximum(1.0);
  _appearanceAtomsForm->atomSpecularIntensityDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAtomsForm->atomSpecularIntensityDoubleSlider->setDoubleMaximum(1.0);
  _appearanceAtomsForm->atomShininessDoubleSlider->setDoubleMinimum(0.1);
  _appearanceAtomsForm->atomShininessDoubleSlider->setDoubleMaximum(128.0);


  QObject::connect(_appearanceAtomsForm->atomRepresentationType,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setRepresentationType);
  QObject::connect(_appearanceAtomsForm->atomRepresentationStyle,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setRepresentationStyle);
  QObject::connect(_appearanceAtomsForm->colorSchemeComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setColorSchemeComboBoxIndex);
  QObject::connect(_appearanceAtomsForm->colorSchemeOrderComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setColorSchemeOrder);
  QObject::connect(_appearanceAtomsForm->forceFieldComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setForcefieldSchemeComboBoxIndex);
  QObject::connect(_appearanceAtomsForm->forceFieldSchemeOrderComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setForceFieldSchemeOrder);

  QObject::connect(_appearanceAtomsForm->atomSelectionStyleComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setAtomSelectionStyle);
  QObject::connect(_appearanceAtomsForm->atomSelectionStyleNuDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomSelectionStyleNu);
  QObject::connect(_appearanceAtomsForm->atomSelectionStyleRhoDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomSelectionStyleRho);
  QObject::connect(_appearanceAtomsForm->atomSelectionItensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomSelectionIntensity);
  QObject::connect(_appearanceAtomsForm->atomSelectionItensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomSelectionIntensity);
  QObject::connect(_appearanceAtomsForm->atomSelectionScalingDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomSelectionScaling);
  QObject::connect(_appearanceAtomsForm->atomSelectionScalingDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomSelectionScaling);

  QObject::connect(_appearanceAtomsForm->atomDrawAtomsCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setAtomDrawAtoms);

  QObject::connect(_appearanceAtomsForm->atomSizeScalingSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomSizeScalingDoubleSpinBox);
  QObject::connect(_appearanceAtomsForm->atomAtomicSizeScalingSlider,static_cast<void (QDoubleSlider::*)(void)>(&QDoubleSlider::sliderPressed),this,&AppearanceTreeWidgetController::setAtomSizeScalingDoubleSliderBegin);
  QObject::connect(_appearanceAtomsForm->atomAtomicSizeScalingSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomSizeScalingDoubleSliderIntermediate);
  QObject::connect(_appearanceAtomsForm->atomAtomicSizeScalingSlider,static_cast<void (QDoubleSlider::*)(void)>(&QDoubleSlider::sliderReleased),this,&AppearanceTreeWidgetController::setAtomSizeScalingDoubleSliderFinal);

  QObject::connect(_appearanceAtomsForm->atomHighDynamicRangeCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setAtomHighDynamicRange);
  QObject::connect(_appearanceAtomsForm->atomHDRExposureDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomHDRExposure);
  QObject::connect(_appearanceAtomsForm->atomHDRExposureDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomHDRExposure);
  QObject::connect(_appearanceAtomsForm->atomHueDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomHue);
  QObject::connect(_appearanceAtomsForm->atomHueDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomHue);
  QObject::connect(_appearanceAtomsForm->atomSaturationDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomSaturation);
  QObject::connect(_appearanceAtomsForm->atomSaturationDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomSaturation);
  QObject::connect(_appearanceAtomsForm->atomValueDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomValue);
  QObject::connect(_appearanceAtomsForm->atomValueDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomValue);


  QObject::connect(_appearanceAtomsForm->atomAmbientOcclusionCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setAtomAmbientOcclusion);
  QObject::connect(_appearanceAtomsForm->atomAmbientIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomAmbientLightIntensity);
  QObject::connect(_appearanceAtomsForm->atomAmbientIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomAmbientLightIntensity);
  QObject::connect(_appearanceAtomsForm->atomDiffuseItensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomDiffuseLightIntensity);
  QObject::connect(_appearanceAtomsForm->atomDiffuseIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomDiffuseLightIntensity);
  QObject::connect(_appearanceAtomsForm->atomSpecularIntensityDoubleSpinBoxBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomSpecularLightIntensity);
  QObject::connect(_appearanceAtomsForm->atomSpecularIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomSpecularLightIntensity);
  QObject::connect(_appearanceAtomsForm->atomShininessDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAtomShininess);
  QObject::connect(_appearanceAtomsForm->atomShininessDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAtomShininess);

  QObject::connect(_appearanceAtomsForm->atomAmbientColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAtomAmbientLightColor);
  QObject::connect(_appearanceAtomsForm->atomDiffuseColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAtomDiffuseLightColor);
  QObject::connect(_appearanceAtomsForm->atomSpecularColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAtomSpecularLightColor);



  // Bonds
  //=========================================================================
  QTreeWidgetItem* BondsItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(BondsItem);

  pushButtonBonds = new QPushButton(QString("Bonds"),this);
  pushButtonBonds->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonBonds->setStyleSheet("text-align:left;");
  setItemWidget(BondsItem,0,pushButtonBonds);

  QTreeWidgetItem *childBondsItem = new QTreeWidgetItem(BondsItem);
  this->setItemWidget(childBondsItem,0, _appearanceBondsForm);

  _appearanceBondsForm->bondColorModeComboBox->insertItem(0,"Uniform");
  _appearanceBondsForm->bondColorModeComboBox->insertItem(1,"Split");
  _appearanceBondsForm->bondColorModeComboBox->insertItem(2,"Gradient");

  _appearanceBondsForm->bondSelectionStyleComboBox->insertItem(0,"None");
  _appearanceBondsForm->bondSelectionStyleComboBox->insertItem(1,"Worley Noise 3D");
  _appearanceBondsForm->bondSelectionStyleComboBox->insertItem(2,"Stripes");
  _appearanceBondsForm->bondSelectionStyleComboBox->insertItem(3,"Glow");


  _appearanceBondsForm->bondAmbientIntensityDoubleSlider->setDoubleMinimum(0.0);
  _appearanceBondsForm->bondAmbientIntensityDoubleSlider->setDoubleMaximum(1.0);
  _appearanceBondsForm->bondDiffuseIntensityDoubleSlider->setDoubleMinimum(0.0);
  _appearanceBondsForm->bondDiffuseIntensityDoubleSlider->setDoubleMaximum(1.0);
  _appearanceBondsForm->bondSpecularIntensityDoubleSlider->setDoubleMinimum(0.0);
  _appearanceBondsForm->bondSpecularIntensityDoubleSlider->setDoubleMaximum(1.0);
  _appearanceBondsForm->bondShininessDoubleSlider->setDoubleMinimum(0.1);
  _appearanceBondsForm->bondShininessDoubleSlider->setDoubleMaximum(128.0);

  _appearanceBondsForm->bondSizeScalingDoubleSlider->setDoubleMinimum(0.0);
  _appearanceBondsForm->bondSizeScalingDoubleSlider->setDoubleMaximum(1.0);

  _appearanceBondsForm->bondSelectionItensityDoubleSlider->setDoubleMinimum(0.0);
  _appearanceBondsForm->bondSelectionItensityDoubleSlider->setDoubleMaximum(3.0);

  _appearanceBondsForm->bondSelectionScalingDoubleSlider->setDoubleMinimum(1.0);
  _appearanceBondsForm->bondSelectionScalingDoubleSlider->setDoubleMaximum(3.0);

  QObject::connect(_appearanceBondsForm->drawBondsCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setBondDrawBonds);
  QObject::connect(_appearanceBondsForm->bondSizeScalingDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondSizeScaling);
  QObject::connect(_appearanceBondsForm->bondSizeScalingDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondSizeScaling);

  QObject::connect(_appearanceBondsForm->bondColorModeComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setBondColorMode);

  QObject::connect(_appearanceBondsForm->bondSelectionStyleComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setBondSelectionStyle);
  QObject::connect(_appearanceBondsForm->bondSelectionStyleNuDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondSelectionStyleNu);
  QObject::connect(_appearanceBondsForm->bondSelectionStyleRhoDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondSelectionStyleRho);
  QObject::connect(_appearanceBondsForm->bondSelectionItensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondSelectionIntensity);
  QObject::connect(_appearanceBondsForm->bondSelectionItensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondSelectionIntensity);
  QObject::connect(_appearanceBondsForm->bondSelectionScalingDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondSelectionScaling);
  QObject::connect(_appearanceBondsForm->bondSelectionScalingDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondSelectionScaling);

  QObject::connect(_appearanceBondsForm->bondHueDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondHue);
  QObject::connect(_appearanceBondsForm->bondHueDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondHue);
  QObject::connect(_appearanceBondsForm->bondSaturationDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondSaturation);
  QObject::connect(_appearanceBondsForm->bondSaturationDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondSaturation);
  QObject::connect(_appearanceBondsForm->bondValueDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondValue);
  QObject::connect(_appearanceBondsForm->bondValueDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondValue);


  QObject::connect(_appearanceBondsForm->bondAmbientOcclusionCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setBondAmbientOcclusion);
  QObject::connect(_appearanceBondsForm->bondAmbientIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondAmbientLightIntensity);
  QObject::connect(_appearanceBondsForm->bondAmbientIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondAmbientLightIntensity);
  QObject::connect(_appearanceBondsForm->bondDiffuseIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondDiffuseLightIntensity);
  QObject::connect(_appearanceBondsForm->bondDiffuseIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondDiffuseLightIntensity);
  QObject::connect(_appearanceBondsForm->bondSpecularIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondSpecularLightIntensity);
  QObject::connect(_appearanceBondsForm->bondSpecularIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondSpecularLightIntensity);
  QObject::connect(_appearanceBondsForm->bondShininessDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setBondShininess);
  QObject::connect(_appearanceBondsForm->bondShininessDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setBondShininess);

  QObject::connect(_appearanceBondsForm->bondAmbientColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setBondAmbientLightColor);
  QObject::connect(_appearanceBondsForm->bondDiffuseColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setBondDiffuseLightColor);
  QObject::connect(_appearanceBondsForm->bondSpecularColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setBondSpecularLightColor);


  // Unit cell
  //=========================================================================
  QTreeWidgetItem* UnitCellItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(UnitCellItem);

  pushButtonUnitCell = new QPushButton(QString("Unit cell"),this);
  pushButtonUnitCell->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonUnitCell->setStyleSheet("text-align:left;");
  setItemWidget(UnitCellItem,0,pushButtonUnitCell);

  QTreeWidgetItem *childUnitCellItem = new QTreeWidgetItem(UnitCellItem);
  this->setItemWidget(childUnitCellItem,0, _appearanceUnitCellForm);

  QObject::connect(_appearanceUnitCellForm->drawUnitCellCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setDrawUnitCell);
  QObject::connect(_appearanceUnitCellForm->unitCellSizeScalingDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setUnitCellSizeScaling);
  QObject::connect(_appearanceUnitCellForm->unitCellSizeScalingDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setUnitCellSizeScaling);
  QObject::connect(_appearanceUnitCellForm->diffuseIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setUnitCellDiffuseLightIntensity);
  QObject::connect(_appearanceUnitCellForm->diffuseIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setUnitCellDiffuseLightIntensity);
  QObject::connect(_appearanceUnitCellForm->diffuseColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setUnitCellDiffuseLightColor);


  // Adsorption surface
  //=========================================================================
  QTreeWidgetItem* AdsorptionSurfaceItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(AdsorptionSurfaceItem);

  pushButtonAdsorptionSurface = new QPushButton(QString("Adsorption surface"),this);
  pushButtonAdsorptionSurface->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonAdsorptionSurface->setStyleSheet("text-align:left;");
  setItemWidget(AdsorptionSurfaceItem,0,pushButtonAdsorptionSurface);

  QTreeWidgetItem *childAdsorptionSurfaceItem = new QTreeWidgetItem(AdsorptionSurfaceItem);
  this->setItemWidget(childAdsorptionSurfaceItem,0, _appearanceAdsorptionSurfaceForm);

  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->insertItem(0, "Helium");
  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->insertItem(1, "Methane");
  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->insertItem(2, "Nitrogen");
  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->insertItem(3, "Hydrogen");
  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->insertItem(4, "Water");
  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->insertItem(5, "CO₂");
  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->insertItem(6, "Xenon");
  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->insertItem(7, "Krypton");
  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->insertItem(8, "Argon");


  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSlider->setDoubleMinimum(-1000.0);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSlider->setDoubleMaximum(1000.0);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSpinBox->setMinimum(-1000.0);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSpinBox->setMaximum(100000.0);

  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox->setMinimum(0.0);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox->setDecimals(5);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox->setSingleStep(0.05);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox->setKeyboardTracking(false);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox->setMinimum(0.0);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox->setDecimals(5);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox->setSingleStep(0.05);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox->setKeyboardTracking(false);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox->setMinimum(0.0);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox->setDecimals(5);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox->setSingleStep(0.05);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox->setKeyboardTracking(false);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSlider->setDoubleMaximum(1.5);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSlider->setDoubleMaximum(1.5);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSlider->setDoubleMinimum(0.0);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSlider->setDoubleMaximum(1.5);

  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setDrawAdsorptionSurface);
  QObject::connect(_appearanceAdsorptionSurfaceForm->probeParticleComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceProbeMolecule);
  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceIsovalue);
  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceIsovalue);
  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOpacity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOpacity);

  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceHue);
  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceHue);
  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceSaturation);
  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceSaturation);
  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceValue);
  QObject::connect(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceValue);

  QObject::connect(_appearanceAdsorptionSurfaceForm->insideHighDynamicRangecheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideHighDynamicRange);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideHDRExposure);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideHDRExposure);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideAmbientLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideAmbientLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideDiffuseLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideDiffuseLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideSpecularLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideSpecularLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideShininessDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideShininess);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideShininessDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideShininess);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideAmbientColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideAmbientLightColor);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideDiffuseColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideDiffuseLightColor);
  QObject::connect(_appearanceAdsorptionSurfaceForm->insideSpecularColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAdsorptionSurfaceInsideSpecularLightColor);

  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideHighDynamicRangecheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideHighDynamicRange);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideHDRExposure);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideHDRExposure);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideAmbientLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideAmbientLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideDiffuseLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideDiffuseLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideSpecularLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideSpecularLightIntensity);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideShininessDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideShininess);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideShininessDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideShininess);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideAmbientColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideAmbientLightColor);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideDiffuseColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideDiffuseLightColor);
  QObject::connect(_appearanceAdsorptionSurfaceForm->outsideSpecularColorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideSpecularLightColor);



  // Annotation
  //=========================================================================
  QTreeWidgetItem* AnnotationItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(AnnotationItem);

  pushButtonAnnotation = new QPushButton(QString("Annotation"),this);
  pushButtonAnnotation->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonAnnotation->setStyleSheet("text-align:left;");
  setItemWidget(AnnotationItem,0,pushButtonAnnotation);

  QTreeWidgetItem *childAnnotationItem = new QTreeWidgetItem(AnnotationItem);
  this->setItemWidget(childAnnotationItem,0, _appearanceAnnotationForm);


  _appearanceAnnotationForm->typeComboBox->insertItem(0, "None");
  _appearanceAnnotationForm->typeComboBox->insertItem(1, "Display name");
  _appearanceAnnotationForm->typeComboBox->insertItem(2, "Identifier");
  _appearanceAnnotationForm->typeComboBox->insertItem(3, "Chemical element");
  _appearanceAnnotationForm->typeComboBox->insertItem(4, "Forcefield type");
  _appearanceAnnotationForm->typeComboBox->insertItem(5, "Position");
  _appearanceAnnotationForm->typeComboBox->insertItem(6, "Charge");


  _appearanceAnnotationForm->alignmentComboBox->insertItem(0, "Center");
  _appearanceAnnotationForm->alignmentComboBox->insertItem(1, "Left");
  _appearanceAnnotationForm->alignmentComboBox->insertItem(2, "Right");
  _appearanceAnnotationForm->alignmentComboBox->insertItem(3, "Top");
  _appearanceAnnotationForm->alignmentComboBox->insertItem(4, "Bottom");
  _appearanceAnnotationForm->alignmentComboBox->insertItem(5, "Top-left");
  _appearanceAnnotationForm->alignmentComboBox->insertItem(6, "Top-right");
  _appearanceAnnotationForm->alignmentComboBox->insertItem(7, "Bottom-left");
  _appearanceAnnotationForm->alignmentComboBox->insertItem(8, "Bottom-right");

  _appearanceAnnotationForm->styleComboBox->insertItem(0, "Flat billboard");

  QFontDatabase database;
  const QStringList fontFamilies = database.families();
  for (const QString &family : fontFamilies)
  {
    _appearanceAnnotationForm->fontComboBox->addItem(family);
  }
  _appearanceAnnotationForm->fontComboBox->setCurrentText("Helvetica");

  const QStringList fontStyles = database.styles("Helvetica");
  for (const QString &style : fontStyles)
  {
    _appearanceAnnotationForm->fontSpecifierComboBox->addItem(style);
  }

  _appearanceAnnotationForm->scalingDoubleSpinBox->setMinimum(0.0);
  _appearanceAnnotationForm->scalingDoubleSpinBox->setMaximum(DBL_MAX);
  _appearanceAnnotationForm->scalingDoubleSpinBox->setDecimals(5);
  _appearanceAnnotationForm->scalingDoubleSpinBox->setSingleStep(0.05);
  _appearanceAnnotationForm->scalingDoubleSpinBox->setKeyboardTracking(false);
  _appearanceAnnotationForm->scalingDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _appearanceAnnotationForm->scalingHorizontalSlider->setDoubleMinimum(0.0);
  _appearanceAnnotationForm->scalingHorizontalSlider->setDoubleMaximum(3.0);

  _appearanceAnnotationForm->offsetXDoubleSpinBox->setMinimum(-DBL_MAX);
  _appearanceAnnotationForm->offsetXDoubleSpinBox->setMaximum(DBL_MAX);
  _appearanceAnnotationForm->offsetXDoubleSpinBox->setDecimals(5);
  _appearanceAnnotationForm->offsetXDoubleSpinBox->setSingleStep(0.05);
  _appearanceAnnotationForm->offsetXDoubleSpinBox->setKeyboardTracking(false);
  _appearanceAnnotationForm->offsetXDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _appearanceAnnotationForm->offsetYDoubleSpinBox->setMinimum(-DBL_MAX);
  _appearanceAnnotationForm->offsetYDoubleSpinBox->setMaximum(DBL_MAX);
  _appearanceAnnotationForm->offsetYDoubleSpinBox->setDecimals(5);
  _appearanceAnnotationForm->offsetYDoubleSpinBox->setSingleStep(0.05);
  _appearanceAnnotationForm->offsetYDoubleSpinBox->setKeyboardTracking(false);
  _appearanceAnnotationForm->offsetYDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _appearanceAnnotationForm->offsetZDoubleSpinBox->setMinimum(-DBL_MAX);
  _appearanceAnnotationForm->offsetZDoubleSpinBox->setMaximum(DBL_MAX);
  _appearanceAnnotationForm->offsetZDoubleSpinBox->setDecimals(5);
  _appearanceAnnotationForm->offsetZDoubleSpinBox->setSingleStep(0.05);
  _appearanceAnnotationForm->offsetZDoubleSpinBox->setKeyboardTracking(false);
  _appearanceAnnotationForm->offsetZDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  QObject::connect(_appearanceAnnotationForm->typeComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setAnnotationType);
  QObject::connect(_appearanceAnnotationForm->fontComboBox,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentTextChanged),this,&AppearanceTreeWidgetController::setAnnotationFontFamily);
  QObject::connect(_appearanceAnnotationForm->fontSpecifierComboBox,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentTextChanged),this,&AppearanceTreeWidgetController::setAnnotationFontMember);


  QObject::connect(_appearanceAnnotationForm->alignmentComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setAnnotationAlignment);
  QObject::connect(_appearanceAnnotationForm->styleComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&AppearanceTreeWidgetController::setAnnotationStyle);

  QObject::connect(_appearanceAnnotationForm->scalingDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAnnotationScaling);
  QObject::connect(_appearanceAnnotationForm->scalingHorizontalSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&AppearanceTreeWidgetController::setAnnotationScaling);

  QObject::connect(_appearanceAnnotationForm->offsetXDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAnnotationOffsetX);
  QObject::connect(_appearanceAnnotationForm->offsetYDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAnnotationOffsetY);
  QObject::connect(_appearanceAnnotationForm->offsetZDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&AppearanceTreeWidgetController::setAnnotationOffsetZ);

  QObject::connect(_appearanceAnnotationForm->colorPushButton,&QPushButton::clicked,this,&AppearanceTreeWidgetController::setAnnotationTextColor);

  // Expanding
  //=========================================================================
  pushButtonPrimitive->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonAtoms->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonBonds->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonUnitCell->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonAdsorptionSurface->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonAnnotation->setFocusPolicy(Qt::FocusPolicy::NoFocus);

  QObject::connect(pushButtonPrimitive, &QPushButton::clicked, this, &AppearanceTreeWidgetController::expandPrimitiveItem);
  QObject::connect(pushButtonAtoms, &QPushButton::clicked, this, &AppearanceTreeWidgetController::expandAtomsItem);
  QObject::connect(pushButtonBonds, &QPushButton::clicked, this, &AppearanceTreeWidgetController::expandBondsItem);
  QObject::connect(pushButtonUnitCell, &QPushButton::clicked, this, &AppearanceTreeWidgetController::expandUnitCellItem);
  QObject::connect(pushButtonAdsorptionSurface, &QPushButton::clicked, this, &AppearanceTreeWidgetController::expandAdsorptionSurfaceItem);
  QObject::connect(pushButtonAnnotation, &QPushButton::clicked, this, &AppearanceTreeWidgetController::expandAnnotationItem);
}


void AppearanceTreeWidgetController::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  _projectTreeNode = projectTreeNode;
  _projectStructure = nullptr;
  _iraspa_structures = std::vector<std::shared_ptr<iRASPAStructure>>{};

  if (projectTreeNode)
  {
    if(std::shared_ptr<iRASPAProject> iraspaProject = projectTreeNode->representedObject())
    {
      if(std::shared_ptr<Project> project = iraspaProject->project())
      {
        if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
        {
          _projectStructure = projectStructure;
          _iraspa_structures = projectStructure->sceneList()->flattenedAllIRASPAStructures();
        }
      }
    }
  }

  reloadData();
}

void AppearanceTreeWidgetController::setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAStructure>> iraspa_structures)
{
  _iraspa_structures = iraspa_structures;
  reloadData();
}

void AppearanceTreeWidgetController::resetData()
{
  reloadData();
}

void AppearanceTreeWidgetController::reloadData()
{
  // refresh the color combobox (new colorsets could have been added)
  whileBlocking(_appearanceAtomsForm->colorSchemeComboBox)->clear();
  if(_mainWindow)
  {
    SKColorSets colorSets = _mainWindow->colorSets();
    for(SKColorSet& colorSet : colorSets.colorSets())
    {
      whileBlocking(_appearanceAtomsForm->colorSchemeComboBox)->addItem(colorSet.displayName());
    }
  }

  // refresh the forcefield combobox (new forcefields could have been added)
  whileBlocking(_appearanceAtomsForm->forceFieldComboBox)->clear();
  if(_mainWindow)
  {
    ForceFieldSets& forceFieldSets = _mainWindow->forceFieldSets();
    for(ForceFieldSet& forceFieldSet : forceFieldSets.forceFieldSets())
    {
      whileBlocking(_appearanceAtomsForm->forceFieldComboBox)->addItem(forceFieldSet.displayName());
    }
  }

  reloadPrimitiveProperties();
  reloadAtomProperties();
  reloadBondProperties();
  reloadUnitCellProperties();
  reloadAdsorptionSurfaceProperties();
  reloadAnnotationProperties();
}

void AppearanceTreeWidgetController::reloadSelection()
{

}

void AppearanceTreeWidgetController::reloadPrimitiveProperties()
{
  reloadRotationAngle();
  reloadEulerAngles();
  reloadTransformationMatrix();
  reloadOpacity();
  reloadNumberOfSides();
  reloadIsCapped();
  reloadPrimitiveSelectionStyle();
  reloadPrimitiveSelectionIntensity();
  reloadPrimitiveSelectionScaling();
  reloadPrimitiveHue();
  reloadPrimitiveSaturation();
  reloadPrimitiveValue();
  reloadFrontPrimitiveHighDynamicRange();
  reloadFrontPrimitiveHDRExposure();
  reloadFrontPrimitiveAmbientLight();
  reloadFrontPrimitiveDiffuseLight();
  reloadFrontPrimitiveSpecularLight();
  reloadFrontPrimitiveShininess();
  reloadBackPrimitiveHighDynamicRange();
  reloadBackPrimitiveHDRExposure();
  reloadBackPrimitiveAmbientLight();
  reloadBackPrimitiveDiffuseLight();
  reloadBackPrimitiveSpecularLight();
  reloadBackPrimitiveShininess();

  _appearancePrimitiveForm->frontAmbientColorPushButton->setDisabled(true);
  _appearancePrimitiveForm->frontDiffuseColorPushButton->setDisabled(true);
  _appearancePrimitiveForm->frontSpecularColorPushButton->setDisabled(true);
  _appearancePrimitiveForm->backAmbientColorPushButton->setDisabled(true);
  _appearancePrimitiveForm->backDiffuseColorPushButton->setDisabled(true);
  _appearancePrimitiveForm->backSpecularColorPushButton->setDisabled(true);
  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->frontAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearancePrimitiveForm->frontDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearancePrimitiveForm->frontSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearancePrimitiveForm->backAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearancePrimitiveForm->backDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearancePrimitiveForm->backSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }

}

void AppearanceTreeWidgetController::reloadRotationAngle()
{
  _appearancePrimitiveForm->rotationAngleDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->rotatePlusXPushButton->setDisabled(true);
  _appearancePrimitiveForm->rotatePlusYPushButton->setDisabled(true);
  _appearancePrimitiveForm->rotatePlusZPushButton->setDisabled(true);
  _appearancePrimitiveForm->rotateMinusXPushButton->setDisabled(true);
  _appearancePrimitiveForm->rotateMinusYPushButton->setDisabled(true);
  _appearancePrimitiveForm->rotateMinusZPushButton->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->rotationAngleDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->rotationAngleDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> angle = rotationAngle())
      {
        whileBlocking(_appearancePrimitiveForm->rotationAngleDoubleSpinBox)->setValue(*angle);

        _appearancePrimitiveForm->rotatePlusXPushButton->setEnabled(_projectTreeNode->isEditable());
        _appearancePrimitiveForm->rotatePlusYPushButton->setEnabled(_projectTreeNode->isEditable());
        _appearancePrimitiveForm->rotatePlusZPushButton->setEnabled(_projectTreeNode->isEditable());
        _appearancePrimitiveForm->rotateMinusXPushButton->setEnabled(_projectTreeNode->isEditable());
        _appearancePrimitiveForm->rotateMinusYPushButton->setEnabled(_projectTreeNode->isEditable());
        _appearancePrimitiveForm->rotateMinusZPushButton->setEnabled(_projectTreeNode->isEditable());

        _appearancePrimitiveForm->rotatePlusXPushButton->setText("Rotate +" + QString::number(*angle));
        _appearancePrimitiveForm->rotatePlusYPushButton->setText("Rotate +" + QString::number(*angle));
        _appearancePrimitiveForm->rotatePlusZPushButton->setText("Rotate +" + QString::number(*angle));
        _appearancePrimitiveForm->rotateMinusXPushButton->setText("Rotate -" + QString::number(*angle));
        _appearancePrimitiveForm->rotateMinusYPushButton->setText("Rotate -" + QString::number(*angle));
        _appearancePrimitiveForm->rotateMinusZPushButton->setText("Rotate -" + QString::number(*angle));
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->rotationAngleDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadEulerAngles()
{
  _appearancePrimitiveForm->EulerAngleXDial->setDisabled(true);
  _appearancePrimitiveForm->EulerAngleYSlider->setDisabled(true);
  _appearancePrimitiveForm->EulerAngleZDial->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->EulerAngleXDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->EulerAngleXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->EulerAngleXDial->setEnabled(_projectTreeNode->isEditable());

      _appearancePrimitiveForm->EulerAngleYDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->EulerAngleYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->EulerAngleYSlider->setEnabled(_projectTreeNode->isEditable());

      _appearancePrimitiveForm->EulerAngleZDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->EulerAngleZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->EulerAngleZDial->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = EulerAngleX())
      {
        whileBlocking(_appearancePrimitiveForm->EulerAngleXDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearancePrimitiveForm->EulerAngleXDial)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->EulerAngleXDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> size = EulerAngleY())
      {
        whileBlocking(_appearancePrimitiveForm->EulerAngleYDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearancePrimitiveForm->EulerAngleYSlider)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->EulerAngleYDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> size = EulerAngleZ())
      {
        whileBlocking(_appearancePrimitiveForm->EulerAngleZDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearancePrimitiveForm->EulerAngleZDial)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->EulerAngleZDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}


void AppearanceTreeWidgetController::reloadTransformationMatrix()
{
  _appearancePrimitiveForm->transformationMatrixAXDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->transformationMatrixAYDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->transformationMatrixAZDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->transformationMatrixBXDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->transformationMatrixBYDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->transformationMatrixBZDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->transformationMatrixCXDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->transformationMatrixCYDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->transformationMatrixCZDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->transformationMatrixAXDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->transformationMatrixAYDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->transformationMatrixAZDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->transformationMatrixBXDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->transformationMatrixBYDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->transformationMatrixBZDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->transformationMatrixCXDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->transformationMatrixCYDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->transformationMatrixCZDoubleSpinBox->setEnabled(true);

      _appearancePrimitiveForm->transformationMatrixAXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->transformationMatrixAYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->transformationMatrixAZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->transformationMatrixBXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->transformationMatrixBYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->transformationMatrixBZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->transformationMatrixCXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->transformationMatrixCYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->transformationMatrixCZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if(!_iraspa_structures.empty())
      {
        if (std::optional<double> value = transformationMatrixAX())
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixAXDoubleSpinBox)->setValue(*value);
        }
        else
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixAXDoubleSpinBox)->setText("Mult. Val.");
        }

        if (std::optional<double> value = transformationMatrixAY())
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixAYDoubleSpinBox)->setValue(*value);
        }
        else
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixAYDoubleSpinBox)->setText("Mult. Val.");
        }

        if (std::optional<double> value = transformationMatrixAZ())
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixAZDoubleSpinBox)->setValue(*value);
        }
        else
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixAZDoubleSpinBox)->setText("Mult. Val.");
        }

        if (std::optional<double> value = transformationMatrixBX())
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixBXDoubleSpinBox)->setValue(*value);
        }
        else
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixBXDoubleSpinBox)->setText("Mult. Val.");
        }

        if (std::optional<double> value = transformationMatrixBY())
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixBYDoubleSpinBox)->setValue(*value);
        }
        else
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixBYDoubleSpinBox)->setText("Mult. Val.");
        }

        if (std::optional<double> value = transformationMatrixBZ())
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixBZDoubleSpinBox)->setValue(*value);
        }
        else
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixBZDoubleSpinBox)->setText("Mult. Val.");
        }

        if (std::optional<double> value = transformationMatrixCX())
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixCXDoubleSpinBox)->setValue(*value);
        }
        else
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixCXDoubleSpinBox)->setText("Mult. Val.");
        }

        if (std::optional<double> value = transformationMatrixCY())
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixCYDoubleSpinBox)->setValue(*value);
        }
        else
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixCYDoubleSpinBox)->setText("Mult. Val.");
        }

        if (std::optional<double> value = transformationMatrixCZ())
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixCZDoubleSpinBox)->setValue(*value);
        }
        else
        {
          whileBlocking(_appearancePrimitiveForm->transformationMatrixCZDoubleSpinBox)->setText("Mult. Val.");
        }
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadOpacity()
{
  _appearancePrimitiveForm->primitiveOpacityDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->primitiveOpacitySlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->primitiveOpacityDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->primitiveOpacityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->primitiveOpacitySlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = primitiveOpacity())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveOpacityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->primitiveOpacitySlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveOpacityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadNumberOfSides()
{
  _appearancePrimitiveForm->primitiveNumberOfSidesSpinBox->setDisabled(true);
  _appearancePrimitiveForm->primitiveNumberOfSidesSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->primitiveNumberOfSidesSpinBox->setEnabled(true);
      _appearancePrimitiveForm->primitiveNumberOfSidesSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->primitiveNumberOfSidesSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<int> value = primitiveNumberOfSides())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveNumberOfSidesSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->primitiveNumberOfSidesSlider)->setValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveNumberOfSidesSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadIsCapped()
{  
  _appearancePrimitiveForm->primitiveCappedCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->primitiveCappedCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = primitiveIsCapped())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveCappedCheckBox)->setTristate(false);
        whileBlocking(_appearancePrimitiveForm->primitiveCappedCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveCappedCheckBox)->setTristate(true);
        whileBlocking(_appearancePrimitiveForm->primitiveCappedCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadPrimitiveSelectionStyle()
{
  _appearancePrimitiveForm->primitiveSelectionStyleComboBox->setDisabled(true);
  _appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->primitiveSelectionStyleComboBox->setEnabled(_projectTreeNode->isEditable());

      _appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<RKSelectionStyle> type = primitiveSelectionStyle())
      {
        if(int index = _appearancePrimitiveForm->primitiveSelectionStyleComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearancePrimitiveForm->primitiveSelectionStyleComboBox)->removeItem(index);
        }
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionStyleComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearancePrimitiveForm->primitiveSelectionStyleComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearancePrimitiveForm->primitiveSelectionStyleComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionStyleComboBox)->setCurrentText("Multiple values");
      }

      if (std::optional<double> size = primitiveSelectionStyleNu())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox)->setValue(*size);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionStyleNuDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> size = primitiveSelectionStyleRho())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox)->setValue(*size);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionStyleRhoDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadPrimitiveSelectionIntensity()
{
  _appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->primitiveSelectionItensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->primitiveSelectionItensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = primitiveSelectionIntensity())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionItensityDoubleSlider)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionItensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadPrimitiveSelectionScaling()
{
  _appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->primitiveSelectionScalingDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->primitiveSelectionScalingDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = primitiveSelectionScaling())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionScalingDoubleSlider)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSelectionScalingDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadPrimitiveHue()
{
  _appearancePrimitiveForm->primitiveHueDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->primitiveHueDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->primitiveHueDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->primitiveHueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->primitiveHueDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = primitiveHue())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveHueDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->primitiveHueDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveHueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadPrimitiveSaturation()
{
  _appearancePrimitiveForm->primitiveSaturationDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->primitiveSaturationDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->primitiveSaturationDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->primitiveSaturationDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->primitiveSaturationDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = primitiveSaturation())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSaturationDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->primitiveSaturationDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveSaturationDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadPrimitiveValue()
{
  _appearancePrimitiveForm->primitiveValueDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->primitiveValueDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->primitiveValueDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->primitiveValueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->primitiveValueDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = primitiveValue())
      {
        whileBlocking(_appearancePrimitiveForm->primitiveValueDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->primitiveValueDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->primitiveValueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadFrontPrimitiveHighDynamicRange()
{
  _appearancePrimitiveForm->frontHDRCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->frontHDRCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = frontPrimitiveHighDynamicRange())
      {
        whileBlocking(_appearancePrimitiveForm->frontHDRCheckBox)->setTristate(false);
        whileBlocking(_appearancePrimitiveForm->frontHDRCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontHDRCheckBox)->setTristate(true);
        whileBlocking(_appearancePrimitiveForm->frontHDRCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadFrontPrimitiveHDRExposure()
{
  _appearancePrimitiveForm->frontExposureDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->frontExposureSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->frontExposureDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->frontExposureDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->frontExposureSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = frontPrimitiveHDRExposure())
      {
        whileBlocking(_appearancePrimitiveForm->frontExposureDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->frontExposureSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontExposureDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}



void AppearanceTreeWidgetController::reloadFrontPrimitiveAmbientLight()
{
  _appearancePrimitiveForm->frontAmbientIntensityDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->frontAmbientIntensitySlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->frontAmbientIntensityDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->frontAmbientIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->frontAmbientIntensitySlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = frontPrimitiveAmbientLightIntensity())
      {
        whileBlocking(_appearancePrimitiveForm->frontAmbientIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->frontAmbientIntensitySlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontAmbientIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = frontPrimitiveAmbientLightColor())
      {
        whileBlocking(_appearancePrimitiveForm->frontAmbientColorPushButton)->setText("color");
        whileBlocking(_appearancePrimitiveForm->frontAmbientColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontAmbientColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearancePrimitiveForm->frontAmbientColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadFrontPrimitiveDiffuseLight()
{
  _appearancePrimitiveForm->frontDiffuseIntensityDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->frontDiffuseIntensitySlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->frontDiffuseIntensityDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->frontDiffuseIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->frontDiffuseIntensitySlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = frontPrimitiveDiffuseLightIntensity())
      {
        whileBlocking(_appearancePrimitiveForm->frontDiffuseIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->frontDiffuseIntensitySlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontDiffuseIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = frontPrimitiveDiffuseLightColor())
      {
        whileBlocking(_appearancePrimitiveForm->frontDiffuseColorPushButton)->setText("color");
        whileBlocking(_appearancePrimitiveForm->frontDiffuseColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontDiffuseColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearancePrimitiveForm->frontDiffuseColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadFrontPrimitiveSpecularLight()
{
  _appearancePrimitiveForm->frontSpecularIntensityDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->frontSpecularIntensitySlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->frontSpecularIntensityDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->frontSpecularIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->frontSpecularIntensitySlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = frontPrimitiveSpecularLightIntensity())
      {
        whileBlocking(_appearancePrimitiveForm->frontSpecularIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->frontSpecularIntensitySlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontSpecularIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = frontPrimitiveSpecularLightColor())
      {
        whileBlocking(_appearancePrimitiveForm->frontSpecularColorPushButton)->setText("color");
        whileBlocking(_appearancePrimitiveForm->frontSpecularColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontSpecularColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearancePrimitiveForm->frontSpecularColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadFrontPrimitiveShininess()
{
  _appearancePrimitiveForm->frontShininessDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->frontShininessSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->frontShininessDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->frontShininessDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->frontShininessSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = frontPrimitiveShininess())
      {
        whileBlocking(_appearancePrimitiveForm->frontShininessDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->frontShininessSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontShininessDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBackPrimitiveHighDynamicRange()
{
  _appearancePrimitiveForm->backHDRCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->backHDRCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = backPrimitiveHighDynamicRange())
      {
        whileBlocking(_appearancePrimitiveForm->backHDRCheckBox)->setTristate(false);
        whileBlocking(_appearancePrimitiveForm->backHDRCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->backHDRCheckBox)->setTristate(true);
        whileBlocking(_appearancePrimitiveForm->backHDRCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBackPrimitiveHDRExposure()
{
  _appearancePrimitiveForm->backExposureDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->backExposureSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->backExposureDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->backExposureDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->backExposureSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = backPrimitiveHDRExposure())
      {
        whileBlocking(_appearancePrimitiveForm->backExposureDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->backExposureSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->backExposureDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}



void AppearanceTreeWidgetController::reloadBackPrimitiveAmbientLight()
{
  _appearancePrimitiveForm->backAmbientIntensityDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->backAmbientIntensitySlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->backAmbientIntensityDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->backAmbientIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->backAmbientIntensitySlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = backPrimitiveAmbientLightIntensity())
      {
        whileBlocking(_appearancePrimitiveForm->backAmbientIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->backAmbientIntensitySlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->backAmbientIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = backPrimitiveAmbientLightColor())
      {
        whileBlocking(_appearancePrimitiveForm->backAmbientColorPushButton)->setText("color");
        whileBlocking(_appearancePrimitiveForm->backAmbientColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->backAmbientColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearancePrimitiveForm->backAmbientColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBackPrimitiveDiffuseLight()
{
  _appearancePrimitiveForm->backDiffuseIntensityDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->backDiffuseIntensitySlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->backDiffuseIntensityDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->backDiffuseIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->backDiffuseIntensitySlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = backPrimitiveDiffuseLightIntensity())
      {
        whileBlocking(_appearancePrimitiveForm->backDiffuseIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->backDiffuseIntensitySlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->backDiffuseIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = backPrimitiveDiffuseLightColor())
      {
        whileBlocking(_appearancePrimitiveForm->backDiffuseColorPushButton)->setText("color");
        whileBlocking(_appearancePrimitiveForm->backDiffuseColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->frontDiffuseColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearancePrimitiveForm->backDiffuseColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBackPrimitiveSpecularLight()
{
  _appearancePrimitiveForm->backSpecularIntensityDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->backSpecularIntensitySlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->backSpecularIntensityDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->backSpecularIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->backSpecularIntensitySlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = backPrimitiveSpecularLightIntensity())
      {
        whileBlocking(_appearancePrimitiveForm->backSpecularIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->backSpecularIntensitySlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->backSpecularIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = backPrimitiveSpecularLightColor())
      {
        whileBlocking(_appearancePrimitiveForm->backSpecularColorPushButton)->setText("color");
        whileBlocking(_appearancePrimitiveForm->backSpecularColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->backSpecularColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearancePrimitiveForm->backSpecularColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBackPrimitiveShininess()
{
  _appearancePrimitiveForm->backShininessDoubleSpinBox->setDisabled(true);
  _appearancePrimitiveForm->backShininessSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearancePrimitiveForm->backShininessDoubleSpinBox->setEnabled(true);
      _appearancePrimitiveForm->backShininessDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearancePrimitiveForm->backShininessSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = backPrimitiveShininess())
      {
        whileBlocking(_appearancePrimitiveForm->backShininessDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearancePrimitiveForm->backShininessSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearancePrimitiveForm->backShininessDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}



void AppearanceTreeWidgetController::setRotationAngle(double angle)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setPrimitiveRotationDelta(angle);
    }
    reloadRotationAngle();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::rotationAngle()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->primitiveRotationDelta());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::rotateYawPlus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->primitiveRotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
      simd_quatd dq = simd_quatd::yaw(rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setPrimitiveOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void AppearanceTreeWidgetController::rotateYawMinus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->primitiveRotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
      simd_quatd dq = simd_quatd::yaw(-rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setPrimitiveOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void AppearanceTreeWidgetController::rotatePitchPlus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->primitiveRotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
      simd_quatd dq = simd_quatd::pitch(rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setPrimitiveOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void AppearanceTreeWidgetController::rotatePitchMinus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->primitiveRotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
      simd_quatd dq = simd_quatd::pitch(-rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setPrimitiveOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void AppearanceTreeWidgetController::rotateRollPlus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->primitiveRotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
      simd_quatd dq = simd_quatd::roll(rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setPrimitiveOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void AppearanceTreeWidgetController::rotateRollMinus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->primitiveRotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
      simd_quatd dq = simd_quatd::roll(-rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setPrimitiveOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void AppearanceTreeWidgetController::setEulerAngleX(double angle)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
      double3 EulerAngles = orientation.EulerAngles();
      EulerAngles.x = angle * M_PI / 180.0;
      simd_quatd newOrientation = simd_quatd(EulerAngles);
      iraspa_structure->structure()->setPrimitiveOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::EulerAngleX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }

  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
    double3 EulerAngle = orientation.EulerAngles();
    set.insert(EulerAngle.x  * 180.0 / M_PI);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setEulerAngleY(double angle)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
      double3 EulerAngles = orientation.EulerAngles();
      EulerAngles.y = angle * M_PI / 180.0;
      simd_quatd newOrientation = simd_quatd(EulerAngles);
      iraspa_structure->structure()->setPrimitiveOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::EulerAngleY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }

  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
    double3 EulerAngle = orientation.EulerAngles();
    set.insert(EulerAngle.y * 180.0 / M_PI);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}


void AppearanceTreeWidgetController::setEulerAngleZ(double angle)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
      double3 EulerAngles = orientation.EulerAngles();
      EulerAngles.z = angle * M_PI / 180.0;
      simd_quatd newOrientation = simd_quatd(EulerAngles);
      iraspa_structure->structure()->setPrimitiveOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::EulerAngleZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }

  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    simd_quatd orientation = iraspa_structure->structure()->primitiveOrientation();
    double3 EulerAngle = orientation.EulerAngles();
    set.insert(EulerAngle.z * 180.0 / M_PI);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

std::optional<double> AppearanceTreeWidgetController::transformationMatrixAX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->transformationMatrix().ax);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setTransformationMatrixAX(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double3x3 matrix = iraspa_structure->structure()->transformationMatrix();
      matrix.ax = value;
      iraspa_structure->structure()->setTransformationMatrix(matrix);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    //emit invalidateCachedAmbientOcclusionTexture(render_structures);
    emit rendererReloadData();

    reloadTransformationMatrix();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::transformationMatrixAY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->transformationMatrix().ay);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setTransformationMatrixAY(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double3x3 matrix = iraspa_structure->structure()->transformationMatrix();
      matrix.ay = value;
      iraspa_structure->structure()->setTransformationMatrix(matrix);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    //emit invalidateCachedAmbientOcclusionTexture(render_structures);
    emit rendererReloadData();

    reloadTransformationMatrix();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::transformationMatrixAZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->transformationMatrix().az);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setTransformationMatrixAZ(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double3x3 matrix = iraspa_structure->structure()->transformationMatrix();
      matrix.az = value;
      iraspa_structure->structure()->setTransformationMatrix(matrix);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    //emit invalidateCachedAmbientOcclusionTexture(render_structures);
    emit rendererReloadData();

    reloadTransformationMatrix();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::transformationMatrixBX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->transformationMatrix().bx);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setTransformationMatrixBX(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double3x3 matrix = iraspa_structure->structure()->transformationMatrix();
      matrix.bx = value;
      iraspa_structure->structure()->setTransformationMatrix(matrix);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    //emit invalidateCachedAmbientOcclusionTexture(render_structures);
    emit rendererReloadData();

    reloadTransformationMatrix();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::transformationMatrixBY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->transformationMatrix().by);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setTransformationMatrixBY(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double3x3 matrix = iraspa_structure->structure()->transformationMatrix();
      matrix.by = value;
      iraspa_structure->structure()->setTransformationMatrix(matrix);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    //emit invalidateCachedAmbientOcclusionTexture(render_structures);
    emit rendererReloadData();

    reloadTransformationMatrix();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::transformationMatrixBZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->transformationMatrix().bz);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setTransformationMatrixBZ(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double3x3 matrix = iraspa_structure->structure()->transformationMatrix();
      matrix.bz = value;
      iraspa_structure->structure()->setTransformationMatrix(matrix);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    //emit invalidateCachedAmbientOcclusionTexture(render_structures);
    emit rendererReloadData();

    reloadTransformationMatrix();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::transformationMatrixCX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->transformationMatrix().cx);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setTransformationMatrixCX(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double3x3 matrix = iraspa_structure->structure()->transformationMatrix();
      matrix.cx = value;
      iraspa_structure->structure()->setTransformationMatrix(matrix);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    //emit invalidateCachedAmbientOcclusionTexture(render_structures);
    emit rendererReloadData();

    reloadTransformationMatrix();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::transformationMatrixCY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->transformationMatrix().cy);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setTransformationMatrixCY(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double3x3 matrix = iraspa_structure->structure()->transformationMatrix();
      matrix.cy = value;
      iraspa_structure->structure()->setTransformationMatrix(matrix);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    //emit invalidateCachedAmbientOcclusionTexture(render_structures);
    emit rendererReloadData();

    reloadTransformationMatrix();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::transformationMatrixCZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->transformationMatrix().cz);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setTransformationMatrixCZ(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double3x3 matrix = iraspa_structure->structure()->transformationMatrix();
      matrix.cz = value;
      iraspa_structure->structure()->setTransformationMatrix(matrix);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    //emit invalidateCachedAmbientOcclusionTexture(render_structures);
    emit rendererReloadData();

    reloadTransformationMatrix();

    _mainWindow->documentWasModified();
  }
}


std::optional<double> AppearanceTreeWidgetController::primitiveOpacity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->primitiveOpacity());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setPrimitiveOpacity(double opacity)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setPrimitiveOpacity(opacity);
    }

    emit rendererReloadData();

    reloadOpacity();

    _mainWindow->documentWasModified();
  }
}
std::optional<int> AppearanceTreeWidgetController::primitiveNumberOfSides()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->primitiveNumberOfSides());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setPrimitiveNumberOfSides(int numberOfSides)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setPrimitiveNumberOfSides(numberOfSides);
    }

    emit rendererReloadData();

    reloadNumberOfSides();

    _mainWindow->documentWasModified();
  }
}




std::optional<bool> AppearanceTreeWidgetController::primitiveIsCapped()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->primitiveIsCapped();
    set.insert(value);
  }
   if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setPrimitiveIsCapped(bool state)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setPrimitiveIsCapped(bool(state));
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}


void AppearanceTreeWidgetController::setPrimitiveSelectionStyle(int value)
{
  if(value>=0 && value<int(RKSelectionStyle::multiple_values))
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setPrimitiveSelectionStyle(RKSelectionStyle(value));
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadPrimitiveProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<RKSelectionStyle> AppearanceTreeWidgetController::primitiveSelectionStyle()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<RKSelectionStyle, enum_hash> set = std::unordered_set<RKSelectionStyle, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    RKSelectionStyle value = iraspa_structure->structure()->primitiveSelectionStyle();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setPrimitiveSelectionStyleNu(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setPrimitiveSelectionFrequency(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::primitiveSelectionStyleNu()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->primitiveSelectionFrequency();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setPrimitiveSelectionStyleRho(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setPrimitiveSelectionDensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::primitiveSelectionStyleRho()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->primitiveSelectionDensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setPrimitiveSelectionIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setPrimitiveSelectionIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::primitiveSelectionIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->primitiveSelectionIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setPrimitiveSelectionScaling(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setPrimitiveSelectionScaling(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::primitiveSelectionScaling()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->primitiveSelectionScaling();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


void AppearanceTreeWidgetController::setPrimitiveHue(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setPrimitiveHue(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::primitiveHue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->primitiveHue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setPrimitiveSaturation(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setPrimitiveSaturation(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::primitiveSaturation()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->primitiveSaturation();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setPrimitiveValue(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setPrimitiveValue(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::primitiveValue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->primitiveValue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}




void AppearanceTreeWidgetController::setFrontPrimitiveHighDynamicRange(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setFrontPrimitiveHDR(bool(value));
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::frontPrimitiveHighDynamicRange()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->frontPrimitiveHDR();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setFrontPrimitiveHDRExposure(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setFrontPrimitiveHDRExposure(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::frontPrimitiveHDRExposure()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->frontPrimitiveHDRExposure();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setFrontPrimitiveAmbientLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setFrontPrimitiveAmbientIntensity(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::frontPrimitiveAmbientLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->frontPrimitiveAmbientIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setFrontPrimitiveAmbientLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomAmbientColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setFrontPrimitiveAmbientColor(color);
    }
    reloadPrimitiveProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::frontPrimitiveAmbientLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->frontPrimitiveAmbientColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setFrontPrimitiveDiffuseLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setFrontPrimitiveDiffuseIntensity(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::frontPrimitiveDiffuseLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->frontPrimitiveDiffuseIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setFrontPrimitiveDiffuseLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomDiffuseColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setFrontPrimitiveDiffuseColor(color);
    }
    reloadPrimitiveProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::frontPrimitiveDiffuseLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->frontPrimitiveDiffuseColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setFrontPrimitiveSpecularLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setFrontPrimitiveSpecularIntensity(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::frontPrimitiveSpecularLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->frontPrimitiveSpecularIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setFrontPrimitiveSpecularLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomSpecularColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setFrontPrimitiveSpecularColor(color);
    }
    reloadPrimitiveProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::frontPrimitiveSpecularLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->frontPrimitiveSpecularColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setFrontPrimitiveShininess(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setFrontPrimitiveShininess(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::frontPrimitiveShininess()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->frontPrimitiveShininess();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


void AppearanceTreeWidgetController::setBackPrimitiveHighDynamicRange(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBackPrimitiveHDR(bool(value));
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::backPrimitiveHighDynamicRange()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->backPrimitiveHDR();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBackPrimitiveHDRExposure(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBackPrimitiveHDRExposure(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::backPrimitiveHDRExposure()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->backPrimitiveHDRExposure();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBackPrimitiveAmbientLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBackPrimitiveAmbientIntensity(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::backPrimitiveAmbientLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->backPrimitiveAmbientIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBackPrimitiveAmbientLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomAmbientColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setBackPrimitiveAmbientColor(color);
    }
    reloadPrimitiveProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::backPrimitiveAmbientLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->backPrimitiveAmbientColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBackPrimitiveDiffuseLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBackPrimitiveDiffuseIntensity(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::backPrimitiveDiffuseLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->backPrimitiveDiffuseIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBackPrimitiveDiffuseLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomDiffuseColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setBackPrimitiveDiffuseColor(color);
    }
    reloadPrimitiveProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::backPrimitiveDiffuseLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->backPrimitiveDiffuseColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBackPrimitiveSpecularLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBackPrimitiveSpecularIntensity(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::backPrimitiveSpecularLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->backPrimitiveSpecularIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBackPrimitiveSpecularLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomSpecularColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setBackPrimitiveSpecularColor(color);
    }
    reloadPrimitiveProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::backPrimitiveSpecularLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->backPrimitiveSpecularColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBackPrimitiveShininess(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBackPrimitiveShininess(value);
  }
  reloadPrimitiveProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::backPrimitiveShininess()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->backPrimitiveShininess();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


// reload atom properties
//========================================================================================================================================

void AppearanceTreeWidgetController::reloadAtomProperties()
{
  reloadAtomRepresentationType();
  reloadAtomRepresentationStyle();
  reloadAtomColorSet();
  reloadAtomColorSetOrder();
  reloadAtomForceFieldSet();
  reloadAtomForceFieldSetOrder();
  reloadAtomDrawAtoms();
  reloadAtomSelectionStyle();
  reloadAtomSelectionIntensity();
  reloadAtomSelectionScaling();
  reloadAtomSizeScalingDoubleSpinBox();
  reloadAtomSizeScalingDoubleSlider();
  reloadAtomHighDynamicRange();
  reloadAtomHDRExposure();
  reloadAtomHue();
  reloadAtomSaturation();
  reloadAtomValue();
  reloadAtomAmbientOcclusion();
  reloadAtomAmbientLight();
  reloadAtomDiffuseLight();
  reloadAtomSpecularLight();
  reloadAtomShininess();

  _appearanceAtomsForm->atomAmbientColorPushButton->setDisabled(true);
  _appearanceAtomsForm->atomDiffuseColorPushButton->setDisabled(true);
  _appearanceAtomsForm->atomSpecularColorPushButton->setDisabled(true);
  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

// reload atom properties
//========================================================================================================================================

void AppearanceTreeWidgetController::reloadAtomRepresentationType()
{
  _appearanceAtomsForm->atomRepresentationType->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomRepresentationType->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::RepresentationType> type=representationType())
      {
        if(int index = _appearanceAtomsForm->atomRepresentationType->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAtomsForm->atomRepresentationType)->removeItem(index);
        }
        whileBlocking(_appearanceAtomsForm->atomRepresentationType)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearanceAtomsForm->atomRepresentationType->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAtomsForm->atomRepresentationType)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAtomsForm->atomRepresentationType)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomRepresentationStyle()
{
  _appearanceAtomsForm->atomRepresentationStyle->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomRepresentationStyle->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::RepresentationStyle> type=representationStyle())
      {
        if(int index = _appearanceAtomsForm->atomRepresentationStyle->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAtomsForm->atomRepresentationStyle)->removeItem(index);
        }
        if(int index = _appearanceAtomsForm->atomRepresentationStyle->findText("Custom"); index>=0)
        {
          whileBlocking(_appearanceAtomsForm->atomRepresentationStyle)->removeItem(index);
        }
        if(int(*type)<0)
        {
           whileBlocking(_appearanceAtomsForm->atomRepresentationStyle)->insertItem(int(Structure::RepresentationStyle::multiple_values),"Custom");
           whileBlocking(_appearanceAtomsForm->atomRepresentationStyle)->setCurrentIndex(int(Structure::RepresentationStyle::multiple_values));
        }
        else
        {
          whileBlocking(_appearanceAtomsForm->atomRepresentationStyle)->setCurrentIndex(int(*type));
        }
      }
      else
      {
        if(int index = _appearanceAtomsForm->atomRepresentationStyle->findText("Custom"); index>=0)
        {
          whileBlocking(_appearanceAtomsForm->atomRepresentationStyle)->removeItem(index);
        }
        if(int index = _appearanceAtomsForm->atomRepresentationStyle->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAtomsForm->atomRepresentationStyle)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAtomsForm->atomRepresentationStyle)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomColorSet()
{
  _appearanceAtomsForm->colorSchemeComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->colorSchemeComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> colorSchemName = colorSchemeIdentifier())
      {
        if(int index = _appearanceAtomsForm->colorSchemeComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAtomsForm->colorSchemeComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAtomsForm->colorSchemeComboBox)->setCurrentText(*colorSchemName);
      }
      else
      {
        if(int index = _appearanceAtomsForm->colorSchemeComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAtomsForm->colorSchemeComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAtomsForm->colorSchemeComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomColorSetOrder()
{
  _appearanceAtomsForm->colorSchemeOrderComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->colorSchemeOrderComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<SKColorSet::ColorSchemeOrder> type = colorSchemeOrder())
      {
        if(int index = _appearanceAtomsForm->colorSchemeOrderComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAtomsForm->colorSchemeOrderComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAtomsForm->colorSchemeOrderComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearanceAtomsForm->colorSchemeOrderComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAtomsForm->colorSchemeOrderComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAtomsForm->colorSchemeOrderComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomForceFieldSet()
{
  _appearanceAtomsForm->forceFieldComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->forceFieldComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> forceFieldIdentifier = forceFieldSchemeIdentifier())
      {
        if(int index = _appearanceAtomsForm->forceFieldComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAtomsForm->forceFieldComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAtomsForm->forceFieldComboBox)->setCurrentText(*forceFieldIdentifier);
      }
      else
      {
        if(int index = _appearanceAtomsForm->forceFieldComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAtomsForm->forceFieldComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAtomsForm->forceFieldComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomForceFieldSetOrder()
{
  _appearanceAtomsForm->forceFieldSchemeOrderComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->forceFieldSchemeOrderComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<ForceFieldSet::ForceFieldSchemeOrder> type = forceFieldSchemeOrder())
      {
        if(int index = _appearanceAtomsForm->forceFieldSchemeOrderComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAtomsForm->forceFieldSchemeOrderComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAtomsForm->forceFieldSchemeOrderComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearanceAtomsForm->forceFieldSchemeOrderComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAtomsForm->forceFieldSchemeOrderComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAtomsForm->forceFieldSchemeOrderComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomDrawAtoms()
{
  _appearanceAtomsForm->atomDrawAtomsCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomDrawAtomsCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = atomDrawAtoms())
      {
        whileBlocking(_appearanceAtomsForm->atomDrawAtomsCheckBox)->setTristate(false);
        whileBlocking(_appearanceAtomsForm->atomDrawAtomsCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomDrawAtomsCheckBox)->setTristate(true);
        whileBlocking(_appearanceAtomsForm->atomDrawAtomsCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomSelectionStyle()
{
  _appearanceAtomsForm->atomSelectionStyleComboBox->setDisabled(true);
  _appearanceAtomsForm->atomSelectionStyleNuDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomSelectionStyleRhoDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomSelectionStyleComboBox->setEnabled(_projectTreeNode->isEditable());

      _appearanceAtomsForm->atomSelectionStyleNuDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomSelectionStyleNuDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomSelectionStyleRhoDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomSelectionStyleRhoDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<RKSelectionStyle> type = atomSelectionStyle())
      {
        if(int index = _appearanceAtomsForm->atomSelectionStyleComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAtomsForm->atomSelectionStyleComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAtomsForm->atomSelectionStyleComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearanceAtomsForm->atomSelectionStyleComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAtomsForm->atomSelectionStyleComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAtomsForm->atomSelectionStyleComboBox)->setCurrentText("Multiple values");
      }

      if (std::optional<double> size = atomSelectionStyleNu())
      {
        whileBlocking(_appearanceAtomsForm->atomSelectionStyleNuDoubleSpinBox)->setValue(*size);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomSelectionStyleNuDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> size = atomSelectionStyleRho())
      {
        whileBlocking(_appearanceAtomsForm->atomSelectionStyleRhoDoubleSpinBox)->setValue(*size);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomSelectionStyleRhoDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomSelectionIntensity()
{
  _appearanceAtomsForm->atomSelectionItensityDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomSelectionItensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomSelectionItensityDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomSelectionItensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomSelectionItensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = atomSelectionIntensity())
      {
        whileBlocking(_appearanceAtomsForm->atomSelectionItensityDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearanceAtomsForm->atomSelectionItensityDoubleSlider)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomSelectionItensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomSelectionScaling()
{
  _appearanceAtomsForm->atomSelectionScalingDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomSelectionScalingDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomSelectionScalingDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomSelectionScalingDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomSelectionScalingDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = atomSelectionScaling())
      {
        whileBlocking(_appearanceAtomsForm->atomSelectionScalingDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearanceAtomsForm->atomSelectionScalingDoubleSlider)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomSelectionScalingDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomSizeScalingDoubleSpinBox()
{
  _appearanceAtomsForm->atomSizeScalingSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomSizeScalingSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomSizeScalingSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> size = atomSizeScaling())
      {
        whileBlocking(_appearanceAtomsForm->atomSizeScalingSpinBox)->setValue(*size);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomSizeScalingSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomSizeScalingDoubleSlider()
{
  _appearanceAtomsForm->atomAtomicSizeScalingSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomAtomicSizeScalingSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = atomSizeScaling())
      {
        whileBlocking(_appearanceAtomsForm->atomAtomicSizeScalingSlider)->setDoubleValue(*size);
      }
    }
  }
}


void AppearanceTreeWidgetController::reloadAtomHighDynamicRange()
{
  _appearanceAtomsForm->atomHighDynamicRangeCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomHighDynamicRangeCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = atomHighDynamicRange())
      {
        whileBlocking(_appearanceAtomsForm->atomHighDynamicRangeCheckBox)->setTristate(false);
        whileBlocking(_appearanceAtomsForm->atomHighDynamicRangeCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomHighDynamicRangeCheckBox)->setTristate(true);
        whileBlocking(_appearanceAtomsForm->atomHighDynamicRangeCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomHDRExposure()
{
  _appearanceAtomsForm->atomHDRExposureDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomHDRExposureDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomHDRExposureDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomHDRExposureDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomHDRExposureDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = atomHDRExposure())
      {
        whileBlocking(_appearanceAtomsForm->atomHDRExposureDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAtomsForm->atomHDRExposureDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomHDRExposureDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomHue()
{
  _appearanceAtomsForm->atomHueDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomHueDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomHueDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomHueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomHueDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = atomHue())
      {
        whileBlocking(_appearanceAtomsForm->atomHueDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAtomsForm->atomHueDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomHueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomSaturation()
{
  _appearanceAtomsForm->atomSaturationDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomSaturationDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomSaturationDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomSaturationDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomSaturationDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = atomSaturation())
      {
        whileBlocking(_appearanceAtomsForm->atomSaturationDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAtomsForm->atomSaturationDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomSaturationDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomValue()
{
  _appearanceAtomsForm->atomValueDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomValueDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomValueDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomValueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomValueDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = atomValue())
      {
        whileBlocking(_appearanceAtomsForm->atomValueDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAtomsForm->atomValueDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomValueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomAmbientOcclusion()
{
  _appearanceAtomsForm->atomAmbientOcclusionCheckBox->setEnabled(false);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomAmbientOcclusionCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = atomAmbientOcclusion())
      {
        whileBlocking(_appearanceAtomsForm->atomAmbientOcclusionCheckBox)->setTristate(false);
        whileBlocking(_appearanceAtomsForm->atomAmbientOcclusionCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomAmbientOcclusionCheckBox)->setTristate(true);
        whileBlocking(_appearanceAtomsForm->atomAmbientOcclusionCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomAmbientLight()
{
  _appearanceAtomsForm->atomAmbientIntensityDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomAmbientIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomAmbientIntensityDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomAmbientIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomAmbientIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = atomAmbientLightIntensity())
      {
        whileBlocking(_appearanceAtomsForm->atomAmbientIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAtomsForm->atomAmbientIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomAmbientIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = atomAmbientLightColor())
      {
        whileBlocking(_appearanceAtomsForm->atomAmbientColorPushButton)->setText("color");
        whileBlocking(_appearanceAtomsForm->atomAmbientColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomAmbientColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAtomsForm->atomAmbientColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomDiffuseLight()
{
  _appearanceAtomsForm->atomDiffuseItensityDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomDiffuseIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomDiffuseItensityDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomDiffuseItensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomDiffuseIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = atomDiffuseLightIntensity())
      {
        whileBlocking(_appearanceAtomsForm->atomDiffuseItensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAtomsForm->atomDiffuseIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomDiffuseItensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = atomDiffuseLightColor())
      {
        whileBlocking(_appearanceAtomsForm->atomDiffuseColorPushButton)->setText("color");
        whileBlocking(_appearanceAtomsForm->atomDiffuseColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomDiffuseColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAtomsForm->atomDiffuseColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomSpecularLight()
{
  _appearanceAtomsForm->atomSpecularIntensityDoubleSpinBoxBox->setDisabled(true);
  _appearanceAtomsForm->atomSpecularIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomSpecularIntensityDoubleSpinBoxBox->setEnabled(true);
      _appearanceAtomsForm->atomSpecularIntensityDoubleSpinBoxBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomSpecularIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = atomSpecularLightIntensity())
      {
        whileBlocking(_appearanceAtomsForm->atomSpecularIntensityDoubleSpinBoxBox)->setValue(*value);
        whileBlocking(_appearanceAtomsForm->atomSpecularIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomSpecularIntensityDoubleSpinBoxBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = atomSpecularLightColor())
      {
        whileBlocking(_appearanceAtomsForm->atomSpecularColorPushButton)->setText("color");
        whileBlocking(_appearanceAtomsForm->atomSpecularColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomSpecularColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAtomsForm->atomSpecularColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAtomShininess()
{
  _appearanceAtomsForm->atomShininessDoubleSpinBox->setDisabled(true);
  _appearanceAtomsForm->atomShininessDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAtomsForm->atomShininessDoubleSpinBox->setEnabled(true);
      _appearanceAtomsForm->atomShininessDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAtomsForm->atomShininessDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = atomShininess())
      {
        whileBlocking(_appearanceAtomsForm->atomShininessDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAtomsForm->atomShininessDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAtomsForm->atomShininessDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}



void AppearanceTreeWidgetController::setRepresentationType(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    if(value>=0 && value<int(Structure::RepresentationType::multiple_values))
    {
      iraspa_structure->structure()->setRepresentationType(Structure::RepresentationType(value));
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
  }
  reloadAtomProperties();
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<Structure::RepresentationType> AppearanceTreeWidgetController::representationType()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<Structure::RepresentationType, enum_hash> set = std::unordered_set<Structure::RepresentationType, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    Structure::RepresentationType value = iraspa_structure->structure()->atomRepresentationType();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setRepresentationStyle(int value)
{
  if(value >= 0 && value <= int(Structure::RepresentationStyle::multiple_values))  // also include "Custom"
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setRepresentationStyle(Structure::RepresentationStyle(value), _mainWindow->colorSets());
    }

    reloadAtomProperties();
    reloadBondProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<Structure::RepresentationStyle> AppearanceTreeWidgetController::representationStyle()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<Structure::RepresentationStyle, enum_hash> set = std::unordered_set<Structure::RepresentationStyle, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    Structure::RepresentationStyle value = iraspa_structure->structure()->atomRepresentationStyle();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setColorSchemeComboBoxIndex([[maybe_unused]]  int value)
{
  QString stringValue = _appearanceAtomsForm->colorSchemeComboBox->currentText();
  if(QString::compare(stringValue, "Multiple values") != 0)
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setRepresentationColorSchemeIdentifier(stringValue, _mainWindow->colorSets());
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadAtomProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QString> AppearanceTreeWidgetController::colorSchemeIdentifier()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->atomColorSchemeIdentifier();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setColorSchemeOrder(int value)
{
  if(value>=0 && value<int(SKColorSet::ColorSchemeOrder::multiple_values))
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setColorSchemeOrder(SKColorSet::ColorSchemeOrder(value));
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadAtomProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<SKColorSet::ColorSchemeOrder> AppearanceTreeWidgetController::colorSchemeOrder()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<SKColorSet::ColorSchemeOrder, enum_hash> set = std::unordered_set<SKColorSet::ColorSchemeOrder, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    SKColorSet::ColorSchemeOrder value = iraspa_structure->structure()->colorSchemeOrder();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setForcefieldSchemeComboBoxIndex([[maybe_unused]] int value)
{
  QString stringValue = _appearanceAtomsForm->forceFieldComboBox->currentText();
  if(QString::compare(stringValue, "Multiple values") != 0)
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAtomForceFieldIdentifier(stringValue,_mainWindow->forceFieldSets());
    }
    reloadAtomProperties();

    emit _mainWindow->invalidateCachedIsoSurfaces({_iraspa_structures});
    emit _mainWindow->rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QString> AppearanceTreeWidgetController::forceFieldSchemeIdentifier()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->atomForceFieldIdentifier();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setForceFieldSchemeOrder(int value)
{
  if(value>=0 && value<int(ForceFieldSet::ForceFieldSchemeOrder::multiple_values))
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setForceFieldSchemeOrder(ForceFieldSet::ForceFieldSchemeOrder(value));
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadAtomProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<ForceFieldSet::ForceFieldSchemeOrder> AppearanceTreeWidgetController::forceFieldSchemeOrder()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<ForceFieldSet::ForceFieldSchemeOrder, enum_hash> set = std::unordered_set<ForceFieldSet::ForceFieldSchemeOrder, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    ForceFieldSet::ForceFieldSchemeOrder value = iraspa_structure->structure()->forceFieldSchemeOrder();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomSelectionStyle(int value)
{
  if(value>=0 && value<int(RKSelectionStyle::multiple_values))
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAtomSelectionStyle(RKSelectionStyle(value));
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadAtomProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<RKSelectionStyle> AppearanceTreeWidgetController::atomSelectionStyle()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<RKSelectionStyle, enum_hash> set = std::unordered_set<RKSelectionStyle, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    RKSelectionStyle value = iraspa_structure->structure()->atomSelectionStyle();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomSelectionStyleNu(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomSelectionFrequency(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomSelectionStyleNu()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomSelectionFrequency();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomSelectionStyleRho(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomSelectionDensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomSelectionStyleRho()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomSelectionDensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomSelectionIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setSelectionIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomSelectionIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomSelectionIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomSelectionScaling(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomSelectionScaling(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomSelectionScaling()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomSelectionScaling();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}



void AppearanceTreeWidgetController::setAtomDrawAtoms(int state)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setDrawAtoms(bool(state));
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::atomDrawAtoms()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->drawAtoms();
    set.insert(value);
  }
   if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomSizeScalingDoubleSpinBox(double size)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomScaleFactor(size);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomRepresentationType();
  reloadAtomSizeScalingDoubleSlider();
  reloadAtomRepresentationStyle();

  emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
  emit redrawWithQuality(RKRenderQuality::high);

  _mainWindow->documentWasModified();
}

void AppearanceTreeWidgetController::setAtomSizeScalingDoubleSliderBegin()
{
  emit emit redrawWithQuality(RKRenderQuality::low);
}

void AppearanceTreeWidgetController::setAtomSizeScalingDoubleSliderIntermediate(double size)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomScaleFactor(size);
  }
  reloadAtomSizeScalingDoubleSpinBox();

  emit redrawWithQuality(RKRenderQuality::low);
}

void AppearanceTreeWidgetController::setAtomSizeScalingDoubleSliderFinal()
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomRepresentationStyle();
  reloadAtomSizeScalingDoubleSpinBox();

  emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
  emit rendererReloadData();
  emit redrawWithQuality(RKRenderQuality::high);

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomSizeScaling()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomScaleFactor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


void AppearanceTreeWidgetController::setAtomHighDynamicRange(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomHDR(bool(value));
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::atomHighDynamicRange()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->atomHDR();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomHDRExposure(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomHDRExposure(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomHDRExposure()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomHDRExposure();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomHue(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomHue(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomHue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomHue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomSaturation(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomSaturation(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomSaturation()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomSaturation();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomValue(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomValue(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomValue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomValue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomAmbientOcclusion(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomAmbientOcclusion(bool(value));
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::atomAmbientOcclusion()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->atomAmbientOcclusion();
    set.insert(value);
  }
  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomAmbientLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomAmbientIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomAmbientLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomAmbientIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomAmbientLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomAmbientColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAtomAmbientColor(color);
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadAtomProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::atomAmbientLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->atomAmbientColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomDiffuseLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomDiffuseIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomDiffuseLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomDiffuseIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomDiffuseLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomDiffuseColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAtomDiffuseColor(color);
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadAtomProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::atomDiffuseLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->atomDiffuseColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomSpecularLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomSpecularIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomSpecularLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomSpecularIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomSpecularLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomSpecularColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAtomSpecularColor(color);
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadAtomProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::atomSpecularLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->atomSpecularColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAtomShininess(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAtomShininess(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::atomShininess()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->atomShininess();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

// reload bond properties
//========================================================================================================================================

void AppearanceTreeWidgetController::reloadBondProperties()
{
  reloadDrawBondsCheckBox();
  reloadBondSizeScaling();
  reloadBondColorMode();
  reloadBondSelectionStyle();
  reloadBondSelectionIntensity();
  reloadBondSelectionScaling();
  reloadBondHighDynamicRange();
  reloadBondHDRExposure();
  reloadBondHue();
  reloadBondSaturation();
  reloadBondValue();
  reloadBondAmbientOcclusion();
  reloadBondAmbientLight();
  reloadBondDiffuseLight();
  reloadBondSpecularLight();
  reloadBondShininess();

  _appearanceBondsForm->bondAmbientColorPushButton->setDisabled(true);
  _appearanceBondsForm->bondDiffuseColorPushButton->setDisabled(true);
  _appearanceBondsForm->bondSpecularColorPushButton->setDisabled(true);
  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceBondsForm->bondDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceBondsForm->bondSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void AppearanceTreeWidgetController::reloadDrawBondsCheckBox()
{
  _appearanceBondsForm->drawBondsCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->drawBondsCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = bondDrawBonds())
      {
        whileBlocking(_appearanceBondsForm->drawBondsCheckBox)->setTristate(false);
        whileBlocking(_appearanceBondsForm->drawBondsCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->drawBondsCheckBox)->setTristate(true);
        whileBlocking(_appearanceBondsForm->drawBondsCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondSizeScaling()
{
  _appearanceBondsForm->bondSizeScalingDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondSizeScalingDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondSizeScalingDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondSizeScalingDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondSizeScalingDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = bondSizeScaling())
      {
        whileBlocking(_appearanceBondsForm->bondSizeScalingDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearanceBondsForm->bondSizeScalingDoubleSlider)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondSizeScalingDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}


void AppearanceTreeWidgetController::reloadBondColorMode()
{
  _appearanceBondsForm->bondColorModeComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondColorModeComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<RKBondColorMode> type=bondColorMode())
      {
        if(int index = _appearanceBondsForm->bondColorModeComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceBondsForm->bondColorModeComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceBondsForm->bondColorModeComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearanceBondsForm->bondColorModeComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceBondsForm->bondColorModeComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceBondsForm->bondColorModeComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondSelectionStyle()
{
  _appearanceBondsForm->bondSelectionStyleComboBox->setDisabled(true);
  _appearanceBondsForm->bondSelectionStyleNuDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondSelectionStyleRhoDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondSelectionStyleComboBox->setEnabled(_projectTreeNode->isEditable());

      _appearanceBondsForm->bondSelectionStyleNuDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondSelectionStyleNuDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondSelectionStyleRhoDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondSelectionStyleRhoDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<RKSelectionStyle> type = bondSelectionStyle())
      {
        if(int index = _appearanceBondsForm->bondSelectionStyleComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceBondsForm->bondSelectionStyleComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceBondsForm->bondSelectionStyleComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearanceBondsForm->bondSelectionStyleComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceBondsForm->bondSelectionStyleComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceBondsForm->bondSelectionStyleComboBox)->setCurrentText("Multiple values");
      }

      if (std::optional<double> size = bondSelectionStyleNu())
      {
        whileBlocking(_appearanceBondsForm->bondSelectionStyleNuDoubleSpinBox)->setValue(*size);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondSelectionStyleNuDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> size = bondSelectionStyleRho())
      {
        whileBlocking(_appearanceBondsForm->bondSelectionStyleRhoDoubleSpinBox)->setValue(*size);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondSelectionStyleRhoDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondSelectionIntensity()
{
  _appearanceBondsForm->bondSelectionItensityDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondSelectionItensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondSelectionItensityDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondSelectionItensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondSelectionItensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = bondSelectionIntensity())
      {
        whileBlocking(_appearanceBondsForm->bondSelectionItensityDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearanceBondsForm->bondSelectionItensityDoubleSlider)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondSelectionItensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondSelectionScaling()
{
  _appearanceBondsForm->bondSelectionScalingDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondSelectionScalingDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondSelectionScalingDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondSelectionScalingDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondSelectionScalingDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = bondSelectionScaling())
      {
        whileBlocking(_appearanceBondsForm->bondSelectionScalingDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearanceBondsForm->bondSelectionScalingDoubleSlider)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondSelectionScalingDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}


void AppearanceTreeWidgetController::reloadBondHighDynamicRange()
{
  _appearanceBondsForm->bondHighDynamicRangeCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondHighDynamicRangeCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = bondHighDynamicRange())
      {
        whileBlocking(_appearanceBondsForm->bondHighDynamicRangeCheckBox)->setTristate(false);
        whileBlocking(_appearanceBondsForm->bondHighDynamicRangeCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondHighDynamicRangeCheckBox)->setTristate(true);
        whileBlocking(_appearanceBondsForm->bondHighDynamicRangeCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondHDRExposure()
{
  _appearanceBondsForm->bondHDRExposureDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondHDRExposureDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondHDRExposureDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondHDRExposureDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondHDRExposureDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = bondHDRExposure())
      {
        whileBlocking(_appearanceBondsForm->bondHDRExposureDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceBondsForm->bondHDRExposureDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondHDRExposureDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondHue()
{
  _appearanceBondsForm->bondHueDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondHueDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondHueDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondHueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondHueDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = bondHue())
      {
        whileBlocking(_appearanceBondsForm->bondHueDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceBondsForm->bondHueDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondHueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondSaturation()
{
  _appearanceBondsForm->bondSaturationDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondSaturationDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondSaturationDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondSaturationDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondSaturationDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = bondSaturation())
      {
        whileBlocking(_appearanceBondsForm->bondSaturationDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceBondsForm->bondSaturationDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondSaturationDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondValue()
{
  _appearanceBondsForm->bondValueDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondValueDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondValueDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondValueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondValueDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = bondValue())
      {
        whileBlocking(_appearanceBondsForm->bondValueDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceBondsForm->bondValueDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondValueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondAmbientOcclusion()
{
  _appearanceBondsForm->bondAmbientOcclusionCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondAmbientOcclusionCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = bondAmbientOcclusion())
      {
        whileBlocking(_appearanceBondsForm->bondAmbientOcclusionCheckBox)->setTristate(false);
        whileBlocking(_appearanceBondsForm->bondAmbientOcclusionCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondAmbientOcclusionCheckBox)->setTristate(true);
        whileBlocking(_appearanceBondsForm->bondAmbientOcclusionCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondAmbientLight()
{
  _appearanceBondsForm->bondAmbientIntensityDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondAmbientIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondAmbientIntensityDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondAmbientIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondAmbientIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = bondAmbientLightIntensity())
      {
        whileBlocking(_appearanceBondsForm->bondAmbientIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceBondsForm->bondAmbientIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondAmbientIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = bondAmbientLightColor())
      {
        whileBlocking(_appearanceBondsForm->bondAmbientColorPushButton)->setText("color");
        whileBlocking(_appearanceBondsForm->bondAmbientColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondAmbientColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceBondsForm->bondAmbientColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondDiffuseLight()
{
  _appearanceBondsForm->bondDiffuseIntensityDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondDiffuseIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondDiffuseIntensityDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondDiffuseIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondDiffuseIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = bondDiffuseLightIntensity())
      {
        whileBlocking(_appearanceBondsForm->bondDiffuseIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceBondsForm->bondDiffuseIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondDiffuseIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = bondDiffuseLightColor())
      {
        whileBlocking(_appearanceBondsForm->bondDiffuseColorPushButton)->setText("color");
        whileBlocking(_appearanceBondsForm->bondDiffuseColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondDiffuseColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceBondsForm->bondDiffuseColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondSpecularLight()
{
  _appearanceBondsForm->bondSpecularIntensityDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondSpecularIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondSpecularIntensityDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondSpecularIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondSpecularIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = bondSpecularLightIntensity())
      {
        whileBlocking(_appearanceBondsForm->bondSpecularIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceBondsForm->bondSpecularIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondSpecularIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = bondSpecularLightColor())
      {
        whileBlocking(_appearanceBondsForm->bondSpecularColorPushButton)->setText("color");
        whileBlocking(_appearanceBondsForm->bondSpecularColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondSpecularColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceBondsForm->bondSpecularColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadBondShininess()
{
  _appearanceBondsForm->bondShininessDoubleSpinBox->setDisabled(true);
  _appearanceBondsForm->bondShininessDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceBondsForm->bondShininessDoubleSpinBox->setEnabled(true);
      _appearanceBondsForm->bondShininessDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceBondsForm->bondShininessDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = bondShininessy())
      {
        whileBlocking(_appearanceBondsForm->bondShininessDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceBondsForm->bondShininessDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceBondsForm->bondShininessDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}



void AppearanceTreeWidgetController::setBondDrawBonds(int state)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setDrawBonds(bool(state));
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::bondDrawBonds()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->drawBonds();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondSizeScaling(double size)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondScaleFactor(size);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondSizeScaling()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondScaleFactor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondColorMode(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondColorMode(RKBondColorMode(value));
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<RKBondColorMode> AppearanceTreeWidgetController::bondColorMode()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<RKBondColorMode, enum_hash> set = std::unordered_set<RKBondColorMode, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    RKBondColorMode value = iraspa_structure->structure()->bondColorMode();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondSelectionStyle(int value)
{
  if(value>=0 && value<int(RKSelectionStyle::multiple_values))
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setBondSelectionStyle(RKSelectionStyle(value));
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadBondProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<RKSelectionStyle> AppearanceTreeWidgetController::bondSelectionStyle()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<RKSelectionStyle, enum_hash> set = std::unordered_set<RKSelectionStyle, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    RKSelectionStyle value = iraspa_structure->structure()->bondSelectionStyle();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondSelectionStyleNu(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondSelectionFrequency(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondSelectionStyleNu()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondSelectionFrequency();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondSelectionStyleRho(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondSelectionDensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondSelectionStyleRho()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondSelectionDensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondSelectionIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondSelectionIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondSelectionIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondSelectionIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondSelectionScaling(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondSelectionScaling(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondSelectionScaling()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondSelectionScaling();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}



void AppearanceTreeWidgetController::setBondHighDynamicRange(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondHDR(bool(value));
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::bondHighDynamicRange()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->bondHDR();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondHDRExposure(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondHDRExposure(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondHDRExposure()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondHDRExposure();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondHue(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondHue(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondHue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondHue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondSaturation(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondSaturation(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondSaturation()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondSaturation();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondValue(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondValue(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondValue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondValue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondAmbientOcclusion(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondAmbientOcclusion(bool(value));
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::bondAmbientOcclusion()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->bondAmbientOcclusion();
    set.insert(value);
  }
  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondAmbientLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondAmbientIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondAmbientLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondAmbientIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondAmbientLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceBondsForm->bondAmbientColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setBondAmbientColor(color);
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadBondProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::bondAmbientLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->bondAmbientColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondDiffuseLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondDiffuseIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondDiffuseLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondDiffuseIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondDiffuseLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceBondsForm->bondDiffuseColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setBondDiffuseColor(color);
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadBondProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::bondDiffuseLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->bondDiffuseColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondSpecularLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondSpecularIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondSpecularLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondSpecularIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondSpecularLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceBondsForm->bondSpecularColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setBondSpecularColor(color);
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadBondProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::bondSpecularLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->bondSpecularColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setBondShininess(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setBondShininess(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadBondProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::bondShininessy()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->bondShininess();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


// reload unit cell properties
//========================================================================================================================================

void AppearanceTreeWidgetController::reloadUnitCellProperties()
{
  reloadDrawUnitCell();
  reloadUnitCellSizeScaling();
  reloadUnitCellDiffuseLight();

  _appearanceUnitCellForm->diffuseColorPushButton->setDisabled(true);
  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceUnitCellForm->diffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}



void AppearanceTreeWidgetController::reloadDrawUnitCell()
{
  _appearanceUnitCellForm->drawUnitCellCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceUnitCellForm->drawUnitCellCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = drawUnitCell())
      {
        whileBlocking(_appearanceUnitCellForm->drawUnitCellCheckBox)->setTristate(false);
        whileBlocking(_appearanceUnitCellForm->drawUnitCellCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceUnitCellForm->drawUnitCellCheckBox)->setTristate(true);
        whileBlocking(_appearanceUnitCellForm->drawUnitCellCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadUnitCellSizeScaling()
{
  _appearanceUnitCellForm->unitCellSizeScalingDoubleSpinBox->setDisabled(true);
  _appearanceUnitCellForm->unitCellSizeScalingDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceUnitCellForm->unitCellSizeScalingDoubleSpinBox->setEnabled(true);
      _appearanceUnitCellForm->unitCellSizeScalingDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceUnitCellForm->unitCellSizeScalingDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> size = unitCellSizeScaling())
      {
        whileBlocking(_appearanceUnitCellForm->unitCellSizeScalingDoubleSpinBox)->setValue(*size);
        whileBlocking(_appearanceUnitCellForm->unitCellSizeScalingDoubleSlider)->setDoubleValue(*size);
      }
      else
      {
        whileBlocking(_appearanceUnitCellForm->unitCellSizeScalingDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadUnitCellDiffuseLight()
{
  _appearanceUnitCellForm->diffuseIntensityDoubleSpinBox->setDisabled(true);
  _appearanceUnitCellForm->diffuseIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceUnitCellForm->diffuseIntensityDoubleSpinBox->setEnabled(true);
      _appearanceUnitCellForm->diffuseIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceUnitCellForm->diffuseIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = unitCellDiffuseLightIntensity())
      {
        whileBlocking(_appearanceUnitCellForm->diffuseIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceUnitCellForm->diffuseIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceUnitCellForm->diffuseIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = bondDiffuseLightColor())
      {
        whileBlocking(_appearanceUnitCellForm->diffuseColorPushButton)->setText("color");
        whileBlocking(_appearanceUnitCellForm->diffuseColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceUnitCellForm->diffuseColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceUnitCellForm->diffuseColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}


void AppearanceTreeWidgetController::setDrawUnitCell(int state)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setDrawUnitCell(bool(state));
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::drawUnitCell()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->drawUnitCell();
    set.insert(value);
  }
  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setUnitCellSizeScaling(double size)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setUnitCellScaleFactor(size);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::unitCellSizeScaling()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->unitCellScaleFactor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setUnitCellDiffuseLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setUnitCellDiffuseIntensity(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::unitCellDiffuseLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->unitCellDiffuseIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setUnitCellDiffuseLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceUnitCellForm->diffuseColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setUnitCellDiffuseColor(color);
      iraspa_structure->structure()->recheckRepresentationStyle();
    }
    reloadAtomProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::unitCellDiffuseLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->unitCellDiffuseColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


// reload Adsorption surface properties
//========================================================================================================================================

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceProperties()
{
  reloadDrawAdsorptionSurfaceCheckBox();
  reloadAdsorptionSurfaceProbeMoleculePopupBox();
  reloadAdsorptionSurfaceIsovalue();
  reloadAdsorptionSurfaceOpacity();
  reloadAdsorptionSurfaceHue();
  reloadAdsorptionSurfaceSaturation();
  reloadAdsorptionSurfaceValue();
  reloadAdsorptionSurfaceInsideHighDynamicRange();
  reloadAdsorptionSurfaceInsideHDRExposure();
  reloadAdsorptionSurfaceInsideAmbientLight();
  reloadAdsorptionSurfaceInsideDiffuseLight();
  reloadAdsorptionSurfaceInsideSpecularLight();
  reloadAdsorptionSurfaceInsideShininess();
  reloadAdsorptionSurfaceOutsideHighDynamicRange();
  reloadAdsorptionSurfaceOutsideHDRExposure();
  reloadAdsorptionSurfaceOutsideAmbientLight();
  reloadAdsorptionSurfaceOutsideDiffuseLight();
  reloadAdsorptionSurfaceOutsideSpecularLight();
  reloadAdsorptionSurfaceOutsideShininess();

  _appearanceAdsorptionSurfaceForm->insideAmbientColorPushButton->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->insideDiffuseColorPushButton->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->insideSpecularColorPushButton->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->outsideAmbientColorPushButton->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->outsideDiffuseColorPushButton->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->outsideSpecularColorPushButton->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->insideAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->insideDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->insideSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->outsideAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->outsideDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->outsideSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}


void AppearanceTreeWidgetController::reloadDrawAdsorptionSurfaceCheckBox()
{
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = drawAdsorptionSurface())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceCheckBox)->setTristate(false);
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceCheckBox)->setTristate(true);
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceProbeMoleculePopupBox()
{
  _appearanceAdsorptionSurfaceForm->probeParticleComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->probeParticleComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::ProbeMolecule> type=adsorptionSurfaceProbeMolecule())
      {
        if(int index = _appearanceAdsorptionSurfaceForm->probeParticleComboBox->findText("Mult. Val."); index>=0)
        {
          whileBlocking(_appearanceAdsorptionSurfaceForm->probeParticleComboBox)->removeItem(index);
        }
        if(int(*type)<0)
        {
         whileBlocking(_appearanceAdsorptionSurfaceForm->probeParticleComboBox)->setCurrentIndex(int(Structure::ProbeMolecule::multiple_values));
        }
        else
        {
          whileBlocking(_appearanceAdsorptionSurfaceForm->probeParticleComboBox)->setCurrentIndex(int(*type));
        }
      }
      else
      {
        if(int index = _appearanceAdsorptionSurfaceForm->probeParticleComboBox->findText("Mult. Val."); index<0)
        {
          whileBlocking(_appearanceAdsorptionSurfaceForm->probeParticleComboBox)->addItem("Mult. Val.");
        }
        whileBlocking(_appearanceAdsorptionSurfaceForm->probeParticleComboBox)->setCurrentText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceIsovalue()
{
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceMinimumValue())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSlider)->setDoubleMinimum(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSpinBox)->setMinimum(*value);
      }

      if (std::optional<double> value = adsorptionSurfaceIsovalue())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceIsovalueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceOpacity()
{
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceOpacity())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceOpacityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}


void AppearanceTreeWidgetController::reloadAdsorptionSurfaceHue()
{
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceHue())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceHueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceSaturation()
{
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceSaturation())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceSaturationDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceValue()
{
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceValue())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->adsorptionSurfaceValueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceInsideHighDynamicRange()
{
  _appearanceAdsorptionSurfaceForm->insideHighDynamicRangecheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->insideHighDynamicRangecheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = adsorptionSurfaceInsideHighDynamicRange())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideHighDynamicRangecheckBox)->setTristate(false);
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideHighDynamicRangecheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideHighDynamicRangecheckBox)->setTristate(true);
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideHighDynamicRangecheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceInsideHDRExposure()
{
  _appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceInsideHDRExposure())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideHDRExposureDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceInsideAmbientLight()
{
  _appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceInsideAmbientLightIntensity())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideAmbientLightIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = adsorptionSurfaceInsideAmbientLightColor())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideAmbientColorPushButton)->setText("color");
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideAmbientColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideAmbientColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideAmbientColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceInsideDiffuseLight()
{
  _appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceInsideDiffuseLightIntensity())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideDiffuseLightIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = adsorptionSurfaceInsideDiffuseLightColor())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideDiffuseColorPushButton)->setText("color");
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideDiffuseColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideDiffuseColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideDiffuseColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceInsideSpecularLight()
{
  _appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceInsideSpecularLightIntensity())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideSpecularLightIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = adsorptionSurfaceInsideSpecularLightColor())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideSpecularColorPushButton)->setText("color");
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideSpecularColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideSpecularColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideSpecularColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceInsideShininess()
{
  _appearanceAdsorptionSurfaceForm->insideShininessDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->insideShininessDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->insideShininessDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->insideShininessDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->insideShininessDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceInsideShininessy())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideShininessDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideShininessDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->insideShininessDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceOutsideHighDynamicRange()
{
  _appearanceAdsorptionSurfaceForm->outsideHighDynamicRangecheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->outsideHighDynamicRangecheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = adsorptionSurfaceOutsideHighDynamicRange())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideHighDynamicRangecheckBox)->setTristate(false);
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideHighDynamicRangecheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideHighDynamicRangecheckBox)->setTristate(true);
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideHighDynamicRangecheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceOutsideHDRExposure()
{
  _appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceOutsideHDRExposure())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideHDRExposureDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceOutsideAmbientLight()
{
  _appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceOutsideAmbientLightIntensity())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideAmbientLightIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = adsorptionSurfaceOutsideAmbientLightColor())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideAmbientColorPushButton)->setText("color");
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideAmbientColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideAmbientColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideAmbientColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceOutsideDiffuseLight()
{
  _appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceOutsideDiffuseLightIntensity())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideDiffuseLightIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = adsorptionSurfaceOutsideDiffuseLightColor())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideDiffuseColorPushButton)->setText("color");
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideDiffuseColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideDiffuseColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideDiffuseColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceOutsideSpecularLight()
{
  _appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceOutsideSpecularLightIntensity())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideSpecularLightIntensityDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<QColor> value = adsorptionSurfaceOutsideSpecularLightColor())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideSpecularColorPushButton)->setText("color");
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideSpecularColorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideSpecularColorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideSpecularColorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAdsorptionSurfaceOutsideShininess()
{
  _appearanceAdsorptionSurfaceForm->outsideShininessDoubleSpinBox->setDisabled(true);
  _appearanceAdsorptionSurfaceForm->outsideShininessDoubleSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAdsorptionSurfaceForm->outsideShininessDoubleSpinBox->setEnabled(true);
      _appearanceAdsorptionSurfaceForm->outsideShininessDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAdsorptionSurfaceForm->outsideShininessDoubleSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = adsorptionSurfaceOutsideShininessy())
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideShininessDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideShininessDoubleSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAdsorptionSurfaceForm->outsideShininessDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::setDrawAdsorptionSurface(int state)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setDrawAdsorptionSurface(bool(state));
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAtomProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::drawAdsorptionSurface()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->drawAdsorptionSurface();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceProbeMolecule(int value)
{
  if(value>=0 && value<int(Structure::ProbeMolecule::multiple_values))
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAdsorptionSurfaceProbeMolecule(Structure::ProbeMolecule(value));
      iraspa_structure->structure()->recheckRepresentationStyle();
    }

    emit _mainWindow->invalidateCachedIsoSurfaces({_iraspa_structures});
    emit _mainWindow->rendererReloadData();

    reloadAdsorptionSurfaceIsovalue();

    _mainWindow->documentWasModified();
  }
}

std::optional<Structure::ProbeMolecule> AppearanceTreeWidgetController::adsorptionSurfaceProbeMolecule()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<Structure::ProbeMolecule, enum_hash> set = std::unordered_set<Structure::ProbeMolecule, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    Structure::ProbeMolecule value = iraspa_structure->structure()->adsorptionSurfaceProbeMolecule();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceIsovalue(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceIsoValue(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceIsovalue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceIsoValue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceMinimumValue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceMinimumValue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


void AppearanceTreeWidgetController::setAdsorptionSurfaceOpacity(double value)
{
  if(value>=0 && value<=1.0)
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAdsorptionSurfaceOpacity(value);
    }
    reloadAdsorptionSurfaceProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceOpacity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceOpacity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceHue(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceHue(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceHue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceHue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceSaturation(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceSaturation(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceSaturation()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceSaturation();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceValue(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceValue(value);
    iraspa_structure->structure()->recheckRepresentationStyle();
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceValue()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceValue();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


void AppearanceTreeWidgetController::setAdsorptionSurfaceInsideHighDynamicRange(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceFrontSideHDR(bool(value));
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::adsorptionSurfaceInsideHighDynamicRange()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool state = iraspa_structure->structure()->adsorptionSurfaceFrontSideHDR();
    set.insert(state);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceInsideHDRExposure(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceFrontSideHDRExposure(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceInsideHDRExposure()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceFrontSideHDRExposure();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceInsideAmbientLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceFrontSideAmbientIntensity(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceInsideAmbientLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceFrontSideAmbientIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceInsideAmbientLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAdsorptionSurfaceFrontSideAmbientColor(color);
    }
    reloadAdsorptionSurfaceProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::adsorptionSurfaceInsideAmbientLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor color = iraspa_structure->structure()->adsorptionSurfaceFrontSideAmbientColor();
    set.insert(color);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceInsideDiffuseLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceFrontSideDiffuseIntensity(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceInsideDiffuseLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceFrontSideDiffuseIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceInsideDiffuseLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAdsorptionSurfaceFrontSideDiffuseColor(color);
    }
    reloadAdsorptionSurfaceProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::adsorptionSurfaceInsideDiffuseLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor color = iraspa_structure->structure()->adsorptionSurfaceFrontSideDiffuseColor();
    set.insert(color);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceInsideSpecularLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceFrontSideSpecularIntensity(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceInsideSpecularLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceFrontSideSpecularIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceInsideSpecularLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAdsorptionSurfaceFrontSideSpecularColor(color);
    }
    reloadAdsorptionSurfaceProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::adsorptionSurfaceInsideSpecularLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor color = iraspa_structure->structure()->adsorptionSurfaceFrontSideSpecularColor();
    set.insert(color);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceInsideShininess(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceFrontSideShininess(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceInsideShininessy()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceFrontSideShininess();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideHighDynamicRange(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceBackSideHDR(bool(value));
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> AppearanceTreeWidgetController::adsorptionSurfaceOutsideHighDynamicRange()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->adsorptionSurfaceBackSideHDR();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideHDRExposure(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceBackSideHDRExposure(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceOutsideHDRExposure()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceBackSideHDRExposure();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideAmbientLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceBackSideAmbientIntensity(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceOutsideAmbientLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceBackSideAmbientIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideAmbientLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAdsorptionSurfaceBackSideAmbientColor(color);
    }
    reloadAdsorptionSurfaceProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::adsorptionSurfaceOutsideAmbientLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->adsorptionSurfaceBackSideAmbientColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideDiffuseLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceBackSideDiffuseIntensity(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceOutsideDiffuseLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceBackSideDiffuseIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideDiffuseLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAdsorptionSurfaceBackSideDiffuseColor(color);
    }
    reloadAdsorptionSurfaceProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::adsorptionSurfaceOutsideDiffuseLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->adsorptionSurfaceBackSideDiffuseColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideSpecularLightIntensity(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceBackSideSpecularIntensity(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceOutsideSpecularLightIntensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceBackSideSpecularIntensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideSpecularLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setAdsorptionSurfaceBackSideSpecularColor(color);
    }
    reloadAdsorptionSurfaceProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::adsorptionSurfaceOutsideSpecularLightColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->adsorptionSurfaceBackSideSpecularColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAdsorptionSurfaceOutsideShininess(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAdsorptionSurfaceBackSideShininess(value);
  }
  reloadAdsorptionSurfaceProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::adsorptionSurfaceOutsideShininessy()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->adsorptionSurfaceBackSideShininess();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

// reload annotation properties
//========================================================================================================================================

void AppearanceTreeWidgetController::reloadAnnotationProperties()
{
  _appearanceAnnotationForm->colorPushButton->setEnabled(false);
  _appearanceAnnotationForm->fontComboBox->setEnabled(false);
  _appearanceAnnotationForm->fontSpecifierComboBox->setEnabled(false);

  reloadAnnotationType();
  reloadAnnotationFont();
  reloadAnnotationColor();
  reloadAnnotationAlignment();
  reloadAnnotationStyle();
  reloadAnnotationScaling();
  reloadAnnotationOffset();
}

void AppearanceTreeWidgetController::reloadAnnotationType()
{
  _appearanceAnnotationForm->typeComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAnnotationForm->typeComboBox->setEnabled(_projectTreeNode->isEditable());


      if (std::optional<RKTextType> type = annotationType())
      {
        if(int index = _appearanceAnnotationForm->typeComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAnnotationForm->typeComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAnnotationForm->typeComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearanceAnnotationForm->typeComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAnnotationForm->typeComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAnnotationForm->typeComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAnnotationFont()
{
  _appearanceAnnotationForm->fontComboBox->setDisabled(true);
  _appearanceAnnotationForm->fontSpecifierComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAnnotationForm->fontComboBox->setEnabled(_projectTreeNode->isEditable());
      _appearanceAnnotationForm->fontSpecifierComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> type = annotationFont())
      {
        QFont font;
        font.fromString(*type);
        QFontInfo fontInfo = QFontInfo(font);
        const QString familyName = fontInfo.family();
        const QString memberName = fontInfo.styleName();

        if(int index = _appearanceAnnotationForm->fontComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAnnotationForm->fontComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAnnotationForm->fontComboBox)->setCurrentText(familyName);


        QFontDatabase database;
        const QStringList fontStyles = database.styles(familyName);
        whileBlocking(_appearanceAnnotationForm->fontSpecifierComboBox)->clear();
        for (const QString &style : fontStyles)
        {
          whileBlocking(_appearanceAnnotationForm->fontSpecifierComboBox)->addItem(style);
        }

        if(int index = _appearanceAnnotationForm->fontSpecifierComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAnnotationForm->fontSpecifierComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAnnotationForm->fontSpecifierComboBox)->setCurrentText(memberName);

      }
      else
      {
        if(int index = _appearanceAnnotationForm->fontComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAnnotationForm->fontComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAnnotationForm->fontComboBox)->setCurrentText("Multiple values");
      }


    }
  }
}

void AppearanceTreeWidgetController::reloadAnnotationColor()
{
  _appearanceAnnotationForm->colorPushButton->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAnnotationForm->colorPushButton->setEnabled(_projectTreeNode->isEditable());
      if (std::optional<QColor> value = annotationTextColor())
      {
        whileBlocking(_appearanceAnnotationForm->colorPushButton)->setText("color");
        whileBlocking(_appearanceAnnotationForm->colorPushButton)->setColor(*value);
      }
      else
      {
        whileBlocking(_appearanceAnnotationForm->colorPushButton)->setColor(QColor(255,255,255,255));
        whileBlocking(_appearanceAnnotationForm->colorPushButton)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAnnotationAlignment()
{
  _appearanceAnnotationForm->alignmentComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAnnotationForm->alignmentComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<RKTextAlignment> type = annotationAlignment())
      {
        if(int index = _appearanceAnnotationForm->alignmentComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAnnotationForm->alignmentComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAnnotationForm->alignmentComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearanceAnnotationForm->alignmentComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAnnotationForm->alignmentComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAnnotationForm->alignmentComboBox)->setCurrentText("Multiple values");
      }


    }
  }
}

void AppearanceTreeWidgetController::reloadAnnotationStyle()
{
  _appearanceAnnotationForm->styleComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAnnotationForm->styleComboBox->setEnabled(_projectTreeNode->isEditable());


      if (std::optional<RKTextStyle> type = annotationStyle())
      {
        if(int index = _appearanceAnnotationForm->styleComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_appearanceAnnotationForm->styleComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceAnnotationForm->styleComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _appearanceAnnotationForm->styleComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_appearanceAnnotationForm->styleComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceAnnotationForm->styleComboBox)->setCurrentText("Multiple values");
      }


    }
  }
}

void AppearanceTreeWidgetController::reloadAnnotationScaling()
{
  _appearanceAnnotationForm->scalingDoubleSpinBox->setDisabled(true);
  _appearanceAnnotationForm->scalingHorizontalSlider->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAnnotationForm->scalingDoubleSpinBox->setEnabled(true);
      _appearanceAnnotationForm->scalingDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAnnotationForm->scalingHorizontalSlider->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> value = annotationScaling())
      {
        whileBlocking(_appearanceAnnotationForm->scalingDoubleSpinBox)->setValue(*value);
        whileBlocking(_appearanceAnnotationForm->scalingHorizontalSlider)->setDoubleValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAnnotationForm->scalingDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadAnnotationOffset()
{
  _appearanceAnnotationForm->offsetXDoubleSpinBox->setDisabled(true);
  _appearanceAnnotationForm->offsetYDoubleSpinBox->setDisabled(true);
  _appearanceAnnotationForm->offsetZDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _appearanceAnnotationForm->offsetXDoubleSpinBox->setEnabled(true);
      _appearanceAnnotationForm->offsetXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAnnotationForm->offsetYDoubleSpinBox->setEnabled(true);
      _appearanceAnnotationForm->offsetYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceAnnotationForm->offsetZDoubleSpinBox->setEnabled(true);
      _appearanceAnnotationForm->offsetZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = annotationOffsetX())
      {
        whileBlocking(_appearanceAnnotationForm->offsetXDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAnnotationForm->offsetXDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = annotationOffsetY())
      {
        whileBlocking(_appearanceAnnotationForm->offsetYDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAnnotationForm->offsetYDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = annotationOffsetZ())
      {
        whileBlocking(_appearanceAnnotationForm->offsetZDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_appearanceAnnotationForm->offsetZDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}


void AppearanceTreeWidgetController::setAnnotationType(int value)
{
  if(value >= 0 && value <= int(RKTextType::multiple_values))  // also include "Custom"
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setRenderTextType(RKTextType(value));
    }

    reloadAnnotationProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<RKTextType> AppearanceTreeWidgetController::annotationType()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<RKTextType> set{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    RKTextType value = iraspa_structure->structure()->renderTextType();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAnnotationFontFamily(QString value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setRenderTextFont(value);
  }

  reloadAnnotationProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

void AppearanceTreeWidgetController::setAnnotationFontMember(QString value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QString fontName = iraspa_structure->structure()->renderTextFont();

    QFontDatabase database = QFontDatabase();
    QFont font = QFont(fontName);
    QFontInfo fontInfo = QFontInfo(font);
    QString fontFamily = fontInfo.family();

    QFont newFont = QFont(fontFamily);
    newFont.setStyleName(value);

    iraspa_structure->structure()->setRenderTextFont(newFont.toString());
  }

  reloadAnnotationProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<QString> AppearanceTreeWidgetController::annotationFont()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->renderTextFont();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAnnotationTextColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    _appearanceAtomsForm->atomSpecularColorPushButton->setColor(color);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setRenderTextColor(color);
    }
    reloadAnnotationProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<QColor> AppearanceTreeWidgetController::annotationTextColor()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<QColor> set = std::unordered_set<QColor>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QColor value = iraspa_structure->structure()->renderTextColor();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


void AppearanceTreeWidgetController::setAnnotationAlignment(int value)
{
  if(value >= 0 && value <= int(RKTextAlignment::multiple_values))  // also include "Custom"
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setRenderTextAlignment(RKTextAlignment(value));
    }

    reloadAnnotationProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<RKTextAlignment> AppearanceTreeWidgetController::annotationAlignment()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<RKTextAlignment> set{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    RKTextAlignment value = iraspa_structure->structure()->renderTextAlignment();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAnnotationStyle(int value)
{
  if(value >= 0 && value <= int(RKTextStyle::multiple_values))  // also include "Custom"
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setRenderTextStyle(RKTextStyle(value));
    }

    reloadAnnotationProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<RKTextStyle> AppearanceTreeWidgetController::annotationStyle()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<RKTextStyle> set{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    RKTextStyle value = iraspa_structure->structure()->renderTextStyle();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAnnotationScaling(double value)
{
  if(value>=0)
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setRenderTextScaling(value);
    }
    reloadAnnotationProperties();
    emit rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> AppearanceTreeWidgetController::annotationScaling()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->renderTextScaling();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


void AppearanceTreeWidgetController::setAnnotationOffsetX(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setRenderTextOffsetX(value);
  }
  reloadAnnotationProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::annotationOffsetX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->renderTextOffset().x;
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAnnotationOffsetY(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setRenderTextOffsetY(value);
  }
  reloadAnnotationProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::annotationOffsetY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->renderTextOffset().y;
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void AppearanceTreeWidgetController::setAnnotationOffsetZ(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setRenderTextOffsetZ(value);
  }
  reloadAnnotationProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> AppearanceTreeWidgetController::annotationOffsetZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->renderTextOffset().z;
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}



// expansion
//========================================================================================================================================

void AppearanceTreeWidgetController::expandPrimitiveItem()
{
  QModelIndex index = indexFromItem(topLevelItem(0),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonPrimitive->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonPrimitive->setIcon(QIcon(":/iraspa/expanded.png"));
    //reloadAtomProperties();
  }
}

void AppearanceTreeWidgetController::expandAtomsItem()
{
  QModelIndex index = indexFromItem(topLevelItem(1),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonAtoms->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonAtoms->setIcon(QIcon(":/iraspa/expanded.png"));
    //reloadAtomProperties();
  }
}

void AppearanceTreeWidgetController::expandBondsItem()
{
  QModelIndex index = indexFromItem(topLevelItem(2),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonBonds->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonBonds->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void AppearanceTreeWidgetController::expandUnitCellItem()
{
  QModelIndex index = indexFromItem(topLevelItem(3),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonUnitCell->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonUnitCell->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void AppearanceTreeWidgetController::expandAdsorptionSurfaceItem()
{
  QModelIndex index = indexFromItem(topLevelItem(4),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonAdsorptionSurface->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonAdsorptionSurface->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void AppearanceTreeWidgetController::expandAnnotationItem()
{
  QModelIndex index = indexFromItem(topLevelItem(5),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonAnnotation->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonAnnotation->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}
