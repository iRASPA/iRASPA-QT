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
                                                                       std::shared_ptr<ProjectStructure> projectStructure, std::shared_ptr<iRASPAObject> iraspaStructure,
                                                                       AtomSelectionNodesAndIndexPaths atomSelection, BondSelectionNodesAndIndexSet bondSelection, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _atomModel(atomModel),
  _bondModel(bondModel),
  _iraspaStructure(iraspaStructure),
  _object(iraspaStructure->object()),
  _atomSelection(atomSelection),
  _bondSelection(bondSelection),
  _reverseBondSelection({})
{
  Q_UNUSED(parent);

  setText(QString("Delete selected atoms"));

  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_object))
  {
    _atomTreeController = atomViewer->atomsTreeController();
  }
  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_object))
  {
    _bondListController = bondViewer->bondSetController();
  }

  _reversedAtomSelection.first = _atomSelection.first;
  std::reverse_copy(_atomSelection.second.begin(), _atomSelection.second.end(), std::inserter(_reversedAtomSelection.second, _reversedAtomSelection.second.begin()));

  std::reverse_copy(bondSelection.begin(), bondSelection.end(), std::back_inserter(_reverseBondSelection));

  _structures = projectStructure->sceneList()->invalidatediRASPAStructures();
}

void AtomTreeViewDeleteSelectionCommand::redo()
{
  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
  {
    if(_bondModel->isActive(_iraspaStructure))
    {
      emit _bondModel->layoutAboutToBeChanged();
      for(const auto &[bondItem, row] : _reverseBondSelection)
      {
        whileBlocking(_bondModel)->removeRow(row);
      }
      emit _bondModel->layoutChanged();

      _bondListController->setSelection(BondSelectionNodesAndIndexSet());
      emit _bondModel->updateSelection();
    }
    else
    {
      for(const auto &[bondItem, row] : _reverseBondSelection)
      {
        bondViewer->bondSetController()->removeBond(row);
      }
    }
  }

  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_object))
  {
    if(_atomModel->isActive(_iraspaStructure))
    {
      emit _atomModel->layoutAboutToBeChanged();
      for(const auto &[atomNode, indexPath] : _reversedAtomSelection.second)
      {
        int row = int(indexPath.lastIndex());
        std::shared_ptr<SKAtomTreeNode> parentNode = _atomTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
        whileBlocking(_atomModel)->removeRow(row, parentNode);
      }
      emit _atomModel->layoutChanged();

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
  }

  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_object))
  {
    _bondListController->setTags();
  }

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
  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_object))
  {
    if(_atomModel->isActive(_iraspaStructure))
    {
      emit _atomModel->layoutAboutToBeChanged();
      for(const auto &[atomNode, indexPath] : _atomSelection.second)
      {
        std::shared_ptr<SKAtomTreeNode> parentNode = _atomTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
        whileBlocking(_atomModel)->insertRow(int(indexPath.lastIndex()), parentNode, atomNode);
      }
      emit _atomModel->layoutChanged();

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
  }

  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
  {
    if(_bondModel->isActive(_iraspaStructure))
    {
      emit _bondModel->layoutAboutToBeChanged();
      for(const auto &[bondItem, row] : _bondSelection)
      {
        whileBlocking(_bondModel)->insertRow(row, bondItem);
      }
      emit _bondModel->layoutChanged();

      _bondListController->setSelection(_bondSelection);
      emit _bondModel->updateSelection();
    }
    else
    {
      for(const auto &[bondItem, row] : _bondSelection)
      {
        std::dynamic_pointer_cast<Structure>(_iraspaStructure->object())->bondSetController()->insertBond(bondItem, row);
      }
    }
  }

  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_object))
  {
    _atomTreeController->setTags();
  }
  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_object))
  {
    _bondListController->setTags();
  }

  if(_mainWindow)
  {
    emit _mainWindow->invalidateCachedAmbientOcclusionTextures(_structures);
    emit _mainWindow->invalidateCachedIsoSurfaces(_structures);
    emit _mainWindow->rendererReloadData();

    _mainWindow->updateControlPanel();
    _mainWindow->documentWasModified();
  }
}
