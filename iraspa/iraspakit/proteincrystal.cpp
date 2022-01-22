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

ProteinCrystal::ProteinCrystal()
{
}

ProteinCrystal::ProteinCrystal(const ProteinCrystal &proteinCrystal): Structure(proteinCrystal)
{
}


ProteinCrystal::ProteinCrystal(std::shared_ptr<SKStructure> frame): Structure(frame)
{
  if(frame->spaceGroupHallNumber)
  {
    this->_spaceGroup = *(frame->spaceGroupHallNumber);
  }

  expandSymmetry();
  _atomsTreeController->setTags();
}

ProteinCrystal::ProteinCrystal(const std::shared_ptr<Object> object): Structure(object)
{
  if (std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(object))
  {
    if(!atomViewer->isFractional())
    {
      convertAsymmetricAtomsToFractional();
      expandSymmetry();
    }
    _atomsTreeController->setTags();
    reComputeBoundingBox();
    computeBonds();
  }
}

std::shared_ptr<Object> ProteinCrystal::shallowClone()
{
  return std::make_shared<ProteinCrystal>(static_cast<const ProteinCrystal&>(*this));
}

// MARK: Rendering
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> ProteinCrystal::renderAtoms() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::vector<RKInPerInstanceAttributesAtoms> atomData = std::vector<RKInPerInstanceAttributesAtoms>();

  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      uint32_t asymmetricAtomIndex = atom->asymmetricIndex();
      bool isVisible = atom->isVisible();
      double w = isVisible ? 1.0 : -1.0;

      QColor color = atom->color();
      float4 ambient = float4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
      float4 diffuse = float4(color.redF(), color.greenF(), color.blueF(), color.alphaF());

      float4 specular = float4(1.0, 1.0, 1.0, 1.0);

      double radius = atom->drawRadius(); // * atom->occupancy();
      float4 scale = float4(radius, radius, radius, 1.0);
      for (std::shared_ptr<SKAtomCopy> copy : atom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          double3 fractionalPosition = _cell->inverseUnitCell()  * (copy->position() + contentShift);
          double3 adjustedFractionalPosition = double3::flip(fractionalPosition, contentFlip, double3(1.0,1.0,1.0));

          for (int k1 = minimumReplicaX;k1 <= maximumReplicaX;k1++)
          {
            for (int k2 = minimumReplicaY;k2 <= maximumReplicaY;k2++)
            {
              for (int k3 = minimumReplicaZ;k3 <= maximumReplicaZ;k3++)
              {
                double3 CartesianPosition = _cell->unitCell() * (adjustedFractionalPosition + double3(k1, k2, k3));
                float4 position = float4(CartesianPosition, w);

                RKInPerInstanceAttributesAtoms atom1 = RKInPerInstanceAttributesAtoms(position, ambient, diffuse, specular, scale, asymmetricAtomIndex);
                atomData.push_back(atom1);
              }
            }
          }
        }
      }
    }
  }

  return atomData;
}

