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

EllipsoidPrimitive::EllipsoidPrimitive()
{

}

EllipsoidPrimitive::EllipsoidPrimitive(const EllipsoidPrimitive &ellipsoidPrimitive): Structure(ellipsoidPrimitive)
{
}

EllipsoidPrimitive::EllipsoidPrimitive(std::shared_ptr<Structure> s): Structure(s)
{
  _cell = std::make_shared<SKCell>(*s->cell());

  if(dynamic_cast<Crystal*>(s.get()) ||
     dynamic_cast<CrystalEllipsoidPrimitive*>(s.get()) ||
     dynamic_cast<CrystalCylinderPrimitive*>(s.get()) ||
     dynamic_cast<CrystalPolygonalPrismPrimitive*>(s.get()))
  {
    convertAsymmetricAtomsToFractional();
  }

  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}

std::shared_ptr<Structure> EllipsoidPrimitive::clone()
{
  return std::make_shared<EllipsoidPrimitive>(static_cast<const EllipsoidPrimitive&>(*this));
}


// MARK: Rendering
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> EllipsoidPrimitive::renderPrimitiveEllipsoidObjects() const
{
  std::vector<RKInPerInstanceAttributesAtoms> atomData;
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  uint32_t asymmetricAtomIndex = 0;
  for (std::shared_ptr<SKAtomTreeNode> node : asymmetricAtomNodes)
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
            QColor color = atom->color();

            float4 position = float4(copyPosition, 1.0);

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
    }
    asymmetricAtomIndex++;
  }

  return atomData;
}


// MARK: Rendering selection
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> EllipsoidPrimitive::renderSelectedPrimitiveEllipsoidObjects() const
{
  std::vector<RKInPerInstanceAttributesAtoms> atomData;
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::set<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->selectedTreeNodes();

  uint32_t asymmetricAtomIndex = 0;
  for (std::shared_ptr<SKAtomTreeNode> node : asymmetricAtomNodes)
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
            QColor color = atom->color();

            float4 position = float4(copyPosition, 1.0);

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
    }
    asymmetricAtomIndex++;
  }

  return atomData;
}


// MARK: Filtering
// =====================================================================

