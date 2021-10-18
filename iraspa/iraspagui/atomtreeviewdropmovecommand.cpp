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

#include "atomtreeviewdropmovecommand.h"
#include <QDebug>
#include <algorithm>

AtomTreeViewDropMoveCommand::AtomTreeViewDropMoveCommand(MainWindow *mainWindow, AtomTreeViewModel *model, std::shared_ptr<iRASPAObject> iraspaObject,
                                                         std::vector<std::tuple<std::shared_ptr<SKAtomTreeNode>, std::shared_ptr<SKAtomTreeNode>, size_t>> moves,
                                                         AtomSelectionIndexPaths oldSelection,
                                                         QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _model(model),
  _iraspaObject(iraspaObject),
  _moves(moves),
  _reverseMoves({}),
  _oldSelection(oldSelection)
{
  setText(QString("Reorder atoms"));
}

void AtomTreeViewDropMoveCommand::redo()
{
  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<Structure>(_iraspaObject->object()))
  {
    _reverseMoves.clear();
    _newSelection.second.clear();

    if(_model->isActive(_iraspaObject))
    {
      emit _model->layoutAboutToBeChanged();
      for(const auto &[atom, parentNode, insertionRow]  : _moves)
      {
        _reverseMoves.insert(_reverseMoves.begin(), std::make_tuple(atom, atom->parent(), atom->row()));

        whileBlocking(_model)->removeRow(atom->row(), atom->parent());

        // workaround qt-bug (the branch is not shown for parents with no children unless it is explicitely set collapsed)
        if(atom->parent()->childNodes().empty())
        {
          QModelIndex parentIndex = _model->indexForNode(atom->parent().get());
          emit _model->collapse(parentIndex);
        }

        emit _model->expand(_model->indexForNode(parentNode.get()));

        whileBlocking(_model)->insertRow(insertionRow, parentNode,atom);
      }
      atomViewer->atomsTreeController()->setTags();
      emit _model->layoutChanged();

      // update selection of moved nodes _after_ all is moved
      // (indexPaths have been changed, including the indexPath of the parentNode)
      for(const auto &[projectTreeNode, parentNode, insertionRow] : _moves)
      {
        _newSelection.second.insert(projectTreeNode->indexPath());
      }



      atomViewer->atomsTreeController()->setSelectionIndexPaths(_newSelection);
      emit _model->updateSelection();
    }
    else
    {
      for(const auto &[atom, parentNode, insertionRow]  : _moves)
      {
        atom->parent()->removeChild(atom->row());
        parentNode->insertChild(insertionRow, atom);
      }
      atomViewer->atomsTreeController()->setTags();

      // update selection of moved nodes _after_ all is moved
      // (indexPaths have been changed, including the indexPath of the parentNode)
      for(const auto &[projectTreeNode, parentNode, insertionRow] : _moves)
      {
        _newSelection.second.insert(projectTreeNode->indexPath());
      }

      atomViewer->atomsTreeController()->setSelectionIndexPaths(_newSelection);
    }

    _mainWindow->documentWasModified();
  }
}

void AtomTreeViewDropMoveCommand::undo()
{
  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_iraspaObject->object()))
  {
    if(_model->isActive(_iraspaObject))
    {
      emit _model->layoutAboutToBeChanged();
      for(const auto &[atom, parentNode, insertionRow]  : _reverseMoves)
      {
        whileBlocking(_model)->removeRow(atom->row(), atom->parent());

        // workaround qt-bug (the branch is not shown for parents with no children unless it is explicitely set collapsed)
        if(atom->parent()->childNodes().empty())
        {
          QModelIndex parentIndex = _model->indexForNode(atom->parent().get());
          emit _model->collapse(parentIndex);
        }

        whileBlocking(_model)->insertRow(insertionRow, parentNode, atom);
      }
      atomViewer->atomsTreeController()->setTags();
      emit _model->layoutChanged();

      atomViewer->atomsTreeController()->setSelectionIndexPaths(_oldSelection);
      emit _model->updateSelection();
    }
    else
    {
      for(const auto &[atom, parentNode, insertionRow]  : _reverseMoves)
      {
        atom->parent()->removeChild(atom->row());
        parentNode->insertChild(insertionRow, atom);
      }
      atomViewer->atomsTreeController()->setTags();
    }

    _mainWindow->documentWasModified();
  }
}