std::vector<RKInPerInstanceAttributesBonds> ProteinCrystal::renderInternalBonds() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::vector<RKInPerInstanceAttributesBonds> data =  std::vector<RKInPerInstanceAttributesBonds>();

  const std::vector<std::shared_ptr<SKAsymmetricBond>> asymmetricBonds = _bondSetController->arrangedObjects();

  uint32_t asymmetricBondIndex = 0;
  for(std::shared_ptr<SKAsymmetricBond> asymmetricBond : asymmetricBonds)
  {
    bool isVisble = asymmetricBond->isVisible() && asymmetricBond->atom1()->isVisible()  && asymmetricBond->atom2()->isVisible();

    if(isVisble)
    {
      const std::vector<std::shared_ptr<SKBond>> bonds = asymmetricBond->copies();
      for(std::shared_ptr<SKBond> bond : bonds)
      {
        if(bond->atom1()->type() == SKAtomCopy::AtomCopyType::copy && bond->atom2()->type() == SKAtomCopy::AtomCopyType::copy && bond->boundaryType() == SKBond::BoundaryType::internal)
        {
          QColor color1 = bond->atom1()->parent()->color();
          QColor color2 = bond->atom2()->parent()->color();

          double3 copyPosition1 = double3::flip(_cell->inverseUnitCell() * bond->atom1()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
          double3 copyPosition2 = double3::flip(_cell->inverseUnitCell() * bond->atom2()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;

          for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
          {
            for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
            {
              for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
              {
                double3 pos1 = _cell->unitCell() * (copyPosition1 + double3(k1,k2,k3));
                double3 pos2 = _cell->unitCell() * (copyPosition2 + double3(k1,k2,k3));
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
      }
    }
    asymmetricBondIndex++;
  }

  return data;
}


std::vector<RKInPerInstanceAttributesAtoms> ProteinCrystal::renderUnitCellSpheres() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  double3 boundingBoxWidths = _cell->boundingBox().widths();
  double scaleFactor = 0.0025 * std::max({boundingBoxWidths.x,boundingBoxWidths.y,boundingBoxWidths.z});

  std::vector<RKInPerInstanceAttributesAtoms> data = std::vector<RKInPerInstanceAttributesAtoms>();

  uint32_t asymmetricBondIndex=0;
  for (int k1 = minimumReplicaX;k1 <= maximumReplicaX + 1;k1++)
  {
    for (int k2 = minimumReplicaY;k2 <= maximumReplicaY + 1;k2++)
    {
      for (int k3 = minimumReplicaZ;k3 <= maximumReplicaZ + 1;k3++)
      {
        double3 position = _cell->unitCell() * double3(k1, k2, k3); // + origin();
        float4 ambient = float4(1.0f, 1.0f, 1.0f, 1.0f);
        float4 diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
        float4 specular = float4(1.0f, 1.0f, 1.0f, 1.0f);
        float4 scale = float4(scaleFactor, scaleFactor, scaleFactor, 1.0f);
        RKInPerInstanceAttributesAtoms sphere = RKInPerInstanceAttributesAtoms(float4(position, 1.0), ambient, diffuse, specular, scale, asymmetricBondIndex);
        data.push_back(sphere);
      }
    }
    asymmetricBondIndex++;
  }

  return data;
}

std::vector<RKInPerInstanceAttributesBonds> ProteinCrystal::renderUnitCellCylinders() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  double3 boundingBoxWidths = _cell->boundingBox().widths();
  double scaleFactor = 0.0025 * std::max({boundingBoxWidths.x,boundingBoxWidths.y,boundingBoxWidths.z});

  std::vector<RKInPerInstanceAttributesBonds> data = std::vector<RKInPerInstanceAttributesBonds>();

  for (int k1 = minimumReplicaX;k1 <= maximumReplicaX + 1;k1++)
  {
    for (int k2 = minimumReplicaY;k2 <= maximumReplicaY + 1;k2++)
    {
      for (int k3 = minimumReplicaZ;k3 <= maximumReplicaZ + 1;k3++)
      {
        if (k1 <= maximumReplicaX)
        {
          double3 position1 = _cell->unitCell() * double3(k1, k2, k3);
          double3 position2 = _cell->unitCell() * double3(k1 + 1, k2, k3);
          float4 scale = float4(scaleFactor, 1.0f, scaleFactor, 1.0f);
          RKInPerInstanceAttributesBonds cylinder =
            RKInPerInstanceAttributesBonds(float4(position1, 1.0),
              float4(position2, 1.0),
              float4(1.0f, 1.0f, 1.0f, 1.0f),
              float4(1.0f, 1.0f, 1.0f, 1.0f),
              scale,0,0);
          data.push_back(cylinder);
        }

        if (k2 <= maximumReplicaY)
        {
          double3 position1 = _cell->unitCell() * double3(k1, k2, k3);
          double3 position2 = _cell->unitCell() * double3(k1, k2 + 1, k3);
          float4 scale = float4(scaleFactor, 1.0f, scaleFactor, 1.0f);
          RKInPerInstanceAttributesBonds cylinder =
            RKInPerInstanceAttributesBonds(float4(position1, 1.0),
              float4(position2, 1.0),
              float4(1.0f, 1.0f, 1.0f, 1.0f),
              float4(1.0f, 1.0f, 1.0f, 1.0f),
              scale,0,0);
          data.push_back(cylinder);
        }

        if (k3 <= maximumReplicaZ)
        {
          double3 position1 = _cell->unitCell() * double3(k1, k2, k3);
          double3 position2 = _cell->unitCell() * double3(k1, k2, k3 + 1);
          float4 scale = float4(scaleFactor, 1.0f, scaleFactor, 1.0f);
          RKInPerInstanceAttributesBonds cylinder =
            RKInPerInstanceAttributesBonds(float4(position1, 1.0),
              float4(position2, 1.0),
              float4(1.0f, 1.0f, 1.0f, 1.0f),
              float4(1.0f, 1.0f, 1.0f, 1.0f),
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

std::vector<RKInPerInstanceAttributesAtoms> ProteinCrystal::renderSelectedAtoms() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

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
        double radius = atom->drawRadius(); // * atom->occupancy();
        float4 scale = float4(radius, radius, radius, 1.0);

        for (std::shared_ptr<SKAtomCopy> copy : atom->copies())
        {
          if (copy->type() == SKAtomCopy::AtomCopyType::copy)
          {
            double3 fractionalPosition = _cell->inverseUnitCell()  * (copy->position() + contentShift);
            double3 adjustedFractionalPosition = double3::flip(fractionalPosition, contentFlip, double3(1.0,1.0,1.0));

            for (int k1 = minimumReplicaX;k1 <= maximumReplicaX;k1++)
            {
              for (int k2 = minimumReplicaY;k2 <= maximumReplicaY;k2++)
              {
                for (int k3 = minimumReplicaZ;k3 <= maximumReplicaZ;k3++)
                {
                  double3 CartesianPosition = _cell->unitCell() * (adjustedFractionalPosition + double3(k1, k2, k3));
                  float4 position = float4(CartesianPosition, 1.0);

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

std::vector<RKInPerInstanceAttributesBonds> ProteinCrystal::renderSelectedInternalBonds() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

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
          double3 fractionalPosition1 = _cell->inverseUnitCell()  * (bond->atom1()->position() + contentShift);
          double3 adjustedFractionalPosition1 = double3::flip(fractionalPosition1, contentFlip, double3(1.0,1.0,1.0));
          double3 fractionalPosition2 = _cell->inverseUnitCell()  * (bond->atom2()->position() + contentShift);
          double3 adjustedFractionalPosition2 = double3::flip(fractionalPosition2, contentFlip, double3(1.0,1.0,1.0));

          QColor color1 = bond->atom1()->parent()->color();
          QColor color2 = bond->atom2()->parent()->color();

          for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
          {
            for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
            {
              for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
              {
                double3 pos1 = _cell->unitCell() * (double3::fract(adjustedFractionalPosition1) + double3(k1, k2, k3));
                double3 pos2 = _cell->unitCell() * (double3::fract(adjustedFractionalPosition2) + double3(k1, k2, k3));

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
      }
    }
  }

  return data;
}


// MARK: Filtering
// =====================================================================

std::set<int> ProteinCrystal::filterCartesianAtomPositions(std::function<bool(double3)> & closure)
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
              double3 fractionalPosition = _cell->inverseUnitCell()  * (copy->position() + contentShift);
              double3 adjustedFractionalPosition = double3::flip(fractionalPosition, contentFlip, double3(1.0,1.0,1.0));

              for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
              {
                for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
                {
                  for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
                  {
                    double3 CartesianPosition = _cell->unitCell() * (adjustedFractionalPosition + double3(k1, k2, k3));

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
          }
        }
      }
      asymmetricAtomIndex++;
    }
  }

  return data;
}

BondSelectionIndexSet ProteinCrystal::filterCartesianBondPositions(std::function<bool(double3)> &closure)
{
  BondSelectionIndexSet data;

  if(_isVisible)
  {
    int minimumReplicaX = _cell->minimumReplicaX();
    int minimumReplicaY = _cell->minimumReplicaY();
    int minimumReplicaZ = _cell->minimumReplicaZ();

    int maximumReplicaX = _cell->maximumReplicaX();
    int maximumReplicaY = _cell->maximumReplicaY();
    int maximumReplicaZ = _cell->maximumReplicaZ();

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
          double3 fractionalPosition1 = _cell->inverseUnitCell()  * (bond->atom1()->position() + contentShift);
          double3 adjustedFractionalPosition1 = double3::flip(fractionalPosition1, contentFlip, double3(1.0,1.0,1.0));
          double3 fractionalPosition2 = _cell->inverseUnitCell()  * (bond->atom2()->position() + contentShift);
          double3 adjustedFractionalPosition2 = double3::flip(fractionalPosition2, contentFlip, double3(1.0,1.0,1.0));

          for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
          {
            for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
            {
              for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
              {
                double3 pos1 = _cell->unitCell() * (double3::fract(adjustedFractionalPosition1) + double3(k1, k2, k3));
                double3 pos2 = _cell->unitCell() * (double3::fract(adjustedFractionalPosition2) + double3(k1, k2, k3));

                double3 dr = (pos2 -pos1);

                double3 cartesianPosition1 = pos1 + 0.5 * dr;
                double4 position1 = rotationMatrix * double4(cartesianPosition1.x, cartesianPosition1.y, cartesianPosition1.z, 1.0);
                double3 absoluteCartesianPosition1 = double3(position1.x,position1.y,position1.z) + _origin;
                if (closure(absoluteCartesianPosition1))
                {
                  data.insert(asymmetricBondIndex);
                }

                double3 cartesianPosition2 = pos2 - 0.5 * dr ;
                double4 position2 = rotationMatrix * double4(cartesianPosition2.x, cartesianPosition2.y, cartesianPosition2.z, 1.0);
                double3 absoluteCartesianPosition2 = double3(position2.x,position2.y,position2.z) + _origin;
                if (closure(absoluteCartesianPosition2))
                {
                  data.insert(asymmetricBondIndex);
                }
              }
            }
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

SKBoundingBox ProteinCrystal::boundingBox() const
{
  double3 minimum = double3(1e10, 1e10, 1e10);
  double3 maximum = double3(-1e10, -1e10, -1e10);

  if (_drawUnitCell)
  {
    minimum = _cell->enclosingBoundingBox().minimum();
    maximum = _cell->enclosingBoundingBox().maximum();
  }

  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

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
          for (int k1 = minimumReplicaX;k1 <= maximumReplicaX;k1++)
          {
            for (int k2 = minimumReplicaY;k2 <= maximumReplicaY;k2++)
            {
              for (int k3 = minimumReplicaZ;k3 <= maximumReplicaZ;k3++)
              {

                double3 CartesianPosition = pos + _cell->unitCell() *  double3(k1, k2, k3);
                minimum.x = std::min(minimum.x, CartesianPosition.x);
                minimum.y = std::min(minimum.y, CartesianPosition.y);
                minimum.z = std::min(minimum.z, CartesianPosition.z);
                maximum.x = std::max(maximum.x, CartesianPosition.x);
                maximum.y = std::max(maximum.y, CartesianPosition.y);
                maximum.z = std::max(maximum.z, CartesianPosition.z);
              }
            }
          }
        }
      }
    }
  }

  return SKBoundingBox(minimum, maximum);
}

void ProteinCrystal::reComputeBoundingBox()
{
  SKBoundingBox boundingBox = this->boundingBox();

  // store in the cell datastructure
  _cell->setBoundingBox(boundingBox);
}



// MARK: Symmetry
// =====================================================================

void ProteinCrystal::expandSymmetry()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>{};

      double3 fractionalPosition = _cell->convertToFractional(asymmetricAtom->position());
      std::vector<double3> images = _spaceGroup.listOfSymmetricPositions(fractionalPosition);

      for (double3 image : images)
      {
        double3 CartesianPosition = _cell->convertToCartesian(image);
        std::shared_ptr<SKAtomCopy> newAtom = std::make_shared<SKAtomCopy>(asymmetricAtom, CartesianPosition);
        newAtom->setType(SKAtomCopy::AtomCopyType::copy);
        atomCopies.push_back(newAtom);
      }
      asymmetricAtom->setCopies(atomCopies);
    }
  }
}

void ProteinCrystal::expandSymmetry(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom)
{
  std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>{};

  double3 fractionalPosition = _cell->convertToFractional(asymmetricAtom->position());
  std::vector<double3> images = _spaceGroup.listOfSymmetricPositions(fractionalPosition);

  for (double3 image : images)
  {
    double3 CartesianPosition = _cell->convertToCartesian(image);
    std::shared_ptr<SKAtomCopy> newAtom = std::make_shared<SKAtomCopy>(asymmetricAtom, CartesianPosition);
    newAtom->setType(SKAtomCopy::AtomCopyType::copy);
    atomCopies.push_back(newAtom);
  }
  asymmetricAtom->setCopies(atomCopies);
}

std::vector<std::tuple<double3, int, double>> ProteinCrystal::atomSymmetryData() const
{
  std::vector<std::tuple<double3, int, double>> atomData{};

  double3x3 unitCellInverse = _cell->inverseUnitCell();
  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      int elementIdentifier = asymmetricAtom->elementIdentifier();
      for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
      {
        if(copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          atomData.push_back(std::make_tuple(double3::fract(unitCellInverse * copy->position()), elementIdentifier, 1.0));
        }
      }
    }
  }

  return atomData;
}

void ProteinCrystal::setAtomSymmetryData(double3x3 unitCell, std::vector<std::tuple<double3, int, double>> atomData)
{
  _cell = std::make_shared<SKCell>(unitCell);

  for(const std::tuple<double3, int, double> &tuple : atomData)
  {
    double3 position = std::get<0>(tuple);
    int elementIdentifier = std::get<1>(tuple);
    double occupancy = std::get<2>(tuple);

    QString displayName = PredefinedElements::predefinedElements[elementIdentifier]._chemicalSymbol;
    std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = std::make_shared<SKAsymmetricAtom>(displayName, elementIdentifier, occupancy);
    asymmetricAtom->setPosition(unitCell * position);
    std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(asymmetricAtom);
    atomtreeNode->setDisplayName(displayName);
    _atomsTreeController->appendToRootnodes(atomtreeNode);
  }
}

std::shared_ptr<Structure> ProteinCrystal::flattenHierarchy() const
{
  std::shared_ptr<ProteinCrystal> proteinCrystal = std::make_shared<ProteinCrystal>(static_cast<const ProteinCrystal&>(*this));

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
      proteinCrystal->atomsTreeController()->appendToRootnodes(atomTreeNode);
    }
  }

  proteinCrystal->atomsTreeController()->setTags();
  proteinCrystal->reComputeBoundingBox();
  proteinCrystal->computeBonds();

  return proteinCrystal;
}

std::shared_ptr<Structure> ProteinCrystal::appliedCellContentShift() const
{
  // use the copy constructor
  std::shared_ptr<ProteinCrystal> proteinCrystal = std::make_shared<ProteinCrystal>(static_cast<const ProteinCrystal&>(*this));
  proteinCrystal->_spaceGroup = this->_spaceGroup;

  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << _atomsTreeController;
  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  streamRead >> proteinCrystal->_atomsTreeController;

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = proteinCrystal->atomsTreeController()->flattenedLeafNodes();

  double3x3 unitCell = _cell->unitCell();
  double3x3 inverseUnitCell = _cell->inverseUnitCell();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      double3 fractionalPosition = inverseUnitCell * (asymmetricAtom->position()  + contentShift);
      double3 position = double3::flip(fractionalPosition, contentFlip, double3(0.0,0.0,0.0));
      asymmetricAtom->setPosition(unitCell * double3::fract(position));
    }
  }

  proteinCrystal->expandSymmetry();
  proteinCrystal->setCell(std::make_shared<SKCell>(*_cell));
  proteinCrystal->cell()->setContentFlip(bool3(false,false,false));
  proteinCrystal->cell()->setContentShift(double3(0.0,0.0,0.0));
  proteinCrystal->atomsTreeController()->setTags();
  proteinCrystal->reComputeBoundingBox();
  proteinCrystal->computeBonds();

  return proteinCrystal;
}


