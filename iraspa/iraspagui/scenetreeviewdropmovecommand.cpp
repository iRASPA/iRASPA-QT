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

#include "scenetreeviewdropmovecommand.h"
#include "scenetreeviewdraganddropinsertscenesubcommand.h"
#include "scenetreeviewdraganddropmovemoviesubcommand.h"
#include <QDebug>
#include <algorithm>

SceneTreeViewDropMoveCommand::SceneTreeViewDropMoveCommand(MainWindow *mainWindow, SceneTreeViewModel *sceneTreeViewModel, std::weak_ptr<FrameListViewModel> frameListViewModel,
                                                           std::shared_ptr<SceneList> sceneList, std::shared_ptr<Scene> parent, int row,
                                                           std::vector<std::shared_ptr<Movie>> movies, SceneListSelection selection, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _sceneTreeViewModel(sceneTreeViewModel),
  _frameListViewModel(frameListViewModel),
  _sceneList(sceneList),
  _parent(parent),
  _row(row),
  _movies(movies),
  _selection(selection)
{
  Q_UNUSED(undoParent);

  setText(QString("Reorder movies"));

  int beginRow = row;
  _newParent = _parent;

  if(!_newParent)
  {
    std::shared_ptr<Scene> scene = std::make_shared<Scene>("NEW SCENE");
    size_t insertionRow = _sceneList->scenes().size();
    new SceneTreeViewDragAndDropInsertSceneSubCommand(_sceneTreeViewModel, scene, insertionRow, this);

    _newParent = scene;
    beginRow = 0;
  }

  for(const std::shared_ptr<Movie> &movie : _movies)
  {
    if(_newParent == _sceneTreeViewModel->parentForMovie(movie) && *_newParent->findChildIndex(movie) < beginRow)
    {
      beginRow--;
    }

    new SceneTreeViewDragAndDropMoveMovieSubCommand(_sceneTreeViewModel, _sceneList, _newParent, movie, beginRow, this);
    beginRow += 1;
  }
}

void SceneTreeViewDropMoveCommand::redo()
{
  QUndoCommand::redo();

  emit _sceneTreeViewModel->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());

  _sceneList->setSelection(SceneListSelection(_newParent,
                                              {_newParent},
                                              {{_newParent, _movies.front()}},
                                              {{_newParent, std::set<std::shared_ptr<Movie>>(_movies.begin(),_movies.end())}} ));
  emit _sceneTreeViewModel->updateSelection();



  if(std::shared_ptr<FrameListViewModel> model = _frameListViewModel.lock())
  {
    emit model->updateSelection();
  }

  emit _sceneTreeViewModel->rendererReloadData();

   _mainWindow->documentWasModified();
}

void SceneTreeViewDropMoveCommand::undo()
{
  QUndoCommand::undo();

  emit _sceneTreeViewModel->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());

  _sceneList->setSelection(_selection);
  emit _sceneTreeViewModel->updateSelection();

  if(std::shared_ptr<FrameListViewModel> model = _frameListViewModel.lock())
  {
    emit model->updateSelection();
  }

  emit _sceneTreeViewModel->rendererReloadData();

   _mainWindow->documentWasModified();
}