std::set<int> EllipsoidPrimitive::filterCartesianAtomPositions(std::function<bool(double3)> & closure)
{
  std::set<int> data;

  if(_isVisible)
  {
    double4x4 rotationMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(_orientation), boundingBox().center());
    double3 contentShift = _cell->contentShift();
    bool3 contentFlip = _cell->contentFlip();

    std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

    uint32_t asymmetricAtomIndex = 0;
    for(std::shared_ptr<SKAtomTreeNode> node: asymmetricAtomNodes)
    {
      if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
      {
        if (atom->isVisible())
        {
          for(std::shared_ptr<SKAtomCopy> copy : atom->copies())
          {
            if(copy->type() == SKAtomCopy::AtomCopyType::copy)
            {
              double3 cartesianPosition = double3::flip(copy->position(), contentFlip, double3(0.0,0.0,0.0)) + contentShift;

              double4 position = rotationMatrix * double4(cartesianPosition.x, cartesianPosition.y, cartesianPosition.z, 1.0);

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

// MARK: Bounding box
// =====================================================================

SKBoundingBox EllipsoidPrimitive::boundingBox() const
{
  double3 minimum = double3(1e10, 1e10, 1e10);
  double3 maximum = double3(-1e10, -1e10, -1e10);

  std::vector<std::shared_ptr<SKAtomCopy>> atoms = _atomsTreeController->atomCopies();

  if (atoms.empty())
  {
    return SKBoundingBox(double3(0.0,0.0,0.0), double3(0.0, 0.0, 0.0));
  }

  for (std::shared_ptr<SKAtomCopy> atom : atoms)
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

// MARK: Symmetry
// =====================================================================

void EllipsoidPrimitive::expandSymmetry()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  for (std::shared_ptr<SKAtomTreeNode> node : asymmetricAtomNodes)
  {
    std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>{};
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

std::shared_ptr<Structure> EllipsoidPrimitive::flattenHierarchy() const
{
  std::shared_ptr<EllipsoidPrimitive> ellipsoid = std::make_shared<EllipsoidPrimitive>(static_cast<const EllipsoidPrimitive&>(*this));

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
      for(std::shared_ptr<SKAtomCopy> atomCopy : asymmetricAtom->copies())
      {
        std::shared_ptr<SKAtomCopy> newAtomCopy = std::make_shared<SKAtomCopy>(newAsymmetricAtom, atomCopy->position());
        newAsymmetricAtom->copies().push_back(newAtomCopy);
      }
      std::shared_ptr<SKAtomTreeNode> atomTreeNode = std::make_shared<SKAtomTreeNode>(newAsymmetricAtom);
      ellipsoid->atomsTreeController()->appendToRootnodes(atomTreeNode);
    }
  }

  ellipsoid->atomsTreeController()->setTags();
  ellipsoid->reComputeBoundingBox();
  ellipsoid->computeBonds();

  return ellipsoid;
}

std::shared_ptr<Structure> EllipsoidPrimitive::appliedCellContentShift() const
{
  // use the copy constructor
  std::shared_ptr<EllipsoidPrimitive> ellipsoid = std::make_shared<EllipsoidPrimitive>(static_cast<const EllipsoidPrimitive&>(*this));
  ellipsoid->_spaceGroup = this->_spaceGroup;

  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << _atomsTreeController;
  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  streamRead >> ellipsoid->_atomsTreeController;

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = ellipsoid->atomsTreeController()->flattenedLeafNodes();

  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      double3 position = double3::flip(asymmetricAtom->position(), contentFlip, double3(0.0,0.0,0.0)) + contentShift;
      asymmetricAtom->setPosition(position);
    }
  }

  ellipsoid->expandSymmetry();
  ellipsoid->setCell(std::make_shared<SKCell>(*_cell));
  ellipsoid->cell()->setContentFlip(bool3(false,false,false));
  ellipsoid->cell()->setContentShift(double3(0.0,0.0,0.0));
  ellipsoid->atomsTreeController()->setTags();
  ellipsoid->reComputeBoundingBox();
  ellipsoid->computeBonds();

  return ellipsoid;
}


std::optional<std::pair<std::shared_ptr<SKCell>, double3>> EllipsoidPrimitive::cellForFractionalPositions()
{
  SKBoundingBox boundingBox = _cell->boundingBox() + SKBoundingBox(double3(-2,-2,-2), double3(2,2,2));
  SKCell cell = SKCell(boundingBox);
  return std::make_pair(std::make_shared<SKCell>(cell), cell.inverseUnitCell() * (boundingBox.minimum()));
}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> EllipsoidPrimitive::cellForCartesianPositions()
{
  return std::nullopt;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> EllipsoidPrimitive::asymmetricAtomsCopiedAndTransformedToFractionalPositions()
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

std::vector<std::shared_ptr<SKAsymmetricAtom>> EllipsoidPrimitive::asymmetricAtomsCopiedAndTransformedToCartesianPositions()
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

std::vector<std::shared_ptr<SKAsymmetricAtom>> EllipsoidPrimitive::atomsCopiedAndTransformedToFractionalPositions()
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

std::vector<std::shared_ptr<SKAsymmetricAtom>> EllipsoidPrimitive::atomsCopiedAndTransformedToCartesianPositions()
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

// MARK: Translation operations
// =====================================================================

double3 EllipsoidPrimitive::centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms) const
{
  double3  com = double3(0.0, 0.0, 0.0);
  double M = 0.0;

  for(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom : asymmetricAtoms)
  {
    for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
    {
      if(copy->type() == SKAtomCopy::AtomCopyType::copy)
      {
        double3 pos = copy->position();
        com = com + pos;
        M += 1.0;
      }
    }
  }
  com = com / M;

  return  com;
}

double3x3 EllipsoidPrimitive::matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const
{
  double3x3 inertiaMatrix = double3x3();
  double3 com = centerOfMassOfSelectionAsymmetricAtoms(atoms);

  for(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom : atoms)
  {
    for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
    {
      if(copy->type() == SKAtomCopy::AtomCopyType::copy)
      {
        double3 dr = copy->position() - com;

        inertiaMatrix.ax += dr.y * dr.y + dr.z * dr.z;
        inertiaMatrix.ay -= dr.x * dr.y;
        inertiaMatrix.az -= dr.x * dr.z;
        inertiaMatrix.bx -= dr.y * dr.x;
        inertiaMatrix.by += dr.x * dr.x + dr.z * dr.z;
        inertiaMatrix.bz -= dr.y * dr.z;
        inertiaMatrix.cx -= dr.z * dr.x;
        inertiaMatrix.cy -= dr.z * dr.y;
        inertiaMatrix.cz += dr.x * dr.x + dr.y * dr.y;
      }
    }
  }
  return inertiaMatrix;
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> EllipsoidPrimitive::translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [translation](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                    {return std::make_pair(atom, atom->position() + translation);});

  return translatedPositions;
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> EllipsoidPrimitive::translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  double3 translationBodyFrame = double3x3::inverse(_selectionBodyFixedBasis) * translation;

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [translationBodyFrame](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                    {return std::make_pair(atom, atom->position() + translationBodyFrame);});

  return translatedPositions;
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> EllipsoidPrimitive::rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
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

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> EllipsoidPrimitive::rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
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


QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<EllipsoidPrimitive> &primitive)
{
  stream << primitive->_versionNumber;
  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<EllipsoidPrimitive> &primitive)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > primitive->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "EllipsoidPrimitive");
  }
  return stream;
}
