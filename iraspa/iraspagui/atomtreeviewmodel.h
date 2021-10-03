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

class MainWindow;

class AtomTreeViewModel: public QAbstractItemModel
{
  Q_OBJECT

public:
  AtomTreeViewModel();

  static char mimeType[];

  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode);
  void setFrame(std::shared_ptr<iRASPAObject> frame);
  bool isActive(std::shared_ptr<iRASPAObject> iraspaStructure) {return (iraspaStructure == _iraspaStructure);}
  std::shared_ptr<iRASPAObject> frame() {return _iraspaStructure;}
  void resetData();

  SKAtomTreeNode* getItem(const QModelIndex &index) const;

  // Helper functions
  QModelIndex indexForNode(SKAtomTreeNode *node, int column = 0) const;
  SKAtomTreeNode *nodeForIndex(const QModelIndex &index) const;
  int rowForNode(SKAtomTreeNode *node) const;
  void insertNode(SKAtomTreeNode *parentNode, int pos, std::shared_ptr<SKAtomTreeNode> node);
  void removeNode(SKAtomTreeNode *node);

  QModelIndexList selectedIndexes();

  //QT
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override final;
  QModelIndex parent(const QModelIndex &index) const override final;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override final;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override final;
  QVariant data(const QModelIndex &index, int role) const override final;
  Qt::ItemFlags flags(const QModelIndex &index) const override final;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override final;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override final;

  bool hasChildren(const QModelIndex &parent) const override final;

  Qt::DropActions supportedDragActions() const override final;
  Qt::DropActions supportedDropActions() const override final;
  QStringList mimeTypes() const override final;
  QMimeData* mimeData(const QModelIndexList &indexes) const override final;
  bool removeRows(int positions, int count, const QModelIndex &parent = QModelIndex()) override final;
  bool insertRows(int position, int rows, const QModelIndex &parent) override final;
  bool insertRow(int position, std::shared_ptr<SKAtomTreeNode> parentNode, std::shared_ptr<SKAtomTreeNode> atomNode);
  bool removeRow(int position, std::shared_ptr<SKAtomTreeNode> parentNode);
  bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override final;
  bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override final;

  void setMainWindow(MainWindow* mainWindow);
private:
  MainWindow* _mainWindow;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<iRASPAObject> _iraspaStructure;
  std::shared_ptr<ProjectStructure> _projectStructure;
signals:
  void updateSelection();
  void rendererReloadData();
  void reloadSelection();
  void collapse(const QModelIndex &index);
  void expand(const QModelIndex &index);
  void updateNetCharge();
};
