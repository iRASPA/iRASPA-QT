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

#include "crystal.h"
#include "molecule.h"
#include "molecularcrystal.h"
#include "proteincrystal.h"
#include "protein.h"
#include "ellipsoidprimitive.h"
#include "cylinderprimitive.h"
#include "polygonalprismprimitive.h"
#include "crystalellipsoidprimitive.h"
#include "crystalcylinderprimitive.h"
#include "crystalpolygonalprismprimitive.h"
#include "primitive.h"
#include "gridvolume.h"

Protein::Protein()
{

}

Protein::Protein(const Protein &protein): Structure(protein)
{
}

Protein::Protein(std::shared_ptr<SKStructure> frame): Structure(frame)
{
  expandSymmetry();
  _atomsTreeController->setTags();
}

Protein::Protein(const std::shared_ptr<const Crystal> structure): Structure(structure)
{
  _cell = std::make_shared<SKCell>(*structure->cell());
  convertAsymmetricAtomsToCartesian();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const MolecularCrystal> structure): Structure(structure)
{
  _cell = std::make_shared<SKCell>(*structure->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const Molecule> structure): Structure(structure)
{
  _cell = std::make_shared<SKCell>(*structure->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const ProteinCrystal> structure): Structure(structure)
{
  _cell = std::make_shared<SKCell>(*structure->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const Protein> structure): Structure(structure)
{
  _cell = std::make_shared<SKCell>(*structure->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const CrystalCylinderPrimitive> primitive): Structure(primitive)
{
  _cell = std::make_shared<SKCell>(*primitive->cell());
  convertAsymmetricAtomsToCartesian();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const CrystalEllipsoidPrimitive> primitive): Structure(primitive)
{
  _cell = std::make_shared<SKCell>(*primitive->cell());
  convertAsymmetricAtomsToCartesian();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const CrystalPolygonalPrismPrimitive> primitive): Structure(primitive)
{
  _cell = std::make_shared<SKCell>(*primitive->cell());
  convertAsymmetricAtomsToCartesian();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const CylinderPrimitive> primitive): Structure(primitive)
{
  _cell = std::make_shared<SKCell>(*primitive->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const EllipsoidPrimitive> primitive): Structure(primitive)
{
  _cell = std::make_shared<SKCell>(*primitive->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const PolygonalPrismPrimitive> primitive): Structure(primitive)
{
  _cell = std::make_shared<SKCell>(*primitive->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

Protein::Protein(const std::shared_ptr<const GridVolume> volume): Structure(volume)
{

}



std::shared_ptr<Object> Protein::shallowClone()
{
  return std::make_shared<Protein>(static_cast<const Protein&>(*this));
}

// MARK: Rendering
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> Protein::renderAtoms() const
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::vector<RKInPerInstanceAttributesAtoms> atomData = std::vector<RKInPerInstanceAttributesAtoms>();

  uint32_t asymmetricAtomIndex = 0;
  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      for (std::shared_ptr<SKAtomCopy> copy : atom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          double3 copyPosition = double3::flip(copy->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;

          QColor color = atom->color();
          double w = atom->isVisible() ? 1.0 : -1.0;

          float4 position = float4(copyPosition, w);

          float4 ambient = float4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
          float4 diffuse = float4(color.redF(), color.greenF(), color.blueF(), color.alphaF());

          float4 specular = float4(1.0, 1.0, 1.0, 1.0);

          double radius = atom->drawRadius() * atom->occupancy();
          float4 scale = float4(radius, radius, radius, 1.0);

          RKInPerInstanceAttributesAtoms atom1 = RKInPerInstanceAttributesAtoms(position, ambient, diffuse, specular, scale, asymmetricAtomIndex);
          atomData.push_back(atom1);
        }
      }
    }
    asymmetricAtomIndex++;
  }

  return atomData;
}

std::vector<RKInPerInstanceAttributesBonds> Protein::renderInternalBonds() const
{
  std::vector<RKInPerInstanceAttributesBonds> data = std::vector<RKInPerInstanceAttributesBonds>();
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  const std::vector<std::shared_ptr<SKAsymmetricBond>> asymmetricBonds = _bondSetController->arrangedObjects();

  uint32_t asymmetricBondIndex = 0;
  for(std::shared_ptr<SKAsymmetricBond> asymmetricBond : asymmetricBonds)
  {
    const std::vector<std::shared_ptr<SKBond>> bonds = asymmetricBond->copies();
    for(std::shared_ptr<SKBond> bond : bonds)
    {
      if(bond->boundaryType() == SKBond::BoundaryType::internal)
      {
        QColor color1 = bond->atom1()->parent()->color();
        QColor color2 = bond->atom2()->parent()->color();
        double w = (asymmetricBond->isVisible() && bond->atom1()->parent()->isVisible() && bond->atom2()->parent()->isVisible()) ? 1.0 : -1.0;

        double3 pos1 = double3::flip(bond->atom1()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
        double3 pos2 = double3::flip(bond->atom2()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
        double bondLength = (pos2 - pos1).length();
        double drawRadius1 = bond->atom1()->parent()->drawRadius() / bondLength;
        double drawRadius2 = bond->atom2()->parent()->drawRadius() / bondLength;

        RKInPerInstanceAttributesBonds bondData = RKInPerInstanceAttributesBonds(
              float4(pos1, w),
              float4(pos2, w),
              float4(color1.redF(), color1.greenF(), color1.blueF(), color1.alphaF()),
              float4(color2.redF(), color2.greenF(), color2.blueF(), color2.alphaF()),
              float4(drawRadius1, std::min(bond->atom1()->parent()->occupancy(), bond->atom2()->parent()->occupancy()), drawRadius2, drawRadius1 / drawRadius2),
              asymmetricBondIndex,
              static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(asymmetricBond->getBondType()));
        data.push_back(bondData);
      }
    }
    asymmetricBondIndex++;
  }

  return data;
}


// MARK: Rendering selection
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> Protein::renderSelectedAtoms() const
{
  std::set<std::shared_ptr<SKAtomTreeNode>> atomTreeNodes = _atomsTreeController->selectedTreeNodes();
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::vector<RKInPerInstanceAttributesAtoms> atomData = std::vector<RKInPerInstanceAttributesAtoms>();

  for (const std::shared_ptr<SKAtomTreeNode> &node : atomTreeNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      bool isVisible = atom->isVisible();

      if(isVisible)
      {
        uint32_t asymmetricAtomIndex = atom->asymmetricIndex();
        QColor color = atom->color();
        float4 ambient = float4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        float4 diffuse = float4(color.redF(), color.greenF(), color.blueF(), color.alphaF());

        float4 specular = float4(1.0, 1.0, 1.0, 1.0);

        double radius = atom->drawRadius() * atom->occupancy();
        float4 scale = float4(radius, radius, radius, 1.0);

        for (std::shared_ptr<SKAtomCopy> copy : atom->copies())
        {
          if (copy->type() == SKAtomCopy::AtomCopyType::copy)
          {
            double3 copyPosition = double3::flip(copy->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
            float4 position = float4(copyPosition, 1.0);

            RKInPerInstanceAttributesAtoms atom1 = RKInPerInstanceAttributesAtoms(position, ambient, diffuse, specular, scale, asymmetricAtomIndex);
            atomData.push_back(atom1);
          }
        }
      }
    }
  }

  return atomData;
}


std::vector<RKInPerInstanceAttributesBonds> Protein::renderSelectedInternalBonds() const
{
  std::vector<RKInPerInstanceAttributesBonds> data =  std::vector<RKInPerInstanceAttributesBonds>();
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  const std::vector<std::shared_ptr<SKAsymmetricBond>> asymmetricBonds = _bondSetController->arrangedObjects();

  for(int asymmetricBondIndex: _bondSetController->selectionIndexSet())
  {
    std::shared_ptr<SKAsymmetricBond> asymmetricBond = _bondSetController->arrangedObjects()[asymmetricBondIndex];
    bool isVisible = asymmetricBond->isVisible() && asymmetricBond->atom1()->isVisible()  && asymmetricBond->atom2()->isVisible();

    if(isVisible)
    {
      const std::vector<std::shared_ptr<SKBond>> bonds = asymmetricBond->copies();
      for(std::shared_ptr<SKBond> bond : bonds)
      {
        if(bond->atom1()->type() == SKAtomCopy::AtomCopyType::copy && bond->atom2()->type() == SKAtomCopy::AtomCopyType::copy && bond->boundaryType() == SKBond::BoundaryType::internal)
        {
          QColor color1 = bond->atom1()->parent()->color();
          QColor color2 = bond->atom2()->parent()->color();


          double3 pos1 = double3::flip(bond->atom1()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
          double3 pos2 = double3::flip(bond->atom2()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
          double bondLength = (pos2-pos1).length();
          double drawRadius1 = bond->atom1()->parent()->drawRadius()/bondLength;
          double drawRadius2 = bond->atom2()->parent()->drawRadius()/bondLength;

          RKInPerInstanceAttributesBonds bondData = RKInPerInstanceAttributesBonds(
                     float4(pos1,1.0),
                     float4(pos2,1.0),
                     float4(color1.redF(),color1.greenF(),color1.blueF(),color1.alphaF()),
                     float4(color2.redF(),color2.greenF(),color2.blueF(),color2.alphaF()),
                     float4(drawRadius1, std::min(bond->atom1()->parent()->occupancy(),bond->atom2()->parent()->occupancy()), drawRadius2, drawRadius1/drawRadius2),
                     asymmetricBondIndex,
                     static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(asymmetricBond->getBondType()));
          data.push_back(bondData);
        }
      }
    }
  }

  return data;
}

// MARK: Filtering
// =====================================================================

std::set<int> Protein::filterCartesianAtomPositions(std::function<bool(double3)> & closure)
{
  std::set<int> data;

  if(_isVisible)
  {
    double4x4 rotationMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(_orientation), boundingBox().center());
    double3 contentShift = _cell->contentShift();
    bool3 contentFlip = _cell->contentFlip();

    std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

    uint32_t asymmetricAtomIndex = 0;
    for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
    {
      if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
      {
        if (atom->isVisible())
        {
          for(std::shared_ptr<SKAtomCopy> copy : atom->copies())
          {
            if(copy->type() == SKAtomCopy::AtomCopyType::copy)
            {
              double3 CartesianPosition = double3::flip(copy->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;

              double4 position = rotationMatrix * double4(CartesianPosition.x, CartesianPosition.y, CartesianPosition.z, 1.0);

              double3 absoluteCartesianPosition = double3(position.x,position.y,position.z) + _origin;
              if(closure(absoluteCartesianPosition))
              {
                data.insert(asymmetricAtomIndex);
              }
            }
          }
        }
      }
      asymmetricAtomIndex++;
    }
  }

  return data;
}


std::set<int> Protein::filterCartesianBondPositions(std::function<bool(double3)> &closure)
{
  std::set<int> data;

  if(_isVisible)
  {
    const std::vector<std::shared_ptr<SKAsymmetricBond>> asymmetricBonds = _bondSetController->arrangedObjects();
    double3 contentShift = _cell->contentShift();
    bool3 contentFlip = _cell->contentFlip();

    double4x4 rotationMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(_orientation), boundingBox().center());

    uint32_t asymmetricBondIndex = 0;
    for(std::shared_ptr<SKAsymmetricBond> asymmetricBond : asymmetricBonds)
    {
      bool isVisible = asymmetricBond->isVisible() && asymmetricBond->atom1()->isVisible()  && asymmetricBond->atom2()->isVisible();

      if (isVisible)
      {
        const std::vector<std::shared_ptr<SKBond>> bonds = asymmetricBond->copies();
        for(const std::shared_ptr<SKBond> &bond : bonds)
        {
          double3 pos1 = double3::flip(bond->atom1()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
          double3 pos2 = double3::flip(bond->atom2()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;

          double3 cartesianPosition = 0.5 * (pos1 + pos2);

          double4 position = rotationMatrix * double4(cartesianPosition.x, cartesianPosition.y, cartesianPosition.z, 1.0);
          double3 absoluteCartesianPosition = double3(position.x,position.y,position.z) + _origin;
          if (closure(absoluteCartesianPosition))
          {
            data.insert(asymmetricBondIndex);
          }
        }
      }
      asymmetricBondIndex++;
    }
  }

  return data;
}


// MARK: Bounding box
// =====================================================================

SKBoundingBox Protein::boundingBox() const
{
  double3 minimum = double3(1e10, 1e10, 1e10);
  double3 maximum = double3(-1e10, -1e10, -1e10);

  qDebug() << "Molecule boundingBox";

  std::vector<std::shared_ptr<SKAtomCopy>> atoms = _atomsTreeController->atomCopies();

  if (atoms.empty())
  {
    return SKBoundingBox(double3(0.0, 0.0, 0.0), double3(0.0, 0.0, 0.0));
  }

  for (const std::shared_ptr<SKAtomCopy> &atom : atoms)
  {
    double3 CartesianPosition = atom->position();
    minimum.x = std::min(minimum.x, CartesianPosition.x);
    minimum.y = std::min(minimum.y, CartesianPosition.y);
    minimum.z = std::min(minimum.z, CartesianPosition.z);
    maximum.x = std::max(maximum.x, CartesianPosition.x);
    maximum.y = std::max(maximum.y, CartesianPosition.y);
    maximum.z = std::max(maximum.z, CartesianPosition.z);
  }

  return SKBoundingBox(minimum, maximum);
}

void Protein::reComputeBoundingBox()
{
  SKBoundingBox boundingBox = this->boundingBox();

  // store in the cell datastructure
  _cell->setBoundingBox(boundingBox);
}


// MARK: Symmetry
// =====================================================================

void Protein::expandSymmetry()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>{};

      std::shared_ptr<SKAtomCopy> newAtom = std::make_shared<SKAtomCopy>(asymmetricAtom, asymmetricAtom->position());
      newAtom->setType(SKAtomCopy::AtomCopyType::copy);
      atomCopies.push_back(newAtom);

      asymmetricAtom->setCopies(atomCopies);
    }
  }
}

void Protein::expandSymmetry(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom)
{
  std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>{};

  std::shared_ptr<SKAtomCopy> newAtom = std::make_shared<SKAtomCopy>(asymmetricAtom, asymmetricAtom->position());
  newAtom->setType(SKAtomCopy::AtomCopyType::copy);
  atomCopies.push_back(newAtom);

  asymmetricAtom->setCopies(atomCopies);
}

std::shared_ptr<Structure> Protein::flattenHierarchy() const
{
  std::shared_ptr<Protein> protein = std::make_shared<Protein>(static_cast<const Protein&>(*this));

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
      for(const std::shared_ptr<SKAtomCopy> &atomCopy : asymmetricAtom->copies())
      {
        std::shared_ptr<SKAtomCopy> newAtomCopy = std::make_shared<SKAtomCopy>(newAsymmetricAtom, atomCopy->position());
        newAsymmetricAtom->copies().push_back(newAtomCopy);
      }
      std::shared_ptr<SKAtomTreeNode> atomTreeNode = std::make_shared<SKAtomTreeNode>(newAsymmetricAtom);
      protein->atomsTreeController()->appendToRootnodes(atomTreeNode);
    }
  }

  protein->atomsTreeController()->setTags();
  protein->reComputeBoundingBox();
  protein->computeBonds();

  return protein;
}

std::shared_ptr<Structure> Protein::appliedCellContentShift() const
{
  // use the copy constructor
  std::shared_ptr<Protein> protein = std::make_shared<Protein>(static_cast<const Protein&>(*this));

  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << _atomsTreeController;
  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  streamRead >> protein->_atomsTreeController;

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = protein->atomsTreeController()->flattenedLeafNodes();

  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      double3 position = double3::flip(asymmetricAtom->position(), contentFlip, double3(0.0,0.0,0.0)) + contentShift;
      asymmetricAtom->setPosition(position);
    }
  }

  protein->expandSymmetry();
  protein->setCell(std::make_shared<SKCell>(*_cell));
  protein->cell()->setContentFlip(bool3(false,false,false));
  protein->cell()->setContentShift(double3(0.0,0.0,0.0));
  protein->atomsTreeController()->setTags();
  protein->reComputeBoundingBox();
  protein->computeBonds();

  return protein;
}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> Protein::cellForFractionalPositions()
{
  SKBoundingBox boundingBox = _cell->boundingBox() + SKBoundingBox(double3(-2,-2,-2), double3(2,2,2));
  SKCell cell = SKCell(boundingBox);
  return std::make_pair(std::make_shared<SKCell>(cell), cell.inverseUnitCell() * (boundingBox.minimum()));
}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> Protein::cellForCartesianPositions()
{
  return std::nullopt;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Protein::asymmetricAtomsCopiedAndTransformedToFractionalPositions()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms{};

  SKBoundingBox boundingBox = _cell->boundingBox() + SKBoundingBox(double3(-2,-2,-2), double3(2,2,2));
  double3x3 inverseUnitCell = SKCell(boundingBox).inverseUnitCell();
  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
      newAsymmetricAtom->setPosition(inverseUnitCell * asymmetricAtom->position());
      atoms.push_back(newAsymmetricAtom);
    }
  }

  return atoms;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Protein::asymmetricAtomsCopiedAndTransformedToCartesianPositions()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms{};

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      atoms.push_back(std::make_shared<SKAsymmetricAtom>(*asymmetricAtom));
    }
  }

  return atoms;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Protein::atomsCopiedAndTransformedToFractionalPositions()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms{};

  SKBoundingBox boundingBox = _cell->boundingBox() + SKBoundingBox(double3(-2,-2,-2), double3(2,2,2));
  double3x3 inverseUnitCell = SKCell(boundingBox).inverseUnitCell();
  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      for (std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
          newAsymmetricAtom->setPosition(inverseUnitCell * copy->position());
          atoms.push_back(newAsymmetricAtom);
        }
      }
    }
  }

  return atoms;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Protein::atomsCopiedAndTransformedToCartesianPositions()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms{};

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      for (std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
          newAsymmetricAtom->setPosition(copy->position());
          atoms.push_back(newAsymmetricAtom);
        }
      }
    }
  }

  return atoms;
}




