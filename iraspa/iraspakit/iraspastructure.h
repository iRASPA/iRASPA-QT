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

class Movie;

class iRASPAStructure: public std::enable_shared_from_this<iRASPAStructure>
{
public:
  static char mimeType[];

  iRASPAStructure():_rawValue(iRASPAStructureType::none), _structure(std::make_shared<Structure>()) {}
  iRASPAStructure(std::shared_ptr<Crystal> crystal):_rawValue(iRASPAStructureType::crystal), _structure(crystal) {}
  iRASPAStructure(std::shared_ptr<MolecularCrystal> molecularCrystal):_rawValue(iRASPAStructureType::molecularCrystal), _structure(molecularCrystal) {}
  iRASPAStructure(std::shared_ptr<ProteinCrystal> proteinCrystal):_rawValue(iRASPAStructureType::proteinCrystal), _structure(proteinCrystal) {}
  iRASPAStructure(std::shared_ptr<Molecule> molecule):_rawValue(iRASPAStructureType::molecule), _structure(molecule) {}
  iRASPAStructure(std::shared_ptr<Protein> protein):_rawValue(iRASPAStructureType::protein), _structure(protein) {}
  iRASPAStructure(std::shared_ptr<EllipsoidPrimitive> ellipsoidPrimitive):_rawValue(iRASPAStructureType::ellipsoidPrimitive), _structure(ellipsoidPrimitive) {}
  iRASPAStructure(std::shared_ptr<CylinderPrimitive> cylinderPrimitive):_rawValue(iRASPAStructureType::cylinderPrimitive), _structure(cylinderPrimitive) {}
  iRASPAStructure(std::shared_ptr<PolygonalPrismPrimitive> polygonalPrismPrimitive):_rawValue(iRASPAStructureType::polygonalPrismPrimitive), _structure(polygonalPrismPrimitive) {}
  iRASPAStructure(std::shared_ptr<CrystalEllipsoidPrimitive> crystalEllipsoidPrimitive):_rawValue(iRASPAStructureType::crystalEllipsoidPrimitive), _structure(crystalEllipsoidPrimitive) {}
  iRASPAStructure(std::shared_ptr<CrystalCylinderPrimitive> crystalCylinderPrimitive):_rawValue(iRASPAStructureType::crystalCylinderPrimitive), _structure(crystalCylinderPrimitive) {}
  iRASPAStructure(std::shared_ptr<CrystalPolygonalPrismPrimitive> crystalPolygonalPrismPrimitive):_rawValue(iRASPAStructureType::crystalPolygonalPrismPrimitive), _structure(crystalPolygonalPrismPrimitive) {}

  std::shared_ptr<iRASPAStructure> clone();

  size_t row() const;
  static std::shared_ptr<iRASPAStructure> create(iRASPAStructureType type);
  inline std::shared_ptr<Structure> structure() const {return _structure;}
  inline iRASPAStructureType type() const {return _rawValue;}
  inline void setStructure(std::shared_ptr<Structure> structure) {_structure = structure;}
  inline void setStructure(std::shared_ptr<Structure> structure, iRASPAStructureType rawValue) {_structure = structure; _rawValue = rawValue;}
  static void swapRepresentedObjects(std::shared_ptr<iRASPAStructure> s1, std::shared_ptr<iRASPAStructure> s2);
  void setParent(std::weak_ptr<Movie> parent) {_parent = parent;}
  std::weak_ptr<Movie> parent() {return _parent;}
  virtual ~iRASPAStructure() {}
private:
  // bug: avoid accidental constructure of a structure without a type, so disable this
  // iRASPAStructure(std::shared_ptr<Structure> structure):_rawValue(iRASPAStructureType::structure), _structure(structure) {}
protected:
  iRASPAStructureType _rawValue;
  std::weak_ptr<Movie> _parent{};
  std::shared_ptr<Structure> _structure;

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<iRASPAStructure> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<iRASPAStructure> &);
};
