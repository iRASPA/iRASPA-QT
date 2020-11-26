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
#include <QAbstractItemModel>
#include <QTreeView>
#include <QModelIndex>
#include <optional>
#include <iraspakit.h>
#include "framelistviewmodel.h"
#include "mainwindow.h"

class SceneTreeViewModel: public QAbstractItemModel
{
  Q_OBJECT

public:
  SceneTreeViewModel();

  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode);
  void setFrameModel(std::shared_ptr<FrameListViewModel> model) {_frameModel = model;}
  bool isMainSelectedItem(std::shared_ptr<Movie> movie);
  QModelIndex indexOfMainSelected() const;
  QModelIndex indexForItem(std::shared_ptr<Movie> movie);
  QModelIndex indexForItem(std::shared_ptr<Scene> scene);
  DisplayableProtocol *getItem(const QModelIndex &index) const;

  //QT
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override final;
  QModelIndex parent(const QModelIndex &index) const override final;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override final;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override final;
  QVariant data(const QModelIndex &index, int role) const override final;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
  Qt::ItemFlags flags(const QModelIndex &index) const override final;
  bool hasChildren(const QModelIndex &parent) const override final;

  bool removeRows(int positions, int count, const QModelIndex &parent = QModelIndex()) override final;
  bool insertRows(int position, int rows, const QModelIndex &parent) override final;
  bool insertRow(int position, std::shared_ptr<Scene> scene, std::shared_ptr<Movie> movie);
  bool insertRow(int position, std::shared_ptr<Scene> scene);
  bool removeRow(int position);
  bool removeRow(int position, std::shared_ptr<Scene> scene, std::shared_ptr<Movie> movie);

  QStringList mimeTypes() const override final;
  Qt::DropActions supportedDropActions() const override final;
  Qt::DropActions supportedDragActions() const override final;
  QMimeData* mimeData(const QModelIndexList &indexes) const override final;
  QMimeData* mimeDataLazy(const QModelIndexList &indexes) const;
  bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override final;
  bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override final;
  std::shared_ptr<Scene> parentForMovie(const std::shared_ptr<Movie> movie) const;
  bool pasteMimeData(const QMimeData *data, int row, int column, const QModelIndex &parent);

  void setMainWindow(MainWindow* mainWindow);
private:
  MainWindow *_mainWindow;
  std::shared_ptr<FrameListViewModel> _frameModel;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<ProjectStructure> _projectStructure;
  std::shared_ptr<SceneList> _sceneList;
signals:
  void expandAll();
  void rendererReloadData();
  void updateSelection();
  void invalidateCachedAmbientOcclusionTexture(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> structures);
};

