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

#include "celltreewidgetcontroller.h"
#include <cmath>
#include <iostream>
#include <type_traits>
#include <tuple>
#include <QLabel>
#include <QSpinBox>
#include <mathkit.h>
#include <renderkit.h>
#include <symmetrykit.h>
#include <foundationkit.h>
#include "textfield.h"
#include "celltreewidgetchangestructurecommand.h"
#include "celltreewidgetappliedcellcontentshiftcommand.h"

CellTreeWidgetController::CellTreeWidgetController(QWidget* parent): QTreeWidget(parent ),
    _cellCellForm(new CellCellForm),
    _cellTransformContentForm(new CellTransformContentForm),
    _cellStructuralForm(new CellStructuralForm),
    _cellSymmetryForm(new CellSymmetryForm)
{
  //this->viewport()->setMouseTracking(true);
  this->setHeaderHidden(true);
  this->setRootIsDecorated(true);
  this->setFrameStyle(QFrame::NoFrame);
  this->setAnimated(true);
  this->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  this->setExpandsOnDoubleClick(false);
  this->setIndentation(0);
  this->setSelectionMode(QAbstractItemView::NoSelection);

  // Cell
  //==============================================================================================
  QTreeWidgetItem* cellItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(cellItem);

  pushButtonCell = new QPushButton(tr("Cell"),this);
  pushButtonCell->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonCell->setStyleSheet("text-align:left;");
  setItemWidget(cellItem,0,pushButtonCell);

  QTreeWidgetItem *childCellItem = new QTreeWidgetItem(cellItem);
  this->setItemWidget(childCellItem,0, _cellCellForm);

  pushButtonCell->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  QFontMetrics fm(font());
  pushButtonCell->resize(size().width(), fm.height());

  _cellCellForm->cellStructureTypeComboBox->insertItem(0, tr("Empty"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(1, tr("Structure"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(2, tr("Crystal"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(3, tr("Molecular crystal"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(4, tr("Molecule"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(5, tr("Protein"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(6, tr("Protein crystal"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(7, tr("Protein crystal solvent"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(8, tr("Crystal solvent"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(9, tr("Molecular crystal solvent"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(10, tr("Crystal ellipsoid"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(11, tr("Crystal cylinder"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(12, tr("Crystal polygonal prism"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(13, tr("Ellipsoid"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(14, tr("Cylinder"));
  _cellCellForm->cellStructureTypeComboBox->insertItem(15, tr("Polygonal prism"));
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( _cellCellForm->cellStructureTypeComboBox->model());
  QStandardItem *itemEmpty = model->item(0);
  itemEmpty->setFlags(itemEmpty->flags() & ~Qt::ItemIsEnabled);
  QStandardItem *itemStructure = model->item(1);
  itemStructure->setFlags(itemStructure->flags() & ~Qt::ItemIsEnabled);
  QStandardItem *itemProteinCrystalSolvent = model->item(7);
  itemProteinCrystalSolvent->setFlags(itemProteinCrystalSolvent->flags() & ~Qt::ItemIsEnabled);
  QStandardItem *itemCrystalSolvent = model->item(8);
  itemCrystalSolvent->setFlags(itemCrystalSolvent->flags() & ~Qt::ItemIsEnabled);
  QStandardItem *itemMolecularCrystalSolvent = model->item(9);
  itemMolecularCrystalSolvent->setFlags(itemMolecularCrystalSolvent->flags() & ~Qt::ItemIsEnabled);
  QObject::connect(_cellCellForm->cellStructureTypeComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CellTreeWidgetController::setStructureType);

  _cellCellForm->cellBoundingBoxMaxXDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellBoundingBoxMaxYDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellBoundingBoxMaxZDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellBoundingBoxMinXDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellBoundingBoxMinYDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellBoundingBoxMinZDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);


  _cellCellForm->unitCellADoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellBDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellCDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellAlphaDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellBetaDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellGammaDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  QObject::connect(_cellCellForm->unitCellADoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CellTreeWidgetController::setUnitCellLengthA);
  QObject::connect(_cellCellForm->unitCellBDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CellTreeWidgetController::setUnitCellLengthB);
  QObject::connect(_cellCellForm->unitCellCDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CellTreeWidgetController::setUnitCellLengthC);

  QObject::connect(_cellCellForm->unitCellAlphaDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CellTreeWidgetController::setUnitCellAngleAlpha);
  QObject::connect(_cellCellForm->unitCellBetaDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CellTreeWidgetController::setUnitCellAngleBeta);
  QObject::connect(_cellCellForm->unitCellGammaDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&CellTreeWidgetController::setUnitCellAngleGamma);

  _cellCellForm->unitCellAXDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellAYDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellAZDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellBXDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellBYDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellBZDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellCXDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellCYDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->unitCellCZDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cellCellForm->volumeDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cellCellForm->perpendicularWidthXDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->perpendicularWidthYDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->perpendicularWidthZDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cellCellForm->cellMaximumReplicaX->setRange(-INT_MAX,INT_MAX);
  _cellCellForm->cellMaximumReplicaX->setSpecialValueText(tr("Mult.Val."));
  _cellCellForm->cellMaximumReplicaY->setRange(-INT_MAX,INT_MAX);
  _cellCellForm->cellMaximumReplicaY->setSpecialValueText(tr("Mult.Val."));
  _cellCellForm->cellMaximumReplicaZ->setRange(-INT_MAX,INT_MAX);
  _cellCellForm->cellMaximumReplicaZ->setSpecialValueText(tr("Mult.Val."));

  _cellCellForm->cellMinimumReplicaX->setRange(-INT_MAX,INT_MAX);
  _cellCellForm->cellMinimumReplicaX->setSpecialValueText(tr("Mult.Val."));
  _cellCellForm->cellMinimumReplicaY->setRange(-INT_MAX,INT_MAX);
  _cellCellForm->cellMinimumReplicaY->setSpecialValueText(tr("Mult.Val."));
  _cellCellForm->cellMinimumReplicaZ->setRange(-INT_MAX,INT_MAX);
  _cellCellForm->cellMinimumReplicaZ->setSpecialValueText(tr("Mult.Val."));




  _cellCellForm->cellMaximumReplicaX->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellMaximumReplicaY->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellMaximumReplicaZ->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellMinimumReplicaX->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellMinimumReplicaY->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->cellMinimumReplicaZ->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  QObject::connect(_cellCellForm->cellMaximumReplicaX, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&CellTreeWidgetController::setMaximumReplicasX);
  QObject::connect(_cellCellForm->cellMaximumReplicaY, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&CellTreeWidgetController::setMaximumReplicasY);
  QObject::connect(_cellCellForm->cellMaximumReplicaZ, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&CellTreeWidgetController::setMaximumReplicasZ);
  QObject::connect(_cellCellForm->cellMinimumReplicaX, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&CellTreeWidgetController::setMinimumReplicasX);
  QObject::connect(_cellCellForm->cellMinimumReplicaY, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&CellTreeWidgetController::setMinimumReplicasY);
  QObject::connect(_cellCellForm->cellMinimumReplicaZ, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&CellTreeWidgetController::setMinimumReplicasZ);

  _cellCellForm->rotationAngleDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->EulerAngleXDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->EulerAngleYDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->EulerAngleZDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->EulerAngleXDial->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->EulerAngleYSlider->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->EulerAngleZDial->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cellCellForm->originXDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->originYDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellCellForm->originZDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cellCellForm->EulerAngleXDoubleSpinBox->setMinimum(-180);
  _cellCellForm->EulerAngleXDoubleSpinBox->setMaximum(180);
  _cellCellForm->EulerAngleXDial->setDoubleMinimum(-180);
  _cellCellForm->EulerAngleXDial->setDoubleMaximum(180);

  _cellCellForm->EulerAngleYDoubleSpinBox->setMinimum(-90.0);
  _cellCellForm->EulerAngleYDoubleSpinBox->setMaximum(90.0);
  _cellCellForm->EulerAngleYSlider->setDoubleMinimum(-90.0);
  _cellCellForm->EulerAngleYSlider->setDoubleMaximum(90.0);

  _cellCellForm->EulerAngleZDoubleSpinBox->setMinimum(-180.0);
  _cellCellForm->EulerAngleZDoubleSpinBox->setMaximum(180.0);
  _cellCellForm->EulerAngleZDial->setDoubleMinimum(-180);
  _cellCellForm->EulerAngleZDial->setDoubleMaximum(180);

  QObject::connect(_cellCellForm->rotationAngleDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CellTreeWidgetController::setRotationAngle);
  QObject::connect(_cellCellForm->rotatePlusXPushButton, &QPushButton::clicked, this, &CellTreeWidgetController::rotateYawPlus);
  QObject::connect(_cellCellForm->rotatePlusZPushButton, &QPushButton::clicked, this, &CellTreeWidgetController::rotatePitchPlus);
  QObject::connect(_cellCellForm->rotatePlusYPushButton, &QPushButton::clicked, this, &CellTreeWidgetController::rotateRollPlus);
  QObject::connect(_cellCellForm->rotateMinusXPushButton, &QPushButton::clicked, this, &CellTreeWidgetController::rotateYawMinus);
  QObject::connect(_cellCellForm->rotateMinusZPushButton, &QPushButton::clicked, this, &CellTreeWidgetController::rotatePitchMinus);
  QObject::connect(_cellCellForm->rotateMinusYPushButton, &QPushButton::clicked, this, &CellTreeWidgetController::rotateRollMinus);


  QObject::connect(_cellCellForm->EulerAngleXDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CellTreeWidgetController::setEulerAngleX);
  QObject::connect(_cellCellForm->EulerAngleYDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CellTreeWidgetController::setEulerAngleY);
  QObject::connect(_cellCellForm->EulerAngleZDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CellTreeWidgetController::setEulerAngleZ);
  QObject::connect(_cellCellForm->EulerAngleXDial,static_cast<void (QDoubleDial::*)(double)>(&QDoubleDial::sliderMoved),this,&CellTreeWidgetController::setEulerAngleX);
  QObject::connect(_cellCellForm->EulerAngleYSlider,static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved),this,&CellTreeWidgetController::setEulerAngleY);
  QObject::connect(_cellCellForm->EulerAngleZDial,static_cast<void (QDoubleDial::*)(double)>(&QDoubleDial::sliderMoved),this,&CellTreeWidgetController::setEulerAngleZ);

  QObject::connect(_cellCellForm->originXDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CellTreeWidgetController::setOriginX);
  QObject::connect(_cellCellForm->originYDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CellTreeWidgetController::setOriginY);
  QObject::connect(_cellCellForm->originZDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CellTreeWidgetController::setOriginZ);

  // Tranform content
  //==============================================================================================
  QTreeWidgetItem* transformContentItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(transformContentItem);

  pushButtonTransformContent = new QPushButton(tr("Tranform content"),this);
  pushButtonTransformContent->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonTransformContent->setStyleSheet("text-align:left;");
  setItemWidget(transformContentItem,0,pushButtonTransformContent);

  QTreeWidgetItem *childTransformContentItem = new QTreeWidgetItem(transformContentItem);
  this->setItemWidget(childTransformContentItem,0, _cellTransformContentForm);


  QObject::connect(_cellTransformContentForm->flipAxisACheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
                   this,&CellTreeWidgetController::setFlipAxisA);
  QObject::connect(_cellTransformContentForm->flipAxisBCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
                   this,&CellTreeWidgetController::setFlipAxisB);
  QObject::connect(_cellTransformContentForm->flipAxisCCheckBox,static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
                   this,&CellTreeWidgetController::setFlipAxisC);

  QObject::connect(_cellTransformContentForm->shiftAxisADoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                   this, &CellTreeWidgetController::setShiftAxisA);
  QObject::connect(_cellTransformContentForm->shiftAxisBDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                   this, &CellTreeWidgetController::setShiftAxisB);
  QObject::connect(_cellTransformContentForm->shiftAxisCDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                   this, &CellTreeWidgetController::setShiftAxisC);

  _cellTransformContentForm->shiftAxisADoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellTransformContentForm->shiftAxisBDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellTransformContentForm->shiftAxisCDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

  _cellTransformContentForm->shiftAxisADoubleSpinBox->setKeyboardTracking(false);
  _cellTransformContentForm->shiftAxisBDoubleSpinBox->setKeyboardTracking(false);
  _cellTransformContentForm->shiftAxisCDoubleSpinBox->setKeyboardTracking(false);

  _cellTransformContentForm->shiftAxisADoubleSpinBox->setMinimum(-DBL_MAX);
  _cellTransformContentForm->shiftAxisADoubleSpinBox->setMaximum(DBL_MAX);
  _cellTransformContentForm->shiftAxisADoubleSpinBox->setSingleStep(0.05);

  _cellTransformContentForm->shiftAxisBDoubleSpinBox->setMinimum(-DBL_MAX);
  _cellTransformContentForm->shiftAxisBDoubleSpinBox->setMaximum(DBL_MAX);
  _cellTransformContentForm->shiftAxisBDoubleSpinBox->setSingleStep(0.05);

  _cellTransformContentForm->shiftAxisCDoubleSpinBox->setMinimum(-DBL_MAX);
  _cellTransformContentForm->shiftAxisCDoubleSpinBox->setMaximum(DBL_MAX);
  _cellTransformContentForm->shiftAxisCDoubleSpinBox->setSingleStep(0.05);

  QObject::connect(_cellTransformContentForm->applyPushButton, &QPushButton::clicked, this, &CellTreeWidgetController::applyTransformContent);

  // Structural
  //==============================================================================================
  QTreeWidgetItem* structuralItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(structuralItem);

  pushButtonStructural = new QPushButton(tr("Structural properties"),this);
  pushButtonStructural->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonStructural->setStyleSheet("text-align:left;");
  setItemWidget(structuralItem,0,pushButtonStructural);

  QTreeWidgetItem *childStructuralItem = new QTreeWidgetItem(structuralItem);
  this->setItemWidget(childStructuralItem,0, _cellStructuralForm);

  _cellStructuralForm->massDoubleSpinBox->setReadOnly(true);

  pushButtonCell->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonCell->resize(size().width(), fm.height());

  QObject::connect(_cellStructuralForm->computeHeliumVoidFractionPushButton, &QPushButton::clicked, this, &CellTreeWidgetController::computeHeliumVoidFractionPushButton);

  _cellStructuralForm->probeMoleculeComboBox->insertItem(0, tr("Helium"));
  _cellStructuralForm->probeMoleculeComboBox->insertItem(1, tr("Methane"));
  _cellStructuralForm->probeMoleculeComboBox->insertItem(2, tr("Nitrogen"));
  _cellStructuralForm->probeMoleculeComboBox->insertItem(3, tr("Hydrogen"));
  _cellStructuralForm->probeMoleculeComboBox->insertItem(4, tr("Water"));
  _cellStructuralForm->probeMoleculeComboBox->insertItem(5, tr("CO₂"));
  _cellStructuralForm->probeMoleculeComboBox->insertItem(6, tr("Xenon"));
  _cellStructuralForm->probeMoleculeComboBox->insertItem(7, tr("Krypton"));
  _cellStructuralForm->probeMoleculeComboBox->insertItem(8, tr("Argon"));
  QObject::connect(_cellStructuralForm->probeMoleculeComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CellTreeWidgetController::setFrameworkProbeMolecule);

  _cellStructuralForm->computeGravimetricSurfaceAreaPushButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  _cellStructuralForm->computeVolumetricSurfaceAreaPushButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);

  _cellStructuralForm->gravimetricSurfaceAreaDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellStructuralForm->gravimetricSurfaceAreaDoubleSpinBox->setKeyboardTracking(false);

  _cellStructuralForm->volumetricSurfaceAreaDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellStructuralForm->volumetricSurfaceAreaDoubleSpinBox->setKeyboardTracking(false);

  _cellStructuralForm->numberOfChannelSystemsSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellStructuralForm->numberOfChannelSystemsSpinBox->setKeyboardTracking(false);

  _cellStructuralForm->numberOfInaccessiblePocketsSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellStructuralForm->numberOfInaccessiblePocketsSpinBox->setKeyboardTracking(false);

  _cellStructuralForm->dimensionalityOfPoreSystemSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellStructuralForm->dimensionalityOfPoreSystemSpinBox->setKeyboardTracking(false);

  _cellStructuralForm->largestOverallCavityDiameterDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellStructuralForm->largestOverallCavityDiameterDoubleSpinBox->setKeyboardTracking(false);

  _cellStructuralForm->restrictingPoreDiameterDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellStructuralForm->restrictingPoreDiameterDoubleSpinBox->setKeyboardTracking(false);

  _cellStructuralForm->largestDiameterAlongAViablePathDoubleSpinBox->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
  _cellStructuralForm->largestDiameterAlongAViablePathDoubleSpinBox->setKeyboardTracking(false);

  QObject::connect(_cellStructuralForm->computeGravimetricSurfaceAreaPushButton, &QPushButton::clicked, this, &CellTreeWidgetController::computeGravimetricSurfaceAreaPushButton);
  QObject::connect(_cellStructuralForm->computeVolumetricSurfaceAreaPushButton, &QPushButton::clicked,this, &CellTreeWidgetController::computeGravimetricSurfaceAreaPushButton);

  QObject::connect(_cellStructuralForm->numberOfChannelSystemsSpinBox, static_cast<void (CustomIntSpinBox::*)(int)>(&CustomIntSpinBox::valueChanged),this, &CellTreeWidgetController::setStructureNumberOfChannelSystems);
  QObject::connect(_cellStructuralForm->numberOfInaccessiblePocketsSpinBox, static_cast<void (CustomIntSpinBox::*)(int)>(&CustomIntSpinBox::valueChanged),this, &CellTreeWidgetController::setStructureNumberOfInaccessiblePockets);
  QObject::connect(_cellStructuralForm->dimensionalityOfPoreSystemSpinBox, static_cast<void (CustomIntSpinBox::*)(int)>(&CustomIntSpinBox::valueChanged),this, &CellTreeWidgetController::setStructureDimensionalityOfPoreSystem);

  QObject::connect(_cellStructuralForm->largestOverallCavityDiameterDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this, &CellTreeWidgetController::setStructureLargestCavityDiameter);
  QObject::connect(_cellStructuralForm->restrictingPoreDiameterDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this, &CellTreeWidgetController::setStructureRestrictingPoreLimitingDiameter);
  QObject::connect(_cellStructuralForm->largestDiameterAlongAViablePathDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this, &CellTreeWidgetController::setStructureLargestCavityDiameterAlongAviablePath);


  _cellStructuralForm->materialTypeComboBox->insertItem(0,tr("Unspecified"));
  _cellStructuralForm->materialTypeComboBox->insertItem(1,tr("Silica"));
  _cellStructuralForm->materialTypeComboBox->insertItem(2,tr("Aluminosilicate"));
  _cellStructuralForm->materialTypeComboBox->insertItem(3,tr("Metallophosphate"));
  _cellStructuralForm->materialTypeComboBox->insertItem(4,tr("Silicialinophosphate"));
  _cellStructuralForm->materialTypeComboBox->insertItem(5,tr("Zeolite"));
  _cellStructuralForm->materialTypeComboBox->insertItem(6,tr("MOF"));
  _cellStructuralForm->materialTypeComboBox->insertItem(7,tr("ZIF"));


  // Symmetry
  //==============================================================================================
  QTreeWidgetItem* symmetryItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(symmetryItem);

  pushButtonSymmetry = new QPushButton(tr("Symmetry properties"),this);
  pushButtonSymmetry->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonSymmetry->setStyleSheet("text-align:left;");
  setItemWidget(symmetryItem,0,pushButtonSymmetry);

  QTreeWidgetItem *childSymmetryItem = new QTreeWidgetItem(symmetryItem);
  this->setItemWidget(childSymmetryItem,0, _cellSymmetryForm);

  pushButtonSymmetry->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonSymmetry->resize(size().width(), fm.height());

  _cellSymmetryForm->spaceGroupHallNamecomboBox->clear();
  int index=0;
  for(const SKSpaceGroupSetting& spaceGroup : SKSpaceGroup::spaceGroupData())
  {
    _cellSymmetryForm->spaceGroupHallNamecomboBox->addItem(QString::number(index) + " " + spaceGroup.HallString());
    index++;
  }

  QObject::connect(_cellSymmetryForm->spaceGroupHallNamecomboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CellTreeWidgetController::setSymmetrySpaceGroupHallNumber);
  QObject::connect(_cellSymmetryForm->spaceGroupITNumberComboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&CellTreeWidgetController::setSymmetrySpaceGroupStandardNumber);

  _cellSymmetryForm->spaceGroupITNumberComboBox->clear();
  for(int i=0; i<=230; i++)
  {
    whileBlocking(_cellSymmetryForm->spaceGroupITNumberComboBox)->insertItem(i,QString::number(i));
  }

  QObject::connect(_cellSymmetryForm->precisionDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CellTreeWidgetController::setSymmetryPrecision);


  // Expanding
  //=========================================================================
  pushButtonCell->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonTransformContent->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonStructural->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonSymmetry->setFocusPolicy(Qt::FocusPolicy::NoFocus);

  QObject::connect(pushButtonCell, &QPushButton::clicked, this, &CellTreeWidgetController::expandCellItem);
  QObject::connect(pushButtonTransformContent, &QPushButton::clicked, this, &CellTreeWidgetController::expandTransformContentItem);
  QObject::connect(pushButtonStructural, &QPushButton::clicked, this, &CellTreeWidgetController::expandStructuralItem);
  QObject::connect(pushButtonSymmetry, &QPushButton::clicked, this, &CellTreeWidgetController::expandSymmetryItem);
}


void CellTreeWidgetController::expandCellItem()
{
  QModelIndex index = indexFromItem(topLevelItem(0),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonCell->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonCell->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CellTreeWidgetController::expandTransformContentItem()
{
  QModelIndex index = indexFromItem(topLevelItem(1),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonTransformContent->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonTransformContent->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CellTreeWidgetController::expandStructuralItem()
{
  QModelIndex index = indexFromItem(topLevelItem(2),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonStructural->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonStructural->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CellTreeWidgetController::expandSymmetryItem()
{
  QModelIndex index = indexFromItem(topLevelItem(3),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonSymmetry->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonSymmetry->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void CellTreeWidgetController::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
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


void CellTreeWidgetController::setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAStructure>> iraspa_structures)
{
  _iraspa_structures = iraspa_structures;
  reloadData();
}

void CellTreeWidgetController::resetData()
{
  reloadData();
}

void CellTreeWidgetController::reloadData()
{
  reloadCellProperties();
  reloadTransformContentProperties();
  reloadStructureProperties();
  reloadSymmetryProperties();
}

void CellTreeWidgetController::reloadSelection()
{

}


// Cell/Bounding properties
//========================================================================================================================================

void CellTreeWidgetController::reloadCellProperties()
{
	reloadStructureType();
  reloadBoundingBox();

  reloadUnitCell();
  reloadUnitCellLengths();
  reloadUnitCellAngles();

  reloadCellVolume();
  reloadCellPerpendicularWidthX();
  reloadCellPerpendicularWidthY();
  reloadCellPerpendicularWidthZ();

  reloadCellMaximumReplicasX();
  reloadCellMinimumReplicasX();
  reloadCellMaximumReplicasY();
  reloadCellMinimumReplicasY();
  reloadCellMaximumReplicasZ();
  reloadCellMinimumReplicasZ();

  reloadRotationAngle();
  reloadEulerAngles();

  reloadCellOriginX();
  reloadCellOriginY();
  reloadCellOriginZ();
}


void CellTreeWidgetController::reloadStructureType()
{
  _cellCellForm->cellStructureTypeComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->cellStructureTypeComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<iRASPAStructureType> type = structureType())
      {
        if (int index = _cellCellForm->cellStructureTypeComboBox->findText("Multiple values"); index >= 0)
        {
          whileBlocking(_cellCellForm->cellStructureTypeComboBox)->removeItem(index);
        }
        whileBlocking(_cellCellForm->cellStructureTypeComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if (int index = _cellCellForm->cellStructureTypeComboBox->findText("Multiple values"); index < 0)
        {
          whileBlocking(_cellCellForm->cellStructureTypeComboBox)->addItem("Multiple values");
        }
        whileBlocking(_cellCellForm->cellStructureTypeComboBox)->setCurrentText("Multiple values");
      }
		}
	}
}

void CellTreeWidgetController::reloadBoundingBox()
{
  _cellCellForm->cellBoundingBoxMaxXDoubleSpinBox->setDisabled(true);
  _cellCellForm->cellBoundingBoxMaxYDoubleSpinBox->setDisabled(true);
  _cellCellForm->cellBoundingBoxMaxZDoubleSpinBox->setDisabled(true);
  _cellCellForm->cellBoundingBoxMinXDoubleSpinBox->setDisabled(true);
  _cellCellForm->cellBoundingBoxMinYDoubleSpinBox->setDisabled(true);
  _cellCellForm->cellBoundingBoxMinZDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->cellBoundingBoxMaxXDoubleSpinBox->setEnabled(true);
      _cellCellForm->cellBoundingBoxMaxYDoubleSpinBox->setEnabled(true);
      _cellCellForm->cellBoundingBoxMaxZDoubleSpinBox->setEnabled(true);
      _cellCellForm->cellBoundingBoxMinXDoubleSpinBox->setEnabled(true);
      _cellCellForm->cellBoundingBoxMinYDoubleSpinBox->setEnabled(true);
      _cellCellForm->cellBoundingBoxMinZDoubleSpinBox->setEnabled(true);

      _cellCellForm->cellBoundingBoxMaxXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->cellBoundingBoxMaxYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->cellBoundingBoxMaxZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->cellBoundingBoxMinXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->cellBoundingBoxMinYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->cellBoundingBoxMinZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      SKBoundingBox overAllBoundingBox = boundingBox();

      whileBlocking(_cellCellForm->cellBoundingBoxMaxXDoubleSpinBox)->setValue(overAllBoundingBox.maximum().x);
      whileBlocking(_cellCellForm->cellBoundingBoxMaxYDoubleSpinBox)->setValue(overAllBoundingBox.maximum().y);
      whileBlocking(_cellCellForm->cellBoundingBoxMaxZDoubleSpinBox)->setValue(overAllBoundingBox.maximum().z);

      whileBlocking(_cellCellForm->cellBoundingBoxMinXDoubleSpinBox)->setValue(overAllBoundingBox.minimum().x);
      whileBlocking(_cellCellForm->cellBoundingBoxMinYDoubleSpinBox)->setValue(overAllBoundingBox.minimum().y);
      whileBlocking(_cellCellForm->cellBoundingBoxMinZDoubleSpinBox)->setValue(overAllBoundingBox.minimum().z);
    }
  }
}

void CellTreeWidgetController::reloadUnitCellLengths()
{
  _cellCellForm->unitCellADoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellBDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellCDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->unitCellADoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellBDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellCDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellADoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellBDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellCDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = unitCellLengthA())
      {
        whileBlocking(_cellCellForm->unitCellADoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellADoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellLengthB())
      {
        whileBlocking(_cellCellForm->unitCellBDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellBDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellLengthC())
      {
        whileBlocking(_cellCellForm->unitCellCDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellCDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadUnitCellAngles()
{
  _cellCellForm->unitCellAlphaDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellBetaDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellGammaDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->unitCellAlphaDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellBetaDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellGammaDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellAlphaDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellBetaDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellGammaDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = unitCellAngleAlpha())
      {
        whileBlocking(_cellCellForm->unitCellAlphaDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellAlphaDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellAngleBeta())
      {
        whileBlocking(_cellCellForm->unitCellBetaDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellBetaDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellAngleGamma())
      {
        whileBlocking(_cellCellForm->unitCellGammaDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellGammaDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadUnitCell()
{
  _cellCellForm->unitCellAXDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellAYDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellAZDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellBXDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellBYDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellBZDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellCXDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellCYDoubleSpinBox->setDisabled(true);
  _cellCellForm->unitCellCZDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->unitCellAXDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellAYDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellAZDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellBXDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellBYDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellBZDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellCXDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellCYDoubleSpinBox->setEnabled(true);
      _cellCellForm->unitCellCZDoubleSpinBox->setEnabled(true);

      _cellCellForm->unitCellAXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellAYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellAZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellBXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellBYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellBZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellCXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellCYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->unitCellCZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = unitCellAX())
      {
        whileBlocking(_cellCellForm->unitCellAXDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellAXDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellAY())
      {
        whileBlocking(_cellCellForm->unitCellAYDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellAYDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellAZ())
      {
        whileBlocking(_cellCellForm->unitCellAZDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellAZDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellBX())
      {
        whileBlocking(_cellCellForm->unitCellBXDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellBXDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellBY())
      {
        whileBlocking(_cellCellForm->unitCellBYDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellBYDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellBZ())
      {
        whileBlocking(_cellCellForm->unitCellBZDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellBZDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellCX())
      {
        whileBlocking(_cellCellForm->unitCellCXDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellCXDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellCY())
      {
        whileBlocking(_cellCellForm->unitCellCYDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellCYDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = unitCellCZ())
      {
        whileBlocking(_cellCellForm->unitCellCZDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->unitCellCZDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellVolume()
{
  _cellCellForm->volumeDoubleSpinBox->setEnabled(true);
  _cellCellForm->volumeDoubleSpinBox->setReadOnly(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      if (std::optional<double> value = unitCellVolume())
      {
        whileBlocking(_cellCellForm->volumeDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->volumeDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellPerpendicularWidthX()
{
  _cellCellForm->perpendicularWidthXDoubleSpinBox->setEnabled(true);
  _cellCellForm->perpendicularWidthXDoubleSpinBox->setReadOnly(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      if (std::optional<double> value = unitCellPerpendicularWidthX())
      {
        whileBlocking(_cellCellForm->perpendicularWidthXDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->perpendicularWidthXDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellPerpendicularWidthY()
{
  _cellCellForm->perpendicularWidthYDoubleSpinBox->setEnabled(true);
  _cellCellForm->perpendicularWidthYDoubleSpinBox->setReadOnly(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      if (std::optional<double> value = unitCellPerpendicularWidthY())
      {
        whileBlocking(_cellCellForm->perpendicularWidthYDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->perpendicularWidthYDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellPerpendicularWidthZ()
{
  _cellCellForm->perpendicularWidthZDoubleSpinBox->setEnabled(true);
  _cellCellForm->perpendicularWidthZDoubleSpinBox->setReadOnly(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      if (std::optional<double> value = unitCellPerpendicularWidthZ())
      {
        whileBlocking(_cellCellForm->perpendicularWidthZDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->perpendicularWidthZDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellMaximumReplicasX()
{
  _cellCellForm->cellMaximumReplicaX->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->cellMaximumReplicaX->setEnabled(true);
      _cellCellForm->cellMaximumReplicaX->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = maximumReplicasX())
      {
        whileBlocking(_cellCellForm->cellMaximumReplicaX)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->cellMaximumReplicaX)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellMinimumReplicasX()
{
  _cellCellForm->cellMinimumReplicaX->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->cellMinimumReplicaX->setEnabled(true);
      _cellCellForm->cellMinimumReplicaX->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = minimumReplicasX())
      {
        whileBlocking(_cellCellForm->cellMinimumReplicaX)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->cellMinimumReplicaX)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellMaximumReplicasY()
{
  _cellCellForm->cellMaximumReplicaY->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->cellMaximumReplicaY->setEnabled(true);
      _cellCellForm->cellMaximumReplicaY->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = maximumReplicasY())
      {
        whileBlocking(_cellCellForm->cellMaximumReplicaY)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->cellMaximumReplicaY)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellMinimumReplicasY()
{
  _cellCellForm->cellMinimumReplicaY->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->cellMinimumReplicaY->setEnabled(true);
      _cellCellForm->cellMinimumReplicaY->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = minimumReplicasY())
      {
        whileBlocking(_cellCellForm->cellMinimumReplicaY)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->cellMinimumReplicaY)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellMaximumReplicasZ()
{
  _cellCellForm->cellMaximumReplicaZ->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->cellMaximumReplicaZ->setEnabled(true);
      _cellCellForm->cellMaximumReplicaZ->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = maximumReplicasZ())
      {
        whileBlocking(_cellCellForm->cellMaximumReplicaZ)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->cellMaximumReplicaZ)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellMinimumReplicasZ()
{
  _cellCellForm->cellMinimumReplicaZ->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->cellMinimumReplicaZ->setEnabled(true);
      _cellCellForm->cellMinimumReplicaZ->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = minimumReplicasZ())
      {
        whileBlocking(_cellCellForm->cellMinimumReplicaZ)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->cellMinimumReplicaZ)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadRotationAngle()
{
  _cellCellForm->rotationAngleDoubleSpinBox->setDisabled(true);
  _cellCellForm->rotatePlusXPushButton->setDisabled(true);
  _cellCellForm->rotatePlusYPushButton->setDisabled(true);
  _cellCellForm->rotatePlusZPushButton->setDisabled(true);
  _cellCellForm->rotateMinusXPushButton->setDisabled(true);
  _cellCellForm->rotateMinusYPushButton->setDisabled(true);
  _cellCellForm->rotateMinusZPushButton->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->rotationAngleDoubleSpinBox->setEnabled(true);
      _cellCellForm->rotationAngleDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> angle = rotationAngle())
      {
        whileBlocking(_cellCellForm->rotationAngleDoubleSpinBox)->setValue(*angle);

        _cellCellForm->rotatePlusXPushButton->setEnabled(_projectTreeNode->isEditable());
        _cellCellForm->rotatePlusYPushButton->setEnabled(_projectTreeNode->isEditable());
        _cellCellForm->rotatePlusZPushButton->setEnabled(_projectTreeNode->isEditable());
        _cellCellForm->rotateMinusXPushButton->setEnabled(_projectTreeNode->isEditable());
        _cellCellForm->rotateMinusYPushButton->setEnabled(_projectTreeNode->isEditable());
        _cellCellForm->rotateMinusZPushButton->setEnabled(_projectTreeNode->isEditable());

        _cellCellForm->rotatePlusXPushButton->setText("Rotate +" + QString::number(*angle));
        _cellCellForm->rotatePlusYPushButton->setText("Rotate +" + QString::number(*angle));
        _cellCellForm->rotatePlusZPushButton->setText("Rotate +" + QString::number(*angle));
        _cellCellForm->rotateMinusXPushButton->setText("Rotate -" + QString::number(*angle));
        _cellCellForm->rotateMinusYPushButton->setText("Rotate -" + QString::number(*angle));
        _cellCellForm->rotateMinusZPushButton->setText("Rotate -" + QString::number(*angle));
      }
      else
      {
        whileBlocking(_cellCellForm->rotationAngleDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

// QT-bug: disabling and re-enabling does not work for QDial and 'sliderMoved'
void CellTreeWidgetController::reloadEulerAngles()
{
  _cellCellForm->EulerAngleXDoubleSpinBox->setDisabled(true);
  _cellCellForm->EulerAngleXDial->setDisabled(true);
  _cellCellForm->EulerAngleYDoubleSpinBox->setDisabled(true);
  _cellCellForm->EulerAngleYSlider->setDisabled(true);
  _cellCellForm->EulerAngleZDoubleSpinBox->setDisabled(true);
  _cellCellForm->EulerAngleZDial->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->EulerAngleXDoubleSpinBox->setEnabled(true);
      _cellCellForm->EulerAngleXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->EulerAngleXDial->setEnabled(_projectTreeNode->isEditable());

      _cellCellForm->EulerAngleYDoubleSpinBox->setEnabled(true);
      _cellCellForm->EulerAngleYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->EulerAngleYSlider->setEnabled(_projectTreeNode->isEditable());

      _cellCellForm->EulerAngleZDoubleSpinBox->setEnabled(true);
      _cellCellForm->EulerAngleZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellCellForm->EulerAngleZDial->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<double> angle = EulerAngleX())
      {
        whileBlocking(_cellCellForm->EulerAngleXDial)->setDoubleValue(*angle);
        whileBlocking(_cellCellForm->EulerAngleXDoubleSpinBox)->setValue(*angle);
      }
      else
      {
        whileBlocking(_cellCellForm->EulerAngleXDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> angle = EulerAngleY())
      {
        whileBlocking(_cellCellForm->EulerAngleYDoubleSpinBox)->setValue(*angle);
        whileBlocking(_cellCellForm->EulerAngleYSlider)->setDoubleValue(*angle);
      }
      else
      {
        whileBlocking(_cellCellForm->EulerAngleYDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> angle = EulerAngleZ())
      {
        whileBlocking(_cellCellForm->EulerAngleZDoubleSpinBox)->setValue(*angle);
        whileBlocking(_cellCellForm->EulerAngleZDial)->setDoubleValue(*angle);
      }
      else
      {
        whileBlocking(_cellCellForm->EulerAngleZDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}


void CellTreeWidgetController::reloadCellOriginX()
{
  _cellCellForm->originXDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->originXDoubleSpinBox->setEnabled(true);
      _cellCellForm->originXDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = originX())
      {
        whileBlocking(_cellCellForm->originXDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->originXDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellOriginY()
{
  _cellCellForm->originYDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->originYDoubleSpinBox->setEnabled(true);
      _cellCellForm->originYDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = originY())
      {
        whileBlocking(_cellCellForm->originYDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->originYDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadCellOriginZ()
{
  _cellCellForm->originZDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellCellForm->originZDoubleSpinBox->setEnabled(true);
      _cellCellForm->originZDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = originZ())
      {
        whileBlocking(_cellCellForm->originZDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellCellForm->originZDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::setStructureType(int value)
{
  CellTreeWidgetChangeStructureCommand *changeCommand = new CellTreeWidgetChangeStructureCommand(_mainWindow, _projectTreeNode, _iraspa_structures, value, nullptr);

  _projectTreeNode->representedObject()->undoManager().push(changeCommand);
}

std::optional<iRASPAStructureType> CellTreeWidgetController::structureType()
{
  if (_iraspa_structures.empty())
	{
    return std::nullopt;
	}
  std::unordered_set<iRASPAStructureType, enum_hash> set = std::unordered_set<iRASPAStructureType, enum_hash>{};
  for (std::shared_ptr<iRASPAStructure> iraspa_structure : _iraspa_structures)
	{
    iRASPAStructureType value = iraspa_structure->structure()->structureType();
    set.emplace(value);
	}

	if (set.size() == 1)
	{
    iRASPAStructureType value = *set.begin();
    return value;
	}
  return std::nullopt;
}

SKBoundingBox CellTreeWidgetController::boundingBox()
{
  if(!_iraspa_structures.empty())
  {
    double3 minimum = double3(DBL_MAX,DBL_MAX, DBL_MAX);
    double3 maximum = double3(-DBL_MAX,-DBL_MAX, -DBL_MAX);
    for(std::shared_ptr<iRASPAStructure> iraspa_structure : _iraspa_structures)
    {
      std::shared_ptr<Structure> structure = iraspa_structure->structure();
      SKBoundingBox transformedBoundingBox = structure->transformedBoundingBox();

      minimum.x = std::min(minimum.x, transformedBoundingBox.minimum().x + structure->origin().x);
      minimum.y = std::min(minimum.y, transformedBoundingBox.minimum().y + structure->origin().y);
      minimum.z = std::min(minimum.z, transformedBoundingBox.minimum().z + structure->origin().z);
      maximum.x = std::max(maximum.x, transformedBoundingBox.maximum().x + structure->origin().x);
      maximum.y = std::max(maximum.y, transformedBoundingBox.maximum().y + structure->origin().y);
      maximum.z = std::max(maximum.z, transformedBoundingBox.maximum().z + structure->origin().z);
    }
    return SKBoundingBox(minimum,maximum);
  }
  return SKBoundingBox(double3(0.0,0.0,0.0),double3(0.0,0.0,0.0));
}

std::optional<double> CellTreeWidgetController::unitCellAX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->unitCell().ax);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}


std::optional<double> CellTreeWidgetController::unitCellAY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->unitCell().ay);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::unitCellAZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->unitCell().az);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}


std::optional<double> CellTreeWidgetController::unitCellBX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->unitCell().bx);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::unitCellBY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->unitCell().by);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::unitCellBZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->unitCell().bz);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::unitCellCX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->unitCell().cx);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::unitCellCY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->unitCell().cy);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::unitCellCZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->unitCell().cz);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::unitCellLengthA()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->a());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setUnitCellLengthA(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->cell()->setLengthA(value);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();
    reloadUnitCell();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::unitCellLengthB()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->b());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setUnitCellLengthB(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->cell()->setLengthB(value);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();
    reloadUnitCell();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::unitCellLengthC()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->c());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setUnitCellLengthC(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->cell()->setLengthC(value);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();
    reloadUnitCell();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::unitCellAngleAlpha()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->alpha() * 180.0/M_PI);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setUnitCellAngleAlpha(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->cell()->setAlphaAngle(value * M_PI / 180.0);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();
    reloadUnitCell();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::unitCellAngleBeta()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->beta() * 180.0/M_PI);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setUnitCellAngleBeta(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->cell()->setBetaAngle(value * M_PI / 180.0);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();
    reloadUnitCell();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::unitCellAngleGamma()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->gamma() * 180.0/M_PI);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setUnitCellAngleGamma(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->cell()->setGammaAngle(value * M_PI / 180.0);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();
    reloadUnitCell();

    _mainWindow->documentWasModified();
  }
}


std::optional<double> CellTreeWidgetController::unitCellVolume()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->volume());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}


std::optional<double> CellTreeWidgetController::unitCellPerpendicularWidthX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->perpendicularWidths().x);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::unitCellPerpendicularWidthY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->perpendicularWidths().y);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}


std::optional<double> CellTreeWidgetController::unitCellPerpendicularWidthZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->perpendicularWidths().z);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}


std::optional<int> CellTreeWidgetController::maximumReplicasX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->maximumReplicaX());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

void CellTreeWidgetController::setMaximumReplicasX(int value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      if (value >= iraspa_structure->structure()->cell()->minimumReplicaX())
      {
        iraspa_structure->structure()->cell()->setMaximumReplicaX(value);
        iraspa_structure->structure()->reComputeBoundingBox();
      }
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadCellMaximumReplicasX();
    reloadBoundingBox();

    _mainWindow->documentWasModified();
  }
}

std::optional<int> CellTreeWidgetController::maximumReplicasY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->maximumReplicaY());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

void CellTreeWidgetController::setMaximumReplicasY(int value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      if (value >= iraspa_structure->structure()->cell()->minimumReplicaY())
      {
        iraspa_structure->structure()->cell()->setMaximumReplicaY(value);
        iraspa_structure->structure()->reComputeBoundingBox();
      }
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadCellMaximumReplicasY();
    reloadBoundingBox();

    _mainWindow->documentWasModified();
  }
}

std::optional<int> CellTreeWidgetController::maximumReplicasZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->maximumReplicaZ());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

void CellTreeWidgetController::setMaximumReplicasZ(int value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      if (value >= iraspa_structure->structure()->cell()->minimumReplicaZ())
      {
        iraspa_structure->structure()->cell()->setMaximumReplicaZ(value);
        iraspa_structure->structure()->reComputeBoundingBox();
      }
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadCellMaximumReplicasZ();
    reloadBoundingBox();

    _mainWindow->documentWasModified();
  }
}

std::optional<int> CellTreeWidgetController::minimumReplicasX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->minimumReplicaX());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

void CellTreeWidgetController::setMinimumReplicasX(int value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      if (value <= iraspa_structure->structure()->cell()->maximumReplicaX())
      {
        iraspa_structure->structure()->cell()->setMinimumReplicaX(value);
        iraspa_structure->structure()->reComputeBoundingBox();
      }
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadCellMinimumReplicasX();
    reloadBoundingBox();

    _mainWindow->documentWasModified();
  }
}

std::optional<int> CellTreeWidgetController::minimumReplicasY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->minimumReplicaY());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

void CellTreeWidgetController::setMinimumReplicasY(int value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      if (value <= iraspa_structure->structure()->cell()->maximumReplicaY())
      {
        iraspa_structure->structure()->cell()->setMinimumReplicaY(value);
        iraspa_structure->structure()->reComputeBoundingBox();
      }
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadCellMinimumReplicasY();
    reloadBoundingBox();

    _mainWindow->documentWasModified();
  }
}

std::optional<int> CellTreeWidgetController::minimumReplicasZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->cell()->minimumReplicaZ());
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

void CellTreeWidgetController::setMinimumReplicasZ(int value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      if (value <= iraspa_structure->structure()->cell()->maximumReplicaZ())
      {
        iraspa_structure->structure()->cell()->setMinimumReplicaZ(value);
        iraspa_structure->structure()->reComputeBoundingBox();
      }
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadCellMinimumReplicasZ();
    reloadBoundingBox();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::rotationAngle()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }

  std::set<double> set = std::set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double angle = iraspa_structure->structure()->rotationDelta();
    set.insert(angle);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setRotationAngle(double angle)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setRotationDelta(angle);
    }
    reloadRotationAngle();

    _mainWindow->documentWasModified();
  }
}

void CellTreeWidgetController::rotateYawPlus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->rotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->orientation();
      simd_quatd dq = simd_quatd::yaw(rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadBoundingBox();
    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void CellTreeWidgetController::rotateYawMinus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->rotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->orientation();
      simd_quatd dq = simd_quatd::yaw(-rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadBoundingBox();
    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void CellTreeWidgetController::rotatePitchPlus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->rotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->orientation();
      simd_quatd dq = simd_quatd::pitch(rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadBoundingBox();
    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void CellTreeWidgetController::rotatePitchMinus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->rotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->orientation();
      simd_quatd dq = simd_quatd::pitch(-rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadBoundingBox();
    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void CellTreeWidgetController::rotateRollPlus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->rotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->orientation();
      simd_quatd dq = simd_quatd::roll(rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadBoundingBox();
    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

void CellTreeWidgetController::rotateRollMinus()
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      double rotationDelta = iraspa_structure->structure()->rotationDelta();
      simd_quatd orientation = iraspa_structure->structure()->orientation();
      simd_quatd dq = simd_quatd::roll(-rotationDelta);
      simd_quatd newOrientation = orientation *dq;
      iraspa_structure->structure()->setOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit rendererReloadData();

    reloadBoundingBox();
    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::EulerAngleX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }

  std::set<double> set = std::set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    simd_quatd orientation = iraspa_structure->structure()->orientation();
    double EulerAngle = orientation.EulerAngles().x * 180.0 / M_PI;
    set.insert(EulerAngle);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setEulerAngleX(double angle)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      simd_quatd orientation = iraspa_structure->structure()->orientation();
      double3 EulerAngles = orientation.EulerAngles();
      EulerAngles.x = angle * M_PI / 180.0;
      simd_quatd newOrientation = simd_quatd(EulerAngles);
      iraspa_structure->structure()->setOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();
    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::EulerAngleY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }

  std::set<double> set = std::set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    simd_quatd orientation = iraspa_structure->structure()->orientation();
    int EulerAngle = orientation.EulerAngles().y * 180.0 / M_PI;
    set.insert(EulerAngle);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setEulerAngleY(double angle)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      simd_quatd orientation = iraspa_structure->structure()->orientation();
      double3 EulerAngles = orientation.EulerAngles();
      EulerAngles.y = angle * M_PI / 180.0;
      simd_quatd newOrientation = simd_quatd(EulerAngles);
      iraspa_structure->structure()->setOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();
    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::EulerAngleZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }

  std::set<double> set = std::set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    simd_quatd orientation = iraspa_structure->structure()->orientation();
    int EulerAngle = orientation.EulerAngles().z * 180.0 / M_PI;
    set.insert(EulerAngle);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }
  return std::nullopt;
}

void CellTreeWidgetController::setEulerAngleZ(double angle)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      simd_quatd orientation = iraspa_structure->structure()->orientation();
      double3 EulerAngles = orientation.EulerAngles();
      EulerAngles.z = angle * M_PI / 180.0;
      simd_quatd newOrientation = simd_quatd(EulerAngles);
      iraspa_structure->structure()->setOrientation(newOrientation);
      iraspa_structure->structure()->reComputeBoundingBox();
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();
    reloadEulerAngles();

    _mainWindow->documentWasModified();
  }
}


std::optional<double>  CellTreeWidgetController::originX()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->origin().x);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

void CellTreeWidgetController::setOriginX(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setOriginX(value);
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();

    _mainWindow->documentWasModified();
  }
}


std::optional<double>  CellTreeWidgetController::originY()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->origin().y);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

void CellTreeWidgetController::setOriginY(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setOriginY(value);
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);

    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();

    _mainWindow->documentWasModified();
  }
}


std::optional<double>  CellTreeWidgetController::originZ()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    set.insert(iraspa_structure->structure()->origin().z);
  }

  if(set.size() == 1)
  {
    return *(set.begin());
  }

  return std::nullopt;
}

void CellTreeWidgetController::setOriginZ(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setOriginZ(value);
    }

    SKBoundingBox box = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(box);


    emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
    emit rendererReloadData();

    reloadBoundingBox();

    _mainWindow->documentWasModified();
  }
}


// Transform content properties
//========================================================================================================================================

void CellTreeWidgetController::reloadTransformContentProperties()
{
  reloadFlipAxesProperties();
  reloadShiftAxesProperties();

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellTransformContentForm->applyPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void CellTreeWidgetController::reloadFlipAxesProperties()
{
  _cellTransformContentForm->flipAxisACheckBox->setDisabled(true);
  _cellTransformContentForm->flipAxisBCheckBox->setDisabled(true);
  _cellTransformContentForm->flipAxisCCheckBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellTransformContentForm->flipAxisACheckBox->setEnabled(_projectTreeNode->isEditable());
      _cellTransformContentForm->flipAxisBCheckBox->setEnabled(_projectTreeNode->isEditable());
      _cellTransformContentForm->flipAxisCCheckBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<bool> state = flipAxisA())
      {
        whileBlocking(_cellTransformContentForm->flipAxisACheckBox)->setTristate(false);
        whileBlocking(_cellTransformContentForm->flipAxisACheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_cellTransformContentForm->flipAxisACheckBox)->setTristate(true);
        whileBlocking(_cellTransformContentForm->flipAxisACheckBox)->setCheckState(Qt::PartiallyChecked);
      }

      if (std::optional<bool> state = flipAxisB())
      {
        whileBlocking(_cellTransformContentForm->flipAxisBCheckBox)->setTristate(false);
        whileBlocking(_cellTransformContentForm->flipAxisBCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_cellTransformContentForm->flipAxisBCheckBox)->setTristate(true);
        whileBlocking(_cellTransformContentForm->flipAxisBCheckBox)->setCheckState(Qt::PartiallyChecked);
      }

      if (std::optional<bool> state = flipAxisC())
      {
        whileBlocking(_cellTransformContentForm->flipAxisCCheckBox)->setTristate(false);
        whileBlocking(_cellTransformContentForm->flipAxisCCheckBox)->setCheckState(*state ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_cellTransformContentForm->flipAxisCCheckBox)->setTristate(true);
        whileBlocking(_cellTransformContentForm->flipAxisCCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void CellTreeWidgetController::reloadShiftAxesProperties()
{
  _cellTransformContentForm->shiftAxisADoubleSpinBox->setDisabled(true);
  _cellTransformContentForm->shiftAxisBDoubleSpinBox->setDisabled(true);
  _cellTransformContentForm->shiftAxisCDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellTransformContentForm->shiftAxisADoubleSpinBox->setEnabled(true);
      _cellTransformContentForm->shiftAxisADoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellTransformContentForm->shiftAxisBDoubleSpinBox->setEnabled(true);
      _cellTransformContentForm->shiftAxisBDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _cellTransformContentForm->shiftAxisCDoubleSpinBox->setEnabled(true);
      _cellTransformContentForm->shiftAxisCDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = shiftAxisA())
      {
        whileBlocking(_cellTransformContentForm->shiftAxisADoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellTransformContentForm->shiftAxisADoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = shiftAxisB())
      {
        whileBlocking(_cellTransformContentForm->shiftAxisBDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellTransformContentForm->shiftAxisBDoubleSpinBox)->setText("Mult. Val.");
      }

      if (std::optional<double> value = shiftAxisC())
      {
        whileBlocking(_cellTransformContentForm->shiftAxisCDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellTransformContentForm->shiftAxisCDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::applyTransformContent()
{
  if(_projectTreeNode->isEditable())
  {
    CellTreeWidgetAppliedCellContentShiftCommand *applyTransformContent = new CellTreeWidgetAppliedCellContentShiftCommand(_mainWindow, this, _projectStructure,
                                                                                                                        _iraspa_structures, nullptr);
    _projectTreeNode->representedObject()->undoManager().push(applyTransformContent);
  }
}

std::optional<bool> CellTreeWidgetController::flipAxisA()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->cell()->contentFlip().x;
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setFlipAxisA(bool state)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->cell()->setContentFlipX(state);
  }
  reloadTransformContentProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> CellTreeWidgetController::flipAxisB()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->cell()->contentFlip().y;
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setFlipAxisB(bool state)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->cell()->setContentFlipY(state);
  }
  reloadTransformContentProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<bool> CellTreeWidgetController::flipAxisC()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool value = iraspa_structure->structure()->cell()->contentFlip().z;
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setFlipAxisC(bool state)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->cell()->setContentFlipZ(state);
  }
  reloadTransformContentProperties();
  emit rendererReloadData();

  _mainWindow->documentWasModified();
}

std::optional<double> CellTreeWidgetController::shiftAxisA()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->cell()->contentShift().x;
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setShiftAxisA(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->cell()->setContentShiftX(value);
    }
    reloadTransformContentProperties();
    rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::shiftAxisB()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->cell()->contentShift().y;
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setShiftAxisB(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->cell()->setContentShiftY(value);
    }
    reloadTransformContentProperties();
    rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

std::optional<double> CellTreeWidgetController::shiftAxisC()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->cell()->contentShift().z;
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setShiftAxisC(double value)
{
  if(!_iraspa_structures.empty())
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->cell()->setContentShiftZ(value);
    }
    reloadTransformContentProperties();
    rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

// Structural properties
//========================================================================================================================================

void CellTreeWidgetController::reloadStructureProperties()
{
  reloadStructuralMaterialType();
  reloadStructuralMass();
  reloadStructuralDensity();
  reloadStructuralHeliumVoidFraction();
  reloadStructuralSpecificVolume();
  reloadStructuralAccessiblePoreVolume();
  reloadFrameworkProbeMoleculePopupBox();
  reloadStructuralVolumetricSurfaceArea();
  reloadStructuralGravimetricSurfaceArea();
  reloadStructuralNumberOfChannelSystems();
  reloadStructuralNumberOfInaccessiblePockets();
  reloadStructuralDimensionalityOfPoreSystem();
  reloadStructuralLargestOverallCavityDiameter();
  reloadStructuralRestrictingPoreDiameter();
  reloadStructuralLargestDiamtereAlongAViablePath();

  _cellStructuralForm->computeGravimetricSurfaceAreaPushButton->setDisabled(true);
  _cellStructuralForm->computeVolumetricSurfaceAreaPushButton->setDisabled(true);
  _cellStructuralForm->computeHeliumVoidFractionPushButton->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->computeGravimetricSurfaceAreaPushButton->setEnabled(_projectTreeNode->isEditable());
      _cellStructuralForm->computeVolumetricSurfaceAreaPushButton->setEnabled(_projectTreeNode->isEditable());
      _cellStructuralForm->computeHeliumVoidFractionPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void CellTreeWidgetController::reloadStructuralMaterialType()
{
  _cellStructuralForm->materialTypeComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->materialTypeComboBox->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void CellTreeWidgetController::reloadStructuralMass()
{
  _cellStructuralForm->massDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->massDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->massDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structuralMass())
      {
        whileBlocking(_cellStructuralForm->massDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->massDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralDensity()
{
  _cellStructuralForm->densityDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->densityDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->densityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structuralDensity())
      {
        whileBlocking(_cellStructuralForm->densityDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->densityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralHeliumVoidFraction()
{
  _cellStructuralForm->heliumVoidFractionDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->heliumVoidFractionDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->heliumVoidFractionDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structureHeliumVoidFraction())
      {
        whileBlocking(_cellStructuralForm->heliumVoidFractionDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->heliumVoidFractionDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralSpecificVolume()
{
  _cellStructuralForm->specificVolumeDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->specificVolumeDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->specificVolumeDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structureSpecificVolume())
      {
        whileBlocking(_cellStructuralForm->specificVolumeDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->specificVolumeDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralAccessiblePoreVolume()
{
  _cellStructuralForm->accessiblePoreVolumeDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->accessiblePoreVolumeDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->accessiblePoreVolumeDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structureAccessiblePoreVolume())
      {
        whileBlocking(_cellStructuralForm->accessiblePoreVolumeDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->accessiblePoreVolumeDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadFrameworkProbeMoleculePopupBox()
{
  _cellStructuralForm->probeMoleculeComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->probeMoleculeComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::ProbeMolecule> type=frameworkProbeMolecule())
      {
        if(int index = _cellStructuralForm->probeMoleculeComboBox->findText("Mult. Val."); index>=0)
        {
          whileBlocking(_cellStructuralForm->probeMoleculeComboBox)->removeItem(index);
        }
        if(int(*type)<0)
        {
         whileBlocking(_cellStructuralForm->probeMoleculeComboBox)->setCurrentIndex(int(Structure::ProbeMolecule::multiple_values));
        }
        else
        {
          whileBlocking(_cellStructuralForm->probeMoleculeComboBox)->setCurrentIndex(int(*type));
        }
      }
      else
      {
        if(int index = _cellStructuralForm->probeMoleculeComboBox->findText("Mult. Val."); index<0)
        {
          whileBlocking(_cellStructuralForm->probeMoleculeComboBox)->addItem("Mult. Val.");
        }
        whileBlocking(_cellStructuralForm->probeMoleculeComboBox)->setCurrentText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralVolumetricSurfaceArea()
{
  _cellStructuralForm->volumetricSurfaceAreaDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->volumetricSurfaceAreaDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->volumetricSurfaceAreaDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structureVolumetricNitrogenSurfaceArea())
      {
        whileBlocking(_cellStructuralForm->volumetricSurfaceAreaDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->volumetricSurfaceAreaDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralGravimetricSurfaceArea()
{
  _cellStructuralForm->gravimetricSurfaceAreaDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->gravimetricSurfaceAreaDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->gravimetricSurfaceAreaDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structureGravimetricNitrogenSurfaceArea())
      {
        whileBlocking(_cellStructuralForm->gravimetricSurfaceAreaDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->gravimetricSurfaceAreaDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralNumberOfChannelSystems()
{
  _cellStructuralForm->numberOfChannelSystemsSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->numberOfChannelSystemsSpinBox->setEnabled(true);
      _cellStructuralForm->numberOfChannelSystemsSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = structureNumberOfChannelSystems())
      {
        whileBlocking(_cellStructuralForm->numberOfChannelSystemsSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->numberOfChannelSystemsSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralNumberOfInaccessiblePockets()
{
  _cellStructuralForm->numberOfInaccessiblePocketsSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->numberOfInaccessiblePocketsSpinBox->setEnabled(true);
      _cellStructuralForm->numberOfInaccessiblePocketsSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = structureNumberOfInaccessiblePockets())
      {
        whileBlocking(_cellStructuralForm->numberOfInaccessiblePocketsSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->numberOfInaccessiblePocketsSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralDimensionalityOfPoreSystem()
{
  _cellStructuralForm->dimensionalityOfPoreSystemSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->dimensionalityOfPoreSystemSpinBox->setEnabled(true);
      _cellStructuralForm->dimensionalityOfPoreSystemSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = structureDimensionalityOfPoreSystem())
      {
        whileBlocking(_cellStructuralForm->dimensionalityOfPoreSystemSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->dimensionalityOfPoreSystemSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralLargestOverallCavityDiameter()
{
  _cellStructuralForm->largestOverallCavityDiameterDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->largestOverallCavityDiameterDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->largestOverallCavityDiameterDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structureLargestCavityDiameter())
      {
        whileBlocking(_cellStructuralForm->largestOverallCavityDiameterDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->largestOverallCavityDiameterDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralRestrictingPoreDiameter()
{
  _cellStructuralForm->restrictingPoreDiameterDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->restrictingPoreDiameterDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->restrictingPoreDiameterDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structureRestrictingPoreLimitingDiameter())
      {
        whileBlocking(_cellStructuralForm->restrictingPoreDiameterDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->restrictingPoreDiameterDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadStructuralLargestDiamtereAlongAViablePath()
{
  _cellStructuralForm->largestDiameterAlongAViablePathDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellStructuralForm->largestDiameterAlongAViablePathDoubleSpinBox->setEnabled(true);
      _cellStructuralForm->largestDiameterAlongAViablePathDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = structureLargestCavityDiameterAlongAviablePath())
      {
        whileBlocking(_cellStructuralForm->largestDiameterAlongAViablePathDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellStructuralForm->largestDiameterAlongAViablePathDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}


std::optional<double> CellTreeWidgetController::structuralMass()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureMass();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::structuralDensity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureDensity();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::structureHeliumVoidFraction()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureHeliumVoidFraction();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


std::optional<double> CellTreeWidgetController::structureSpecificVolume()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureSpecificVolume();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::structureAccessiblePoreVolume()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureAccessiblePoreVolume();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setFrameworkProbeMolecule(int value)
{
  if(value>=0 && value<int(Structure::ProbeMolecule::multiple_values))
  {
    for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setFrameworkProbeMolecule(Structure::ProbeMolecule(value));
      iraspa_structure->structure()->recheckRepresentationStyle();
    }

    std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
    std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                    [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

    emit computeNitrogenSurfaceArea(render_structures);
    this->reloadStructureProperties();

    reloadFrameworkProbeMoleculePopupBox();

    _mainWindow->documentWasModified();
  }
}

std::optional<Structure::ProbeMolecule> CellTreeWidgetController::frameworkProbeMolecule()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<Structure::ProbeMolecule, enum_hash> set = std::unordered_set<Structure::ProbeMolecule, enum_hash>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    Structure::ProbeMolecule value = iraspa_structure->structure()->frameworkProbeMolecule();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::structureVolumetricNitrogenSurfaceArea()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureVolumetricNitrogenSurfaceArea();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::structureGravimetricNitrogenSurfaceArea()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureGravimetricNitrogenSurfaceArea();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<int> CellTreeWidgetController::structureNumberOfChannelSystems()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureNumberOfChannelSystems();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setStructureNumberOfChannelSystems(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setStructureNumberOfChannelSystems(value);
  }

  _mainWindow->documentWasModified();
}

std::optional<int> CellTreeWidgetController::structureNumberOfInaccessiblePockets()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    int value = iraspa_structure->structure()->structureNumberOfInaccessiblePockets();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setStructureNumberOfInaccessiblePockets(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setStructureNumberOfInaccessiblePockets(value);
  }
  _mainWindow->documentWasModified();
}

std::optional<int> CellTreeWidgetController::structureDimensionalityOfPoreSystem()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    int value = iraspa_structure->structure()->structureDimensionalityOfPoreSystem();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setStructureDimensionalityOfPoreSystem(int value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setStructureDimensionalityOfPoreSystem(value);
  }
  _mainWindow->documentWasModified();
}

std::optional<double> CellTreeWidgetController::structureLargestCavityDiameter()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureLargestCavityDiameter();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setStructureLargestCavityDiameter(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setStructureLargestCavityDiameter(value);
  }
  _mainWindow->documentWasModified();
}

std::optional<double> CellTreeWidgetController::structureRestrictingPoreLimitingDiameter()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureRestrictingPoreLimitingDiameter();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setStructureRestrictingPoreLimitingDiameter(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setStructureRestrictingPoreLimitingDiameter(value);
  }
  _mainWindow->documentWasModified();
}

std::optional<double> CellTreeWidgetController::structureLargestCavityDiameterAlongAviablePath()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->structureLargestCavityDiameterAlongAviablePath();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setStructureLargestCavityDiameterAlongAviablePath(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setStructureLargestCavityDiameterAlongAviablePath(value);
  }
  _mainWindow->documentWasModified();
}

// Symmetry properties
//========================================================================================================================================

void CellTreeWidgetController::reloadSymmetryProperties()
{
  reloadSpaceGroupHallName();
  reloadSpaceGroupITNumber();
  reloadSpaceGroupHolohedry();
  reloadSpaceGroupQualifier();
  reloadSpaceGroupPrecision();
  reloadSpaceGroupCenteringType();
  reloadSpaceGroupCenteringVectors();
  reloadSpaceGroupInversion();
  reloadSpaceGroupInversionCenter();
  reloadSpaceGroupCentroSymmetric();
  reloadSpaceGroupEnantiomorphic();
  reloadSpaceGroupLaueGroup();
  reloadSpaceGroupPointGroup();
  reloadSpaceGroupSchoenfliesSymbol();
  reloadSpaceGroupSymmorphicity();
}


void CellTreeWidgetController::reloadSpaceGroupHallName()
{
  _cellSymmetryForm->spaceGroupHallNamecomboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->spaceGroupHallNamecomboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<int> type = symmetrySpaceGroupHallNumber())
      {
        if(int index = _cellSymmetryForm->spaceGroupHallNamecomboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_cellSymmetryForm->spaceGroupHallNamecomboBox)->removeItem(index);
        }
        whileBlocking(_cellSymmetryForm->spaceGroupHallNamecomboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _cellSymmetryForm->spaceGroupHallNamecomboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_cellSymmetryForm->spaceGroupHallNamecomboBox)->addItem("Multiple values");
        }
        whileBlocking(_cellSymmetryForm->spaceGroupHallNamecomboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupITNumber()
{
  _cellSymmetryForm->spaceGroupITNumberComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->spaceGroupITNumberComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<int> type = symmetrySpaceGroupStamdardNumber())
      {
        if(int index = _cellSymmetryForm->spaceGroupITNumberComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_cellSymmetryForm->spaceGroupITNumberComboBox)->removeItem(index);
        }
        whileBlocking(_cellSymmetryForm->spaceGroupITNumberComboBox)->setCurrentIndex(int(*type));
      }
      else
      {
        if(int index = _cellSymmetryForm->spaceGroupITNumberComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_cellSymmetryForm->spaceGroupITNumberComboBox)->addItem("Multiple values");
        }
        whileBlocking(_cellSymmetryForm->spaceGroupITNumberComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupHolohedry()
{
  _cellSymmetryForm->holohedryLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->holohedryLineEdit->setEnabled(true);
      _cellSymmetryForm->holohedryLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<QString> string = symmetryHolohedryString())
      {
        whileBlocking(_cellSymmetryForm->holohedryLineEdit)->setText(*string);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->holohedryLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupQualifier()
{
  _cellSymmetryForm->qualifierLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->qualifierLineEdit->setEnabled(true);
      _cellSymmetryForm->qualifierLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<QString> string = symmetryQualifierString())
      {
        whileBlocking(_cellSymmetryForm->qualifierLineEdit)->setText(*string);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->qualifierLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupPrecision()
{
  _cellSymmetryForm->precisionDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->precisionDoubleSpinBox->setEnabled(true);
      _cellSymmetryForm->precisionDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<double> value = symmetryPrecision())
      {
        whileBlocking(_cellSymmetryForm->precisionDoubleSpinBox)->setValue(*value);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->precisionDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupCenteringType()
{
  _cellSymmetryForm->centeringLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->centeringLineEdit->setEnabled(true);
      _cellSymmetryForm->centeringLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<QString> value = symmetryCenteringString())
      {
        whileBlocking(_cellSymmetryForm->centeringLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->centeringLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupCenteringVectors()
{
  _cellSymmetryForm->centerintVector1LineEdit->setDisabled(true);
  _cellSymmetryForm->centerintVector2LineEdit->setDisabled(true);
  _cellSymmetryForm->centerintVector3LineEdit->setDisabled(true);
  _cellSymmetryForm->centerintVector4LineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->centerintVector1LineEdit->setEnabled(true);
      _cellSymmetryForm->centerintVector2LineEdit->setEnabled(true);
      _cellSymmetryForm->centerintVector3LineEdit->setEnabled(true);
      _cellSymmetryForm->centerintVector4LineEdit->setEnabled(true);

      _cellSymmetryForm->centerintVector1LineEdit->setReadOnly(!_projectTreeNode->isEditable());
      _cellSymmetryForm->centerintVector2LineEdit->setReadOnly(!_projectTreeNode->isEditable());
      _cellSymmetryForm->centerintVector3LineEdit->setReadOnly(!_projectTreeNode->isEditable());
      _cellSymmetryForm->centerintVector4LineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> value = symmetrySpaceGroupHallNumber())
      {
        std::vector<QString> latticeVector =  SKSpaceGroup::latticeTranslationStrings(*value);
        whileBlocking(_cellSymmetryForm->centerintVector1LineEdit)->setText(latticeVector[0]);
        whileBlocking(_cellSymmetryForm->centerintVector2LineEdit)->setText(latticeVector[1]);
        whileBlocking(_cellSymmetryForm->centerintVector3LineEdit)->setText(latticeVector[2]);
        whileBlocking(_cellSymmetryForm->centerintVector4LineEdit)->setText(latticeVector[3]);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->centerintVector1LineEdit)->setText("Mult. Val.");
        whileBlocking(_cellSymmetryForm->centerintVector2LineEdit)->setText("Mult. Val.");
        whileBlocking(_cellSymmetryForm->centerintVector3LineEdit)->setText("Mult. Val.");
        whileBlocking(_cellSymmetryForm->centerintVector4LineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupInversion()
{
  _cellSymmetryForm->inversionLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->inversionLineEdit->setEnabled(true);
      _cellSymmetryForm->inversionLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<bool> type = symmetryInversion())
      {
        whileBlocking(_cellSymmetryForm->inversionLineEdit)->setText(*type ? "yes" : "no");
      }
      else
      {
        whileBlocking(_cellSymmetryForm->inversionLineEdit)->setText("Multiple values");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupInversionCenter()
{
  _cellSymmetryForm->inversionCenterLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->inversionCenterLineEdit->setEnabled(true);
      _cellSymmetryForm->inversionCenterLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<QString> value = symmetryInversionCenterString())
      {
        whileBlocking(_cellSymmetryForm->inversionCenterLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->inversionCenterLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupCentroSymmetric()
{
  _cellSymmetryForm->centrosymmetricLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->centrosymmetricLineEdit->setEnabled(true);
      _cellSymmetryForm->centrosymmetricLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<bool> type = symmetryCentrosymmetric())
      {
        whileBlocking(_cellSymmetryForm->centrosymmetricLineEdit)->setText(*type ? "yes" : "no");
      }
      else
      {
        whileBlocking(_cellSymmetryForm->centrosymmetricLineEdit)->setText("Multiple values");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupEnantiomorphic()
{
  _cellSymmetryForm->enantiomorphicLineEdit->setDisabled(true);
  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->enantiomorphicLineEdit->setEnabled(true);
      _cellSymmetryForm->enantiomorphicLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<bool> type = symmetryEnantiomorphic())
      {
        whileBlocking(_cellSymmetryForm->enantiomorphicLineEdit)->setText(*type ? "yes" : "no");
      }
      else
      {
        whileBlocking(_cellSymmetryForm->enantiomorphicLineEdit)->setText("Multiple values");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupLaueGroup()
{
  _cellSymmetryForm->LaueGroupLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->LaueGroupLineEdit->setEnabled(true);
      _cellSymmetryForm->LaueGroupLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> type = symmetryPointGroup())
      {
        QString name = SKPointGroup::pointGroupData[*type].LaueString();
        whileBlocking(_cellSymmetryForm->LaueGroupLineEdit)->setText(name);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->LaueGroupLineEdit)->setText("Multiple values");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupPointGroup()
{
  _cellSymmetryForm->pointGroupLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->pointGroupLineEdit->setEnabled(true);
      _cellSymmetryForm->pointGroupLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> type = symmetryPointGroup())
      {
        QString name = SKPointGroup::pointGroupData[*type].symbol();
        whileBlocking(_cellSymmetryForm->pointGroupLineEdit)->setText(name);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->pointGroupLineEdit)->setText("Multiple values");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupSchoenfliesSymbol()
{
  _cellSymmetryForm->SchoenfliesLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->SchoenfliesLineEdit->setEnabled(true);
      _cellSymmetryForm->SchoenfliesLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<int> type = symmetryPointGroup())
      {
        QString name = SKPointGroup::pointGroupData[*type].schoenflies();
        whileBlocking(_cellSymmetryForm->SchoenfliesLineEdit)->setText(name);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->SchoenfliesLineEdit)->setText("Multiple values");
      }
    }
  }
}

void CellTreeWidgetController::reloadSpaceGroupSymmorphicity()
{
  _cellSymmetryForm->symmorphicityLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _cellSymmetryForm->symmorphicityLineEdit->setEnabled(true);
      _cellSymmetryForm->symmorphicityLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if (std::optional<QString> string = symmetrySymmorphicity())
      {
        whileBlocking(_cellSymmetryForm->symmorphicityLineEdit)->setText(*string);
      }
      else
      {
        whileBlocking(_cellSymmetryForm->symmorphicityLineEdit)->setText("Multiple values");
      }
    }
  }
}

std::optional<int> CellTreeWidgetController::symmetrySpaceGroupHallNumber()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    int value = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().HallNumber();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setSymmetrySpaceGroupHallNumber(int value)
{
  CellTreeWidgetChangeSpaceGroupCommand *changeCommand = new CellTreeWidgetChangeSpaceGroupCommand(_mainWindow, this, _projectStructure, _iraspa_structures, value, nullptr);

  _projectTreeNode->representedObject()->undoManager().push(changeCommand);

  reloadSymmetryProperties();

  _mainWindow->documentWasModified();
}

std::optional<int> CellTreeWidgetController::symmetrySpaceGroupStamdardNumber()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    int value = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().number();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setSymmetrySpaceGroupStandardNumber(int value)
{
  int HallNumber = SKSpaceGroup::spaceGroupHallData[value].front();

  CellTreeWidgetChangeSpaceGroupCommand *changeCommand = new CellTreeWidgetChangeSpaceGroupCommand(_mainWindow, this, _projectStructure, _iraspa_structures, HallNumber, nullptr);

  _projectTreeNode->representedObject()->undoManager().push(changeCommand);

  reloadSymmetryProperties();

  _mainWindow->documentWasModified();
}

std::optional<QString> CellTreeWidgetController::symmetryHolohedryString()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    int pointGroup = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().pointGroup();
    QString value = SKPointGroup::pointGroupData[pointGroup].holohedryString();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<QString> CellTreeWidgetController::symmetryQualifierString()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().qualifier();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<double> CellTreeWidgetController::symmetryPrecision()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<double> set = std::unordered_set<double>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    double value = iraspa_structure->structure()->cell()->precision();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::setSymmetryPrecision(double value)
{
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->cell()->setPrecision(value);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> CellTreeWidgetController::symmetryCenteringString()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().centringString();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<bool> CellTreeWidgetController::symmetryInversion()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    bool inversion = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().inversionAtOrigin();
    set.insert(inversion);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


std::optional<QString> CellTreeWidgetController::symmetryInversionCenterString()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    int HallNumber = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().HallNumber();
    QString string = SKSpaceGroup::inversionCenterString(HallNumber);
    set.insert(string);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<bool> CellTreeWidgetController::symmetryCentrosymmetric()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    int pointGroup = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().pointGroup();
    bool centroSymmetric = SKPointGroup::pointGroupData[pointGroup].centrosymmetric();
    set.insert(centroSymmetric);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

std::optional<bool> CellTreeWidgetController::symmetryEnantiomorphic()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<bool> set = std::unordered_set<bool>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    int pointGroup = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().pointGroup();
    bool enantiomorphic = SKPointGroup::pointGroupData[pointGroup].enantiomorphic();
    set.insert(enantiomorphic);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


std::optional<int> CellTreeWidgetController::symmetryPointGroup()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::unordered_set<int> set = std::unordered_set<int>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    int pointGroup = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().pointGroup();
    set.insert(pointGroup);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


std::optional<QString> CellTreeWidgetController::symmetrySymmorphicity()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(std::shared_ptr<iRASPAStructure> iraspa_structure: _iraspa_structures)
  {
    QString symmorphicity = iraspa_structure->structure()->spaceGroup().spaceGroupSetting().symmorphicityString();
    set.insert(symmorphicity);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void CellTreeWidgetController::computeHeliumVoidFractionPushButton()
{
  std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
  std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                  [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

  emit computeHeliumVoidFraction(render_structures);
  this->reloadStructureProperties();

  _mainWindow->documentWasModified();
}

void CellTreeWidgetController::computeGravimetricSurfaceAreaPushButton()
{
  std::vector<std::shared_ptr<RKRenderStructure>> render_structures{};
  std::transform(_iraspa_structures.begin(),_iraspa_structures.end(),std::back_inserter(render_structures),
                  [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});

  emit computeNitrogenSurfaceArea(render_structures);
  this->reloadStructureProperties();

  _mainWindow->documentWasModified();
}

