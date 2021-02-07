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

#include <QString>
#include <vector>
#include <memory>
#include <iostream>
#include <foundationkit.h>
#include "iraspaproject.h"

class ProjectTreeController;

class ProjectTreeNode: public std::enable_shared_from_this<ProjectTreeNode>
{
public:

    enum class Type
    {
      gallery = 0, user = 1, cloud = 2
    };

    static char mimeType[];

    ProjectTreeNode(QString displayName = "Default", bool isEditable=true, bool isDropEnabled=true): _type(Type::user), _displayName(displayName), _representedObject(std::make_shared<iRASPAProject>()), _isEditable(isEditable), _isDropEnabled(isDropEnabled) {}
    ProjectTreeNode(std::shared_ptr<iRASPAProject> representedObject, bool isEditable=true, bool isDropEnabled=true): _type(Type::user), _representedObject(representedObject), _isEditable(isEditable), _isDropEnabled(isDropEnabled) {}
    ProjectTreeNode(QString displayName, std::shared_ptr<iRASPAProject> representedObject, bool isEditable=true, bool isDropEnabled=true): _type(Type::user), _displayName(displayName), _representedObject(representedObject), _isEditable(isEditable), _isDropEnabled(isDropEnabled) {}
    ~ProjectTreeNode();

    std::shared_ptr<ProjectTreeNode> shallowClone();

    bool removeChild(size_t row);
    bool insertChild(size_t row, std::shared_ptr<ProjectTreeNode> child);

    std::shared_ptr<ProjectTreeNode> getChildNode(size_t index) {return _childNodes[index];}
    size_t row() const;
    size_t childCount() {return _childNodes.size();}

    inline  std::vector<std::shared_ptr<ProjectTreeNode>> &childNodes() {return this->_childNodes;}
    inline  std::vector<std::shared_ptr<ProjectTreeNode>> &filteredChildNodes() {return this->_filteredAndSortedNodes;}
    //inline std::weak_ptr<ProjectTreeNode> parent()  {return this->_parent;}
    inline const std::shared_ptr<ProjectTreeNode> parent() const {return _parent.lock();}
    std::shared_ptr<iRASPAProject> representedObject();

    inline const QString displayName() const {return this->_displayName;}
    void setDisplayName(QString name) {_displayName = name;}

    void setRepresentedObject(std::shared_ptr<iRASPAProject> object) {_representedObject = object;}

    void setIsEditable(bool editable) {_isEditable = editable;}
    inline bool isEditable() {return _isEditable;}
    inline bool isDropEnabled() {return _isDropEnabled;}
    void insertInParent(std::shared_ptr<ProjectTreeNode> parent, size_t index);
    void appendToParent(std::shared_ptr<ProjectTreeNode> parent);
    const IndexPath indexPath();
    std::optional<size_t> findChildIndex(std::shared_ptr<ProjectTreeNode> child);
    std::shared_ptr<ProjectTreeNode> descendantNodeAtIndexPath(IndexPath indexPath);
    std::vector<std::shared_ptr<ProjectTreeNode>> flattenedNodes();
    std::vector<std::shared_ptr<ProjectTreeNode>> flattenedLeafNodes();
    std::vector<std::shared_ptr<ProjectTreeNode>> flattenedGroupNodes();
    std::vector<std::shared_ptr<ProjectTreeNode>> descendants();
    std::vector<std::shared_ptr<ProjectTreeNode>> descendantNodes();
    std::vector<std::shared_ptr<ProjectTreeNode>> descendantLeafNodes();
    std::vector<std::shared_ptr<ProjectTreeNode>> descendantGroupNodes();
    std::vector<std::shared_ptr<iRASPAProject>> flattenedObjects();
    std::vector<std::shared_ptr<iRASPAProject>> descendantObjects();
    bool isDescendantOfNode(ProjectTreeNode* parent);
    void updateFilteredChildren(std::function<bool(std::shared_ptr<ProjectTreeNode>)> predicate);
    void updateFilteredChildrenRecursively(std::function<bool(std::shared_ptr<ProjectTreeNode>)> predicate);
    void setFilteredNodesAsMatching();
    bool insertChildren(size_t position, size_t count, size_t columns);
    bool setData(const QVariant &value);
    Type type() {return _type;}
    void setType(Type type) {_type = type;}
    void setIsDropEnabled(bool enabled) {_isDropEnabled = enabled;}
    void removeFromParent();
private:
    qint64 _versionNumber{1};
    Type _type;
    QString _displayName;
    std::weak_ptr<ProjectTreeNode> _parent{};
    std::shared_ptr<iRASPAProject> _representedObject{};

    std::vector<std::shared_ptr<ProjectTreeNode>> _childNodes{};
    std::vector<std::shared_ptr<ProjectTreeNode>> _filteredAndSortedNodes{};

    bool _matchesFilter = true;
    bool _selected = true;
    bool _isEditable;
    bool _isDropEnabled;

    friend ProjectTreeController;

    friend QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<ProjectTreeNode>>& val);
    friend QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<ProjectTreeNode>>& val);

    friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<ProjectTreeNode> &);
    friend QDataStream &operator>>(QDataStream &, std::shared_ptr<ProjectTreeNode> &);
    friend QDataStream &operator<<=(QDataStream &, const std::shared_ptr<ProjectTreeNode> &);
    friend QDataStream &operator>>=(QDataStream &, std::shared_ptr<ProjectTreeNode> &);
};
