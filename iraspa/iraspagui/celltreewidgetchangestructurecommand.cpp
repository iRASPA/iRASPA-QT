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
                 [](std::shared_ptr<iRASPAObject> iraspastructure) -> std::tuple<std::shared_ptr<iRASPAObject>, std::shared_ptr<Object>, ObjectType>
                 {return std::make_tuple(iraspastructure,iraspastructure->object(), iraspastructure->type());});
}




void CellTreeWidgetChangeStructureCommand::redo()
{
  for(const std::shared_ptr<iRASPAObject> &iraspa_structure: _old_iraspa_structures)
  {
    switch(_value)
    {
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystal):
      {
        std::shared_ptr<Object> newObject = std::make_shared<Crystal>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::crystal);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::molecularCrystal):
      {
        std::shared_ptr<Object> newObject = std::make_shared<MolecularCrystal>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::molecularCrystal);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::molecule):
      {
        std::shared_ptr<Object> newObject = std::make_shared<Molecule>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::molecule);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::protein):
      {
        std::shared_ptr<Object> newObject = std::make_shared<Protein>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::protein);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::proteinCrystal):
      {
        std::shared_ptr<Object> newObject = std::make_shared<ProteinCrystal>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::proteinCrystal);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystalEllipsoidPrimitive):
      {
        std::shared_ptr<Object> newObject = std::make_shared<CrystalEllipsoidPrimitive>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::crystalEllipsoidPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystalCylinderPrimitive):
      {
        std::shared_ptr<Object> newObject = std::make_shared<CrystalCylinderPrimitive>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::crystalCylinderPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystalPolygonalPrismPrimitive):
      {
        std::shared_ptr<Object> newObject = std::make_shared<CrystalPolygonalPrismPrimitive>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::crystalPolygonalPrismPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::ellipsoidPrimitive):
      {
        std::shared_ptr<Object> newObject = std::make_shared<EllipsoidPrimitive>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::ellipsoidPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::cylinderPrimitive):
      {
        std::shared_ptr<Object> newObject = std::make_shared<CylinderPrimitive>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::cylinderPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::polygonalPrismPrimitive):
      {
        std::shared_ptr<Object> newObject = std::make_shared<PolygonalPrismPrimitive>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::polygonalPrismPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::gridVolume):
      {
        std::shared_ptr<Object> newObject = std::make_shared<GridVolume>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::gridVolume);
        break;
      }        
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::RASPADensityVolume):
      {
        std::shared_ptr<Object> newObject = std::make_shared<RASPADensityVolume>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::RASPADensityVolume);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::VTKDensityVolume):
      {
        std::shared_ptr<Object> newObject = std::make_shared<VTKDensityVolume>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::VTKDensityVolume);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::VASPDensityVolume):
      {
        std::shared_ptr<Object> newObject = std::make_shared<VASPDensityVolume>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::VASPDensityVolume);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::GaussianCubeVolume):
      {
        std::shared_ptr<Object> newObject = std::make_shared<GaussianCubeVolume>(iraspa_structure->object());
        iraspa_structure->setObject(newObject, ObjectType::GaussianCubeVolume);
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


