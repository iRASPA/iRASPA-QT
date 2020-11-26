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

#include "skatomtreenode.h"
#include <assert.h>
#include <vector>
#include <QtDebug>

SKAtomTreeNode::~SKAtomTreeNode()
{
}

const IndexPath SKAtomTreeNode::indexPath()
{
  if (std::shared_ptr<SKAtomTreeNode> lockedParent = _parent.lock())
  {
    IndexPath indexpath = lockedParent->indexPath();

    std::optional<int> index = lockedParent->findChildIndex(shared_from_this());
    if(index)
    {
      if (indexpath.size() > 0)
      {
        return indexpath.appending(*index);
      }
      else
      {
        return IndexPath(*index);
      }
    }
  }

  return IndexPath();
}

std::shared_ptr<SKAtomTreeNode> SKAtomTreeNode::descendantNodeAtIndexPath(IndexPath indexPath)
{
  size_t length = indexPath.size();
  std::shared_ptr<SKAtomTreeNode> node = shared_from_this();

  for(size_t i=0; i < length; i++)
  {
    size_t index = indexPath[i];
    if(index>=node->_childNodes.size())
    {
      return nullptr;
    }
    else
    {
      node = node->_childNodes[index];
    }
  }

  return node;
}

bool SKAtomTreeNode::insertChild(int row, std::shared_ptr<SKAtomTreeNode> child)
{
  if (row < 0 || size_t(row) > _childNodes.size())
  {
    qDebug() << "Error insertion at: " << row << ", size: " << _childNodes.size();
    return false;
  }

  child->_parent = shared_from_this();
  _childNodes.insert(_childNodes.begin() + row, child);
  return true;
}

void SKAtomTreeNode::insertInParent(std::shared_ptr<SKAtomTreeNode> parent, int index)
{
   this->_parent = parent;
   parent->_childNodes.insert(parent->_childNodes.begin() + index, shared_from_this());
}

void SKAtomTreeNode::appendToParent(std::shared_ptr<SKAtomTreeNode> parent)
{
    this->_parent = parent;
    parent->_childNodes.push_back(shared_from_this());

}

bool SKAtomTreeNode::removeChild(size_t row)
{
  if (row < 0 || row >= _childNodes.size())
     return false;

  _childNodes.erase(_childNodes.begin() + row);
  return true;
}

bool SKAtomTreeNode::removeChildren(size_t position, size_t count)
{
  if (position < 0 || position + count > _childNodes.size())
    return false;
  std::vector<std::shared_ptr<SKAtomTreeNode>>::iterator start = _childNodes.begin() + position;
  std::vector<std::shared_ptr<SKAtomTreeNode>>::iterator end = _childNodes.begin() + position + count;
  _childNodes.erase(start, end);

  return true;
}

void SKAtomTreeNode::removeFromParent()
{
  if(std::shared_ptr<SKAtomTreeNode> lockedParent = _parent.lock())
  {
    std::optional<int> index = lockedParent->findChildIndex(shared_from_this());
    assert(index);
    if(index)
    {
      lockedParent->_childNodes.erase(lockedParent->_childNodes.begin() + *index);
    }
    _parent.reset();
  }
}


std::vector<std::shared_ptr<SKAtomTreeNode>> SKAtomTreeNode::flattenedNodes()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> nodes{};

  std::vector<std::shared_ptr<SKAtomTreeNode>> otherNodes = this->descendantNodes();
  for(std::shared_ptr<SKAtomTreeNode> child : otherNodes)
  {
    nodes.push_back(child);
  }
  return nodes;
}

std::vector<std::shared_ptr<SKAtomTreeNode>> SKAtomTreeNode::flattenedLeafNodes()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> nodes{};

  std::vector<std::shared_ptr<SKAtomTreeNode>> otherNodes = this->descendantLeafNodes();
  for(std::shared_ptr<SKAtomTreeNode> child : otherNodes)
  {
    nodes.push_back(child);
  }
  return nodes;
}

std::vector<std::shared_ptr<SKAtomTreeNode>> SKAtomTreeNode::flattenedGroupNodes()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> nodes{};

  if(this->_isGroup)
  {
    nodes.push_back(shared_from_this());
  }

  std::vector<std::shared_ptr<SKAtomTreeNode>> otherNodes = this->descendantGroupNodes();
  for(std::shared_ptr<SKAtomTreeNode> child : otherNodes)
  {
    nodes.push_back(child);
  }
  return nodes;
}


bool SKAtomTreeNode::isDescendantOfNode(std::shared_ptr<SKAtomTreeNode> parent)
{
  std::shared_ptr<SKAtomTreeNode> treeNode = shared_from_this();

  while(treeNode != nullptr)
  {
    if(treeNode == parent)
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

std::vector<std::shared_ptr<SKAtomTreeNode>> SKAtomTreeNode::descendants()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> descendants{};
  for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
  {
    if(child->isLeaf())
    {
      descendants.push_back(child);
    }
    else
    {
      std::vector<std::shared_ptr<SKAtomTreeNode>> otherNodes = child->descendants();
      for(std::shared_ptr<SKAtomTreeNode> node : otherNodes)
      {
        descendants.push_back(node);
      }
    }
  }

  return descendants;
}

std::vector<std::shared_ptr<SKAtomTreeNode>> SKAtomTreeNode::descendantNodes()
{
    std::vector<std::shared_ptr<SKAtomTreeNode>> descendants{};
    for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
    {
      descendants.push_back(child);
      if (child->_isGroup)
      {
        std::vector<std::shared_ptr<SKAtomTreeNode>> otherNodes = child->descendantNodes();
        for(std::shared_ptr<SKAtomTreeNode> node : otherNodes)
        {
          descendants.push_back(node);
        }
      }
    }

    return descendants;
}

std::vector<std::shared_ptr<SKAtomTreeNode>> SKAtomTreeNode::descendantLeafNodes()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> descendants{};
  for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
  {
    assert(child);
    if(child->isLeaf())
    {
      descendants.push_back(child);
    }
    else
    {
      std::vector<std::shared_ptr<SKAtomTreeNode>> otherNodes = child->descendantLeafNodes();
      for(std::shared_ptr<SKAtomTreeNode> node : otherNodes)
      {
        descendants.push_back(node);
      }
    }
  }

  return descendants;
}