// MARK: bond-computations
// =====================================================================



double Protein::bondLength(std::shared_ptr<SKBond> bond) const
{
	double3 pos1 = bond->atom1()->position();
	double3 pos2 = bond->atom2()->position();
	return (pos2 - pos1).length();
}

double3 Protein::bondVector(std::shared_ptr<SKBond> bond) const
{
  double3 pos1 = bond->atom1()->position();
  double3 pos2 = bond->atom2()->position();
  double3 ds = pos2 - pos1;
  return ds;
}

std::pair<double3, double3> Protein::computeChangedBondLength(std::shared_ptr<SKBond> bond, double bondLength) const
{
  double3 pos1 = bond->atom1()->position();
  std::shared_ptr<SKAsymmetricAtom> asymmetricAtom1 = bond->atom1()->parent();

  double3 pos2 = bond->atom2()->position();
  std::shared_ptr<SKAsymmetricAtom> asymmetricAtom2 = bond->atom2()->parent();

  double oldBondLength = this->bondLength(bond);

  double3 bondVector = this->bondVector(bond).normalise();

  bool isAllFixed1 = asymmetricAtom1->isFixed().x && asymmetricAtom1->isFixed().y && asymmetricAtom1->isFixed().z;
  bool isAllFixed2 = asymmetricAtom2->isFixed().x && asymmetricAtom2->isFixed().y && asymmetricAtom2->isFixed().z;

  if(!isAllFixed1 && !isAllFixed2)
  {
    double3 newPos1 = pos1 - 0.5 * (bondLength - oldBondLength) * bondVector;
    double3 newPos2 = pos2 + 0.5 * (bondLength - oldBondLength) * bondVector;
    return std::make_pair(newPos1, newPos2);
  }
  else if(isAllFixed1 && !isAllFixed2)
  {
    double3 newPos2 = pos1 + bondLength * bondVector;
    return std::make_pair(pos1, newPos2);
  }
  else if(!isAllFixed1 && isAllFixed2)
  {
    double3 newPos1 = pos2 - bondLength * bondVector;
    return std::make_pair(newPos1, pos2);
  }

  return std::make_pair(pos1,pos2);
}




