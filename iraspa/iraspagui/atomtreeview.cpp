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

#include "atomtreeview.h"
#include <QPainter>
#include <QtCore>
#include <QStyleOption>
#include <QHeaderView>
#include <QLineEdit>
#include <QItemSelection>
#include "atomtreeviewinsertatomcommand.h"
#include "atomtreeviewinsertatomgroupcommand.h"
#include "atomtreeviewdeleteselectioncommand.h"
#include "atomtreeviewchangeselectioncommand.h"
#include "atomtreeviewmakesupercellcommand.h"
#include "atomtreeviewflattenhierarchycommand.h"
#include "atomtreeviewcopyselectiontonewmoviecommand.h"
#include "atomtreeviewmoveselectiontonewmoviecommand.h"
#include "atomtreeviewinvertselectioncommand.h"
#include "skpdbwriter.h"
#include "skcifwriter.h"
#include "skmmcifwriter.h"
#include "skposcarwriter.h"
#include "skxyzwriter.h"
#include "savecifformatdialog.h"
#include "savemmcifformatdialog.h"
#include "savepdbformatdialog.h"
#include "savexyzformatdialog.h"
#include "saveposcarformatdialog.h"
#include "mainwindow.h"
#include "atomtreeviewfindsymmetrycommand.h"
#include "atomtreeviewfindprimitivecommand.h"
#include "atomtreeviewwrapatomstocellcommand.h"
#include "atomtreeviewremovesymmetrycommand.h"

AtomTreeView::AtomTreeView(QWidget* parent): QTreeView(parent ), _atomModel(std::make_shared<AtomTreeViewModel>())
{
  this->setModel(_atomModel.get());

  this->viewport()->setMouseTracking(true);

  this->setUniformRowHeights(true);
  this->setHeaderHidden(true);
  this->setAlternatingRowColors(true);
  this->setSelectionBehavior (QAbstractItemView::SelectRows);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);

  this->setStyleSheet(":disabled { color: dark-grey;  background-color: rgb(255, 255, 255);}");

  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);
  setDragDropMode(DragDropMode::InternalMove);
  setDragDropOverwriteMode(false);
  //setDefaultDropAction(Qt::MoveAction);

  setItemsExpandable(true);
  setRootIsDecorated(true);

  this->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(this, &QWidget::customContextMenuRequested, this, &AtomTreeView::ShowContextMenu);

  _dropIndicatorRect = QRect();

  pushButtonDelegate = new AtomTreeViewPushButtonStyledItemDelegate(this);
  this->setItemDelegateForColumn(1, pushButtonDelegate);

  this->header()->setStretchLastSection(true);
  this->setColumnWidth(0,110);
  this->setColumnWidth(1,50);
  this->setColumnWidth(2,20);
  this->setColumnWidth(3,40);
  this->setColumnWidth(4,65);
  this->setColumnWidth(5,65);
  this->setColumnWidth(6,65);
  this->setColumnWidth(7,65);

  QObject::connect(model(),&QAbstractItemModel::modelReset, this, &AtomTreeView::reloadSelection);
  QObject::connect(_atomModel.get(),&AtomTreeViewModel::collapse, this, &AtomTreeView::collapse);
  QObject::connect(_atomModel.get(),&AtomTreeViewModel::expand, this, &AtomTreeView::expand);
  QObject::connect(_atomModel.get(), &AtomTreeViewModel::reloadSelection, this, &AtomTreeView::reloadSelection);
  QObject::connect(_atomModel.get(), &AtomTreeViewModel::rendererReloadData, this, &AtomTreeView::rendererReloadData);

  QObject::connect(_atomModel.get(), &AtomTreeViewModel::updateSelection, this, &AtomTreeView::reloadSelection);
  QObject::connect(_atomModel.get(), &AtomTreeViewModel::updateNetCharge, this, &AtomTreeView::updateNetChargeLineEdit);
}

void AtomTreeView::setNetChargeLineEdit(QLineEdit* lineEdit)
{
  _netChargeLineEdit = lineEdit;
  _netChargeLineEdit->setReadOnly(true);
  _netChargeLineEdit->setEnabled(true);
}

void AtomTreeView::setMainWindow(MainWindow* mainWindow)
{
  _mainWindow = mainWindow;
  _atomModel->setMainWindow(mainWindow);
}


void AtomTreeView::setBondModel(std::shared_ptr<BondListViewModel> bondModel)
{
  _bondModel = bondModel;
}

void AtomTreeView::setSceneModel(std::shared_ptr<SceneTreeViewModel> sceneModel)
{
  _sceneModel = sceneModel;
}

void AtomTreeView::resetData()
{
  _atomModel->resetData();
}

void AtomTreeView::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  _projectTreeNode = projectTreeNode;
  _iRASPAProject.reset();
  _iraspaStructure.reset();

  if (projectTreeNode)
  {
    _atomModel->setProject(projectTreeNode);
    setEnabled(projectTreeNode->isEditable());
    _iRASPAProject = projectTreeNode->representedObject();
    if(_iRASPAProject)
    {
      if(std::shared_ptr<Project> project = _iRASPAProject->project())
      {
        if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
        {
          std::shared_ptr<Scene> selectedScene = projectStructure->sceneList()->selectedScene();
          std::shared_ptr<Movie> selectedMovie = selectedScene? selectedScene->selectedMovie() : nullptr;
          std::shared_ptr<iRASPAStructure> iraspaStructure = selectedMovie ? selectedMovie->frameAtIndex(0) : nullptr;
          _iraspaStructure = iraspaStructure;

          if(iraspaStructure)
          {
            double netCharge = iraspaStructure->structure()->atomsTreeController()->netCharge();
            _netChargeLineEdit->setText(QString::number(netCharge));
          }
        }
      }
    }
  }

  // reload and resize the columns
  _atomModel->setFrame(_iraspaStructure);
  this->reloadData();
}

