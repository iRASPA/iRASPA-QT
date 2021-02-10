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

#pragma once

#include <QUndoCommand>
#include <set>
#include <vector>
#include "iraspakit.h"
#include "indexpath.h"
#include "symmetrykit.h"
#include "mathkit.h"
#include "mainwindow.h"
#include "projecttreeview.h"

class ProjectTreeViewInsertProjectGroupCommand : public QUndoCommand
{
public:
  ProjectTreeViewInsertProjectGroupCommand(MainWindow *mainWindow, ProjectTreeView *projectTreeView, std::shared_ptr<ProjectTreeNode> parent, size_t row,
                         ProjectSelectionNodesAndIndexPaths selection, QUndoCommand *undoParent = nullptr);
  void undo() override final;
  void redo() override final;
private:
  MainWindow *_mainWindow;
  ProjectTreeView *_projectTreeView;
  size_t _row;
  std::shared_ptr<ProjectTreeNode> _parent;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  ProjectSelectionNodesAndIndexPaths _selection;
  std::shared_ptr<ProjectTreeNode> _newItem;
};
