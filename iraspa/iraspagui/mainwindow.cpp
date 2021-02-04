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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"

#include <QUrl>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QtConcurrent>
#include <QDesktopServices>
#include <QtGlobal>
#include <iraspakit.h>
#include <foundationkit.h>
#include <renderkit.h>
#include "glwidget.h"
#include "masterstackedwidget.h"
#include "importfiledialog.h"
#include <iraspatreeview.h>
#include "openiraspaformatdialog.h"
#include "saveiraspaformatdialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  qApp->installEventFilter(this);

  _documentData = std::make_shared<DocumentData>();

  // atomTreeView needs to update the bonds via the bondModel (used in undo/redo).
  // (the bondListView does not need to know the atomModel because deleting bonds does not change the atom-selection)
  ui->atomTreeView->setBondModel(ui->bondListView->bondModel());
  ui->atomTreeView->setSceneModel(ui->sceneTreeView->sceneModel());
  ui->atomTreeView->setNetChargeLineEdit(ui->atomChargeLineEdit);

  // the renderView needs to update via the atomModel and bondModel (used in undo/redo).
  ui->stackedRenderers->setAtomModel(ui->atomTreeView->atomModel());
  ui->stackedRenderers->setBondModel(ui->bondListView->bondModel());

  ui->sceneTreeView->setFrameModel(ui->frameListView->frameModel());

  // create the file, edit, help menus
  this->createMenus();

  // connect the toolbar slide left-, right-, down- panels
  QObject::connect(ui->mainToolBar->rightPanel(),&QToolButton::clicked,this,&MainWindow::slideRightPanel);
  QObject::connect(ui->mainToolBar->downPanel(),&QToolButton::clicked,this,&MainWindow::slideDownPanel);
  QObject::connect(ui->mainToolBar->leftPanel(),&QToolButton::clicked,this,&MainWindow::slideLeftPanel);

  // propagate mainWindow to all interested controllers
  this->propagateMainWindow(this,this);

  // propagate the logWidget to all interested controller
  this->propagateLogReporter(this->ui->logPlainTextEdit,this);

  ui->projectTreeView->setDocumentData(_documentData);


  this->propagateProject(std::shared_ptr<ProjectTreeNode>(nullptr), this);

  // monitor the opengl mouse events in stackRenderers
  ui->glwidget->installEventFilter(ui->stackedRenderers);

  ui->detailTabViewController->setStyleSheet("QTabWidget::tab-bar { left: 0;}");

  // connect the project-toolbar to the stackedWidget
  #if QT_VERSION < QT_VERSION_CHECK(5, 15, 0) 
    QObject::connect(ui->masterToolBar->mapper(), static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped),ui->masterStackedWidget, &MasterStackedWidget::reloadTab);
  #else
    QObject::connect(ui->masterToolBar->mapper(), static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mappedInt),ui->masterStackedWidget, &MasterStackedWidget::reloadTab);
  #endif

  // connect the sceneTreeView
  QObject::connect(ui->sceneTreeView, &SceneTreeView::updateRenderer,ui->stackedRenderers, &RenderStackedWidget::reloadData);
  QObject::connect(ui->sceneTreeView, &SceneTreeView::setFlattenedSelectedFrames, ui->infoTreeWidget, &InfoTreeWidgetController::setFlattenedSelectedFrames);
  QObject::connect(ui->sceneTreeView, &SceneTreeView::setFlattenedSelectedFrames, ui->appearanceTreeWidget, &AppearanceTreeWidgetController::setFlattenedSelectedFrames);
  QObject::connect(ui->sceneTreeView, &SceneTreeView::setFlattenedSelectedFrames, ui->cellTreeWidget, &CellTreeWidgetController::setFlattenedSelectedFrames);
  QObject::connect(ui->sceneTreeView, &SceneTreeView::setSelectedFrame, ui->atomTreeView, &AtomTreeView::setSelectedFrame);
  QObject::connect(ui->sceneTreeView, &SceneTreeView::setSelectedFrame, ui->bondListView, &BondListView::setSelectedFrame);
  QObject::connect(ui->sceneTreeView, &SceneTreeView::setSelectedRenderFrames, ui->stackedRenderers, &RenderStackedWidget::setSelectedRenderFrames);
  QObject::connect(ui->sceneTreeView, &SceneTreeView::setSelectedMovie, ui->frameListView, &FrameListView::setSelectedMovie);
  QObject::connect(ui->sceneTreeView, &SceneTreeView::rendererReloadData,ui->stackedRenderers, &RenderStackedWidget::reloadRenderData);
  QObject::connect(ui->sceneTreeView, &SceneTreeView::invalidateCachedAmbientOcclusionTexture,ui->stackedRenderers, &RenderStackedWidget::invalidateCachedAmbientOcclusionTextures);

  // connect the frameListView
  QObject::connect(ui->frameListView, &FrameListView::updateRenderer,ui->stackedRenderers, &RenderStackedWidget::reloadData);
  QObject::connect(ui->frameListView, &FrameListView::setFlattenedSelectedFrames, ui->infoTreeWidget, &InfoTreeWidgetController::setFlattenedSelectedFrames);
  QObject::connect(ui->frameListView, &FrameListView::setFlattenedSelectedFrames, ui->appearanceTreeWidget, &AppearanceTreeWidgetController::setFlattenedSelectedFrames);
  QObject::connect(ui->frameListView, &FrameListView::setFlattenedSelectedFrames, ui->cellTreeWidget, &CellTreeWidgetController::setFlattenedSelectedFrames);
  QObject::connect(ui->frameListView, &FrameListView::setSelectedFrame, ui->atomTreeView, &AtomTreeView::setSelectedFrame);
  QObject::connect(ui->frameListView, &FrameListView::setSelectedFrame, ui->bondListView, &BondListView::setSelectedFrame);
  QObject::connect(ui->frameListView, &FrameListView::setSelectedRenderFrames, ui->stackedRenderers, &RenderStackedWidget::setSelectedRenderFrames);
  QObject::connect(ui->frameListView, &FrameListView::rendererReloadData,ui->stackedRenderers, &RenderStackedWidget::reloadRenderData);
  QObject::connect(ui->frameListView, &FrameListView::invalidateCachedAmbientOcclusionTexture,ui->stackedRenderers, &RenderStackedWidget::invalidateCachedAmbientOcclusionTextures);

  // connect the camera tab
  QObject::connect(ui->cameraTreeWidget, &CameraTreeWidgetController::updateRenderer,ui->stackedRenderers, &RenderStackedWidget::redraw);
  QObject::connect(ui->cameraTreeWidget, &CameraTreeWidgetController::rendererReloadBackgroundImage,ui->stackedRenderers, &RenderStackedWidget::reloadBackgroundImage);
  QObject::connect(ui->cameraTreeWidget, &CameraTreeWidgetController::rendererReloadSelectionData,ui->stackedRenderers, &RenderStackedWidget::reloadSelectionData);
  QObject::connect(ui->cameraTreeWidget, &CameraTreeWidgetController::rendererCreatePicture,ui->stackedRenderers, &RenderStackedWidget::createPicture);
  QObject::connect(ui->cameraTreeWidget, &CameraTreeWidgetController::rendererCreateMovie,ui->stackedRenderers, &RenderStackedWidget::createMovie);

  // connect the elements tab
  QObject::connect(ui->elementListWidget, &ElementListWidgetController::rendererReloadData,ui->stackedRenderers, &RenderStackedWidget::reloadRenderData);
  QObject::connect(ui->elementListWidget, &ElementListWidgetController::invalidateCachedAmbientOcclusionTextures,ui->stackedRenderers, &RenderStackedWidget::invalidateCachedAmbientOcclusionTextures);
  QObject::connect(ui->elementListWidget, &ElementListWidgetController::invalidateCachedIsoSurfaces,ui->stackedRenderers, &RenderStackedWidget::invalidateCachedIsoSurfaces);

  // copnnect the appearance tab
  QObject::connect(ui->appearanceTreeWidget, &AppearanceTreeWidgetController::rendererReloadData,ui->stackedRenderers, &RenderStackedWidget::reloadRenderData);
  QObject::connect(ui->appearanceTreeWidget, &AppearanceTreeWidgetController::redrawRenderer,ui->stackedRenderers, &RenderStackedWidget::redraw);
  QObject::connect(ui->appearanceTreeWidget, &AppearanceTreeWidgetController::redrawWithQuality,ui->stackedRenderers, &RenderStackedWidget::redrawWithQuality);
  QObject::connect(ui->appearanceTreeWidget, &AppearanceTreeWidgetController::invalidateCachedAmbientOcclusionTextures,ui->stackedRenderers, &RenderStackedWidget::invalidateCachedAmbientOcclusionTextures);
  QObject::connect(ui->appearanceTreeWidget, &AppearanceTreeWidgetController::invalidateCachedIsoSurfaces,ui->stackedRenderers, &RenderStackedWidget::invalidateCachedIsoSurfaces);

  // connect the cell tab
  QObject::connect(ui->cellTreeWidget, &CellTreeWidgetController::reloadAllViews,this, &MainWindow::reloadAllViews);
  QObject::connect(ui->cellTreeWidget, &CellTreeWidgetController::rendererReloadData,ui->stackedRenderers, &RenderStackedWidget::reloadRenderData);
  QObject::connect(ui->cellTreeWidget, &CellTreeWidgetController::redrawWithQuality,ui->stackedRenderers, &RenderStackedWidget::redrawWithQuality);
  QObject::connect(ui->cellTreeWidget, &CellTreeWidgetController::computeHeliumVoidFraction,ui->stackedRenderers, &RenderStackedWidget::computeHeliumVoidFraction);
  QObject::connect(ui->cellTreeWidget, &CellTreeWidgetController::computeNitrogenSurfaceArea,ui->stackedRenderers, &RenderStackedWidget::computeNitrogenSurfaceArea);
  QObject::connect(ui->cellTreeWidget, &CellTreeWidgetController::invalidateCachedAmbientOcclusionTextures,ui->stackedRenderers, &RenderStackedWidget::invalidateCachedAmbientOcclusionTextures);
  QObject::connect(ui->cellTreeWidget, &CellTreeWidgetController::invalidateCachedIsoSurfaces,ui->stackedRenderers, &RenderStackedWidget::invalidateCachedIsoSurfaces);

  // connect the atom tab
  QObject::connect(ui->atomTreeView, &AtomTreeView::rendererReloadData,ui->stackedRenderers, &RenderStackedWidget::reloadRenderData);
  QObject::connect(ui->atomTreeView, &AtomTreeView::invalidateCachedAmbientOcclusionTextures,ui->stackedRenderers, &RenderStackedWidget::invalidateCachedAmbientOcclusionTextures);

  // connect the bond tab
  QObject::connect(ui->bondListView, &BondListView::rendererReloadData,ui->stackedRenderers, &RenderStackedWidget::reloadRenderData);

  // connect the renderer
  QObject::connect(ui->stackedRenderers, &RenderStackedWidget::updateCameraModelViewMatrix, ui->cameraTreeWidget, &CameraTreeWidgetController::reloadCameraModelViewMatrix);
  QObject::connect(ui->stackedRenderers, &RenderStackedWidget::updateCameraEulerAngles, ui->cameraTreeWidget, &CameraTreeWidgetController::reloadCameraEulerAngles);
  QObject::connect(ui->stackedRenderers, &RenderStackedWidget::updateCameraProjection, ui->cameraTreeWidget, &CameraTreeWidgetController::reloadCameraProjection);
  QObject::connect(ui->stackedRenderers, &RenderStackedWidget::updateCameraResetDirection, ui->cameraTreeWidget, &CameraTreeWidgetController::reloadCameraResetDirection);
  QObject::connect(ui->stackedRenderers, &RenderStackedWidget::rendererWidgetResized,ui->detailTabViewController, &DetailTabViewController::rendererWidgetResized);
  QObject::connect(ui->stackedRenderers, &RenderStackedWidget::updateAtomSelection,ui->atomTreeView, &AtomTreeView::reloadSelection);
  QObject::connect(ui->stackedRenderers, &RenderStackedWidget::updateBondSelection,ui->bondListView, &BondListView::reloadSelection);

  // forward to renderer
  QObject::connect(this, &MainWindow::invalidateCachedAmbientOcclusionTextures, ui->stackedRenderers, &RenderStackedWidget::invalidateCachedAmbientOcclusionTextures);
  QObject::connect(this, &MainWindow::invalidateCachedIsoSurfaces, ui->stackedRenderers, &RenderStackedWidget::invalidateCachedIsoSurfaces);
  QObject::connect(this, &MainWindow::rendererReloadData, ui->stackedRenderers, &RenderStackedWidget::reloadRenderData);

  if (GLWidget* widget = dynamic_cast<GLWidget*>(ui->stackedRenderers->currentWidget()))
  {
    QObject::connect(widget, &GLWidget::pressedTranslateCartesianMinusX, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateCartesianMinusX);
    QObject::connect(widget, &GLWidget::pressedTranslateCartesianPlusX, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateCartesianPlusX);
    QObject::connect(widget, &GLWidget::pressedTranslateCartesianMinusY, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateCartesianMinusY);
    QObject::connect(widget, &GLWidget::pressedTranslateCartesianPlusY, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateCartesianPlusY);
    QObject::connect(widget, &GLWidget::pressedTranslateCartesianMinusZ, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateCartesianMinusZ);
    QObject::connect(widget, &GLWidget::pressedTranslateCartesianPlusZ, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateCartesianPlusZ);
    QObject::connect(widget, &GLWidget::pressedRotateCartesianMinusX, ui->stackedRenderers, &RenderStackedWidget::pressedRotateCartesianMinusX);
    QObject::connect(widget, &GLWidget::pressedRotateCartesianPlusX, ui->stackedRenderers, &RenderStackedWidget::pressedRotateCartesianPlusX);
    QObject::connect(widget, &GLWidget::pressedRotateCartesianMinusY, ui->stackedRenderers, &RenderStackedWidget::pressedRotateCartesianMinusY);
    QObject::connect(widget, &GLWidget::pressedRotateCartesianPlusY, ui->stackedRenderers, &RenderStackedWidget::pressedRotateCartesianPlusY);
    QObject::connect(widget, &GLWidget::pressedRotateCartesianMinusZ, ui->stackedRenderers, &RenderStackedWidget::pressedRotateCartesianMinusZ);
    QObject::connect(widget, &GLWidget::pressedRotateCartesianPlusZ, ui->stackedRenderers, &RenderStackedWidget::pressedRotateCartesianPlusZ);

    QObject::connect(widget, &GLWidget::pressedTranslateBodyFrameMinusX, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateBodyFrameMinusX);
    QObject::connect(widget, &GLWidget::pressedTranslateBodyFramePlusX, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateBodyFramePlusX);
    QObject::connect(widget, &GLWidget::pressedTranslateBodyFrameMinusY, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateBodyFrameMinusY);
    QObject::connect(widget, &GLWidget::pressedTranslateBodyFramePlusY, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateBodyFramePlusY);
    QObject::connect(widget, &GLWidget::pressedTranslateBodyFrameMinusZ, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateBodyFrameMinusZ);
    QObject::connect(widget, &GLWidget::pressedTranslateBodyFramePlusZ, ui->stackedRenderers, &RenderStackedWidget::pressedTranslateBodyFramePlusZ);
    QObject::connect(widget, &GLWidget::pressedRotateBodyFrameMinusX, ui->stackedRenderers, &RenderStackedWidget::pressedRotateBodyFrameMinusX);
    QObject::connect(widget, &GLWidget::pressedRotateBodyFramePlusX, ui->stackedRenderers, &RenderStackedWidget::pressedRotateBodyFramePlusX);
    QObject::connect(widget, &GLWidget::pressedRotateBodyFrameMinusY, ui->stackedRenderers, &RenderStackedWidget::pressedRotateBodyFrameMinusY);
    QObject::connect(widget, &GLWidget::pressedRotateBodyFramePlusY, ui->stackedRenderers, &RenderStackedWidget::pressedRotateBodyFramePlusY);
    QObject::connect(widget, &GLWidget::pressedRotateBodyFrameMinusZ, ui->stackedRenderers, &RenderStackedWidget::pressedRotateBodyFrameMinusZ);
    QObject::connect(widget, &GLWidget::pressedRotateBodyFramePlusZ, ui->stackedRenderers, &RenderStackedWidget::pressedRotateBodyFramePlusZ);
  }


  QObject::connect(ui->addProjectToolButton, &QToolButton::pressed,ui->projectTreeView, &ProjectTreeView::addProjectStructure);
  QObject::connect(ui->removeProjectToolButton, &QToolButton::pressed,ui->projectTreeView, &ProjectTreeView::deleteSelection);

  QObject::connect(ui->addMovieToolButton, &QToolButton::pressed,ui->sceneTreeView, &SceneTreeView::newCrystal);
  QObject::connect(ui->removeMovieToolButton, &QToolButton::pressed,ui->sceneTreeView, &SceneTreeView::deleteSelection);

  QObject::connect(ui->addFrameToolButton, &QToolButton::pressed,ui->frameListView, &FrameListView::addFrame);
  QObject::connect(ui->removeFrameToolButton, &QToolButton::pressed,ui->frameListView, &FrameListView::deleteSelection);

  QObject::connect(ui->addAtomToolButton, &QToolButton::pressed, ui->atomTreeView, static_cast<void (AtomTreeView::*)()>(&AtomTreeView::addAtom));
  QObject::connect(ui->removeAtomToolButton, &QToolButton::pressed, ui->atomTreeView, &AtomTreeView::deleteSelection);


  recheckRemovalButtons();
  updateMenuToProjectTab();

  readDatabaseGalleryWorker = QtConcurrent::run([this]() -> std::shared_ptr<DocumentData> { return readDatabase("libraryofstructures.irspdoc");});
  QFutureWatcher<std::shared_ptr<DocumentData>> *watcherDatabaseGalleryReadDone = new QFutureWatcher<std::shared_ptr<DocumentData>>(this);
  connect(watcherDatabaseGalleryReadDone, &QFutureWatcher<std::shared_ptr<DocumentData>>::finished, this, &MainWindow::insertDatabaseGallery);
  watcherDatabaseGalleryReadDone->setFuture(readDatabaseGalleryWorker);

  readDatabaseCoREMOFWorker = QtConcurrent::run([this]() -> std::shared_ptr<DocumentData> { return readDatabase("databasecoremof.irspdoc");});
  QFutureWatcher<std::shared_ptr<DocumentData>> *watcherDatabaseCoREMOFReadDone = new QFutureWatcher<std::shared_ptr<DocumentData>>(this);
  connect(watcherDatabaseCoREMOFReadDone, &QFutureWatcher<std::shared_ptr<DocumentData>>::finished, this, &MainWindow::insertDatabaseCoREMOF);
  watcherDatabaseCoREMOFReadDone->setFuture(readDatabaseCoREMOFWorker);

  readDatabaseCoREMOFDDECWorker = QtConcurrent::run([this]() -> std::shared_ptr<DocumentData> { return readDatabase("databasecoremofddec.irspdoc");});
  QFutureWatcher<std::shared_ptr<DocumentData>> *watcherDatabaseCoREMOFDDECReadDone = new QFutureWatcher<std::shared_ptr<DocumentData>>(this);
  connect(watcherDatabaseCoREMOFReadDone, &QFutureWatcher<std::shared_ptr<DocumentData>>::finished, this, &MainWindow::insertDatabaseCoREMOFDDEC);
  watcherDatabaseCoREMOFDDECReadDone->setFuture(readDatabaseCoREMOFDDECWorker);

  readDatabaseIZAWorker = QtConcurrent::run([this]() -> std::shared_ptr<DocumentData> { return readDatabase("databaseiza.irspdoc");});
  QFutureWatcher<std::shared_ptr<DocumentData>> *watcherDatabaseIZAReadDone = new QFutureWatcher<std::shared_ptr<DocumentData>>(this);
  connect(watcherDatabaseIZAReadDone, &QFutureWatcher<std::shared_ptr<DocumentData>>::finished, this, &MainWindow::insertDatabaseIZA);
  watcherDatabaseIZAReadDone->setFuture(readDatabaseIZAWorker);

  setCurrentFile(QString());
}

