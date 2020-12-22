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

#include "framelistviewdeleteselectioncommand.h"
#include <QDebug>
#include <algorithm>

FrameListViewDeleteSelectionCommand::FrameListViewDeleteSelectionCommand(MainWindow *mainWindow, FrameListView *frameListView,
                                                                         std::shared_ptr<SceneList> sceneList, std::shared_ptr<Movie> movie,
                                                                         FrameSelectionNodesAndIndexSet deletedSelection, FrameSelectionIndexSet selection, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _frameListView(frameListView),
  _sceneList(sceneList),
  _movie(movie),
  _deletedSelection(deletedSelection),
  _reversedDeletedSelection(),
  _selection(selection),
  _selectedFrameIndex(sceneList->selectedFrameIndex())
{
  Q_UNUSED(parent);

  setText(QString("Delete selected projects"));

  std::reverse_copy(_deletedSelection.begin(), _deletedSelection.end(), std::inserter(_reversedDeletedSelection, _reversedDeletedSelection.begin()));
}

void FrameListViewDeleteSelectionCommand::redo()
{
  emit _frameListView->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());

  if(FrameListViewModel *model = qobject_cast<FrameListViewModel*>(_frameListView->model()))
  {
    model->layoutAboutToBeChanged();
    for(const auto &[frame, row] : _reversedDeletedSelection)
    {
      whileBlocking(model)->removeRow(row, _movie);
    }
    model->layoutChanged();

    _sceneList->setSelectedFrameIndex(_selectedFrameIndex);
    emit model->updateSelection();

    emit _frameListView->setSelectedFrame(_movie->frameAtIndex(_selectedFrameIndex));

    _mainWindow->recheckRemovalButtons();

    emit _frameListView->rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

void FrameListViewDeleteSelectionCommand::undo()
{
  if(FrameListViewModel *model = qobject_cast<FrameListViewModel*>(_frameListView->model()))
  {
    model->layoutAboutToBeChanged();
    for(const auto &[frame, row] : _deletedSelection)
    {
      whileBlocking(model)->insertRow(row, _movie, frame);
    }
    model->layoutChanged();

    _movie->setSelection(_selection);
    emit model->updateSelection();

    emit _frameListView->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());
    emit _frameListView->rendererReloadData();

    emit _frameListView->setSelectedFrame(_movie->frameAtIndex(_selectedFrameIndex));

    _mainWindow->documentWasModified();
  }
}
