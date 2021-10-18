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

#include "atomtreeviewchangeelementcommand.h"
#include <QDebug>
#include <algorithm>

AtomTreeViewChangeElementCommand::AtomTreeViewChangeElementCommand(MainWindow *mainWindow, AtomTreeViewModel *model,
                                                                   std::shared_ptr<ProjectStructure> projectStructure, std::shared_ptr<iRASPAObject> iraspaStructure,
                                                                   std::shared_ptr<SKAtomTreeNode> atomTreeNode, int newValue, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _model(model),
  _iraspaStructure(iraspaStructure),
  _object(iraspaStructure->object()),
  _atomTreeNode(atomTreeNode),
  _newValue(newValue),
  _structures()
{
  setText(QString("Change atom element"));
  _structures = projectStructure->sceneList()->invalidatediRASPAStructures();
}

void AtomTreeViewChangeElementCommand::redo()
{
  if(std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(_object))
  {
    _oldValue = _atomTreeNode->representedObject()->elementIdentifier();
    _atomTreeNode->representedObject()->setElementIdentifier(_newValue);

    if(_model)
    {
      if(_model->isActive(_iraspaStructure))
      {
        QModelIndex index = _model->indexForNode(_atomTreeNode.get(),2);
        emit _model->dataChanged(index,index);
      }
    }

    structure->setRepresentationColorSchemeIdentifier(structure->atomColorSchemeIdentifier(), _mainWindow->colorSets());
    structure->setRepresentationStyle(structure->atomRepresentationStyle(), _mainWindow->colorSets());

    if(_mainWindow)
    {
      emit _mainWindow->invalidateCachedAmbientOcclusionTextures(_structures);
      emit _mainWindow->invalidateCachedIsoSurfaces(_structures);
      emit _mainWindow->rendererReloadData();

      _mainWindow->documentWasModified();
    }
  }
}

void AtomTreeViewChangeElementCommand::undo()
{
  if(std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(_object))
  {
    _atomTreeNode->representedObject()->setElementIdentifier(_oldValue);

    if(_model)
    {
      if(_model->isActive(_iraspaStructure))
      {
        QModelIndex index = _model->indexForNode(_atomTreeNode.get(),2);
        emit _model->dataChanged(index,index);
      }
    }

    structure->setRepresentationColorSchemeIdentifier(structure->atomColorSchemeIdentifier(), _mainWindow->colorSets());
    structure->setRepresentationStyle(structure->atomRepresentationStyle(), _mainWindow->colorSets());

    if(_mainWindow)
    {
      emit _mainWindow->invalidateCachedAmbientOcclusionTextures(_structures);
      emit _mainWindow->invalidateCachedIsoSurfaces(_structures);
      emit _mainWindow->rendererReloadData();

      _mainWindow->documentWasModified();
    }
  }
}