MainWindow::~MainWindow()
{
  delete ui;
}

std::shared_ptr<DocumentData> MainWindow::readDatabase(QString fileName)
{
  #ifdef Q_OS_LINUX
    #ifdef USE_SNAP
      QDir dir("/snap/iraspa/current/usr/share/iraspa/");
    #else
      QDir dir("/usr/share/iraspa/");
      //QDir dir(QCoreApplication::applicationDirPath());
    #endif
    QUrl fileURL(dir.absoluteFilePath(fileName));
  #else
    QDir dir(QCoreApplication::applicationDirPath());
    QUrl fileURL(dir.absoluteFilePath(fileName));
  #endif
  QFileInfo info(fileURL.toString());

  if(info.exists())
  {

    ZipReader reader = ZipReader(fileURL.toString(), QIODevice::ReadOnly);

    QByteArray data = reader.fileData("nl.darkwing.iRASPA_projectData");
    QDataStream stream(&data, QIODevice::ReadOnly);

    std::shared_ptr<DocumentData> libraryData = std::make_shared<DocumentData>();
    try
    {
      stream >> libraryData;
    }
    catch (std::exception e)
    {
      return nullptr;
    }

    for (std::shared_ptr<ProjectTreeNode> localNode : libraryData->projectTreeController()->localProjects()->descendantNodes())
    {
      localNode->representedObject()->readData(reader);
      localNode->setType(ProjectTreeNode::Type::gallery);
      localNode->setIsEditable(false);
    }
    reader.close();

    return libraryData;
  }

  return nullptr;
}


