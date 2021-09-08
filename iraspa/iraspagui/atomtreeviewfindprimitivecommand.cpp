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

#include "atomtreeviewfindprimitivecommand.h"
#include <QDebug>
#include <algorithm>
#include <set>

AtomTreeViewFindPrimitiveCommand::AtomTreeViewFindPrimitiveCommand(MainWindow *mainWindow, std::shared_ptr<iRASPAStructure> iraspaStructure,
                                     AtomSelectionIndexPaths atomSelection, BondSelectionNodesAndIndexSet bondSelection, QUndoCommand *parent):
  _mainWindow(mainWindow),
  _iraspaStructure(iraspaStructure),
  _structure(iraspaStructure->structure()),
  _atomSelection(atomSelection),
  _bondSelection(bondSelection)
{
  Q_UNUSED(parent);

  setText(QString("Find and impose symmetry"));


}

void AtomTreeViewFindPrimitiveCommand::redo()
{
  const std::vector<std::tuple<double3,int,double>> symmetryData = _iraspaStructure->structure()->atomSymmetryData();

  double3x3 unitCell = _iraspaStructure->structure()->cell()->unitCell();
  std::optional<SKSpaceGroup::FoundPrimitiveCellInfo> foundSpaceGroup = SKSpaceGroup::SKFindPrimitive(unitCell,symmetryData,true,1e-2);
  if(foundSpaceGroup)
  {
    std::shared_ptr<Structure> structure = _iraspaStructure->structure()->clone();

    double3x3 newUnitCell = foundSpaceGroup->cell.unitCell();
    std::vector<std::tuple<double3, int, double>> primitiveAtoms = foundSpaceGroup->atoms;

    structure->setAtomSymmetryData(newUnitCell, primitiveAtoms);

    structure->reComputeBoundingBox();

    structure->atomsTreeController()->setTags();
    structure->setRepresentationColorSchemeIdentifier(structure->atomColorSchemeIdentifier(), _mainWindow->colorSets());
    structure->setRepresentationStyle(structure->atomRepresentationStyle(), _mainWindow->colorSets());
    structure->updateForceField(_mainWindow->forceFieldSets());

    structure->setSpaceGroupHallNumber(1);
    _iraspaStructure->setStructure(structure);

    // emit _controller->invalidateCachedAmbientOcclusionTexture(_projectStructure->sceneList()->allIRASPAStructures());

    _mainWindow->resetData();

    _mainWindow->documentWasModified();
  }
}

void AtomTreeViewFindPrimitiveCommand::undo()
{
  _iraspaStructure->setStructure(_structure);

  if(std::shared_ptr<Structure> structure = _structure)
  {
    structure->atomsTreeController()->setSelectionIndexPaths(_atomSelection);
    structure->bondSetController()->setSelection(_bondSelection);
  }

  _mainWindow->resetData();

  _mainWindow->documentWasModified();
}

