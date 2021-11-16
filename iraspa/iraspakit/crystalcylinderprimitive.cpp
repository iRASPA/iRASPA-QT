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

CrystalCylinderPrimitive::CrystalCylinderPrimitive()
{

}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const CrystalCylinderPrimitive &crystalCylinderPrimitive): Primitive(crystalCylinderPrimitive)
{
}

std::shared_ptr<Object> CrystalCylinderPrimitive::shallowClone()
{
  return std::make_shared<CrystalCylinderPrimitive>(static_cast<const CrystalCylinderPrimitive&>(*this));
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const Crystal> structure): Primitive(structure)
{
  _cell = std::make_shared<SKCell>(*structure->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const MolecularCrystal> structure): Primitive(structure)
{
  _cell = std::make_shared<SKCell>(*structure->cell());
  convertAsymmetricAtomsToFractional();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const Molecule> structure): Primitive(structure)
{
  _cell = std::make_shared<SKCell>(structure->boundingBox());
  convertAsymmetricAtomsToFractional();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const ProteinCrystal> structure): Primitive(structure)
{
  _cell = std::make_shared<SKCell>(*structure->cell());
  convertAsymmetricAtomsToFractional();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const Protein> structure): Primitive(structure)
{
  _cell = std::make_shared<SKCell>(structure->boundingBox());
  convertAsymmetricAtomsToFractional();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const CrystalCylinderPrimitive> primitive): Primitive(primitive)
{
  _cell = std::make_shared<SKCell>(*primitive->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const CrystalEllipsoidPrimitive> primitive): Primitive(primitive)
{
  _cell = std::make_shared<SKCell>(*primitive->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const CrystalPolygonalPrismPrimitive> primitive): Primitive(primitive)
{
  _cell = std::make_shared<SKCell>(*primitive->cell());
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const CylinderPrimitive> primitive): Primitive(primitive)
{
  _cell = std::make_shared<SKCell>(primitive->boundingBox());
  convertAsymmetricAtomsToFractional();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const EllipsoidPrimitive> primitive): Primitive(primitive)
{
  _cell = std::make_shared<SKCell>(primitive->boundingBox());
  convertAsymmetricAtomsToFractional();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const PolygonalPrismPrimitive> primitive): Primitive(primitive)
{
  _cell = std::make_shared<SKCell>(primitive->boundingBox());
  convertAsymmetricAtomsToFractional();
  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
}

CrystalCylinderPrimitive::CrystalCylinderPrimitive(const std::shared_ptr<const GridVolume> volume): Primitive(volume)
{

}




// MARK: Rendering
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> CrystalCylinderPrimitive::renderCrystalPrimitiveCylinderObjects() const
{
  std::vector<RKInPerInstanceAttributesAtoms> atomData;

  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  uint32_t asymmetricAtomIndex = 0;
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      bool isVisible = atom->isVisible();

      if(isVisible)
      {
        for(std::shared_ptr<SKAtomCopy> copy : atom->copies())
        {
          if(copy->type() == SKAtomCopy::AtomCopyType::copy)
          {
            float4 ambient = float4(1.0,1.0,1.0,1.0);
            float4 diffuse = float4(1.0,1.0,1.0,1.0);
            float4 specular = float4(1.0,1.0,1.0,1.0);

            double radius = 1.0;
            float4 scale = float4(radius,radius,radius,1.0);
            double3 copyPosition = double3::flip(copy->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;

            for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
            {
              for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
              {
                for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
                {
                  float4 position = float4(_cell->unitCell() * (copyPosition + double3(k1,k2,k3)), 1.0);

                  RKInPerInstanceAttributesAtoms atom1 = RKInPerInstanceAttributesAtoms(position, ambient, diffuse, specular, scale, asymmetricAtomIndex);
                  atomData.push_back(atom1);
                }
              }
            }
          }
        }
      }
    }
    asymmetricAtomIndex++;
  }

  return atomData;
}

std::vector<RKInPerInstanceAttributesAtoms> CrystalCylinderPrimitive::renderUnitCellSpheres() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  std::vector<RKInPerInstanceAttributesAtoms> data = std::vector<RKInPerInstanceAttributesAtoms>();

  uint32_t asymmetricBondIndex=0;
  for(int k1=minimumReplicaX;k1<=maximumReplicaX+1;k1++)
  {
    for(int k2=minimumReplicaY;k2<=maximumReplicaY+1;k2++)
    {
      for(int k3=minimumReplicaZ;k3<=maximumReplicaZ+1;k3++)
      {
        double3 position = _cell->unitCell() * double3(k1,k2,k3); // + origin();
        float4 ambient = float4(1.0f,1.0f,1.0f,1.0f);
        float4 diffuse = float4(1.0f,1.0f,1.0f,1.0f);
        float4 specular = float4(1.0f,1.0f,1.0f,1.0f);
        float4 scale = float4(0.1f,0.1f,0.1f,1.0f);
        RKInPerInstanceAttributesAtoms sphere = RKInPerInstanceAttributesAtoms(float4(position,1.0), ambient, diffuse, specular, scale, asymmetricBondIndex);
        data.push_back(sphere);
      }
    }
    asymmetricBondIndex++;
  }

  return data;
}

std::vector<RKInPerInstanceAttributesBonds> CrystalCylinderPrimitive::renderUnitCellCylinders() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  std::vector<RKInPerInstanceAttributesBonds> data = std::vector<RKInPerInstanceAttributesBonds>();

  for(int k1=minimumReplicaX;k1<=maximumReplicaX+1;k1++)
  {
    for(int k2=minimumReplicaY;k2<=maximumReplicaY+1;k2++)
    {
      for(int k3=minimumReplicaZ;k3<=maximumReplicaZ+1;k3++)
      {
        if(k1<=maximumReplicaX)
        {
          double3 position1 = _cell->unitCell() * double3(k1,k2,k3); // + origin();
          double3 position2 = _cell->unitCell() * double3(k1+1,k2,k3); // + origin();
          float4 scale = float4(0.1f,1.0f,0.1f,1.0f);
          RKInPerInstanceAttributesBonds cylinder =
                  RKInPerInstanceAttributesBonds(float4(position1,1.0),
                                                 float4(position2,1.0),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 scale,0,0);
          data.push_back(cylinder);
        }

        if(k2<=maximumReplicaY)
        {
          double3 position1 = _cell->unitCell() * double3(k1,k2,k3); // + origin();
          double3 position2 = _cell->unitCell() * double3(k1,k2+1,k3); // + origin();
          float4 scale = float4(0.1f,1.0f,0.1f,1.0f);
          RKInPerInstanceAttributesBonds cylinder =
                  RKInPerInstanceAttributesBonds(float4(position1,1.0),
                                                 float4(position2,1.0),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 scale,0,0);
          data.push_back(cylinder);
        }

        if(k3<=maximumReplicaZ)
        {
          double3 position1 = _cell->unitCell() * double3(k1,k2,k3); // + origin();
          double3 position2 = _cell->unitCell() * double3(k1,k2,k3+1); // + origin();
          float4 scale = float4(0.1f,1.0f,0.1f,1.0f);
          RKInPerInstanceAttributesBonds cylinder =
                  RKInPerInstanceAttributesBonds(float4(position1,1.0),
                                                 float4(position2,1.0),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 scale,0,0);
          data.push_back(cylinder);
        }
      }
    }
  }

  return data;
}