void AtomTreeView::setSelectedFrame(std::shared_ptr<iRASPAStructure> iraspastructure)
{
  _iraspaStructure = iraspastructure;

  if(AtomTreeViewModel* atomModel = qobject_cast<AtomTreeViewModel*>(model()))
  {
    atomModel->setFrame(iraspastructure);
  }
}

SKAtomTreeNode* AtomTreeView::getItem(const QModelIndex &index) const
{
  if(_iraspaStructure)
  {
    std::shared_ptr<Structure> structure = _iraspaStructure->structure();
    if (index.isValid())
    {
       SKAtomTreeNode *item = static_cast<SKAtomTreeNode*>(index.internalPointer());
       if (item)
       {
         return item;
       }
    }
    return structure->atomsTreeController()->hiddenRootNode().get();
  }
  return nullptr;
}

void AtomTreeView::keyPressEvent(QKeyEvent *event)
{
  if( event->type() == QEvent::KeyPress )
  {
    QKeyEvent * keyEvent = dynamic_cast<QKeyEvent*>(event);
    if( keyEvent->key() == Qt::Key_Delete )
    {
      deleteSelection();
    }
  }
  QTreeView::keyPressEvent(event);
}

void AtomTreeView::mousePressEvent(QMouseEvent *e)
{
  if(e->button() == Qt::MouseButton::RightButton)
  {
    return;
  }
  else
  {
     QTreeView::mousePressEvent(e);
  }
}


void AtomTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  qDebug() << "Number of selected atoms: " << selected.size();

  if(_iraspaStructure)
  {
    std::shared_ptr<Structure> structure = _iraspaStructure->structure();
    std::shared_ptr<SKAtomTreeController> atomTreeControler = structure->atomsTreeController();
    std::shared_ptr<SKBondSetController> bondListControler = structure->bondSetController();

    AtomSelectionIndexPaths previousAtomSelection = atomTreeControler->selectionIndexPaths();
    BondSelectionIndexSet previousBondSelection = bondListControler->selectionIndexSet();

    for(QModelIndex index : deselected.indexes())
    {
      if(index.column() == 0)
      {
        IndexPath indexPath = IndexPath::indexPath(index);
        atomTreeControler->removeSelectionIndexPath(indexPath);

        SKAtomTreeNode *node = getItem(index);
        std::shared_ptr<SKAtomTreeNode> atom = node->shared_from_this();
        atomTreeControler->removeSelectionIndexPath(atom->indexPath());

        // remove bonds that are connected to this atom from the selection
        int bondIndex=0;
        for(std::shared_ptr<SKAsymmetricBond> bond : bondListControler->arrangedObjects())
        {
          if(bond->atom1() == atom->representedObject() || bond->atom2() == atom->representedObject())
          {
            bondListControler->selectionIndexSet().erase(bondIndex);
          }
          bondIndex++;
        }
      }
    }

    for(QModelIndex index : selected.indexes())
    {
      if(index.column() == 0)
      {
        IndexPath indexPath = IndexPath::indexPath(index);
        atomTreeControler->insertSelectionIndexPath(indexPath);
      }
    }

    bondListControler->correctBondSelectionDueToAtomSelection();


    AtomSelectionIndexPaths atomSelection = atomTreeControler->selectionIndexPaths();
    BondSelectionIndexSet bondSelection = bondListControler->selectionIndexSet();

    AtomTreeViewChangeSelectionCommand *changeSelectionCommand = new AtomTreeViewChangeSelectionCommand(_mainWindow, _atomModel, _bondModel,
                                                                                                        _iraspaStructure,
                                                                                                        atomSelection, previousAtomSelection,
                                                                                                        bondSelection, previousBondSelection, nullptr);
    _iRASPAProject->undoManager().push(changeSelectionCommand);
  }
}

void AtomTreeView::paintEvent(QPaintEvent *event)
{
  QPainter painter(viewport());
  drawTree(&painter, event->region());
  paintDropIndicator(painter);
}

void AtomTreeView::paintDropIndicator(QPainter& painter)
{
  if(state() == QAbstractItemView::DraggingState)
  {
    QStyleOption opt = QStyleOption();
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      opt.init(this);
    #else
      opt.initFrom(this);
    #endif
    opt.rect = _dropIndicatorRect;
    QRect rect = opt.rect;

    QBrush brush = QBrush(QColor(Qt::black));

    if(rect.height() == 0)
    {
      QPen pen = QPen(brush, 2, Qt::SolidLine);
      painter.setPen(pen);
      painter.drawLine(rect.topLeft(), rect.topRight());
    }
    else
    {
      QPen pen = QPen(brush, 2, Qt::SolidLine);
      painter.setPen(pen);
      painter.drawRect(rect);
    }
  }
}

