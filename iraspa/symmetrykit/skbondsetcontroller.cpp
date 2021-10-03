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

#include "skbondsetcontroller.h"
#include <set>
#include <map>
#include <unordered_map>
#include <type_traits>
#include <qdebug.h>
#include <algorithm>
#include <map>
#include <unordered_set>

qint64 SKBondSetController::_versionNumber = 2;

SKBondSetController::SKBondSetController(std::shared_ptr<SKAtomTreeController> atomTreeController):_atomTreecontroller(atomTreeController)
{

}


struct SKAysmmetricBondCompare
{
   bool operator () (const std::shared_ptr<SKAsymmetricBond> & s1, const std::shared_ptr<SKAsymmetricBond>& s2) const
   {
      if(s1->atom1()->elementIdentifier() == s2->atom1()->elementIdentifier())
      {
        if(s1->atom2()->elementIdentifier() == s2->atom2()->elementIdentifier())
        {
          if(s1->atom1()->tag() == s2->atom1()->tag())
          {
            return (s1->atom2()->tag() < s2->atom2()->tag());
          }
          else
          {
             return (s1->atom1()->tag() < s2->atom1()->tag());
          }
        }
        else
        {
          return (s1->atom2()->elementIdentifier() > s2->atom2()->elementIdentifier());
        }
      }
      else
      {
        return (s1->atom1()->elementIdentifier() > s2->atom1()->elementIdentifier());
      }
    }
};

std::vector<std::shared_ptr<SKBond>> SKBondSetController::getBonds()
{
  std::vector<std::shared_ptr<SKBond>> bonds;

  size_t count = getNumberOfBonds();
  bonds.reserve(count);

  for(std::shared_ptr<SKAsymmetricBond> asymmetricBond: _arrangedObjects)
  {
    std::vector<std::shared_ptr<SKBond>> copies =  asymmetricBond->copies();
    bonds.insert( bonds.end(), copies.begin(), copies.end() );
  }

  return bonds;
}

size_t SKBondSetController::getNumberOfBonds()
{
  size_t count = 0;
  for(std::shared_ptr<SKAsymmetricBond> asymmetricBond: _arrangedObjects)
  {
     count += asymmetricBond->copies().size();
  }
  return count;
}

void SKBondSetController::setBonds(std::vector<std::shared_ptr<SKBond>> &bonds)
{
  this->_arrangedObjects.clear();

  std::unordered_set<std::shared_ptr<SKAsymmetricBond>, SKAsymmetricBond::KeyHash, SKAsymmetricBond::KeyEqual> asymmetricBonds;

  std::transform(bonds.begin(),bonds.end(),std::inserter(asymmetricBonds, asymmetricBonds.begin()),[](std::shared_ptr<SKBond> b) -> std::shared_ptr<SKAsymmetricBond>
          {return std::make_shared<SKAsymmetricBond>(SKAsymmetricBond(b->atom1()->parent(), b->atom2()->parent()));});


  // partition the bonds
  for(std::shared_ptr<SKBond> &bond: bonds)
  {
    std::shared_ptr<SKAsymmetricBond> asymmetricBond = std::make_shared<SKAsymmetricBond>(SKAsymmetricBond(bond->atom1()->parent(), bond->atom2()->parent()));

    std::unordered_set<std::shared_ptr<SKAsymmetricBond>, SKAsymmetricBond::KeyHash, SKAsymmetricBond::KeyEqual>::iterator it = asymmetricBonds.find(asymmetricBond);
    if (it != asymmetricBonds.end())
    {
      it->get()->copies().push_back(bond);
    }
  }

  _arrangedObjects.clear();
  std::copy(asymmetricBonds.begin(), asymmetricBonds.end(),std::back_inserter(_arrangedObjects));
  std::sort(_arrangedObjects.begin(), _arrangedObjects.end(), SKAysmmetricBondCompare());
}

bool SKBondSetController::insertBond(std::shared_ptr<SKAsymmetricBond> bondItem, int index)
{
  _arrangedObjects.insert(_arrangedObjects.begin() + index, bondItem);
  return true;
}

bool SKBondSetController::removeBond(int index)
{
  _arrangedObjects.erase(_arrangedObjects.begin() + index);
  return true;
}

void SKBondSetController::insertBonds(std::vector<std::shared_ptr<SKAsymmetricBond>> bonds, std::set<int> indexSet)
{
  int bondIndex=0;
  for(int index: indexSet)
  {
    _arrangedObjects.insert(_arrangedObjects.begin() + index, bonds[bondIndex]);
    bondIndex++;
  }
}

