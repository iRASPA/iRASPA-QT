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

#include "atomtreeviewchangedisplaynamecommand.h"
#include <QDebug>
#include <algorithm>

AtomTreeViewChangeDisplayNameCommand::AtomTreeViewChangeDisplayNameCommand(MainWindow *mainWindow, AtomTreeViewModel *model, std::shared_ptr<iRASPAObject> iraspaStructure,
                                                                           std::shared_ptr<SKAtomTreeNode> atomTreeNode, QString newValue, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _model(model),
  _iraspaObject(iraspaStructure),
  _atomTreeNode(atomTreeNode),
  _newValue(newValue)
{
  setText(QString("Change atom name"));
}

void AtomTreeViewChangeDisplayNameCommand::redo()
{
  _oldValue = _atomTreeNode->representedObject()->displayName();
  _atomTreeNode->representedObject()->setDisplayName(_newValue);

  if(_model)
  {
    // check if the current object is loaded in the atom-viewer
    if(_model->isActive(_iraspaObject))
    {
      QModelIndex index = _model->indexForNode(_atomTreeNode.get(), 0);
      emit _model->dataChanged(index,index);
    }

    _mainWindow->documentWasModified();
  }
}

void AtomTreeViewChangeDisplayNameCommand::undo()
{
  _atomTreeNode->representedObject()->setDisplayName(_oldValue);

  if(_model)
  {
    // check if the current object is loaded in the atom-viewer
    if(_model->isActive(_iraspaObject))
    {
      QModelIndex index = _model->indexForNode(_atomTreeNode.get(), 0);
      emit _model->dataChanged(index,index);
    }

    _mainWindow->documentWasModified();
  }
}
