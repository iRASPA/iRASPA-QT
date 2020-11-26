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

#include "indexpath.h"
#include <algorithm>

IndexPath::IndexPath()
{
  _path.reserve(10);
}

IndexPath::IndexPath(const size_t index)
{
  _path.reserve(10);
  _path.push_back(index);
}

IndexPath IndexPath::indexPath(QModelIndex index)
{
  int row = index.row();
  QModelIndex parent = index.parent();

  if (parent.isValid())
  {
    IndexPath indexpath = indexPath(parent);
    return indexpath.appending(row);
  }

  return IndexPath(row);
}

size_t& IndexPath::operator[] (const size_t index) // for non-const objects: can be used for assignment
{
  return _path[index];
}

const size_t& IndexPath::operator[] (const size_t index) const // for const objects: can only be used for access
{
  return _path[index];
}

void IndexPath::increaseValueAtLastIndex()
{
  if (!_path.empty())
  {
    _path.back() += 1;
  }
}

void IndexPath::decreaseValueAtLastIndex()
{
  if (!_path.empty())
  {
    _path.back() -= 1;
  }
}

size_t IndexPath::size()
{
  return _path.size();
}

IndexPath IndexPath::appending(size_t index)
{
  IndexPath indexpath{};
  indexpath._path.insert(indexpath._path.end(), _path.begin(), _path.end());
  indexpath._path.push_back(index);
  return indexpath;
}

IndexPath IndexPath::removingLastIndex() const
{
  IndexPath indexPath{};
  if (_path.size() <= 1) {return indexPath;}
  indexPath._path = std::vector<size_t>(_path.begin(),std::prev(_path.end(), 1));
  return indexPath;
}

const IndexPath IndexPath::operator+(const IndexPath& rhs)
{
  IndexPath indexpath{};
  indexpath._path.insert(indexpath._path.end(), _path.begin(), _path.end());
  indexpath._path.insert(indexpath._path.end(), rhs._path.begin(), rhs._path.end());
  return indexpath;
}

bool IndexPath::operator<( const IndexPath& otherObject ) const
{
  size_t l1 = _path.size();
  size_t l2 = otherObject._path.size();
  for(size_t pos=0;pos<std::min(l1,l2); pos++)
  {
    size_t i1 = _path[pos];
    size_t i2 = otherObject._path[pos];
    if (i1 < i2)
    {
      return true;
    }
    else if (i1 > i2)
    {
      return false;
    }
  }
  if (l1 < l2)
  {
    return true;
  }
  else if (l1 > l2)
  {
    return false;
  }
  return false;
}

bool IndexPath::operator>( const IndexPath& otherObject ) const
{
  size_t l1 = _path.size();
  size_t l2 = otherObject._path.size();
  for(size_t pos=0;pos<std::min(l1,l2); pos++)
  {
    size_t i1 = _path[pos];
    size_t i2 = otherObject._path[pos];
    if (i1 < i2)
    {
      return false;
    }
    else if (i1 > i2)
    {
      return true;
    }
  }
  if (l1 < l2)
  {
    return false;
  }
  else if (l1 > l2)
  {
    return true;
  }
  return false;
}

bool IndexPath::operator==( const IndexPath& otherObject ) const
{
  size_t l1 = _path.size();
  size_t l2 = otherObject._path.size();
  for(size_t pos=0;pos<std::min(l1,l2); pos++)
  {
    size_t i1 = _path[pos];
    size_t i2 = otherObject._path[pos];
    if (i1 < i2)
    {
      return false;
    }
    else if (i1 > i2)
    {
      return false;
    }
  }
  if (l1 < l2)
  {
    return false;
  }
  else if (l1 > l2)
  {
    return false;
  }
  return false;
}

QDebug operator<<(QDebug debug, const IndexPath &c)
{
  QDebugStateSaver saver(debug);
  qDebug() << '(' << c._path.size() << ") , ";
  for(size_t element : c._path)
  {
    qDebug() << element;
  }
  return debug;
}
