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

#include "framelistviewchangedisplaynamecommand.h"
#include <QDebug>
#include <algorithm>

FrameListViewChangeDisplayNameCommand::FrameListViewChangeDisplayNameCommand(MainWindow *mainWindow, FrameListViewModel *model, std::shared_ptr<iRASPAObject> iraspaStructure, QString newValue, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _model(model),
  _iraspaStructure(iraspaStructure),
  _newValue(newValue)
{
  setText(QString("Change atom name"));
}

void FrameListViewChangeDisplayNameCommand::redo()
{
  if(std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(_iraspaStructure->object()))
  {
    _oldValue = structure->displayName();
    structure->setDisplayName(_newValue);

    if(_model)
    {
      QModelIndex index = _model->indexForNode(_iraspaStructure.get(), 0);
      emit _model->dataChanged(index,index);

      _mainWindow->documentWasModified();
    }
  }
}

void FrameListViewChangeDisplayNameCommand::undo()
{
  if(std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(_iraspaStructure->object()))
  {
    structure->setDisplayName(_oldValue);

    if(_model)
    {
      QModelIndex index = _model->indexForNode(_iraspaStructure.get(), 0);
      emit _model->dataChanged(index,index);

      _mainWindow->documentWasModified();
    }
  }
}
