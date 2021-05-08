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

#include "projecttreecontroller.h"

#include <QColor>
#include <stack>
#include <QCoreApplication>

ProjectTreeController::ProjectTreeController(): _hiddenRootNode(std::make_shared<ProjectTreeNode>(QString("hidden root node"),true,false))
{
  std::shared_ptr<ProjectTreeNode> rootGalleryProjects = std::make_shared<ProjectTreeNode>(QCoreApplication::translate("ProjectTreeController", "GALLERY"), std::make_shared<iRASPAProject>(std::make_shared<ProjectGroup>()), false, false);
  std::shared_ptr<ProjectTreeNode> emptyLocalProjects1 = std::make_shared<ProjectTreeNode>(QString("  "), false, false);
  std::shared_ptr<ProjectTreeNode> rootLocalProjects = std::make_shared<ProjectTreeNode>(QCoreApplication::translate("ProjectTreeController", "LOCAL PROJECTS"), std::make_shared<iRASPAProject>(std::make_shared<ProjectGroup>()), false, false);
  std::shared_ptr<ProjectTreeNode> emptyLocalProjects2 = std::make_shared<ProjectTreeNode>(QString("  "), false, false);
  std::shared_ptr<ProjectTreeNode> rootIcloudProjects = std::make_shared<ProjectTreeNode>(QCoreApplication::translate("ProjectTreeController", "DATABASES PUBLIC"), std::make_shared<iRASPAProject>(std::make_shared<ProjectGroup>()), false, false);

  _hiddenRootNode->insertChild(0, rootGalleryProjects);
  _hiddenRootNode->insertChild(1, emptyLocalProjects1);
  _hiddenRootNode->insertChild(2, rootLocalProjects);
  _hiddenRootNode->insertChild(3, emptyLocalProjects2);
  _hiddenRootNode->insertChild(4, rootIcloudProjects);

  _galleryProjects = std::make_shared<ProjectTreeNode>(QCoreApplication::translate("ProjectTreeController", "Gallery"), std::make_shared<iRASPAProject>(std::make_shared<ProjectGroup>()), false, false);
  _localProjects = std::make_shared<ProjectTreeNode>(QCoreApplication::translate("ProjectTreeController", "Local Projects"), std::make_shared<iRASPAProject>(std::make_shared<ProjectGroup>()), false, false);
  _icloudProjects = std::make_shared<ProjectTreeNode>(QCoreApplication::translate("ProjectTreeController", "Databases Public"), std::make_shared<iRASPAProject>(std::make_shared<ProjectGroup>()), false, false);

  rootGalleryProjects->insertChild(0,_galleryProjects);
  rootLocalProjects->insertChild(0,_localProjects);
  rootIcloudProjects->insertChild(0,_icloudProjects);

  _localProjects->setIsDropEnabled(true);
}

ProjectTreeController::~ProjectTreeController()
{
  _hiddenRootNode = nullptr;
  _selectionIndexPath = IndexPath();
  _selectionIndexPaths.clear();
}

bool ProjectTreeController::isRootNode(ProjectTreeNode *node) const
{
  return node == _hiddenRootNode.get();
}

void ProjectTreeController::insertNodeInParent(std::shared_ptr<ProjectTreeNode> node, std::shared_ptr<ProjectTreeNode> parent, int index)
{
  if (parent != nullptr)
  {
    parent->_childNodes.insert(parent->_childNodes.begin() + index, node);
    node->_parent = parent;
    parent->updateFilteredChildren(this->_filterPredicate);
  }
  else
  {
    _hiddenRootNode->_childNodes.insert(_hiddenRootNode->_childNodes.begin() + index, node);
    node->_parent = _hiddenRootNode;
    _hiddenRootNode->updateFilteredChildren(this->_filterPredicate);
  }
}

std::shared_ptr<ProjectTreeNode> ProjectTreeController::nodeAtIndexPath(IndexPath indexPath) const
{
  if(indexPath.empty()) return nullptr;
  return _hiddenRootNode->descendantNodeAtIndexPath(indexPath);
}

void ProjectTreeController::insertNodeAtIndexPath(std::shared_ptr<ProjectTreeNode> node, IndexPath path)
{
  int index = int(path.lastIndex());
  std::shared_ptr<ProjectTreeNode> parent = _hiddenRootNode->descendantNodeAtIndexPath(path.removingLastIndex());
  node->insertInParent(parent, index);
}



void ProjectTreeController::appendToRootnodes(std::shared_ptr<ProjectTreeNode> item)
{
  item->_parent = _hiddenRootNode;
  _hiddenRootNode->_childNodes.push_back(item);
}



// MARK: Selection
// =========================

// Deletion is done bottom-up (post-order).

void ProjectTreeController::deleteSelection()
{
    /*
  std::vector<ProjectTreeNode*> atomNodes;
  std::copy(_selectedTreeNodes.begin(), _selectedTreeNodes.end(), std::back_inserter(atomNodes));

  std::sort(atomNodes.begin(), atomNodes.end(), [](ProjectTreeNode* a, ProjectTreeNode* b) -> bool {
      return a->indexPath() > b->indexPath();
  });

  for(ProjectTreeNode* atom : atomNodes)
  {
    IndexPath indexPath = atom->indexPath();
    int lastIndex = indexPath.lastIndex();
    const ProjectTreeNode* parentItem = atom->parent();
    parentItem->childNodes().erase(parentItem->childNodes().begin() + lastIndex);
  };

  _selectedTreeNode = nullptr;
  _selectedTreeNodes.clear();
  */
}

