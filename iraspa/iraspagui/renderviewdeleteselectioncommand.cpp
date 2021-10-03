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

#include "renderviewdeleteselectioncommand.h"
#include "renderviewdeleteselectionsubcommand.h"
#include <QDebug>
#include <algorithm>

RenderViewDeleteSelectionCommand::RenderViewDeleteSelectionCommand(MainWindow *mainWindow, AtomTreeViewModel *atomModel, BondListViewModel *bondModel,
                                                                   std::shared_ptr<ProjectStructure> projectStructure, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _atomModel(atomModel),
  _bondModel(bondModel),
  _structures()
{
  Q_UNUSED(parent);

  setText(QString("Delete selected atoms"));

  _structures = projectStructure->sceneList()->invalidatediRASPAStructures();
  std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures = projectStructure->sceneList()->selectediRASPAStructures();

  for(const std::vector<std::shared_ptr<iRASPAObject>> &scene : structures)
  {
    for(const std::shared_ptr<iRASPAObject> &iraspaStructure : scene)
    {
      AtomSelectionIndexPaths atomSelection = std::dynamic_pointer_cast<Structure>(iraspaStructure->object())->atomsTreeController()->selectionIndexPaths();
      BondSelectionNodesAndIndexSet bondSelection = std::dynamic_pointer_cast<Structure>(iraspaStructure->object())->bondSetController()->selectionNodesAndIndexSet();
      new RenderViewDeleteSelectionSubCommand(_mainWindow, atomModel, bondModel,iraspaStructure,
                                              atomSelection, bondSelection, this);
    }
  }
}

void RenderViewDeleteSelectionCommand::redo()
{
  QUndoCommand::redo();

  emit _atomModel->updateSelection();
  emit _bondModel->updateSelection();

  if(_mainWindow)
  {
    emit _mainWindow->invalidateCachedAmbientOcclusionTextures(_structures);
    emit _mainWindow->invalidateCachedIsoSurfaces(_structures);
    emit _mainWindow->rendererReloadData();

    _mainWindow->updateControlPanel();
    _mainWindow->documentWasModified();
  }
}

void RenderViewDeleteSelectionCommand::undo()
{
  QUndoCommand::undo();

  emit _atomModel->updateSelection();
  emit _bondModel->updateSelection();

  if(_mainWindow)
  {
    emit _mainWindow->invalidateCachedAmbientOcclusionTextures(_structures);
    emit _mainWindow->invalidateCachedIsoSurfaces(_structures);
    emit _mainWindow->rendererReloadData();

    _mainWindow->updateControlPanel();
    _mainWindow->documentWasModified();
  }
}
