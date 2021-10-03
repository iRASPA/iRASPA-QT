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

#include "atomtreeviewcopyselectiontonewmoviecommand.h"
#include <QDebug>
#include <algorithm>

AtomTreeViewCopySelectionToNewMovieCommand::AtomTreeViewCopySelectionToNewMovieCommand(MainWindow *mainWindow,
                                                                                       AtomTreeViewModel* atomTreeViewModel,
                                                                                       SceneTreeViewModel* sceneTreeViewModel,
                                                                                       std::shared_ptr<SceneList> sceneList,
                                                                                       std::shared_ptr<iRASPAObject> iraspaStructure,
                                                                                       AtomSelectionIndexPaths atomSelection,
                                                                                       BondSelectionIndexSet bondSelection,
                                                                                       QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _atomTreeViewModel(atomTreeViewModel),
  _sceneTreeViewModel(sceneTreeViewModel),
  _sceneList(sceneList),
  _iraspaStructure(iraspaStructure),
  _atomSelection(atomSelection),
  _bondSelection(bondSelection),
  _scene(nullptr),
  _newMovie(nullptr),
  _row(0)
{
  setText(QString("Copy atoms to new movie"));
}

void AtomTreeViewCopySelectionToNewMovieCommand::redo()
{
  if(std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(_iraspaStructure->object()))
  {
    if(std::shared_ptr<Movie> movie = _iraspaStructure->parent().lock())
    {
      if((_scene = movie->parent().lock()))
      {
        _row = int(_scene->movies().size());

        std::shared_ptr<iRASPAObject> newiRASPAStructure = _iraspaStructure->clone();
        if(std::shared_ptr<Structure> newStructure = std::dynamic_pointer_cast<Structure>(newiRASPAStructure->object()))
        {
          newStructure->setSpaceGroupHallNumber(structure->spaceGroup().spaceGroupSetting().HallNumber());
          _newMovie = Movie::create(newiRASPAStructure);

          for(const IndexPath &indexPath : _atomSelection.second)
          {
            const std::shared_ptr<SKAtomTreeNode> atomTreeNode = structure->atomsTreeController()->nodeAtIndexPath(indexPath);
            if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = atomTreeNode->representedObject())
            {
              std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
              std::shared_ptr<SKAtomTreeNode> newAtomTreeNode = std::make_shared<SKAtomTreeNode>(newAsymmetricAtom);
              newStructure->atomsTreeController()->appendToRootnodes(newAtomTreeNode);
            }

          }
          newStructure->expandSymmetry();
          newStructure->reComputeBoundingBox();
          newStructure->computeBonds();
          newStructure->atomsTreeController()->setTags();
          newStructure->bondSetController()->setTags();

          _sceneTreeViewModel->insertRow(_row, _scene, _newMovie);

          structure->atomsTreeController()->setSelectionIndexPaths(_atomSelection);
          structure->bondSetController()->setSelectionIndexSet(_bondSelection);
        }
      }

      emit _sceneTreeViewModel->rendererReloadData();

      _mainWindow->documentWasModified();
    }
  }
}

void AtomTreeViewCopySelectionToNewMovieCommand::undo()
{
  _sceneTreeViewModel->removeRow(_row, _scene, _newMovie);
  emit _sceneTreeViewModel->rendererReloadData();

  _mainWindow->documentWasModified();
}
