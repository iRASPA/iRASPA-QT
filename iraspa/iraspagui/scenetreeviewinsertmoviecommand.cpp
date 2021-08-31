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

#include "scenetreeviewinsertmoviecommand.h"
#include <QDebug>
#include <algorithm>

SceneTreeViewInsertMovieCommand::SceneTreeViewInsertMovieCommand(MainWindow *mainWindow, SceneTreeView *sceneTreeView,
                                                                 std::weak_ptr<FrameListViewModel> frameListViewModel,
                                                                 std::shared_ptr<ProjectStructure> projectStructure,
                                                                 std::shared_ptr<SceneList> sceneList, std::shared_ptr<Movie> movie,
                                                                 std::shared_ptr<Scene> parent, int row,
                                                                 SceneListSelection selection, QUndoCommand *undoParent):
  _mainWindow(mainWindow),
  _sceneTreeView(sceneTreeView),
  _frameListViewModel(frameListViewModel),
  _sceneList(sceneList),
  _projectStructure(projectStructure),
  _movie(movie),
  _row(row),
  _parent(parent),
  _selection(selection)
{
  Q_UNUSED(undoParent);

  setText(QString("Insert movie"));
}

void SceneTreeViewInsertMovieCommand::redo()
{
  if(_sceneTreeView)
  {
    if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(_sceneTreeView->model()))
    {
      _sceneList->setSelection(_selection);
      _sceneTreeView->reloadSelection();
      pModel->insertRow(_row, _parent, _movie);
    }
  }

  if(_mainWindow)
  {
    SKBoundingBox boundingBox = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(boundingBox);
    if(_sceneTreeView)
      _sceneTreeView->reloadSelection();
    _mainWindow->recheckRemovalButtons();

    _mainWindow->documentWasModified();
  }

  if(std::shared_ptr<FrameListViewModel> model = _frameListViewModel.lock())
  {
    emit model->updateSelection();
  }
}

void SceneTreeViewInsertMovieCommand::undo()
{
  if(_sceneTreeView)
  {
    if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(_sceneTreeView->model()))
    {
      _sceneList->setSelection(_selection);
      _sceneTreeView->reloadSelection();
      pModel->removeRow(_row, _parent, _movie);
    }
  }

  if(_mainWindow)
  {
    SKBoundingBox boundingBox = _projectStructure->renderBoundingBox();
    if(_projectStructure)
      _projectStructure->camera()->resetForNewBoundingBox(boundingBox);
    if(_sceneTreeView)
      _sceneTreeView->reloadSelection();
    _mainWindow->recheckRemovalButtons();

    _mainWindow->documentWasModified();
  }

  if(std::shared_ptr<FrameListViewModel> model = _frameListViewModel.lock())
  {
    emit model->updateSelection();
  }
}
