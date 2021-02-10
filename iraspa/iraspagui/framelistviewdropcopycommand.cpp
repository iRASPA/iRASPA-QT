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

#include "framelistviewdropcopycommand.h"
#include <QDebug>
#include <algorithm>

FrameListViewDropCopyCommand::FrameListViewDropCopyCommand(MainWindow *mainWindow, FrameListViewModel *frameListViewModel,
                                                           std::shared_ptr<SceneList> sceneList, std::shared_ptr<Movie> movie,
                                                           std::vector<std::pair<std::shared_ptr<iRASPAStructure>, size_t>> nodes,
                                                           FrameSelectionIndexSet selection, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _frameListViewModel(frameListViewModel),
  _sceneList(sceneList),
  _movie(movie),
  _nodes(nodes),
  _oldSelection(selection),
  _newSelection({})
{
  Q_UNUSED(undoParent);

  setText(QString("Drop frames(s)"));
}

void FrameListViewDropCopyCommand::redo()
{
  _reverseMoves.clear();
  _newSelection.clear();

  emit _frameListViewModel->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());

  _frameListViewModel->layoutAboutToBeChanged();
  for(auto [iraspStructure, insertionRow] : _nodes)
  {
    _reverseMoves.push_back(std::make_pair(iraspStructure, iraspStructure->row()));

    whileBlocking(_frameListViewModel)->insertRow(insertionRow, iraspStructure);
  }
  _frameListViewModel->layoutChanged();

  // update selection of moved nodes _after_ all is moved
  // (indexPaths have been changed, including the indexPath of the parentNode)
  for(const auto &[iraspStructure, insertionRow] : _nodes)
  {
    _newSelection.insert(iraspStructure->row());
  }
  _newSelection.insert(_sceneList->selectedFrameIndex());

  _movie->setSelection(_newSelection);
  emit _frameListViewModel->updateSelection();

  emit _frameListViewModel->rendererReloadData();

  _mainWindow->documentWasModified();
}

void FrameListViewDropCopyCommand::undo()
{
  _frameListViewModel->layoutAboutToBeChanged();
  for(const auto &[iraspStructure, insertionRow] : _reverseMoves)
  {
    whileBlocking(_frameListViewModel)->removeRow(int(iraspStructure->row()));
  }
  _frameListViewModel->layoutChanged();

  _movie->setSelection(_oldSelection);
  emit _frameListViewModel->updateSelection();

  emit _frameListViewModel->invalidateCachedAmbientOcclusionTexture(_sceneList->allIRASPAStructures());
  emit _frameListViewModel->rendererReloadData();

  _mainWindow->documentWasModified();
}
