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

#include <QMainWindow>
#include <QObject>
#include <QDockWidget>
#include <QMenu>
#include <QDir>
#include <QAction>
#include <QItemSelection>
#include <QFuture>
#include <QCloseEvent>
#include <QMessageBox>
#include <QTimer>
#include <foundationkit.h>
#include <iraspakit.h>
#include <symmetrykit.h>
#include "helpwidget.h"
//#include "atomtreeviewmodel.h"
//#include "bondlistviewmodel.h"

namespace Ui {
class MainWindow;
}

struct ProjectConsumer;

class MainWindow : public QMainWindow  //, public ProjectConsumer
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void setDocumentData(std::shared_ptr<DocumentData> documentData) {_documentData = documentData;}
  bool eventFilter(QObject*object, QEvent *event ) override final;
  void propagateProject(std::shared_ptr<ProjectTreeNode> project, QObject *widget);
  void propagateMainWindow(MainWindow* mainWindow, QObject *widget);
  void propagateLogReporter(LogReporting* logReporting, QObject *widget);
  std::shared_ptr<ProjectTreeController> projectTreeController() {return _documentData->projectTreeController();}
  Ui::MainWindow* mainWindowUI() {return ui;}
  SKColorSets &colorSets() {return _documentData->colorSets();}
  ForceFieldSets &forceFieldSets() {return _documentData->forceFieldSets();}
  void setUndoAction(QAction *newUndoAction);
  void setRedoAction(QAction *newRedoAction);
  void reloadDetailViews();
  void reloadAllViews();
  void resetData();
  void reloadSelectionDetailViews();
  void recheckRemovalButtons();
  void updateMenuToProjectTab();
  void updateMenuToSceneTab();
  void updateMenuToFrameTab();
  void updateControlPanel();
  void closeEvent(QCloseEvent *event)  override final;
  void documentWasModified();
private:
  Ui::MainWindow *ui;
  std::shared_ptr<DocumentData> _documentData;

  int projectInsertionIndex();

  QString currentDocumentFileName{};
  bool _isEdited = false;
  bool maybeSave();
  void openFile(const QString &fileName);
  bool saveFile(const QString &fileName);
  void setCurrentFile(const QString &fileName);

  void resizeEvent(QResizeEvent* pEvent) override final;
  QTimer *_timer;

  QAction *_helpAction;
  void acknowledgements();
  QDir directoryOf(const QString &subdir);
  void createMenus();

  std::shared_ptr<DocumentData> readDatabase(QString name);
  QFuture<std::shared_ptr<DocumentData>> readDatabaseGalleryWorker;
  void insertDatabaseGallery();
  QFuture<std::shared_ptr<DocumentData>> readDatabaseCoREMOFWorker;
  void insertDatabaseCoREMOF();
  QFuture<std::shared_ptr<DocumentData>> readDatabaseCoREMOFDDECWorker;
  void insertDatabaseCoREMOFDDEC();
  QFuture<std::shared_ptr<DocumentData>> readDatabaseIZAWorker;
  void insertDatabaseIZA();

  void reportError(QString error);
  void reportWarning(QString warning);

  QMenu *_newMenu{nullptr};
  QAction *_newWorkSpaceAction{nullptr};
  QAction *_newStructureProjectAction{nullptr};
  QAction *_newProjectGroupAction{nullptr};
  QAction *_newCrystalAction{nullptr};
  QAction *_newMolecularCrystalAction{nullptr};
  QAction *_newProteinCrystalAction{nullptr};
  QAction *_newMoleculeAction{nullptr};
  QAction *_newProteinAction{nullptr};
  QMenu *_newObjectsMenu{nullptr};
  QAction *_newCrystalEllipsoidAction{nullptr};
  QAction *_newCrystalCylinderAction{nullptr};
  QAction *_newCrystalPolygonalPrismAction{nullptr};
  QAction *_newEllipsoidAction{nullptr};
  QAction *_newCylinderAction{nullptr};
  QAction *_newPolygonalPrismAction{nullptr};

  QMenu *_editMenu{nullptr};
  QAction *_undoAction{nullptr};
  QAction *_redoAction{nullptr};

  QAction *_cutAction{nullptr};
  QAction *_copyAction{nullptr};
  QAction *_pasteAction{nullptr};

  void moveEvent(QMoveEvent *event) override;
public slots:
  void importFile();
  void open();
  bool save();
  bool saveAs();
  void showAboutDialog();
  void ShowContextAddStructureMenu(const QPoint &pos);
  void newWorkSpace();
  void newStructureProject();
  void newProjectGroup();
  int slideRightPanel(void);
  int slideDownPanel(void);
  int slideLeftPanel(void);
  void actionTriggered();
  void focusChanged(QWidget *old, QWidget *now);
  void updatePasteStatus();
  void focusWidgetSelectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/);
private slots:
  void resizeStopped();
signals:
  void rendererReloadData();
  void invalidateCachedAmbientOcclusionTextures(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
  void invalidateCachedIsoSurfaces(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
};