void MainWindow::reportError(QString error)
{
  ui->logPlainTextEdit->logMessage(LogReporting::ErrorLevel::error,error);
}

void MainWindow::reportWarning(QString warning)
{
  ui->logPlainTextEdit->logMessage(LogReporting::ErrorLevel::warning, warning);
}


void MainWindow::insertDatabaseGallery()
{
  std::shared_ptr<DocumentData> database = readDatabaseGalleryWorker.result();

  if(database)
  {
    ui->projectTreeView->insertGalleryData(database);
  }
  else
  {
    ui->logPlainTextEdit->logMessage(LogReporting::ErrorLevel::error,"Gallery database not found");
  }
}

void MainWindow::insertDatabaseCoREMOF()
{
  std::shared_ptr<DocumentData> database = readDatabaseCoREMOFWorker.result();

  if(database)
  {
    ui->projectTreeView->insertDatabaseCoReMOFData(database);
  }
  else
  {
    ui->logPlainTextEdit->logMessage(LogReporting::ErrorLevel::error,"CoREMOF database not found");
  }
}

void MainWindow::insertDatabaseCoREMOFDDEC()
{
  std::shared_ptr<DocumentData> database = readDatabaseCoREMOFDDECWorker.result();

  if(database)
  {
    ui->projectTreeView->insertDatabaseCoReMOFDDECData(database);
  }
  else
  {
    ui->logPlainTextEdit->logMessage(LogReporting::ErrorLevel::error,"CoREMOF-DDEC database not found");
  }
}