std::vector<std::shared_ptr<ProjectTreeNode>> ProjectTreeController::flattenedLeafNodes()
{
  return _hiddenRootNode->flattenedLeafNodes();
}


// MARK: Selection
// =============================================================================================

ProjectSelectionIndexPaths ProjectTreeController::selectionIndexPaths() const
{
  return std::make_pair(_selectionIndexPath, _selectionIndexPaths);
}

void ProjectTreeController::setSelectionIndexPaths(AtomSelectionIndexPaths selection)
{
  _selectionIndexPath = selection.first;
  _selectionIndexPaths = selection.second;
}

ProjectSelectionNodesAndIndexPaths ProjectTreeController::selectionNodesAndIndexPaths() const
{
  std::set<std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>, project_indexPath_compare_less> nodesAndIndexPaths{};

  std::transform(_selectionIndexPaths.begin(), _selectionIndexPaths.end(), std::inserter(nodesAndIndexPaths, nodesAndIndexPaths.begin()),
                 [this](IndexPath indexPath) -> std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>
                 {return std::make_pair(this->nodeAtIndexPath(indexPath), indexPath);});
  return std::make_pair(std::make_pair(this->nodeAtIndexPath(_selectionIndexPath), _selectionIndexPath),nodesAndIndexPaths);
}

ProjectSelectionNodesAndIndexPaths ProjectTreeController::selectionEditableNodesAndIndexPaths() const
{
  ProjectSelectionNodesAndIndexPaths editableNodesAndIndexPaths{};
  ProjectSelectionNodesAndIndexPaths nodesAndIndexPaths = selectionNodesAndIndexPaths();

  if(nodesAndIndexPaths.first.first && nodesAndIndexPaths.first.first->isEditable())
  {
    editableNodesAndIndexPaths.first = nodesAndIndexPaths.first;
  }
  else
  {
    editableNodesAndIndexPaths.first = std::make_pair(nullptr, IndexPath{});
  }

  std::copy_if(nodesAndIndexPaths.second.begin(), nodesAndIndexPaths.second.end(), std::inserter(editableNodesAndIndexPaths.second, editableNodesAndIndexPaths.second.begin()),
                 [](std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath> pair) -> bool
                 {return pair.first->isEditable();});
  return editableNodesAndIndexPaths;
}

void ProjectTreeController::setSelectionIndexPaths(ProjectSelectionNodesAndIndexPaths selection)
{
  _selectionIndexPaths.clear();
  _selectionIndexPath = selection.first.second;
  std::transform(selection.second.begin(), selection.second.end(), std::inserter(_selectionIndexPaths, _selectionIndexPaths.begin()),
                 [](std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath> pair) -> IndexPath
                 {return pair.second;});
}

void ProjectTreeController::subtractSelection(ProjectSelectionNodesAndIndexPaths selection)
{
  std::set<IndexPath> differenceSelectionIndexPaths{};
  std::set<IndexPath> newSelectionIndexPaths{};
  std::transform(selection.second.begin(), selection.second.end(), std::inserter(newSelectionIndexPaths, newSelectionIndexPaths.begin()),
                 [](std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath> pair) -> IndexPath
                 {return pair.second;});

  std::set_difference(_selectionIndexPaths.begin(), _selectionIndexPaths.end(), newSelectionIndexPaths.begin(), newSelectionIndexPaths.end(),
                          std::inserter(differenceSelectionIndexPaths, differenceSelectionIndexPaths.begin()));
  _selectionIndexPaths = differenceSelectionIndexPaths;

  qDebug() << "first: " << selectedTreeNode()->displayName()  << ", second: " << selection.first.first->displayName();
  if(selectedTreeNode() == selection.first.first)
  {
    _selectionIndexPath = IndexPath{};
  }
}

std::shared_ptr<ProjectTreeNode> ProjectTreeController::selectedTreeNode()
{
  if(_selectionIndexPath.empty())
    return nullptr;
  return nodeAtIndexPath(_selectionIndexPath);
}

std::set<std::shared_ptr<ProjectTreeNode>> ProjectTreeController::selectedTreeNodes()
{
  std::set<std::shared_ptr<ProjectTreeNode>> selectedAtomTreeNodes{};
  std::transform(_selectionIndexPaths.begin(), _selectionIndexPaths.end(), std::inserter(selectedAtomTreeNodes, selectedAtomTreeNodes.begin()),
                   [this](IndexPath indexPath) -> std::shared_ptr<ProjectTreeNode>
                          {return nodeAtIndexPath(indexPath);});
  return selectedAtomTreeNodes;
}

ProjectSelectionIndexPaths ProjectTreeController::updateIndexPathsFromNodes(ProjectSelectionNodesAndIndexPaths selection)
{
  ProjectSelectionIndexPaths updatedSelection{};
  if(selection.first.first)
  {
    updatedSelection.first = selection.first.first->indexPath();
  }

  std::transform(selection.second.begin(), selection.second.end(), std::inserter(updatedSelection.second, updatedSelection.second.begin()),
                 [](std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath> pair) -> IndexPath
                 {return pair.first->indexPath();});

  return updatedSelection;
}

QDataStream &operator<<(QDataStream& stream, const std::shared_ptr<ProjectTreeController>& controller)
{
  stream << controller->_versionNumber;
  stream << controller->_localProjects;
  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::shared_ptr<ProjectTreeController>& controller)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > controller->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "Project");
  }
  stream >> controller->_localProjects;
  return stream;
}