void AtomTreeView::startDrag(Qt::DropActions supportedActions)
{
  QModelIndexList selectionIndexes = selectionModel()->selectedRows();

  std::cout << "Start drag: " << selectionIndexes.count() << std::endl;

  if(!selectionIndexes.isEmpty())
  {
    QMimeData* mimeData = model()->mimeData(selectionIndexes);
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    Qt::DropAction dropAction = Qt::IgnoreAction;
    if((supportedActions & Qt::CopyAction) && (dragDropMode() != QAbstractItemView::InternalMove))
    {
      dropAction = Qt::CopyAction;
    }
    if(drag->exec(supportedActions, dropAction) == Qt::MoveAction)
    {

     // emit invalidateCachedAmbientOcclusionTexture(std::vector<std::shared_ptr<RKRenderStructure>>{_structures.begin(), _structures.end()});
      emit rendererReloadData();

      reloadSelection();
    }
  }
}


void AtomTreeView::dragMoveEvent(QDragMoveEvent *event)
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  QPoint pos = event->pos();
#else
  QPoint pos = event->position().toPoint();
#endif
  QModelIndex index = indexAt(mapFromParent(pos));

  int columnCount = model()->columnCount();

  QRect rect = visualRect(index);
  QRect rect_left = visualRect(index.sibling(index.row(), 0));
  QRect rect_right = visualRect(index.sibling(index.row(), header()->logicalIndex(columnCount - 1)));

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  QAbstractItemView::DropIndicatorPosition dropIndicatorPosition = position(event->pos(), rect, index);
#else
  QAbstractItemView::DropIndicatorPosition dropIndicatorPosition = position(event->position().toPoint(), rect, index);
#endif

  if (dropIndicatorPosition == AboveItem)
  {
    _dropIndicatorRect = QRect(rect_left.left(), rect_left.top(), rect_right.right() - rect_left.left(), 0);
    event->accept();
  }
  else if(dropIndicatorPosition == BelowItem)
  {
    _dropIndicatorRect = QRect(rect_left.left(), rect_left.bottom(), rect_right.right() - rect_left.left(), 0);
    event->accept();
  }
  else if(dropIndicatorPosition == OnItem && index.flags() & Qt::ItemIsDropEnabled)
  {
    _dropIndicatorRect = QRect(rect_left.left(), rect_left.top(), rect_right.right() - rect_left.left(), rect.height());
    event->accept();
  }
  else
  {
    _dropIndicatorRect = QRect();
  }

  model()->setData(index, dropIndicatorPosition, Qt::UserRole);

  viewport()->update();
}

QAbstractItemView::DropIndicatorPosition AtomTreeView::position(QPoint pos, QRect rect, QModelIndex index)
{
  Q_UNUSED(index);
  QAbstractItemView::DropIndicatorPosition r = OnViewport;

  //  margin*2 must be smaller than row height, or the drop onItem rect won't show
  double margin = 6;
  if (pos.y() - rect.top() < margin)
    r = QAbstractItemView::AboveItem;
  else if(rect.bottom() - pos.y() < margin)
    r = QAbstractItemView::BelowItem;
  else if((pos.y() - rect.top() > margin) && (rect.bottom() - pos.y() > margin))
    r = QAbstractItemView::OnItem;

  return r;
}

void AtomTreeView::reloadSelection()
{
  qDebug() << "AtomTreeView::reloadSelection()";
  if(AtomTreeViewModel* pModel = qobject_cast<AtomTreeViewModel*>(model()))
  {
    whileBlocking(selectionModel())->clearSelection();

    for(QModelIndex index : pModel->selectedIndexes())
    {
      whileBlocking(selectionModel())->select(index, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }
  }
  viewport()->update();
}

void AtomTreeView::reloadData()
{
  if(_iraspaStructure)
  {
    std::shared_ptr<Structure> structure = _iraspaStructure->structure();
    std::shared_ptr<SKAtomTreeController> atomTreeController = structure->atomsTreeController();

    if(atomTreeController->rootNodes().size() > 0)
    {
      this->header()->setStretchLastSection(true);
      this->setColumnWidth(0,120);
      this->resizeColumnToContents(1);
      this->resizeColumnToContents(2);
      this->resizeColumnToContents(3);
      this->resizeColumnToContents(4);
      this->resizeColumnToContents(5);
      this->resizeColumnToContents(6);
    }
  }
}

void AtomTreeView::dropEvent(QDropEvent * event)
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  QModelIndex droppedIndex = indexAt( event->pos() );
#else
  QModelIndex droppedIndex = indexAt( event->position().toPoint() );
#endif
  if( !droppedIndex.isValid() )
    return;

  QTreeView::dropEvent(event);

  DropIndicatorPosition dp = dropIndicatorPosition();
  if (dp == QAbstractItemView::BelowItem)
  {
     droppedIndex = droppedIndex.sibling(droppedIndex.row() + 1, droppedIndex.column());
  }
}



QSize AtomTreeView::sizeHint() const
{
  return QSize(500, 800);
}

void AtomTreeView::deleteSelection()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            AtomSelectionNodesAndIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionNodesAndIndexPaths();
            BondSelectionNodesAndIndexSet bondSelection = _iraspaStructure->structure()->bondSetController()->selectionNodesAndIndexSet();

            AtomTreeViewDeleteSelectionCommand *deleteSelectionCommand = new AtomTreeViewDeleteSelectionCommand(_mainWindow, _atomModel.get(), _bondModel.get(),
                                                                                                        projectStructure, _iraspaStructure, atomSelection, bondSelection, nullptr);
            _iRASPAProject->undoManager().push(deleteSelectionCommand);
          }
        }
      }
    }
  }
}

