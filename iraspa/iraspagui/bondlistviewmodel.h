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

class BondListViewModel: public QAbstractItemModel
{
  Q_OBJECT

public:
  BondListViewModel();

  void resetData();
  void setFrame(std::shared_ptr<iRASPAObject> frame);
  bool isActive(std::shared_ptr<iRASPAObject> iraspaStructure) {return (iraspaStructure == _iraspaStructure);}
  std::shared_ptr<iRASPAObject> frame() {return _iraspaStructure;}

  //QT
  QModelIndex index(int row, int column, const QModelIndex &parent) const override final;
  QModelIndex parent(const QModelIndex &index) const override final;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override final;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override final;
  QVariant data(const QModelIndex &index, int role) const override final;
  Qt::ItemFlags flags(const QModelIndex &index) const override final;

  bool insertRow(int position, std::shared_ptr<SKAsymmetricBond> bondNode);
  bool removeRow(int position);

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override final;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override final;

  QModelIndexList selectedIndexes();

  void deleteSelection(std::shared_ptr<Structure> structure, std::set<int> indexSet);
  void deleteSelection(std::shared_ptr<Structure> structure, BondSelectionNodesAndIndexSet selection);
  void insertSelection(std::shared_ptr<Structure> structure, std::vector<std::shared_ptr<SKAsymmetricBond>> bonds, std::set<int> indexSet);
  void insertSelection(std::shared_ptr<Structure> structure, BondSelectionNodesAndIndexSet selection);
private:
  std::shared_ptr<iRASPAObject> _iraspaStructure;
signals:
  void updateSelection();
  void reloadRenderDataRenderer();
  void rendererReloadData();
};
