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

#include <vector>
#include <set>
#include <tuple>
#include <memory>
#include <functional>
#include <QDataStream>
#include <optional>
#include <foundationkit.h>
#include "indexpath.h"
#include "skasymmetricatom.h"

class SKAtomTreeController;

class SKAtomTreeNode: public std::enable_shared_from_this<SKAtomTreeNode>
{
public:
    SKAtomTreeNode(): _representedObject(std::make_shared<SKAsymmetricAtom>()) {}
    SKAtomTreeNode(std::shared_ptr<SKAsymmetricAtom> representedObject): _representedObject(representedObject) {_displayName = representedObject->displayName();}
    SKAtomTreeNode(std::weak_ptr<SKAtomTreeNode> parent, std::shared_ptr<SKAsymmetricAtom> representedObject): _parent(parent), _representedObject(representedObject) {_displayName = representedObject->displayName();}
    ~SKAtomTreeNode();

    inline  std::vector<std::shared_ptr<SKAtomTreeNode>> childNodes() const {return this->_childNodes;}
    std::shared_ptr<SKAtomTreeNode> getChildNode(int index) {return _childNodes[index];}
    inline  std::vector<std::shared_ptr<SKAtomTreeNode>> filteredChildNodes() const {return this->_filteredAndSortedNodes;}
    inline const std::shared_ptr<SKAtomTreeNode> parent() const {return _parent.lock();}
    inline const std::shared_ptr<SKAsymmetricAtom> representedObject() const {return this->_representedObject;}
    inline const QString displayName() const {return this->_displayName;}
    void setDisplayName(QString name) {_displayName = name;}

    size_t childCount() {return _childNodes.size();}
    inline bool isLeaf() {return !_isGroup;}
    inline bool isGroup() {return _isGroup;}
    void setIsGroup(bool value) {_isGroup = value;}
    bool insertChild(size_t row, std::shared_ptr<SKAtomTreeNode> child);
    void insertInParent(std::shared_ptr<SKAtomTreeNode> parent, size_t index);
    void appendToParent(std::shared_ptr<SKAtomTreeNode> parent);
    bool removeChild(size_t position);
    bool removeChildren(size_t position, size_t count);
    void removeFromParent();
    const IndexPath indexPath();
    std::shared_ptr<SKAtomTreeNode> descendantNodeAtIndexPath(IndexPath indexPath);
    std::vector<std::shared_ptr<SKAtomTreeNode>> flattenedNodes();
    std::vector<std::shared_ptr<SKAtomTreeNode>> flattenedLeafNodes();
    std::vector<std::shared_ptr<SKAtomTreeNode>> flattenedGroupNodes();
    std::vector<std::shared_ptr<SKAtomTreeNode>> descendants();
    std::vector<std::shared_ptr<SKAtomTreeNode>> descendantNodes();
    std::vector<std::shared_ptr<SKAtomTreeNode>> descendantLeafNodes();
    std::vector<std::shared_ptr<SKAtomTreeNode>> descendantGroupNodes();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> flattenedObjects();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> descendantObjects();
    bool isDescendantOfNode(std::shared_ptr<SKAtomTreeNode> parent);
    void updateFilteredChildren(std::function<bool(std::shared_ptr<SKAtomTreeNode>)> predicate);
    void updateFilteredChildrenRecursively(std::function<bool(std::shared_ptr<SKAtomTreeNode>)> predicate);
    void setFilteredNodesAsMatching();
    std::optional<size_t> findChildIndex(std::shared_ptr<SKAtomTreeNode> child);
    int row() const;
    void setParent(std::shared_ptr<SKAtomTreeNode> parent) {_parent = parent;}
    void setGroupItem(bool state) {_isGroup = state;}
private:
    qint64 _versionNumber{1};
    QString _displayName;
    std::weak_ptr<SKAtomTreeNode> _parent;
    std::shared_ptr<SKAsymmetricAtom> _representedObject;

    std::vector<std::shared_ptr<SKAtomTreeNode>> _childNodes{};
    std::vector<std::shared_ptr<SKAtomTreeNode>> _filteredAndSortedNodes{};

    bool _matchesFilter = true;
    bool _selected = true;
    bool _isGroup = false;
    bool _isEditable = false;

    friend SKAtomTreeController ;

    friend QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<SKAtomTreeNode>>& val);
    friend QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<SKAtomTreeNode>>& val);

    friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<SKAtomTreeNode> &);
    friend QDataStream &operator>>(QDataStream &, std::shared_ptr<SKAtomTreeNode> &);
};

