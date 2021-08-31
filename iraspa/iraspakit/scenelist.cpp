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

#include "scenelist.h"
#include <iostream>
#include <optional>
#include <algorithm>

SceneList::SceneList()
{

}

QString SceneList::displayName() const
{
 return _displayName;
}

void SceneList::appendScene(std::shared_ptr<Scene> scene)
{
  _scenes.push_back(scene);
  scene->setParent(this->shared_from_this());
}

std::shared_ptr<Scene> SceneList::selectedScene()
{
  if(_selectedScene)
    return _selectedScene;
  return nullptr;
}

std::optional<int> SceneList::findChildIndex(std::shared_ptr<Scene> scene)
{
  std::vector<std::shared_ptr<Scene>>::const_iterator itr = std::find(_scenes.begin(), _scenes.end(), scene);
  if (itr != _scenes.end())
  {
    int row = itr-_scenes.begin();
    return row;
  }

  return std::nullopt;
}


bool SceneList::removeChild(size_t row)
{
  if (row < 0 || row >= _scenes.size())
     return false;

  _scenes.erase(_scenes.begin() + row);
  return true;
}

bool SceneList::removeChildren(size_t position, size_t count)
{
  if (position < 0 || position + count > _scenes.size())
    return false;
  std::vector<std::shared_ptr<Scene>>::iterator start = _scenes.begin() + position;
  std::vector<std::shared_ptr<Scene>>::iterator end = _scenes.begin() + position + count;
  _scenes.erase(start, end);

  return true;
}

bool SceneList::insertChild(size_t row, std::shared_ptr<Scene> child)
{
  if (row < 0 || row > _scenes.size())
    return false;

  _scenes.insert(_scenes.begin() + row, child);
  child->setParent(this->shared_from_this());
  return true;
}


void SceneList::setSelectedFrameIndex(size_t frameIndex)
{
  _selectedFrameIndex = frameIndex;
  for(const std::shared_ptr<Scene> &scene : _scenes)
  {
    for(std::shared_ptr<Movie> movie : scene->movies())
    {
      movie->selectedFrameIndex(frameIndex);
    }
  }
}

void SceneList::setSelectedScene(std::shared_ptr<Scene> scene)
{
  _selectedScene = scene;
  if(_selectedScene)
  {
    _selectedScenes.insert(_selectedScene);
  }
}

std::optional<int> SceneList::selectedSceneIndex()
{
  std::vector<std::shared_ptr<Scene>>::const_iterator itr = std::find(_scenes.begin(), _scenes.end(), selectedScene());
  if (itr != _scenes.end())
  {
    int row = itr-_scenes.begin();
    return row;
  }
  return std::nullopt;
}

std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> SceneList::allIRASPAStructures() const
{
  std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> sceneStructures = std::vector<std::vector<std::shared_ptr<iRASPAStructure>>>();

  for(const std::shared_ptr<Scene> &scene : _scenes)
  {
    std::vector<std::shared_ptr<iRASPAStructure>> structures = std::vector<std::shared_ptr<iRASPAStructure>>();
    for(const std::shared_ptr<Movie> &movie: scene->movies())
    {
      for(const std::shared_ptr<iRASPAStructure> &frame: movie->frames())
      {
        structures.push_back(frame);
      }
    }
    sceneStructures.push_back(structures);
  }
  return sceneStructures;
}


std::vector<std::shared_ptr<iRASPAStructure>> SceneList::flattenedAllIRASPAStructures()
{
  std::vector<std::shared_ptr<iRASPAStructure>> structures{};
  for(const std::shared_ptr<Scene> &scene: scenes())
  {
    for(const std::shared_ptr<Movie> &movie: scene->movies())
    {
      for(const std::shared_ptr<iRASPAStructure> &frame: movie->frames())
      {
        structures.push_back(frame);
      }
    }
  }
  return structures;
}


std::vector<std::shared_ptr<iRASPAStructure>> SceneList::selectedMoviesIRASPAStructures()
{
  std::vector<std::shared_ptr<iRASPAStructure>> structures{};
  for(std::shared_ptr<Scene> scene: scenes())
  {
    for(const std::shared_ptr<Movie> &movie: scene->selectedMovies())
    {
      for(const std::shared_ptr<iRASPAStructure> &frame: movie->frames())
      {
        structures.push_back(frame);
      }
    }
  }
  return structures;
}

