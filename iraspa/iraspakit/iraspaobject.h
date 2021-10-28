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

#pragma once

#include <QString>
#include <QDebug>
#include <typeinfo>
#include <numeric>
#include "structure.h"
#include "crystal.h"
#include "molecularcrystal.h"
#include "iraspakitprotocols.h"
#include "proteincrystal.h"
#include "molecule.h"
#include "protein.h"
#include "ellipsoidprimitive.h"
#include "cylinderprimitive.h"
#include "polygonalprismprimitive.h"
#include "crystalellipsoidprimitive.h"
#include "crystalcylinderprimitive.h"
#include "crystalpolygonalprismprimitive.h"
#include "gridvolume.h"
#include "object.h"
#include "raspadensityvolume.h"
#include "vtkdensityvolume.h"
#include "vaspdensityvolume.h"
#include "gaussiancubevolume.h"

class Movie;

class iRASPAObject: public std::enable_shared_from_this<iRASPAObject>
{
public:
  static char mimeType[];

  iRASPAObject():_rawValue(ObjectType::none), _object(std::make_shared<Object>()) {}
  iRASPAObject(std::shared_ptr<Crystal> crystal):_rawValue(ObjectType::crystal), _object(crystal) {}
  iRASPAObject(std::shared_ptr<MolecularCrystal> molecularCrystal):_rawValue(ObjectType::molecularCrystal), _object(molecularCrystal) {}
  iRASPAObject(std::shared_ptr<ProteinCrystal> proteinCrystal):_rawValue(ObjectType::proteinCrystal), _object(proteinCrystal) {}
  iRASPAObject(std::shared_ptr<Molecule> molecule):_rawValue(ObjectType::molecule), _object(molecule) {}
  iRASPAObject(std::shared_ptr<Protein> protein):_rawValue(ObjectType::protein), _object(protein) {}
  iRASPAObject(std::shared_ptr<EllipsoidPrimitive> ellipsoidPrimitive):_rawValue(ObjectType::ellipsoidPrimitive), _object(ellipsoidPrimitive) {}
  iRASPAObject(std::shared_ptr<CylinderPrimitive> cylinderPrimitive):_rawValue(ObjectType::cylinderPrimitive), _object(cylinderPrimitive) {}
  iRASPAObject(std::shared_ptr<PolygonalPrismPrimitive> polygonalPrismPrimitive):_rawValue(ObjectType::polygonalPrismPrimitive), _object(polygonalPrismPrimitive) {}
  iRASPAObject(std::shared_ptr<CrystalEllipsoidPrimitive> crystalEllipsoidPrimitive):_rawValue(ObjectType::crystalEllipsoidPrimitive), _object(crystalEllipsoidPrimitive) {}
  iRASPAObject(std::shared_ptr<CrystalCylinderPrimitive> crystalCylinderPrimitive):_rawValue(ObjectType::crystalCylinderPrimitive), _object(crystalCylinderPrimitive) {}
  iRASPAObject(std::shared_ptr<CrystalPolygonalPrismPrimitive> crystalPolygonalPrismPrimitive):_rawValue(ObjectType::crystalPolygonalPrismPrimitive), _object(crystalPolygonalPrismPrimitive) {}
  iRASPAObject(std::shared_ptr<GridVolume> densityVolume):_rawValue(ObjectType::gridVolume), _object(densityVolume) {}
  iRASPAObject(std::shared_ptr<RASPADensityVolume> densityVolume):_rawValue(ObjectType::RASPADensityVolume), _object(densityVolume) {}
  iRASPAObject(std::shared_ptr<VTKDensityVolume> densityVolume):_rawValue(ObjectType::VTKDensityVolume), _object(densityVolume) {}
  iRASPAObject(std::shared_ptr<VASPDensityVolume> densityVolume):_rawValue(ObjectType::VASPDensityVolume), _object(densityVolume) {}
  iRASPAObject(std::shared_ptr<GaussianCubeVolume> densityVolume):_rawValue(ObjectType::GaussianCubeVolume), _object(densityVolume) {}

  std::shared_ptr<iRASPAObject> shallowClone();

  size_t row() const;
  static std::shared_ptr<iRASPAObject> create(ObjectType type);
  inline std::shared_ptr<Object> object() const {return _object;}
  inline ObjectType type() const {return _rawValue;}
  inline void setObject(std::shared_ptr<Object> object) {_object = object;}
  inline void setObject(std::shared_ptr<Object> structure, ObjectType rawValue) {_object = structure; _rawValue = rawValue;}
  static void swapRepresentedObjects(std::shared_ptr<iRASPAObject> s1, std::shared_ptr<iRASPAObject> s2);
  void setParent(std::weak_ptr<Movie> parent) {_parent = parent;}
  std::weak_ptr<Movie> parent() {return _parent;}
  virtual ~iRASPAObject() {}
private:
  // bug: avoid accidental construction of a structure without a type, so disable this
  // iRASPAStructure(std::shared_ptr<Structure> structure):_rawValue(iRASPAStructureType::structure), _structure(structure) {}
protected:
  ObjectType _rawValue;
  std::weak_ptr<Movie> _parent{};
  std::shared_ptr<Object> _object;

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<iRASPAObject> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<iRASPAObject> &);
};



