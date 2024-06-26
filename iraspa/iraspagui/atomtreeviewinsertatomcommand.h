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
#include "symmetrykit.h"
#include "mathkit.h"
#include "mainwindow.h"
#include "atomtreeview.h"
#include "skatomtreecontroller.h"
#include "skbondsetcontroller.h"
#include "renderstackedwidget.h"

class AtomTreeViewInsertAtomCommand : public QUndoCommand
{
public:
  AtomTreeViewInsertAtomCommand(MainWindow *mainWindow, AtomTreeView *atomTreeView, std::shared_ptr<iRASPAObject> iraspaStructure,
                                std::shared_ptr<SKAtomTreeNode> parentTreeNode, int row, AtomSelectionNodesAndIndexPaths selection, QUndoCommand *parent = nullptr);
  void undo() override final;
  void redo() override final;
private:
  MainWindow *_mainWindow;
  AtomTreeView *_atomTreeView;
  std::shared_ptr<iRASPAObject> _iraspaStructure;
  std::shared_ptr<Object> _object;
  std::shared_ptr<SKAtomTreeNode> _parentTreeNode;
  std::shared_ptr<SKAtomTreeNode> _newAtomtreeNode;
  int _row;
  AtomSelectionNodesAndIndexPaths _selection;
};
