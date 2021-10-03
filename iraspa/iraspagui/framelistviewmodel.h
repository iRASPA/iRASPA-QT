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
#include "mainwindow.h"

class FrameListViewModel: public QAbstractItemModel
{
  Q_OBJECT

public:
  FrameListViewModel();

  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode);
  void setMainWindow(MainWindow* mainWindow);
  void setMovie(std::shared_ptr<Movie> movie);
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override final;
  QModelIndex parent(const QModelIndex &child) const override final;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override final;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override final;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override final;

  bool insertRow(int position, std::shared_ptr<Movie> parent, std::shared_ptr<iRASPAObject> iraspaStructure);
  bool insertRow(int position, std::shared_ptr<iRASPAObject> iraspaStructure);
  bool removeRow(int position, std::shared_ptr<Movie> parent);
  bool removeRow(int position);

  Qt::ItemFlags flags(const QModelIndex &index) const override final;
  Qt::DropActions supportedDropActions() const override final;
  Qt::DropActions supportedDragActions() const override final;
  bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override final;

  QMimeData* mimeDataLazy(const QModelIndexList &indexes) const;
  bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override final;

  QMimeData* mimeData(const QModelIndexList &indexes) const override final;
  bool pasteMimeData(const QMimeData *data, int row, int column, const QModelIndex &parent);

  // Helper functions
  QModelIndex indexForNode(iRASPAObject *node, int column = 0) const;
  iRASPAObject *nodeForIndex(const QModelIndex &index) const;
private:
  MainWindow* _mainWindow;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<SceneList> _sceneList;
  std::shared_ptr<Movie> _movie;
signals:
  void updateSelection();
  void rendererReloadData();
  void invalidateCachedAmbientOcclusionTexture(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
};