// MARK: Rendering selection
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> CrystalCylinderPrimitive::renderSelectedCrystalPrimitiveCylinderObjects() const
{
  std::vector<RKInPerInstanceAttributesAtoms> atomData;

  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  std::set<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->selectedTreeNodes();
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      uint32_t asymmetricAtomIndex = atom->asymmetricIndex();
      bool isVisible = atom->isVisible();

      if(isVisible)
      {
        for(std::shared_ptr<SKAtomCopy> copy : atom->copies())
        {
          if(copy->type() == SKAtomCopy::AtomCopyType::copy)
          {
            float4 ambient = float4(1.0,1.0,1.0,1.0);
            float4 diffuse = float4(1.0,1.0,1.0,1.0);
            float4 specular = float4(1.0,1.0,1.0,1.0);

            double radius = 1.0;
            float4 scale = float4(radius,radius,radius,1.0);
            double3 copyPosition = double3::flip(copy->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;

            for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
            {
              for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
              {
                for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
                {
                  float4 position = float4(_cell->unitCell() * (copyPosition + double3(k1,k2,k3)), 1.0);

                  RKInPerInstanceAttributesAtoms atom1 = RKInPerInstanceAttributesAtoms(position, ambient, diffuse, specular, scale, asymmetricAtomIndex);
                  atomData.push_back(atom1);
                }
              }
            }
          }
        }
      }
    }
  }

  return atomData;
}


