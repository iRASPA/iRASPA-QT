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

#include "renderviewtranslatepositionscartesiancommand.h"
#include "renderviewtranslatepositionscartesiansubcommand.h"

#include <QDebug>
#include <algorithm>

RenderViewTranslatePositionsCartesianCommand::RenderViewTranslatePositionsCartesianCommand(MainWindow *mainWindow, std::shared_ptr<ProjectStructure> projectStructure, std::vector<std::vector<std::shared_ptr<iRASPAObject>>> iraspaStructures, double3 translation, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _iraspaStructures(iraspaStructures),
  _translation(translation),
  _oldPositions()
{
  Q_UNUSED(parent);

  setText(QString("Translate atom selection"));
  _structures = projectStructure->sceneList()->invalidatediRASPAStructures();

  for(const std::vector<std::shared_ptr<iRASPAObject>> &_iraspaSubStructures : _iraspaStructures)
  {
    for(const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspaSubStructures)
    {
      new RenderViewTranslatePositionsCartesianSubCommand(_mainWindow, iraspaStructure, translation, this);
    }
  }
}

void RenderViewTranslatePositionsCartesianCommand::redo()
{
  QUndoCommand::redo();

  if(_mainWindow)
  {
    emit _mainWindow->invalidateCachedAmbientOcclusionTextures(_structures);
    emit _mainWindow->invalidateCachedIsoSurfaces(_structures);
    emit _mainWindow->rendererReloadData();

    _mainWindow->documentWasModified();
  }
}

void RenderViewTranslatePositionsCartesianCommand::undo()
{
  QUndoCommand::undo();

  if(_mainWindow)
  {
    emit _mainWindow->invalidateCachedAmbientOcclusionTextures(_structures);
    emit _mainWindow->invalidateCachedIsoSurfaces(_structures);
    emit _mainWindow->rendererReloadData();

    _mainWindow->documentWasModified();
  }
}
