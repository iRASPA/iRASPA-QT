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

#include "framelistviewinsertcommand.h"
#include "framelistviewmodel.h"
#include <QDebug>
#include <algorithm>

FrameListViewInsertCommand::FrameListViewInsertCommand(MainWindow *mainWindow, SceneList *sceneList, FrameListView *frameListView,
                                                       std::shared_ptr<Movie> movie, int row,
                                                       FrameSelectionNodesAndIndexSet selection, int selectedFrame, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _sceneList(sceneList),
  _frameListView(frameListView),
  _movie(movie),
  _row(row),
  _oldSelection(selection),
  _newSelection({}),
  _selectedFrame(selectedFrame)
{
  setText(QString("Insert frame"));
}

void FrameListViewInsertCommand::redo()
{
  std::shared_ptr<iRASPAStructure> newStructure = iRASPAStructure::create(_movie->movieType());

  if(FrameListViewModel* model = qobject_cast<FrameListViewModel*>(_frameListView->model()))
  {
    emit model->layoutAboutToBeChanged();
    whileBlocking(model)->insertRow(_row, _movie, newStructure);
    emit model->layoutChanged();


    // update selection of moved nodes _after_ all is moved
    // (indexPaths have been changed, including the indexPath of the parentNode)
    _newSelection.clear();
    for(const auto &[iraspStructure, insertionRow] : _oldSelection)
    {
      _newSelection.insert(iraspStructure->row());
    }

    _movie->setSelection(_newSelection);
    _sceneList->setSelectedFrameIndex(_selectedFrame);
    emit model->updateSelection();

    emit _frameListView->rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

void FrameListViewInsertCommand::undo()
{

  if(FrameListViewModel* model = qobject_cast<FrameListViewModel*>(_frameListView->model()))
  {
    emit model->layoutAboutToBeChanged();
    whileBlocking(model)->removeRow(_row, _movie);
    emit model->layoutChanged();

    _movie->setSelection(_oldSelection);
    _sceneList->setSelectedFrameIndex(_selectedFrame);
    emit model->updateSelection();

    emit _frameListView->rendererReloadData();

    _mainWindow->documentWasModified();
  }
}