std::optional<std::pair<std::shared_ptr<SKCell>, double3>> ProteinCrystal::cellForFractionalPositions()
{
  return std::make_pair(std::make_shared<SKCell>(*_cell), double3(0.0,0.0,0.0));
}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> ProteinCrystal::cellForCartesianPositions()
{
  return std::make_pair(std::make_shared<SKCell>(*_cell), double3(0.0,0.0,0.0));
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> ProteinCrystal::asymmetricAtomsCopiedAndTransformedToCartesianPositions()
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

std::vector<std::shared_ptr<SKAsymmetricAtom>> ProteinCrystal::asymmetricAtomsCopiedAndTransformedToFractionalPositions()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms{};

  double3x3 inverseUnitCell = _cell->inverseUnitCell();
  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom  = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
      double3 position = inverseUnitCell * newAsymmetricAtom->position();
      newAsymmetricAtom->setPosition(position);
      atoms.push_back(newAsymmetricAtom);
    }
  }

  return atoms;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> ProteinCrystal::atomsCopiedAndTransformedToCartesianPositions()
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
          std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom  = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
          newAsymmetricAtom->setPosition(copy->position());
          atoms.push_back(newAsymmetricAtom);
        }
      }
    }
  }

  return atoms;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> ProteinCrystal::atomsCopiedAndTransformedToFractionalPositions()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms{};

  double3x3 inverseUnitCell = _cell->inverseUnitCell();
  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      for (std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom  = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
          double3 position = inverseUnitCell * copy->position();
          newAsymmetricAtom->setPosition(position);
          atoms.push_back(newAsymmetricAtom);
        }
      }
    }
  }

  return atoms;
}