void MainWindow::insertDatabaseIZA()
{
  std::shared_ptr<DocumentData> database = readDatabaseIZAWorker.result();

  if(database)
  {
    ui->projectTreeView->insertDatabaseIZAFData(database);
  }
  else
  {
    ui->logPlainTextEdit->logMessage(LogReporting::ErrorLevel::error,"IZA database not found");
  }
}

void MainWindow::recheckRemovalButtons()
{
  ui->addProjectToolButton->setEnabled(true);
  if(ui->projectTreeView->model()->rowCount(QModelIndex())>0)
  {
    ui->removeProjectToolButton->setEnabled(true);
  }

  ui->removeMovieToolButton->setDisabled(true);
  if(ui->sceneTreeView->model()->rowCount(QModelIndex())>0)
  {
    ui->removeMovieToolButton->setEnabled(true);
  }

  ui->addMovieToolButton->setDisabled(false);


  ui->addFrameToolButton->setEnabled(true);
  if(ui->frameListView->model()->rowCount(QModelIndex())>0)
  {
    ui->removeFrameToolButton->setEnabled(true);
  }

  ui->addElementToolButton->setDisabled(false);
  ui->removeElementToolButton->setDisabled(false);

  ui->addAtomToolButton->setDisabled(false);

  ui->removeAtomToolButton->setDisabled(true);
  if(ui->atomTreeView->model()->rowCount(QModelIndex())>0)
  {
    ui->removeAtomToolButton->setEnabled(true);
  }

  ui->atomFilterLineEdit->setDisabled(false);
  ui->atomChargeLineEdit->setDisabled(false);
}

void MainWindow::updateMenuToProjectTab()
{
  _newWorkSpaceAction->setEnabled(true);
  _newStructureProjectAction->setEnabled(true);
  _newProjectGroupAction->setEnabled(true);
  _newCrystalAction->setDisabled(true);
  _newMolecularCrystalAction->setDisabled(true);
  _newProteinCrystalAction->setDisabled(true);
  _newMoleculeAction->setDisabled(true);
  _newProteinAction->setDisabled(true);
  _newCrystalEllipsoidAction->setDisabled(true);
  _newCrystalCylinderAction->setDisabled(true);
  _newCrystalPolygonalPrismAction->setDisabled(true);
  _newEllipsoidAction->setDisabled(true);
  _newCylinderAction->setDisabled(true);
  _newPolygonalPrismAction->setDisabled(true);
}

void MainWindow::updateMenuToSceneTab()
{
  _newWorkSpaceAction->setEnabled(true);
  _newStructureProjectAction->setDisabled(true);
  _newProjectGroupAction->setDisabled(true);
  _newCrystalAction->setEnabled(true);
  _newMolecularCrystalAction->setEnabled(true);
  _newProteinCrystalAction->setEnabled(true);
  _newMoleculeAction->setEnabled(true);
  _newProteinAction->setEnabled(true);
  _newCrystalEllipsoidAction->setEnabled(true);
  _newCrystalCylinderAction->setEnabled(true);
  _newCrystalPolygonalPrismAction->setEnabled(true);
  _newEllipsoidAction->setEnabled(true);
  _newCylinderAction->setEnabled(true);
  _newPolygonalPrismAction->setEnabled(true);
}

void MainWindow::updateMenuToFrameTab()
{
  _newWorkSpaceAction->setEnabled(true);
  _newStructureProjectAction->setDisabled(true);
  _newProjectGroupAction->setDisabled(true);
  _newCrystalAction->setDisabled(true);
  _newMolecularCrystalAction->setDisabled(true);
  _newProteinCrystalAction->setDisabled(true);
  _newMoleculeAction->setDisabled(true);
  _newProteinAction->setDisabled(true);
  _newCrystalEllipsoidAction->setDisabled(true);
  _newCrystalCylinderAction->setDisabled(true);
  _newCrystalPolygonalPrismAction->setDisabled(true);
  _newEllipsoidAction->setDisabled(true);
  _newCylinderAction->setDisabled(true);
  _newPolygonalPrismAction->setDisabled(true);
}

void MainWindow::resetData()
{  
  ui->stackedRenderers->resetData();

  ui->cameraTreeWidget->resetData();
  ui->infoTreeWidget->resetData();
  ui->appearanceTreeWidget->resetData();
  ui->cellTreeWidget->resetData();
  ui->atomTreeView->resetData();
  ui->bondListView->resetData();
}

void MainWindow::reloadAllViews()
{
  ui->stackedRenderers->reloadData();
  ui->atomTreeView->reloadData();
  ui->bondListView->reloadData();
}

void MainWindow::reloadDetailViews()
{
  ui->stackedRenderers->reloadData();
}

void MainWindow::reloadSelectionDetailViews()
{
  ui->atomTreeView->reloadSelection();
  ui->bondListView->reloadSelection();
  ui->stackedRenderers->reloadData();
}