// MARK: Filtering
// =====================================================================

std::set<int> CrystalCylinderPrimitive::filterCartesianAtomPositions(std::function<bool(double3)> & closure)
{
  std::set<int> data;

  if(_isVisible)
  {
    int minimumReplicaX = _cell->minimumReplicaX();
    int minimumReplicaY = _cell->minimumReplicaY();
    int minimumReplicaZ = _cell->minimumReplicaZ();

    int maximumReplicaX = _cell->maximumReplicaX();
    int maximumReplicaY = _cell->maximumReplicaY();
    int maximumReplicaZ = _cell->maximumReplicaZ();

    double3 contentShift = _cell->contentShift();
    bool3 contentFlip = _cell->contentFlip();

    double4x4 rotationMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(_orientation), boundingBox().center());

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
              double3 pos = double3::flip(copy->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
              for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
              {
                for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
                {
                  for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
                  {
                    double3 fractionalPosition = pos + double3(k1,k2,k3);
                    double3 cartesianPosition = _cell->convertToCartesian(fractionalPosition);

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

SKBoundingBox CrystalCylinderPrimitive::boundingBox() const
{
  double3 minimumReplica = _cell->minimumReplicas();
  double3 maximumReplica = _cell->maximumReplicas();

  double3 c0 = _cell->unitCell() * double3(minimumReplica);
  double3 c1 = _cell->unitCell() * double3(maximumReplica.x+1, minimumReplica.y,   minimumReplica.z);
  double3 c2 = _cell->unitCell() * double3(maximumReplica.x+1, maximumReplica.y+1, minimumReplica.z);
  double3 c3 = _cell->unitCell() * double3(minimumReplica.x,   maximumReplica.y+1, minimumReplica.z);
  double3 c4 = _cell->unitCell() * double3(minimumReplica.x,   minimumReplica.y,   maximumReplica.z+1);
  double3 c5 = _cell->unitCell() * double3(maximumReplica.x+1, minimumReplica.y,   maximumReplica.z+1);
  double3 c6 = _cell->unitCell() * double3(maximumReplica.x+1, maximumReplica.y+1, maximumReplica.z+1);
  double3 c7 = _cell->unitCell() * double3(minimumReplica.x,   maximumReplica.y+1, maximumReplica.z+1);

  double valuesX[8] = {c0.x, c1.x, c2.x, c3.x, c4.x, c5.x, c6.x, c7.x};
  double valuesY[8] = {c0.y, c1.y, c2.y, c3.y, c4.y, c5.y, c6.y, c7.y};
  double valuesZ[8] = {c0.z, c1.z, c2.z, c3.z, c4.z, c5.z, c6.z, c7.z};

  double3 minimum = double3(*std::min_element(valuesX,valuesX+8),
                            *std::min_element(valuesY,valuesY+8),
                            *std::min_element(valuesZ,valuesZ+8));

  double3 maximum = double3(*std::max_element(valuesX,valuesX+8),
                            *std::max_element(valuesY,valuesY+8),
                            *std::max_element(valuesZ,valuesZ+8));

  return SKBoundingBox(minimum,maximum);
}

void CrystalCylinderPrimitive::reComputeBoundingBox()
{
  SKBoundingBox boundingBox = this->boundingBox();

  // store in the cell datastructure
  _cell->setBoundingBox(boundingBox);
}

// MARK: Symmetry
// =====================================================================

void CrystalCylinderPrimitive::expandSymmetry()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>{};

      std::vector<double3> images = _spaceGroup.listOfSymmetricPositions(asymmetricAtom->position());

      for(double3 image : images)
      {
        std::shared_ptr<SKAtomCopy> newAtom = std::make_shared<SKAtomCopy>(asymmetricAtom, image);
        newAtom->setType(SKAtomCopy::AtomCopyType::copy);
        atomCopies.push_back(newAtom);
      }
      asymmetricAtom->setCopies(atomCopies);
    }
  }
}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> CrystalCylinderPrimitive::cellForFractionalPositions()
{
  return std::make_pair(std::make_shared<SKCell>(*_cell), double3(0.0,0.0,0.0));
}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> CrystalCylinderPrimitive::cellForCartesianPositions()
{
  return std::make_pair(std::make_shared<SKCell>(*_cell), double3(0.0,0.0,0.0));
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> CrystalCylinderPrimitive::asymmetricAtomsCopiedAndTransformedToCartesianPositions()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms{};

  double3x3 unitCell = _cell->unitCell();
  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      double3 position = unitCell * double3::fract(asymmetricAtom->position());

      std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
      newAsymmetricAtom->setPosition(position);
      atoms.push_back(newAsymmetricAtom);
    }
  }

  return atoms;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> CrystalCylinderPrimitive::asymmetricAtomsCopiedAndTransformedToFractionalPositions()
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

std::vector<std::shared_ptr<SKAsymmetricAtom>> CrystalCylinderPrimitive::atomsCopiedAndTransformedToFractionalPositions()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms{};

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
      {
        if(copy->type() == SKAtomCopy::AtomCopyType::copy)
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

std::vector<std::shared_ptr<SKAsymmetricAtom>> CrystalCylinderPrimitive::atomsCopiedAndTransformedToCartesianPositions()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms{};

  double3x3 unitCell = _cell->unitCell();
  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
      {
        if(copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          double3 position = unitCell * double3::fract(copy->position());
          std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
          newAsymmetricAtom->setPosition(position);
          atoms.push_back(newAsymmetricAtom);
        }
      }
    }
  }

  return atoms;
}

