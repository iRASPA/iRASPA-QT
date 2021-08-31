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

#include "projecttreenode.h"
#include <assert.h>
#include <vector>
#include <foundationkit.h>
#include <QBuffer>

ProjectTreeNode::~ProjectTreeNode()
{
}

char ProjectTreeNode::mimeType[] = "application/x-qt-iraspa-project-mime";

std::shared_ptr<iRASPAProject> ProjectTreeNode::representedObject()
{
  return _representedObject;
}

size_t ProjectTreeNode::row() const
{
  if (std::shared_ptr<ProjectTreeNode> lockedParent = _parent.lock())
  {
    std::vector<std::shared_ptr<ProjectTreeNode>>::iterator it = std::find(lockedParent->_childNodes.begin(), lockedParent->_childNodes.end(), shared_from_this());
    if (it != lockedParent->_childNodes.end())
    {
      return  std::distance(lockedParent->_childNodes.begin(), it);
    }
  }

  return 0;
}


std::optional<size_t> ProjectTreeNode::findChildIndex(std::shared_ptr<ProjectTreeNode> child)
{
  std::vector<std::shared_ptr<ProjectTreeNode>>::iterator itr = std::find_if(_childNodes.begin(), _childNodes.end(), [=](const std::shared_ptr<ProjectTreeNode> x) {
      return x == child;
  });

  if (itr != _childNodes.end())
  {
    return std::distance(_childNodes.begin(), itr);
  }


  return std::nullopt;
}

bool ProjectTreeNode::removeChild(size_t row)
{
  if (row < 0 || row >= _childNodes.size())
     return false;

  _childNodes.erase(_childNodes.begin() + row);
  return true;
}

bool ProjectTreeNode::insertChild(size_t row, std::shared_ptr<ProjectTreeNode> child)
{
  if (row < 0 || row > _childNodes.size())
    return false;

  child->_parent = shared_from_this();
  _childNodes.insert(_childNodes.begin() + row, child);
  return true;
}

const IndexPath ProjectTreeNode::indexPath()
{
  if (std::shared_ptr<ProjectTreeNode> lockedParent =  _parent.lock())
  {
    IndexPath indexpath = lockedParent->indexPath();
    std::optional<size_t> index = lockedParent->findChildIndex(shared_from_this());
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
    else
    {
      qDebug() << "NOT FOUND!";
    }
  }

  return IndexPath();
}

void ProjectTreeNode::removeFromParent()
{
  if(std::shared_ptr<ProjectTreeNode> lockedParent = _parent.lock())
  {
    std::optional<size_t> index = lockedParent->findChildIndex(shared_from_this());
    assert(index);
    if(index)
    {
      lockedParent->_childNodes.erase(lockedParent->_childNodes.begin() + *index);
    }
    _parent.reset();
  }
}

