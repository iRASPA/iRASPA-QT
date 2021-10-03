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

#include "atomtreeviewinsertatomgroupcommand.h"
#include <QDebug>
#include <algorithm>

AtomTreeViewInsertAtomGroupCommand::AtomTreeViewInsertAtomGroupCommand(MainWindow *mainWindow, AtomTreeView *atomTreeView, std::shared_ptr<iRASPAObject> iraspaStructure,
                                                                       std::shared_ptr<SKAtomTreeNode> parentTreeNode, int row,
                                                                       AtomSelectionNodesAndIndexPaths selection, QUndoCommand *parent):
  _mainWindow(mainWindow),
  _atomTreeView(atomTreeView),
  _iraspaStructure(iraspaStructure),
  _structure(std::dynamic_pointer_cast<Structure>(iraspaStructure->object())),
  _parentTreeNode(parentTreeNode),
  _row(row),
  _selection(selection)
{
  Q_UNUSED(parent);

  setText(QString("Insert group atom"));

  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>("C", 6);
  atom->setDisplayName("New atom group");
  _newAtomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  _newAtomtreeNode->setGroupItem(true);
  _newAtomtreeNode->setDisplayName("New atom group");
}

void AtomTreeViewInsertAtomGroupCommand::redo()
{
  if(_atomTreeView)
  {
    if(AtomTreeViewModel* pModel = qobject_cast<AtomTreeViewModel*>(_atomTreeView->model()))
    {
      if(pModel->isActive(_iraspaStructure))
      {
        pModel->insertRow(_row, _parentTreeNode, _newAtomtreeNode);
        QModelIndex index = pModel->indexForNode(_newAtomtreeNode.get());
        _atomTreeView->setFirstColumnSpanned(index.row(),index.parent(), true);

        AtomSelectionIndexPaths newSelection = _structure->atomsTreeController()->updateIndexPathsFromNodes(_selection);
        _structure->atomsTreeController()->setSelectionIndexPaths(newSelection);
        _atomTreeView->reloadSelection();
      }
      else
      {
        _parentTreeNode->insertChild(_row, _newAtomtreeNode);

        AtomSelectionIndexPaths newSelection = _structure->atomsTreeController()->updateIndexPathsFromNodes(_selection);
        _structure->atomsTreeController()->setSelectionIndexPaths(newSelection);
      }
    }
  }
  _structure->atomsTreeController()->setTags();
  _structure->setRepresentationColorSchemeIdentifier(_structure->atomColorSchemeIdentifier(), _mainWindow->colorSets());
  _structure->setRepresentationStyle(_structure->atomRepresentationStyle(), _mainWindow->colorSets());
  if(_atomTreeView)
    emit _atomTreeView->rendererReloadData();

  _mainWindow->documentWasModified();
}

void AtomTreeViewInsertAtomGroupCommand::undo()
{
  if(_atomTreeView)
  {
    if(AtomTreeViewModel* pModel = qobject_cast<AtomTreeViewModel*>(_atomTreeView->model()))
    {
      if(pModel->isActive(_iraspaStructure))
      {
        pModel->removeRow(_row, _parentTreeNode);

        _structure->atomsTreeController()->setSelectionIndexPaths(_selection);

        _atomTreeView->reloadSelection();
      }
      else
      {
        _parentTreeNode->removeChild(_row);

        _structure->atomsTreeController()->setSelectionIndexPaths(_selection);
      }
    }
  }
  _structure->atomsTreeController()->setTags();
  _structure->setRepresentationColorSchemeIdentifier(_structure->atomColorSchemeIdentifier(), _mainWindow->colorSets());
  _structure->setRepresentationStyle(_structure->atomRepresentationStyle(), _mainWindow->colorSets());
  if(_atomTreeView)
    emit _atomTreeView->rendererReloadData();

  _mainWindow->documentWasModified();
}