void Protein::computeBonds()
{
  std::vector<std::shared_ptr<SKAtomCopy>> copies = _atomsTreeController->atomCopies();

  std::vector<std::shared_ptr<SKBond>> bonds;

  for (size_t i = 0;i < copies.size();i++)
  {
    copies[i]->setType(SKAtomCopy::AtomCopyType::copy);
    double3 posA = _cell->unitCell() * copies[i]->position();
    int elementIdentifierA = copies[i]->parent()->elementIdentifier();
    double covalentRadiusA = PredefinedElements::predefinedElements[elementIdentifierA]._covalentRadius;
    for (size_t j = i + 1;j < copies.size();j++)
    {
      if ((copies[i]->parent()->occupancy() == 1.0 && copies[j]->parent()->occupancy() == 1.0) ||
        (copies[i]->parent()->occupancy() < 1.0 && copies[j]->parent()->occupancy() < 1.0))
      {
        double3 posB = _cell->unitCell() * copies[j]->position();
        int elementIdentifierB = copies[j]->parent()->elementIdentifier();
        double covalentRadiusB = PredefinedElements::predefinedElements[elementIdentifierB]._covalentRadius;

        double length = (posA - posB).length();
        if (length < covalentRadiusA + covalentRadiusB + 0.56)
        {
          if (length < 0.1)
          {
            copies[i]->setType(SKAtomCopy::AtomCopyType::duplicate);
          }

          std::shared_ptr<SKBond> bond = std::make_shared<SKBond>(copies[i], copies[j]);
          bonds.push_back(bond);
        }
      }
    }
  }

  std::vector<std::shared_ptr<SKBond>> filtered_bonds;
  std::copy_if (bonds.begin(), bonds.end(), std::back_inserter(filtered_bonds), [](std::shared_ptr<SKBond> i){return i->atom1()->type() == SKAtomCopy::AtomCopyType::copy && i->atom2()->type() == SKAtomCopy::AtomCopyType::copy;} );
  _bondSetController->setBonds(filtered_bonds);
}