std::shared_ptr<ProjectTreeNode> ProjectTreeNode::descendantNodeAtIndexPath(IndexPath indexPath)
{
  size_t length = indexPath.size();
  std::shared_ptr<ProjectTreeNode> node = shared_from_this();

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


void ProjectTreeNode::insertInParent(std::shared_ptr<ProjectTreeNode> parent, size_t index)
{
  this->_parent = parent;
  parent->_childNodes.insert(parent->_childNodes.begin() + index, shared_from_this());
}


void ProjectTreeNode::appendToParent(std::shared_ptr<ProjectTreeNode> parent1)
{
  this->_parent = parent1;
  parent1->_childNodes.push_back(shared_from_this());
}



std::vector<std::shared_ptr<ProjectTreeNode>> ProjectTreeNode::flattenedNodes()
{
  std::vector<std::shared_ptr<ProjectTreeNode>> nodes{shared_from_this()};

  std::vector<std::shared_ptr<ProjectTreeNode>> otherNodes = this->descendantNodes();
  for(const std::shared_ptr<ProjectTreeNode> &child : otherNodes)
  {
    nodes.push_back(child);
  }
  return nodes;
}

std::vector<std::shared_ptr<ProjectTreeNode>> ProjectTreeNode::flattenedLeafNodes()
{
  std::vector<std::shared_ptr<ProjectTreeNode>> nodes{};

  if(this->representedObject()->isLeaf())
  {
    nodes.push_back(shared_from_this());
  }

  std::vector<std::shared_ptr<ProjectTreeNode>> otherNodes = this->descendantLeafNodes();
  for(const std::shared_ptr<ProjectTreeNode> &child : otherNodes)
  {
    nodes.push_back(child);
  }
  return nodes;
}

std::vector<std::shared_ptr<ProjectTreeNode>> ProjectTreeNode::flattenedGroupNodes()
{
  std::vector<std::shared_ptr<ProjectTreeNode>> nodes{};

  if(this->_representedObject->isGroup())
  {
    nodes.push_back(shared_from_this());
  }

  std::vector<std::shared_ptr<ProjectTreeNode>> otherNodes = this->descendantGroupNodes();
  for(const std::shared_ptr<ProjectTreeNode> &child : otherNodes)
  {
    nodes.push_back(child);
  }
  return nodes;
}


bool ProjectTreeNode::isDescendantOfNode(ProjectTreeNode* parent)
{
  ProjectTreeNode* treeNode = this;

  while(treeNode != nullptr)
  {
    if(treeNode == parent)
    {
      return true;
    }
    else
    {
      treeNode = treeNode->_parent.lock().get();
    }
  }

  return false;
}

std::vector<std::shared_ptr<ProjectTreeNode>> ProjectTreeNode::descendants()
{
  std::vector<std::shared_ptr<ProjectTreeNode>> descendants{};
  for(std::shared_ptr<ProjectTreeNode> child : this->_childNodes)
  {
    if(child->representedObject()->isLeaf())
    {
      descendants.push_back(child);
    }
    else
    {
      std::vector<std::shared_ptr<ProjectTreeNode>> otherNodes = child->descendants();
      for(const std::shared_ptr<ProjectTreeNode> &node : otherNodes)
      {
        descendants.push_back(node);
      }
    }
  }

  return descendants;
}

std::vector<std::shared_ptr<ProjectTreeNode>> ProjectTreeNode::descendantNodes()
{
    std::vector<std::shared_ptr<ProjectTreeNode>> descendants{};
    for(std::shared_ptr<ProjectTreeNode> child : this->_childNodes)
    {
      descendants.push_back(child);
      if (child->representedObject()->isGroup())
      {
        std::vector<std::shared_ptr<ProjectTreeNode>> otherNodes = child->descendantNodes();
        for(const std::shared_ptr<ProjectTreeNode> &node : otherNodes)
        {
          descendants.push_back(node);
        }
      }
    }

    return descendants;
}

std::vector<std::shared_ptr<ProjectTreeNode>> ProjectTreeNode::descendantLeafNodes()
{
  std::vector<std::shared_ptr<ProjectTreeNode>> descendants{};
  for(std::shared_ptr<ProjectTreeNode> child : this->_childNodes)
  {
    if(child->representedObject()->isLeaf())
    {
      descendants.push_back(child);
    }
    else
    {
      std::vector<std::shared_ptr<ProjectTreeNode>> otherNodes = child->descendantLeafNodes();
      for(const std::shared_ptr<ProjectTreeNode> &node : otherNodes)
      {
        descendants.push_back(node);
      }
    }
  }

  return descendants;
}

std::vector<std::shared_ptr<ProjectTreeNode>> ProjectTreeNode::descendantGroupNodes()
{
  std::vector<std::shared_ptr<ProjectTreeNode>> descendants{};
  for(std::shared_ptr<ProjectTreeNode> child : this->_childNodes)
  {
    if(child->representedObject()->isGroup())
    {
      descendants.push_back(child);
      std::vector<std::shared_ptr<ProjectTreeNode>> otherNodes = child->descendantGroupNodes();
      for(const std::shared_ptr<ProjectTreeNode> &node : otherNodes)
      {
        descendants.push_back(node);
      }
    }
  }

  return descendants;
}

std::vector<std::shared_ptr<iRASPAProject>> ProjectTreeNode::flattenedObjects()
{
  std::vector<std::shared_ptr<iRASPAProject>> objects{this->_representedObject};

  std::vector<std::shared_ptr<iRASPAProject>> groupObjects = this->descendantObjects();
  for(const std::shared_ptr<iRASPAProject> &object : groupObjects)
  {
    objects.push_back(object);
  }
  return objects;
}


std::vector<std::shared_ptr<iRASPAProject>> ProjectTreeNode::descendantObjects()
{
  std::vector<std::shared_ptr<iRASPAProject>> descendants{};
  for(std::shared_ptr<ProjectTreeNode> child : this->_childNodes)
  {
    descendants.push_back(child->_representedObject);
    if(child->representedObject()->isGroup())
    {
      std::vector<std::shared_ptr<iRASPAProject>> groupObjects = child->descendantObjects();
      for(const std::shared_ptr<iRASPAProject> &object : groupObjects)
      {
        descendants.push_back(object);
      }
    }
  }
  return descendants;
}


// MARK: -
// MARK: Filtering support

void ProjectTreeNode::updateFilteredChildren(std::function<bool(std::shared_ptr<ProjectTreeNode>)> predicate)
{
  for(const std::shared_ptr<ProjectTreeNode> &child : this->_childNodes)
  {
    child->_matchesFilter = true;
    child->_matchesFilter = predicate(shared_from_this());
  }

  this->_filteredAndSortedNodes.clear();
  for(const std::shared_ptr<ProjectTreeNode> &child : this->_childNodes)
  {
    if (child->_matchesFilter) this->_filteredAndSortedNodes.push_back(child);
  }

  // if we have filtered nodes, then all parents of this node needs to be included
  if(this->_filteredAndSortedNodes.size() > 0)
  {
    this->_matchesFilter = true;
  }
}

void ProjectTreeNode::updateFilteredChildrenRecursively(std::function<bool(std::shared_ptr<ProjectTreeNode>)> predicate)
{
  this->_matchesFilter = false;
  this->_matchesFilter = predicate(shared_from_this());

  for(std::shared_ptr<ProjectTreeNode> child : this->_childNodes)
  {
    child->updateFilteredChildrenRecursively(predicate);
  }

  if(this->_matchesFilter)
  {
      /*
    if(std::shared_ptr<ProjectTreeNode> lockedParent = _parent.lock())
    {
      lockedParent->_matchesFilter = true;
    }*/
  }

  this->_filteredAndSortedNodes.clear();
  for(const std::shared_ptr<ProjectTreeNode> &child : this->_childNodes)
  {
    if (child->_matchesFilter) this->_filteredAndSortedNodes.push_back(child);
  }
}

void ProjectTreeNode::setFilteredNodesAsMatching()
{
  this->_matchesFilter = false;

  this->_filteredAndSortedNodes.clear();
  for(const std::shared_ptr<ProjectTreeNode> &child : this->_childNodes)
  {
    this->_filteredAndSortedNodes.push_back(child);
  }
}




bool ProjectTreeNode::insertChildren(size_t position, size_t count, [[maybe_unused]] size_t columns)
{
  if (position < 0 || position > _childNodes.size())
    return false;

  for (size_t row = 0; row < count; ++row)
  {
    std::shared_ptr<ProjectTreeNode> item = std::make_shared<ProjectTreeNode>(QString("tree node"));
    item->_parent = shared_from_this();
    _childNodes.insert(_childNodes.begin() + position, item);
  }

  return true;
}

bool ProjectTreeNode::setData(const QVariant &value)
{
  _displayName = value.toString();
  return true;
}

std::shared_ptr<ProjectTreeNode> ProjectTreeNode::shallowClone()
{
  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream <<= this->shared_from_this();

  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  std::shared_ptr<ProjectTreeNode> projectTreeNode = std::make_shared<ProjectTreeNode>();
  streamRead >>= projectTreeNode;

  return projectTreeNode;
}

QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<ProjectTreeNode>>& val)
{
  stream << static_cast<int32_t>(val.size());
  for(const std::shared_ptr<ProjectTreeNode>& singleVal : val)
    stream << singleVal;
  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<ProjectTreeNode>>& val)
{
  int32_t vecSize;
  val.clear();
  stream >> vecSize;
  val.reserve(vecSize);

  while(vecSize--)
  {
    std::shared_ptr<ProjectTreeNode> tempVal = std::make_shared<ProjectTreeNode>();
    stream >> tempVal;
    val.push_back(tempVal);
  }
  return stream;
}

