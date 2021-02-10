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

#include "skatomtreecontroller.h"
#include <stack>
#include <algorithm>
#include <QDebug>

SKAtomTreeController::SKAtomTreeController()
{
  _hiddenRootNode = std::make_shared<SKAtomTreeNode>();
}

SKAtomTreeController::~SKAtomTreeController()
{
}

bool SKAtomTreeController::isRootNode(SKAtomTreeNode *node)
{
  return node == _hiddenRootNode.get();
}

void SKAtomTreeController::insertNodeInParent(std::shared_ptr<SKAtomTreeNode> node, std::weak_ptr<SKAtomTreeNode> parent, int index)
{
  if (std::shared_ptr<SKAtomTreeNode> lockedParent = parent.lock())
  {
    lockedParent->_childNodes.insert(lockedParent->_childNodes.begin() + index, node);
    node->_parent = parent;
    lockedParent->updateFilteredChildren(this->_filterPredicate);
  }
  else
  {
    _hiddenRootNode->_childNodes.insert(_hiddenRootNode->_childNodes.begin() + index, node);
    node->_parent = parent;
    _hiddenRootNode->updateFilteredChildren(this->_filterPredicate);
  }
}

void SKAtomTreeController::insertNodeAtIndexPath(std::shared_ptr<SKAtomTreeNode> node, IndexPath path)
{
  size_t index = path.lastIndex();
  std::shared_ptr<SKAtomTreeNode> parent = _hiddenRootNode->descendantNodeAtIndexPath(path.removingLastIndex());
  node->insertInParent(parent, index);
}

void SKAtomTreeController::removeNode(std::shared_ptr<SKAtomTreeNode> node)
{
  if(std::shared_ptr<SKAtomTreeNode> parentNode = node->_parent.lock())
  {
    node->removeFromParent();
    parentNode->updateFilteredChildren(_filterPredicate);
  }
}

/*
std::shared_ptr<SKAtomTreeNode> SKAtomTreeController::parentItem(std::shared_ptr<SKAtomTreeNode> node)
{
  if(node->_parent.lock() == _hiddenRootNode)
  {
    return nullptr;
  }
  else
  {
    return node->_parent.lock();
  }
}*/


void SKAtomTreeController::appendToRootnodes(std::shared_ptr<SKAtomTreeNode> item)
{
  item->_parent = _hiddenRootNode;
  _hiddenRootNode->_childNodes.push_back(item);
}

bool SKAtomTreeController::nodeIsChildOfItem(std::shared_ptr<SKAtomTreeNode> node, std::shared_ptr<SKAtomTreeNode> item)
{
  std::shared_ptr<SKAtomTreeNode> itemNode = item == nullptr ? _hiddenRootNode : item;
  return node->_parent.lock() == itemNode;
}


size_t SKAtomTreeController::filteredChildIndexOfItem(std::shared_ptr<SKAtomTreeNode> parentItem, size_t index)
{
  std::shared_ptr<SKAtomTreeNode> parentNode = parentItem == nullptr ? _hiddenRootNode : parentItem;

  if(index < parentNode->_filteredAndSortedNodes.size())
  {
    std::shared_ptr<SKAtomTreeNode> node = parentNode->_filteredAndSortedNodes[index];

    std::optional<size_t> filteredIndex = parentNode->findChildIndex(node);

    return *filteredIndex;
  }
  else
  {
    return parentNode->_childNodes.size();
  }
}

bool SKAtomTreeController::isDescendantOfNode(std::shared_ptr<SKAtomTreeNode> item, std::shared_ptr<SKAtomTreeNode> parentNode)
{
  std::shared_ptr<SKAtomTreeNode> treeNode = item == nullptr ? _hiddenRootNode : item;

  while(treeNode != nullptr)
  {
    if (treeNode == parentNode)
    {
      return true;
    }
    else
    {
      treeNode = treeNode->_parent.lock();
    }
  }
  return false;
}




std::vector<std::shared_ptr<SKAtomTreeNode>> SKAtomTreeController::childrenForItem(std::shared_ptr<SKAtomTreeNode> item)
{
  if (item==nullptr)
  {
    return _hiddenRootNode->_childNodes;
  }
  else
  {
    return item->_childNodes;
  }
}

std::shared_ptr<SKAtomTreeNode> SKAtomTreeController::nodeAtIndexPath(IndexPath indexPath) const
{
  return _hiddenRootNode->descendantNodeAtIndexPath(indexPath);
}

std::vector<std::shared_ptr<SKAtomTreeNode> > SKAtomTreeController::flattenedLeafNodes() const
{
  return _hiddenRootNode->flattenedLeafNodes();
}

std::vector<std::shared_ptr<SKAsymmetricAtom> > SKAtomTreeController::flattenedObjects() const
{
   return _hiddenRootNode->flattenedObjects();
}



std::vector<std::shared_ptr<SKAtomCopy>> SKAtomTreeController::allAtomCopies() const
{
  std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>();

  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = flattenedLeafNodes();

  for(std::shared_ptr<SKAtomTreeNode> node: asymmetricAtomNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      for(std::shared_ptr<SKAtomCopy> copy: atom->copies())
      {
        atomCopies.push_back(copy);
      }
    }
  }
  return atomCopies;
}

