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

#include "projecttreeviewpasteprojectscommand.h"
#include <QDebug>
#include <algorithm>

ProjectTreeViewPasteProjectsCommand::ProjectTreeViewPasteProjectsCommand(MainWindow *mainWindow, ProjectTreeViewModel *projectTreeViewModel, std::shared_ptr<ProjectTreeNode> parentNode, int row,
                                                               std::vector<std::shared_ptr<ProjectTreeNode>> nodes, ProjectSelectionIndexPaths selection, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _projectTreeViewModel(projectTreeViewModel),
  _parentNode(parentNode),
  _row(row),
  _nodes(nodes),
  _selection(selection)
{
  Q_UNUSED(undoParent);

  setText(QString("Copy project(s)"));
}

void ProjectTreeViewPasteProjectsCommand::redo()
{
  qDebug() << "PASTING MOVIE";
  int row = _row;
  _projectTreeViewModel->layoutAboutToBeChanged();
  for(std::shared_ptr<ProjectTreeNode> node : _nodes)
  {
    whileBlocking(_projectTreeViewModel)->insertRow(row, _parentNode, node);
    row += 1;
  }
  _projectTreeViewModel->layoutChanged();

  _mainWindow->documentWasModified();
}

void ProjectTreeViewPasteProjectsCommand::undo()
{
  int row = _row;
  _projectTreeViewModel->layoutAboutToBeChanged();
  for(std::shared_ptr<ProjectTreeNode> node : _nodes)
  {
    whileBlocking(_projectTreeViewModel)->removeRow(row, _parentNode);
  }
  _projectTreeViewModel->layoutChanged();

  _mainWindow->documentWasModified();
}