void MainWindow::createMenus()
{
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  QAction *actionFileAbout = new QAction(tr("&About iRASPA"), this);
  QObject::connect(actionFileAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
  fileMenu->addAction(actionFileAbout);

  _newMenu = fileMenu->addMenu(tr("&New"));
  _newStructureProjectAction = new QAction(tr("&Structure project"), this);
  QObject::connect(_newStructureProjectAction, &QAction::triggered, this, &MainWindow::newStructureProject);
  _newMenu->addAction(_newStructureProjectAction);
  _newProjectGroupAction = new QAction(tr("&Project group"), this);
  QObject::connect(_newProjectGroupAction, &QAction::triggered, this, &MainWindow::newProjectGroup);
  _newMenu->addAction(_newProjectGroupAction);
  _newMenu->addSeparator();

  _newCrystalAction = new QAction(tr("&Crystal"), this);
  QObject::connect(_newCrystalAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newCrystal);
  _newMenu->addAction(_newCrystalAction);
  _newMolecularCrystalAction = new QAction(tr("&Molecular crystal"), this);
  QObject::connect(_newMolecularCrystalAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newMolecularCrystal);
  _newMenu->addAction(_newMolecularCrystalAction);
  _newProteinCrystalAction = new QAction(tr("&Protein crystal"), this);
  QObject::connect(_newProteinCrystalAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newProteinCrystal);
  _newMenu->addAction(_newProteinCrystalAction);
  _newMoleculeAction = new QAction(tr("&Molecule"), this);
  QObject::connect(_newMoleculeAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newMolecule);
  _newMenu->addAction(_newMoleculeAction);
  _newProteinAction = new QAction(tr("&Protein"), this);
  QObject::connect(_newProteinAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newProtein);
  _newMenu->addAction(_newProteinAction);

  _newObjectsMenu = _newMenu->addMenu(tr("&Objects"));
  _newCrystalEllipsoidAction = new QAction(tr("&Crystal ellipsoid"), this);
  QObject::connect(_newCrystalEllipsoidAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newCrystalEllipsoid);
  _newObjectsMenu->addAction(_newCrystalEllipsoidAction);
  _newCrystalCylinderAction = new QAction(tr("&Crystal cylinder"), this);
  QObject::connect(_newCrystalCylinderAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newCrystalCylinder);
  _newObjectsMenu->addAction(_newCrystalCylinderAction);
  _newCrystalPolygonalPrismAction = new QAction(tr("&Crystal polygonal prism"), this);
  QObject::connect(_newCrystalPolygonalPrismAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newCrystalPolygonalPrism);
  _newObjectsMenu->addAction(_newCrystalPolygonalPrismAction);
  _newEllipsoidAction = new QAction(tr("&Ellipsoid"), this);
  QObject::connect(_newEllipsoidAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newEllipsoid);
  _newObjectsMenu->addAction(_newEllipsoidAction);
  _newCylinderAction = new QAction(tr("&Cylinder"), this);
  QObject::connect(_newCylinderAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newCylinder);
  _newObjectsMenu->addAction(_newCylinderAction);
  _newPolygonalPrismAction = new QAction(tr("&Polygonal prism"), this);
  QObject::connect(_newPolygonalPrismAction, &QAction::triggered, ui->sceneTreeView, &SceneTreeView::newPolygonalPrism);
  _newObjectsMenu->addAction(_newPolygonalPrismAction);

  _newMenu->addSeparator();
  _newWorkSpaceAction = new QAction(tr("&Workspace"), this);
  QObject::connect(_newWorkSpaceAction, &QAction::triggered, this, &MainWindow::newWorkSpace);
  _newMenu->addAction(_newWorkSpaceAction);

  QAction *actionFileImport = new QAction(tr("&Import"), this);
  QObject::connect(actionFileImport, &QAction::triggered, this, &MainWindow::importFile);
  fileMenu->addAction(actionFileImport);
  QAction *actionFileOpen = new QAction(tr("&Open"), this);
  actionFileOpen->setShortcuts(QKeySequence::Open);
  actionFileOpen->setStatusTip(tr("Open an existing document"));
  QObject::connect(actionFileOpen, &QAction::triggered, this, &MainWindow::open);
  fileMenu->addAction(actionFileOpen);
  QAction *actionFileSave = new QAction(tr("&Save"), this);
  actionFileSave->setShortcuts(QKeySequence::Save);
  actionFileSave->setStatusTip(tr("Save the document to disk"));
  QObject::connect(actionFileSave, &QAction::triggered, this, &MainWindow::save);
  fileMenu->addAction(actionFileSave);
  QAction *actionFileSaveAs = new QAction(tr("Save &As"), this);
  actionFileSaveAs->setShortcuts(QKeySequence::SaveAs);
  actionFileSaveAs->setStatusTip(tr("Save the document to disk"));
  QObject::connect(actionFileSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
  fileMenu->addAction(actionFileSaveAs);

  _undoAction = ui->projectTreeView->undoManager().createUndoAction(this, tr("&Undo"));
  _undoAction->setShortcuts(QKeySequence::Undo);

  _redoAction = ui->projectTreeView->undoManager().createRedoAction(this, tr("&Redo"));
  _redoAction->setShortcuts(QKeySequence::Redo);

  _copyAction = new QAction(tr("&Copy"), this);
  _copyAction->setObjectName(QStringLiteral("actionCopy"));

  _pasteAction = new QAction(tr("&Paste"), this);
  _pasteAction->setObjectName(QStringLiteral("actionPaste"));

  _cutAction = new QAction(tr("&Cut"), this);
  _cutAction->setObjectName(QStringLiteral("actionCut"));


  QObject::connect(_copyAction, &QAction::triggered, this, &MainWindow::actionTriggered);
  QObject::connect(_pasteAction, &QAction::triggered, this, &MainWindow::actionTriggered);
  QObject::connect(_cutAction, &QAction::triggered, this, &MainWindow::actionTriggered);

  _editMenu = menuBar()->addMenu(tr("&Edit"));
  _editMenu->addAction(_undoAction);
  _editMenu->addAction(_redoAction);
  _editMenu->addSeparator();
  _editMenu->addAction(_copyAction);
  _editMenu->addAction(_pasteAction);
  _editMenu->addAction(_cutAction);

  // copy/paste implementation from: https://srivatsp.com/ostinato/qt-cut-copy-paste/
  QObject::connect(qApp, &QApplication::focusChanged, this, &MainWindow::focusChanged);

  QAction *actionHelpContents = new QAction(tr("&Contents"), this);
  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(actionHelpContents);

  QObject::connect(actionHelpContents, &QAction::triggered, [this](bool){
     HelpWidget* helpWindow = new HelpWidget(this);
     helpWindow->resize(1200,800);
     helpWindow->show();
  });

  ui->addMovieToolButton->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(ui->addMovieToolButton, &QToolButton::customContextMenuRequested, this, &MainWindow::ShowContextAddStructureMenu);

  ui->addProjectToolButton->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(ui->addProjectToolButton, &QToolButton::customContextMenuRequested, [this](const QPoint &p){
       QPoint q = ui->addProjectToolButton->mapToGlobal(p);
       ui->projectTreeView->ShowToolButtonContextMenu(q);
     });
}

void MainWindow::ShowContextAddStructureMenu(const QPoint &pos)
{
  QMenu contextMenu(tr("Context menu"), this);

  contextMenu.addAction(_newCrystalAction);
  contextMenu.addAction(_newMolecularCrystalAction);
  contextMenu.addAction(_newProteinCrystalAction);
  contextMenu.addAction(_newMoleculeAction);
  contextMenu.addAction(_newProteinAction);
  QMenu *menu = new QMenu("Objects", this);
  contextMenu.addMenu(menu);
  menu->addAction(_newCrystalEllipsoidAction);
  menu->addAction(_newCrystalCylinderAction);
  menu->addAction(_newCrystalPolygonalPrismAction);
  menu->addAction(_newEllipsoidAction);
  menu->addAction(_newCylinderAction);
  menu->addAction(_newPolygonalPrismAction);

  QPoint globalPos = ui->addMovieToolButton->mapToGlobal(pos);
  contextMenu.exec(globalPos);
}

void MainWindow::setUndoAction(QAction *newUndoAction)
{
  _editMenu->insertAction(_undoAction, newUndoAction);
  _editMenu->removeAction(_undoAction);
  _undoAction = newUndoAction;
  _undoAction->setShortcuts(QKeySequence::Undo);
}

void MainWindow::setRedoAction(QAction *newRedoAction)
{
  _editMenu->insertAction(_redoAction, newRedoAction);
  _editMenu->removeAction(_redoAction);
  _redoAction = newRedoAction;
  _redoAction->setShortcuts(QKeySequence::Redo);
}

void MainWindow::propagateProject(std::shared_ptr<ProjectTreeNode> project, QObject *widget)
{
  if(project)
  {
    ui->removeProjectToolButton->setEnabled(project->isEditable());

    ui->addMovieToolButton->setEnabled(project->isEditable());
    ui->removeMovieToolButton->setEnabled(project->isEditable());

    ui->addFrameToolButton->setEnabled(project->isEditable());
    ui->removeFrameToolButton->setEnabled(project->isEditable());

    ui->addElementToolButton->setEnabled(project->isEditable());
    ui->removeElementToolButton->setEnabled(project->isEditable());

    ui->addAtomToolButton->setEnabled(project->isEditable());
    ui->removeAtomToolButton->setEnabled(project->isEditable());
    ui->atomFilterLineEdit->setEnabled(project->isEditable());
    ui->atomChargeLineEdit->setEnabled(project->isEditable());
  }
  else
  {
    ui->removeProjectToolButton->setDisabled(true);

    ui->addMovieToolButton->setDisabled(true);
    ui->removeMovieToolButton->setDisabled(true);

    ui->addFrameToolButton->setDisabled(true);
    ui->removeFrameToolButton->setDisabled(true);

    ui->addElementToolButton->setDisabled(true);
    ui->removeElementToolButton->setDisabled(true);

    ui->addAtomToolButton->setDisabled(true);
    ui->removeAtomToolButton->setDisabled(true);
    ui->atomFilterLineEdit->setDisabled(true);
    ui->atomChargeLineEdit->setDisabled(true);
  }

  if (ProjectConsumer *projectConsumer = dynamic_cast<ProjectConsumer*>(widget))
  {
    projectConsumer->setProject(project);
  }

  // set the project for a RKRenderViewController
  if (RKRenderViewController *renderViewController = dynamic_cast<RKRenderViewController*>(widget))
  {
    renderViewController->setRenderDataSource(nullptr);
    if (project)
    {
      if(std::shared_ptr<iRASPAProject> iraspaProject = project->representedObject())
      {
        if(std::shared_ptr<Project> project = iraspaProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            renderViewController->setRenderDataSource(projectStructure);
          }
        }
      }
    }
  }

  foreach(QObject *child, widget->children())
  {
    this->propagateProject(project, child);
  }

}

void MainWindow::propagateMainWindow(MainWindow* mainWindow, QObject *widget)
{
  if (MainWindowConsumer *mainWindowConsumer = dynamic_cast<MainWindowConsumer*>(widget))
  {
    mainWindowConsumer->setMainWindow(mainWindow);
  }

  foreach(QObject *child, widget->children())
  {
    this->propagateMainWindow(mainWindow, child);
  }
}

void MainWindow::propagateLogReporter(LogReporting* logReporting, QObject *widget)
{
  if (LogReportingConsumer *logReportingConsumer = dynamic_cast<LogReportingConsumer*>(widget))
  {
    logReportingConsumer->setLogReportingWidget(logReporting);
  }

  foreach(QObject *child, widget->children())
  {
    this->propagateLogReporter(logReporting, child);
  }
}



// snoop global keyboard events
bool MainWindow::eventFilter( QObject* object, QEvent *event )
{
  if(GLWidget* widget = dynamic_cast<GLWidget*>(ui->stackedRenderers->currentWidget()))
  {
    if(object == this || object == widget)
    {
      if(QKeyEvent * keyEvent = dynamic_cast<QKeyEvent*>(event))
      {
        if (event->type() == QEvent::KeyPress)
        {
          if (keyEvent->modifiers().testFlag(Qt::AltModifier))
          {
            widget->setControlPanel(true);
          }
        }
        if (event->type() == QEvent::KeyRelease)
        {
          if (keyEvent->key() == Qt::Key_Alt)
          {
            widget->setControlPanel(false);
          }
        }
      }
    }
  }
  return QMainWindow::eventFilter(object, event);
}

void MainWindow::updateControlPanel()
{
  if(GLWidget* widget = dynamic_cast<GLWidget*>(ui->stackedRenderers->currentWidget()))
  {
    widget->updateControlPanel();
  }
}

void MainWindow::showAboutDialog()
{
  AboutDialog* about = new AboutDialog(this);
  QObject::connect(about->acknowledgmentsPushButton, &QPushButton::clicked, this, &MainWindow::acknowledgements);

  about->acknowledgmentsPushButton->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa);");

  about->exec();
}

