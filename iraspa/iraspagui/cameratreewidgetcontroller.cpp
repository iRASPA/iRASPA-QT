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

#include "cameratreewidgetcontroller.h"
#include <cmath>
#include <iostream>
#include <QLabel>
#include <QPixmap>
#include <QColorDialog>
#include <mathkit.h>
#include <renderkit.h>
#include <foundationkit.h>
#include <textfield.h>
#include "moviemaker.h"
#include "savemoviedialog.h"
#include "savepicturedialog.h"

CameraTreeWidgetController::CameraTreeWidgetController(QWidget* parent): QTreeWidget(parent ),
    _cameraCameraForm(new CameraCameraForm(this)),
    _cameraSelectionForm(new CameraSelectionForm(this)),
    _cameraAxesForm(new CameraAxesForm(this)),
    _cameraLightsForm(new CameraLightsForm(this)),
    _cameraPicturesForm(new CameraPicturesForm(this)),
    _cameraBackgroundForm(new CameraBackgroundForm(this))
{
  this->setHeaderHidden(true);
  this->setRootIsDecorated(true);
  this->setFrameStyle(QFrame::NoFrame);
  this->setAnimated(true);
  this->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  this->setExpandsOnDoubleClick(false);
  this->setIndentation(0);
  this->setSelectionMode(QAbstractItemView::NoSelection);



  // Camera
  //==========================================================================================================
  QTreeWidgetItem* cameraItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(cameraItem);

  pushButtonCamera = new QPushButton(tr("Camera"),this);
  pushButtonCamera->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonCamera->setStyleSheet("text-align:left;");
  setItemWidget(cameraItem,0,pushButtonCamera);

  QTreeWidgetItem *childCameraItem = new QTreeWidgetItem(cameraItem);
  this->setItemWidget(childCameraItem,0, _cameraCameraForm);

  pushButtonCamera->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  QFontMetrics fm(font());
  pushButtonCamera->resize(size().width(), fm.height());

  _cameraCameraForm->resetPercentageSpinBox->setMinimum(0.0);
  _cameraCameraForm->resetPercentageSpinBox->setMaximum(100.0);
  _cameraCameraForm->resetPercentageSpinBox->setDecimals(1);
  _cameraCameraForm->resetPercentageSpinBox->setKeyboardTracking(false);
  _cameraCameraForm->resetPercentageSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  QObject::connect(_cameraCameraForm->resetPercentageSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setResetPercentage);
  QObject::connect(_cameraCameraForm->resetCameraToDirectionPushButton, &QPushButton::clicked,this,&CameraTreeWidgetController::resetCameraToDefaultDirection);
  QObject::connect(_cameraCameraForm->resetCameraToMinusXRadioButton, &QRadioButton::clicked,this,&CameraTreeWidgetController::setResetDirectionToMinusX);
  QObject::connect(_cameraCameraForm->resetCameraToPlusXRadioButton, &QRadioButton::clicked,this,&CameraTreeWidgetController::setResetDirectionToPlusX);
  QObject::connect(_cameraCameraForm->resetCameraToMinusYRadioButton, &QRadioButton::clicked,this,&CameraTreeWidgetController::setResetDirectionToMinusY);
  QObject::connect(_cameraCameraForm->resetCameraToPlusYRadioButton, &QRadioButton::clicked,this,&CameraTreeWidgetController::setResetDirectionToPlusY);
  QObject::connect(_cameraCameraForm->resetCameraToMinusZRadioButton, &QRadioButton::clicked,this,&CameraTreeWidgetController::setResetDirectionToMinusZ);
  QObject::connect(_cameraCameraForm->resetCameraToPlusZRadioButton, &QRadioButton::clicked,this,&CameraTreeWidgetController::setResetDirectionToPlusZ);


  QObject::connect(_cameraCameraForm->orthographicCameraRadioButton, &QRadioButton::clicked,this,&CameraTreeWidgetController::setCameraToOrthographic);
  QObject::connect(_cameraCameraForm->perspectiveCameraRadioButton, &QRadioButton::clicked,this,&CameraTreeWidgetController::setCameraToPerspective);

  _cameraCameraForm->angleOfViewSpinBox->setMinimum(-180.0);
  _cameraCameraForm->angleOfViewSpinBox->setMaximum(180.0);
  _cameraCameraForm->angleOfViewSpinBox->setDecimals(1);
  _cameraCameraForm->angleOfViewSpinBox->setKeyboardTracking(false);
  _cameraCameraForm->angleOfViewSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  QObject::connect(_cameraCameraForm->angleOfViewSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAngleOfView);

  _cameraCameraForm->rotateAngleSpinBox->setMinimum(0.0);
  _cameraCameraForm->rotateAngleSpinBox->setMaximum(100.0);
  _cameraCameraForm->rotateAngleSpinBox->setDecimals(1);
  _cameraCameraForm->rotateAngleSpinBox->setKeyboardTracking(false);
  _cameraCameraForm->rotateAngleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  QObject::connect(_cameraCameraForm->rotateAngleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setRotationAngle);
  QObject::connect(_cameraCameraForm->rotateMinusXPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::rotateYawPlus);
  QObject::connect(_cameraCameraForm->rotatePlusXPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::rotateYawMinus);
  QObject::connect(_cameraCameraForm->rotateMinusYPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::rotatePitchPlus);
  QObject::connect(_cameraCameraForm->rotatePlusYPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::rotatePitchMinus);
  QObject::connect(_cameraCameraForm->rotateMinusZPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::rotateRollPlus);
  QObject::connect(_cameraCameraForm->rotatePlusZPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::rotateRollMinus);


  _cameraCameraForm->EulerAngleXSpinBox->setMinimum(-180.0);
  _cameraCameraForm->EulerAngleXSpinBox->setMaximum(180.0);
  _cameraCameraForm->EulerAngleXSpinBox->setDecimals(5);
  _cameraCameraForm->EulerAngleXSpinBox->setKeyboardTracking(false);
  _cameraCameraForm->EulerAngleXSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraCameraForm->EulerAngleZSpinBox->setMinimum(-180.0);
  _cameraCameraForm->EulerAngleZSpinBox->setMaximum(180.0);
  _cameraCameraForm->EulerAngleZSpinBox->setDecimals(5);
  _cameraCameraForm->EulerAngleZSpinBox->setKeyboardTracking(false);
  _cameraCameraForm->EulerAngleZSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraCameraForm->EulerAngleYSpinBox->setMinimum(-90.0);
  _cameraCameraForm->EulerAngleYSpinBox->setMaximum(90.0);
  _cameraCameraForm->EulerAngleYSpinBox->setDecimals(5);
  _cameraCameraForm->EulerAngleYSpinBox->setKeyboardTracking(false);
  _cameraCameraForm->EulerAngleYSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  whileBlocking(_cameraCameraForm->EulerAngleXDial)->setMinimum(-180);
  whileBlocking(_cameraCameraForm->EulerAngleXDial)->setMaximum(180);
  whileBlocking(_cameraCameraForm->EulerAngleZDial)->setMinimum(-180);
  whileBlocking(_cameraCameraForm->EulerAngleZDial)->setMaximum(180);
  whileBlocking(_cameraCameraForm->EulerAngleYSlider)->setMinimum(-90);
  whileBlocking(_cameraCameraForm->EulerAngleYSlider)->setMaximum(90);

  QObject::connect(_cameraCameraForm->EulerAngleXDial, &QDial::valueChanged,this,static_cast<void (CameraTreeWidgetController::*)(int)>(&CameraTreeWidgetController::setEulerAngleX));
  QObject::connect(_cameraCameraForm->EulerAngleXSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,static_cast<void (CameraTreeWidgetController::*)(double)>(&CameraTreeWidgetController::setEulerAngleX));
  QObject::connect(_cameraCameraForm->EulerAngleZDial, &QDial::valueChanged,this,static_cast<void (CameraTreeWidgetController::*)(int)>(&CameraTreeWidgetController::setEulerAngleZ));
  QObject::connect(_cameraCameraForm->EulerAngleZSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,static_cast<void (CameraTreeWidgetController::*)(double)>(&CameraTreeWidgetController::setEulerAngleZ));
  QObject::connect(_cameraCameraForm->EulerAngleYSlider, &QDial::valueChanged,this,static_cast<void (CameraTreeWidgetController::*)(int)>(&CameraTreeWidgetController::setEulerAngleY));
  QObject::connect(_cameraCameraForm->EulerAngleYSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,static_cast<void (CameraTreeWidgetController::*)(double)>(&CameraTreeWidgetController::setEulerAngleY));

  // Selection
  //==========================================================================================================
  QTreeWidgetItem* selectionItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(selectionItem);

  pushButtonSelection = new QPushButton(tr("Selection"),this);
  pushButtonSelection->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonSelection->setStyleSheet("text-align:left;");
  setItemWidget(selectionItem,0,pushButtonSelection);

  QTreeWidgetItem *childSelectionItem = new QTreeWidgetItem(selectionItem);
  this->setItemWidget(childSelectionItem,0, _cameraSelectionForm);

  QObject::connect(_cameraSelectionForm->selectionIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setSelectionIntensity);
  QObject::connect(_cameraSelectionForm->selectionIntensityDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&CameraTreeWidgetController::setSelectionIntensity);


  pushButtonSelection->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonSelection->resize(size().width(), fm.height());

  _cameraSelectionForm->selectionIntensityDoubleSlider->setDoubleMinimum(0.0);
  _cameraSelectionForm->selectionIntensityDoubleSlider->setDoubleMaximum(2.0);

  // Axes
  //==========================================================================================================
  QTreeWidgetItem* axesItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(axesItem);

  pushButtonAxes = new QPushButton(tr("Axes"),this);
  pushButtonAxes->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonAxes->setStyleSheet("text-align:left;");
  setItemWidget(axesItem,0,pushButtonAxes);

  QTreeWidgetItem *childAxesItem = new QTreeWidgetItem(axesItem);
  this->setItemWidget(childAxesItem,0, _cameraAxesForm);

  _cameraAxesForm->cameraAxesPosition->insertItem(0,tr("None"));
  _cameraAxesForm->cameraAxesPosition->insertItem(1,tr("Bottom-Left"));
  _cameraAxesForm->cameraAxesPosition->insertItem(2,tr("Mid-Left"));
  _cameraAxesForm->cameraAxesPosition->insertItem(3,tr("Top-Left"));
  _cameraAxesForm->cameraAxesPosition->insertItem(4,tr("Mid-Top"));
  _cameraAxesForm->cameraAxesPosition->insertItem(5,tr("Top-Right"));
  _cameraAxesForm->cameraAxesPosition->insertItem(6,tr("Mid-Right"));
  _cameraAxesForm->cameraAxesPosition->insertItem(7,tr("Bottom-Right"));
  _cameraAxesForm->cameraAxesPosition->insertItem(8,tr("Mid-Bottom"));
  _cameraAxesForm->cameraAxesPosition->insertItem(9,tr("Center"));

  _cameraAxesForm->cameraAxesStyle->insertItem(0,tr("Default"));
  _cameraAxesForm->cameraAxesStyle->insertItem(1,tr("Thick RGB"));
  _cameraAxesForm->cameraAxesStyle->insertItem(2,tr("Thick"));
  _cameraAxesForm->cameraAxesStyle->insertItem(3,tr("Thin RGB"));
  _cameraAxesForm->cameraAxesStyle->insertItem(4,tr("Thin"));
  _cameraAxesForm->cameraAxesStyle->insertItem(5,tr("Beam-Arrow RGB"));
  _cameraAxesForm->cameraAxesStyle->insertItem(6,tr("Beam-Arrow"));
  _cameraAxesForm->cameraAxesStyle->insertItem(7,tr("Beam RGB"));
  _cameraAxesForm->cameraAxesStyle->insertItem(8,tr("Beam"));
  _cameraAxesForm->cameraAxesStyle->insertItem(9,tr("Squashed RGB"));
  _cameraAxesForm->cameraAxesStyle->insertItem(10,tr("Squashed"));

  _cameraAxesForm->cameraAxesBackgroundShape->insertItem(0,tr("None"));
  _cameraAxesForm->cameraAxesBackgroundShape->insertItem(1,tr("Filled Circle"));
  _cameraAxesForm->cameraAxesBackgroundShape->insertItem(2,tr("Filled Square"));
  _cameraAxesForm->cameraAxesBackgroundShape->insertItem(3,tr("Filled Rounded Square"));
  _cameraAxesForm->cameraAxesBackgroundShape->insertItem(4,tr("Circle"));
  _cameraAxesForm->cameraAxesBackgroundShape->insertItem(5,tr("Square"));
  _cameraAxesForm->cameraAxesBackgroundShape->insertItem(6,tr("Rounded Square"));

  _cameraAxesForm->cameraAxesSize->setMinimum(0.0);
  _cameraAxesForm->cameraAxesSize->setMaximum(100.0);
  _cameraAxesForm->cameraAxesSize->setDecimals(5);
  _cameraAxesForm->cameraAxesSize->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesSize->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesOffset->setMinimum(0.0);
  _cameraAxesForm->cameraAxesOffset->setMaximum(100.0);
  _cameraAxesForm->cameraAxesOffset->setDecimals(5);
  _cameraAxesForm->cameraAxesOffset->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesOffset->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesBackgroundAdditionalSize->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesBackgroundAdditionalSize->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesBackgroundAdditionalSize->setDecimals(5);
  _cameraAxesForm->cameraAxesBackgroundAdditionalSize->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesBackgroundAdditionalSize->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesTextScalingX->setMinimum(0.0);
  _cameraAxesForm->cameraAxesTextScalingX->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesTextScalingX->setDecimals(5);
  _cameraAxesForm->cameraAxesTextScalingX->setSingleStep(0.5);
  _cameraAxesForm->cameraAxesTextScalingX->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesTextScalingX->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesTextScalingY->setMinimum(0.0);
  _cameraAxesForm->cameraAxesTextScalingY->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesTextScalingY->setDecimals(5);
  _cameraAxesForm->cameraAxesTextScalingY->setSingleStep(0.5);
  _cameraAxesForm->cameraAxesTextScalingY->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesTextScalingY->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesTextScalingZ->setMinimum(0.0);
  _cameraAxesForm->cameraAxesTextScalingZ->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesTextScalingZ->setDecimals(5);
  _cameraAxesForm->cameraAxesTextScalingZ->setSingleStep(0.5);
  _cameraAxesForm->cameraAxesTextScalingZ->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesTextScalingZ->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesXTextDisplacementX->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesXTextDisplacementX->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesXTextDisplacementX->setDecimals(5);
  _cameraAxesForm->cameraAxesXTextDisplacementX->setSingleStep(0.1);
  _cameraAxesForm->cameraAxesXTextDisplacementX->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesXTextDisplacementX->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesXTextDisplacementY->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesXTextDisplacementY->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesXTextDisplacementY->setDecimals(5);
  _cameraAxesForm->cameraAxesXTextDisplacementY->setSingleStep(0.1);
  _cameraAxesForm->cameraAxesXTextDisplacementY->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesXTextDisplacementY->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesXTextDisplacementZ->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesXTextDisplacementZ->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesXTextDisplacementZ->setDecimals(5);
  _cameraAxesForm->cameraAxesXTextDisplacementZ->setSingleStep(0.1);
  _cameraAxesForm->cameraAxesXTextDisplacementZ->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesXTextDisplacementZ->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesYTextDisplacementX->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesYTextDisplacementX->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesYTextDisplacementX->setDecimals(5);
  _cameraAxesForm->cameraAxesYTextDisplacementX->setSingleStep(0.1);
  _cameraAxesForm->cameraAxesYTextDisplacementX->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesYTextDisplacementX->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesYTextDisplacementY->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesYTextDisplacementY->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesYTextDisplacementY->setDecimals(5);
  _cameraAxesForm->cameraAxesYTextDisplacementY->setSingleStep(0.1);
  _cameraAxesForm->cameraAxesYTextDisplacementY->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesYTextDisplacementY->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesYTextDisplacementZ->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesYTextDisplacementZ->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesYTextDisplacementZ->setDecimals(5);
  _cameraAxesForm->cameraAxesYTextDisplacementZ->setSingleStep(0.1);
  _cameraAxesForm->cameraAxesYTextDisplacementZ->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesYTextDisplacementZ->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesZTextDisplacementX->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesZTextDisplacementX->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesZTextDisplacementX->setDecimals(5);
  _cameraAxesForm->cameraAxesZTextDisplacementX->setSingleStep(0.1);
  _cameraAxesForm->cameraAxesZTextDisplacementX->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesZTextDisplacementX->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesZTextDisplacementY->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesZTextDisplacementY->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesZTextDisplacementY->setDecimals(5);
  _cameraAxesForm->cameraAxesZTextDisplacementY->setSingleStep(0.1);
  _cameraAxesForm->cameraAxesZTextDisplacementY->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesZTextDisplacementY->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cameraAxesForm->cameraAxesZTextDisplacementZ->setMinimum(-DBL_MAX);
  _cameraAxesForm->cameraAxesZTextDisplacementZ->setMaximum(DBL_MAX);
  _cameraAxesForm->cameraAxesZTextDisplacementZ->setDecimals(5);
  _cameraAxesForm->cameraAxesZTextDisplacementZ->setSingleStep(0.1);
  _cameraAxesForm->cameraAxesZTextDisplacementZ->setKeyboardTracking(false);
  _cameraAxesForm->cameraAxesZTextDisplacementZ->setFocusPolicy(Qt::FocusPolicy::ClickFocus);



  QObject::connect(_cameraAxesForm->cameraAxesPosition,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CameraTreeWidgetController::setAxesPosition);
  QObject::connect(_cameraAxesForm->cameraAxesStyle,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CameraTreeWidgetController::setAxesStyle);
  QObject::connect(_cameraAxesForm->cameraAxesSize,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesSize);
  QObject::connect(_cameraAxesForm->cameraAxesOffset,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesOffset);

  QObject::connect(_cameraAxesForm->cameraAxesBackgroundShape,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CameraTreeWidgetController::setAxesBackgroundStyle);
  QObject::connect(_cameraAxesForm->cameraAxesBackgroundColor,&QPushButton::clicked,this,&CameraTreeWidgetController::setAxesBackgroundColor);
  QObject::connect(_cameraAxesForm->cameraAxesBackgroundAdditionalSize,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesBackgroundAdditionalSize);


  QObject::connect(_cameraAxesForm->cameraAxesTextScalingX,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesTextScalingX);
  QObject::connect(_cameraAxesForm->cameraAxesTextScalingY,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesTextScalingY);
  QObject::connect(_cameraAxesForm->cameraAxesTextScalingZ,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesTextScalingZ);

  QObject::connect(_cameraAxesForm->cameraAxesTextColorX,&QPushButton::clicked,this,&CameraTreeWidgetController::setAxesTextolorX);
  QObject::connect(_cameraAxesForm->cameraAxesTextColorY,&QPushButton::clicked,this,&CameraTreeWidgetController::setAxesTextolorY);
  QObject::connect(_cameraAxesForm->cameraAxesTextColorZ,&QPushButton::clicked,this,&CameraTreeWidgetController::setAxesTextolorZ);

  QObject::connect(_cameraAxesForm->cameraAxesXTextDisplacementX,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesXTextDisplacementX);
  QObject::connect(_cameraAxesForm->cameraAxesXTextDisplacementY,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesXTextDisplacementY);
  QObject::connect(_cameraAxesForm->cameraAxesXTextDisplacementZ,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesXTextDisplacementZ);
  QObject::connect(_cameraAxesForm->cameraAxesYTextDisplacementX,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesYTextDisplacementX);
  QObject::connect(_cameraAxesForm->cameraAxesYTextDisplacementY,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesYTextDisplacementY);
  QObject::connect(_cameraAxesForm->cameraAxesYTextDisplacementZ,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesYTextDisplacementZ);
  QObject::connect(_cameraAxesForm->cameraAxesZTextDisplacementX,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesZTextDisplacementX);
  QObject::connect(_cameraAxesForm->cameraAxesZTextDisplacementY,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesZTextDisplacementY);
  QObject::connect(_cameraAxesForm->cameraAxesZTextDisplacementZ,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAxesZTextDisplacementZ);


  pushButtonAxes->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonAxes->resize(size().width(), fm.height());

  // Lights
  //==========================================================================================================
  QTreeWidgetItem* lightsItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(lightsItem);

  pushButtonLights = new QPushButton(tr("Lights"),this);
  pushButtonLights->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonLights->setStyleSheet("text-align:left;");
  setItemWidget(lightsItem,0,pushButtonLights);

  QTreeWidgetItem *childLightsItem = new QTreeWidgetItem(lightsItem);
  this->setItemWidget(childLightsItem,0, _cameraLightsForm);


  _cameraLightsForm->ambientLightIntensityDoubleSpinBox->setMinimum(0.0);
  _cameraLightsForm->ambientLightIntensityDoubleSpinBox->setDecimals(5);
  _cameraLightsForm->ambientLightIntensityDoubleSpinBox->setSingleStep(0.1);
  _cameraLightsForm->ambientLightIntensityDoubleSpinBox->setKeyboardTracking(false);
  _cameraLightsForm->ambientLightIntensityDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cameraLightsForm->ambientLightIntensityDoubleSlider->setMinimum(0);
  _cameraLightsForm->ambientLightIntensityDoubleSlider->setMaximum(1000);

  _cameraLightsForm->diffuseLightIntensityDoubleSpinBox->setMinimum(0.0);
  _cameraLightsForm->diffuseLightIntensityDoubleSpinBox->setDecimals(5);
  _cameraLightsForm->diffuseLightIntensityDoubleSpinBox->setSingleStep(0.1);
  _cameraLightsForm->diffuseLightIntensityDoubleSpinBox->setKeyboardTracking(false);
  _cameraLightsForm->diffuseLightIntensityDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cameraLightsForm->diffuseLightIntensityDoubleSlider->setMinimum(0);
  _cameraLightsForm->diffuseLightIntensityDoubleSlider->setMaximum(1000);

  _cameraLightsForm->specularLightIntensityDoubleSpinBox->setMinimum(0.0);
  _cameraLightsForm->specularLightIntensityDoubleSpinBox->setDecimals(5);
  _cameraLightsForm->specularLightIntensityDoubleSpinBox->setSingleStep(0.1);
  _cameraLightsForm->specularLightIntensityDoubleSpinBox->setKeyboardTracking(false);
  _cameraLightsForm->specularLightIntensityDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cameraLightsForm->specularLightIntensityDoubleSlider->setMinimum(0);
  _cameraLightsForm->specularLightIntensityDoubleSlider->setMaximum(1000);

  QObject::connect(_cameraLightsForm->ambientLightIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setAmbientLightIntensity);
  QObject::connect(_cameraLightsForm->ambientLightIntensityDoubleSlider,static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged),this,&CameraTreeWidgetController::setAmbientLightIntensity);
  QObject::connect(_cameraLightsForm->ambientLightPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::setAmbientColor);

  QObject::connect(_cameraLightsForm->diffuseLightIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setDiffuseLightIntensity);
  QObject::connect(_cameraLightsForm->diffuseLightIntensityDoubleSlider,static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged),this,&CameraTreeWidgetController::setDiffuseLightIntensity);
  QObject::connect(_cameraLightsForm->diffuseLightPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::setDiffuseColor);

  QObject::connect(_cameraLightsForm->specularLightIntensityDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setSpecularLightIntensity);
  QObject::connect(_cameraLightsForm->specularLightIntensityDoubleSlider,static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged),this,&CameraTreeWidgetController::setSpecularLightIntensity);
  QObject::connect(_cameraLightsForm->specularLightPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::setSpecularColor);  

  pushButtonLights->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonLights->resize(size().width(), fm.height());

  // Pictures
  //==========================================================================================================
  QTreeWidgetItem* picturesItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(picturesItem);

  pushButtonPictures = new QPushButton(tr("Pictures/Movies"),this);
  pushButtonPictures->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonPictures->setStyleSheet("text-align:left;");
  setItemWidget(picturesItem,0,pushButtonPictures);

  QTreeWidgetItem *childPicturesItem = new QTreeWidgetItem(picturesItem);
  this->setItemWidget(childPicturesItem,0, _cameraPicturesForm);

  pushButtonPictures->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonPictures->resize(size().width(), fm.height());

  _cameraPicturesForm->dotsPerInchComboBox->insertItem(0,"72 dpi");
  _cameraPicturesForm->dotsPerInchComboBox->insertItem(1,"75 dpi");
  _cameraPicturesForm->dotsPerInchComboBox->insertItem(2,"150 dpi");
  _cameraPicturesForm->dotsPerInchComboBox->insertItem(3,"300 dpi");
  _cameraPicturesForm->dotsPerInchComboBox->insertItem(4,"600 dpi");
  _cameraPicturesForm->dotsPerInchComboBox->insertItem(5,"1200 dpi");

  _cameraPicturesForm->qualityComboBox->insertItem(0,"8-bits, RGBA");
  _cameraPicturesForm->qualityComboBox->insertItem(1,"16-bits, RGBA");
  _cameraPicturesForm->qualityComboBox->insertItem(2,"8-bits, CMYK");
  _cameraPicturesForm->qualityComboBox->insertItem(3,"16-bits, CMYK");

  _cameraPicturesForm->movieFormatComboBox->insertItem(0,"hevc (h265)");
  _cameraPicturesForm->movieFormatComboBox->insertItem(1,"Format");
  _cameraPicturesForm->movieFormatComboBox->insertItem(2,"vp9");

  _cameraPicturesForm->movieTypeComboBox->insertItem(0,"Frames");
  _cameraPicturesForm->movieTypeComboBox->insertItem(1,"Rotation Around Y");

  QObject::connect(_cameraPicturesForm->dotsPerInchComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CameraTreeWidgetController::setPictureDotsPerInch);
  QObject::connect(_cameraPicturesForm->qualityComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CameraTreeWidgetController::setPictureQuality);

  QObject::connect(_cameraPicturesForm->physicalWidthDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setPicturePhysicalSize);
  QObject::connect(_cameraPicturesForm->pixelWidthSpinBox,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&CameraTreeWidgetController::setPictureNumberOfPixels);

  QObject::connect(_cameraPicturesForm->physicalDimensionsRadioButton, &QRadioButton::clicked, this,&CameraTreeWidgetController::setPictureDimensionToPhysical);
  QObject::connect(_cameraPicturesForm->pixelDimensionsRadioButton, &QRadioButton::clicked, this,&CameraTreeWidgetController::setPictureDimensionToPixels);
  QObject::connect(_cameraPicturesForm->inchRadioButton, &QRadioButton::clicked, this,&CameraTreeWidgetController::setPictureUnitsToInch);
  QObject::connect(_cameraPicturesForm->cmRadioButton, &QRadioButton::clicked, this,&CameraTreeWidgetController::setPictureUnitsToCentimeters);

  QObject::connect(_cameraPicturesForm->createPicturePushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::savePicture);
  QObject::connect(_cameraPicturesForm->createMoviePushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::saveMovie);

  QObject::connect(_cameraPicturesForm->movieFormatComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CameraTreeWidgetController::setMovieFormat);
  QObject::connect(_cameraPicturesForm->movieTypeComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CameraTreeWidgetController::setMovieType);
  QObject::connect(_cameraPicturesForm->framesPerSecondSpinBox,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&CameraTreeWidgetController::setNumberOfFramesPerSecond);


  // Background
  //==========================================================================================================
  QTreeWidgetItem* backgroundItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(backgroundItem);

  pushButtonBackground = new QPushButton(tr("Background"),this);
  pushButtonBackground->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonBackground->setStyleSheet("text-align:left;");
  setItemWidget(backgroundItem,0,pushButtonBackground);

  QTreeWidgetItem *childBackgroundItem = new QTreeWidgetItem(backgroundItem);
  this->setItemWidget(childBackgroundItem,0, _cameraBackgroundForm);

  pushButtonBackground->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonBackground->resize(size().width(), fm.height());


  QObject::connect(_cameraBackgroundForm->colorRadioButton, &QRadioButton::clicked, this, &CameraTreeWidgetController::setBackgroundToColor);
  QObject::connect(_cameraBackgroundForm->linearGradientRadioButton, &QRadioButton::clicked, this, &CameraTreeWidgetController::setBackgroundToLinearGradient);
  QObject::connect(_cameraBackgroundForm->radialGradientRadioButton, &QRadioButton::clicked, this, &CameraTreeWidgetController::setBackgroundToRadialGradient);
  QObject::connect(_cameraBackgroundForm->imageRadioButton, &QRadioButton::clicked, this, &CameraTreeWidgetController::setBackgroundToImage);

  QObject::connect(_cameraBackgroundForm->backgroundColorPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::setBackgroundColor);
  QObject::connect(_cameraBackgroundForm->linearGradientFromColorPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::setLinearGradientFromColor);
  QObject::connect(_cameraBackgroundForm->linearGradientToColorPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::setLinearGradientToColor);
  QObject::connect(_cameraBackgroundForm->radialGradientFromColorPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::setRadialGradientFromColor);
  QObject::connect(_cameraBackgroundForm->radialGradientToColorPushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::setRadialGradientToColor);

  QObject::connect(_cameraBackgroundForm->selectImagePushButton,&QPushButton::clicked,this,&CameraTreeWidgetController::selectBackgroundImage);

  QObject::connect(_cameraBackgroundForm->linearGradientAngleDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setLinearGradientAngle);
  QObject::connect(_cameraBackgroundForm->linearGradientAngleDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&CameraTreeWidgetController::setLinearGradientAngle);

  _cameraBackgroundForm->linearGradientAngleDoubleSlider->setDoubleMinimum(0.0);
  _cameraBackgroundForm->linearGradientAngleDoubleSlider->setDoubleMaximum(360.0);

  QObject::connect(_cameraBackgroundForm->radialGradientRoundnessDoubleSpinBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CameraTreeWidgetController::setRadialGradientRoundness);
  QObject::connect(_cameraBackgroundForm->radialGradientRoundnessDoubleSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&CameraTreeWidgetController::setRadialGradientRoundness);

  _cameraBackgroundForm->radialGradientRoundnessDoubleSlider->setDoubleMinimum(0.0);
  _cameraBackgroundForm->radialGradientRoundnessDoubleSlider->setDoubleMaximum(5.0);


  // Expanding
  //=========================================================================
  pushButtonCamera->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonSelection->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonAxes->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonLights->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonPictures->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonBackground->setFocusPolicy(Qt::FocusPolicy::NoFocus);

  QObject::connect(pushButtonCamera, &QPushButton::clicked, this, &CameraTreeWidgetController::expandCameraItem);
  QObject::connect(pushButtonSelection, &QPushButton::clicked, this, &CameraTreeWidgetController::expandSelectionItem);
  QObject::connect(pushButtonAxes, &QPushButton::clicked, this, &CameraTreeWidgetController::expandAxesItem);
  QObject::connect(pushButtonLights, &QPushButton::clicked, this, &CameraTreeWidgetController::expandLightsItem);
  QObject::connect(pushButtonPictures, &QPushButton::clicked, this, &CameraTreeWidgetController::expandPicturesItem);
  QObject::connect(pushButtonBackground, &QPushButton::clicked, this, &CameraTreeWidgetController::expandBackgroundItem);
}

