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

#include <QObject>
#include <QEvent>
#include <QPoint>
#include <QUrl>
#include <QStackedWidget>
#include <QToolButton>
#include <iraspaproject.h>
#include <iraspamainwindowconsumerprotocol.h>
#include "atomtreeviewmodel.h"
#include "bondlistviewmodel.h"
#include "symmetrykit.h"
#include "logreporting.h"
#include "moviemaker.h"

class RenderStackedWidget : public QWidget, public ProjectConsumer, public MainWindowConsumer, public LogReportingConsumer
{
  Q_OBJECT

public:
  RenderStackedWidget(QWidget* parent = nullptr );
  void setProject(std::shared_ptr<ProjectTreeNode> _projectTreeNode) override final;
  void setMainWindow(MainWindow *mainWindow) override final {_mainWindow = mainWindow;}
  void setAtomModel(std::shared_ptr<AtomTreeViewModel> atomModel);
  void setBondModel(std::shared_ptr<BondListViewModel> bondModel);
  void setLogReportingWidget(LogReporting *logReporting) override final;
  void updateControlPanel();
  void setControlPanel(bool iskeyAltOn);

  void hideToolBarMenuMenu();
  void showToolBarMenuMenu();
private:
  enum class Tracking
  {
    none = 0,
    panning = 1,                   // alt + right-mouse drag
    trucking = 2,                  // ctrl + right-mouse drag
    addToSelection = 3,            // command-key
    newSelection = 4,              // shift-key
    draggedAddToSelection = 5,     // drag + command
    draggedNewSelection = 6,       // drag + shift
    backgroundClick = 7,
    measurement = 8,               // alt-key
    translateSelection = 9,        // alt and command-key
    other = 10
  };
  QWidget *renderWidget;
  QWindow *renderWindow;
  MainWindow *_mainWindow;
  QMenu *toolBar;

  RKRenderViewController *renderViewController;
  LogReporting* _logReporter = nullptr;
  std::shared_ptr<AtomTreeViewModel> _atomModel;
  std::shared_ptr<BondListViewModel> _bondModel;
  std::weak_ptr<ProjectTreeNode> _projectTreeNode;
  std::weak_ptr<ProjectStructure> _project;
  std::weak_ptr<iRASPAProject> _iRASPAProject;
  std::weak_ptr<RKCamera> _camera;
  std::vector<std::vector<std::shared_ptr<iRASPAObject>>> _iraspa_structures;
  Tracking _tracking;
  QPoint _origin;
  QPoint _panStartPoint;
  QPoint _truckStartPoint;
  void selectAsymetricAtomsInRectangle(QRect rect, bool extend);
  void exportToPDB() const;
  void exportToMMCIF() const;
  void exportToCIF() const;
  void exportToXYZ() const;
  void exportToPOSCAR() const;
  void deleteSelection();
  QStackedWidget *_controlPanel;
protected:
  bool eventFilter(QObject *obj, QEvent *event) override final;
public slots:
  void setSelectedRenderFrames(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
  void redraw();
  void redrawWithQuality(RKRenderQuality quality);
  void resetData();
  void reloadRenderData();
  void reloadData();
  void reloadBoundingBoxData();
  void reloadAmbientOcclusionData();
  void reloadSelectionData();
  void reloadBackgroundImage();
  void createPicture(QUrl fileURL, int width, int height);
  void createMovie(QUrl fileURL, int width, int height, MovieWriter::Type type);
  void resizeEvent(QResizeEvent *event) override final;
  void invalidateCachedAmbientOcclusionTextures(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
  void invalidateCachedIsoSurfaces(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
  void computeHeliumVoidFraction(std::vector<std::shared_ptr<RKRenderObject>> structures);
  void computeNitrogenSurfaceArea(std::vector<std::shared_ptr<RKRenderObject>> structures);
  void pressedTranslateCartesianMinusX();
  void pressedTranslateCartesianPlusX();
  void pressedTranslateCartesianMinusY();
  void pressedTranslateCartesianPlusY();
  void pressedTranslateCartesianMinusZ();
  void pressedTranslateCartesianPlusZ();
  void pressedRotateCartesianMinusX();
  void pressedRotateCartesianPlusX();
  void pressedRotateCartesianMinusY();
  void pressedRotateCartesianPlusY();
  void pressedRotateCartesianMinusZ();
  void pressedRotateCartesianPlusZ();
  void pressedTranslateBodyFrameMinusX();
  void pressedTranslateBodyFramePlusX();
  void pressedTranslateBodyFrameMinusY();
  void pressedTranslateBodyFramePlusY();
  void pressedTranslateBodyFrameMinusZ();
  void pressedTranslateBodyFramePlusZ();
  void pressedRotateBodyFrameMinusX();
  void pressedRotateBodyFramePlusX();
  void pressedRotateBodyFrameMinusY();
  void pressedRotateBodyFramePlusY();
  void pressedRotateBodyFrameMinusZ();
  void pressedRotateBodyFramePlusZ();
private slots:
  void ShowContextMenu(const QPoint &pos);
  void resetCameraDistance();
  void resetCameraToZ();
  void resetCameraToY();
  void resetCameraToX();
  void setCameraToOrthographic();
  void setCameraToPerspective();
  void showBoundingBox(bool checked);
  void computeAOHighQuality();
signals:
  void updateCameraMovement();
  void updateCameraModelViewMatrix();
  void updateCameraEulerAngles();
  void updateCameraResetDirection();
  void updateCameraProjection();
  void rendererWidgetResized();
  void updateAtomSelection();
  void updateBondSelection();
};
