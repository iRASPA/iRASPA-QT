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

#include <unordered_set>
#include <vector>
#include <utility>
#include <optional>
#include <foundationkit.h>
#include "skelement.h"
#include "skatomtreenode.h"

// inserting with nil as parent means inserting in the rootNodes
// hiddenRootNode is convenient for recursion visiting all nodes

struct atom_indexPath_compare_less
{
  bool operator() (const std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>& lhs, const std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>& rhs) const
  {
    return lhs.second < rhs.second;
  }
};

struct atom_indexPath_compare_greater_than
{
  bool operator() (const std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>& lhs, const std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>& rhs) const
  {
    return lhs.second > rhs.second;
  }
};

// set are ordered accoording to indexPaths
using AtomSelectionIndexPaths = std::pair<IndexPath, std::set<IndexPath>>;

using AtomSelectionNodesAndIndexPaths = std::pair<std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>,
                                                  std::set<std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>, atom_indexPath_compare_less>>;

using ReversedAtomSelectionNodesAndIndexPaths = std::pair<std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>,
                                                          std::set<std::pair<std::shared_ptr<SKAtomTreeNode>, IndexPath>, atom_indexPath_compare_greater_than>>;


class SKAtomTreeController
{
public:
    SKAtomTreeController();
    ~SKAtomTreeController();

    const std::vector<std::shared_ptr<SKAtomTreeNode>> rootNodes() const {return _hiddenRootNode->_childNodes;}
    const std::vector<std::shared_ptr<SKAtomTreeNode>> filteredRootNodes() const {return _hiddenRootNode->_filteredAndSortedNodes;}

    std::shared_ptr<SKAtomTreeNode> hiddenRootNode() {return _hiddenRootNode;}
    void insertNodeInParent(std::shared_ptr<SKAtomTreeNode> node, std::weak_ptr<SKAtomTreeNode> parent, int index);
    void insertNodeAtIndexPath(std::shared_ptr<SKAtomTreeNode> node, IndexPath path);
    void removeNode(std::shared_ptr<SKAtomTreeNode> node);
    bool isRootNode(SKAtomTreeNode *node);
    bool nodeIsChildOfItem(std::shared_ptr<SKAtomTreeNode> node, std::shared_ptr<SKAtomTreeNode> item);
    bool isDescendantOfNode(std::shared_ptr<SKAtomTreeNode> item, std::shared_ptr<SKAtomTreeNode> parentNode);
    size_t filteredChildIndexOfItem(std::shared_ptr<SKAtomTreeNode> parentItem, size_t index);
    std::vector<std::shared_ptr<SKAtomTreeNode>> childrenForItem(std::shared_ptr<SKAtomTreeNode> item);
    std::shared_ptr<SKAtomTreeNode> nodeAtIndexPath(IndexPath indexpath) const;
    void appendToRootnodes(std::shared_ptr<SKAtomTreeNode> item);

    std::vector<std::shared_ptr<SKAtomTreeNode>> flattenedLeafNodes() const;
    std::vector<std::shared_ptr<SKAsymmetricAtom>> flattenedObjects() const;
    std::vector<std::shared_ptr<SKAtomCopy>> allAtomCopies() const;
    std::vector<std::shared_ptr<SKAtomCopy>> atomCopies() const;

    void setTags();

    void clearSelection() {_selectionIndexPaths.clear();}

    IndexPath selectionIndexPath() {return _selectionIndexPath;}
    std::set<IndexPath> &selectionIndexPathSet() {return _selectionIndexPaths;}

    void insertSelectionIndexPath(IndexPath index) {_selectionIndexPaths.insert(index);}
    void removeSelectionIndexPath(IndexPath index) {_selectionIndexPaths.erase(index);}

    AtomSelectionIndexPaths selectionIndexPaths() const;
    void setSelectionIndexPaths(AtomSelectionIndexPaths selection);

    AtomSelectionNodesAndIndexPaths selectionNodesAndIndexPaths() const;
    void setSelectionIndexPaths(AtomSelectionNodesAndIndexPaths selection);

    double netCharge();

    std::set<std::shared_ptr<SKAtomTreeNode>> selectedTreeNodes();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> selectedObjects();

    AtomSelectionIndexPaths updateIndexPathsFromNodes(AtomSelectionNodesAndIndexPaths selection);
private:
    qint64 _versionNumber{2};

    std::shared_ptr<SKAtomTreeNode> _hiddenRootNode{nullptr};

    IndexPath _selectionIndexPath;
    std::set<IndexPath> _selectionIndexPaths{};

    std::function<bool(std::shared_ptr<SKAtomTreeNode>)> _filterPredicate = [](std::shared_ptr<SKAtomTreeNode>) { return true; };

    friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<SKAtomTreeController> &);
    friend QDataStream &operator>>(QDataStream &, std::shared_ptr<SKAtomTreeController> &);
};