std::vector<double3> Protein::atomPositions() const
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  std::vector<double3> atomData = std::vector<double3>();


  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      for (std::shared_ptr<SKAtomCopy> copy : atom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          double3 pos = copy->position();

          double4x4 rotationMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(_orientation), boundingBox().center());

          double4 position = rotationMatrix * double4(pos.x, pos.y, pos.z, 1.0);

          atomData.push_back(double3(position.x, position.y, position.z));
        }
      }
    }
  }

  return atomData;
}

std::vector<RKInPerInstanceAttributesText> Protein::atomTextData(RKFontAtlas *fontAtlas) const
{
  std::vector<RKInPerInstanceAttributesText> data{};

  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  uint32_t asymmetricAtomIndex = 0;
  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      bool isVisible = atom->isVisible();

      if(isVisible)
      {
        for (std::shared_ptr<SKAtomCopy> copy : atom->copies())
        {
          if (copy->type() == SKAtomCopy::AtomCopyType::copy)
          {
            double3 copyPosition = double3::flip(copy->position(), contentFlip, double3(0.0,0.0,0.0)) + contentShift;

            float4 position = float4(copyPosition, 1.0);
            double radius = atom->drawRadius() * atom->occupancy();
            float4 scale = float4(radius, radius, radius, 1.0);

            int elementIdentifier = atom->elementIdentifier();

            QString text;
            switch(_atomTextType)
            {
            case RKTextType::none:
            case RKTextType::multiple_values:
              text = "";
              break;
            case RKTextType::displayName:
              text = atom->displayName();
              break;
            case RKTextType::identifier:
              text = QString::number(copy->tag());
              break;
            case RKTextType::chemicalElement:
              text = PredefinedElements::predefinedElements[elementIdentifier]._chemicalSymbol;
              break;
            case RKTextType::forceFieldType:
              text = atom->uniqueForceFieldName();
              break;
            case RKTextType::position:
              text = "(" + QString::number(copy->position().x) + "," + QString::number(copy->position().y) + "," + QString::number(copy->position().z) + ")";
              break;
            case RKTextType::charge:
              text = QString::number(atom->charge());
              break;
            }

            float x = 0.0;
            float y = 0.0;
            float sx = 1.0;
            float sy = 1.0;

            std::vector<RKInPerInstanceAttributesText> subdata{};
            QRectF rect = QRectF();
            for (int i = 0 ; i < text.size(); i++)
            {
              int id = fontAtlas->characterIndex[text[i].unicode()];
              if(id >= 0 && id<=255)
              {
                float x2 = x + fontAtlas->characters[id].xoff * sx;
                float y2 = -y - fontAtlas->characters[id].yoff * sy;
                float w = fontAtlas->characters[id].width * sx;
                float h = fontAtlas->characters[id].height * sy;

                x += fontAtlas->characters[id].xadv * sx;
                y += fontAtlas->characters[id].yadv * sy;

                float4 vertex = float4(x2,y2,w,h);
                rect=rect.united(QRectF(x2,y2,w,h));
                float4 uv = float4(fontAtlas->characters[id].x/fontAtlas->width,fontAtlas->characters[id].y/fontAtlas->height,
                                   fontAtlas->characters[id].width/fontAtlas->width, fontAtlas->characters[id].height/fontAtlas->height);

                RKInPerInstanceAttributesText atomText = RKInPerInstanceAttributesText(position, scale, vertex, uv);
                subdata.push_back(atomText);
              }
            }

            float2 shift(0.0,0.0);
            switch(_atomTextAlignment)
            {
            case RKTextAlignment::center:
            case RKTextAlignment::multiple_values:
              shift = float2(0.0,0.0);
              break;
            case RKTextAlignment::left:
              shift = float2(-rect.center().x(),0.0);
              break;
            case RKTextAlignment::right:
              shift = float2(rect.center().x(),0.0);
              break;
            case RKTextAlignment::top:
              shift = float2(0.0,rect.center().y());
              break;
            case RKTextAlignment::bottom:
              shift = float2(0.0,-rect.center().y());
              break;
            case RKTextAlignment::topLeft:
              shift = float2(-rect.center().x(),rect.center().y());
              break;
            case RKTextAlignment::topRight:
              shift = float2(rect.center().x(),rect.center().y());
              break;
            case RKTextAlignment::bottomLeft:
              shift = float2(-rect.center().x(), -rect.center().y());
              break;
            case RKTextAlignment::bottomRight:
              shift = float2(rect.center().x(), -rect.center().y());
              break;
            }

            for(RKInPerInstanceAttributesText &subdataText: subdata)
            {
              subdataText.vertexCoordinatesData.x -= rect.center().x();
              subdataText.vertexCoordinatesData.y -= rect.center().y();
              subdataText.vertexCoordinatesData.x += shift.x;
              subdataText.vertexCoordinatesData.y += shift.y;
              subdataText.vertexCoordinatesData.x /= 50.0;
              subdataText.vertexCoordinatesData.y /= 50.0;
              subdataText.vertexCoordinatesData.z /= 50.0;
              subdataText.vertexCoordinatesData.w /= 50.0;
            }

            std::copy(subdata.begin(), subdata.end(), std::inserter(data, data.end()));
          }
        }
      }
    }
    asymmetricAtomIndex++;
  }

  return data;
}