void AtomTreeView::addAtom()
{
  if(_iraspaStructure)
  {
    AtomSelectionNodesAndIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionNodesAndIndexPaths();
    if(atomSelection.second.empty())
    {
      SKAtomTreeNode *parentNode = _iraspaStructure->structure()->atomsTreeController()->hiddenRootNode().get();
      int row = parentNode->childCount();
      AtomTreeViewInsertAtomCommand *insertAtomCommand = new AtomTreeViewInsertAtomCommand(_mainWindow, this, _iraspaStructure, parentNode->shared_from_this(), row,
                                                                                           atomSelection, nullptr);
      _iRASPAProject->undoManager().push(insertAtomCommand);
      return;
    }
  }
}

void AtomTreeView::addAtomGroup()
{
  if(_iraspaStructure)
  {
    AtomSelectionNodesAndIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionNodesAndIndexPaths();
    if(atomSelection.second.empty())
    {
      SKAtomTreeNode *parentNode = _iraspaStructure->structure()->atomsTreeController()->hiddenRootNode().get();
      int row = parentNode->childCount();
      AtomTreeViewInsertAtomGroupCommand *insertAtomCommand = new AtomTreeViewInsertAtomGroupCommand(_mainWindow, this, _iraspaStructure, parentNode->shared_from_this(), row,
                                                                                                     atomSelection, nullptr);
      _iRASPAProject->undoManager().push(insertAtomCommand);
      return;
    }
  }
}

void AtomTreeView::addAtom(QModelIndex index)
{
  if(_iraspaStructure)
  {
    if(AtomTreeViewModel* pModel = qobject_cast<AtomTreeViewModel*>(model()))
    {
      QModelIndex parentIndex = index.isValid() ? index.parent() : QModelIndex();
      int row = index.isValid() ? index.row() + 1 : pModel->rowCount(index);
      SKAtomTreeNode *parentNode = pModel->nodeForIndex(parentIndex);

      AtomSelectionNodesAndIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionNodesAndIndexPaths();

      AtomTreeViewInsertAtomCommand *insertAtomCommand = new AtomTreeViewInsertAtomCommand(_mainWindow, this, _iraspaStructure, parentNode->shared_from_this(), row,
                                                                                                     atomSelection, nullptr);
      _iRASPAProject->undoManager().push(insertAtomCommand);
    }
  }
}

void AtomTreeView::addAtomGroup(QModelIndex index)
{
  if(_iraspaStructure)
  {
    if(AtomTreeViewModel* pModel = qobject_cast<AtomTreeViewModel*>(model()))
    {
      QModelIndex parentIndex = index.isValid() ? index.parent() : QModelIndex();
      int row = index.isValid() ? index.row() + 1 : pModel->rowCount(index);
      SKAtomTreeNode *parentNode = pModel->nodeForIndex(parentIndex);

      AtomSelectionNodesAndIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionNodesAndIndexPaths();

      AtomTreeViewInsertAtomGroupCommand *insertAtomCommand = new AtomTreeViewInsertAtomGroupCommand(_mainWindow, this, _iraspaStructure, parentNode->shared_from_this(), row,
                                                                                                     atomSelection, nullptr);
      _iRASPAProject->undoManager().push(insertAtomCommand);
    }
  }
}

void AtomTreeView::flattenHierachy()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            AtomTreeViewFlattenHierarchyCommand *flattenHierarchyCommand = new AtomTreeViewFlattenHierarchyCommand(_mainWindow, _atomModel.get(), _iraspaStructure, nullptr);
            iRASPAProject->undoManager().push(flattenHierarchyCommand);
          }
        }
      }
    }
  }
}

void AtomTreeView::makeSuperCell()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            AtomTreeViewMakeSuperCellCommand *makeSuperCellCommand = new AtomTreeViewMakeSuperCellCommand(_mainWindow, _atomModel.get(), _iraspaStructure, nullptr);
            iRASPAProject->undoManager().push(makeSuperCellCommand);
          }
        }
      }
    }
  }
}

void AtomTreeView::copyToNewMovie()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            AtomSelectionIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionIndexPaths();
            BondSelectionIndexSet bondSelection = _iraspaStructure->structure()->bondSetController()->selectionIndexSet();
            AtomTreeViewCopySelectionToNewMovieCommand *newCopySelectionCommand = new AtomTreeViewCopySelectionToNewMovieCommand(_mainWindow,
                                                                                    _atomModel.get(), _sceneModel.get(),
                                                                                    projectStructure->sceneList(), _iraspaStructure,
                                                                                    atomSelection, bondSelection, nullptr);
            iRASPAProject->undoManager().push(newCopySelectionCommand);
          }
        }
      }
    }
  }
}

void AtomTreeView::moveToNewMovie()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            AtomSelectionNodesAndIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionNodesAndIndexPaths();
            BondSelectionNodesAndIndexSet bondSelection = _iraspaStructure->structure()->bondSetController()->selectionNodesAndIndexSet();
            AtomTreeViewMoveSelectionToNewMovieCommand *newMoveSelectionCommand = new AtomTreeViewMoveSelectionToNewMovieCommand(_mainWindow,
                                                                                    _atomModel.get(), _bondModel.get(), _sceneModel.get(),
                                                                                    projectStructure->sceneList(), _iraspaStructure,
                                                                                    atomSelection, bondSelection, nullptr);
            iRASPAProject->undoManager().push(newMoveSelectionCommand);
          }
        }
      }
    }
  }
}