// per scene, if a structure is selected than the corresponding structures of the other movies must be included too
std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> SceneList::invalidatediRASPAStructures() const
{
  std::vector<std::shared_ptr<iRASPAStructure>> invalidatedStructures{};

  std::vector<FrameSelectionIndexSet> sceneSelectedIndexes{};
  for(const std::shared_ptr<Scene> &scene : _scenes)
  {
    FrameSelectionIndexSet selectedIndexes{};
    for(std::shared_ptr<Movie> movie: scene->movies())
    {
      for(size_t index : movie->selectedFramesIndexSet())
      {
        selectedIndexes.insert(index);
      }
    }
    sceneSelectedIndexes.push_back(selectedIndexes);
  }

  std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> sceneStructures{};
  int sceneIndex=0;
  for(const std::shared_ptr<Scene> &scene : _scenes)
  {
    FrameSelectionIndexSet selectedIndexes = sceneSelectedIndexes[sceneIndex];
    std::vector<std::shared_ptr<iRASPAStructure>> structures = std::vector<std::shared_ptr<iRASPAStructure>>();
    for(std::shared_ptr<Movie> movie: scene->movies())
    {
      for(size_t index : selectedIndexes)
      {
        if(index < movie->frames().size())
        {
          structures.push_back(movie->frameAtIndex(index));
        }
      }
    }
    sceneStructures.push_back(structures);
    sceneIndex += 1;
  }
  return sceneStructures;
}

std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> SceneList::selectediRASPARenderStructures() const
{
  std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> sceneStructures{};

  for(const std::shared_ptr<Scene> &scene : _scenes)
  {
    std::vector<std::shared_ptr<iRASPAStructure>> structures = std::vector<std::shared_ptr<iRASPAStructure>>();
    for(std::shared_ptr<Movie> movie: scene->movies())
    {
      for(const std::shared_ptr<iRASPAStructure> &selectedFrame: movie->selectedFrames())
      {
        structures.push_back(selectedFrame);
      }
    }
    sceneStructures.push_back(structures);
  }
  return sceneStructures;
}

SceneListSelection SceneList::allSelection()
{
  std::map<std::shared_ptr<Scene>, std::shared_ptr<Movie>> s{};

  for(std::shared_ptr<Scene> scene : scenes())
  {
    s.insert({scene, scene->selectedMovie()});
  }

  std::map<std::shared_ptr<Scene>, std::set<std::shared_ptr<Movie>>> t{};

  for(std::shared_ptr<Scene> scene : scenes())
  {
    t.insert({scene, scene->selectedMovies()});
  }

  return SceneListSelection(_selectedScene, _selectedScenes, s, t);
}

void SceneList::setSelection(SceneListSelection selection)
{
  _selectedScene = std::get<0>(selection);
  _selectedScenes = std::get<1>(selection);

  for(auto const& [selectedScene, selectedMovie] : std::get<2>(selection))
  {
    if(selectedScene)
    {
       selectedScene->setSelectedMovie(selectedMovie);
    }
  }

  for(auto const& [selectedScene, selectedMovies] : std::get<3>(selection))
  {
    if(selectedScene)
    {
      selectedScene->setSelectedMovies(selectedMovies);
    }
  }
}

QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<Scene>>& val)
{
  stream << static_cast<int32_t>(val.size());
  for(const std::shared_ptr<Scene>& singleVal : val)
    stream << singleVal;
  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<Scene>>& val)
{
  int32_t vecSize;
  val.clear();
  stream >> vecSize;
  val.reserve(vecSize);

  while(vecSize--)
  {
    std::shared_ptr<Scene> tempVal = std::make_shared<Scene>();
    stream >> tempVal;
    val.push_back(tempVal);
  }
  return stream;
}


QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<SceneList> &sceneList)
{
  stream << sceneList->_versionNumber;
  stream << sceneList->_displayName;
  stream << sceneList->_scenes;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<SceneList> &sceneList)
{
  qint64 versionNumber;
  stream >> versionNumber;

  if(versionNumber > sceneList->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "SceneList");
  }
  stream >> sceneList->_displayName;
  stream >> sceneList->_scenes;

  for(std::shared_ptr<Scene> scene : sceneList->_scenes)
  {
    scene->setParent(sceneList);
  }

  return stream;
}
