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
#include <QMainWindow>
#include <QTreeView>
#include <QModelIndex>
#include <optional>
#include <iraspakit.h>
#include "framelistviewmodel.h"
#include "scenetreeviewmodel.h"
#include "iraspamainwindowconsumerprotocol.h"

class SceneTreeView: public QTreeView, public MainWindowConsumer, public ProjectConsumer, public Reloadable, public TabControlled
{
  Q_OBJECT

public:
  SceneTreeView(QWidget* parent = nullptr);
  void TabItemWasSelected() override final;
  QSize sizeHint() const override final;
  SceneTreeViewModel* sceneTreeModel() {return _model.get();}
  void setMainWindow(MainWindow* mainWindow) final override;
  void setFrameModel(std::shared_ptr<FrameListViewModel> model);
  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override final;
  void reloadSelection() override final;
  void reloadData() override final;

  void startDrag(Qt::DropActions supportedActions) override final;
  void dragMoveEvent(QDragMoveEvent* event) override final;

  void keyPressEvent(QKeyEvent *event) override final;
  void restoreSelection(SceneListSelection selection);

  std::shared_ptr<SceneTreeViewModel> sceneModel() {return _model;}
  std::shared_ptr<SceneList> sceneList() {return _sceneList;}
private:
  MainWindow* _mainWindow;
  std::shared_ptr<SceneTreeViewModel> _model;
  std::weak_ptr<FrameListViewModel> _frameModel;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<SceneList> _sceneList;
  QString nameOfItem(const QModelIndex &current);
  QPixmap selectionToPixmap();

  void copy();
  void paste();
  void cut();
public slots:
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override final;
  void newCrystal();
  void newMolecularCrystal();
  void newProteinCrystal();
  void newMolecule();
  void newProtein();
  void newCrystalEllipsoid();
  void newCrystalCylinder();
  void newCrystalPolygonalPrism();
  void newEllipsoid();
  void newCylinder();
  void newPolygonalPrism();
  void insertMovie(std::shared_ptr<iRASPAStructure> iraspaStructure);
  void deleteSelection();
signals:
  void updateRenderer();
  void setSelectedFrame(std::shared_ptr<iRASPAStructure> structure);
  void setSelectedMovie(std::shared_ptr<Movie> movie);
  void setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAStructure>> structures);
  void setSelectedRenderFrames(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> structures);
  void rendererReloadData();
  void invalidateCachedAmbientOcclusionTexture(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> structures);
};