std::vector<std::shared_ptr<SKAtomTreeNode>> SKAtomTreeNode::descendantGroupNodes()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> descendants{};
  for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
  {
    if(child->_isGroup)
    {
      descendants.push_back(child);
      std::vector<std::shared_ptr<SKAtomTreeNode>> otherNodes = child->descendantGroupNodes();
      for(std::shared_ptr<SKAtomTreeNode> node : otherNodes)
      {
        descendants.push_back(node);
      }
    }
  }

  return descendants;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> SKAtomTreeNode::flattenedObjects()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> objects{};

  std::vector<std::shared_ptr<SKAsymmetricAtom>> groupObjects = this->descendantObjects();
  for(std::shared_ptr<SKAsymmetricAtom> object : groupObjects)
  {
    objects.push_back(object);
  }
  return objects;
}


std::vector<std::shared_ptr<SKAsymmetricAtom>> SKAtomTreeNode::descendantObjects()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> descendants{};
  for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
  {
    descendants.push_back(child->_representedObject);
    if(child->_isGroup)
    {
      std::vector<std::shared_ptr<SKAsymmetricAtom>> groupObjects = child->descendantObjects();
      for(std::shared_ptr<SKAsymmetricAtom> object : groupObjects)
      {
        descendants.push_back(object);
      }
    }
  }
  return descendants;
}


// MARK: -
// MARK: Filtering support

void SKAtomTreeNode::updateFilteredChildren(std::function<bool(std::shared_ptr<SKAtomTreeNode>)> predicate)
{
  for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
  {
    child->_matchesFilter = true;
    child->_matchesFilter = predicate(shared_from_this());
  }

  this->_filteredAndSortedNodes.clear();
  for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
  {
    if (child->_matchesFilter) this->_filteredAndSortedNodes.push_back(child);
  }

  // if we have filtered nodes, then all parents of this node needs to be included
  if(this->_filteredAndSortedNodes.size() > 0)
  {
    this->_matchesFilter = true;
  }
}

void SKAtomTreeNode::updateFilteredChildrenRecursively(std::function<bool(std::shared_ptr<SKAtomTreeNode>)> predicate)
{
  this->_matchesFilter = false;
  this->_matchesFilter = predicate(shared_from_this());

  for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
  {
    child->updateFilteredChildrenRecursively(predicate);
  }

  if(this->_matchesFilter)
  {
    if(std::shared_ptr<SKAtomTreeNode> lockedParent = _parent.lock())
    {
      lockedParent->_matchesFilter = true;
    }
  }

  this->_filteredAndSortedNodes.clear();
  for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
  {
    if (child->_matchesFilter) this->_filteredAndSortedNodes.push_back(child);
  }
}

void SKAtomTreeNode::setFilteredNodesAsMatching()
{
  this->_matchesFilter = false;

  this->_filteredAndSortedNodes.clear();
  for(std::shared_ptr<SKAtomTreeNode> child : this->_childNodes)
  {
    this->_filteredAndSortedNodes.push_back(child);
  }
}


std::optional<size_t> SKAtomTreeNode::findChildIndex(std::shared_ptr<SKAtomTreeNode> child)
{
  std::vector<std::shared_ptr<SKAtomTreeNode>>::iterator itr = std::find_if(_childNodes.begin(), _childNodes.end(), [=](const std::shared_ptr<SKAtomTreeNode> x) {
      return x == child;
  });

  if (itr != _childNodes.end())
  {
    return std::distance(_childNodes.begin(), itr);
  }


  return std::nullopt;
}

int SKAtomTreeNode::row() const
{
  if (std::shared_ptr<SKAtomTreeNode> lockedParent = _parent.lock())
  {
    std::vector<std::shared_ptr<SKAtomTreeNode>>::iterator it = std::find(lockedParent->_childNodes.begin(), lockedParent->_childNodes.end(), shared_from_this());
    if (it != lockedParent->_childNodes.end())
    {
      return  std::distance(lockedParent->_childNodes.begin(), it);
    }
  }

  return 0;
}


QDataStream &operator<<(QDataStream& stream, const std::shared_ptr<SKAtomTreeNode>& node)
{
  stream << node->_versionNumber;
  stream << node->_displayName;
  stream << node-> _isGroup;
  stream << node-> _isEditable;

  stream << node->_representedObject;
  stream << node->_childNodes;

  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::shared_ptr<SKAtomTreeNode>& node)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > node->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "SKAtomTreeNode");
  }

  stream >> node->_displayName;
  stream >> node-> _isGroup;
  stream >> node-> _isEditable;

  stream >> node->_representedObject;
  stream >> node->_childNodes;

  for(std::shared_ptr<SKAtomTreeNode> child : node->_childNodes)
  {
    child->_parent = node;
  }

  return stream;
}