void SKBondSetController::deleteBonds(BondSelectionIndexSet indexSet)
{
  std::vector<int> reversedIndexSet;
  std::reverse_copy(indexSet.begin(), indexSet.end(), std::back_inserter(reversedIndexSet));

  for (int index : reversedIndexSet)
  {
    _arrangedObjects.erase(_arrangedObjects.begin() + index);
  }
}

void SKBondSetController::addSelectedObjects(BondSelectionIndexSet selection)
{
 _selectedIndexSet.insert(selection.begin(), selection.end());
}


void SKBondSetController::setTags()
{
  int asymmetricBondIndex = 0;
  for(std::shared_ptr<SKAsymmetricBond> asymmetricBond: _arrangedObjects)
  {
    asymmetricBond->setAsymmetricIndex(asymmetricBondIndex);
    asymmetricBondIndex++;
  }
}

BondSelectionNodesAndIndexSet SKBondSetController::selectionNodesAndIndexSet() const
{
  BondSelectionNodesAndIndexSet s;
  for(int index: _selectedIndexSet)
  {
    s.insert(std::make_pair(_arrangedObjects[index], index));
  }
  return s;
}

void  SKBondSetController::setSelection(BondSelectionNodesAndIndexSet selection)
{
  _selectedIndexSet.clear();
  for(const auto &[bondItem, index] : selection)
  {
    _selectedIndexSet.insert(index);
  }
}

void SKBondSetController::insertSelection(BondSelectionNodesAndIndexSet selection)
{
  _selectedIndexSet.clear();
  for(const auto &[bondItem, index] : selection)
  {
    _arrangedObjects.insert(_arrangedObjects.begin() + index, bondItem);
    _selectedIndexSet.insert(index);
  }
}

void SKBondSetController::deleteBonds(BondSelectionNodesAndIndexSet selection)
{
  for (const auto &[bondItem, index] : selection)
  {
    _arrangedObjects.erase(_arrangedObjects.begin() + index);
  }
}

std::vector<std::shared_ptr<SKAsymmetricBond>> SKBondSetController::selectedObjects() const
{
  std::vector<std::shared_ptr<SKAsymmetricBond>> objects;
  for(int index: _selectedIndexSet)
  {
    objects.push_back(_arrangedObjects[index]);
  }
  return objects;
}

void SKBondSetController::correctBondSelectionDueToAtomSelection()
{
  AtomSelectionNodesAndIndexPaths selectedAtomNodes = _atomTreecontroller->selectionNodesAndIndexPaths();
  std::set<std::shared_ptr<SKAsymmetricAtom>> selectedAtoms;
  std::transform(selectedAtomNodes.second.begin(),selectedAtomNodes.second.end(),std::inserter(selectedAtoms, selectedAtoms.begin()),
                 [](std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath> pair) -> std::shared_ptr<SKAsymmetricAtom>
                        {return pair.first->representedObject();});

  int bondIndex=0;
  for(const std::shared_ptr<SKAsymmetricBond> &bond : _arrangedObjects)
  {
    // if one the atoms of the bond is selected then so must be the bond
    const bool Atom1isSelected = selectedAtoms.find(bond->atom1()) != selectedAtoms.end();
    const bool Atom2isSelected = selectedAtoms.find(bond->atom2()) != selectedAtoms.end();
    if(Atom1isSelected || Atom2isSelected)
    {
      _selectedIndexSet.insert(bondIndex);
    }
    bondIndex++;
  }
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<SKBondSetController> &controller)
{
  stream << controller->_versionNumber;
  stream << controller->_arrangedObjects;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<SKBondSetController>& controller)
{
  qint64 versionNumber;
  stream >> versionNumber;

  if(versionNumber > controller->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "SKBondSetController");
  }

  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms = controller->_atomTreecontroller->flattenedObjects();
  std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = controller->_atomTreecontroller->allAtomCopies();

  stream >> controller->_arrangedObjects;

  // fill in the atoms from the tags
  for(std::shared_ptr<SKAsymmetricBond> &bond: controller->_arrangedObjects)
  {
    int atom1Tag = bond->getTag1();
    int atom2Tag = bond->getTag2();
    bond->setAtom1(asymmetricAtoms[atom1Tag]);
    bond->setAtom2(asymmetricAtoms[atom2Tag]);

    for(std::shared_ptr<SKBond> &bondCopy: bond->copies())
    {
      int tag1 = bondCopy->getTag1();
      int tag2 = bondCopy->getTag2();
      bondCopy->setAtoms(atomCopies[tag1],atomCopies[tag2]);
    }
  }

  return stream;
}