void MainWindow::importFile()
{
  ImportFileDialog dialog(this);

  if(dialog.exec() == QDialog::Accepted)
  {
    QList<QUrl> fileURLs = dialog.selectedUrls();
    if(fileURLs.isEmpty())
    {
      return;
    }
    else
    {
      bool asSeparateProject=dialog.checkboxSeperateProjects->checkState() == Qt::CheckState::Checked;
      bool onlyAsymmetricUnit=dialog.checkboxOnlyAsymmetricUnitCell->checkState() == Qt::CheckState::Checked;
      bool asMolecule=dialog.checkboxImportAsMolecule->checkState() == Qt::CheckState::Checked;

      ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(ui->projectTreeView->model());
      std::shared_ptr<ProjectTreeNode> localProjects = _documentData->projectTreeController()->localProjects();
      QModelIndex insertionParentIndex = pModel->indexForNode(localProjects.get());
      int insertRow = 0;

      QModelIndex index = ui->projectTreeView->selectionModel()->currentIndex();
      if(index.isValid())
      {
        if(ProjectTreeNode* node = pModel->nodeForIndex(index))
        {
          if (node->isDescendantOfNode(localProjects.get()))
          {
            // a project is selected
              qDebug() << "YEAH";
            insertionParentIndex = index.parent();
            insertRow = index.row() + 1;
          }
        }
      }

      qDebug() << "row: " << insertRow;

      if(asSeparateProject)
      {
        for(QUrl url : fileURLs)
        {
          QString fileName = url.toString();

          QFileInfo fileInfo(fileName);


          ui->logPlainTextEdit->logMessage(LogReporting::ErrorLevel::info,"start reading CIF-file: " + fileInfo.baseName());

          std::shared_ptr<ProjectStructure> project = std::make_shared<ProjectStructure>(QList{url}, _documentData->colorSets(), _documentData->forceFieldSets(), ui->logPlainTextEdit, asSeparateProject, onlyAsymmetricUnit, asMolecule);
          std::shared_ptr<iRASPAProject>  iraspaproject = std::make_shared<iRASPAProject>(project);
          std::shared_ptr<ProjectTreeNode> newProject = std::make_shared<ProjectTreeNode>(fileInfo.baseName(), iraspaproject, true, true);

          ui->projectTreeView->insertRows(insertRow, 1, insertionParentIndex, newProject);
          insertRow++;
        }
      }
      else
      {
        QString fileName = fileURLs.first().toString();

        QFileInfo fileInfo(fileName);

        ui->logPlainTextEdit->logMessage(LogReporting::ErrorLevel::info,"start reading CIF-file: " + fileInfo.baseName());

        std::shared_ptr<ProjectStructure> project = std::make_shared<ProjectStructure>(fileURLs, _documentData->colorSets(), _documentData->forceFieldSets(), ui->logPlainTextEdit, asSeparateProject, onlyAsymmetricUnit, asMolecule);
        std::shared_ptr<iRASPAProject>  iraspaproject = std::make_shared<iRASPAProject>(project);
        std::shared_ptr<ProjectTreeNode> newProject = std::make_shared<ProjectTreeNode>(fileInfo.baseName(), iraspaproject, true, true);

        ui->projectTreeView->insertRows(insertRow, 1, insertionParentIndex, newProject);
      }
    }
  }
}


