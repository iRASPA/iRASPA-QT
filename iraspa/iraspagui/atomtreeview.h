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
#include "atomtreeviewmodel.h"
#include "bondlistviewmodel.h"
#include "scenetreeviewmodel.h"
#include "iraspamainwindowconsumerprotocol.h"
#include "atomtreeviewpushbuttonstyleditemdelegate.h"

class QAbstractItemViewPrivate;
class AtomTreeView: public QTreeView, public MainWindowConsumer, public ProjectConsumer, public Reloadable
{
  Q_OBJECT

public:
  AtomTreeView(QWidget* parent = nullptr);
  QSize sizeHint() const override final;

  void setMainWindow(MainWindow* mainWindow) override final;
  void setBondModel(std::shared_ptr<BondListViewModel> bondModel);
  void setSceneModel(std::shared_ptr<SceneTreeViewModel> sceneModel);
  void setNetChargeLineEdit(QLineEdit* lineEdit);

  // update the atomTreeView when a new project is selected in the ProjectTreeView
  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override final;

  // update the atomTreeView when (implicitely or explicitly) a new frame is selected in the Scene/Movie-ListView or frameListView
  void setSelectedFrame(std::shared_ptr<iRASPAObject> structure);

  void resetData();
  void reloadData() override final;
  void reloadSelection() override final;

  virtual void dropEvent(QDropEvent * event) final override;
  void paintEvent(QPaintEvent *event) override final;
  void startDrag(Qt::DropActions supportedActions) override final;
  void dragMoveEvent(QDragMoveEvent *event) override final;

  std::shared_ptr<AtomTreeViewModel> atomModel() {return _atomModel;}
  void deleteSelection();
  void updateNetChargeLineEdit();
private:
  QRect _dropIndicatorRect;
  MainWindow* _mainWindow;
  QLineEdit* _netChargeLineEdit;
  std::shared_ptr<AtomTreeViewModel> _atomModel;
  std::shared_ptr<BondListViewModel> _bondModel;
  std::shared_ptr<SceneTreeViewModel> _sceneModel;
  std::shared_ptr<iRASPAObject> _iraspaStructure;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<iRASPAProject> _iRASPAProject;
  void paintDropIndicator(QPainter& painter);
  QAbstractItemView::DropIndicatorPosition position(QPoint pos, QRect rect, QModelIndex index);
  void addAtom(QModelIndex index);
  void addAtomGroup(QModelIndex index);
  void flattenHierachy();
  void makeSuperCell();
  void copyToNewMovie();
  void moveToNewMovie();
  void invertSelection();
  void removeSymmetry();
  void wrapAtoms();
  void findNiggli();
  void findPrimitive();
  void findSymmetry();
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override final;
  AtomTreeViewPushButtonStyledItemDelegate *pushButtonDelegate;
  SKAtomTreeNode* getItem(const QModelIndex &index) const;
  void exportToPDB() const;
  void exportToMMCIF() const;
  void exportToCIF() const;
  void exportToXYZ() const;
  void exportToPOSCAR() const;

  void scrollToFirstSelected();
  void scrollToLastSelected();
  void visibilityMatchSelection();
  void visibilityInvert();
protected:
  void keyPressEvent(QKeyEvent *event) override final;
  void mousePressEvent(QMouseEvent *e) override final;
public slots:
  void ShowContextMenu(const QPoint &pos);
  void addAtom();
  void addAtomGroup();
signals:
  void rendererReloadData();
  void invalidateCachedAmbientOcclusionTextures(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
};
