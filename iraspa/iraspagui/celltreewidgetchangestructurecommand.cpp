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


template<typename T> std::shared_ptr<Object> CellTreeWidgetChangeStructureCommand::createiRASPAObject(std::shared_ptr<iRASPAObject> iraspa_structure)
{
  if(const std::shared_ptr<Crystal> crystal = std::dynamic_pointer_cast<Crystal>(iraspa_structure->object()))
  {
    return std::make_shared<T>(crystal);
  }
  else if(const std::shared_ptr<MolecularCrystal> molecularCrystal = std::dynamic_pointer_cast<MolecularCrystal>(iraspa_structure->object()))
  {
    return std::make_shared<T>(molecularCrystal);
  }
  else if(const std::shared_ptr<Molecule> molecule = std::dynamic_pointer_cast<Molecule>(iraspa_structure->object()))
  {
    return std::make_shared<T>(molecule);
  }
  else if(const std::shared_ptr<ProteinCrystal> proteinCrystal = std::dynamic_pointer_cast<ProteinCrystal>(iraspa_structure->object()))
  {
    return std::make_shared<T>(proteinCrystal);
  }
  else if(const std::shared_ptr<Protein> protein = std::dynamic_pointer_cast<Protein>(iraspa_structure->object()))
  {
    return std::make_shared<T>(protein);
  }
  else if(const std::shared_ptr<CrystalCylinderPrimitive> crystalCylinderPrimitive = std::dynamic_pointer_cast<CrystalCylinderPrimitive>(iraspa_structure->object()))
  {
    return std::make_shared<T>(crystalCylinderPrimitive);
  }
  else if(const std::shared_ptr<CrystalEllipsoidPrimitive> crystalEllipsoidPrimitive = std::dynamic_pointer_cast<CrystalEllipsoidPrimitive>(iraspa_structure->object()))
  {
    return std::make_shared<T>(crystalEllipsoidPrimitive);
  }
  else if(const std::shared_ptr<CrystalPolygonalPrismPrimitive> crystalPolygonalPrismPrimitive = std::dynamic_pointer_cast<CrystalPolygonalPrismPrimitive>(iraspa_structure->object()))
  {
    return std::make_shared<T>(crystalPolygonalPrismPrimitive);
  }
  else if(const std::shared_ptr<CylinderPrimitive> cylinderPrimitive = std::dynamic_pointer_cast<CylinderPrimitive>(iraspa_structure->object()))
  {
    return std::make_shared<T>(cylinderPrimitive);
  }
  else if(const std::shared_ptr<EllipsoidPrimitive> ellipsoidPrimitive = std::dynamic_pointer_cast<EllipsoidPrimitive>(iraspa_structure->object()))
  {
    return std::make_shared<T>(ellipsoidPrimitive);
  }
  else if(const std::shared_ptr<PolygonalPrismPrimitive> polygonalPrismPrimitive = std::dynamic_pointer_cast<PolygonalPrismPrimitive>(iraspa_structure->object()))
  {
    return std::make_shared<T>(polygonalPrismPrimitive);
  }
  else if(const std::shared_ptr<GridVolume> volume = std::dynamic_pointer_cast<GridVolume>(iraspa_structure->object()))
  {
    return std::make_shared<T>(volume);
  }
  return std::make_shared<Object>();
}

void CellTreeWidgetChangeStructureCommand::redo()
{
  for(const std::shared_ptr<iRASPAObject> &iraspa_structure: _old_iraspa_structures)
  {
    switch(_value)
    {

      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystal):
      {
        iraspa_structure->setObject(createiRASPAObject<Crystal>(iraspa_structure), ObjectType::crystal);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::molecularCrystal):
      {
        iraspa_structure->setObject(createiRASPAObject<MolecularCrystal>(iraspa_structure), ObjectType::molecularCrystal);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::molecule):
      {
        iraspa_structure->setObject(createiRASPAObject<Molecule>(iraspa_structure), ObjectType::molecule);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::protein):
      {
        iraspa_structure->setObject(createiRASPAObject<Protein>(iraspa_structure), ObjectType::protein);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::proteinCrystal):
      {
        iraspa_structure->setObject(createiRASPAObject<ProteinCrystal>(iraspa_structure), ObjectType::proteinCrystal);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystalEllipsoidPrimitive):
      {
        iraspa_structure->setObject(createiRASPAObject<CrystalEllipsoidPrimitive>(iraspa_structure), ObjectType::crystalEllipsoidPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystalCylinderPrimitive):
      {
        iraspa_structure->setObject(createiRASPAObject<CrystalCylinderPrimitive>(iraspa_structure), ObjectType::crystalCylinderPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::crystalPolygonalPrismPrimitive):
      {
        iraspa_structure->setObject(createiRASPAObject<CrystalPolygonalPrismPrimitive>(iraspa_structure), ObjectType::crystalPolygonalPrismPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::ellipsoidPrimitive):
      {
        iraspa_structure->setObject(createiRASPAObject<EllipsoidPrimitive>(iraspa_structure), ObjectType::ellipsoidPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::cylinderPrimitive):
      {
        iraspa_structure->setObject(createiRASPAObject<CylinderPrimitive>(iraspa_structure), ObjectType::cylinderPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::polygonalPrismPrimitive):
      {
        iraspa_structure->setObject(createiRASPAObject<PolygonalPrismPrimitive>(iraspa_structure), ObjectType::polygonalPrismPrimitive);
        break;
      }
      case static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(SKStructure::Kind::gridVolume):
      {
        iraspa_structure->setObject(createiRASPAObject<GridVolume>(iraspa_structure), ObjectType::gridVolume);
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


