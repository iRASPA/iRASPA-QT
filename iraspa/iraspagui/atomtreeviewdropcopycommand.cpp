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

#include "atomtreeviewdropcopycommand.h"
#include <QDebug>
#include <algorithm>

AtomTreeViewDropCopyCommand::AtomTreeViewDropCopyCommand(MainWindow *mainWindow, AtomTreeViewModel *model, std::shared_ptr<iRASPAStructure> iraspaStructure,
                                                         std::vector<std::tuple<std::shared_ptr<SKAtomTreeNode>, std::shared_ptr<SKAtomTreeNode>, int>> moves,
                                                         QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _model(model),
  _iraspaStructure(iraspaStructure),
  _structure(iraspaStructure->structure()),
  _moves(moves),
  _reverseMoves({})
{
  setText(QString("Copy atoms"));
}

void AtomTreeViewDropCopyCommand::redo()
{
  _reverseMoves.clear();

  if(_model->isActive(_iraspaStructure))
  {
    _model->layoutAboutToBeChanged();
    for(const auto &[atom, parentNode, insertionRow] : _moves)
    {
      _reverseMoves.insert(_reverseMoves.begin(), std::make_tuple(atom, atom->parent(), atom->row()));

      whileBlocking(_model)->insertRow(insertionRow, parentNode,atom);
    }
    _model->layoutChanged();
  }
  else
  {
    for(const auto &[atom, parentNode, insertionRow] : _moves)
    {
      _reverseMoves.insert(_reverseMoves.begin(), std::make_tuple(atom, atom->parent(), atom->row()));
      parentNode->insertChild(insertionRow, atom);
    }
  }

  _structure->atomsTreeController()->setTags();

  _mainWindow->documentWasModified();
}

void AtomTreeViewDropCopyCommand::undo()
{
  if(_model->isActive(_iraspaStructure))
  {
    _model->layoutAboutToBeChanged();
    for(const auto &[atom, parentNode, insertionRow] : _reverseMoves)
    {
      whileBlocking(_model)->removeRow(atom->row(), atom->parent());
    }

    _model->layoutChanged();
  }
  else
  {
    for(const auto &[atom, parentNode, insertionRow] : _reverseMoves)
    {
      parentNode->removeChild(insertionRow);
    }
  }

  _structure->atomsTreeController()->setTags();

  _mainWindow->documentWasModified();
}