std::shared_ptr<Structure> ProteinCrystal::superCell() const
{
  qDebug() << "Crystal::superCell";

  // use the copy constructor
  std::shared_ptr<ProteinCrystal> crystal = std::make_shared<ProteinCrystal>(static_cast<const ProteinCrystal&>(*this));

  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  double3x3 unitCell = _cell->unitCell();
  double3x3 inverseUnitCell = _cell->inverseUnitCell();
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
                double3 fractionalPosition = (inverseUnitCell * atomCopy->position() + double3(k1,k2,k3)) / double3(dx+1, dy+1, dz+1);
                double3 CartesianPosition = unitCell * fractionalPosition;

                std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom);
                newAsymmetricAtom->setPosition(CartesianPosition);
                std::shared_ptr<SKAtomCopy> newAtomCopy = std::make_shared<SKAtomCopy>(newAsymmetricAtom, CartesianPosition);
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
  crystal->computeBonds();

  return crystal;
}

std::shared_ptr<Structure> ProteinCrystal::removeSymmetry() const
{
  // use the copy constructor
  std::shared_ptr<ProteinCrystal> crystal = std::make_shared<ProteinCrystal>(static_cast<const ProteinCrystal&>(*this));

  // copy all atoms
  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << _atomsTreeController;
  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  streamRead >> crystal->_atomsTreeController;

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = crystal->atomsTreeController()->flattenedLeafNodes();

  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(node->isLeaf())
    {
      node->setIsGroup(true);

      if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
      {
        for(const std::shared_ptr<SKAtomCopy> &atomCopy : asymmetricAtom->copies())
        {
          std::shared_ptr<SKAsymmetricAtom> newAsymmetricAtom = std::make_shared<SKAsymmetricAtom>(*asymmetricAtom.get());
          newAsymmetricAtom->setPosition(atomCopy->position());

          std::shared_ptr<SKAtomCopy> copy = std::make_shared<SKAtomCopy>(newAsymmetricAtom, atomCopy->position());
          newAsymmetricAtom->setCopies({copy});

          std::shared_ptr<SKAtomTreeNode> newNode = std::make_shared<SKAtomTreeNode>(newAsymmetricAtom);
          newNode->appendToParent(node);
        }
      }
    }
  }

  crystal->setSpaceGroupHallNumber(1);
  crystal->atomsTreeController()->setTags();
  crystal->reComputeBoundingBox();
  crystal->computeBonds();

  return crystal;
}

