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
#include "atomtreeviewmodel.h"
#include "bondlistviewmodel.h"

class RenderViewDeleteSelectionSubCommand : public QUndoCommand
{
public:
  RenderViewDeleteSelectionSubCommand(MainWindow *mainWindow, AtomTreeViewModel *atomModel, BondListViewModel *bondModel,
                                     std::shared_ptr<iRASPAObject> iraspaStructure,
                                     AtomSelectionIndexPaths atomSelection, BondSelectionNodesAndIndexSet bondSelection, QUndoCommand *parent = nullptr);
  void undo() override final;
  void redo() override final;
private:
  [[maybe_unused]] MainWindow *_mainWindow;
  AtomTreeViewModel *_atomModel;
  BondListViewModel *_bondModel;
  std::shared_ptr<iRASPAObject> _iraspaStructure;
  AtomSelectionIndexPaths _atomSelection;
  BondSelectionNodesAndIndexSet _bondSelection;
  std::shared_ptr<SKAtomTreeController> _atomTreeController;
  std::shared_ptr<SKBondSetController> _bondListController;
  std::vector<std::tuple<std::shared_ptr<SKAtomTreeNode>, std::shared_ptr<SKAtomTreeNode>, size_t>> _selectedAtomNodes;
  std::vector<std::tuple<std::shared_ptr<SKAtomTreeNode>, std::shared_ptr<SKAtomTreeNode>, size_t>> _reverseSelectedAtomNodes;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricBond>, size_t>> _reverseBondSelection;
};