double3 Protein::centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms) const
{
  double3  com = double3(0.0, 0.0, 0.0);
  double M = 0.0;

  for(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom : asymmetricAtoms)
  {
    int elementIdentifier = asymmetricAtom->elementIdentifier();
    double mass = PredefinedElements::predefinedElements[elementIdentifier]._mass;

    for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
    {
      if(copy->type() == SKAtomCopy::AtomCopyType::copy)
      {
        double3 pos = copy->position();
        com = com + mass * pos;
        M += mass;
      }
    }
  }
  com = com / M;

  return  com;
}

double3x3 Protein::matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const
{
  double3x3 inertiaMatrix = double3x3();
  double3 com = centerOfMassOfSelectionAsymmetricAtoms(atoms);

  for(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom : atoms)
  {
    int elementIdentifier = asymmetricAtom->elementIdentifier();
    double mass = PredefinedElements::predefinedElements[elementIdentifier]._mass;

    for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
    {
      if(copy->type() == SKAtomCopy::AtomCopyType::copy)
      {
        double3 dr = copy->position() - com;

        inertiaMatrix.ax += mass * (dr.y * dr.y + dr.z * dr.z);
        inertiaMatrix.ay -= mass * dr.x * dr.y;
        inertiaMatrix.az -= mass * dr.x * dr.z;
        inertiaMatrix.bx -= mass * dr.y * dr.x;
        inertiaMatrix.by += mass * (dr.x * dr.x + dr.z * dr.z);
        inertiaMatrix.bz -= mass * dr.y * dr.z;
        inertiaMatrix.cx -= mass * dr.z * dr.x;
        inertiaMatrix.cy -= mass * dr.z * dr.y;
        inertiaMatrix.cz += mass * (dr.x * dr.x + dr.y * dr.y);
      }
    }
  }
  return inertiaMatrix;
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Protein::translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [translation](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                    {return std::make_pair(atom, atom->position() + translation);});

  return translatedPositions;
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Protein::translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  double3 translationBodyFrame = double3x3::inverse(_selectionBodyFixedBasis) * translation;

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [translationBodyFrame](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                    {return std::make_pair(atom, atom->position() + translationBodyFrame);});

  return translatedPositions;
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Protein::rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  double3 com = centerOfMassOfSelectionAsymmetricAtoms(atoms);
  double3x3 rotationMatrix = double3x3(rotation);

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [com,rotationMatrix](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                 {
                   double3 ds = atom->position() - com;
                   double3 position = rotationMatrix * ds;
                   return std::make_pair(atom, position + com);
                 });

  return translatedPositions;
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Protein::rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  double3 com = centerOfMassOfSelectionAsymmetricAtoms(atoms);
  double3x3 rotationMatrix =  _selectionBodyFixedBasis * double3x3(rotation) * double3x3::inverse(_selectionBodyFixedBasis);

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [com,rotationMatrix](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                 {
                   double3 ds = atom->position() - com;
                   double3 position = rotationMatrix * ds;
                   return std::make_pair(atom, position + com);
                 });

  return translatedPositions;
}


QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<Protein> &protein)
{
  stream << protein->_versionNumber;

  // handle super class
  stream << std::static_pointer_cast<Structure>(protein);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<Protein> &protein)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > protein->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "Protein");
  }

  std::shared_ptr<Structure> structure = std::static_pointer_cast<Structure>(protein);
  stream >> structure;

  return stream;
}