std::shared_ptr<Primitive> CrystalCylinderPrimitive::superCell() const
{
  qDebug() << "CrystalCylinderPrimitive::superCell";

  // use the copy constructor
  std::shared_ptr<CrystalCylinderPrimitive> crystal = std::make_shared<CrystalCylinderPrimitive>(static_cast<const CrystalCylinderPrimitive&>(*this));

  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  int dx = maximumReplicaX-minimumReplicaX;
  int dy = maximumReplicaY-minimumReplicaY;
  int dz = maximumReplicaZ-minimumReplicaZ;
  for(int k1=0;k1<=dx;k1++)
  {
    for(int k2=0;k2<=dy;k2++)
    {
      for(int k3=0;k3<=dz;k3++)
      {

        for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
        {
          if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
          {
            for(const std::shared_ptr<SKAtomCopy> &atomCopy : asymmetricAtom->copies())
            {
              if(atomCopy->type() == SKAtomCopy::AtomCopyType::copy)
              {
                double3 position = (atomCopy->position() + double3(k1,k2,k3)) / double3(dx+1, dy+1, dz+1);

                std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
                newAsymmetricAtom->setPosition(position);
                std::shared_ptr<SKAtomCopy> newAtomCopy = std::make_shared<SKAtomCopy>(newAsymmetricAtom, position);
                newAsymmetricAtom->setCopies({newAtomCopy});
                std::shared_ptr<SKAtomTreeNode> atomTreeNode = std::make_shared<SKAtomTreeNode>(newAsymmetricAtom);
                crystal->atomsTreeController()->appendToRootnodes(atomTreeNode);
              }
            }
          }
        }
      }
    }
  }

  crystal->setCell(_cell->superCell());
  crystal->setOrigin(this->origin() + _cell->unitCell() * double3(minimumReplicaX, minimumReplicaY, minimumReplicaZ));
  crystal->atomsTreeController()->setTags();
  crystal->reComputeBoundingBox();
//  crystal->computeBonds();

  return crystal;
}

