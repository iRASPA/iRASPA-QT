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

#include <QHeaderView>
#include "bondlistview.h"
#include "bondlistviewsliderstyleditemdelegate.h"
#include "bondlistviewdeleteselectioncommand.h"
#include "bondlistviewchangeselectioncommand.h"


BondListView::BondListView(QWidget* parent): QTreeView(parent ), _bondModel(std::make_shared<BondListViewModel>())
{
  this->setModel(_bondModel.get());

  QObject::connect(model(),&QAbstractItemModel::modelReset, this, &BondListView::reloadSelection);

  this->viewport()->setMouseTracking(true);

  this->setUniformRowHeights(true);
  this->setAlternatingRowColors(true);
  this->setSelectionBehavior (QAbstractItemView::SelectRows);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);

  this->setStyleSheet(":disabled { color: dark-grey;  background-color: rgb(255, 255, 255);}");

  this->setRootIsDecorated(false);
  this->setItemsExpandable(false);
  this->setIndentation(0);

  pushButtonDelegate = new BondListPushButtonStyledItemDelegate(this);
  this->setItemDelegateForColumn(2, pushButtonDelegate);

  comboBoxDelegate = new BondListViewComboBoxStyledItemDelegate(this);
  this->setItemDelegateForColumn(5, comboBoxDelegate);

  sliderDelegate = new BondListViewSliderStyledItemDelegate(this);
  this->setItemDelegateForColumn(7, sliderDelegate);

  this->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

  QObject::connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &BondListView::setSelectedBonds);
  QObject::connect(_bondModel.get(), &BondListViewModel::updateSelection, this, &BondListView::reloadSelection);
  QObject::connect(_bondModel.get(), &BondListViewModel::rendererReloadData, this, &BondListView::rendererReloadData);
}

void BondListView::resetData()
{
  _bondModel->resetData();
}

void BondListView::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  _projectTreeNode = projectTreeNode;
  _iRASPAProject.reset();
  _iraspaStructure.reset();

  if (projectTreeNode)
  {
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
        }
      }
    }
  }

  // reload and resize the columns
  _bondModel->setFrame(_iraspaStructure.lock());
  this->reloadData();
}

void BondListView::setSelectedFrame(std::shared_ptr<iRASPAStructure> iraspaStructure)
{
  _iraspaStructure = iraspaStructure;

  if(BondListViewModel* bondModel = qobject_cast<BondListViewModel*>(model()))
  {
    bondModel->setFrame(iraspaStructure);
  }
}

void BondListView::reloadSelection()
{

  whileBlocking(selectionModel())->clearSelection();

  if(BondListViewModel* pModel = qobject_cast<BondListViewModel*>(model()))
  {
    for(QModelIndex index : pModel->selectedIndexes())
    {
      whileBlocking(selectionModel())->select(index, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }
  }
  viewport()->update();
}

void BondListView::reloadData()
{
  if(std::shared_ptr<iRASPAStructure> iraspaStructure = _iraspaStructure.lock())
  {
    if(iraspaStructure->structure()->bondSetController()->getNumberOfBonds() > 0)
    {
      this->header()->setStretchLastSection(false);
      this->setColumnWidth(0,25);
      this->setColumnWidth(1,50);
      this->setColumnWidth(2,100);
      this->setColumnWidth(3,50);
      this->setColumnWidth(4,50);
      this->setColumnWidth(5,60);
      this->setColumnWidth(6,60);
      this->setColumnWidth(7,85);  // 20 left for scroller
    }
  }
}


QSize BondListView::sizeHint() const
{
  return QSize(500, 800);
}

void BondListView::keyPressEvent(QKeyEvent *event)
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

void BondListView::deleteSelection()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAStructure> iraspaStructure = _iraspaStructure.lock())
      {
        std::shared_ptr<Structure> structure = iraspaStructure->structure();
        if(_iRASPAProject)
        {
          std::vector<std::shared_ptr<SKAsymmetricBond>> bonds = structure->bondSetController()->selectedObjects();
          std::set<int> bondSelection = structure->bondSetController()->selectionIndexSet();
          BondListViewDeleteSelectionCommand *deleteSelectionCommand = new BondListViewDeleteSelectionCommand(_mainWindow, _bondModel, structure, bonds, bondSelection);
          _iRASPAProject->undoManager().push(deleteSelectionCommand);
        }
      }
    }
  }
}


SKAsymmetricBond* BondListView::getItem(const QModelIndex &index) const
{
  if (index.isValid())
  {
     SKAsymmetricBond *item = static_cast<SKAsymmetricBond*>(index.internalPointer());
     if (item)
     {
       return item;
     }
  }
  return nullptr;
}
void BondListView::setSelectedBonds(const QItemSelection &selected, const QItemSelection &deselected)
{
  if(std::shared_ptr<iRASPAStructure> iraspaStructure = _iraspaStructure.lock())
  {
    std::shared_ptr<Structure> structure = iraspaStructure->structure();
    std::set<int> previousBondSelection = structure->bondSetController()->selectionIndexSet();

    for(QModelIndex index : deselected.indexes())
    {
      SKAsymmetricBond *item = getItem(index);
      std::shared_ptr<SKAsymmetricBond> asymmetricBond = item->shared_from_this();

      structure->bondSetController()->selectionIndexSet().erase(asymmetricBond->asymmetricIndex());
    }

    for(QModelIndex index : selected.indexes())
    {
      SKAsymmetricBond *item = getItem(index);
      std::shared_ptr<SKAsymmetricBond> asymmetricBond = item->shared_from_this();

      structure->bondSetController()->selectionIndexSet().insert(asymmetricBond->asymmetricIndex());
    }

    // Force bonds to selected (bonds that are automatically selected due to their atoms being selected).
    structure->bondSetController()->correctBondSelectionDueToAtomSelection();
    this->reloadSelection();

    if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
    {
      if(projectTreeNode->isEditable())
      {
        if(_iRASPAProject)
        {
          std::set<int> bondSelection = structure->bondSetController()->selectionIndexSet();

          BondListViewChangeSelectionCommand *changeSelectionCommand = new BondListViewChangeSelectionCommand(structure->bondSetController(), _mainWindow,
                                                                         structure, bondSelection, previousBondSelection);
          _iRASPAProject->undoManager().push(changeSelectionCommand);
        }
      }
    }
  }
}


