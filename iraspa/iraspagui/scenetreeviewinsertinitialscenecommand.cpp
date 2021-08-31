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

#include "scenetreeviewinsertinitialscenecommand.h"
#include <QDebug>
#include <algorithm>

SceneTreeViewInsertInitialSceneCommand::SceneTreeViewInsertInitialSceneCommand(MainWindow *mainWindow, SceneTreeView *sceneTreeView, std::shared_ptr<SceneList> sceneList,
                                                                               std::shared_ptr<Scene> scene, std::shared_ptr<Movie> movie,
                                                                               int row, QUndoCommand *undoParent):
  _mainWindow(mainWindow),
  _sceneTreeView(sceneTreeView),
  _sceneList(sceneList),
  _scene(scene),
  _movie(movie),
  _row(row)
{
  Q_UNUSED(undoParent);

  setText(QString("Insert scene/movie"));
}

void SceneTreeViewInsertInitialSceneCommand::redo()
{
  if(_sceneTreeView)
  {
    if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(_sceneTreeView->model()))
    {
      pModel->insertRow(0, _scene);
      _sceneTreeView->expandAll();
      _sceneList->setSelection(SceneListSelection(_scene, {_scene}, {{_scene, _movie}}, {{_scene, {_movie}}}));
    }
  }

  if(_mainWindow)
  {
    if(_sceneTreeView)
      _sceneTreeView->reloadSelection();
    _mainWindow->recheckRemovalButtons();

    _mainWindow->documentWasModified();
  }
}

void SceneTreeViewInsertInitialSceneCommand::undo()
{
  if(_sceneTreeView)
  {
    if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(_sceneTreeView->model()))
    {
      pModel->removeRow(0);
      _sceneList->setSelection(SceneListSelection(nullptr, {}, {}, {}));
    }
  }

  if(_mainWindow)
  {
    if(_sceneTreeView)
      _sceneTreeView->reloadSelection();
    _mainWindow->recheckRemovalButtons();

    _mainWindow->documentWasModified();
  }
}