std::vector<std::shared_ptr<SKAtomCopy>> SKAtomTreeController::atomCopies() const
{
  std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>();

  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = flattenedLeafNodes();

  for(std::shared_ptr<SKAtomTreeNode> node: asymmetricAtomNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      for(std::shared_ptr<SKAtomCopy> copy: atom->copies())
      {
        if(copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          atomCopies.push_back(copy);
        }
      }
    }
  }
  return atomCopies;
}

void SKAtomTreeController::setTags()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = flattenedLeafNodes();

  int atomIndex=0;
  int asymmetricAtomIndex=0;
  for(std::shared_ptr<SKAtomTreeNode> node: asymmetricAtomNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      for(std::shared_ptr<SKAtomCopy> atom: asymmetricAtom->copies())
      {
        atom->setTag(atomIndex);
        atom->setAsymmetricIndex(asymmetricAtomIndex);
        atomIndex++;
      }
      asymmetricAtom->setTag(asymmetricAtomIndex);
      asymmetricAtom->setAsymmetricIndex(asymmetricAtomIndex);
      asymmetricAtomIndex++;
    }
  }
}


// MARK: Selection
// =============================================================================================

AtomSelectionIndexPaths SKAtomTreeController::selectionIndexPaths() const
{
  return std::make_pair(_selectionIndexPath, _selectionIndexPaths);
}

void SKAtomTreeController::setSelectionIndexPaths(AtomSelectionIndexPaths selection)
{
  _selectionIndexPath = selection.first;
  _selectionIndexPaths = selection.second;
}

AtomSelectionNodesAndIndexPaths SKAtomTreeController::selectionNodesAndIndexPaths() const
{
  std::set<std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>, atom_indexPath_compare_less> nodesAndIndexPaths{};

  std::transform(_selectionIndexPaths.begin(), _selectionIndexPaths.end(), std::inserter(nodesAndIndexPaths, nodesAndIndexPaths.begin()),
                 [this](IndexPath indexPath) -> std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>
                 {return std::make_pair(this->nodeAtIndexPath(indexPath), indexPath);});
  return std::make_pair(std::make_pair(this->nodeAtIndexPath(_selectionIndexPath), _selectionIndexPath),nodesAndIndexPaths);
}

void SKAtomTreeController::setSelectionIndexPaths(AtomSelectionNodesAndIndexPaths selection)
{
  _selectionIndexPaths.clear();
  std::transform(selection.second.begin(), selection.second.end(), std::inserter(_selectionIndexPaths, _selectionIndexPaths.begin()),
                 [](std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath> pair) -> IndexPath
                 {return pair.second;});
}

std::set<std::shared_ptr<SKAtomTreeNode>> SKAtomTreeController::selectedTreeNodes()
{
  std::set<std::shared_ptr<SKAtomTreeNode>> selectedAtomTreeNodes{};
  std::transform(_selectionIndexPaths.begin(), _selectionIndexPaths.end(), std::inserter(selectedAtomTreeNodes, selectedAtomTreeNodes.begin()),
                   [this](IndexPath indexPath) -> std::shared_ptr<SKAtomTreeNode>
                          {return nodeAtIndexPath(indexPath);});
  return selectedAtomTreeNodes;
}


std::vector<std::shared_ptr<SKAsymmetricAtom>> SKAtomTreeController::selectedObjects()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> selectedObjects{};
  std::transform(_selectionIndexPaths.begin(), _selectionIndexPaths.end(), std::inserter(selectedObjects, selectedObjects.begin()),
                   [this](IndexPath indexPath) -> std::shared_ptr<SKAsymmetricAtom>
                          {return nodeAtIndexPath(indexPath)->representedObject();});
  return selectedObjects;
}

AtomSelectionIndexPaths SKAtomTreeController::updateIndexPathsFromNodes(AtomSelectionNodesAndIndexPaths selection)
{
  AtomSelectionIndexPaths updatedSelection{};
  if(selection.first.first)
  {
    updatedSelection.first = selection.first.first->indexPath();
  }

  std::transform(selection.second.begin(), selection.second.end(), std::inserter(updatedSelection.second, updatedSelection.second.begin()),
                 [](std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath> pair) -> IndexPath
                 {return pair.first->indexPath();});

  return updatedSelection;
}

double SKAtomTreeController::netCharge()
{
  const std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = flattenedLeafNodes();

  double net_charge=0.0;
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(const std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      for(const std::shared_ptr<SKAtomCopy> &atomCopy : asymmetricAtom->copies())
      {
        if(atomCopy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          net_charge += atomCopy->parent()->charge();
        }
      }
    }
  }
  return net_charge;
}

QDataStream &operator<<(QDataStream& stream, const std::shared_ptr<SKAtomTreeController>& controller)
{
  stream << controller->_versionNumber;
  stream << controller->_hiddenRootNode;
  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::shared_ptr<SKAtomTreeController>& controller)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > controller->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "SKAtomTreeController");
  }

  stream >> controller->_hiddenRootNode;
  return stream;
}
