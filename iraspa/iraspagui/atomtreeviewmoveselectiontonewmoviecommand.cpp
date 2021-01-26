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

#include "atomtreeviewmoveselectiontonewmoviecommand.h"
#include <QDebug>
#include <algorithm>

AtomTreeViewMoveSelectionToNewMovieCommand::AtomTreeViewMoveSelectionToNewMovieCommand(MainWindow *mainWindow,
                                                                                       AtomTreeViewModel *atomTreeViewModel,
                                                                                       BondListViewModel *bondListViewModel,
                                                                                       SceneTreeViewModel *sceneTreeViewModel,
                                                                                       std::shared_ptr<SceneList> sceneList,
                                                                                       std::shared_ptr<iRASPAStructure> iraspaStructure,
                                                                                       AtomSelectionNodesAndIndexPaths atomSelection,
                                                                                       BondSelectionNodesAndIndexSet bondSelection,
                                                                                       QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _atomTreeViewModel(atomTreeViewModel),
  _bondListViewModel(bondListViewModel),
  _sceneTreeViewModel(sceneTreeViewModel),
  _sceneList(sceneList),
  _iraspaStructure(iraspaStructure),
  _atomTreeController(iraspaStructure->structure()->atomsTreeController()),
  _bondSetController(iraspaStructure->structure()->bondSetController()),
  _atomSelection(atomSelection),
  _reversedAtomSelection({}),
  _bondSelection(bondSelection),
  _sceneSelection(sceneList->allSelection()),
  _scene(nullptr),
  _newMovie(nullptr),
  _row(0)
{
  setText(QString("Move atoms to new movie"));

  _reversedAtomSelection.first = _atomSelection.first;
  std::reverse_copy(_atomSelection.second.begin(), _atomSelection.second.end(), std::inserter(_reversedAtomSelection.second, _reversedAtomSelection.second.begin()));

  std::reverse_copy(bondSelection.begin(), bondSelection.end(), std::back_inserter(_reverseBondSelection));
}

void AtomTreeViewMoveSelectionToNewMovieCommand::redo()
{
  if(std::shared_ptr<Movie> movie = _iraspaStructure->parent().lock())
  {
    if((_scene = movie->parent().lock()))
    {
      _row = _scene->movies().size();

      std::shared_ptr<iRASPAStructure> newiRASPAStructure = _iraspaStructure->clone();
      newiRASPAStructure->structure()->setSpaceGroupHallNumber(_iraspaStructure->structure()->spaceGroup().spaceGroupSetting().HallNumber());
      std::shared_ptr<Movie> _newMovie = Movie::create(newiRASPAStructure);

      for(const auto &[atomNode, indexPath] : _atomSelection.second)
      {
        const std::shared_ptr<SKAtomTreeNode> atomTreeNode = _atomTreeController->nodeAtIndexPath(indexPath);
        if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = atomTreeNode->representedObject())
        {
          std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
          std::shared_ptr<SKAtomTreeNode> newAtomTreeNode = std::make_shared<SKAtomTreeNode>(newAsymmetricAtom);
          newiRASPAStructure->structure()->atomsTreeController()->appendToRootnodes(newAtomTreeNode);
        }

      }
      newiRASPAStructure->structure()->expandSymmetry();
      newiRASPAStructure->structure()->reComputeBoundingBox();
      newiRASPAStructure->structure()->computeBonds();
      newiRASPAStructure->structure()->atomsTreeController()->setTags();
      newiRASPAStructure->structure()->bondSetController()->setTags();

      _sceneTreeViewModel->insertRow(_row, _scene, _newMovie);
      _sceneList->setSelection(_sceneSelection);

      // delete selection
      // ======================

      if(_bondListViewModel->isActive(_iraspaStructure))
      {
        _bondListViewModel->layoutAboutToBeChanged();
        for(const auto &[bondItem, row] : _reverseBondSelection)
        {
          whileBlocking(_bondListViewModel)->removeRow(row);
        }
        _bondListViewModel->layoutChanged();

        _bondSetController->setSelection(BondSelectionNodesAndIndexSet());
        emit _bondListViewModel->updateSelection();
      }
      else
      {
        for(const auto &[bondItem, row] : _reverseBondSelection)
        {
          _bondSetController->removeBond(row);
        }
      }


      if(_atomTreeViewModel->isActive(_iraspaStructure))
      {
        _atomTreeViewModel->layoutAboutToBeChanged();
        for(const auto &[atomNode, indexPath] : _reversedAtomSelection.second)
        {
          int row = indexPath.lastIndex();
          std::shared_ptr<SKAtomTreeNode> parentNode = _atomTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
          whileBlocking(_atomTreeViewModel)->removeRow(row, parentNode);
        }
        _atomTreeViewModel->layoutChanged();

        _atomTreeController->clearSelection();
        emit _atomTreeViewModel->updateSelection();
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
      _bondSetController->setTags();

      emit _mainWindow->rendererReloadData();

      emit _sceneTreeViewModel->updateSelection();

      _mainWindow->documentWasModified();
    }
  }
}

void AtomTreeViewMoveSelectionToNewMovieCommand::undo()
{
  _sceneTreeViewModel->removeRow(_row, _scene, _newMovie);

  // insert selection
  //=====================================

  if(_atomTreeViewModel->isActive(_iraspaStructure))
  {
    _atomTreeViewModel->layoutAboutToBeChanged();
    for(const auto &[atomNode, indexPath] : _atomSelection.second)
    {
      std::shared_ptr<SKAtomTreeNode> parentNode = _atomTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
      whileBlocking(_atomTreeViewModel)->insertRow(indexPath.lastIndex(), parentNode, atomNode);
    }
    _atomTreeViewModel->layoutChanged();

    _atomTreeController->setSelectionIndexPaths(_atomSelection);
    emit _atomTreeViewModel->updateSelection();
  }
  else
  {
    for(const auto &[atomNode, indexPath] : _atomSelection.second)
    {
      int row = indexPath.lastIndex();
      std::shared_ptr<SKAtomTreeNode> parentNode = _atomTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
      parentNode->insertChild(row, atomNode);
    }
    _atomTreeController->setSelectionIndexPaths(_atomSelection);
  }

  if(_bondListViewModel->isActive(_iraspaStructure))
  {
    _bondListViewModel->layoutAboutToBeChanged();
    for(const auto &[bondItem, row] : _bondSelection)
    {
      whileBlocking(_bondListViewModel)->insertRow(row, bondItem);
    }
    _bondListViewModel->layoutChanged();

    _bondSetController->setSelection(_bondSelection);
    emit _bondListViewModel->updateSelection();
  }
  else
  {
    for(const auto &[bondItem, row] : _bondSelection)
    {
      _iraspaStructure->structure()->bondSetController()->insertBond(bondItem, row);
    }
  }

  _atomTreeController->setTags();
  _bondSetController->setTags();


  emit _atomTreeViewModel->rendererReloadData();
  _sceneList->setSelection(_sceneSelection);
  emit _sceneTreeViewModel->updateSelection();

  _mainWindow->documentWasModified();
}
