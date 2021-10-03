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
#include <QAbstractItemView>
#include <optional>
#include <iraspakit.h>
#include "bondlistviewmodel.h"
#include "atomtreeviewmodel.h"
#include "iraspamainwindowconsumerprotocol.h"
#include "bondlistpushbuttonstyleditemdelegate.h"
#include "bondlistviewcomboboxstyleditemdelegate.h"
#include "bondlistviewsliderstyleditemdelegate.h"

class BondListView: public QTreeView, public MainWindowConsumer, public ProjectConsumer, public Reloadable
{
  Q_OBJECT

public:
  BondListView(QWidget* parent = nullptr);
  QSize sizeHint() const override final;
  void setMainWindow(MainWindow* mainWindow) override final {_mainWindow = mainWindow;}

  // update the bondListView when a new project is selected in the ProjectTreeView
  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override final;

  // update the bondListView when (implicitely or explicitly) a new frame is selected in the Scene/Movie-ListView or frameListView
  void setSelectedFrame(std::shared_ptr<iRASPAObject> iraspastructure);

  void resetData();
  void reloadSelection() override final;
  void reloadData() override final;

  std::shared_ptr<BondListViewModel> bondModel() {return _bondModel;}
private:
  MainWindow* _mainWindow;
  std::shared_ptr<BondListViewModel> _bondModel;
  std::weak_ptr<iRASPAObject> _iraspaStructure;
  std::weak_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<iRASPAProject> _iRASPAProject;

  BondListPushButtonStyledItemDelegate *pushButtonDelegate;
  BondListViewComboBoxStyledItemDelegate *comboBoxDelegate;
  BondListViewSliderStyledItemDelegate *sliderDelegate;
  SKAsymmetricBond* getItem(const QModelIndex &index) const;
  void setSelectedBonds(const QItemSelection &selected, const QItemSelection &deselected);
  void deleteSelection();
protected:
  void keyPressEvent(QKeyEvent *event) override final;
signals:
  void rendererReloadData();
};
