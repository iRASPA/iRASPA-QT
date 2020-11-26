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

#include "scenetreeviewpasteprojectscommand.h"
#include <QDebug>
#include <algorithm>

SceneTreeViewPasteProjectsCommand::SceneTreeViewPasteProjectsCommand(MainWindow *mainWindow, SceneTreeViewModel *sceneTreeViewModel,
                                                                     std::shared_ptr<ProjectStructure> projectStructure, SceneList *sceneList,
                                                                     DisplayableProtocol *parentNode, int row, std::vector<std::shared_ptr<Movie>> nodes,
                                                                     SceneListSelection oldSelection, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _sceneTreeViewModel(sceneTreeViewModel),
  _projectStructure(projectStructure),
  _sceneList(sceneList),
  _row(row),
  _nodes(nodes),
  _oldSelection(oldSelection),
  _sceneParent(nullptr),
  _sceneListParent(nullptr)
{
  Q_UNUSED(undoParent);

  setText(QString("Copy project(s)"));

  if(Scene *scene = dynamic_cast<Scene *>(parentNode))
  {
    _sceneParent = scene->shared_from_this();
    _newSelection = oldSelection;
  }

  if(SceneList *sceneList = dynamic_cast<SceneList *>(parentNode))
  {
    _sceneListParent = sceneList->shared_from_this();
    _row = 0;
    _sceneParent = std::make_shared<Scene>("New scene");
    _sceneParent->setSelectedMovie(nodes.front());

    _newSelection = SceneListSelection(_sceneParent, {_sceneParent}, {{_sceneParent, nodes.front()}}, {{_sceneParent, {nodes.front()}}});
  }
}

void SceneTreeViewPasteProjectsCommand::redo()
{
  int row = _row;
  _sceneTreeViewModel->layoutAboutToBeChanged();

  if(_sceneListParent)
  {
    _sceneTreeViewModel->insertRow(0, _sceneParent);
    emit _sceneTreeViewModel->expandAll();
  }


  for(std::shared_ptr<Movie> node : _nodes)
  {
    whileBlocking(_sceneTreeViewModel)->insertRow(row, _sceneParent, node);
    row += 1;
  }
  _sceneTreeViewModel->layoutChanged();

  if(_mainWindow)
  {
    SKBoundingBox boundingBox = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(boundingBox);

    _sceneList->setSelection(_newSelection);
    _sceneTreeViewModel->updateSelection();

    _mainWindow->reloadSelectionDetailViews();
    _mainWindow->rendererReloadData();
    _mainWindow->recheckRemovalButtons();

    _mainWindow->documentWasModified();
  }
}

void SceneTreeViewPasteProjectsCommand::undo()
{
  _sceneTreeViewModel->layoutAboutToBeChanged();

  if(_sceneListParent)
  {
    _sceneTreeViewModel->removeRow(0);
  }

  for(std::shared_ptr<Movie> node : _nodes)
  {
    whileBlocking(_sceneTreeViewModel)->removeRow(_row, _sceneParent, node);
  }
  _sceneTreeViewModel->layoutChanged();

  if(_mainWindow)
  {
    SKBoundingBox boundingBox = _projectStructure->renderBoundingBox();
    _projectStructure->camera()->resetForNewBoundingBox(boundingBox);

    _sceneList->setSelection(_oldSelection);
    _sceneTreeViewModel->updateSelection();

    _mainWindow->reloadSelectionDetailViews();
    _mainWindow->rendererReloadData();
    _mainWindow->recheckRemovalButtons();

    _mainWindow->documentWasModified();
  }
}