std::shared_ptr<Structure> ProteinCrystal::wrapAtomsToCell() const
{
  // use the copy constructor
  std::shared_ptr<ProteinCrystal> proteinCrystal = std::make_shared<ProteinCrystal>(static_cast<const ProteinCrystal&>(*this));

  // copy all atoms
  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << _atomsTreeController;
  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  streamRead >> proteinCrystal->_atomsTreeController;

  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = proteinCrystal->atomsTreeController()->flattenedLeafNodes();

  double3x3 unitCell = _cell->unitCell();
  double3x3 inverseUnitcell = _cell->inverseUnitCell();
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> &asymmetricAtom = node->representedObject())
    {
      double3 asymmetricAtomPosition = unitCell * double3::fract(inverseUnitcell * asymmetricAtom->position());
      asymmetricAtom->setPosition(asymmetricAtomPosition);
      for(const std::shared_ptr<SKAtomCopy> &atomCopy : asymmetricAtom->copies())
      {
        double3 position = unitCell * double3::fract(inverseUnitcell * atomCopy->position());
        atomCopy->setPosition(position);
      }
    }
  }

  proteinCrystal->atomsTreeController()->setTags();
  proteinCrystal->reComputeBoundingBox();
  proteinCrystal->computeBonds();

  return proteinCrystal;
}



// MARK: bond-computations
// =====================================================================

std::vector<double3> ProteinCrystal::atomPositions() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

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
          for (int k1 = minimumReplicaX;k1 <= maximumReplicaX;k1++)
          {
            for (int k2 = minimumReplicaY;k2 <= maximumReplicaY;k2++)
            {
              for (int k3 = minimumReplicaZ;k3 <= maximumReplicaZ;k3++)
              {
                double4x4 rotationMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(_orientation), boundingBox().center());

                double3 fractionalPosition = pos + _cell->unitCell()  * double3(double(k1), double(k2), double(k3));
                double3 cartesianPosition = _cell->convertToCartesian(fractionalPosition);

                double4 position = rotationMatrix * double4(cartesianPosition.x, cartesianPosition.y, cartesianPosition.z, 1.0);

                atomData.push_back(double3(position.x, position.y, position.z));
              }
            }
          }
        }
      }
    }
  }

  return atomData;
}

std::vector<double3> ProteinCrystal::atomUnitCellPositions() const
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  std::vector<double3> atomData = std::vector<double3>();

  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      for (std::shared_ptr<SKAtomCopy> copy : atom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          double3 fractionalPosition = _cell->inverseUnitCell() * (copy->position()  + contentShift);
          double3 position = double3::flip(fractionalPosition, contentFlip, double3(0.0,0.0,0.0));

          atomData.push_back(double3::fract(position));
        }
      }
    }
  }

  return atomData;
}

std::vector<double2> ProteinCrystal::potentialParameters() const
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  std::vector<double2> atomData = std::vector<double2>();

  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      for (std::shared_ptr<SKAtomCopy> copy : atom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          double2 parameter = atom->potentialParameters();
          atomData.push_back(parameter);
        }
      }
    }
  }

  return atomData;
}