void AtomTreeView::invertSelection()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {

            AtomSelectionIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionIndexPaths();
            BondSelectionNodesAndIndexSet bondSelection = _iraspaStructure->structure()->bondSetController()->selectionNodesAndIndexSet();
            AtomTreeViewInvertSelectionCommand *newInvertSelectionCommand = new AtomTreeViewInvertSelectionCommand(_mainWindow,
                                                                                    _iraspaStructure->structure(), atomSelection, bondSelection, nullptr);
            iRASPAProject->undoManager().push(newInvertSelectionCommand);
          }
        }
      }
    }
  }
}

void AtomTreeView::scrollToFirstSelected()
{
  if(_iraspaStructure)
  {
    AtomSelectionNodesAndIndexPaths selection = _iraspaStructure->structure()->atomsTreeController()->selectionNodesAndIndexPaths();
    if(!selection.second.empty())
    {
      std::shared_ptr<SKAtomTreeNode> firstSelected = (*selection.second.begin()).first;
      QModelIndex index = _atomModel->indexForNode(firstSelected.get());
      scrollTo(index,EnsureVisible);
    }
  }
}

void AtomTreeView::scrollToLastSelected()
{
  if(_iraspaStructure)
  {
    AtomSelectionNodesAndIndexPaths selection = _iraspaStructure->structure()->atomsTreeController()->selectionNodesAndIndexPaths();
    if(!selection.second.empty())
    {
      std::shared_ptr<SKAtomTreeNode> firstSelected = (*selection.second.rbegin()).first;
      QModelIndex index = _atomModel->indexForNode(firstSelected.get());
      scrollTo(index,EnsureVisible);
    }
  }
}

void AtomTreeView::visibilityMatchSelection()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(_iraspaStructure)
      {
        std::vector<std::shared_ptr<SKAtomTreeNode>> flattenedNodes = _iraspaStructure->structure()->atomsTreeController()->flattenedLeafNodes();
        std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms{};

        std::transform(flattenedNodes.begin(),flattenedNodes.end(), std::inserter(asymmetricAtoms, asymmetricAtoms.begin()),
                           [](std::shared_ptr<SKAtomTreeNode> treeNode) -> std::shared_ptr<SKAsymmetricAtom>
                           {return treeNode->representedObject();});
        for(const std::shared_ptr<SKAsymmetricAtom> &atom : asymmetricAtoms)
        {
          atom->setVisibility(false);
        }

        AtomSelectionNodesAndIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionNodesAndIndexPaths();
        for(const auto &[atom, indexPath] : atomSelection.second)
        {
          atom->representedObject()->setVisibility(true);
        }
        emit rendererReloadData();
      }
    }
  }
}

void AtomTreeView::visibilityInvert()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(_iraspaStructure)
      {
        std::vector<std::shared_ptr<SKAtomTreeNode>> flattenedNodes = _iraspaStructure->structure()->atomsTreeController()->flattenedLeafNodes();
        std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms{};

        std::transform(flattenedNodes.begin(),flattenedNodes.end(), std::inserter(asymmetricAtoms, asymmetricAtoms.begin()),
                           [](std::shared_ptr<SKAtomTreeNode> treeNode) -> std::shared_ptr<SKAsymmetricAtom>
                           {return treeNode->representedObject();});
        for(const std::shared_ptr<SKAsymmetricAtom> &atom : asymmetricAtoms)
        {
          atom->toggleVisibility();
        }

        emit rendererReloadData();
      }
    }
  }
}

