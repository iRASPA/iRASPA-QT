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

#include "atomtreeviewfindnigglicommand.h"
#include <QDebug>
#include <algorithm>
#include <set>

AtomTreeViewFindNiggliCommand::AtomTreeViewFindNiggliCommand(MainWindow *mainWindow, std::shared_ptr<iRASPAObject> iraspaStructure,
                                     AtomSelectionIndexPaths atomSelection, BondSelectionNodesAndIndexSet bondSelection, QUndoCommand *parent):
  _mainWindow(mainWindow),
  _iraspaObject(iraspaStructure),
  _object(iraspaStructure->object()),
  _atomSelection(atomSelection),
  _bondSelection(bondSelection)
{
  Q_UNUSED(parent);

  setText(QString("Find and impose symmetry"));
}

void AtomTreeViewFindNiggliCommand::redo()
{
  if(std::shared_ptr<SpaceGroupViewer> spaceGroupViewer = std::dynamic_pointer_cast<SpaceGroupViewer>(_iraspaObject->object()))
  {
    if(std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(_iraspaObject->object()))
    {
      const std::vector<std::tuple<double3,int,double>> symmetryData = structure->atomSymmetryData();

      double3x3 unitCell = _iraspaObject->object()->cell()->unitCell();
      double precision = _iraspaObject->object()->cell()->precision();
      std::optional<SKSpaceGroup::FoundNiggliCellInfo> foundSpaceGroup = SKSpaceGroup::findNiggliCell(unitCell,symmetryData,false,precision);
      if(foundSpaceGroup)
      {
        int HallNumber = foundSpaceGroup->HallNumber;
        double3x3 newUnitCell = foundSpaceGroup->cell.unitCell();

        std::shared_ptr<Object> newObject = _iraspaObject->object()->shallowClone();

        if(std::shared_ptr<Structure> newStructure = std::dynamic_pointer_cast<Structure>(newObject))
        {
          std::vector<std::tuple<double3, int, double>> primitiveAtoms = foundSpaceGroup->atoms;

          newStructure->setAtomSymmetryData(newUnitCell, primitiveAtoms);

          newStructure->reComputeBoundingBox();

          newStructure->atomsTreeController()->setTags();
          newStructure->setRepresentationColorSchemeIdentifier(newStructure->atomColorSchemeIdentifier(), _mainWindow->colorSets());
          newStructure->setRepresentationStyle(newStructure->atomRepresentationStyle(), _mainWindow->colorSets());
          newStructure->updateForceField(_mainWindow->forceFieldSets());

          if(std::shared_ptr<SpaceGroupViewer> newSpaceGroupViewer = std::dynamic_pointer_cast<SpaceGroupViewer>(newStructure))
          {
            newSpaceGroupViewer->setSpaceGroupHallNumber(HallNumber);
          }
          newStructure->expandSymmetry();
          newStructure->atomsTreeController()->setTags();
          newStructure->computeBonds();
          _iraspaObject->setObject(newStructure);

          // emit _controller->invalidateCachedAmbientOcclusionTexture(_projectStructure->sceneList()->allIRASPAStructures());

          _mainWindow->resetData();

          _mainWindow->documentWasModified();
        }
      }
    }
  }
}

void AtomTreeViewFindNiggliCommand::undo()
{
  if(std::shared_ptr<SpaceGroupViewer> spaceGroupViewer = std::dynamic_pointer_cast<SpaceGroupViewer>(_object))
  {
    if(std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(_object))
    {
      _iraspaObject->setObject(_object);

      structure->atomsTreeController()->setSelectionIndexPaths(_atomSelection);
      structure->bondSetController()->setSelection(_bondSelection);

      _mainWindow->resetData();

      _mainWindow->documentWasModified();
    }
  }
}

