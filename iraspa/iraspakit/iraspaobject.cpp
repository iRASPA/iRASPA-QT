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
#include "iraspaobject.h"

char iRASPAObject::mimeType[] = "application/x-qt-iraspa-structure-mime";

std::shared_ptr<iRASPAObject> iRASPAObject::shallowClone()
{
  std::shared_ptr<iRASPAObject> structure = std::make_shared<iRASPAObject>();
  structure->_rawValue = this->_rawValue;
  structure->_object = this->_object->shallowClone();
  return structure;
}

void iRASPAObject::swapRepresentedObjects(std::shared_ptr<iRASPAObject> s1, std::shared_ptr<iRASPAObject> s2)
{
  ObjectType tempRawValue = s2->_rawValue;
  s2->_rawValue = s1->_rawValue;
  s1->_rawValue = tempRawValue;

  std::shared_ptr<Object> tempStructure = s2->_object;
  s2->_object = s1->_object;
  s1->_object = tempStructure;
}

size_t iRASPAObject::row() const
{
  if (std::shared_ptr<Movie> lockedParent = _parent.lock())
  {
    std::vector<std::shared_ptr<iRASPAObject>>::const_iterator it = std::find(lockedParent->frames().begin(), lockedParent->frames().end(), shared_from_this());
    if (it != lockedParent->frames().end())
    {
      return std::distance(lockedParent->frames().begin(), it);
    }
  }

  return 0;
}


