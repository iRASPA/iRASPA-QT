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

#pragma once

#include <QUndoCommand>
#include <set>
#include <vector>
#include "iraspakit.h"
#include "indexpath.h"
#include "symmetrykit.h"
#include "mathkit.h"
#include "mainwindow.h"
#include "scenetreeview.h"
#include "framelistviewmodel.h"

class SceneTreeViewDropCopyCommand : public QUndoCommand
{
public:
public:
  SceneTreeViewDropCopyCommand(MainWindow *mainWindow, SceneTreeViewModel *sceneTreeViewModel, std::weak_ptr<FrameListViewModel> frameListViewModel,
                               std::shared_ptr<SceneList> sceneList, std::shared_ptr<Scene> parent, int row,
                               std::vector<std::shared_ptr<Movie>> movies, SceneListSelection selection, QUndoCommand *undoParent = nullptr);
  void redo() override final;
  void undo() override final;
private:
  MainWindow *_mainWindow;
  SceneTreeViewModel *_sceneTreeViewModel;
  std::weak_ptr<FrameListViewModel> _frameListViewModel;
  std::shared_ptr<SceneList> _sceneList;
  std::shared_ptr<Scene> _parent;
  [[maybe_unused]] int _row;
  std::vector<std::shared_ptr<Movie>> _movies;
  SceneListSelection _selection;
  std::shared_ptr<Scene> _newParent;
};
