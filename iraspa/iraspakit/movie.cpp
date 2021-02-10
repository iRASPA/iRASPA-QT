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

#include "movie.h"
#include <algorithm>

char Movie::mimeType[] = "application/x-qt-iraspa-movie-mime";

// _movieType is to set the type of the movie based on the type of the first frame.
// In this way, an empty movie (after deleting all its frames) can create a frame of the correct type.

Movie::Movie(): _displayName("movie"), _frames{}, _movieType(iRASPAStructureType::crystal)
{

}

// private constructor, parent of structure needs to be set
Movie::Movie(std::shared_ptr<iRASPAStructure> structure): _movieType(structure->type()), _frames{structure}
{
}

// private constructor, parent of structure needs to be set
Movie::Movie(std::vector<std::shared_ptr<iRASPAStructure>> structures): _frames{structures}
{
}

std::shared_ptr<Movie> Movie::create(std::shared_ptr<iRASPAStructure> structure)
{
  // std::make_shared can not call private constructor
  std::shared_ptr<Movie> movie =  std::shared_ptr<Movie>( new Movie( std::forward<Movie>(structure)));
  movie->_movieType = structure->type();
  movie->addSelectedFrameIndex(0);
  structure->setParent(movie);
  return movie;
}

std::shared_ptr<Movie> Movie::create(QString displayName, std::vector<std::shared_ptr<iRASPAStructure>> iraspaStructures)
{
  std::shared_ptr<Movie> movie =  std::shared_ptr<Movie>( new Movie( std::forward<Movie>(iraspaStructures)));
  movie->setDisplayName(displayName);

  if(!movie->_frames.empty())
  {
    movie->_movieType = movie->_frames.front()->type();
    movie->addSelectedFrameIndex(0);
  }

  for(std::shared_ptr<iRASPAStructure> frame : movie->_frames)
  {
    frame->setParent(movie);
  }

  return movie;
}

void Movie::append(std::shared_ptr<iRASPAStructure> structure)
{
  _frames.push_back(structure);
  structure->setParent(this->shared_from_this());
}

bool Movie::insertChild(size_t row, std::shared_ptr<iRASPAStructure> child)
{
  if (row < 0 || row > _frames.size())
    return false;

  child->setParent(shared_from_this());
  _frames.insert(_frames.begin() + row, child);
  return true;
}

std::shared_ptr<Movie> Movie::shallowClone()
{
  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << this->shared_from_this();

  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  std::shared_ptr<Movie> movie = std::make_shared<Movie>();
  streamRead >> movie;

  return movie;
}

void Movie::setVisibility(bool visibility)
{
  _isVisible = visibility;
  for(std::shared_ptr<iRASPAStructure> frame : _frames)
  {
    frame->structure()->setVisibility(visibility);
  }
}


std::vector<std::shared_ptr<iRASPAStructure>> Movie::selectedFramesiRASPAStructures() const
{
  std::vector<std::shared_ptr<iRASPAStructure>> structures = std::vector<std::shared_ptr<iRASPAStructure>>();

  for(size_t index: _selectedFramesIndexSet)
  {
    structures.push_back(_frames[index]);
  }
  return structures;
}

QString Movie::displayName() const
{
 return _displayName;
}

bool Movie::removeChildren(size_t position, size_t count)
{
  if (position < 0 || position + count > _frames.size())
    return false;
  std::vector<std::shared_ptr<iRASPAStructure>>::iterator start = _frames.begin() + position;
  std::vector<std::shared_ptr<iRASPAStructure>>::iterator end = _frames.begin() + position + count;
  _frames.erase(start, end);

  return true;
}

std::shared_ptr<iRASPAStructure> Movie::frameAtIndex(size_t index)
{
  if(_frames.empty())
  {
    return nullptr;
  }

  return _frames[std::min(index, _frames.size()-1)];
}

void Movie::clearSelection()
{
  _selectedFramesIndexSet.clear();
}


std::vector<std::shared_ptr<iRASPAStructure>> Movie::selectedFrames()
{
  std::vector<std::shared_ptr<iRASPAStructure>> frames{};
  for(size_t index : _selectedFramesIndexSet)
  {
    frames.push_back(_frames[index]);
  }
  return frames;
}

void Movie::addSelectedFrameIndex(size_t value)
{
  _selectedFramesIndexSet.insert(value);
}

FrameSelectionIndexSet Movie::selectedFramesIndexSet()
{
  return _selectedFramesIndexSet;
}

FrameSelectionNodesAndIndexSet Movie::selectedFramesNodesAndIndexSet()
{
  FrameSelectionNodesAndIndexSet selection{};

  for(size_t index : _selectedFramesIndexSet)
  {
    selection.insert(std::pair(_frames[index], index));
  }
  return selection;
}

void Movie::setSelection(FrameSelectionIndexSet selection)
{
  _selectedFramesIndexSet.clear();

  size_t numberOfFrames = _frames.size();
  std::copy_if (selection.begin(), selection.end(), std::inserter(_selectedFramesIndexSet, _selectedFramesIndexSet.begin()), [numberOfFrames](size_t i){return i < numberOfFrames;} );
}

void Movie::setSelection(FrameSelectionNodesAndIndexSet selection)
{
  _selectedFramesIndexSet.clear();

  FrameSelectionIndexSet s{};
  std::transform(selection.begin(), selection.end(), std::inserter(s, s.begin()), [](std::pair<std::shared_ptr<iRASPAStructure>, size_t> s) -> size_t {return s.second;});
  setSelection(s);
}

void Movie::selectedFrameIndex(size_t index)
{
  _selectedFramesIndexSet.clear();
  if(!_frames.empty())
  {
    _selectedFramesIndexSet = {std::min(_frames.size()-1, index)};
  }
}

QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<iRASPAStructure>>& val)
{
  stream << static_cast<int32_t>(val.size());
  for(const std::shared_ptr<iRASPAStructure>& singleVal : val)
    stream << singleVal;
  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<iRASPAStructure>>& val)
{
  int32_t vecSize;
  val.clear();
  stream >> vecSize;
  val.reserve(vecSize);

  while(vecSize--)
  {
    std::shared_ptr<iRASPAStructure> tempVal = std::make_shared<iRASPAStructure>();
    stream >> tempVal;
    val.push_back(tempVal);
  }
  return stream;
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<Movie> &movie)
{
  stream << movie->_versionNumber;
  stream << movie->_displayName;
  stream << movie->_frames;

  return stream;
}

QDebug operator<< (QDebug dbg, const std::shared_ptr<Movie> & movie)
{
  QDebugStateSaver stateSaver(dbg);
  dbg.noquote() << "Movie displayname: " + movie->displayName() + "\n";
  dbg.noquote() << "Movie number of frames: " <<  QString::number(movie->frames().size()) + "\n";
  for(std::shared_ptr<iRASPAStructure> frame: movie->frames())
  {
    dbg.noquote() << "\tFrame displayName: " <<  frame->structure()->displayName() << " nr atoms: "
                  << QString::number(frame->structure()->atomsTreeController()->flattenedLeafNodes().size())
                  << "Expired parent: " << frame->parent().expired() <<
                     + "\n";
  }
  return dbg;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<Movie> &movie)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > movie->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "Movie");
  }
  stream >> movie->_displayName;
  stream >> movie->_frames;

  for(std::shared_ptr<iRASPAStructure> frame : movie->_frames)
  {
    frame->setParent(movie);
  }

  if(!movie->_frames.empty())
  {
    movie->_movieType = movie->_frames.front()->type();
  }

  return stream;
}
