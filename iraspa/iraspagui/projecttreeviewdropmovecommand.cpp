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

#include "projecttreeviewdropmovecommand.h"
#include <QDebug>
#include <algorithm>

ProjectTreeViewDropMoveCommand::ProjectTreeViewDropMoveCommand(MainWindow *mainWindow, ProjectTreeViewModel *projectTreeViewModel, ProjectTreeController *projectTreeController,
                                                               std::vector<std::tuple<std::shared_ptr<ProjectTreeNode>, std::shared_ptr<ProjectTreeNode>, int, bool>> nodes,
                                                               ProjectSelectionIndexPaths oldSelection, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _projectTreeViewModel(projectTreeViewModel),
  _projectTreeController(projectTreeController),
  _nodes(nodes),
  _oldSelection(oldSelection),
  _newSelection({})
{
  Q_UNUSED(undoParent);

  setText(QString("Reorder project(s)"));
}

void ProjectTreeViewDropMoveCommand::redo()
{
  _reverseMoves.clear();
  _newSelection.second.clear();

  _projectTreeViewModel->layoutAboutToBeChanged();
  for(const auto &[projectTreeNode, parentNode, insertionRow, moved] : _nodes)
  {
    _reverseMoves.insert(_reverseMoves.begin(), std::make_tuple(projectTreeNode, projectTreeNode->parent(), projectTreeNode->row(), moved));

    if(moved)
    {
      whileBlocking(_projectTreeViewModel)->removeRow(projectTreeNode->row(), projectTreeNode->parent());
    }

    // workaround qt-bug (the branch is not shown for parents with no children unless it is explicitely set collapsed)
    if(parentNode->childNodes().empty())
    {
      QModelIndex parentIndex = _projectTreeViewModel->indexForNode(parentNode.get());
      emit _projectTreeViewModel->expand(parentIndex);
    }

    whileBlocking(_projectTreeViewModel)->insertRow(insertionRow, parentNode, projectTreeNode);
  }
  _projectTreeViewModel->layoutChanged();

  // update selection of moved nodes _after_ all is moved
  // (indexPaths have been changed, including the indexPath of the parentNode)
  for(const auto &[projectTreeNode, parentNode, insertionRow, moved] : _nodes)
  {
    _newSelection.second.insert(projectTreeNode->indexPath());
  }

  std::shared_ptr<ProjectTreeNode> selectedProject = std::get<0>(_nodes.front());
  _newSelection.first = selectedProject ? selectedProject->indexPath() : IndexPath();

  if(selectedProject)
  {
    selectedProject->representedObject()->project()->setInitialSelectionIfNeeded();
  }

  _projectTreeViewModel->projectTreeController()->setSelectionIndexPaths(_newSelection);
  emit _projectTreeViewModel->updateSelection();

  _mainWindow->propagateProject(selectedProject, _mainWindow);

  _mainWindow->documentWasModified();
}

void ProjectTreeViewDropMoveCommand::undo()
{
  _projectTreeViewModel->layoutAboutToBeChanged();
  for(const auto &[projectTreeNode, parentNode, insertionRow, moved] : _reverseMoves)
  {
    whileBlocking(_projectTreeViewModel)->removeRow(projectTreeNode->row(), projectTreeNode->parent());

    // workaround qt-bug (the branch is not shown for parents with no children unless it is explicitely set collapsed)
    if(projectTreeNode->parent()->childNodes().empty())
    {
      QModelIndex parentIndex = _projectTreeViewModel->indexForNode(projectTreeNode->parent().get());
      emit _projectTreeViewModel->collapse(parentIndex);
    }

    if(moved)
    {
      whileBlocking(_projectTreeViewModel)->insertRow(insertionRow, parentNode, projectTreeNode);
    }
  }
  _projectTreeViewModel->layoutChanged();

  std::shared_ptr<ProjectTreeNode> selectedProject = _projectTreeController->nodeAtIndexPath(_oldSelection.first);
  if(selectedProject)
  {
    selectedProject->representedObject()->project()->setInitialSelectionIfNeeded();
  }

  _projectTreeViewModel->projectTreeController()->setSelectionIndexPaths(_oldSelection);
  emit _projectTreeViewModel->updateSelection();

  _mainWindow->propagateProject(selectedProject, _mainWindow);

  _mainWindow->documentWasModified();
}
