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

#include "renderviewtranslatepositionsbodyframesubcommand.h"
#include <QDebug>
#include <algorithm>

RenderViewTranslatePositionsBodyFrameSubCommand::RenderViewTranslatePositionsBodyFrameSubCommand(MainWindow *mainWindow, std::shared_ptr<iRASPAObject> iraspaStructure, double3 translation, QUndoCommand *parent):
  QUndoCommand(parent),
  _mainWindow(mainWindow),
  _iraspaStructure(iraspaStructure),
  _translation(translation),
  _oldPositions()
{
  Q_UNUSED(parent);

  setText(QString("Translate atom selection"));

  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_iraspaStructure->object()))
  {
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = atomViewer->atomsTreeController()->selectedObjects();
    std::transform(atoms.begin(), atoms.end(), std::back_inserter(_oldPositions),
                     [](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                       {return std::make_pair(atom, atom->position());});
  }
}

void RenderViewTranslatePositionsBodyFrameSubCommand::redo()
{
  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_iraspaStructure->object()))
  {
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = atomViewer->atomsTreeController()->selectedObjects();
    std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions = atomViewer->translatedPositionsSelectionBodyFrame(atoms, _translation);

    for(const auto &[atom, translatedPosition] : translatedPositions)
    {
      atom->setPosition(translatedPosition);
    }
    atomViewer->expandSymmetry();
  }
  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
  {
    bondViewer->computeBonds();
  }
}

void RenderViewTranslatePositionsBodyFrameSubCommand::undo()
{
  if(std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_iraspaStructure->object()))
  {
    for(const auto &[atom, translatedPosition] : _oldPositions)
    {
      atom->setPosition(translatedPosition);
    }
    atomViewer->expandSymmetry();
  }
  if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
  {
    bondViewer->computeBonds();
  }
  emit _mainWindow->rendererReloadData();
}
