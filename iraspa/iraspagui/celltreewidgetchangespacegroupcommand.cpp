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

#include "celltreewidgetchangespacegroupcommand.h"
#include "celltreewidgetcontroller.h"
#include <QDebug>
#include <algorithm>

// Note: The iRASPAStructure is not modified, but the structure it contains is replaced.

CellTreeWidgetChangeSpaceGroupCommand::CellTreeWidgetChangeSpaceGroupCommand(MainWindow *mainWindow, CellTreeWidgetController *controller, std::shared_ptr<ProjectStructure> projectStructure, std::vector<std::shared_ptr<iRASPAObject>> iraspa_structures,
                                     int value, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _controller(controller),
  _projectStructure(projectStructure),
  _iraspa_structures(iraspa_structures),
  _old_iraspa_structures(),
  _value(value)
{
  setText(QString("Change space group"));

  std::transform(iraspa_structures.begin(), iraspa_structures.end(), std::back_inserter(_old_iraspa_structures),
                 [](std::shared_ptr<iRASPAObject> iraspastructure) -> std::pair<std::shared_ptr<iRASPAObject>, std::shared_ptr<Structure>>
                  {return std::make_pair(iraspastructure, std::dynamic_pointer_cast<Structure>(iraspastructure->object()));});
}

void CellTreeWidgetChangeSpaceGroupCommand::redo()
{
  for(std::shared_ptr<iRASPAObject> iraspa_structure: _iraspa_structures)
  {
    std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspa_structure->object())->clone();

    QByteArray byteArray = QByteArray();
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream << std::dynamic_pointer_cast<Structure>(iraspa_structure->object())->atomsTreeController();

    std::shared_ptr<SKAtomTreeController> atomTreeController;
    QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
    streamRead >> structure->atomsTreeController();

    structure->setSpaceGroupHallNumber(_value);
    iraspa_structure->setObject(structure);
  }

  emit _controller->invalidateCachedAmbientOcclusionTextures(_projectStructure->sceneList()->allIRASPAStructures());

  _mainWindow->resetData();

  _mainWindow->documentWasModified();
}

void CellTreeWidgetChangeSpaceGroupCommand::undo()
{
  for(const auto &[iraspa_structure, structure]: _old_iraspa_structures)
  {
    iraspa_structure->setObject(structure);
  }

  emit _controller->invalidateCachedAmbientOcclusionTextures(_projectStructure->sceneList()->allIRASPAStructures());

  _mainWindow->resetData();

  _mainWindow->documentWasModified();
}

