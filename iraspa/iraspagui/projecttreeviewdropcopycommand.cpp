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

#include "projecttreeviewdropcopycommand.h"
#include <QDebug>
#include <algorithm>

ProjectTreeViewDropCopyCommand::ProjectTreeViewDropCopyCommand(MainWindow *mainWindow, ProjectTreeViewModel *projectTreeViewModel, std::shared_ptr<ProjectTreeController> projectTreeController,
                                                               ProjectSelectionNodesAndIndexPaths nodes, ProjectSelectionIndexPaths selection, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _projectTreeViewModel(projectTreeViewModel),
  _projectTreeController(projectTreeController),
  _nodes(nodes),
  _selection(selection)
{
  Q_UNUSED(undoParent);

  setText(QString("Copy project(s)"));

  _reversedNodes.first = nodes.first;
  std::reverse_copy(nodes.second.begin(), nodes.second.end(), std::inserter(_reversedNodes.second, _reversedNodes.second.begin()));
}

void ProjectTreeViewDropCopyCommand::redo()
{
  if(std::shared_ptr<ProjectTreeController> projectTreeController = _projectTreeController.lock())
  {
    _projectTreeViewModel->layoutAboutToBeChanged();
    for(auto const &[projectTreeNode, indexPath] : _nodes.second)
    {
      int row = int(indexPath.lastIndex());
      std::shared_ptr<ProjectTreeNode> parentNode = projectTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
      whileBlocking(_projectTreeViewModel)->insertRow(row, parentNode, projectTreeNode);
    }
    _projectTreeViewModel->layoutChanged();

    projectTreeController->setSelectionIndexPaths(_nodes);
    _projectTreeViewModel->updateSelection();

    std::shared_ptr<ProjectTreeNode> selectedProject = projectTreeController->selectedTreeNode();
    if(selectedProject)
    {
      selectedProject->representedObject()->project()->setInitialSelectionIfNeeded();
    }
    _mainWindow->propagateProject(selectedProject, _mainWindow);

    _mainWindow->documentWasModified();
  }
}

void ProjectTreeViewDropCopyCommand::undo()
{
  if(std::shared_ptr<ProjectTreeController> projectTreeController = _projectTreeController.lock())
  {
    _projectTreeViewModel->layoutAboutToBeChanged();
    for(const auto &[projectTreeNode, indexPath] : _reversedNodes.second)
    {
      int row = int(indexPath.lastIndex());
      std::shared_ptr<ProjectTreeNode> parentNode = projectTreeController->nodeAtIndexPath(indexPath.removingLastIndex());
      whileBlocking(_projectTreeViewModel)->removeRow(row, parentNode);
    }
    _projectTreeViewModel->layoutChanged();

    projectTreeController->setSelectionIndexPaths(_selection);
    _projectTreeViewModel->updateSelection();

    std::shared_ptr<ProjectTreeNode> selectedProject = projectTreeController->selectedTreeNode();
    _mainWindow->propagateProject(selectedProject, _mainWindow);

    _mainWindow->documentWasModified();
  }
}