std::shared_ptr<iRASPAObject> iRASPAObject::create(ObjectType type)
{
  switch(type)
  {
  case ObjectType::none:
    return nullptr;
  case ObjectType::object:
    return nullptr;
  case ObjectType::structure:
    return nullptr;
  case ObjectType::crystal:
    return std::make_shared<iRASPAObject>(std::make_shared<Crystal>());
  case ObjectType::molecularCrystal:
    return std::make_shared<iRASPAObject>(std::make_shared<MolecularCrystal>());
  case ObjectType::proteinCrystal:
    return std::make_shared<iRASPAObject>(std::make_shared<ProteinCrystal>());
  case ObjectType::molecule:
    return std::make_shared<iRASPAObject>(std::make_shared<Molecule>());
  case ObjectType::protein:
    return std::make_shared<iRASPAObject>(std::make_shared<Protein>());
  case ObjectType::proteinCrystalSolvent:
    return nullptr;
  case ObjectType::crystalSolvent:
    return nullptr;
  case ObjectType::molecularCrystalSolvent:
    return nullptr;
  case ObjectType::crystalEllipsoidPrimitive:
    return std::make_shared<iRASPAObject>(std::make_shared<CrystalEllipsoidPrimitive>());
  case ObjectType::crystalCylinderPrimitive:
    return std::make_shared<iRASPAObject>(std::make_shared<CrystalCylinderPrimitive>());
  case ObjectType::crystalPolygonalPrismPrimitive:
    return std::make_shared<iRASPAObject>(std::make_shared<CrystalPolygonalPrismPrimitive>());
  case ObjectType::ellipsoidPrimitive:
    return std::make_shared<iRASPAObject>(std::make_shared<EllipsoidPrimitive>());
  case ObjectType::cylinderPrimitive:
    return std::make_shared<iRASPAObject>(std::make_shared<CylinderPrimitive>());
  case ObjectType::polygonalPrismPrimitive:
    return std::make_shared<iRASPAObject>(std::make_shared<PolygonalPrismPrimitive>());
  case ObjectType::gridVolume:
    return std::make_shared<iRASPAObject>(std::make_shared<GridVolume>());
  case ObjectType::RASPADensityVolume:
    return std::make_shared<iRASPAObject>(std::make_shared<RASPADensityVolume>());
  case ObjectType::VTKDensityVolume:
    return std::make_shared<iRASPAObject>(std::make_shared<VTKDensityVolume>());
  case ObjectType::VASPDensityVolume:
    return std::make_shared<iRASPAObject>(std::make_shared<VASPDensityVolume>());
  case ObjectType::GaussianCubeVolume:
    return std::make_shared<iRASPAObject>(std::make_shared<GaussianCubeVolume>());
  }
  return nullptr;
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<iRASPAObject> &iraspa_structure)
{
  stream << static_cast<typename std::underlying_type<ObjectType>::type>(iraspa_structure->_rawValue);
  switch(iraspa_structure->_rawValue)
  {
  case ObjectType::none:
    break;
  case ObjectType::object:
    break;
  case ObjectType::structure:
    stream << iraspa_structure->_object;
    break;
  case ObjectType::crystal:
    stream << std::static_pointer_cast<Crystal>(iraspa_structure->_object);
    break;
  case ObjectType::molecularCrystal:
    stream << std::static_pointer_cast<MolecularCrystal>(iraspa_structure->_object);
    break;
  case ObjectType::proteinCrystal:
    stream << std::static_pointer_cast<ProteinCrystal>(iraspa_structure->_object);
    break;
  case ObjectType::molecule:
    stream << std::static_pointer_cast<Molecule>(iraspa_structure->_object);
    break;
  case ObjectType::protein:
    stream << std::static_pointer_cast<Protein>(iraspa_structure->_object);
    break;
  case ObjectType::proteinCrystalSolvent:
    break;
  case ObjectType::crystalSolvent:
    break;
  case ObjectType::molecularCrystalSolvent:
    break;
  case ObjectType::crystalEllipsoidPrimitive:
    stream << std::static_pointer_cast<CrystalEllipsoidPrimitive>(iraspa_structure->_object);
    break;
  case ObjectType::crystalCylinderPrimitive:
    stream << std::static_pointer_cast<CrystalCylinderPrimitive>(iraspa_structure->_object);
    break;
  case ObjectType::crystalPolygonalPrismPrimitive:
    stream << std::static_pointer_cast<CrystalPolygonalPrismPrimitive>(iraspa_structure->_object);
    break;
  case ObjectType::ellipsoidPrimitive:
    stream << std::static_pointer_cast<EllipsoidPrimitive>(iraspa_structure->_object);
    break;
  case ObjectType::cylinderPrimitive:
    stream << std::static_pointer_cast<CylinderPrimitive>(iraspa_structure->_object);
    break;
  case ObjectType::polygonalPrismPrimitive:
    stream << std::static_pointer_cast<PolygonalPrismPrimitive>(iraspa_structure->_object);
    break;
  case ObjectType::gridVolume:
    stream << std::static_pointer_cast<GridVolume>(iraspa_structure->_object);
    break;
  case ObjectType::RASPADensityVolume:
    stream << std::static_pointer_cast<RASPADensityVolume>(iraspa_structure->_object);
    break;
  case ObjectType::VTKDensityVolume:
    stream << std::static_pointer_cast<VTKDensityVolume>(iraspa_structure->_object);
    break;
  case ObjectType::VASPDensityVolume:
    stream << std::static_pointer_cast<VASPDensityVolume>(iraspa_structure->_object);
    break;
  case ObjectType::GaussianCubeVolume:
    stream << std::static_pointer_cast<GaussianCubeVolume>(iraspa_structure->_object);
    break;
  }

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<iRASPAObject> &iraspa_structure)
{
  qint64 type;
  stream >> type;
  iraspa_structure->_rawValue = ObjectType(type);

  switch(iraspa_structure->_rawValue)
  {
    case ObjectType::none:
      break;
    case ObjectType::object:
      break;
    case ObjectType::structure:
    {
      std::shared_ptr<Structure> structure = std::make_shared<Structure>();
      stream >> structure;
      iraspa_structure->_object = structure;
      break;
    }
    case ObjectType::crystal:
    {
      std::shared_ptr<Crystal> crystal = std::make_shared<Crystal>();
      stream >> crystal;
      iraspa_structure->_object = crystal;
      break;
    }
    case ObjectType::molecularCrystal:
    {
      std::shared_ptr<MolecularCrystal> molecularCrystal = std::make_shared<MolecularCrystal>();
      stream >> molecularCrystal;
      iraspa_structure->_object = molecularCrystal;
      break;
    }
    case ObjectType::proteinCrystal:
    {
      std::shared_ptr<ProteinCrystal> proteinCrystal = std::make_shared<ProteinCrystal>();
      stream >> proteinCrystal;
      iraspa_structure->_object = proteinCrystal;
      break;
    }
    case ObjectType::molecule:
    {
      std::shared_ptr<Molecule> molecule = std::make_shared<Molecule>();
      stream >> molecule;
      iraspa_structure->_object = molecule;
      break;
    }
    case ObjectType::protein:
    {
      std::shared_ptr<Protein> protein = std::make_shared<Protein>();
      stream >> protein;
      iraspa_structure->_object = protein;
      break;
    }
    case ObjectType::proteinCrystalSolvent:
      break;
    case ObjectType::crystalSolvent:
      break;
    case ObjectType::molecularCrystalSolvent:
      break;
    case ObjectType::crystalEllipsoidPrimitive:
    {
      std::shared_ptr<CrystalEllipsoidPrimitive> primitive = std::make_shared<CrystalEllipsoidPrimitive>();
      stream >> primitive;
      iraspa_structure->_object = primitive;
      break;
    }
    case ObjectType::crystalCylinderPrimitive:
    {
      std::shared_ptr<CrystalCylinderPrimitive> primitive = std::make_shared<CrystalCylinderPrimitive>();
      stream >> primitive;
      iraspa_structure->_object = primitive;
      break;
    }
    case ObjectType::crystalPolygonalPrismPrimitive:
    {
      std::shared_ptr<CrystalPolygonalPrismPrimitive> primitive = std::make_shared<CrystalPolygonalPrismPrimitive>();
      stream >> primitive;
      iraspa_structure->_object = primitive;
      break;
    }
    case ObjectType::ellipsoidPrimitive:
    {
      std::shared_ptr<EllipsoidPrimitive> primitive = std::make_shared<EllipsoidPrimitive>();
      stream >> primitive;
      iraspa_structure->_object = primitive;
      break;
    }
    case ObjectType::cylinderPrimitive:
    {
      std::shared_ptr<CylinderPrimitive> primitive = std::make_shared<CylinderPrimitive>();
      stream >> primitive;
      iraspa_structure->_object = primitive;
      break;
    }
    case ObjectType::polygonalPrismPrimitive:
    {
      std::shared_ptr<PolygonalPrismPrimitive> primitive = std::make_shared<PolygonalPrismPrimitive>();
      stream >> primitive;
      iraspa_structure->_object = primitive;
      break;
    }
    case ObjectType::gridVolume:
    {
      std::shared_ptr<GridVolume> densityVolume = std::make_shared<GridVolume>();
      stream >> densityVolume;
      iraspa_structure->_object = densityVolume;
      break;
    }
    case ObjectType::RASPADensityVolume:
    {
      std::shared_ptr<RASPADensityVolume> densityVolume = std::make_shared<RASPADensityVolume>();
      stream >> densityVolume;
      iraspa_structure->_object = densityVolume;
      break;
    }
    case ObjectType::VTKDensityVolume:
    {
      std::shared_ptr<VTKDensityVolume> densityVolume = std::make_shared<VTKDensityVolume>();
      stream >> densityVolume;
      iraspa_structure->_object = densityVolume;
      break;
    }
    case ObjectType::VASPDensityVolume:
    {
      std::shared_ptr<VASPDensityVolume> densityVolume = std::make_shared<VASPDensityVolume>();
      stream >> densityVolume;
      iraspa_structure->_object = densityVolume;
      break;
    }
    case ObjectType::GaussianCubeVolume:
    {
      std::shared_ptr<GaussianCubeVolume> densityVolume = std::make_shared<GaussianCubeVolume>();
      stream >> densityVolume;
      iraspa_structure->_object = densityVolume;
      break;
    }
  }

  return stream;
}
