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

#include "movie.h"
#include "iraspastructure.h"

char iRASPAStructure::mimeType[] = "application/x-qt-iraspa-structure-mime";

std::shared_ptr<iRASPAStructure> iRASPAStructure::clone()
{
  std::shared_ptr<iRASPAStructure> structure = std::make_shared<iRASPAStructure>();
  structure->_rawValue = this->_rawValue;
  structure->_structure = this->_structure->clone();
  return structure;
}

void iRASPAStructure::swapRepresentedObjects(std::shared_ptr<iRASPAStructure> s1, std::shared_ptr<iRASPAStructure> s2)
{
  iRASPAStructureType tempRawValue = s2->_rawValue;
  s2->_rawValue = s1->_rawValue;
  s1->_rawValue = tempRawValue;

  std::shared_ptr<Structure> tempStructure = s2->_structure;
  s2->_structure = s1->_structure;
  s1->_structure = tempStructure;
}

size_t iRASPAStructure::row() const
{
  if (std::shared_ptr<Movie> lockedParent = _parent.lock())
  {
    std::vector<std::shared_ptr<iRASPAStructure>>::const_iterator it = std::find(lockedParent->frames().begin(), lockedParent->frames().end(), shared_from_this());
    if (it != lockedParent->frames().end())
    {
      return std::distance(lockedParent->frames().begin(), it);
    }
  }

  return 0;
}