void CameraTreeWidgetController::setMainWindow(MainWindow *mainWindow)
{
  _mainWindow = mainWindow;
}

void CameraTreeWidgetController::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  _camera.reset();
  _project.reset();

  if (projectTreeNode)
  {
    if(std::shared_ptr<iRASPAProject> iraspaProject = projectTreeNode->representedObject())
    {
      if(std::shared_ptr<Project> project = iraspaProject->project())
      {
        if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
        {
          _camera = projectStructure->camera();
          _project = projectStructure;
        }
      }
    }
  }

  reloadCameraData();
  update();
}

void CameraTreeWidgetController::expandCameraItem()
{
  QModelIndex index = indexFromItem(topLevelItem(0),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonCamera->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonCamera->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CameraTreeWidgetController::expandSelectionItem()
{
  QModelIndex index = indexFromItem(topLevelItem(1),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonSelection->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonSelection->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CameraTreeWidgetController::expandAxesItem()
{
  QModelIndex index = indexFromItem(topLevelItem(2),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonAxes->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonAxes->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CameraTreeWidgetController::expandLightsItem()
{
  QModelIndex index = indexFromItem(topLevelItem(3),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonLights->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonLights->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CameraTreeWidgetController::expandPicturesItem()
{
  QModelIndex index = indexFromItem(topLevelItem(4),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonPictures->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonPictures->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CameraTreeWidgetController::expandBackgroundItem()
{
  QModelIndex index = indexFromItem(topLevelItem(5),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonBackground->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonBackground->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CameraTreeWidgetController::renderWidgetResize([[maybe_unused]] QSize size)
{

}

void CameraTreeWidgetController::resetData()
{
  reloadData();
}

void CameraTreeWidgetController::reloadData()
{
  reloadCameraProperties();
  reloadCameraProjection();
  reloadCameraResetDirection();
  reloadCameraEulerAngles();
  reloadCameraModelViewMatrix();

  reloadSelectionProperties();
  reloadAxesProperties();
  reloadAxesBackgroundProperties();
  reloadAxesTextProperties();
  reloadLightsProperties();
  reloadPictureProperties();
  reloadBackgroundProperties();
}

void CameraTreeWidgetController::reloadSelection()
{

}

void CameraTreeWidgetController::reloadCameraResetDirection()
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    switch(camera->resetCameraDirection())
    {
      case ResetDirectionType::minus_X:
        whileBlocking(_cameraCameraForm->resetCameraToMinusXRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::minus_Y:
        whileBlocking(_cameraCameraForm->resetCameraToMinusYRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::minus_Z:
        whileBlocking(_cameraCameraForm->resetCameraToMinusZRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::plus_X:
        whileBlocking(_cameraCameraForm->resetCameraToPlusXRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::plus_Y:
        whileBlocking(_cameraCameraForm->resetCameraToPlusYRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::plus_Z:
        whileBlocking(_cameraCameraForm->resetCameraToPlusZRadioButton)->setChecked(true);
        break;
     }
  }
}

void CameraTreeWidgetController::reloadCameraProjection()
{
  _cameraCameraForm->resetPercentageSpinBox->setDisabled(true);
  _cameraCameraForm->orthographicCameraRadioButton->setDisabled(true);
  _cameraCameraForm->perspectiveCameraRadioButton->setDisabled(true);

  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _cameraCameraForm->resetPercentageSpinBox->setEnabled(true);
    _cameraCameraForm->orthographicCameraRadioButton->setEnabled(true);
    _cameraCameraForm->perspectiveCameraRadioButton->setEnabled(true);

    whileBlocking(_cameraCameraForm->resetPercentageSpinBox)->setValue(100.0*camera->resetFraction());
    if(camera->isOrthographic()) whileBlocking(_cameraCameraForm->orthographicCameraRadioButton)->setChecked(true);
    if(camera->isPerspective()) whileBlocking(_cameraCameraForm->perspectiveCameraRadioButton)->setChecked(true);
  }
}

void CameraTreeWidgetController::reloadCameraProperties()
{
  _cameraCameraForm->resetCameraToMinusXRadioButton->setDisabled(true);
  _cameraCameraForm->resetCameraToMinusYRadioButton->setDisabled(true);
  _cameraCameraForm->resetCameraToMinusZRadioButton->setDisabled(true);
  _cameraCameraForm->resetCameraToPlusXRadioButton->setDisabled(true);
  _cameraCameraForm->resetCameraToPlusYRadioButton->setDisabled(true);
  _cameraCameraForm->resetCameraToPlusZRadioButton->setDisabled(true);
  _cameraCameraForm->angleOfViewSpinBox->setDisabled(true);
  _cameraCameraForm->centerOfSceneXLineEdit->setDisabled(true);
  _cameraCameraForm->centerOfSceneYLineEdit->setDisabled(true);
  _cameraCameraForm->centerOfSceneZLineEdit->setDisabled(true);

  _cameraCameraForm->rotateAngleSpinBox->setDisabled(true);
  _cameraCameraForm->rotatePlusXPushButton->setDisabled(true);
  _cameraCameraForm->rotatePlusYPushButton->setDisabled(true);
  _cameraCameraForm->rotatePlusZPushButton->setDisabled(true);
  _cameraCameraForm->rotateMinusXPushButton->setDisabled(true);
  _cameraCameraForm->rotateMinusYPushButton->setDisabled(true);
  _cameraCameraForm->rotateMinusZPushButton->setDisabled(true);

  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _cameraCameraForm->resetCameraToMinusXRadioButton->setEnabled(true);
    _cameraCameraForm->resetCameraToMinusYRadioButton->setEnabled(true);
    _cameraCameraForm->resetCameraToMinusZRadioButton->setEnabled(true);
    _cameraCameraForm->resetCameraToPlusXRadioButton->setEnabled(true);
    _cameraCameraForm->resetCameraToPlusYRadioButton->setEnabled(true);
    _cameraCameraForm->resetCameraToPlusZRadioButton->setEnabled(true);
    _cameraCameraForm->angleOfViewSpinBox->setEnabled(true);
    _cameraCameraForm->centerOfSceneXLineEdit->setEnabled(true);
    _cameraCameraForm->centerOfSceneYLineEdit->setEnabled(true);
    _cameraCameraForm->centerOfSceneZLineEdit->setEnabled(true);

    _cameraCameraForm->rotateAngleSpinBox->setEnabled(true);
    _cameraCameraForm->rotatePlusXPushButton->setEnabled(true);
    _cameraCameraForm->rotatePlusYPushButton->setEnabled(true);
    _cameraCameraForm->rotatePlusZPushButton->setEnabled(true);
    _cameraCameraForm->rotateMinusXPushButton->setEnabled(true);
    _cameraCameraForm->rotateMinusYPushButton->setEnabled(true);
    _cameraCameraForm->rotateMinusZPushButton->setEnabled(true);

    switch(camera->resetCameraDirection())
    {
      case ResetDirectionType::minus_X:
        whileBlocking(_cameraCameraForm->resetCameraToMinusXRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::minus_Y:
        whileBlocking(_cameraCameraForm->resetCameraToMinusYRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::minus_Z:
        whileBlocking(_cameraCameraForm->resetCameraToMinusZRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::plus_X:
        whileBlocking(_cameraCameraForm->resetCameraToPlusXRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::plus_Y:
        whileBlocking(_cameraCameraForm->resetCameraToPlusYRadioButton)->setChecked(true);
        break;
      case ResetDirectionType::plus_Z:
        whileBlocking(_cameraCameraForm->resetCameraToPlusZRadioButton)->setChecked(true);
        break;
     }

     whileBlocking(_cameraCameraForm->angleOfViewSpinBox)->setValue(camera->angleOfView()*180.0/M_PI);

    double3 centerOfScene = camera->centerOfScene();
    whileBlocking(_cameraCameraForm->centerOfSceneXLineEdit)->setText(QString::number(centerOfScene.x, 'f', 5));
    whileBlocking(_cameraCameraForm->centerOfSceneYLineEdit)->setText(QString::number(centerOfScene.y, 'f', 5));
    whileBlocking(_cameraCameraForm->centerOfSceneZLineEdit)->setText(QString::number(centerOfScene.z, 'f', 5));

    double rotationAngle = camera->rotationAngle();
    whileBlocking(_cameraCameraForm->rotateAngleSpinBox)->setValue(rotationAngle);
    whileBlocking(_cameraCameraForm->rotatePlusXPushButton)->setText(tr("Rotate +%1").arg(QString::number(rotationAngle, 'f', 2)));
    whileBlocking(_cameraCameraForm->rotatePlusYPushButton)->setText(tr("Rotate +%1").arg(QString::number(rotationAngle, 'f', 2)));
    whileBlocking(_cameraCameraForm->rotatePlusZPushButton)->setText(tr("Rotate +%1").arg(QString::number(rotationAngle, 'f', 2)));
    whileBlocking(_cameraCameraForm->rotateMinusXPushButton)->setText(tr("Rotate -%1").arg(QString::number(rotationAngle, 'f', 2)));
    whileBlocking(_cameraCameraForm->rotateMinusYPushButton)->setText(tr("Rotate -%1").arg(QString::number(rotationAngle, 'f', 2)));
    whileBlocking(_cameraCameraForm->rotateMinusZPushButton)->setText(tr("Rotate -%1").arg(QString::number(rotationAngle, 'f', 2)));
  }
}

void CameraTreeWidgetController::reloadCameraEulerAngles()
{
  _cameraCameraForm->EulerAngleXDial->setDisabled(true);
  _cameraCameraForm->EulerAngleXDial->setDisabled(true);
  _cameraCameraForm->EulerAngleXDial->setDisabled(true);
  _cameraCameraForm->EulerAngleZDial->setDisabled(true);
  _cameraCameraForm->EulerAngleZDial->setDisabled(true);
  _cameraCameraForm->EulerAngleZDial->setDisabled(true);
  _cameraCameraForm->EulerAngleYSlider->setDisabled(true);
  _cameraCameraForm->EulerAngleYSlider->setDisabled(true);
  _cameraCameraForm->EulerAngleYSlider->setDisabled(true);

  _cameraCameraForm->EulerAngleXSpinBox->setDisabled(true);
  _cameraCameraForm->EulerAngleZSpinBox->setDisabled(true);
  _cameraCameraForm->EulerAngleYSpinBox->setDisabled(true);

  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _cameraCameraForm->EulerAngleXDial->setEnabled(true);
    _cameraCameraForm->EulerAngleXDial->setEnabled(true);
    _cameraCameraForm->EulerAngleXDial->setEnabled(true);
    _cameraCameraForm->EulerAngleZDial->setEnabled(true);
    _cameraCameraForm->EulerAngleZDial->setEnabled(true);
    _cameraCameraForm->EulerAngleZDial->setEnabled(true);
    _cameraCameraForm->EulerAngleYSlider->setEnabled(true);
    _cameraCameraForm->EulerAngleYSlider->setEnabled(true);
    _cameraCameraForm->EulerAngleYSlider->setEnabled(true);

    _cameraCameraForm->EulerAngleXSpinBox->setEnabled(true);
    _cameraCameraForm->EulerAngleZSpinBox->setEnabled(true);
    _cameraCameraForm->EulerAngleYSpinBox->setEnabled(true);

    double3 EulerAngles = camera->EulerAngles();
    whileBlocking(_cameraCameraForm->EulerAngleXDial)->setValue(EulerAngles.x*180.0/M_PI);
    whileBlocking(_cameraCameraForm->EulerAngleZDial)->setValue(EulerAngles.z*180.0/M_PI);
    whileBlocking(_cameraCameraForm->EulerAngleYSlider)->setValue(EulerAngles.y*180.0/M_PI);

    whileBlocking(_cameraCameraForm->EulerAngleXSpinBox)->setValue(EulerAngles.x*180.0/M_PI);
    whileBlocking(_cameraCameraForm->EulerAngleZSpinBox)->setValue(EulerAngles.z*180.0/M_PI);
    whileBlocking(_cameraCameraForm->EulerAngleYSpinBox)->setValue(EulerAngles.y*180.0/M_PI);
  }
}

void CameraTreeWidgetController::reloadCameraModelViewMatrix()
{
  _cameraCameraForm->viewMatrixM11->setDisabled(true);
  _cameraCameraForm->viewMatrixM12->setDisabled(true);
  _cameraCameraForm->viewMatrixM13->setDisabled(true);
  _cameraCameraForm->viewMatrixM14->setDisabled(true);

  _cameraCameraForm->viewMatrixM21->setDisabled(true);
  _cameraCameraForm->viewMatrixM22->setDisabled(true);
  _cameraCameraForm->viewMatrixM23->setDisabled(true);
  _cameraCameraForm->viewMatrixM24->setDisabled(true);

  _cameraCameraForm->viewMatrixM31->setDisabled(true);
  _cameraCameraForm->viewMatrixM32->setDisabled(true);
  _cameraCameraForm->viewMatrixM33->setDisabled(true);
  _cameraCameraForm->viewMatrixM34->setDisabled(true);

  _cameraCameraForm->viewMatrixM41->setDisabled(true);
  _cameraCameraForm->viewMatrixM42->setDisabled(true);
  _cameraCameraForm->viewMatrixM43->setDisabled(true);
  _cameraCameraForm->viewMatrixM44->setDisabled(true);

  _cameraCameraForm->cameraPositionXLineEdit->setDisabled(true);
  _cameraCameraForm->cameraPositionYLineEdit->setDisabled(true);
  _cameraCameraForm->cameraPositionZLineEdit->setDisabled(true);
  _cameraCameraForm->cameraDistanceLineEdit->setDisabled(true);

  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _cameraCameraForm->viewMatrixM11->setEnabled(true);
    _cameraCameraForm->viewMatrixM12->setEnabled(true);
    _cameraCameraForm->viewMatrixM13->setEnabled(true);
    _cameraCameraForm->viewMatrixM14->setEnabled(true);

    _cameraCameraForm->viewMatrixM21->setEnabled(true);
    _cameraCameraForm->viewMatrixM22->setEnabled(true);
    _cameraCameraForm->viewMatrixM23->setEnabled(true);
    _cameraCameraForm->viewMatrixM24->setEnabled(true);

    _cameraCameraForm->viewMatrixM31->setEnabled(true);
    _cameraCameraForm->viewMatrixM32->setEnabled(true);
    _cameraCameraForm->viewMatrixM33->setEnabled(true);
    _cameraCameraForm->viewMatrixM34->setEnabled(true);

    _cameraCameraForm->viewMatrixM41->setEnabled(true);
    _cameraCameraForm->viewMatrixM42->setEnabled(true);
    _cameraCameraForm->viewMatrixM43->setEnabled(true);
    _cameraCameraForm->viewMatrixM44->setEnabled(true);

    _cameraCameraForm->cameraPositionXLineEdit->setEnabled(true);
    _cameraCameraForm->cameraPositionYLineEdit->setEnabled(true);
    _cameraCameraForm->cameraPositionZLineEdit->setEnabled(true);
    _cameraCameraForm->cameraDistanceLineEdit->setEnabled(true);

    double4x4 viewMatrix = camera->modelViewMatrix();
    whileBlocking(_cameraCameraForm->viewMatrixM11)->setText(QString::number(viewMatrix.m11, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM12)->setText(QString::number(viewMatrix.m12, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM13)->setText(QString::number(viewMatrix.m13, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM14)->setText(QString::number(viewMatrix.m14, 'f', 5));

    whileBlocking(_cameraCameraForm->viewMatrixM21)->setText(QString::number(viewMatrix.m21, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM22)->setText(QString::number(viewMatrix.m22, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM23)->setText(QString::number(viewMatrix.m23, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM24)->setText(QString::number(viewMatrix.m24, 'f', 5));

    whileBlocking(_cameraCameraForm->viewMatrixM31)->setText(QString::number(viewMatrix.m31, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM32)->setText(QString::number(viewMatrix.m32, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM33)->setText(QString::number(viewMatrix.m33, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM34)->setText(QString::number(viewMatrix.m34, 'f', 5));

    whileBlocking(_cameraCameraForm->viewMatrixM41)->setText(QString::number(viewMatrix.m41, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM42)->setText(QString::number(viewMatrix.m42, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM43)->setText(QString::number(viewMatrix.m43, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM44)->setText(QString::number(viewMatrix.m44, 'f', 5));

    double3 cameraPosition = camera->position();
    whileBlocking(_cameraCameraForm->cameraPositionXLineEdit)->setText(QString::number(cameraPosition.x, 'f', 5));
    whileBlocking(_cameraCameraForm->cameraPositionYLineEdit)->setText(QString::number(cameraPosition.y, 'f', 5));
    whileBlocking(_cameraCameraForm->cameraPositionZLineEdit)->setText(QString::number(cameraPosition.z, 'f', 5));
    whileBlocking(_cameraCameraForm->cameraDistanceLineEdit)->setText(QString::number(cameraPosition.length(), 'f', 5));
  }
}

void CameraTreeWidgetController::reloadCameraMovement()
{
  _cameraCameraForm->EulerAngleXDial->setDisabled(true);
  _cameraCameraForm->EulerAngleXDial->setDisabled(true);
  _cameraCameraForm->EulerAngleXDial->setDisabled(true);
  _cameraCameraForm->EulerAngleZDial->setDisabled(true);
  _cameraCameraForm->EulerAngleZDial->setDisabled(true);
  _cameraCameraForm->EulerAngleZDial->setDisabled(true);
  _cameraCameraForm->EulerAngleYSlider->setDisabled(true);
  _cameraCameraForm->EulerAngleYSlider->setDisabled(true);
  _cameraCameraForm->EulerAngleYSlider->setDisabled(true);

  _cameraCameraForm->EulerAngleXSpinBox->setDisabled(true);
  _cameraCameraForm->EulerAngleZSpinBox->setDisabled(true);
  _cameraCameraForm->EulerAngleYSpinBox->setDisabled(true);

  _cameraCameraForm->viewMatrixM11->setDisabled(true);
  _cameraCameraForm->viewMatrixM12->setDisabled(true);
  _cameraCameraForm->viewMatrixM13->setDisabled(true);
  _cameraCameraForm->viewMatrixM14->setDisabled(true);

  _cameraCameraForm->viewMatrixM21->setDisabled(true);
  _cameraCameraForm->viewMatrixM22->setDisabled(true);
  _cameraCameraForm->viewMatrixM23->setDisabled(true);
  _cameraCameraForm->viewMatrixM24->setDisabled(true);

  _cameraCameraForm->viewMatrixM31->setDisabled(true);
  _cameraCameraForm->viewMatrixM32->setDisabled(true);
  _cameraCameraForm->viewMatrixM33->setDisabled(true);
  _cameraCameraForm->viewMatrixM34->setDisabled(true);

  _cameraCameraForm->viewMatrixM41->setDisabled(true);
  _cameraCameraForm->viewMatrixM42->setDisabled(true);
  _cameraCameraForm->viewMatrixM43->setDisabled(true);
  _cameraCameraForm->viewMatrixM44->setDisabled(true);

  _cameraCameraForm->cameraPositionXLineEdit->setDisabled(true);
  _cameraCameraForm->cameraPositionYLineEdit->setDisabled(true);
  _cameraCameraForm->cameraPositionZLineEdit->setDisabled(true);
  _cameraCameraForm->cameraDistanceLineEdit->setDisabled(true);

  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _cameraCameraForm->EulerAngleXDial->setEnabled(true);
    _cameraCameraForm->EulerAngleXDial->setEnabled(true);
    _cameraCameraForm->EulerAngleXDial->setEnabled(true);
    _cameraCameraForm->EulerAngleZDial->setEnabled(true);
    _cameraCameraForm->EulerAngleZDial->setEnabled(true);
    _cameraCameraForm->EulerAngleZDial->setEnabled(true);
    _cameraCameraForm->EulerAngleYSlider->setEnabled(true);
    _cameraCameraForm->EulerAngleYSlider->setEnabled(true);
    _cameraCameraForm->EulerAngleYSlider->setEnabled(true);

    _cameraCameraForm->EulerAngleXSpinBox->setEnabled(true);
    _cameraCameraForm->EulerAngleZSpinBox->setEnabled(true);
    _cameraCameraForm->EulerAngleYSpinBox->setEnabled(true);

    _cameraCameraForm->viewMatrixM11->setEnabled(true);
    _cameraCameraForm->viewMatrixM12->setEnabled(true);
    _cameraCameraForm->viewMatrixM13->setEnabled(true);
    _cameraCameraForm->viewMatrixM14->setEnabled(true);

    _cameraCameraForm->viewMatrixM21->setEnabled(true);
    _cameraCameraForm->viewMatrixM22->setEnabled(true);
    _cameraCameraForm->viewMatrixM23->setEnabled(true);
    _cameraCameraForm->viewMatrixM24->setEnabled(true);

    _cameraCameraForm->viewMatrixM31->setEnabled(true);
    _cameraCameraForm->viewMatrixM32->setEnabled(true);
    _cameraCameraForm->viewMatrixM33->setEnabled(true);
    _cameraCameraForm->viewMatrixM34->setEnabled(true);

    _cameraCameraForm->viewMatrixM41->setEnabled(true);
    _cameraCameraForm->viewMatrixM42->setEnabled(true);
    _cameraCameraForm->viewMatrixM43->setEnabled(true);
    _cameraCameraForm->viewMatrixM44->setEnabled(true);

    _cameraCameraForm->cameraPositionXLineEdit->setEnabled(true);
    _cameraCameraForm->cameraPositionYLineEdit->setEnabled(true);
    _cameraCameraForm->cameraPositionZLineEdit->setEnabled(true);
    _cameraCameraForm->cameraDistanceLineEdit->setEnabled(true);

    double3 EulerAngles = camera->EulerAngles();
    whileBlocking(_cameraCameraForm->EulerAngleXDial)->setValue(EulerAngles.x*180.0/M_PI);
    whileBlocking(_cameraCameraForm->EulerAngleZDial)->setValue(EulerAngles.z*180.0/M_PI);
    whileBlocking(_cameraCameraForm->EulerAngleYSlider)->setValue(EulerAngles.y*180.0/M_PI);

    whileBlocking(_cameraCameraForm->EulerAngleXSpinBox)->setValue(EulerAngles.x*180.0/M_PI);
    whileBlocking(_cameraCameraForm->EulerAngleZSpinBox)->setValue(EulerAngles.z*180.0/M_PI);
    whileBlocking(_cameraCameraForm->EulerAngleYSpinBox)->setValue(EulerAngles.y*180.0/M_PI);

    double4x4 viewMatrix = camera->modelViewMatrix();
    whileBlocking(_cameraCameraForm->viewMatrixM11)->setText(QString::number(viewMatrix.m11, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM12)->setText(QString::number(viewMatrix.m12, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM13)->setText(QString::number(viewMatrix.m13, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM14)->setText(QString::number(viewMatrix.m14, 'f', 5));

    whileBlocking(_cameraCameraForm->viewMatrixM21)->setText(QString::number(viewMatrix.m21, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM22)->setText(QString::number(viewMatrix.m22, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM23)->setText(QString::number(viewMatrix.m23, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM24)->setText(QString::number(viewMatrix.m24, 'f', 5));

    whileBlocking(_cameraCameraForm->viewMatrixM31)->setText(QString::number(viewMatrix.m31, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM32)->setText(QString::number(viewMatrix.m32, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM33)->setText(QString::number(viewMatrix.m33, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM34)->setText(QString::number(viewMatrix.m34, 'f', 5));

    whileBlocking(_cameraCameraForm->viewMatrixM41)->setText(QString::number(viewMatrix.m41, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM42)->setText(QString::number(viewMatrix.m42, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM43)->setText(QString::number(viewMatrix.m43, 'f', 5));
    whileBlocking(_cameraCameraForm->viewMatrixM44)->setText(QString::number(viewMatrix.m44, 'f', 5));

    double3 cameraPosition = camera->position();
    whileBlocking(_cameraCameraForm->cameraPositionXLineEdit)->setText(QString::number(cameraPosition.x, 'f', 5));
    whileBlocking(_cameraCameraForm->cameraPositionYLineEdit)->setText(QString::number(cameraPosition.y, 'f', 5));
    whileBlocking(_cameraCameraForm->cameraPositionZLineEdit)->setText(QString::number(cameraPosition.z, 'f', 5));
    whileBlocking(_cameraCameraForm->cameraDistanceLineEdit)->setText(QString::number(cameraPosition.length(), 'f', 5));
  }
}

void CameraTreeWidgetController::reloadCameraData()
{
  reloadCameraProperties();
  reloadCameraProjection();
  reloadCameraResetDirection();
  reloadCameraEulerAngles();
  reloadCameraModelViewMatrix();

  reloadAxesProperties();
  reloadAxesBackgroundProperties();
  reloadAxesTextProperties();

  reloadSelectionProperties();
  reloadLightsProperties();
  reloadPictureProperties();
  reloadBackgroundProperties();
}





void CameraTreeWidgetController::setResetPercentage(double resetPercentage)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->setResetFraction(resetPercentage/100.0);

    reloadCameraProjection();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}


void CameraTreeWidgetController::resetCameraToDefaultDirection()
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->resetCameraToDirection();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}


void CameraTreeWidgetController::setResetDirectionToMinusX(bool checked)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    if(checked)
    {
      camera->setResetDirectionType(ResetDirectionType::minus_X);
      emit updateRenderer();
      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setResetDirectionToPlusX(bool checked)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    if(checked)
    {
      camera->setResetDirectionType(ResetDirectionType::plus_X);
      emit updateRenderer();
      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setResetDirectionToMinusY(bool checked)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    if(checked)
    {
      camera->setResetDirectionType(ResetDirectionType::minus_Y);
      emit updateRenderer();
      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setResetDirectionToPlusY(bool checked)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    if(checked)
    {
      camera->setResetDirectionType(ResetDirectionType::plus_Y);
      emit updateRenderer();
      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setResetDirectionToMinusZ(bool checked)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    if(checked)
    {
      camera->setResetDirectionType(ResetDirectionType::minus_Z);
      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setResetDirectionToPlusZ(bool checked)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    if(checked)
    {
      camera->setResetDirectionType(ResetDirectionType::plus_Z);
      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setCameraToOrthographic(bool checked)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    if(checked)
    {
      camera->setCameraToOrthographic();
    }
    else
    {
      camera->setCameraToPerspective();
    }
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setCameraToPerspective(bool checked)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    if(checked)
    {
      camera->setCameraToPerspective();
    }
    else
    {
      camera->setCameraToOrthographic();
    }
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAngleOfView(double d)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->setAngleOfView(d * M_PI/180.0);
    emit updateRenderer();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setRotationAngle(double angle)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->setRotationAngle(angle);
    QString angleString = QString::number(angle);
    whileBlocking(_cameraCameraForm->rotatePlusXPushButton)->setText(tr("Rotate +%1").arg(angleString));
    whileBlocking(_cameraCameraForm->rotatePlusYPushButton)->setText(tr("Rotate +%1").arg(angleString));
    whileBlocking(_cameraCameraForm->rotatePlusZPushButton)->setText(tr("Rotate +%1").arg(angleString));
    whileBlocking(_cameraCameraForm->rotateMinusXPushButton)->setText(tr("Rotate -%1").arg(angleString));
    whileBlocking(_cameraCameraForm->rotateMinusYPushButton)->setText(tr("Rotate -%1").arg(angleString));
    whileBlocking(_cameraCameraForm->rotateMinusZPushButton)->setText(tr("Rotate -%1").arg(angleString));

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::rotateYawPlus()
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    simd_quatd rotation = camera->worldRotation();
    simd_quatd dq = simd_quatd:: yaw(camera->rotationAngle());

    camera->setWorldRotation(rotation * dq);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::rotateYawMinus()
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    simd_quatd rotation = camera->worldRotation();
    simd_quatd dq = simd_quatd:: yaw(-camera->rotationAngle());

    camera->setWorldRotation(rotation * dq);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::rotatePitchPlus()
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    simd_quatd rotation = camera->worldRotation();
    simd_quatd dq = simd_quatd:: pitch(camera->rotationAngle());

    camera->setWorldRotation(rotation * dq);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::rotatePitchMinus()
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    simd_quatd rotation = camera->worldRotation();
    simd_quatd dq = simd_quatd:: pitch(-camera->rotationAngle());

    camera->setWorldRotation(rotation * dq);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::rotateRollPlus()
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    simd_quatd rotation = camera->worldRotation();
    simd_quatd dq = simd_quatd:: roll(camera->rotationAngle());

    camera->setWorldRotation(rotation * dq);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::rotateRollMinus()
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    simd_quatd rotation = camera->worldRotation();
    simd_quatd dq = simd_quatd:: roll(-camera->rotationAngle());

    camera->setWorldRotation(rotation * dq);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setEulerAngleX(int angle)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    double3 EulerAngles  = camera->EulerAngles();
    double newValue = double(angle*M_PI/180.0);

    simd_quatd worldRotation = simd_quatd(double3(newValue, EulerAngles.y, EulerAngles.z));
    camera->setWorldRotation(worldRotation);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setEulerAngleX(double angle)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    double3 EulerAngles  = camera->EulerAngles();
    double newValue = double(angle*M_PI/180.0);

    simd_quatd worldRotation = simd_quatd(double3(newValue, EulerAngles.y, EulerAngles.z));
    camera->setWorldRotation(worldRotation);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setEulerAngleZ(int angle)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    double3 EulerAngles  = camera->EulerAngles();
    double newValue = double(angle*M_PI/180.0);

    simd_quatd worldRotation = simd_quatd(double3(EulerAngles.x, EulerAngles.y, newValue));
    camera->setWorldRotation(worldRotation);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setEulerAngleZ(double angle)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    double3 EulerAngles  = camera->EulerAngles();
    double newValue = double(angle*M_PI/180.0);

    simd_quatd worldRotation = simd_quatd(double3(EulerAngles.x, EulerAngles.y, newValue));
    camera->setWorldRotation(worldRotation);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setEulerAngleY(int angle)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    double3 EulerAngles  = camera->EulerAngles();
    double newValue = double(angle*M_PI/180.0);

    simd_quatd worldRotation = simd_quatd(double3(EulerAngles.x, newValue, EulerAngles.z));
    camera->setWorldRotation(worldRotation);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setEulerAngleY(double angle)
{
  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    double3 EulerAngles  = camera->EulerAngles();
    double newValue = double(angle*M_PI/180.0);

    simd_quatd worldRotation = simd_quatd(double3(EulerAngles.x, newValue, EulerAngles.z));
    camera->setWorldRotation(worldRotation);
    emit updateRenderer();
    reloadCameraEulerAngles();
    reloadCameraModelViewMatrix();

    _mainWindow->documentWasModified();
  }
}


// Selection properties
//========================================================================================================================================

void CameraTreeWidgetController::reloadSelectionProperties()
{
  _cameraSelectionForm->selectionIntensityDoubleSpinBox->setDisabled(true);
  _cameraSelectionForm->selectionIntensityDoubleSlider->setDisabled(true);

  if(std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _cameraSelectionForm->selectionIntensityDoubleSpinBox->setEnabled(true);
    _cameraSelectionForm->selectionIntensityDoubleSlider->setEnabled(true);

    whileBlocking(_cameraSelectionForm->selectionIntensityDoubleSpinBox)->setValue(camera->bloomLevel());
    whileBlocking(_cameraSelectionForm->selectionIntensityDoubleSlider)->setDoubleValue(camera->bloomLevel());
  }
}

void CameraTreeWidgetController::setSelectionIntensity(double level)
{
  _cameraSelectionForm->selectionIntensityDoubleSpinBox->setDisabled(true);
  _cameraSelectionForm->selectionIntensityDoubleSlider->setDisabled(true);

  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _cameraSelectionForm->selectionIntensityDoubleSpinBox->setEnabled(true);
    _cameraSelectionForm->selectionIntensityDoubleSlider->setEnabled(true);

    camera->setBloomLevel(level);
    whileBlocking(_cameraSelectionForm->selectionIntensityDoubleSpinBox)->setValue(level);
    whileBlocking(_cameraSelectionForm->selectionIntensityDoubleSlider)->setDoubleValue(level);
  }
  emit rendererReloadSelectionData();
  emit updateRenderer();
}

// Global Axes properties
//========================================================================================================================================

void CameraTreeWidgetController::reloadAxesProperties()
{
  _cameraAxesForm->cameraAxesPosition->setDisabled(true);
  _cameraAxesForm->cameraAxesStyle->setDisabled(true);
  _cameraAxesForm->cameraAxesSize->setDisabled(true);
  _cameraAxesForm->cameraAxesOffset->setDisabled(true);

  if(_project)
  {
    _cameraAxesForm->cameraAxesPosition->setEnabled(true);
    int positionIndex = static_cast<typename std::underlying_type<RKGlobalAxes::Position>::type>(_project->axes()->position());
    whileBlocking(_cameraAxesForm->cameraAxesPosition)->setCurrentIndex(positionIndex);

    if(_project->axes()->position() != RKGlobalAxes::Position::none)
    {
      _cameraAxesForm->cameraAxesStyle->setEnabled(true);
      int styleIndex = static_cast<typename std::underlying_type<RKGlobalAxes::Style>::type>(_project->axes()->style());
      whileBlocking(_cameraAxesForm->cameraAxesStyle)->setCurrentIndex(styleIndex);

      _cameraAxesForm->cameraAxesSize->setEnabled(true);
      whileBlocking(_cameraAxesForm->cameraAxesSize)->setValue(100.0 * _project->axes()->sizeScreenFraction());

      _cameraAxesForm->cameraAxesOffset->setEnabled(true);
      whileBlocking(_cameraAxesForm->cameraAxesOffset)->setValue(100.0 * _project->axes()->borderOffsetScreenFraction());
    }
  }
}



void CameraTreeWidgetController::setAxesPosition(int value)
{
  if(_project)
  {
    if(value>=0 && value<=int(RKGlobalAxes::Position::center))
    {
      _project->axes()->setPosition(RKGlobalAxes::Position(value));

      reloadAxesProperties();
      reloadAxesBackgroundProperties();
      reloadAxesTextProperties();

      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setAxesStyle(int value)
{
  if(_project)
  {
    if(value>=0 && value<=int(RKGlobalAxes::Style::squashed))
    {
      _project->axes()->setStyle(RKGlobalAxes::Style(value));

      reloadAxesProperties();

      emit rendererReloadData();
      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setAxesSize(double value)
{
  if(_project)
  {
    _project->axes()->setSizeScreenFraction(value/100.0);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesOffset(double value)
{
  if(_project)
  {
    _project->axes()->setBorderOffsetScreenFraction(value/100.0);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::reloadAxesBackgroundProperties()
{
  _cameraAxesForm->cameraAxesBackgroundShape->setDisabled(true);
  _cameraAxesForm->cameraAxesBackgroundColor->setDisabled(true);
  _cameraAxesForm->cameraAxesBackgroundAdditionalSize->setDisabled(true);
  if(_project)
  {
    if(_project->axes()->position() != RKGlobalAxes::Position::none)
    {
      _cameraAxesForm->cameraAxesBackgroundShape->setEnabled(true);
      _cameraAxesForm->cameraAxesBackgroundColor->setEnabled(true);
      _cameraAxesForm->cameraAxesBackgroundAdditionalSize->setEnabled(true);
      int positionIndex = static_cast<typename std::underlying_type<RKGlobalAxes::BackgroundStyle>::type>(_project->axes()->axesBackgroundStyle());
      whileBlocking(_cameraAxesForm->cameraAxesBackgroundShape)->setCurrentIndex(positionIndex);

      whileBlocking(_cameraAxesForm->cameraAxesBackgroundColor)->setColor(_project->axes()->axesBackgroundColor());
      whileBlocking(_cameraAxesForm->cameraAxesBackgroundAdditionalSize)->setValue(_project->axes()->axesBackgroundAdditionalSize());
    }
  }
}


void CameraTreeWidgetController::setAxesBackgroundStyle(int value)
{
  if(_project)
  {
    if(value>=0 && value<=int(RKGlobalAxes::Style::squashed))
    {
      _project->axes()->setAxesBackgroundStyle(RKGlobalAxes::BackgroundStyle(value));

      reloadAxesBackgroundProperties();

      emit rendererReloadData();
      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setAxesBackgroundColor()
{
  if (_project)
  {
    QColor color = QColorDialog::getColor(_project->axes()->axesBackgroundColor(),this,"Choose Color",QColorDialog::ShowAlphaChannel);
    if(color.isValid())
    {
      _project->axes()->setAxesBackgroundColor(color);

      reloadAxesBackgroundProperties();

      emit rendererReloadData();
      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setAxesBackgroundAdditionalSize(double value)
{
  if(_project)
  {
    _project->axes()->setAxesBackgroundAdditionalSize(value);

    reloadAxesBackgroundProperties();

    emit rendererReloadData();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::reloadAxesTextProperties()
{
  _cameraAxesForm->cameraAxesTextScalingX->setDisabled(true);
  _cameraAxesForm->cameraAxesTextScalingY->setDisabled(true);
  _cameraAxesForm->cameraAxesTextScalingZ->setDisabled(true);

  _cameraAxesForm->cameraAxesTextColorX->setDisabled(true);
  _cameraAxesForm->cameraAxesTextColorY->setDisabled(true);
  _cameraAxesForm->cameraAxesTextColorZ->setDisabled(true);

  _cameraAxesForm->cameraAxesXTextDisplacementX->setDisabled(true);
  _cameraAxesForm->cameraAxesXTextDisplacementY->setDisabled(true);
  _cameraAxesForm->cameraAxesXTextDisplacementZ->setDisabled(true);
  _cameraAxesForm->cameraAxesYTextDisplacementX->setDisabled(true);
  _cameraAxesForm->cameraAxesYTextDisplacementY->setDisabled(true);
  _cameraAxesForm->cameraAxesYTextDisplacementZ->setDisabled(true);
  _cameraAxesForm->cameraAxesZTextDisplacementX->setDisabled(true);
  _cameraAxesForm->cameraAxesZTextDisplacementY->setDisabled(true);
  _cameraAxesForm->cameraAxesZTextDisplacementZ->setDisabled(true);

  if(_project)
  {
    if(_project->axes()->position() != RKGlobalAxes::Position::none)
    {
      _cameraAxesForm->cameraAxesTextScalingX->setEnabled(true);
      _cameraAxesForm->cameraAxesTextScalingY->setEnabled(true);
      _cameraAxesForm->cameraAxesTextScalingZ->setEnabled(true);

      _cameraAxesForm->cameraAxesTextColorX->setEnabled(true);
      _cameraAxesForm->cameraAxesTextColorY->setEnabled(true);
      _cameraAxesForm->cameraAxesTextColorZ->setEnabled(true);

      _cameraAxesForm->cameraAxesXTextDisplacementX->setEnabled(true);
      _cameraAxesForm->cameraAxesXTextDisplacementY->setEnabled(true);
      _cameraAxesForm->cameraAxesXTextDisplacementZ->setEnabled(true);
      _cameraAxesForm->cameraAxesYTextDisplacementX->setEnabled(true);
      _cameraAxesForm->cameraAxesYTextDisplacementY->setEnabled(true);
      _cameraAxesForm->cameraAxesYTextDisplacementZ->setEnabled(true);
      _cameraAxesForm->cameraAxesZTextDisplacementX->setEnabled(true);
      _cameraAxesForm->cameraAxesZTextDisplacementY->setEnabled(true);
      _cameraAxesForm->cameraAxesZTextDisplacementZ->setEnabled(true);

      whileBlocking(_cameraAxesForm->cameraAxesTextScalingX)->setValue(_project->axes()->textScale().x);
      whileBlocking(_cameraAxesForm->cameraAxesTextScalingY)->setValue(_project->axes()->textScale().y);
      whileBlocking(_cameraAxesForm->cameraAxesTextScalingZ)->setValue(_project->axes()->textScale().z);

      whileBlocking(_cameraAxesForm->cameraAxesTextColorX)->setColor(_project->axes()->textColorX());
      whileBlocking(_cameraAxesForm->cameraAxesTextColorY)->setColor(_project->axes()->textColorY());
      whileBlocking(_cameraAxesForm->cameraAxesTextColorZ)->setColor(_project->axes()->textColorZ());

      whileBlocking(_cameraAxesForm->cameraAxesXTextDisplacementX)->setValue(_project->axes()->textDisplacementX().x);
      whileBlocking(_cameraAxesForm->cameraAxesXTextDisplacementY)->setValue(_project->axes()->textDisplacementX().y);
      whileBlocking(_cameraAxesForm->cameraAxesXTextDisplacementZ)->setValue(_project->axes()->textDisplacementX().z);
      whileBlocking(_cameraAxesForm->cameraAxesYTextDisplacementX)->setValue(_project->axes()->textDisplacementY().x);
      whileBlocking(_cameraAxesForm->cameraAxesYTextDisplacementY)->setValue(_project->axes()->textDisplacementY().y);
      whileBlocking(_cameraAxesForm->cameraAxesYTextDisplacementZ)->setValue(_project->axes()->textDisplacementY().z);
      whileBlocking(_cameraAxesForm->cameraAxesZTextDisplacementX)->setValue(_project->axes()->textDisplacementZ().x);
      whileBlocking(_cameraAxesForm->cameraAxesZTextDisplacementY)->setValue(_project->axes()->textDisplacementZ().y);
      whileBlocking(_cameraAxesForm->cameraAxesZTextDisplacementZ)->setValue(_project->axes()->textDisplacementZ().z);
    }
  }
}

void CameraTreeWidgetController::setAxesTextScalingX(double value)
{
  if(_project)
  {
    _project->axes()->setTextScaleX(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesTextScalingY(double value)
{
  if(_project)
  {
    _project->axes()->setTextScaleY(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesTextScalingZ(double value)
{
  if(_project)
  {
    _project->axes()->setTextScaleZ(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}
void CameraTreeWidgetController::setAxesTextolorX()
{
  if (_project)
  {
    QColor color = QColorDialog::getColor(_project->axes()->axesBackgroundColor(),this,"Choose Color");
    if(color.isValid())
    {
      _project->axes()->setTextColorX(color);

      reloadAxesTextProperties();

      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setAxesTextolorY()
{
  if (_project)
  {
    QColor color = QColorDialog::getColor(_project->axes()->axesBackgroundColor(),this,"Choose Color");
    if(color.isValid())
    {
      _project->axes()->setTextColorY(color);

      reloadAxesTextProperties();

      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setAxesTextolorZ()
{
  if (_project)
  {
    QColor color = QColorDialog::getColor(_project->axes()->axesBackgroundColor(),this,"Choose Color");
    if(color.isValid())
    {
      _project->axes()->setTextColorZ(color);

      reloadAxesTextProperties();

      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setAxesXTextDisplacementX(double value)
{
  if(_project)
  {
    _project->axes()->setXTextDisplacementX(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesXTextDisplacementY(double value)
{
  if(_project)
  {
    _project->axes()->setXTextDisplacementY(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesXTextDisplacementZ(double value)
{
  if(_project)
  {
    _project->axes()->setXTextDisplacementZ(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesYTextDisplacementX(double value)
{
  if(_project)
  {
    _project->axes()->setYTextDisplacementX(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesYTextDisplacementY(double value)
{
  if(_project)
  {
    _project->axes()->setYTextDisplacementY(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesYTextDisplacementZ(double value)
{
  if(_project)
  {
    _project->axes()->setYTextDisplacementZ(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesZTextDisplacementX(double value)
{
  if(_project)
  {
    _project->axes()->setZTextDisplacementX(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesZTextDisplacementY(double value)
{
  if(_project)
  {
    _project->axes()->setZTextDisplacementY(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAxesZTextDisplacementZ(double value)
{
  if(_project)
  {
    _project->axes()->setZTextDisplacementZ(value);
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}


// Light properties
//========================================================================================================================================

void CameraTreeWidgetController::reloadLightsProperties()
{
  _cameraLightsForm->ambientLightIntensityDoubleSpinBox->setDisabled(true);
  _cameraLightsForm->ambientLightIntensityDoubleSlider->setDisabled(true);
  _cameraLightsForm->ambientLightPushButton->setDisabled(true);

  _cameraLightsForm->diffuseLightIntensityDoubleSpinBox->setDisabled(true);
  _cameraLightsForm->diffuseLightIntensityDoubleSlider->setDisabled(true);
  _cameraLightsForm->diffuseLightPushButton->setDisabled(true);

  _cameraLightsForm->specularLightIntensityDoubleSpinBox->setDisabled(true);
  _cameraLightsForm->specularLightIntensityDoubleSlider->setDisabled(true);
  _cameraLightsForm->specularLightPushButton->setDisabled(true);

  if(_project)
  {
    _cameraLightsForm->ambientLightIntensityDoubleSpinBox->setEnabled(true);
    _cameraLightsForm->ambientLightIntensityDoubleSlider->setEnabled(true);
    _cameraLightsForm->ambientLightPushButton->setEnabled(true);

    _cameraLightsForm->diffuseLightIntensityDoubleSpinBox->setEnabled(true);
    _cameraLightsForm->diffuseLightIntensityDoubleSlider->setEnabled(true);
    _cameraLightsForm->diffuseLightPushButton->setEnabled(true);

    _cameraLightsForm->specularLightIntensityDoubleSpinBox->setEnabled(true);
    _cameraLightsForm->specularLightIntensityDoubleSlider->setEnabled(true);
    _cameraLightsForm->specularLightPushButton->setEnabled(true);

    double ambientIntensity = _project->renderLights().front()->ambientIntensity();
    double diffuseIntensity = _project->renderLights().front()->diffuseIntensity();
    double specularIntensity = _project->renderLights().front()->specularIntensity();

    QColor ambientColor = _project->renderLights().front()->ambientColor();
    QColor diffuseColor = _project->renderLights().front()->diffuseColor();
    QColor specularColor = _project->renderLights().front()->specularColor();

    whileBlocking(_cameraLightsForm->ambientLightIntensityDoubleSpinBox)->setValue(ambientIntensity);
    whileBlocking(_cameraLightsForm->ambientLightIntensityDoubleSlider)->setValue(ambientIntensity * 1000.0);
    whileBlocking(_cameraLightsForm->ambientLightPushButton)->setColor(ambientColor);

    whileBlocking(_cameraLightsForm->diffuseLightIntensityDoubleSpinBox)->setValue(diffuseIntensity);
    whileBlocking(_cameraLightsForm->diffuseLightIntensityDoubleSlider)->setValue(diffuseIntensity * 1000.0);
    whileBlocking(_cameraLightsForm->diffuseLightPushButton)->setColor(diffuseColor);

    whileBlocking(_cameraLightsForm->specularLightIntensityDoubleSpinBox)->setValue(specularIntensity);
    whileBlocking(_cameraLightsForm->specularLightIntensityDoubleSlider)->setValue(specularIntensity * 1000.0);
    whileBlocking(_cameraLightsForm->specularLightPushButton)->setColor(specularColor);
  }
}

void CameraTreeWidgetController::setAmbientLightIntensity(double intensity)
{
  if (_project)
  {
    _project->renderLights().front()->setAmbientIntensity(intensity / 1000.0);
    reloadLightsProperties();

    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setDiffuseLightIntensity(double intensity)
{
  if (_project)
  {
    _project->renderLights().front()->setDiffuseIntensity(intensity / 1000.0);
    reloadLightsProperties();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setSpecularLightIntensity(double intensity)
{
  if (_project)
  {
    _project->renderLights().front()->setSpecularIntensity(intensity / 1000.0);
    reloadLightsProperties();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setAmbientColor()
{
  if (_project)
  {
    QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
    if(color.isValid())
    {
      _project->renderLights().front()->setAmbientColor(color);
      whileBlocking(_cameraLightsForm->ambientLightPushButton)->setColor(color);
      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setDiffuseColor()
{
  if (_project)
  {
    QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
    if(color.isValid())
    {
      _project->renderLights().front()->setDiffuseColor(color);
      whileBlocking(_cameraLightsForm->diffuseLightPushButton)->setColor(color);
      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

void CameraTreeWidgetController::setSpecularColor()
{
  if (_project)
  {
    QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
    if(color.isValid())
    {
      _project->renderLights().front()->setSpecularColor(color);
      whileBlocking(_cameraLightsForm->specularLightPushButton)->setColor(color);
      emit updateRenderer();

      _mainWindow->documentWasModified();
    }
  }
}

// Picture/movie properties
//========================================================================================================================================

void CameraTreeWidgetController::reloadPictureProperties()
{
  _cameraPicturesForm->inchRadioButton->setDisabled(true);
  _cameraPicturesForm->physicalWidthDoubleSpinBox->setDisabled(true);
  _cameraPicturesForm->physicalHeightDoubleSpinBox->setDisabled(true);
  _cameraPicturesForm->cmRadioButton->setDisabled(true);
  _cameraPicturesForm->physicalWidthDoubleSpinBox->setDisabled(true);
  _cameraPicturesForm->physicalHeightDoubleSpinBox->setDisabled(true);

  _cameraPicturesForm->physicalDimensionsRadioButton->setDisabled(true);
  _cameraPicturesForm->physicalWidthDoubleSpinBox->setDisabled(true);
  _cameraPicturesForm->pixelWidthSpinBox->setDisabled(true);

  _cameraPicturesForm->pixelDimensionsRadioButton->setDisabled(true);
  _cameraPicturesForm->pixelWidthSpinBox->setDisabled(true);
  _cameraPicturesForm->physicalWidthDoubleSpinBox->setDisabled(true);

  _cameraPicturesForm->dotsPerInchComboBox->setDisabled(true);
  _cameraPicturesForm->qualityComboBox->setDisabled(true);

  _cameraPicturesForm->pixelWidthSpinBox->setDisabled(true);
  _cameraPicturesForm->pixelHeightSpinBox->setDisabled(true);

  _cameraPicturesForm->inchRadioButton->setDisabled(true);
  _cameraPicturesForm->cmRadioButton->setDisabled(true);
  _cameraPicturesForm->movieFormatComboBox->setDisabled(true);
  _cameraPicturesForm->movieTypeComboBox->setDisabled(true);
  _cameraPicturesForm->framesPerSecondSpinBox->setDisabled(true);

  _cameraPicturesForm->createPicturePushButton->setDisabled(true);
  _cameraPicturesForm->createMoviePushButton->setDisabled(true);

  if(_project)
  {
    _cameraPicturesForm->inchRadioButton->setEnabled(true);
    _cameraPicturesForm->physicalWidthDoubleSpinBox->setEnabled(true);
    _cameraPicturesForm->physicalHeightDoubleSpinBox->setEnabled(true);
    _cameraPicturesForm->cmRadioButton->setEnabled(true);
    _cameraPicturesForm->physicalWidthDoubleSpinBox->setEnabled(true);
    _cameraPicturesForm->physicalHeightDoubleSpinBox->setEnabled(true);

    _cameraPicturesForm->physicalDimensionsRadioButton->setEnabled(true);
    _cameraPicturesForm->pixelDimensionsRadioButton->setEnabled(true);

    _cameraPicturesForm->dotsPerInchComboBox->setEnabled(true);
    _cameraPicturesForm->qualityComboBox->setEnabled(true);

    _cameraPicturesForm->pixelWidthSpinBox->setEnabled(true);
    _cameraPicturesForm->pixelHeightSpinBox->setEnabled(true);

    _cameraPicturesForm->inchRadioButton->setEnabled(true);
    _cameraPicturesForm->cmRadioButton->setEnabled(true);
    _cameraPicturesForm->movieFormatComboBox->setEnabled(true);
    _cameraPicturesForm->movieTypeComboBox->setEnabled(true);
    _cameraPicturesForm->framesPerSecondSpinBox->setEnabled(true);

    _cameraPicturesForm->createPicturePushButton->setEnabled(true);
    _cameraPicturesForm->createMoviePushButton->setEnabled(true);

    switch(_project->imageUnits())
    {
    case RKImageUnits::inch:
      whileBlocking(_cameraPicturesForm->inchRadioButton)->setChecked(true);
      whileBlocking(_cameraPicturesForm->physicalWidthDoubleSpinBox)->setValue(double(_project->renderImagePhysicalSizeInInches()));
      whileBlocking(_cameraPicturesForm->physicalHeightDoubleSpinBox)->setValue(double(_project->renderImagePhysicalSizeInInches())/(_project->imageAspectRatio()));
      break;
    case RKImageUnits::cm:
      whileBlocking(_cameraPicturesForm->cmRadioButton)->setChecked(true);
      whileBlocking(_cameraPicturesForm->physicalWidthDoubleSpinBox)->setValue(double(2.54 * _project->renderImagePhysicalSizeInInches()));
      whileBlocking(_cameraPicturesForm->physicalHeightDoubleSpinBox)->setValue(double(2.54 * _project->renderImagePhysicalSizeInInches())/(_project->imageAspectRatio()));
      break;
    }

    switch(_project->imageDimensions())
    {
    case RKImageDimensions::physical:
      whileBlocking(_cameraPicturesForm->physicalDimensionsRadioButton)->setChecked(true);
      whileBlocking(_cameraPicturesForm->physicalWidthDoubleSpinBox)->setEnabled(true);
      whileBlocking(_cameraPicturesForm->pixelWidthSpinBox)->setEnabled(false);
      break;
    case RKImageDimensions::pixels:
      whileBlocking(_cameraPicturesForm->pixelDimensionsRadioButton)->setChecked(true);
      whileBlocking(_cameraPicturesForm->pixelWidthSpinBox)->setEnabled(true);
      whileBlocking(_cameraPicturesForm->physicalWidthDoubleSpinBox)->setEnabled(false);
      break;
    }

    whileBlocking(_cameraPicturesForm->dotsPerInchComboBox)->setCurrentIndex(int(_project->imageDPI()));
    whileBlocking(_cameraPicturesForm->qualityComboBox)->setCurrentIndex(int(_project->renderImageQuality()));

    whileBlocking(_cameraPicturesForm->pixelWidthSpinBox)->setValue(_project->renderImageNumberOfPixels());
    whileBlocking(_cameraPicturesForm->pixelHeightSpinBox)->setValue(_project->renderImageNumberOfPixels() / _project->imageAspectRatio());


    switch(_project->imageUnits())
    {
    case RKImageUnits::inch:
      whileBlocking(_cameraPicturesForm->inchRadioButton)->setChecked(true);
      break;
    case RKImageUnits::cm:
      whileBlocking(_cameraPicturesForm->cmRadioButton)->setChecked(true);
      break;
    }

    whileBlocking(_cameraPicturesForm->movieFormatComboBox)->setCurrentIndex(int(_movieFormat));
    whileBlocking(_cameraPicturesForm->movieTypeComboBox)->setCurrentIndex(int(_project->movieType()));

    whileBlocking(_cameraPicturesForm->framesPerSecondSpinBox)->setValue(_project->movieFramesPerSecond());
  }
}

void CameraTreeWidgetController::setPictureDotsPerInch(int value)
{
  if(_project)
  {
    _project->setImageDPI(RKImageDPI(value));

    switch(_project->imageDimensions())
    {
    case RKImageDimensions::physical:
      _project->setImagePhysicalSizeInInches(_project->renderImageNumberOfPixels() / _project->imageDotsPerInchValue());
      break;
    case RKImageDimensions::pixels:
       //_project->setImageNumberOfPixels(int(rint(_project->imageDotsPerInchValue() * _project->renderImagePhysicalSizeInInches())));
        _project->setImagePhysicalSizeInInches(_project->renderImageNumberOfPixels() / _project->imageDotsPerInchValue());
      break;
    }

    reloadPictureProperties();

    _mainWindow->documentWasModified();
  }
}


void CameraTreeWidgetController::setPictureDimensionToPhysical(bool checked)
{
   if(_project && checked)
   {
     std::cout << "check: " << _project.get() <<std::endl;
     _project->setImageDimensions(RKImageDimensions::physical);
     this->reloadPictureProperties();

     _mainWindow->documentWasModified();
   }
}

void CameraTreeWidgetController::setPictureDimensionToPixels(bool checked)
{
   if(_project && checked)
   {
     _project->setImageDimensions(RKImageDimensions::pixels);
     this->reloadPictureProperties();

     _mainWindow->documentWasModified();
   }
}

void CameraTreeWidgetController::setPictureUnitsToInch(bool checked)
{
  if(_project && checked)
  {
    _project->setImageUnits(RKImageUnits::inch);
    this->reloadPictureProperties();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setPictureUnitsToCentimeters(bool checked)
{
  if(_project && checked)
  {
    _project->setImageUnits(RKImageUnits::cm);
    this->reloadPictureProperties();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setPictureQuality(int value)
{
  if(_project)
  {
    _project->setImageQuality(RKImageQuality(value));

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setPictureNumberOfPixels(int width)
{
  if(_project)
  {
    _project->setImageNumberOfPixels(width);

    _project->setImagePhysicalSizeInInches(_project->renderImageNumberOfPixels() / _project->imageDotsPerInchValue());
    reloadPictureProperties();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setPicturePhysicalSize(double width)
{
  if(_project)
  {
    switch(_project->imageUnits())
    {
    case RKImageUnits::inch:
      _project->setImagePhysicalSizeInInches(width);
      break;
    case RKImageUnits::cm:
      _project->setImagePhysicalSizeInInches(width/2.54);
      break;
    }

    _project->setImageNumberOfPixels(int(rint(_project->imageDotsPerInchValue() * _project->renderImagePhysicalSizeInInches())));


    reloadPictureProperties();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setPictureEditDimensions(int dimensions)
{
  if(_project)
  {
    _project->setImageDimensions(RKImageDimensions(dimensions));
    reloadPictureProperties();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setPictureEditUnits(int units)
{
  if(_project)
  {
    _project->setImageUnits(RKImageUnits(units));
    reloadPictureProperties();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setMovieFramesPerSecond(int fps)
{
  if(_project)
  {
    _project->setMovieFramesPerSecond(fps);

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::savePicture()
{
  SavePictureDialog dialog(this);

  if(dialog.exec() == QDialog::Accepted)
  {
    QList<QUrl> fileURLs = dialog.selectedUrls();
    if(fileURLs.isEmpty())
    {
      return;
    }
    else
    {
      if(_project)
      {
        int width = _project->renderImageNumberOfPixels();
        int height = _project->renderImageNumberOfPixels() / _project->imageAspectRatio();
        emit rendererCreatePicture(fileURLs.first(), width, height);
      }
    }
  }
}

void CameraTreeWidgetController::saveMovie()
{
  SaveMovieDialog dialog(this);

  if(dialog.exec() == QDialog::Accepted)
  {
    QList<QUrl> fileURLs = dialog.selectedUrls();
    if(fileURLs.isEmpty())
    {
      return;
    }
    else
    {
      if(_project)
      {
        int width = _project->renderImageNumberOfPixels();
        int height = _project->renderImageNumberOfPixels() / _project->imageAspectRatio();
        ProjectStructure::MovieType movieType = _project->movieType();
        emit rendererCreateMovie(fileURLs.first(), width, height, _movieFormat, movieType);
      }
    }
  }
}

void CameraTreeWidgetController::setNumberOfFramesPerSecond(int value)
{
  if(_project)
  {
    _project->setMovieFramesPerSecond(value);

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setMovieFormat(int value)
{
  _movieFormat = MovieWriter::Format(value);

  _mainWindow->documentWasModified();
}

void CameraTreeWidgetController::setMovieType(int value)
{
  if(_project)
  {
    _project->setMovieType(ProjectStructure::MovieType(value));

    _mainWindow->documentWasModified();
  }
}

// Background properties
//========================================================================================================================================

void CameraTreeWidgetController::reloadBackgroundProperties()
{
  _cameraBackgroundForm->colorRadioButton->setDisabled(true);
  _cameraBackgroundForm->imageRadioButton->setDisabled(true);
  _cameraBackgroundForm->linearGradientRadioButton->setDisabled(true);
  _cameraBackgroundForm->radialGradientRadioButton->setDisabled(true);

  _cameraBackgroundForm->backgroundColorPushButton->setDisabled(true);

  _cameraBackgroundForm->selectImagePushButton->setDisabled(true);
  _cameraBackgroundForm->imagePlaceholderQLabel->setDisabled(true);

  _cameraBackgroundForm->linearGradientAngleDoubleSlider->setDisabled(true);
  _cameraBackgroundForm->linearGradientAngleDoubleSpinBox->setDisabled(true);

  _cameraBackgroundForm->linearGradientFromColorPushButton->setDisabled(true);
  _cameraBackgroundForm->linearGradientToColorPushButton->setDisabled(true);

  _cameraBackgroundForm->radialGradientRoundnessDoubleSlider->setDisabled(true);
  _cameraBackgroundForm->radialGradientRoundnessDoubleSpinBox->setDisabled(true);

  _cameraBackgroundForm->radialGradientFromColorPushButton->setDisabled(true);
  _cameraBackgroundForm->radialGradientToColorPushButton->setDisabled(true);

  if(_project)
  {
    _cameraBackgroundForm->colorRadioButton->setEnabled(true);
    _cameraBackgroundForm->imageRadioButton->setEnabled(true);
    _cameraBackgroundForm->linearGradientRadioButton->setEnabled(true);
    _cameraBackgroundForm->radialGradientRadioButton->setEnabled(true);

    _cameraBackgroundForm->backgroundColorPushButton->setEnabled(true);

    _cameraBackgroundForm->selectImagePushButton->setEnabled(true);
    _cameraBackgroundForm->imagePlaceholderQLabel->setEnabled(true);

    _cameraBackgroundForm->linearGradientAngleDoubleSlider->setEnabled(true);
    _cameraBackgroundForm->linearGradientAngleDoubleSpinBox->setEnabled(true);

    _cameraBackgroundForm->linearGradientFromColorPushButton->setEnabled(true);
    _cameraBackgroundForm->linearGradientToColorPushButton->setEnabled(true);

    _cameraBackgroundForm->radialGradientRoundnessDoubleSlider->setEnabled(true);
    _cameraBackgroundForm->radialGradientRoundnessDoubleSpinBox->setEnabled(true);

    _cameraBackgroundForm->radialGradientFromColorPushButton->setEnabled(true);
    _cameraBackgroundForm->radialGradientToColorPushButton->setEnabled(true);

    QString fileName = _project->backgroundImageFilename();


    if(fileName.isEmpty())
    {
      whileBlocking(_cameraBackgroundForm->selectImagePushButton)->setText(tr("Select Picture"));
      whileBlocking(_cameraBackgroundForm->imagePlaceholderQLabel)->setText(tr("Image Placeholder"));
    }
    else
    {
      QPixmap pixmap = QPixmap::fromImage(_project->renderBackgroundCachedImage(),Qt::AutoColor);
      QString baseFileName = QFileInfo(fileName).fileName();

      whileBlocking(_cameraBackgroundForm->selectImagePushButton)->setText(baseFileName);
      whileBlocking(_cameraBackgroundForm->imagePlaceholderQLabel)->setPixmap(pixmap);
    }

    whileBlocking(_cameraBackgroundForm->linearGradientAngleDoubleSlider)->setDoubleValue(_project->linearGradientAngle());
    whileBlocking(_cameraBackgroundForm->linearGradientAngleDoubleSpinBox)->setValue(_project->linearGradientAngle());

    whileBlocking(_cameraBackgroundForm->linearGradientFromColorPushButton)->setColor(_project->linearGradientFromColor());
    whileBlocking(_cameraBackgroundForm->linearGradientToColorPushButton)->setColor(_project->linearGradientToColor());

    whileBlocking(_cameraBackgroundForm->radialGradientRoundnessDoubleSlider)->setDoubleValue(_project->radialGradientRoundness());
    whileBlocking(_cameraBackgroundForm->radialGradientRoundnessDoubleSpinBox)->setValue(_project->radialGradientRoundness());

    whileBlocking(_cameraBackgroundForm->radialGradientFromColorPushButton)->setColor(_project->radialGradientFromColor());
    whileBlocking(_cameraBackgroundForm->radialGradientToColorPushButton)->setColor(_project->radialGradientToColor());

  }
}

void CameraTreeWidgetController::setBackgroundColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    if (_project)
    {
      _project->setBackgroundColor(color);
    }
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setLinearGradientFromColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    if (_project)
    {
      _project->setLinearGradientFromColor(color);
      whileBlocking(_cameraBackgroundForm->linearGradientFromColorPushButton)->setColor(color);
    }
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setLinearGradientToColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    if (_project)
    {
      _project->setLinearGradientToColor(color);
      whileBlocking(_cameraBackgroundForm->linearGradientToColorPushButton)->setColor(color);
    }
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setLinearGradientAngle(double angle)
{
  if (_project)
  {
    _project->setLinearGradientAngle(angle);
    whileBlocking(_cameraBackgroundForm->linearGradientAngleDoubleSlider)->setDoubleValue(angle);
    whileBlocking(_cameraBackgroundForm->linearGradientAngleDoubleSpinBox)->setValue(angle);
  }
  emit rendererReloadBackgroundImage();
  emit updateRenderer();

  _mainWindow->documentWasModified();
}

void CameraTreeWidgetController::setRadialGradientFromColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    if (_project)
    {
      _project->setRadialGradientFromColor(color);
      whileBlocking(_cameraBackgroundForm->radialGradientFromColorPushButton)->setColor(color);
    }
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setRadialGradientToColor()
{
  QColor color = QColorDialog::getColor(Qt::white,this,"Choose Color");
  if(color.isValid())
  {
    if (_project)
    {
      _project->setRadialGradientToColor(color);
      whileBlocking(_cameraBackgroundForm->radialGradientToColorPushButton)->setColor(color);
    }
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setRadialGradientRoundness(double roundness)
{
  if (_project)
  {
    _project->setRadialGradientRoundness(roundness);
    whileBlocking(_cameraBackgroundForm->radialGradientRoundnessDoubleSlider)->setDoubleValue(roundness);
    whileBlocking(_cameraBackgroundForm->radialGradientRoundnessDoubleSpinBox)->setValue(roundness);
  }
  emit rendererReloadBackgroundImage();
  emit updateRenderer();

  _mainWindow->documentWasModified();
}

void CameraTreeWidgetController::selectBackgroundImage()
{
  QUrl url = QFileDialog::getOpenFileUrl(this, QDir::homePath(),
             tr("Open Background Picture"),
             tr("png or jpg files (*.png *.jpg)"));


  if(!url.isValid())
  {
    whileBlocking(_cameraBackgroundForm->selectImagePushButton)->setText(tr("Select Picture"));
    whileBlocking(_cameraBackgroundForm->imagePlaceholderQLabel)->setText(tr("Image Placeholder"));
    return;
  }
  else
  {
    _project->loadBackgroundImage(url.path());


    QPixmap pixmap = QPixmap::fromImage(_project->renderBackgroundCachedImage(),Qt::AutoColor);
    QString baseFileName = QFileInfo(url.fileName()).fileName();

    whileBlocking(_cameraBackgroundForm->selectImagePushButton)->setText(baseFileName);
    whileBlocking(_cameraBackgroundForm->imagePlaceholderQLabel)->setPixmap(pixmap);
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setBackgroundToColor(bool checked)
{
  if(_project && checked)
  {
    _cameraBackgroundForm->stackedWidget->setCurrentIndex(0);
    _project->setBackgroundType(RKBackgroundType::color);
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setBackgroundToLinearGradient(bool checked)
{
  if(_project && checked)
  {
    _cameraBackgroundForm->stackedWidget->setCurrentIndex(1);
    _project->setBackgroundType(RKBackgroundType::linearGradient);
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setBackgroundToRadialGradient(bool checked)
{
  if(_project && checked)
  {
    _cameraBackgroundForm->stackedWidget->setCurrentIndex(2);
    _project->setBackgroundType(RKBackgroundType::radialGradient);
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}

void CameraTreeWidgetController::setBackgroundToImage(bool checked)
{
  if(_project && checked)
  {
    _cameraBackgroundForm->stackedWidget->setCurrentIndex(3);
    _project->setBackgroundType(RKBackgroundType::image);
    emit rendererReloadBackgroundImage();
    emit updateRenderer();

    _mainWindow->documentWasModified();
  }
}