int MainWindow::slideRightPanel(void)
{
  if(ui->structureMainHorizontalSplitter->widget(2)->isHidden())
    ui->structureMainHorizontalSplitter->widget(2)->show();
  else
    ui->structureMainHorizontalSplitter->widget(2)->hide();
  return 0;
}

int MainWindow::slideDownPanel(void)
{
  if(ui->structureDetailVerticalSplitter->widget(1)->isHidden())
    ui->structureDetailVerticalSplitter->widget(1)->show();
  else
    ui->structureDetailVerticalSplitter->widget(1)->hide();

  return 0;
}

int MainWindow::slideLeftPanel(void)
{
  if(ui->structureMainHorizontalSplitter->widget(0)->isHidden())
    ui->structureMainHorizontalSplitter->widget(0)->show();
  else
    ui->structureMainHorizontalSplitter->widget(0)->hide();

  return 0;
}

void MainWindow::acknowledgements()
{
#ifdef Q_OS_LINUX
  #ifdef USE_SNAP
    QDir dir("/snap/iraspa/current/usr/share/iraspa/");
  #else
    QDir dir("/usr/share/iraspa/");
  #endif
  QUrl url1(dir.absoluteFilePath("acknowledgedlicenses.pdf"));
  QUrl url2(dir.absoluteFilePath("license-gpl.pdf"));
#else
  QDir dir(QApplication::applicationDirPath());
  QUrl url1(dir.absoluteFilePath("acknowledgedlicenses.pdf"));
  QUrl url2(dir.absoluteFilePath("license-gpl.pdf"));
#endif
  url1.setScheme("file");
  QDesktopServices::openUrl(url1);
  url2.setScheme("file");
  QDesktopServices::openUrl(url2);

}

QDir MainWindow::directoryOf(const QString &subdir)
{
  QDir dir(QApplication::applicationDirPath());
#if defined(Q_OS_WIN)
  if (dir.dirName().toLower() == "debug" || dir.dirName().toLower() == "release")
    dir.cdUp();
#elif defined(Q_OS_MAC)
  if (dir.dirName() == "MacOS")
  {
    dir.cdUp();
    dir.cd("Resources");
  }
#endif
  dir.cd(subdir);
  return dir;
}


void MainWindow::newWorkSpace()
{
  MainWindow *mainWindow = new MainWindow();
  mainWindow->setAttribute(Qt::WA_DeleteOnClose);
  mainWindow->show();
}

void MainWindow::newStructureProject()
{

}

void MainWindow::newProjectGroup()
{

}



// see https://srivatsp.com/ostinato/qt-cut-copy-paste/
void MainWindow::actionTriggered()
{
  QWidget *focusWidget = qApp->focusWidget();

  if  (!focusWidget)
    return;

  // single slot to handle cut/copy/paste - find which action was triggered
  QString action = sender()->objectName().remove("action").append("()").toLower();
  if (focusWidget->metaObject()->indexOfSlot(qPrintable(action)) < 0)
  {
    // slot not found in focus widget corresponding to action
    return;
  }

  action.remove("()");
  QMetaObject::invokeMethod(focusWidget, qPrintable(action), Qt::DirectConnection);
}

void MainWindow::focusChanged(QWidget *old, QWidget *now)
{
  // after leaving a widget, the widget needs to be redrawn to grey-out the selection
  if(old)
  {
    old->update();
  }

  // after entering a widget, the widget needs to be redrawn to highlight the selection
  if(now)
  {
    now->update();
  }

  updatePasteStatus();

  const iRASPATreeView *view = dynamic_cast<iRASPATreeView*>(old);
  if(view)
  {
    disconnect(view->selectionModel(),  SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
               this, SLOT(focusWidgetSelectionChanged(const QItemSelection&, const QItemSelection&)));
  }

  if(!now)
  {
    // No focus widget to copy from
    _cutAction->setEnabled(false);
    _copyAction->setEnabled(false);
    return;
  }

  const QMetaObject *meta = now->metaObject();
  if (meta->indexOfSlot("copy()") < 0)
  {
    // Focus Widget doesn't have a copy slot
    _cutAction->setEnabled(false);
    _copyAction->setEnabled(false);
    return;
  }

  view = dynamic_cast<iRASPATreeView*>(now);
  if (view)
  {
    connect(view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(focusWidgetSelectionChanged(const QItemSelection&, const QItemSelection&)));
    if (!view->hasSelection())
    {
      // view doesn't have anything selected to copy
      _cutAction->setEnabled(false);
      _copyAction->setEnabled(false);
      return;
    }
    _cutAction->setEnabled(view->canCut());
  }

  // focus widget has a selection and copy slot: copy possible
  _copyAction->setEnabled(true);
}

void MainWindow::focusWidgetSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);

  // Selection changed in the iRASPATreeView that has focus
  if(const iRASPATreeView *view = dynamic_cast<iRASPATreeView*>(qApp->focusWidget()))
  {
    _copyAction->setEnabled(view->hasSelection());
    _cutAction->setEnabled(view->hasSelection() && view && view->canCut());
  }
}

