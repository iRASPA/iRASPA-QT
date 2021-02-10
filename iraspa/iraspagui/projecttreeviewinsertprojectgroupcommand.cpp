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

#include "projecttreeviewinsertprojectgroupcommand.h"
#include <QDebug>
#include <algorithm>

ProjectTreeViewInsertProjectGroupCommand::ProjectTreeViewInsertProjectGroupCommand(MainWindow *mainWindow, ProjectTreeView *projectTreeView,
                                                                                   std::shared_ptr<ProjectTreeNode> parent, size_t row,
                                                                                   ProjectSelectionNodesAndIndexPaths selection,
                                                                                   QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _projectTreeView(projectTreeView),
  _row(row),
  _parent(parent),
  _selection(selection)
{
  setText(QString("Insert project group"));

  std::shared_ptr<ProjectGroup> project = std::make_shared<ProjectGroup>();
  std::shared_ptr<iRASPAProject>  iraspaproject = std::make_shared<iRASPAProject>(project);
  _newItem = std::make_shared<ProjectTreeNode>("New project group", iraspaproject, true, true);
}

void ProjectTreeViewInsertProjectGroupCommand::redo()
{
  if(_projectTreeView)
  {
    if(ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(_projectTreeView->model()))
    {
      pModel->insertRow(int(_row), _parent, _newItem);

      ProjectSelectionIndexPaths newSelection = _projectTreeView->projectTreeController()->updateIndexPathsFromNodes(_selection);
      pModel->projectTreeController()->setSelectionIndexPaths(newSelection);
    }
    _projectTreeView->reloadSelection();

    _mainWindow->documentWasModified();
  }
}

void ProjectTreeViewInsertProjectGroupCommand::undo()
{
  if(_projectTreeView)
  {
    if(ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(_projectTreeView->model()))
    {
      pModel->removeRow(int(_row), _parent);

      pModel->projectTreeController()->setSelectionIndexPaths(_selection);
    }

    _projectTreeView->reloadSelection();
  }

  if(_mainWindow)
  {
    _mainWindow->reloadSelectionDetailViews();

    _mainWindow->documentWasModified();
  }
}
