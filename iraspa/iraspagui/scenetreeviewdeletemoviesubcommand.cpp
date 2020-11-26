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

#include "scenetreeviewdeletemoviesubcommand.h"
#include <QDebug>
#include <algorithm>

SceneTreeViewDeleteMovieSubCommand::SceneTreeViewDeleteMovieSubCommand(MainWindow *mainWindow, SceneTreeView *sceneTreeView, std::shared_ptr<SceneList> sceneList, std::shared_ptr<Movie> movie, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _sceneTreeView(sceneTreeView),
  _sceneList(sceneList),
  _movie(movie)

{
  setText(QString("Delete movie"));
}

void SceneTreeViewDeleteMovieSubCommand::redo()
{
  if(_sceneTreeView)
  {
    if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(_sceneTreeView->model()))
    {
      _scene = pModel->parentForMovie(_movie);
      std::optional<int> row = _scene->findChildIndex(_movie);
      if(row)
      {
        _row = *row;
        pModel->removeRow(_row, _scene, _movie);
      }

      _sceneRow = std::nullopt;
      if(_scene->movies().empty())
      {
        _sceneRow = _sceneList->findChildIndex(_scene);
        if(_sceneRow)
        {
          pModel->removeRow(*_sceneRow);
        }
      }
    }

    emit _sceneTreeView->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());
    emit _sceneTreeView->rendererReloadData();
  }
}

void SceneTreeViewDeleteMovieSubCommand::undo()
{
  if(_sceneTreeView)
  {
    if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(_sceneTreeView->model()))
    {
      if(_sceneRow)
      {
        pModel->insertRow(*_sceneRow, _scene);
        _sceneTreeView->expandAll();
      }

      pModel->insertRow(_row, _scene, _movie);
    }

    emit _sceneTreeView->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());
    emit _sceneTreeView->rendererReloadData();
  }
}
