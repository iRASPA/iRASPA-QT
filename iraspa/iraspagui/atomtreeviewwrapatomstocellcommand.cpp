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

#include "atomtreeviewwrapatomstocellcommand.h"
#include <QDebug>
#include <algorithm>

// Note: The iRASPAStructure is not modified, but the structure it contains is replaced.

AtomTreeViewWrapAtomsToCellCommand::AtomTreeViewWrapAtomsToCellCommand(MainWindow *mainWindow, AtomTreeViewModel *model, std::shared_ptr<iRASPAStructure> iraspa_structure,
                                     QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _model(model),
  _iraspa_structure(iraspa_structure),
  _structure(iraspa_structure->structure())
{
  setText(QString("Make super cell"));
}

void AtomTreeViewWrapAtomsToCellCommand::redo()
{
  std::shared_ptr<Structure> structure = _iraspa_structure->structure()->wrapAtomsToCell();

  _iraspa_structure->setStructure(structure);

  _mainWindow->resetData();

  _mainWindow->documentWasModified();
}

void AtomTreeViewWrapAtomsToCellCommand::undo()
{
  _iraspa_structure->setStructure(_structure);

  _mainWindow->resetData();

  _mainWindow->documentWasModified();
}