void AtomTreeView::ShowContextMenu(const QPoint &pos)
{
  QModelIndex index = indexAt(pos);

  QMenu contextMenu(tr("Context menu"), nullptr);

  bool isEnabled = false;
  bool isSymmetryEnabled = false;
  if(_iraspaStructure)
  {
    isEnabled = _projectTreeNode->isEditable()  && (model()->rowCount(QModelIndex()) > 0);
    isSymmetryEnabled = isEnabled && _iraspaStructure->structure()->hasSymmetry();
  }

  QAction actionAddAtom("Add atom", this);
  actionAddAtom.setEnabled(isEnabled);
  connect(&actionAddAtom, &QAction::triggered, [this, index](void) {
     this->addAtom(index);
  });
  contextMenu.addAction(&actionAddAtom);

  QAction actionAddAtomGroup("Add atom group", this);
  actionAddAtomGroup.setEnabled(isEnabled);
  connect(&actionAddAtomGroup, &QAction::triggered, [this, index](void) {
     this->addAtomGroup(index);
  });
  contextMenu.addAction(&actionAddAtomGroup);

  QAction actionFlattenHierarchy("Flatten hierarchy", this);
  actionFlattenHierarchy.setEnabled(isEnabled);
  connect(&actionFlattenHierarchy, &QAction::triggered, this, &AtomTreeView::flattenHierachy);
  contextMenu.addAction(&actionFlattenHierarchy);

  QAction actionMakeSuperCell("Make super-cell", this);
  actionMakeSuperCell.setEnabled(isEnabled && isSymmetryEnabled);
  connect(&actionMakeSuperCell, &QAction::triggered, this, &AtomTreeView::makeSuperCell);
  contextMenu.addAction(&actionMakeSuperCell);

  QMenu* subMenuSymmetry = contextMenu.addMenu( "Symmetry" );
  QActionGroup* symmetryGroup = new QActionGroup(this);
  QAction actionSymmetryRemove("Remove symmetry", this);
  actionSymmetryRemove.setEnabled(isEnabled && isSymmetryEnabled);
  symmetryGroup->addAction(&actionSymmetryRemove);
  subMenuSymmetry->addAction(&actionSymmetryRemove);
  connect(&actionSymmetryRemove, &QAction::triggered, this, &AtomTreeView::removeSymmetry);
  QAction actionSymmetryWrapAtoms("Wrap atoms to cell", this);
  actionSymmetryWrapAtoms.setEnabled(isEnabled && isSymmetryEnabled);
  symmetryGroup->addAction(&actionSymmetryWrapAtoms);
  subMenuSymmetry->addAction(&actionSymmetryWrapAtoms);
  connect(&actionSymmetryWrapAtoms, &QAction::triggered, this, &AtomTreeView::wrapAtoms);
  QAction actionFindPrimitive("Find primitive", this);
  actionFindPrimitive.setEnabled(isEnabled && isSymmetryEnabled);
  symmetryGroup->addAction(&actionFindPrimitive);
  subMenuSymmetry->addAction(&actionFindPrimitive);
  connect(&actionFindPrimitive, &QAction::triggered, this, &AtomTreeView::findPrimitive);
  QAction actionFindSymmetry("Find and impose symmetry", this);
  actionFindSymmetry.setEnabled(isEnabled && isSymmetryEnabled);
  symmetryGroup->addAction(&actionFindSymmetry);
  subMenuSymmetry->addAction(&actionFindSymmetry);
  connect(&actionFindSymmetry, &QAction::triggered, this, &AtomTreeView::findSymmetry);

  QMenu* subMenuSelection = contextMenu.addMenu( "Selection" );
  QActionGroup* selectionGroup = new QActionGroup(this);
  QAction actionSelectionInvert("Invert", this);
  actionSelectionInvert.setEnabled(isEnabled);
  selectionGroup->addAction(&actionSelectionInvert);
  subMenuSelection->addAction(&actionSelectionInvert);
  connect(&actionSelectionInvert, &QAction::triggered, this, &AtomTreeView::invertSelection);
  QAction actionCopyToNewMovie("CopyToNewMovie", this);
  actionCopyToNewMovie.setEnabled(isEnabled);
  selectionGroup->addAction(&actionCopyToNewMovie);
  subMenuSelection->addAction(&actionCopyToNewMovie);
  connect(&actionCopyToNewMovie, &QAction::triggered, this, &AtomTreeView::copyToNewMovie);
  QAction actionMoveToNewMovie("MoveToNewMovie", this);
  actionMoveToNewMovie.setEnabled(isEnabled);
  selectionGroup->addAction(&actionMoveToNewMovie);
  subMenuSelection->addAction(&actionMoveToNewMovie);
  connect(&actionMoveToNewMovie, &QAction::triggered, this, &AtomTreeView::moveToNewMovie);

  QMenu* subMenuVisibility = contextMenu.addMenu( "Visibility" );
  QActionGroup* visibilityGroup = new QActionGroup(this);
  QAction actionVisibilityMatchSelection("Match selection", this);
  actionVisibilityMatchSelection.setEnabled(isEnabled);
  visibilityGroup->addAction(&actionVisibilityMatchSelection);
  subMenuVisibility->addAction(&actionVisibilityMatchSelection);
  connect(&actionVisibilityMatchSelection, &QAction::triggered, this, &AtomTreeView::visibilityMatchSelection);
  QAction actionVisibilityInvert("Invert", this);
  actionVisibilityInvert.setEnabled(isEnabled);
  visibilityGroup->addAction(&actionVisibilityInvert);
  subMenuVisibility->addAction(&actionVisibilityInvert);
  connect(&actionVisibilityInvert, &QAction::triggered, this, &AtomTreeView::visibilityInvert);

  QMenu* subMenuScrollTo = contextMenu.addMenu( "Scroll to" );
  QActionGroup* scrollToGroup = new QActionGroup(this);
  QAction actionScrollToTop("Top", this);
  actionScrollToTop.setEnabled(isEnabled);
  scrollToGroup->addAction(&actionScrollToTop);
  subMenuScrollTo->addAction(&actionScrollToTop);
  connect(&actionScrollToTop, &QAction::triggered, this, &AtomTreeView::scrollToTop);
  QAction actionScrollToBottom("Bottom", this);
  actionScrollToBottom.setEnabled(isEnabled);
  scrollToGroup->addAction(&actionScrollToBottom);
  subMenuScrollTo->addAction(&actionScrollToBottom);
  connect(&actionScrollToBottom, &QAction::triggered, this, &AtomTreeView::scrollToBottom);
  QAction actionScrollToFirstSelected("First selected", this);
  actionScrollToFirstSelected.setEnabled(isEnabled);
  scrollToGroup->addAction(&actionScrollToFirstSelected);
  subMenuScrollTo->addAction(&actionScrollToFirstSelected);
  connect(&actionScrollToFirstSelected, &QAction::triggered, this, &AtomTreeView::scrollToFirstSelected);
  QAction actionScrollToLastSelected("Last selected", this);
  actionScrollToLastSelected.setEnabled(isEnabled);
  scrollToGroup->addAction(&actionScrollToLastSelected);
  subMenuScrollTo->addAction(&actionScrollToLastSelected);
  connect(&actionScrollToLastSelected, &QAction::triggered, this, &AtomTreeView::scrollToLastSelected);


  QMenu* subMenuExport = contextMenu.addMenu( "Export to" );
  QActionGroup* exportToGroup = new QActionGroup(this);
  QAction actionExportToPDB("PDB", this);
  actionExportToPDB.setEnabled(isEnabled);
  exportToGroup->addAction(&actionExportToPDB);
  subMenuExport->addAction(&actionExportToPDB);
  connect(&actionExportToPDB, &QAction::triggered, this, &AtomTreeView::exportToPDB);
  QAction actionExportToMMCIF("mmCIF", this);
  actionExportToMMCIF.setEnabled(isEnabled);
  exportToGroup->addAction(&actionExportToMMCIF);
  subMenuExport->addAction(&actionExportToMMCIF);
  connect(&actionExportToMMCIF, &QAction::triggered, this, &AtomTreeView::exportToMMCIF);
  QAction actionExportToCIF("CIF", this);
  actionExportToCIF.setEnabled(isEnabled);
  exportToGroup->addAction(&actionExportToCIF);
  subMenuExport->addAction(&actionExportToCIF);
  connect(&actionExportToCIF, &QAction::triggered, this, &AtomTreeView::exportToCIF);
  QAction actionExportToXYZ("XYZ", this);
  actionExportToXYZ.setEnabled(isEnabled);
  exportToGroup->addAction(&actionExportToXYZ);
  subMenuExport->addAction(&actionExportToXYZ);
  connect(&actionExportToXYZ, &QAction::triggered, this, &AtomTreeView::exportToXYZ);
  QAction actionExportToPOSCAR("VASP POSCAR", this);
  actionExportToPOSCAR.setEnabled(isEnabled);
  exportToGroup->addAction(&actionExportToPOSCAR);
  subMenuExport->addAction(&actionExportToPOSCAR);
  connect(&actionExportToPOSCAR, &QAction::triggered, this, &AtomTreeView::exportToPOSCAR);

  contextMenu.exec(viewport()->mapToGlobal(pos));
}

