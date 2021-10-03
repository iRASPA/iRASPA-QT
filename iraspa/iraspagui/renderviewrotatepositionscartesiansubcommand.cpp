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

#include "renderviewrotatepositionscartesiansubcommand.h"
#include <QDebug>
#include <algorithm>

RenderViewRotatePositionsCartesianSubCommand::RenderViewRotatePositionsCartesianSubCommand(MainWindow *mainWindow, std::shared_ptr<iRASPAObject> iraspaStructure, simd_quatd rotation, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _iraspaStructure(iraspaStructure),
  _rotation(rotation),
  _oldPositions()
{
  Q_UNUSED(parent);

  setText(QString("Rotate atom selection"));

  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = std::dynamic_pointer_cast<Structure>(iraspaStructure->object())->atomsTreeController()->selectedObjects();
  std::transform(atoms.begin(), atoms.end(), std::back_inserter(_oldPositions),
                   [](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                     {return std::make_pair(atom, atom->position());});
}

void RenderViewRotatePositionsCartesianSubCommand::redo()
{

  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = std::dynamic_pointer_cast<Structure>(_iraspaStructure->object())->atomsTreeController()->selectedObjects();
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions = std::dynamic_pointer_cast<Structure>(_iraspaStructure->object())->rotatedPositionsSelectionCartesian(atoms, _rotation);

  for(const auto &[atom, translatedPosition] : translatedPositions)
  {
    atom->setPosition(translatedPosition);
  }
  std::dynamic_pointer_cast<Structure>(_iraspaStructure->object())->expandSymmetry();
  std::dynamic_pointer_cast<Structure>(_iraspaStructure->object())->computeBonds();
}

void RenderViewRotatePositionsCartesianSubCommand::undo()
{

  for(const auto &[atom, translatedPosition] : _oldPositions)
  {
    atom->setPosition(translatedPosition);
  }
  std::dynamic_pointer_cast<Structure>(_iraspaStructure->object())->expandSymmetry();
  std::dynamic_pointer_cast<Structure>(_iraspaStructure->object())->computeBonds();

  emit _mainWindow->rendererReloadData();
}
