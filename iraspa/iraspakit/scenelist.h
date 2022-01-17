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
#include <QSharedPointer>
#include <vector>
#include <tuple>
#include <memory>
#include <ostream>
#include "scene.h"
#include "iraspakitprotocols.h"

using SceneListSelection = std::tuple<std::shared_ptr<Scene>,
                                      std::set<std::shared_ptr<Scene>>,
                                      std::map<std::shared_ptr<Scene>, std::shared_ptr<Movie>>,
                                      std::map<std::shared_ptr<Scene>, std::set<std::shared_ptr<Movie>>> >;


class SceneList: public std::enable_shared_from_this<SceneList>, public DisplayableProtocol
{
public:
  SceneList();
  void appendScene(std::shared_ptr<Scene> scene);
  const std::vector<std::shared_ptr<Scene>> &scenes() const {return _scenes;}

  QString displayName() const override final {return _displayName;}
  void setDisplayName(QString name) override final {_displayName = name;}
  std::optional<int> findChildIndex(std::shared_ptr<Scene> scene);
  bool removeChild(size_t row);
  bool removeChildren(size_t position, size_t count);
  bool insertChild(size_t row, std::shared_ptr<Scene> child);

  std::vector<std::vector<std::shared_ptr<iRASPAObject>>> invalidatediRASPAStructures() const;
  std::vector<std::vector<std::shared_ptr<iRASPAObject>>> allIRASPAStructures() const;
  std::vector<std::shared_ptr<iRASPAObject>> flattenedAllIRASPAStructures();
  std::vector<std::shared_ptr<iRASPAObject>> selectedMoviesIRASPAStructures();
  std::vector<std::vector<std::shared_ptr<iRASPAObject>>> selectediRASPAStructures() const;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> selectediRASPARenderStructures() const;

  std::shared_ptr<Scene> selectedScene();
  std::set<std::shared_ptr<Scene>> &selectedScenes() {return _selectedScenes;}
  void setSelectedScene(std::shared_ptr<Scene> scene);


  std::optional<int> selectedSceneIndex();
  SceneListSelection allSelection();
  void setSelection(SceneListSelection selection);

  void setSelectedFrameIndex(size_t frameIndex);
  size_t selectedFrameIndex() {return _selectedFrameIndex;}
private:
  qint64 _versionNumber{1};
  QString _displayName = QString("");
  std::vector<std::shared_ptr<Scene>> _scenes{};

  std::shared_ptr<Scene> _selectedScene;
  std::set<std::shared_ptr<Scene>> _selectedScenes;
  size_t _selectedFrameIndex = 0;

  friend QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<Scene>>& val);
  friend QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<Scene>>& val);

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<SceneList> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<SceneList> &);
};
