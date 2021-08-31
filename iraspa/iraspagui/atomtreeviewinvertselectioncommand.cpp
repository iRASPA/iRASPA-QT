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

#include "atomtreeviewinvertselectioncommand.h"
#include <QDebug>
#include <algorithm>
#include <set>

AtomTreeViewInvertSelectionCommand::AtomTreeViewInvertSelectionCommand(MainWindow *main_window, std::shared_ptr<Structure> structure,
                                     AtomSelectionIndexPaths atomSelection, BondSelectionNodesAndIndexSet bondSelection, QUndoCommand *parent):
  _main_window(main_window),
  _structure(structure),
  _atomSelection(atomSelection),
  _bondSelection(bondSelection),
  _invertedAtomSelection(),
  _invertedBondSelection()
{
  Q_UNUSED(parent);

  setText(QString("Invert atom/bond selection"));

  // allAtomTreeNodes: set of all the treenodes
  std::set<std::shared_ptr<SKAtomTreeNode>> allAtomTreeNodes{};
  std::vector<std::shared_ptr<SKAtomTreeNode>> flattenedNodes = structure->atomsTreeController()->flattenedLeafNodes();
  std::copy(flattenedNodes.begin(),flattenedNodes.end(), std::inserter(allAtomTreeNodes, allAtomTreeNodes.begin()));

  // selectedAtoms: set of all the selected treenodes
  std::set<std::shared_ptr<SKAtomTreeNode>> selectedAtoms = structure->atomsTreeController()->selectedTreeNodes();

  // invertedTreeNodeAtoms: the difference between all nodes and the selected nodes
  std::set<std::shared_ptr<SKAtomTreeNode>> invertedTreeNodeAtoms{};
  std::set_difference(allAtomTreeNodes.begin(), allAtomTreeNodes.end(), selectedAtoms.begin(), selectedAtoms.end(),
      std::inserter(invertedTreeNodeAtoms, invertedTreeNodeAtoms.end()));

  // invertedAsymmetricAtoms: the asymmetric atoms in the invertedTreeNodeAtoms set
  std::set<std::shared_ptr<SKAsymmetricAtom>> invertedAsymmetricAtoms{};
  std::transform(invertedTreeNodeAtoms.begin(),invertedTreeNodeAtoms.end(), std::inserter(invertedAsymmetricAtoms, invertedAsymmetricAtoms.begin()),
                 [](std::shared_ptr<SKAtomTreeNode> treeNode) -> std::shared_ptr<SKAsymmetricAtom>
                 {return treeNode->representedObject();});
  std::transform(invertedTreeNodeAtoms.begin(),invertedTreeNodeAtoms.end(), std::inserter(_invertedAtomSelection.second, _invertedAtomSelection.second.begin()),
                 [](std::shared_ptr<SKAtomTreeNode> treeNode) -> IndexPath
                 {return treeNode->indexPath();});


  std::set<int> selectedBonds = structure->bondSetController()->selectionIndexSet();
  std::set<int> allBondIndices{};
  std::generate_n( inserter( allBondIndices, allBondIndices.begin() ), selectedBonds.size(), [ i=0 ]() mutable { return i++; });

  std::set<int> invertedBonds{};
  std::set_difference(allBondIndices.begin(), allBondIndices.end(), selectedBonds.begin(), selectedBonds.end(),
      std::inserter(invertedBonds, invertedBonds.end()));


  const std::vector<std::shared_ptr<SKAsymmetricBond>> asymmetricBonds = structure->bondSetController()->arrangedObjects();
  int index=0;
  for(const std::shared_ptr<SKAsymmetricBond> &asymmetricBond: asymmetricBonds)
  {
    const bool containsAtom1 = invertedAsymmetricAtoms.find(asymmetricBond->atom1()) != invertedAsymmetricAtoms.end();
    const bool containsAtom2 = invertedAsymmetricAtoms.find(asymmetricBond->atom2()) != invertedAsymmetricAtoms.end();
    if(containsAtom1 || containsAtom2)
    {
       invertedBonds.insert(index);
    }
    index++;
  }

  _invertedBondSelection.clear();
  std::transform(invertedBonds.begin(),invertedBonds.end(), std::inserter(_invertedBondSelection, _invertedBondSelection.begin()),
                 [asymmetricBonds](int index) -> std::pair<std::shared_ptr<SKAsymmetricBond>, int>
                 {return std::make_pair(asymmetricBonds[index], index);});

}

void AtomTreeViewInvertSelectionCommand::redo()
{
  if(std::shared_ptr<Structure> structure = _structure)
  {
    structure->atomsTreeController()->setSelectionIndexPaths(_invertedAtomSelection);
    structure->bondSetController()->setSelection(_invertedBondSelection);
  }

  if(_main_window)
  {
    _main_window->updateControlPanel();
    _main_window->reloadSelectionDetailViews();
  }
}

void AtomTreeViewInvertSelectionCommand::undo()
{
  if(std::shared_ptr<Structure> structure = _structure)
  {
    structure->atomsTreeController()->setSelectionIndexPaths(_atomSelection);
    structure->bondSetController()->setSelection(_bondSelection);
  }

  if(_main_window)
  {
    _main_window->updateControlPanel();
    _main_window->reloadSelectionDetailViews();
  }
}