void AtomTreeView::removeSymmetry()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            AtomSelectionIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionIndexPaths();
            BondSelectionNodesAndIndexSet bondSelection = _iraspaStructure->structure()->bondSetController()->selectionNodesAndIndexSet();
            AtomTreeViewRemoveSymmetryCommand *removeSymmetryCommand = new AtomTreeViewRemoveSymmetryCommand(_mainWindow, _atomModel.get(), _iraspaStructure, nullptr);
            iRASPAProject->undoManager().push(removeSymmetryCommand);
          }
        }
      }
    }
  }
}

void AtomTreeView::wrapAtoms()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            AtomSelectionIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionIndexPaths();
            BondSelectionNodesAndIndexSet bondSelection = _iraspaStructure->structure()->bondSetController()->selectionNodesAndIndexSet();
            AtomTreeViewWrapAtomsToCellCommand *wrapAtomsToCellCommand = new AtomTreeViewWrapAtomsToCellCommand(_mainWindow, _atomModel.get(), _iraspaStructure, nullptr);
            iRASPAProject->undoManager().push(wrapAtomsToCellCommand);
          }
        }
      }
    }
  }
}

void AtomTreeView::findPrimitive()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            AtomSelectionIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionIndexPaths();
            BondSelectionNodesAndIndexSet bondSelection = _iraspaStructure->structure()->bondSetController()->selectionNodesAndIndexSet();
            AtomTreeViewFindPrimitiveCommand *findSymmetryCommand = new AtomTreeViewFindPrimitiveCommand(_mainWindow,
                                                                                    _iraspaStructure, atomSelection, bondSelection, nullptr);
            iRASPAProject->undoManager().push(findSymmetryCommand);

          }
        }
      }
    }
  }
}

void AtomTreeView::findSymmetry()
{
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            AtomSelectionIndexPaths atomSelection = _iraspaStructure->structure()->atomsTreeController()->selectionIndexPaths();
            BondSelectionNodesAndIndexSet bondSelection = _iraspaStructure->structure()->bondSetController()->selectionNodesAndIndexSet();
            AtomTreeViewFindSymmetryCommand *findSymmetryCommand = new AtomTreeViewFindSymmetryCommand(_mainWindow,
                                                                                    _iraspaStructure, atomSelection, bondSelection, nullptr);
            iRASPAProject->undoManager().push(findSymmetryCommand);

          }
        }
      }
    }
  }
}

void AtomTreeView::exportToPDB() const
{
  if(_iraspaStructure)
  {
    QString displayName = _iraspaStructure->structure()->displayName();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = _iraspaStructure->structure()->asymmetricAtomsCopiedAndTransformedToCartesianPositions();
    SKSpaceGroup &spaceGroup = _iraspaStructure->structure()->spaceGroup();
    std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellData = _iraspaStructure->structure()->cellForCartesianPositions();
    std::shared_ptr<SKCell> cell = cellData.has_value() ? cellData->first : nullptr;
    double3 origin = cellData.has_value() ? cellData->second : double3(0.0,0.0,0.0);

    SKPDBWriter writer = SKPDBWriter(displayName, spaceGroup, cell, origin, atoms);

    SavePDBFormatDialog dialog(nullptr);

    if(dialog.exec() == QDialog::Accepted)
    {
      QList<QUrl> fileURLs = dialog.selectedUrls();
      if(fileURLs.isEmpty())
      {
        return;
      }
      else
      {
        QFile qFile = fileURLs.first().toLocalFile();
        if (qFile.open(QIODevice::WriteOnly))
        {
          QTextStream out(&qFile);
          out << writer.string();
          qFile.close();
        }
      }
    }
  }
}

