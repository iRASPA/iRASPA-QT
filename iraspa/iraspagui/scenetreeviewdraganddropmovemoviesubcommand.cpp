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

#include "scenetreeviewdraganddropmovemoviesubcommand.h"
#include <QDebug>
#include <algorithm>

SceneTreeViewDragAndDropMoveMovieSubCommand::SceneTreeViewDragAndDropMoveMovieSubCommand(SceneTreeViewModel *sceneTreeViewModel, std::shared_ptr<SceneList> sceneList, std::shared_ptr<Scene> parent,
                                                                                         std::shared_ptr<Movie> movie, int row, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _sceneTreeViewModel(sceneTreeViewModel),
  _sceneList(sceneList),
  _parent(parent),
  _movie(movie),
  _row(row),
  _movieRow(std::nullopt),
  _sceneRow(std::nullopt)
{
  Q_UNUSED(undoParent);

  setText(QString("Move movie(s)"));
}

void SceneTreeViewDragAndDropMoveMovieSubCommand::redo()
{
  _movieParent = _sceneTreeViewModel->parentForMovie(_movie);
  _movieRow = _movieParent->findChildIndex(_movie);
  if(_movieRow)
  {
    _sceneTreeViewModel->removeRow(*_movieRow, _movieParent, _movie);
    if(_movieParent->movies().empty())
    {
      // remove _movieParent if it would become empty
      if((_sceneRow = _sceneList->findChildIndex(_movieParent)))
      {
        _sceneTreeViewModel->removeRow(*_sceneRow);
      }
    }
  }
  _sceneTreeViewModel->insertRow(_row, _parent, _movie);
}

void SceneTreeViewDragAndDropMoveMovieSubCommand::undo()
{
  _sceneTreeViewModel->removeRow(_row, _parent, _movie);

  if(_sceneRow)
  {
    _sceneTreeViewModel->insertRow(*_sceneRow, _movieParent);
  }

  if(_movieRow)
  {
    _sceneTreeViewModel->insertRow(*_movieRow, _movieParent, _movie);
  }
}
