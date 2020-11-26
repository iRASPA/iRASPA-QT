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

#include <QUuid>
#include <QByteArray>
#include "project.h"
#include "projectstructure.h"
#include "projectgroup.h"
#include <foundationkit.h>



class iRASPAProject
{
public:
  iRASPAProject();
  iRASPAProject(std::shared_ptr<Project> project);
  iRASPAProject(std::shared_ptr<ProjectStructure> project);
  iRASPAProject(std::shared_ptr<ProjectGroup> project);

  enum class ProjectType: qint64 {none=0, generic=1, group=2, structure=3, VASP=4, RASPA=5, GULP=6, CP2K=7};
  enum class NodeType: qint64 {group=0, leaf=1};
  enum class StorageType: qint64 {local = 0, publicCloud = 1, privateCloud = 2, sharedCloud};
  enum class LazyStatus: qint64 { lazy = 0, loaded = 1, error = 2};

  void readData(ZipReader& reader);
  void saveData(ZipWriter& writer);
  void unwrapIfNeeded();
  bool isLeaf() {return _nodeType == NodeType::leaf;}
  bool isGroup() {return _nodeType == NodeType::group;}
  void setIsGroup(bool value) {_nodeType = value ? NodeType::group : NodeType::leaf;}
  inline const std::shared_ptr<Project> project() const {return _project;}
  std::shared_ptr<Project> project() {return _project;}
  ~iRASPAProject() {}
  QUndoStack& undoManager() {return _undoStack;}
private:
  qint64 _versionNumber{1};
  ProjectType _projectType;
  QString _fileNameUUID;
  std::shared_ptr<Project> _project;
  NodeType _nodeType;
  StorageType _storageType;
  LazyStatus _lazyStatus;
  QByteArray _data;

  QUndoStack _undoStack;

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<iRASPAProject> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<iRASPAProject> &);

  friend QDataStream &operator<<=(QDataStream &, const std::shared_ptr<iRASPAProject> &);
  friend QDataStream &operator>>=(QDataStream &, std::shared_ptr<iRASPAProject> &);
};