std::shared_ptr<Primitive> CrystalCylinderPrimitive::flattenHierarchy() const
{
  std::shared_ptr<CrystalCylinderPrimitive> cylinderCrystal = std::make_shared<CrystalCylinderPrimitive>(static_cast<const CrystalCylinderPrimitive&>(*this));

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
      cylinderCrystal->atomsTreeController()->appendToRootnodes(atomTreeNode);
    }
  }

  cylinderCrystal->atomsTreeController()->setTags();
  cylinderCrystal->reComputeBoundingBox();
 // cylinderCrystal->computeBonds();

  return cylinderCrystal;
}

std::shared_ptr<Primitive> CrystalCylinderPrimitive::appliedCellContentShift() const
{
  // use the copy constructor
  std::shared_ptr<CrystalCylinderPrimitive> cylinderCrystal = std::make_shared<CrystalCylinderPrimitive>(static_cast<const CrystalCylinderPrimitive&>(*this));
  cylinderCrystal->_spaceGroup = this->_spaceGroup;

  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << _atomsTreeController;
  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  streamRead >> cylinderCrystal->_atomsTreeController;

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = cylinderCrystal->atomsTreeController()->flattenedLeafNodes();

  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      double3 position = double3::flip(asymmetricAtom->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
      asymmetricAtom->setPosition(double3::fract(position));
    }
  }

  cylinderCrystal->expandSymmetry();
  cylinderCrystal->setCell(std::make_shared<SKCell>(*_cell));
  cylinderCrystal->cell()->setContentFlip(bool3(false,false,false));
  cylinderCrystal->cell()->setContentShift(double3(0.0,0.0,0.0));
  cylinderCrystal->atomsTreeController()->setTags();
  cylinderCrystal->reComputeBoundingBox();
 // cylinderCrystal->computeBonds();

  return cylinderCrystal;
}


// MARK: bond-computations
// =====================================================================

// MARK: Translation operations
// =====================================================================

double3 CrystalCylinderPrimitive::centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms) const
{
  double3 centerOfMassCosTheta = double3(0.0, 0.0, 0.0);
  double3 centerOfMassSinTheta = double3(0.0, 0.0, 0.0);
  double M = 0.0;

  for(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom : asymmetricAtoms)
  {
    for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
    {
      if(copy->type() == SKAtomCopy::AtomCopyType::copy)
      {
        double3 pos = copy->position() * 2.0 * M_PI;
        double3 cosTheta = double3(cos(pos.x), cos(pos.y), cos(pos.z));
        double3 sinTheta = double3(sin(pos.x), sin(pos.y), sin(pos.z));
        centerOfMassCosTheta = centerOfMassCosTheta + cosTheta;
        centerOfMassSinTheta = centerOfMassSinTheta + sinTheta;
        M += 1.0;
      }
    }
  }
  centerOfMassCosTheta = centerOfMassCosTheta / M;
  centerOfMassSinTheta = centerOfMassSinTheta / M;

  double3 com = double3((atan2(-centerOfMassSinTheta.x, -centerOfMassCosTheta.x) + M_PI)/(2.0 * M_PI),
                        (atan2(-centerOfMassSinTheta.y, -centerOfMassCosTheta.y) + M_PI)/(2.0 * M_PI),
                        (atan2(-centerOfMassSinTheta.z, -centerOfMassCosTheta.z) + M_PI)/(2.0 * M_PI));

  return com;
}