QDataStream &operator<<(QDataStream& stream, const std::shared_ptr<ProjectTreeNode>& node)
{
  stream << node->_versionNumber;
  stream << node->_displayName;

  stream << node->_isEditable;
  stream << node->_isDropEnabled;

  stream << node->_representedObject;

  // save picture in PNG format
  QByteArray imageByteArray;
  QBuffer buffer(&imageByteArray);
  buffer.open(QIODevice::WriteOnly);
  node->_thumbnail.save(&buffer,"PNG");
  stream << imageByteArray;

  stream << node->_childNodes;

  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::shared_ptr<ProjectTreeNode>& node)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > node->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "ProjectTreeNode");
  }
  stream >> node->_displayName;
  stream >> node->_isEditable;
  stream >> node->_isDropEnabled;

  stream >> node->_representedObject;

  if(versionNumber >= 2) // introduced in version 2
  {
     // read picture in PNG-format
     QByteArray imageByteArray;
     stream >> imageByteArray;
     QBuffer buffer(&imageByteArray);
     buffer.open(QIODevice::ReadOnly);
     node->_thumbnail.load(&buffer, "PNG");
  }

  stream >> node->_childNodes;

  for(std::shared_ptr<ProjectTreeNode> &child : node->_childNodes)
  {
    child->_parent = node;
  }

  return stream;
}

QDataStream &operator<<=(QDataStream& stream, const std::shared_ptr<ProjectTreeNode>& node)
{
  stream << node->_versionNumber;
  stream << node->_displayName;

  stream << node->_isEditable;
  stream << node->_isDropEnabled;

  stream <<= node->_representedObject;

  // save picture in PNG format
  QByteArray imageByteArray;
  QBuffer buffer(&imageByteArray);
  buffer.open(QIODevice::WriteOnly);
  node->_thumbnail.save(&buffer,"PNG");
  stream << imageByteArray;


  return stream;
}

QDataStream &operator>>=(QDataStream& stream, std::shared_ptr<ProjectTreeNode>& node)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > node->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "ProjectTreeNode");
  }
  stream >> node->_displayName;
  stream >> node->_isEditable;
  stream >> node->_isDropEnabled;

  stream >>= node->_representedObject;

  if(versionNumber >= 2) // introduced in version 2
  {
     // read picture in PNG-format
     QByteArray imageByteArray;
     stream >> imageByteArray;
     QBuffer buffer(&imageByteArray);
     buffer.open(QIODevice::ReadOnly);
     node->_thumbnail.load(&buffer, "PNG");
  }

  return stream;
}

