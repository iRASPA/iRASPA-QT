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
#include <unordered_set>
#include "iraspastructure.h"
#include "iraspakitprotocols.h"

class Scene;

using FrameSelectionIndexSet = std::set<size_t>;

struct frame_index_compare_less
{
  bool operator() (const std::pair<std::shared_ptr<iRASPAStructure>, int>& lhs, const std::pair<std::shared_ptr<iRASPAStructure>, int>& rhs) const
  {
    return lhs.second < rhs.second;
  }
};

struct frame_index_compare_greater_than
{
  bool operator() (const std::pair<std::shared_ptr<iRASPAStructure>, int>& lhs, const std::pair<std::shared_ptr<iRASPAStructure>, int>& rhs) const
  {
    return lhs.second > rhs.second;
  }
};

using FrameSelectionNodesAndIndexSet = std::set<std::pair<std::shared_ptr<iRASPAStructure>, int>, frame_index_compare_less>;
using ReversedFrameSelectionNodesAndIndexSet = std::set<std::pair<std::shared_ptr<iRASPAStructure>, int>, frame_index_compare_greater_than>;

class Movie: public std::enable_shared_from_this<Movie>, public DisplayableProtocol
{
public:

  static char mimeType[];

  Movie();
  Movie(QString displayName, std::vector<std::shared_ptr<iRASPAStructure>> iraspaStructures);

  static std::shared_ptr<Movie> create(std::shared_ptr<iRASPAStructure> structure);
  virtual ~Movie() {}
  std::shared_ptr<Movie> shallowClone();

  void append(std::shared_ptr<iRASPAStructure> structure);
  bool insertChild(size_t row, std::shared_ptr<iRASPAStructure> child);
  const std::vector<std::shared_ptr<iRASPAStructure>> &frames() const {return _frames;}
  QString displayName() const override final;
  void setDisplayName(QString name) {_displayName = name;}
  bool isVisible() {return _isVisible;}
  void setVisibility(bool visibility);

  void clearSelection();

  std::vector<std::shared_ptr<iRASPAStructure>> selectedFrames();
  void addSelectedFrameIndex(size_t value);
  FrameSelectionIndexSet selectedFramesIndexSet();
  FrameSelectionNodesAndIndexSet selectedFramesNodesAndIndexSet();
  void setSelection(FrameSelectionIndexSet selection);
  void setSelection(FrameSelectionNodesAndIndexSet selection);

  std::vector<std::shared_ptr<iRASPAStructure>> selectedFramesiRASPAStructures() const;

  bool removeChildren(size_t position, size_t count);
  void setParent(std::weak_ptr<Scene> parent) {_parent = parent;}
  std::weak_ptr<Scene> parent() {return _parent;}
  iRASPAStructureType movieType() {return _movieType;}
  std::shared_ptr<iRASPAStructure> frameAtIndex(size_t index);
  void selectedFrameIndex(size_t index);
private:
  iRASPAStructureType _movieType;
  Movie(std::shared_ptr<iRASPAStructure> structure);
  qint64 _versionNumber{1};
  bool _isVisible = true;
  [[maybe_unused]] bool _isLoading = false;
  QString _displayName = QString("Movie");

  std::weak_ptr<Scene> _parent{};
  std::vector<std::shared_ptr<iRASPAStructure>> _frames{};
  std::set<size_t> _selectedFramesIndexSet;

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<Movie> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<Movie> &);
};