void ProteinCrystal::computeBonds()
{
  std::vector<std::shared_ptr<SKAtomCopy>> copies =  _atomsTreeController->atomCopies();

  std::vector<std::shared_ptr<SKBond>> bonds;

  for(size_t i=0;i<copies.size();i++)
  {
    copies[i]->setType(SKAtomCopy::AtomCopyType::copy);
    double3 posA = copies[i]->position();
    int elementIdentifierA = copies[i]->parent()->elementIdentifier();
    double covalentRadiusA = PredefinedElements::predefinedElements[elementIdentifierA]._covalentRadius;
    for(size_t j=i+1;j<copies.size();j++)
    {
      double3 posB = copies[j]->position();
      int elementIdentifierB = copies[j]->parent()->elementIdentifier();
      double covalentRadiusB = PredefinedElements::predefinedElements[elementIdentifierB]._covalentRadius;

      double3 separationVector = (posA-posB);
      double3 periodicSeparationVector = _cell->applyUnitCellBoundaryCondition(separationVector);
      double bondCriteria = covalentRadiusA + covalentRadiusB + 0.56;
      double bondLength = periodicSeparationVector.length();
      if(bondLength < bondCriteria)
      {
        if(bondLength<0.1)
        {
          // a duplicate when: (a) both occupancies are 1.0, or (b) when they are the same asymmetric type
          if(!(copies[i]->parent()->occupancy() < 1.0 || copies[j]->parent()->occupancy() < 1.0) || copies[i]->asymmetricIndex() == copies[j]->asymmetricIndex())
          {
            copies[i]->setType(SKAtomCopy::AtomCopyType::duplicate);
          }
        }
        if (separationVector.length() > bondCriteria)
        {
          std::shared_ptr<SKBond> bond = std::make_shared<SKBond>(copies[i],copies[j], SKBond::BoundaryType::external);
          bonds.push_back(bond);
        }
        else
        {
          std::shared_ptr<SKBond> bond = std::make_shared<SKBond>(copies[i],copies[j], SKBond::BoundaryType::internal);
          bonds.push_back(bond);
        }
      }
    }
  }

  std::vector<std::shared_ptr<SKBond>> filtered_bonds;
  std::copy_if (bonds.begin(), bonds.end(), std::back_inserter(filtered_bonds), [](std::shared_ptr<SKBond> i){return i->atom1()->type() == SKAtomCopy::AtomCopyType::copy && i->atom2()->type() == SKAtomCopy::AtomCopyType::copy;} );
  _bondSetController->setBonds(filtered_bonds);
}

double ProteinCrystal::bondLength(std::shared_ptr<SKBond> bond) const
{
  double3 pos1 = bond->atom1()->position();
  double3 pos2 = bond->atom2()->position();
  double3 ds = pos2 - pos1;
  return (_cell->applyFullCellBoundaryCondition(ds)).length();
}

double3 ProteinCrystal::bondVector(std::shared_ptr<SKBond> bond) const
{
  double3 pos1 = bond->atom1()->position();
  double3 pos2 = bond->atom2()->position();
  double3 ds = pos2 - pos1;
  return _cell->applyFullCellBoundaryCondition(ds);
}

std::pair<double3, double3> ProteinCrystal::computeChangedBondLength(std::shared_ptr<SKBond> bond, double bondLength) const
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

