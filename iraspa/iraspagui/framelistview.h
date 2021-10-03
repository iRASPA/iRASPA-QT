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
#include <QListView>
#include <QModelIndex>
#include <optional>
#include <iraspakit.h>
#include "iraspamainwindowconsumerprotocol.h"
#include "framelistviewmodel.h"

// model: std::shared_ptr<Movie>

class FrameListView: public QListView, public MainWindowConsumer, public ProjectConsumer, public Reloadable, public TabControlled
{
  Q_OBJECT

public:
  FrameListView(QWidget* parent = 0);
  void TabItemWasSelected() override final;
  QSize sizeHint() const override final;
  std::shared_ptr<FrameListViewModel> frameModel() {return _model;}
  void setMainWindow(MainWindow* mainWindow) final override;
  void reloadSelection() override final;
  void reloadData() override final;
  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override final;
  void setSelectedMovie(std::shared_ptr<Movie> movie);
  void addFrame();
  void deleteSelection();
  bool isMainSelectedItem(size_t row);
private:
  MainWindow* _mainWindow;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<FrameListViewModel> _model;
  std::shared_ptr<Movie> _movie;
  std::shared_ptr<SceneList> _sceneList;
  void keyPressEvent(QKeyEvent *event) override final;
  void copy();
  void paste();
  void cut();
  QPixmap selectionToPixmap();
  void startDrag(Qt::DropActions supportedActions) override final;
  void dragMoveEvent(QDragMoveEvent* event) override final;
private slots:
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override final;
signals:
  void updateRenderer();
  void rendererReloadData();
  void setSelectedFrame(std::shared_ptr<iRASPAObject> structure);
  void setSelectedRenderFrames(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
  void setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAObject>> structures);
  void invalidateCachedAmbientOcclusionTexture(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
};
