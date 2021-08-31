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

#include "scenetreeviewdeleteselectioncommand.h"
#include "scenetreeviewdeletemoviesubcommand.h"
#include <QDebug>
#include <algorithm>

SceneTreeViewDeleteSelectionCommand::SceneTreeViewDeleteSelectionCommand(MainWindow *mainWindow,
                                                                         SceneTreeView *sceneTreeView, std::weak_ptr<FrameListViewModel> frameListViewModel,
                                                                         std::shared_ptr<ProjectStructure> projectStructure,
                                                                         std::shared_ptr<SceneList> sceneList,
                                                                         std::vector<std::shared_ptr<Movie>> movies,
                                                                         SceneListSelection selection,
                                                                         QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _sceneTreeView(sceneTreeView),
  _frameListViewModel(frameListViewModel),
  _projectStructure(projectStructure),
  _sceneList(sceneList),
  _movies(movies),
  _selection(selection)
{
  Q_UNUSED(undoParent);

  setText(QString("Delete movie selection"));

  for(const std::shared_ptr<Movie> &movie : _movies)
  {
    new SceneTreeViewDeleteMovieSubCommand(_mainWindow, _sceneTreeView, _sceneList, movie, this);
  }
}

void SceneTreeViewDeleteSelectionCommand::redo()
{
   QUndoCommand::redo();

   if(_mainWindow)
   {
     // after removing the selection, choose the first scene/movie as the new selection
     std::shared_ptr<Scene> newSelectedScene = _sceneList->scenes().empty() ? nullptr : _sceneList->scenes().front();
     std::shared_ptr<Movie> newSelectedMovie = newSelectedScene ? newSelectedScene->movies().front() : nullptr;
     _sceneList->setSelection({newSelectedScene, {newSelectedScene}, {{newSelectedScene, newSelectedMovie}}, {{newSelectedScene, {newSelectedMovie}}}});

     SKBoundingBox boundingBox = _projectStructure->renderBoundingBox();
     _projectStructure->camera()->resetForNewBoundingBox(boundingBox);

     _sceneTreeView->reloadSelection();
     _mainWindow->recheckRemovalButtons();

     emit _sceneTreeView->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());
     emit _sceneTreeView->rendererReloadData();

     if(std::shared_ptr<FrameListViewModel> model = _frameListViewModel.lock())
     {
       emit model->updateSelection();
     }

      _mainWindow->documentWasModified();
   }
}

void SceneTreeViewDeleteSelectionCommand::undo()
{
  // automatically peform the undo's
  QUndoCommand::undo();

  if(_mainWindow)
  {
    SKBoundingBox boundingBox = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(boundingBox);

    _sceneList->setSelection(_selection);
    _sceneTreeView->reloadSelection();
    _mainWindow->recheckRemovalButtons();

    emit _sceneTreeView->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());
    emit _sceneTreeView->rendererReloadData();

    if(std::shared_ptr<FrameListViewModel> model = _frameListViewModel.lock())
    {
      emit model->updateSelection();
    }

     _mainWindow->documentWasModified();
  }
}
