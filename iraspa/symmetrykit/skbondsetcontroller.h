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

#include <QDataStream>
#include <unordered_set>
#include <vector>
#include <set>
#include <tuple>
#include "skbond.h"
#include "skasymmetricatom.h"
#include "skatomcopy.h"
#include "skasymmetricbond.h"
#include "skatomtreecontroller.h"

struct bond_indexSet_compare_less
{
  bool operator() (const std::pair<std::shared_ptr<SKAsymmetricBond>, IndexPath>& lhs, const std::pair<std::shared_ptr<SKAsymmetricBond>, IndexPath>& rhs) const
  {
    return lhs.second < rhs.second;
  }
};

struct bond_indexSet_compare_greater_than
{
  bool operator() (const std::pair<std::shared_ptr<SKAsymmetricBond>, IndexPath>& lhs, const std::pair<std::shared_ptr<SKAsymmetricBond>, IndexPath>& rhs) const
  {
    return lhs.second > rhs.second;
  }
};

using BondSelectionIndexSet = std::set<int64_t>;
using BondSelectionNodesAndIndexSet = std::set<std::pair<std::shared_ptr<SKAsymmetricBond>, int64_t>, bond_indexSet_compare_less>;
using ReversedBondSelectionNodesAndIndexSet = std::set<std::pair<std::shared_ptr<SKAsymmetricBond>, int64_t>, bond_indexSet_compare_greater_than>;

class SKBondSetController
{
public:
  SKBondSetController(std::shared_ptr<SKAtomTreeController> atomTreeController);
  const std::vector<std::shared_ptr<SKAsymmetricBond>>& arrangedObjects() const {return _arrangedObjects;}
  void append(std::shared_ptr<SKAsymmetricBond> bond) {_arrangedObjects.push_back(bond);}
  void clear() {_arrangedObjects.clear();}
  size_t getNumberOfBonds();
  std::vector<std::shared_ptr<SKBond>> getBonds();
  void setBonds(std::vector<std::shared_ptr<SKBond>> &bonds);
  bool insertBond(std::shared_ptr<SKAsymmetricBond> bondItem, int index);
  bool removeBond(int index);
  void insertBonds(std::vector<std::shared_ptr<SKAsymmetricBond> > bonds, BondSelectionIndexSet indexSet);
  void deleteBonds(BondSelectionIndexSet indexSet);


  std::vector<std::shared_ptr<SKAsymmetricBond>> selectedObjects() const;
  void setSelectionIndexSet(BondSelectionIndexSet selection) {_selectedIndexSet = selection;}
  void addSelectedObjects(BondSelectionIndexSet selectionNodesAndIndexSet);
  void setTags();
  void correctBondSelectionDueToAtomSelection();
  BondSelectionIndexSet &selectionIndexSet() {return _selectedIndexSet;}
  BondSelectionNodesAndIndexSet selectionNodesAndIndexSet() const;
  void setSelection(BondSelectionNodesAndIndexSet selectionNodesAndIndexSet);
  void deleteBonds(BondSelectionNodesAndIndexSet selectionNodesAndIndexSet);
  void insertSelection(BondSelectionNodesAndIndexSet selectionNodesAndIndexSet);
private:
  static qint64 _versionNumber;
  std::shared_ptr<SKAtomTreeController> _atomTreecontroller;

  std::vector<std::shared_ptr<SKAsymmetricBond>> _arrangedObjects;
  BondSelectionIndexSet _selectedIndexSet;

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<SKBondSetController> &);
  friend QDataStream &operator>>(QDataStream &stream, std::shared_ptr<SKBondSetController>& controller);
};
