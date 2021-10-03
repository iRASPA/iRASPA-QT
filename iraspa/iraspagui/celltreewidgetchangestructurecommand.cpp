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

#include "celltreewidgetchangestructurecommand.h"
#include <QDebug>
#include <algorithm>

CellTreeWidgetChangeStructureCommand::CellTreeWidgetChangeStructureCommand(MainWindow *mainWindow, std::shared_ptr<ProjectTreeNode> projectTreeNode, std::vector<std::shared_ptr<iRASPAObject>> old_iraspa_structures, int value, QUndoCommand *undoParent):
  QUndoCommand(undoParent),
  _mainWindow(mainWindow),
  _projectTreeNode(projectTreeNode),
  _old_iraspa_structures(old_iraspa_structures),
  _old_structures(),
  _value(value)
{
  setText(QString("Change structure-type"));

  _old_structures.clear();
  std::transform(old_iraspa_structures.begin(),old_iraspa_structures.end(),std::back_inserter(_old_structures),
                 [](std::shared_ptr<iRASPAObject> iraspastructure) -> std::tuple<std::shared_ptr<iRASPAObject>, std::shared_ptr<Structure>, ObjectType>
                 {return std::make_tuple(iraspastructure,std::dynamic_pointer_cast<Structure>(iraspastructure->object()), iraspastructure->type());});
}

void CellTreeWidgetChangeStructureCommand::redo()
{
  for(std::shared_ptr<iRASPAObject> iraspa_structure: _old_iraspa_structures)
  {
    switch(_value)
    {
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystal):
      {
        std::shared_ptr<Crystal> crystal = std::make_shared<Crystal>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(crystal, ObjectType::crystal);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::molecularCrystal):
      {
        std::shared_ptr<MolecularCrystal> molecularCrystal = std::make_shared<MolecularCrystal>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(molecularCrystal, ObjectType::molecularCrystal);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::molecule):
      {
        std::shared_ptr<Molecule> molecule = std::make_shared<Molecule>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(molecule, ObjectType::molecule);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::protein):
      {
        std::shared_ptr<Protein> protein = std::make_shared<Protein>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(protein, ObjectType::protein);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::proteinCrystal):
      {
        std::shared_ptr<ProteinCrystal> proteinCrystal = std::make_shared<ProteinCrystal>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(proteinCrystal, ObjectType::proteinCrystal);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystalEllipsoidPrimitive):
      {
        std::shared_ptr<CrystalEllipsoidPrimitive> crystalEllipsoidPrimitive = std::make_shared<CrystalEllipsoidPrimitive>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(crystalEllipsoidPrimitive, ObjectType::crystalEllipsoidPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystalCylinderPrimitive):
      {
        std::shared_ptr<CrystalCylinderPrimitive> crystalCylinderPrimitive = std::make_shared<CrystalCylinderPrimitive>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(crystalCylinderPrimitive, ObjectType::crystalCylinderPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystalPolygonalPrismPrimitive):
      {
        std::shared_ptr<CrystalPolygonalPrismPrimitive> crystalPolygonalPrismPrimitive = std::make_shared<CrystalPolygonalPrismPrimitive>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(crystalPolygonalPrismPrimitive, ObjectType::crystalPolygonalPrismPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::ellipsoidPrimitive):
      {
        std::shared_ptr<EllipsoidPrimitive> ellipsoidPrimitive = std::make_shared<EllipsoidPrimitive>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(ellipsoidPrimitive, ObjectType::ellipsoidPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::cylinderPrimitive):
      {
        std::shared_ptr<CylinderPrimitive> cylinderPrimitive = std::make_shared<CylinderPrimitive>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(cylinderPrimitive, ObjectType::cylinderPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::polygonalPrismPrimitive):
      {
        std::shared_ptr<PolygonalPrismPrimitive> polygonalPrismPrimitive = std::make_shared<PolygonalPrismPrimitive>(std::dynamic_pointer_cast<Structure>(iraspa_structure->object()));
        iraspa_structure->setObject(polygonalPrismPrimitive, ObjectType::polygonalPrismPrimitive);
        break;
      }
      default:
        break;
    }
  }

  _mainWindow->resetData();

  _mainWindow->documentWasModified();
}

void CellTreeWidgetChangeStructureCommand::undo()
{
  for(const auto &[iraspaStructure, structure, type] : _old_structures)
  {
    iraspaStructure->setObject(structure, type);
  }

  _mainWindow->resetData();

  _mainWindow->documentWasModified();
}


