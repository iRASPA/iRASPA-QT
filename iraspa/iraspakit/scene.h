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
#include <QUrl>
#include <QSharedPointer>
#include <vector>
#include <tuple>
#include <unordered_set>
#include <optional>
#include <ostream>
#include <sstream>
#include <fstream>
#include <simulationkit.h>
#include "movie.h"
#include "iraspakitprotocols.h"

class SceneList;

class Scene: public std::enable_shared_from_this<Scene>, public DisplayableProtocol
{
public:
  Scene();
  Scene(QString displayName);
  static std::shared_ptr<Scene> create(std::shared_ptr<Movie> movie);
  Scene(const QUrl url, const SKColorSets& colorSets, ForceFieldSets &forcefieldSets, LogReporting *log, bool asSeparateProject, bool onlyAsymmetricUnit, bool asMolecule);
  const std::vector<std::shared_ptr<Movie>> &movies() const {return _movies;}
  std::optional<int> findChildIndex(std::shared_ptr<Movie> movie);
  bool removeChild(size_t row);
  bool removeChildren(size_t position, size_t count);
  bool insertChild(size_t row, std::shared_ptr<Movie> child);
  void setSelectedMovie(std::shared_ptr<Movie> movie);
  void setSelectedMovies(std::set<std::shared_ptr<Movie>> movies);
  //void setSelectedFrameIndices(int frameIndex);
  std::set<std::shared_ptr<Movie>>& selectedMovies() {return _selectedMovies;}
  std::shared_ptr<Movie> selectedMovie();
  QString displayName() const override final;
  void setDisplayName(QString displayName) {_displayName = displayName;}
  std::optional<int> selectMovieIndex();
  void setParent(std::weak_ptr<SceneList> parent) {_parent = parent;}
  std::weak_ptr<SceneList> parent() {return _parent;}
private:
  Scene(std::shared_ptr<Movie> movie);
  qint64 _versionNumber{1};
  QString _displayName = QString("Scene");
  std::weak_ptr<SceneList> _parent{};
  std::vector<std::shared_ptr<Movie>> _movies{};
  std::shared_ptr<Movie> _selectedMovie{nullptr};
  std::set<std::shared_ptr<Movie>> _selectedMovies;

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<Scene> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<Scene> &);
};