std::shared_ptr<iRASPAStructure> iRASPAStructure::create(iRASPAStructureType type)
{
  switch(type)
  {
  case iRASPAStructureType::none:
    return nullptr;
  case iRASPAStructureType::structure:
    return nullptr;
    //return std::make_shared<iRASPAStructure>(std::make_shared<Structure>());
  case iRASPAStructureType::crystal:
    return std::make_shared<iRASPAStructure>(std::make_shared<Crystal>());
  case iRASPAStructureType::molecularCrystal:
    return std::make_shared<iRASPAStructure>(std::make_shared<MolecularCrystal>());
  case iRASPAStructureType::proteinCrystal:
    return std::make_shared<iRASPAStructure>(std::make_shared<ProteinCrystal>());
  case iRASPAStructureType::molecule:
    return std::make_shared<iRASPAStructure>(std::make_shared<Molecule>());
  case iRASPAStructureType::protein:
    return std::make_shared<iRASPAStructure>(std::make_shared<Protein>());
  case iRASPAStructureType::proteinCrystalSolvent:
    return nullptr;
  case iRASPAStructureType::crystalSolvent:
    return nullptr;
  case iRASPAStructureType::molecularCrystalSolvent:
    return nullptr;
  case iRASPAStructureType::crystalEllipsoidPrimitive:
    return std::make_shared<iRASPAStructure>(std::make_shared<CrystalEllipsoidPrimitive>());
  case iRASPAStructureType::crystalCylinderPrimitive:
    return std::make_shared<iRASPAStructure>(std::make_shared<CrystalCylinderPrimitive>());
  case iRASPAStructureType::crystalPolygonalPrismPrimitive:
    return std::make_shared<iRASPAStructure>(std::make_shared<CrystalPolygonalPrismPrimitive>());
  case iRASPAStructureType::ellipsoidPrimitive:
    return std::make_shared<iRASPAStructure>(std::make_shared<EllipsoidPrimitive>());
  case iRASPAStructureType::cylinderPrimitive:
    return std::make_shared<iRASPAStructure>(std::make_shared<CylinderPrimitive>());
  case iRASPAStructureType::polygonalPrismPrimitive:
    return std::make_shared<iRASPAStructure>(std::make_shared<PolygonalPrismPrimitive>());
  }
  return nullptr;
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<iRASPAStructure> &iraspa_structure)
{
  stream << static_cast<typename std::underlying_type<iRASPAStructureType>::type>(iraspa_structure->_rawValue);
  switch(iraspa_structure->_rawValue)
  {
  case iRASPAStructureType::none:
    break;
  case iRASPAStructureType::structure:
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::crystal:
    stream << std::dynamic_pointer_cast<Crystal>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::molecularCrystal:
    stream << std::dynamic_pointer_cast<MolecularCrystal>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::proteinCrystal:
    stream << std::dynamic_pointer_cast<ProteinCrystal>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::molecule:
    stream << std::dynamic_pointer_cast<Molecule>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::protein:
    stream << std::dynamic_pointer_cast<Protein>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::proteinCrystalSolvent:
    break;
  case iRASPAStructureType::crystalSolvent:
    break;
  case iRASPAStructureType::molecularCrystalSolvent:
    break;
  case iRASPAStructureType::crystalEllipsoidPrimitive:
    stream << std::dynamic_pointer_cast<CrystalEllipsoidPrimitive>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::crystalCylinderPrimitive:
    stream << std::dynamic_pointer_cast<CrystalCylinderPrimitive>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::crystalPolygonalPrismPrimitive:
    stream << std::dynamic_pointer_cast<CrystalPolygonalPrismPrimitive>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::ellipsoidPrimitive:
    stream << std::dynamic_pointer_cast<EllipsoidPrimitive>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::cylinderPrimitive:
    stream << std::dynamic_pointer_cast<CylinderPrimitive>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  case iRASPAStructureType::polygonalPrismPrimitive:
    stream << std::dynamic_pointer_cast<PolygonalPrismPrimitive>(iraspa_structure->_structure);
    stream << iraspa_structure->_structure;
    break;
  }

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<iRASPAStructure> &iraspa_structure)
{
  qint64 type;
  stream >> type;
  iraspa_structure->_rawValue = iRASPAStructureType(type);

  switch(iraspa_structure->_rawValue)
  {
    case iRASPAStructureType::none:
      break;
    case iRASPAStructureType::structure:
    {
      std::shared_ptr<Structure> structure = std::make_shared<Structure>();
      stream >> structure;
      iraspa_structure->_structure = structure;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::crystal:
    {
      std::shared_ptr<Crystal> crystal = std::make_shared<Crystal>();
      stream >> crystal;
      iraspa_structure->_structure = crystal;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::molecularCrystal:
    {
      std::shared_ptr<MolecularCrystal> molecularCrystal = std::make_shared<MolecularCrystal>();
      stream >> molecularCrystal;
      iraspa_structure->_structure = molecularCrystal;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::proteinCrystal:
    {
      std::shared_ptr<ProteinCrystal> proteinCrystal = std::make_shared<ProteinCrystal>();
      stream >> proteinCrystal;
      iraspa_structure->_structure = proteinCrystal;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::molecule:
    {
      std::shared_ptr<Molecule> molecule = std::make_shared<Molecule>();
      stream >> molecule;
      iraspa_structure->_structure = molecule;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::protein:
    {
      std::shared_ptr<Protein> protein = std::make_shared<Protein>();
      stream >> protein;
      iraspa_structure->_structure = protein;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::proteinCrystalSolvent:
      break;
    case iRASPAStructureType::crystalSolvent:
      break;
    case iRASPAStructureType::molecularCrystalSolvent:
      break;
    case iRASPAStructureType::crystalEllipsoidPrimitive:
    {
      std::shared_ptr<CrystalEllipsoidPrimitive> primitive = std::make_shared<CrystalEllipsoidPrimitive>();
      stream >> primitive;
      iraspa_structure->_structure = primitive;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::crystalCylinderPrimitive:
    {
      std::shared_ptr<CrystalCylinderPrimitive> primitive = std::make_shared<CrystalCylinderPrimitive>();
      stream >> primitive;
      iraspa_structure->_structure = primitive;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::crystalPolygonalPrismPrimitive:
    {
      std::shared_ptr<CrystalPolygonalPrismPrimitive> primitive = std::make_shared<CrystalPolygonalPrismPrimitive>();
      stream >> primitive;
      iraspa_structure->_structure = primitive;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::ellipsoidPrimitive:
    {
      std::shared_ptr<EllipsoidPrimitive> primitive = std::make_shared<EllipsoidPrimitive>();
      stream >> primitive;
      iraspa_structure->_structure = primitive;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::cylinderPrimitive:
    {
      std::shared_ptr<CylinderPrimitive> primitive = std::make_shared<CylinderPrimitive>();
      stream >> primitive;
      iraspa_structure->_structure = primitive;
      stream >> iraspa_structure->_structure;
      break;
    }
    case iRASPAStructureType::polygonalPrismPrimitive:
    {
      std::shared_ptr<PolygonalPrismPrimitive> primitive = std::make_shared<PolygonalPrismPrimitive>();
      stream >> primitive;
      iraspa_structure->_structure = primitive;
      stream >> iraspa_structure->_structure;
      break;
    }
  }

  return stream;
}