std::vector<RKInPerInstanceAttributesText> ProteinCrystal::atomTextData(RKFontAtlas *fontAtlas) const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::vector<RKInPerInstanceAttributesText> atomData{};

  uint32_t asymmetricAtomIndex = 0;
  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      bool isVisible = atom->isVisible();

      if(isVisible)
      {
        QColor color = atom->color();
        float4 ambient = float4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        float4 diffuse = float4(color.redF(), color.greenF(), color.blueF(), color.alphaF());

        float4 specular = float4(1.0, 1.0, 1.0, 1.0);

        double radius = atom->drawRadius() * atom->occupancy();
        float4 scale = float4(radius, radius, radius, 1.0);
        int elementIdentifier = atom->elementIdentifier();

        if(atom->occupancy()<1.0)
        {
         diffuse = float4(1.0,1.0,1.0,1.0);
        }

        for (std::shared_ptr<SKAtomCopy> copy : atom->copies())
        {
          double3 fractionalPosition = _cell->inverseUnitCell()  * (copy->position() + contentShift);
          double3 adjustedFractionalPosition = double3::flip(fractionalPosition, contentFlip, double3(1.0,1.0,1.0));

          if (copy->type() == SKAtomCopy::AtomCopyType::copy)
          {
            for (int k1 = minimumReplicaX;k1 <= maximumReplicaX;k1++)
            {
              for (int k2 = minimumReplicaY;k2 <= maximumReplicaY;k2++)
              {
                for (int k3 = minimumReplicaZ;k3 <= maximumReplicaZ;k3++)
                {
                  double3 CartesianPosition = _cell->unitCell() * (adjustedFractionalPosition + double3(k1, k2, k3));
                  float4 position = float4(CartesianPosition, 1.0);

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

                  std::copy(subdata.begin(), subdata.end(), std::inserter(atomData, atomData.end()));
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

// MARK: Translation operations
// =====================================================================

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> ProteinCrystal::translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [translation](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                    {return std::make_pair(atom, atom->position() + translation);});

  return translatedPositions;
}


std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> ProteinCrystal::translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  double3 translationBodyFrame = _selectionBodyFixedBasis * translation;

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [translationBodyFrame](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                    {return std::make_pair(atom, atom->position() + translationBodyFrame);});

  return translatedPositions;
}

double3 ProteinCrystal::centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms) const
{
  double3  centerOfMassCosTheta = double3(0.0, 0.0, 0.0);
  double3 centerOfMassSinTheta = double3(0.0, 0.0, 0.0);
  double M = 0.0;

  for(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom : asymmetricAtoms)
  {
    int elementIdentifier = asymmetricAtom->elementIdentifier();
    double mass = PredefinedElements::predefinedElements[elementIdentifier]._mass;

    for(std::shared_ptr<SKAtomCopy> copy : asymmetricAtom->copies())
    {
      if(copy->type() == SKAtomCopy::AtomCopyType::copy)
      {
        double3 pos = _cell->convertToFractional(copy->position()) * 2.0 * M_PI;
        double3 cosTheta = double3(cos(pos.x), cos(pos.y), cos(pos.z));
        double3 sinTheta = double3(sin(pos.x), sin(pos.y), sin(pos.z));
        centerOfMassCosTheta = centerOfMassCosTheta + mass * cosTheta;
        centerOfMassSinTheta = centerOfMassSinTheta + mass * sinTheta;
        M += mass;
      }
    }
  }
  centerOfMassCosTheta = centerOfMassCosTheta / M;
  centerOfMassSinTheta = centerOfMassSinTheta / M;

  double3 com = double3((atan2(-centerOfMassSinTheta.x, -centerOfMassCosTheta.x) + M_PI)/(2.0 * M_PI),
                        (atan2(-centerOfMassSinTheta.y, -centerOfMassCosTheta.y) + M_PI)/(2.0 * M_PI),
                        (atan2(-centerOfMassSinTheta.z, -centerOfMassCosTheta.z) + M_PI)/(2.0 * M_PI));

  return  _cell->convertToCartesian(com);
}

double3x3 ProteinCrystal::matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const
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
        double3 ds = _cell->convertToFractional(copy->position() - com);
        ds.x -= std::rint(ds.x);
        ds.y -= std::rint(ds.y);
        ds.z -= std::rint(ds.z);
        double3 dr = _cell->convertToCartesian(ds);

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

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> ProteinCrystal::rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  double3 com = centerOfMassOfSelectionAsymmetricAtoms(atoms);
  double3x3 rotationMatrix = double3x3(rotation);

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [this,com,rotationMatrix](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                 {
                   double3 ds = _cell->convertToFractional(atom->position() - com);
                   ds.x -= std::rint(ds.x);
                   ds.y -= std::rint(ds.y);
                   ds.z -= std::rint(ds.z);
                   double3 translatedPositionCartesian = _cell->convertToCartesian(ds);
                   double3 position = rotationMatrix * translatedPositionCartesian;
                   return std::make_pair(atom, _cell->convertToFractional(position) + com);
                 });

  return translatedPositions;
}


std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> ProteinCrystal::rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  double3 com = centerOfMassOfSelectionAsymmetricAtoms(atoms);
  double3x3 rotationMatrix =  _selectionBodyFixedBasis * double3x3(rotation) * double3x3::inverse(_selectionBodyFixedBasis);

  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositions{};

  std::transform(atoms.begin(), atoms.end(),  std::back_inserter(translatedPositions),
                 [this,com,rotationMatrix](std::shared_ptr<SKAsymmetricAtom> atom) -> std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>
                 {
                   double3 ds = _cell->convertToFractional(atom->position() - com);
                   ds.x -= std::rint(ds.x);
                   ds.y -= std::rint(ds.y);
                   ds.z -= std::rint(ds.z);
                   double3 translatedPositionCartesian = _cell->convertToCartesian(ds);
                   double3 position = rotationMatrix * translatedPositionCartesian;
                   return std::make_pair(atom, _cell->convertToFractional(position) + com);
                 });

  return translatedPositions;
}

// Protocol: RKRenderAdsorptionSurfaceSource

std::vector<float> ProteinCrystal::gridData()
{
  int size = pow(2,encompassingPowerOfTwoCubicGridSize());
  _dimensions = int3(size,size,size);
  double2 probeParameter = adsorptionSurfaceProbeParameters();
  std::vector<double3> positions = atomUnitCellPositions();
  std::vector<double2> parameters = potentialParameters();
  double3x3 unitCell = cell()->unitCell();
  int3 numberOfReplicas = cell()->numberOfReplicas(12.0);

  try
  {
    std::vector<float> gridData = SKComputeEnergyGrid::ComputeEnergyGrid(int3(size,size,size),
                                                        probeParameter, positions, parameters, unitCell, numberOfReplicas);
    _adsorptionVolumeStepLength = 0.25 / double(size);

    const auto [min, max] = std::minmax_element(begin(gridData), end(gridData));
    _range = std::pair(double(*min),double(*max));
    return gridData;
  }
  catch (char const* e)
  {
    std::cout << "Exception caught: " << e << std::endl;
    qDebug() << QString::fromUtf8(e);
    return std::vector<float>();
  }
}
std::vector<float4> ProteinCrystal::gridValueAndGradientData()
{
  std::vector<float> energyData = gridData();

  if (energyData.empty()) { return std::vector<float4>(); }

  int encompassingcubicsize = pow(2,encompassingPowerOfTwoCubicGridSize());
  std::vector<float4> gradientData(encompassingcubicsize*encompassingcubicsize*encompassingcubicsize);

  for(int z=0;z<_dimensions.z;z++)
  {
    for(int y=0;y<_dimensions.y;y++)
    {
      for(int x=0;x<_dimensions.x;x++)
      {
        float temp = 1000.0*(1.0/300.0)*(energyData[x+_dimensions.x*y+z*_dimensions.x*_dimensions.y]-_range.first);
        float value;
        if(temp<0)
        {
          value = 0.0;
        }
        else if(temp>54000)
        {
         value = 1.0;
        }
        else
        {
          value=temp/65535.0;
        }


        // x
        int xi = (int)(x + 0.5f);
        float xf = x + 0.5f - xi;
        float xd0 = energyData[int(((xi-1 + _dimensions.x) % _dimensions.x)+y*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float xd1 = energyData[int((xi)+y*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float xd2 = energyData[int(((xi+1 + _dimensions.x) % _dimensions.x)+y*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float gx = (xd1 - xd0) * (1.0f - xf) + (xd2 - xd1) * xf; // lerp

        // y
        int yi = (int)(y + 0.5f);
        float yf = y + 0.5f - yi;
        float yd0 = energyData[int(x + ((yi-1+_dimensions.y) % _dimensions.y)*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float yd1 = energyData[int(x + (yi)*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float yd2 = energyData[int(x + ((yi+1+_dimensions.y) % _dimensions.y)*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float gy = (yd1 - yd0) * (1.0f - yf) + (yd2 - yd1) * yf; // lerp

        // z
        int zi = (int)(z + 0.5f);
        float zf = z + 0.5f - zi;
        float zd0 = energyData[int(x+y*_dimensions.x+((zi-1+_dimensions.z) % _dimensions.z)*_dimensions.x*_dimensions.y)];
        float zd1 = energyData[int(x+y*_dimensions.x+(zi)*_dimensions.x*_dimensions.y)];
        float zd2 = energyData[int(x+y*_dimensions.x+((zi+1+_dimensions.z) % _dimensions.z)*_dimensions.x*_dimensions.y)];
        float gz =  (zd1 - zd0) * (1.0f - zf) + (zd2 - zd1) * zf; // lerp

        int index = int(x+encompassingcubicsize*y+z*encompassingcubicsize*encompassingcubicsize);
        gradientData[index] = float4(value, gx, gy, gz);
      }
    }
  }
  return gradientData;
}

// Protocol: VolumetricDataEditor

void ProteinCrystal::setEncompassingPowerOfTwoCubicGridSize(int value)
{
  _encompassingPowerOfTwoCubicGridSize = value;
  int size = pow(2,value);
  _dimensions = int3(size,size,size);
  _adsorptionVolumeStepLength = 0.25 / double(size);
}

double ProteinCrystal::computeVoidFraction() const
{
  int3 size = int3(128,128,128);
  double2 probeParameter = double2(10.9, 2.64);
  std::vector<double3> positions = atomUnitCellPositions();
  std::vector<double2> parameters = potentialParameters();
  double3x3 unitCell = cell()->unitCell();
  int3 numberOfReplicas = cell()->numberOfReplicas(12.0);

  try
  {
    std::vector<float> gridData = SKComputeEnergyGrid::ComputeEnergyGrid(size, probeParameter, positions,
                                                                         parameters, unitCell, numberOfReplicas);
    double sumBoltzmannWeight = 0.0;
    for(const float &value: gridData)
    {
      sumBoltzmannWeight += exp(-(1.0/298.0) * value);  // K_B  chosen as 1.0 (energy units are Kelvin)
    }
    return (double)sumBoltzmannWeight/(double)(size.x*size.y*size.z);
  }
  catch (char const* e)
  {
    std::cout << "Exception caught: " << e << std::endl;
    return 0.0;
  }
  return 0.0;
}

double ProteinCrystal::computeNitrogenSurfaceArea() const
{
  int3 size = int3(128,128,128);

  double2 probeParameter = frameworkProbeParameters();
  std::vector<double3> positions = atomUnitCellPositions();
  std::vector<double2> parameters = potentialParameters();
  double3x3 unitCell = cell()->unitCell();
  int3 numberOfReplicas = cell()->numberOfReplicas(12.0);

  try
  {
    std::vector<float> gridData = SKComputeEnergyGrid::ComputeEnergyGrid(size, probeParameter, positions,
                                                                         parameters, unitCell, numberOfReplicas);

    double isoValue = 0.0;

    std::vector<float4> triangleData = SKComputeIsosurface::computeIsosurface(size, &gridData, isoValue);

    double totalArea=0.0;
    for(size_t i=0; i<triangleData.size(); i+=9)
    {
      double3 p1 = unitCell * double3(triangleData[i].x,triangleData[i].y,triangleData[i].z);
      double3 p2 = unitCell * double3(triangleData[i+3].x,triangleData[i+3].y,triangleData[i+3].z);
      double3 p3 = unitCell * double3(triangleData[i+6].x,triangleData[i+6].y,triangleData[i+6].z);
      double3 v = double3::cross(p2-p1,p3-p1);
      double area = 0.5 * v.length();
      if(std::isfinite(area) && fabs(area) < 1.0 )
      {
        totalArea += area;
      }
    }
    return totalArea;
  }
  catch (char const* e)
  {
    std::cout << "Exception caught: " << e << std::endl;
    return 0.0;
  }
  return 0.0;
}

void ProteinCrystal::setStructureVolumetricNitrogenSurfaceArea(double value)
{
  _structureVolumetricNitrogenSurfaceArea = value * 1e4 / cell()->volume();
}

void ProteinCrystal::setStructureGravimetricNitrogenSurfaceArea(double value)
{
  _structureGravimetricNitrogenSurfaceArea = value * Constants::AvogadroConstantPerAngstromSquared / _structureMass;
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<ProteinCrystal> &proteinCrystal)
{
  stream << proteinCrystal->_versionNumber;

  // handle super class
  stream << std::static_pointer_cast<Structure>(proteinCrystal);

  stream << proteinCrystal->_spaceGroup;
  stream << qint64(0x6f6b6185);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<ProteinCrystal> &proteinCrystal)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > proteinCrystal->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "ProteinCrystal");
  }

  if(versionNumber >= 2) // introduced in version 2
  {
    stream >> proteinCrystal->_spaceGroup;

    qint64 magicNumber;
    stream >> magicNumber;
    if(magicNumber != qint64(0x6f6b6185))
    {
      throw InvalidArchiveVersionException(__FILE__, __LINE__, "ProteinCrystal invalid magic-number");
    }
  }

  std::shared_ptr<Structure> structure = std::static_pointer_cast<Structure>(proteinCrystal);
  stream >> structure;

  if(versionNumber <= 1)
  {
    proteinCrystal->_spaceGroup = structure->legacySpaceGroup();
  }

  return stream;
}
