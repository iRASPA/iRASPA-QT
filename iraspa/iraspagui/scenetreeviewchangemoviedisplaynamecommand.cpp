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

#include "scenetreeviewchangemoviedisplaynamecommand.h"
#include <QDebug>
#include <algorithm>

SceneTreeViewChangeMovieDisplayNameCommand::SceneTreeViewChangeMovieDisplayNameCommand(MainWindow *mainWindow, SceneTreeViewModel *sceneModel, std::shared_ptr<Movie> movie,
                                                                                       QString newValue, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _sceneModel(sceneModel),
  _movie(movie),
  _newValue(newValue)
{
  setText(QString("Change atom name"));
}

void SceneTreeViewChangeMovieDisplayNameCommand::redo()
{
  _oldValue = _movie->displayName();
  _movie->setDisplayName(_newValue);

  if(_sceneModel)
  {
    QModelIndex index = _sceneModel->indexForItem(_movie);
    emit _sceneModel->dataChanged(index,index);

    _mainWindow->documentWasModified();
  }
}

void SceneTreeViewChangeMovieDisplayNameCommand::undo()
{
  _movie->setDisplayName(_oldValue);

  if(_sceneModel)
  {
    QModelIndex index = _sceneModel->indexForItem(_movie);
    emit _sceneModel->dataChanged(index,index);

    _mainWindow->documentWasModified();
  }
}
