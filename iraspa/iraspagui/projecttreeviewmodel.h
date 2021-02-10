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
#include <QUndoStack>
#include <QModelIndex>
#include <optional>
#include <iraspakit.h>
#include "mainwindow.h"

class ProjectTreeViewModel: public QAbstractItemModel
{
  Q_OBJECT

public:
  ProjectTreeViewModel();
  void setDocumentData(std::shared_ptr<DocumentData> documentData);
  void insertGalleryData(std::shared_ptr<DocumentData> database);
  void insertDatabaseCoReMOFData(std::shared_ptr<DocumentData> database);
  void insertDatabaseCoReMOFDDECData(std::shared_ptr<DocumentData> database);
  void insertDatabaseIZAFData(std::shared_ptr<DocumentData> database);
  std::shared_ptr<ProjectTreeController> projectTreeController() {return _projectTreeController;}

  bool isMainSelectedItem(std::shared_ptr<ProjectTreeNode> treeNode);
  QModelIndexList selectedIndexes();

	bool insertRows(int position, int rows, const QModelIndex &parent, std::shared_ptr<ProjectTreeNode> item);

  bool removeRows(int positions, int rows, const QModelIndex &parent = QModelIndex()) override final;
  bool insertRows(int position, int rows, const QModelIndex &parent) override final;

  bool insertRow(int position, std::shared_ptr<ProjectTreeNode> parent, std::shared_ptr<ProjectTreeNode> project);
  bool removeRow(int position, std::shared_ptr<ProjectTreeNode> parent);

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override final;
  QModelIndex parent(const QModelIndex &child) const override final;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override final;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override final;
  QVariant data(const QModelIndex &index, int role) const override final;
  Qt::ItemFlags flags(const QModelIndex &index) const override final;

  bool hasChildren(const QModelIndex &parent) const override final;

  bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
  bool setData(const QModelIndex &index, std::shared_ptr<iRASPAProject> value, bool isGroup=false);

  Qt::DropActions supportedDropActions() const override final;
  Qt::DropActions supportedDragActions() const override final;
  QStringList mimeTypes() const override final;
  QMimeData* mimeData(const QModelIndexList &indexes) const override final;
  QMimeData* mimeDataLazy(const QModelIndexList &indexes) const;
  bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override final;
  bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override final;
  bool pasteMimeData(const QMimeData *data, int row, int column, const QModelIndex &parent);

  void setUndoStack(QUndoStack *undoStack) {_undoStack = undoStack;}
  void setMainWindow(MainWindow* mainWindow);

  // Helper functions
  QModelIndex indexForNode(ProjectTreeNode *node, int column = 0) const;
  ProjectTreeNode *nodeForIndex(const QModelIndex &index) const;
  size_t rowForNode(ProjectTreeNode *node) const;
signals:
  void updateSelection();
  void collapse(const QModelIndex &index);
  void expand(const QModelIndex &index);
private:
  MainWindow *_mainWindow;
  std::shared_ptr<ProjectTreeController> _projectTreeController;
  QUndoStack *_undoStack;
  QIcon _folderIcon;
  QFont _font;
};
