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

#include "atomtreeviewdeleteselectioncommand.h"
#include <QDebug>
#include <algorithm>

AtomTreeViewDeleteSelectionCommand::AtomTreeViewDeleteSelectionCommand(MainWindow *mainWindow, AtomTreeViewModel *atomModel, BondListViewModel *bondModel,
                                                                       std::shared_ptr<ProjectStructure> projectStructure, std::shared_ptr<iRASPAStructure> iraspaStructure,
                                                                       AtomSelectionNodesAndIndexPaths atomSelection, BondSelectionNodesAndIndexSet bondSelection, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _atomModel(atomModel),
  _bondModel(bondModel),
  _iraspaStructure(iraspaStructure),
  _structure(iraspaStructure->structure()),
  _atomSelection(atomSelection),
  _bondSelection(bondSelection),
  _reverseBondSelection({})
{
  Q_UNUSED(parent);

  setText(QString("Delete selected atoms"));

  _atomTreeController = _structure->atomsTreeController();
  _bondListController = _structure->bondSetController();

  _reversedAtomSelection.first = _atomSelection.first;
  std::reverse_copy(_atomSelection.second.begin(), _atomSelection.second.end(), std::inserter(_reversedAtomSelection.second, _reversedAtomSelection.second.begin()));

  std::reverse_copy(bondSelection.begin(), bondSelection.end(), std::back_inserter(_reverseBondSelection));

  _structures = projectStructure->sceneList()->invalidatediRASPAStructures();
}

void AtomTreeViewDeleteSelectionCommand::redo()
{
  if(_bondModel->isActive(_iraspaStructure))
  {
    _bondModel->layoutAboutToBeChanged();
    for(const auto &[bondItem, row] : _reverseBondSelection)
    {
      whileBlocking(_bondModel)->removeRow(row);
    }
    _bondModel->layoutChanged();

    _bondListController->setSelection(BondSelectionNodesAndIndexSet());
    emit _bondModel->updateSelection();
  }
  else
  {
    for(const auto &[bondItem, row] : _reverseBondSelection)
    {
      _iraspaStructure->structure()->bondSetController()->removeBond(row);
    }
  }


  if(_atomModel->isActive(_iraspaStructure))
  {
    _atomModel->layoutAboutToBeChanged();
    for(const auto &[atomNode, indexPath] : _reversedAtomSelection.second)
    {
      int row = int(indexPath.lastIndex());
      std::shared_ptr<SKAtomTreeNode> parentNode = _atomTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
      whileBlocking(_atomModel)->removeRow(row, parentNode);
    }
    _atomModel->layoutChanged();

    _atomTreeController->clearSelection();
    emit _atomModel->updateSelection();
  }
  else
  {
    for(const auto &[atomNode, indexPath] : _reversedAtomSelection.second)
    {
      atomNode->removeFromParent();
    }
    _atomTreeController->setSelectionIndexPaths(AtomSelectionIndexPaths());
  }

  _atomTreeController->setTags();
  _bondListController->setTags();

  if(_mainWindow)
  {
    emit _mainWindow->invalidateCachedAmbientOcclusionTextures(_structures);
    emit _mainWindow->invalidateCachedIsoSurfaces(_structures);
    emit _mainWindow->rendererReloadData();

    _mainWindow->updateControlPanel();
    _mainWindow->documentWasModified();
  }
}

void AtomTreeViewDeleteSelectionCommand::undo()
{
  if(_atomModel->isActive(_iraspaStructure))
  {
    _atomModel->layoutAboutToBeChanged();
    for(const auto &[atomNode, indexPath] : _atomSelection.second)
    {
      std::shared_ptr<SKAtomTreeNode> parentNode = _atomTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
      whileBlocking(_atomModel)->insertRow(int(indexPath.lastIndex()), parentNode, atomNode);
    }
    _atomModel->layoutChanged();

    _atomTreeController->setSelectionIndexPaths(_atomSelection);
    emit _atomModel->updateSelection();
  }
  else
  {
    for(const auto &[atomNode, indexPath] : _atomSelection.second)
    {
      int row = int(indexPath.lastIndex());
      std::shared_ptr<SKAtomTreeNode> parentNode = _atomTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
      parentNode->insertChild(row, atomNode);
    }
    _atomTreeController->setSelectionIndexPaths(_atomSelection);
  }

  if(_bondModel->isActive(_iraspaStructure))
  {
    _bondModel->layoutAboutToBeChanged();
    for(const auto &[bondItem, row] : _bondSelection)
    {
      whileBlocking(_bondModel)->insertRow(row, bondItem);
    }
    _bondModel->layoutChanged();

    _bondListController->setSelection(_bondSelection);
    emit _bondModel->updateSelection();
  }
  else
  {
    for(const auto &[bondItem, row] : _bondSelection)
    {
      _iraspaStructure->structure()->bondSetController()->insertBond(bondItem, row);
    }
  }

  _atomTreeController->setTags();
  _bondListController->setTags();

  if(_mainWindow)
  {
    emit _mainWindow->invalidateCachedAmbientOcclusionTextures(_structures);
    emit _mainWindow->invalidateCachedIsoSurfaces(_structures);
    emit _mainWindow->rendererReloadData();

    _mainWindow->updateControlPanel();
    _mainWindow->documentWasModified();
  }
}
