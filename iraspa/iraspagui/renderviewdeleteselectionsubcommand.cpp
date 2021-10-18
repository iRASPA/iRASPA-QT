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

#include "renderviewdeleteselectionsubcommand.h"
#include <QDebug>
#include <algorithm>

RenderViewDeleteSelectionSubCommand::RenderViewDeleteSelectionSubCommand(MainWindow *mainWindow, AtomTreeViewModel *atomModel, BondListViewModel *bondModel,
                                                                       std::shared_ptr<iRASPAObject> iraspaStructure,
                                                                       AtomSelectionIndexPaths atomSelection, BondSelectionNodesAndIndexSet bondSelection, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _atomModel(atomModel),
  _bondModel(bondModel),
  _iraspaStructure(iraspaStructure),
  _atomSelection(atomSelection),
  _bondSelection(bondSelection),
  _selectedAtomNodes({}),
  _reverseSelectedAtomNodes({}),
  _reverseBondSelection({})
{
  Q_UNUSED(parent);

  setText(QString("Delete selected atoms"));

  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(iraspaStructure->object()))
  {
    _atomTreeController = atomViewer->atomsTreeController();

    std::transform(atomSelection.second.begin(), atomSelection.second.end(), std::back_inserter(_selectedAtomNodes),
                   [this](IndexPath indexPath) -> std::tuple< std::shared_ptr<SKAtomTreeNode>, std::shared_ptr<SKAtomTreeNode>, size_t>
                   {  std::shared_ptr<SKAtomTreeNode> node = _atomTreeController->nodeAtIndexPath(indexPath);
                      return std::make_tuple(node, node->parent(), indexPath.lastIndex());});

    std::reverse_copy(_selectedAtomNodes.begin(), _selectedAtomNodes.end(), std::back_inserter(_reverseSelectedAtomNodes));

    if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(iraspaStructure->object()))
    {
      _bondListController = bondViewer->bondSetController();
      std::reverse_copy(bondSelection.begin(), bondSelection.end(), std::back_inserter(_reverseBondSelection));
    }
  }
}

void RenderViewDeleteSelectionSubCommand::redo()
{
  if(_bondModel->isActive(_iraspaStructure))
  {
    emit _bondModel->layoutAboutToBeChanged();
    for(const auto &[bondItem, row] : _reverseBondSelection)
    {
      whileBlocking(_bondModel)->removeRow(row);
    }
    emit _bondModel->layoutChanged();
  }
  else
  {
    if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
    {
      for(const auto &[bondItem, row] : _reverseBondSelection)
      {
        bondViewer->bondSetController()->removeBond(row);
      }
    }
  }

  if(_atomModel->isActive(_iraspaStructure))
  {
    emit _atomModel->layoutAboutToBeChanged();
    for(const auto &[projectNode, parentNode, row] : _reverseSelectedAtomNodes)
    {
      whileBlocking(_atomModel)->removeRow(row, parentNode);
    }
    emit _atomModel->layoutChanged();
  }
  else
  {
    for(const auto &[projectNode, parentNode, row] : _reverseSelectedAtomNodes)
    {
      parentNode->removeChild(row);
    }
  }

  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_iraspaStructure->object()))
  {
    _atomTreeController->setTags();
    _atomTreeController->setSelectionIndexPaths(AtomSelectionIndexPaths());
  }

  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
  {
    _bondListController->setTags();
    _bondListController->setSelection(BondSelectionNodesAndIndexSet());
  }
}

void RenderViewDeleteSelectionSubCommand::undo()
{
  if(_atomModel->isActive(_iraspaStructure))
  {
    emit _atomModel->layoutAboutToBeChanged();
    for(const auto &[projectNode, parentNode, row] : _selectedAtomNodes)
    {
      whileBlocking(_atomModel)->insertRow(row, parentNode, projectNode);
    }
    emit _atomModel->layoutChanged();
  }
  else
  {
    for(const auto &[projectNode, parentNode, row] : _selectedAtomNodes)
    {
      parentNode->insertChild(row, projectNode);
    }
  }

  if(_bondModel->isActive(_iraspaStructure))
  {
    emit _bondModel->layoutAboutToBeChanged();
    for(const auto &[bondItem, row] : _bondSelection)
    {
      whileBlocking(_bondModel)->insertRow(row, bondItem);
    }
    emit _bondModel->layoutChanged();
  }
  else
  {
    if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
    {
      for(const auto &[bondItem, row] : _bondSelection)
      {
        bondViewer->bondSetController()->insertBond(bondItem, row);
      }
    }
  }

  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_iraspaStructure->object()))
  {
    _atomTreeController->setTags();
    _atomTreeController->setSelectionIndexPaths(_atomSelection);
  }

  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
  {
    _bondListController->setTags();
    _bondListController->setSelection(_bondSelection);
  }
}
