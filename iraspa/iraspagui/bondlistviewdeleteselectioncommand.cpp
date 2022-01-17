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

#include "bondlistviewdeleteselectioncommand.h"
#include <QDebug>
#include <algorithm>

BondListViewDeleteSelectionCommand::BondListViewDeleteSelectionCommand(MainWindow *mainWindow, std::shared_ptr<BondListViewModel> bondTreeModel,
                                               std::shared_ptr<Object> object,
                                               std::vector<std::shared_ptr<SKAsymmetricBond>> bonds, BondSelectionIndexSet bondSelection, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _bondTreeModel(bondTreeModel),
  _object(object),
  _bonds(bonds),
  _bondSelection(bondSelection)
{
  Q_UNUSED(parent);

  setText(QString("Delete bond selection"));
}

void BondListViewDeleteSelectionCommand::redo()
{
  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_object))
  {
    if(std::shared_ptr<BondListViewModel> bondListModel = _bondTreeModel.lock())
    {
      bondListModel->deleteSelection(_object, _bondSelection);
    }

    if(_mainWindow)
    {
      _mainWindow->reloadDetailViews();

      _mainWindow->documentWasModified();
    }
  }
}

void BondListViewDeleteSelectionCommand::undo()
{
  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_object))
  {
    if(std::shared_ptr<BondListViewModel> bondListModel = _bondTreeModel.lock())
    {
      bondListModel->insertSelection(_object, _bonds, _bondSelection);
    }

    if(_mainWindow)
    {
      _mainWindow->reloadDetailViews();

      _mainWindow->documentWasModified();
    }
  }
}