double3x3 CrystalCylinderPrimitive::matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const
{
  double3x3 inertiaMatrix = double3x3();
  double3 comFrac = centerOfMassOfSelectionAsymmetricAtoms(atoms);

  for(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom : atoms)
  {
    for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
    {
      if(copy->type() == SKAtomCopy::AtomCopyType::copy)
      {
        double3 ds = copy->position() - comFrac;
        ds.x -= std::rint(ds.x);
        ds.y -= std::rint(ds.y);
        ds.z -= std::rint(ds.z);
        double3 dr = _cell->convertToCartesian(ds);

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


std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> CrystalCylinderPrimitive::translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  double3 fractionalTranslation = _cell->convertToFractional(translation);

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [fractionalTranslation](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                    {return std::make_pair(atom, atom->position() + fractionalTranslation);});

  return translatedPositions;
}


std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> CrystalCylinderPrimitive::translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  double3 translationBodyFrame = _selectionBodyFixedBasis * translation;
  double3 fractionalTranslation = _cell->convertToFractional(translationBodyFrame);

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [fractionalTranslation](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                    {return std::make_pair(atom, atom->position() + fractionalTranslation);});

  return translatedPositions;
}


std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> CrystalCylinderPrimitive::rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  double3 comFrac = centerOfMassOfSelectionAsymmetricAtoms(atoms);
  double3x3 rotationMatrix = double3x3(rotation);

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(rotatedPositions),
                 [this,comFrac,rotationMatrix](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                 {
                   double3 ds = atom->position() - comFrac;
                   double3 translatedPositionCartesian = _cell->convertToCartesian(ds);
                   double3 position = rotationMatrix * translatedPositionCartesian;
                   return std::make_pair(atom, _cell->convertToFractional(position) + comFrac);
                 });

  return rotatedPositions;
}


std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> CrystalCylinderPrimitive::rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  double3 comFrac = centerOfMassOfSelectionAsymmetricAtoms(atoms);
  double3x3 rotationMatrix =  _selectionBodyFixedBasis * double3x3(rotation) * double3x3::inverse(_selectionBodyFixedBasis);

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(rotatedPositions),
                 [this,comFrac,rotationMatrix](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                 {
                   double3 ds = atom->position() - comFrac;
                   double3 translatedPositionCartesian = _cell->convertToCartesian(ds);
                   double3 position = rotationMatrix * translatedPositionCartesian;
                   return std::make_pair(atom, _cell->convertToFractional(position) + comFrac);
                 });

  return rotatedPositions;
}


QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<CrystalCylinderPrimitive> &primitive)
{
  stream << primitive->_versionNumber;

  stream << qint64(0x6f6b6190);

  // handle super class
  stream << std::static_pointer_cast<Primitive>(primitive);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<CrystalCylinderPrimitive> &primitive)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > primitive->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "CrystalCylinderPrimitive");
  }

  // Legacy reading of the primitive from a 'Structure' (remove in the future)
  if(versionNumber <= 1)
  {
    std::shared_ptr<Structure> structure = std::make_shared<Structure>();
    stream >> structure;

    primitive->_authorFirstName = structure->authorFirstName();
    primitive->_authorMiddleName = structure->authorMiddleName();
    primitive->_authorLastName = structure->authorLastName();
    primitive->_authorOrchidID = structure->authorOrchidID();
    primitive->_authorResearcherID = structure->authorResearcherID();
    primitive->_authorAffiliationUniversityName = structure->authorAffiliationUniversityName();
    primitive->_authorAffiliationFacultyName = structure->authorAffiliationFacultyName();
    primitive->_authorAffiliationInstituteName = structure->authorAffiliationInstituteName();
    primitive->_authorAffiliationCityName = structure->authorAffiliationCityName();
    primitive->_authorAffiliationCountryName = structure->authorAffiliationCountryName();
    primitive->_creationDate = structure->creationDate();

    primitive->_displayName = structure->displayName();
    primitive->_isVisible = structure->isVisible();

    primitive->_cell = structure->cell();
    primitive->_periodic = true;
    primitive->_origin = structure->origin();
    primitive->_scaling = double3(1.0, 1.0, 1.0);
    primitive->_orientation = structure->orientation();
    primitive->_rotationDelta = structure->rotationDelta();

    primitive->_drawUnitCell = structure->drawUnitCell();
    primitive->_unitCellScaleFactor = structure->unitCellScaleFactor();
    primitive->_unitCellDiffuseColor = structure->unitCellDiffuseColor();
    primitive->_unitCellDiffuseIntensity = structure->unitCellDiffuseIntensity();

    primitive->_localAxes = structure->renderLocalAxes();

    primitive->_drawAtoms = structure->drawAtoms();
    primitive->_atomsTreeController = structure->atomsTreeController();

    primitive->_primitiveTransformationMatrix = structure->transformationMatrix();
    primitive->_primitiveOrientation = structure->primitiveOrientation();
    primitive->_primitiveRotationDelta = structure->primitiveRotationDelta();

    primitive->_primitiveOpacity = structure->primitiveOpacity();
    primitive->_primitiveIsCapped = structure->primitiveIsCapped();
    primitive->_primitiveIsFractional = true;
    primitive->_primitiveNumberOfSides = structure->primitiveNumberOfSides();
    primitive->_primitiveThickness = 0.05;

    primitive->_primitiveHue = structure->primitiveHue();
    primitive->_primitiveSaturation = structure->primitiveSaturation();
    primitive->_primitiveValue = structure->primitiveValue();

    primitive->_primitiveSelectionStyle = structure->primitiveSelectionStyle();
    primitive->_primitiveSelectionStripesDensity = structure->primitiveSelectionStripesDensity();
    primitive->_primitiveSelectionStripesFrequency = structure->primitiveSelectionStripesFrequency();
    primitive->_primitiveSelectionWorleyNoise3DFrequency = structure->primitiveSelectionWorleyNoise3DFrequency();
    primitive->_primitiveSelectionWorleyNoise3DJitter = structure->primitiveSelectionWorleyNoise3DJitter();
    primitive->_primitiveSelectionScaling = structure->primitiveSelectionScaling();
    primitive->_primitiveSelectionIntensity = structure->primitiveSelectionIntensity();

    primitive->_primitiveFrontSideHDR = structure->frontPrimitiveHDR();
    primitive->_primitiveFrontSideHDRExposure = structure->frontPrimitiveHDRExposure();
    primitive->_primitiveFrontSideAmbientColor = structure->frontPrimitiveAmbientColor();
    primitive->_primitiveFrontSideDiffuseColor = structure->frontPrimitiveDiffuseColor();
    primitive->_primitiveFrontSideSpecularColor = structure->frontPrimitiveSpecularColor();
    primitive->_primitiveFrontSideAmbientIntensity = structure->frontPrimitiveAmbientIntensity();
    primitive->_primitiveFrontSideDiffuseIntensity = structure->frontPrimitiveDiffuseIntensity();
    primitive->_primitiveFrontSideSpecularIntensity = structure->frontPrimitiveSpecularIntensity();
    primitive->_primitiveFrontSideShininess = structure->frontPrimitiveShininess();

    primitive->_primitiveBackSideHDR = structure->backPrimitiveHDR();
    primitive->_primitiveBackSideHDRExposure = structure->backPrimitiveHDRExposure();
    primitive->_primitiveBackSideAmbientColor = structure->backPrimitiveAmbientColor();
    primitive->_primitiveBackSideDiffuseColor = structure->backPrimitiveDiffuseColor();
    primitive->_primitiveBackSideSpecularColor = structure->backPrimitiveSpecularColor();
    primitive->_primitiveBackSideAmbientIntensity = structure->backPrimitiveAmbientIntensity();
    primitive->_primitiveBackSideDiffuseIntensity = structure->backPrimitiveDiffuseIntensity();
    primitive->_primitiveBackSideSpecularIntensity = structure->backPrimitiveSpecularIntensity();
    primitive->_primitiveBackSideShininess = structure->backPrimitiveShininess();
  }
  else
  {
    qint64 magicNumber;
    stream >> magicNumber;
    if(magicNumber != qint64(0x6f6b6190))
    {
      throw InvalidArchiveVersionException(__FILE__, __LINE__, "CrystalCylinderPrimitive invalid magic-number");
    }

    std::shared_ptr<Primitive> structure = std::static_pointer_cast<Primitive>(primitive);
    stream >> structure;
  }

  return stream;
}