void MainWindow::updatePasteStatus()
{
  QWidget *focusWidget = qApp->focusWidget();

  if (!focusWidget)
  {
    // No focus widget to paste into
    _pasteAction->setEnabled(false);
    return;
  }

  const QMimeData *item = QGuiApplication::clipboard()->mimeData();
  if  (!item || item->formats().isEmpty())
  {
    // Nothing on clipboard to paste
    _pasteAction->setEnabled(false);
    return;
  }

  const QMetaObject *meta = focusWidget->metaObject();
  if (meta->indexOfSlot("paste()") < 0)
  {
    // Focus Widget doesn't have a paste slot
    _pasteAction->setEnabled(false);
    return;
  }

  const iRASPATreeView *view = dynamic_cast<iRASPATreeView*>(focusWidget);
  if (view && !view->canPaste(item))
  {
    // Focus widget view cannot accept this item
    _pasteAction->setEnabled(false);
    return;
  }

  // Focus widget can accept this item: paste possible",
  _pasteAction->setEnabled(true);
}


// https://code.qt.io/cgit/qt/qtbase.git/tree/examples/widgets/mainwindows/application/mainwindow.cpp?h=5.15

void MainWindow::documentWasModified()
{
  _isEdited = true;
  setWindowModified(_isEdited);
}

// The function returns true if the user wants the application to close; otherwise, it returns false
bool MainWindow::maybeSave()
{
    if (!_isEdited)
        return true;
    const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret)
    {
    case QMessageBox::Save:
      return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::open()
{
  if (maybeSave())
  {
    OpeniRASPAFormatDialog dialog(nullptr);

    if(dialog.exec() == QDialog::Accepted)
    {
      QList<QUrl> fileURLs = dialog.selectedUrls();
      if(fileURLs.isEmpty())
      {
        return;
      }
      else
      {
        openFile(fileURLs.first().toLocalFile());
      }
    }
  }
}

bool MainWindow::save()
{
  if (currentDocumentFileName.isEmpty())
  {
    return saveAs();
  }
  else
  {
   return saveFile(currentDocumentFileName);
  }
}

bool MainWindow::saveAs()
{
  SaveiRASPAFormatDialog dialog(nullptr);

  if(dialog.exec() == QDialog::Accepted)
  {
    QList<QUrl> fileURLs = dialog.selectedUrls();
    if(fileURLs.isEmpty())
    {
      return false;
    }
    else
    {
      return saveFile(fileURLs.first().toLocalFile());
    }
  }
  return false;
}

void MainWindow::openFile(const QString &fileName)
{
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    std::shared_ptr<DocumentData> documentData = std::make_shared<DocumentData>();

    qDebug() << "Reading zip-file";

      ZipReader reader = ZipReader(fileName, QIODevice::ReadOnly);

      QByteArray colorData = reader.fileData("nl.darkwing.iRASPA_colorData");
      QDataStream colorStream(&colorData, QIODevice::ReadOnly);

      try
      {
        qDebug() << "start reading color data: " << colorData.size();
        colorStream >> documentData->colorSets();
      }
      catch(std::exception e)
      {
        std::cout << "Error: " << e.what() << std::endl;
      }

      qDebug() << "done reading color data" << colorData.size();

      QByteArray forceFieldData = reader.fileData("nl.darkwing.iRASPA_forceFieldData");
      QDataStream forceFieldStream(&forceFieldData, QIODevice::ReadOnly);

      try
      {
        qDebug() << "start reading force field data" << forceFieldData.size();
        forceFieldStream >> documentData->forceFieldSets();
      }
      catch(std::exception e)
      {
        std::cout << "Error: " << e.what() << std::endl;
      }
      qDebug() << "done reading force field data: " << forceFieldData.size();

      QByteArray data = reader.fileData("nl.darkwing.iRASPA_projectData");
      QDataStream stream(&data, QIODevice::ReadOnly);

      try
      {
        qDebug() << "start reading document data: " << data.size();
        stream >> documentData;
      }
      catch (InvalidArchiveVersionException ex)
      {
        std::cout << "Error: " << ex.message().toStdString() << std::endl;
        std::cout << ex.what() << ex.get_file() << std::endl;
        std::cout << "Function: " << ex.get_func() << std::endl;
      }
      catch(InconsistentArchiveException ex)
      {
        std::cout << "Error: " << ex.message().toStdString() << std::endl;
        std::cout << ex.what() << ex.get_file() << std::endl;
        std::cout << "Function: " << ex.get_func() << std::endl;
      }
      catch(std::exception e)
      {
        std::cout << "Error: " << e.what() << std::endl;
      }

      documentData->projectTreeController()->localProjects()->setIsDropEnabled(true);
      for(std::shared_ptr<ProjectTreeNode> localNode : documentData->projectTreeController()->localProjects()->descendantNodes())
      {
        localNode->representedObject()->readData(reader);
      }
      reader.close();

      ui->projectTreeView->setDocumentData(documentData);

      /*
      QModelIndex index1 = ui->projectTreeView->model()->index(0,0,QModelIndex());
      QModelIndex index2 = ui->projectTreeView->model()->index(2,0,QModelIndex());
      QModelIndex index3 = ui->projectTreeView->model()->index(4,0,QModelIndex());
      ui->projectTreeView->expand(index1);
      ui->projectTreeView->expand(index2);
      ui->projectTreeView->expand(index3);

      QModelIndex index4 = ui->projectTreeView->model()->index(0,0,index2);
      ui->projectTreeView->expand(index4);
*/
      std::cout << "End openFile" <<  std::endl;

#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif
  setCurrentFile(fileName);
}


bool MainWindow::saveFile(const QString &fileName)
{
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif

  QFileInfo info(fileName);
  QString strNewName = info.path() + "/" + info.completeBaseName() + ".irspdoc";

  ZipWriter zipwriter = ZipWriter(strNewName, QIODevice::WriteOnly|QIODevice::Truncate);
  zipwriter.setCompressionPolicy(ZipWriter::CompressionPolicy::NeverCompress);

  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);

  stream << _documentData;
  zipwriter.addFile("nl.darkwing.iRASPA_projectData", byteArray);

  std::vector<std::shared_ptr<ProjectTreeNode>> nodes = _documentData->projectTreeController()->localProjects()->descendantNodes();

  for(std::shared_ptr<ProjectTreeNode> &node : nodes)
  {
    node->representedObject()->saveData(zipwriter);
  }

  QByteArray colorByteArray = QByteArray();
  QDataStream colorStream(&colorByteArray, QIODevice::WriteOnly);
  colorStream << _documentData->colorSets();
  zipwriter.addFile(QString("nl.darkwing.iRASPA_colorData"), colorByteArray);

  QByteArray forceFieldSetsByteArray = QByteArray();
  QDataStream forceFieldSetsStream(&forceFieldSetsByteArray, QIODevice::WriteOnly);
  forceFieldSetsStream << _documentData->forceFieldSets();
  zipwriter.addFile(QString("nl.darkwing.iRASPA_forceFieldData"), forceFieldSetsByteArray);

  zipwriter.close();

#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

  setCurrentFile(fileName);
  return true;
}



void MainWindow::setCurrentFile(const QString &fileName)
{
  currentDocumentFileName = fileName;
  _isEdited = false;
  setWindowModified(false);

  QString shownName = currentDocumentFileName;
  if (currentDocumentFileName.isEmpty())
    shownName = "untitled.txt";
  setWindowFilePath(shownName);
}




void MainWindow::closeEvent(QCloseEvent *event)  // show prompt when user wants to close app
{
  if (maybeSave())
  {
    //writeSettings();
    event->accept();
  }
  else
  {
    event->ignore();
  }
}
