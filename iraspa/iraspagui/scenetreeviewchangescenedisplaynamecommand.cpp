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

#include "scenetreeviewchangescenedisplaynamecommand.h"
#include <QDebug>
#include <algorithm>

SceneTreeViewChangeSceneDisplayNameCommand::SceneTreeViewChangeSceneDisplayNameCommand(MainWindow *mainWindow, SceneTreeViewModel *sceneModel, std::shared_ptr<Scene> scene,
                                                                                       QString newValue, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _sceneModel(sceneModel),
  _scene(scene),
  _newValue(newValue)
{
  setText(QString("Change scene name"));
}

void SceneTreeViewChangeSceneDisplayNameCommand::redo()
{
  _oldValue = _scene->displayName();
  _scene->setDisplayName(_newValue);

  if(_sceneModel)
  {
    QModelIndex index = _sceneModel->indexForItem(_scene);
    emit _sceneModel->dataChanged(index,index);

    _mainWindow->documentWasModified();
  }
}

void SceneTreeViewChangeSceneDisplayNameCommand::undo()
{
  _scene->setDisplayName(_oldValue);

  if(_sceneModel)
  {
    QModelIndex index = _sceneModel->indexForItem(_scene);
    emit _sceneModel->dataChanged(index,index);

    _mainWindow->documentWasModified();
  }
}
