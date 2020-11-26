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

#include "projecttreeviewdeleteselectioncommand.h"
#include <QDebug>
#include <algorithm>

ProjectTreeViewDeleteSelectionCommand::ProjectTreeViewDeleteSelectionCommand(MainWindow *mainWindow, ProjectTreeViewModel *model,
                                                                             std::shared_ptr<ProjectTreeController> projectTreeController,
                                                                             ProjectSelectionNodesAndIndexPaths deletedSelection,
                                                                             ProjectSelectionNodesAndIndexPaths selection, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _model(model),
  _projectTreeController(projectTreeController),
  _deletedSelection(deletedSelection),
  _selection(selection)
{
  Q_UNUSED(parent);

  setText(QString("Delete selected projects"));

  _reversedDeletedSelection.first = _deletedSelection.first;
  std::reverse_copy(_deletedSelection.second.begin(), _deletedSelection.second.end(), std::inserter(_reversedDeletedSelection.second, _reversedDeletedSelection.second.begin()));
}

void ProjectTreeViewDeleteSelectionCommand::redo()
{
  _model->layoutAboutToBeChanged();
  for(const auto [projectNode, indexPath] : _reversedDeletedSelection.second)
  {
    int row = indexPath.lastIndex();
    std::shared_ptr<ProjectTreeNode> parentNode = _projectTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
    whileBlocking(_model)->removeRow(row, parentNode);
  }
  _model->layoutChanged();

  _projectTreeController->clearSelection();

  _mainWindow->propagateProject(_projectTreeController->selectedTreeNode(), _mainWindow);

  emit _model->updateSelection();

  _mainWindow->documentWasModified();
}

void ProjectTreeViewDeleteSelectionCommand::undo()
{
  _model->layoutAboutToBeChanged();
  for(const auto [projectNode, indexPath] : _deletedSelection.second)
  {
    std::shared_ptr<ProjectTreeNode> parentNode = _projectTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
    whileBlocking(_model)->insertRow(indexPath.lastIndex(), parentNode, projectNode);
  }
  _model->layoutChanged();

  _projectTreeController->setSelectionIndexPaths(_selection);
  emit _model->updateSelection();

  std::shared_ptr<ProjectTreeNode> selectedProject = _projectTreeController->selectedTreeNode();
  _mainWindow->propagateProject(selectedProject, _mainWindow);

  _mainWindow->documentWasModified();
}