void AtomTreeView::exportToMMCIF() const
{
  if(_iraspaStructure)
  {
    QString displayName = _iraspaStructure->structure()->displayName();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = _iraspaStructure->structure()->asymmetricAtomsCopiedAndTransformedToCartesianPositions();
    SKSpaceGroup &spaceGroup = _iraspaStructure->structure()->spaceGroup();
    std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellData = _iraspaStructure->structure()->cellForCartesianPositions();
    std::shared_ptr<SKCell> cell = cellData.has_value() ? cellData->first : nullptr;
    double3 origin = cellData.has_value() ? cellData->second : double3(0.0,0.0,0.0);

    SKmmCIFWriter writer = SKmmCIFWriter(displayName, spaceGroup, cell, origin, atoms);

    SaveMMCIFFormatDialog dialog(nullptr);

    if(dialog.exec() == QDialog::Accepted)
    {
      QList<QUrl> fileURLs = dialog.selectedUrls();
      if(fileURLs.isEmpty())
      {
        return;
      }
      else
      {
        QFile qFile = fileURLs.first().toLocalFile();
        if (qFile.open(QIODevice::WriteOnly))
        {
          QTextStream out(&qFile);
          out << writer.string();
          qFile.close();
        }
      }
    }
  }
}

void AtomTreeView::exportToCIF() const
{
  if(_iraspaStructure)
  {
    QString displayName = _iraspaStructure->structure()->displayName();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = _iraspaStructure->structure()->asymmetricAtomsCopiedAndTransformedToFractionalPositions();
    SKSpaceGroup &spaceGroup = _iraspaStructure->structure()->spaceGroup();
    std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cell = _iraspaStructure->structure()->cellForFractionalPositions();

    SKCIFWriter writer = SKCIFWriter(displayName, spaceGroup, cell->first, cell->second, atoms);

    SaveCIFFormatDialog dialog(nullptr);

    if(dialog.exec() == QDialog::Accepted)
    {
      QList<QUrl> fileURLs = dialog.selectedUrls();
      if(fileURLs.isEmpty())
      {
        return;
      }
      else
      {
        QFile qFile = fileURLs.first().toLocalFile();
        if (qFile.open(QIODevice::WriteOnly))
        {
          QTextStream out(&qFile);
          out << writer.string();
          qFile.close();
        }
      }
    }
  }
}

void AtomTreeView::exportToXYZ() const
{
  if(_iraspaStructure)
  {
    QString displayName = _iraspaStructure->structure()->displayName();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = _iraspaStructure->structure()->atomsCopiedAndTransformedToCartesianPositions();
    SKSpaceGroup &spaceGroup = _iraspaStructure->structure()->spaceGroup();
    std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellData = _iraspaStructure->structure()->cellForCartesianPositions();
    std::shared_ptr<SKCell> cell = cellData.has_value() ? cellData->first : nullptr;
    double3 origin = cellData.has_value() ? cellData->second : double3(0.0,0.0,0.0);

    SKXYZWriter writer = SKXYZWriter(displayName, spaceGroup, cell, origin, atoms);

    SaveXYZFormatDialog dialog(nullptr);

    if(dialog.exec() == QDialog::Accepted)
    {
      QList<QUrl> fileURLs = dialog.selectedUrls();
      if(fileURLs.isEmpty())
      {
        return;
      }
      else
      {
        QFile qFile = fileURLs.first().toLocalFile();
        if (qFile.open(QIODevice::WriteOnly))
        {
          QTextStream out(&qFile);
          out << writer.string();
          qFile.close();
        }
      }
    }
  }
}

void AtomTreeView::exportToPOSCAR() const
{
  if(_iraspaStructure)
  {
    QString displayName = _iraspaStructure->structure()->displayName();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = _iraspaStructure->structure()->atomsCopiedAndTransformedToFractionalPositions();
    SKSpaceGroup &spaceGroup = _iraspaStructure->structure()->spaceGroup();
    std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cell = _iraspaStructure->structure()->cellForFractionalPositions();

    SKPOSCARWriter writer = SKPOSCARWriter(displayName, spaceGroup, cell->first, cell->second, atoms);

    SavePOSCARFormatDialog dialog(nullptr);

    if(dialog.exec() == QDialog::Accepted)
    {
      QList<QUrl> fileURLs = dialog.selectedUrls();
      if(fileURLs.isEmpty())
      {
        return;
      }
      else
      {
        QFile qFile = fileURLs.first().toLocalFile();
        if (qFile.open(QIODevice::WriteOnly))
        {
          QTextStream out(&qFile);
          out << writer.string();
          qFile.close();
        }
      }
    }
  }
}

void AtomTreeView::updateNetChargeLineEdit()
{
  if(_iraspaStructure)
  {
    double netCharge = _iraspaStructure->structure()->atomsTreeController()->netCharge();
    _netChargeLineEdit->setText(QString::number(netCharge));
  }
}


