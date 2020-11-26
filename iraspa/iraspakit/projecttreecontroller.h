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

#include <QAbstractItemModel>
#include <QModelIndex>
#include <unordered_set>
#include <vector>
#include <optional>
#include <foundationkit.h>
#include "projecttreenode.h"


// inserting with nil as parent means inserting in the rootNodes
// hiddenRooNode convenient for resursion of all nodes

struct project_indexPath_compare_less
{
  bool operator() (const std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>& lhs, const std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>& rhs) const
  {
    return lhs.second < rhs.second;
  }
};

struct project_indexPath_compare_greater_than
{
  bool operator() (const std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>& lhs, const std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>& rhs) const
  {
    return lhs.second > rhs.second;
  }
};

// set are ordered accoording to indexPaths
using ProjectSelectionIndexPaths = std::pair<IndexPath, std::set<IndexPath>>;

using ProjectSelectionNodesAndIndexPaths = std::pair<std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>,
                                                  std::set<std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>, project_indexPath_compare_less>>;

using ReversedProjectSelectionNodesAndIndexPaths = std::pair<std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>,
                                                          std::set<std::pair<std::shared_ptr<ProjectTreeNode>, IndexPath>, project_indexPath_compare_greater_than>>;

class ProjectTreeController
{
public:
    ProjectTreeController();
    ~ProjectTreeController();

    const std::shared_ptr<ProjectTreeNode> hiddenRootNode() const {return _hiddenRootNode;}
    void setHiddenRootNode(std::shared_ptr<ProjectTreeNode> rootNode) {_hiddenRootNode = rootNode;}

    const std::vector<std::shared_ptr<ProjectTreeNode>> rootNodes() const {return _hiddenRootNode->_childNodes;}
    const std::vector<std::shared_ptr<ProjectTreeNode>> filteredRootNodes() const {return _hiddenRootNode->_filteredAndSortedNodes;}

    void insertNodeInParent(std::shared_ptr<ProjectTreeNode> node, std::shared_ptr<ProjectTreeNode> parent, int index);
    void insertNodeAtIndexPath(std::shared_ptr<ProjectTreeNode> node, IndexPath path);

    bool isRootNode(ProjectTreeNode *node) const;
    bool nodeIsChildOfItem(std::shared_ptr<ProjectTreeNode> node, std::weak_ptr<ProjectTreeNode> item);
    bool isDescendantOfNode(std::shared_ptr<ProjectTreeNode> item, std::weak_ptr<ProjectTreeNode> parentNode);
    int filteredChildIndexOfItem(std::shared_ptr<ProjectTreeNode> parentItem, int index);
    std::vector<std::shared_ptr<ProjectTreeNode>> childrenForItem(std::shared_ptr<ProjectTreeNode> item);
    std::shared_ptr<ProjectTreeNode> nodeAtIndexPath(IndexPath indexpath) const;
    void appendToRootnodes(std::shared_ptr<ProjectTreeNode> item);
    void deleteSelection();

    std::vector<std::shared_ptr<ProjectTreeNode>> flattenedLeafNodes();

    std::shared_ptr<ProjectTreeNode> galleryProjects() {return _galleryProjects;}
    std::shared_ptr<ProjectTreeNode> localProjects() {return _localProjects;}
    std::shared_ptr<ProjectTreeNode> icloudProjects() {return _icloudProjects;}

    void clearSelection() {_selectionIndexPath = IndexPath(); _selectionIndexPaths.clear();}
    void clearFirstSelection() {_selectionIndexPath = IndexPath();;}
    void clearSecondSelection() {_selectionIndexPaths.clear();}

    IndexPath selectionIndexPath() const {return _selectionIndexPath;}
    void setSelectionIndexPath(IndexPath indexPath) {_selectionIndexPath = indexPath;}

    void insertSelectionIndexPath(IndexPath index) {_selectionIndexPaths.insert(index);}
    void removeSelectionIndexPath(IndexPath index) {_selectionIndexPaths.erase(index);}

    ProjectSelectionIndexPaths selectionIndexPaths() const;
    void setSelectionIndexPaths(ProjectSelectionIndexPaths selection);

    ProjectSelectionNodesAndIndexPaths selectionNodesAndIndexPaths() const;
    ProjectSelectionNodesAndIndexPaths selectionEditableNodesAndIndexPaths() const;
    void setSelectionIndexPaths(ProjectSelectionNodesAndIndexPaths selection);
    void subtractSelection(ProjectSelectionNodesAndIndexPaths selection);

    std::shared_ptr<ProjectTreeNode> selectedTreeNode();
    std::set<std::shared_ptr<ProjectTreeNode>> selectedTreeNodes();
    ProjectSelectionIndexPaths updateIndexPathsFromNodes(ProjectSelectionNodesAndIndexPaths selection);
private:
    qint64 _versionNumber{1};
    std::shared_ptr<ProjectTreeNode> _galleryProjects;
    std::shared_ptr<ProjectTreeNode> _localProjects;
    std::shared_ptr<ProjectTreeNode> _icloudProjects;

    std::shared_ptr<ProjectTreeNode> _hiddenRootNode;

    IndexPath _selectionIndexPath;
    std::set<IndexPath> _selectionIndexPaths{};
    std::function<bool(std::shared_ptr<ProjectTreeNode>)> _filterPredicate = [](std::shared_ptr<ProjectTreeNode>) { return true; };

    friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<ProjectTreeController> &);
    friend QDataStream &operator>>(QDataStream &, std::shared_ptr<ProjectTreeController> &);
};



