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

#include <iostream>
#include <QDataStream>
#include <QString>
#include <vector>
#include <map>
#include <memory>
#include <unordered_set>
#include <type_traits>

#include <exception>

class InvalidArchiveVersionException : public std::exception
{
public:
  InvalidArchiveVersionException(const char* file_, int line_, const char* func_, const char* info_ = "") :
      _file (file_),
      _line (line_),
      _func (func_),
      _info (info_)
  {

  }
  QString message() {return QString("Invalid archive version (upgrade to latest iRASPA version)");}
  const char* get_file() const { return _file; }
  int get_line() const { return _line; }
  const char* get_func() const { return _func; }
  const char* get_info() const { return _info; }
private:
  const char* _file;
  int _line;
  const char* _func;
  const char* _info;
};

class InconsistentArchiveException : public std::exception
{
public:
  InconsistentArchiveException(const char* file_, int line_, const char* func_, const char* info_ = "") :
      _file (file_),
      _line (line_),
      _func (func_),
      _info (info_)
  {

  }
  QString message() {return QString("Archive is inconsistent (internal bug)");}
  const char* get_file() const { return _file; }
  int get_line() const { return _line; }
  const char* get_func() const { return _func; }
  const char* get_info() const { return _info; }
private:
  const char* _file;
  int _line;
  const char* _func;
  const char* _info;
};


//a function that can serialize any enum into QDataStream
//it stores the enum in a qint64
template<typename Enum,
         typename = typename std::enable_if<std::is_enum<Enum>::value>::type>
QDataStream& operator<<(QDataStream& stream, const Enum& e) {
    stream << static_cast<qint64>(e);
    return stream;
}

//a function that can deserialize any enum from QDataStream
//it reads the enum as if it was stored in qint64
template<typename Enum,
         typename = typename std::enable_if<std::is_enum<Enum>::value>::type>
QDataStream& operator>>(QDataStream& stream, Enum& e) {
    qint64 v;
    stream >> v;
    e = static_cast<Enum>(v);
    return stream;
}

/*
template<class T> QDataStream &operator<<(QDataStream& stream, const std::vector<T>& val)
{
  stream << static_cast<int32_t>(val.size());
  for(const T& singleVal : val)
    stream << singleVal;
  return stream;
}

template<class T> QDataStream &operator>>(QDataStream& stream, std::vector<T>& val)
{
  int32_t vecSize;
  val.clear();
  stream >> vecSize;
  val.reserve(vecSize);
  T tempVal;
  while(vecSize--)
  {
    stream >> tempVal;
    val.push_back(tempVal);
  }
  return stream;
}*/

template<class T> QDataStream &operator<<(QDataStream& stream, const std::unordered_set<T>& val)
{
  stream << static_cast<int32_t>(val.size());
  for(const T& singleVal : val)
    stream << singleVal;
  return stream;
}

template<class T> QDataStream &operator>>(QDataStream& stream, std::unordered_set<T>& val)
{
  int32_t vecSize;
  val.clear();
  stream >> vecSize;
  val.reserve(vecSize);
  T tempVal;
  while(vecSize--)
  {
    stream >> tempVal;
    val.insert(tempVal);
  }
  return stream;
}

/*
template<class T> QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<T>>& val)
{
  stream << static_cast<int32_t>(val.size());
  for(const std::shared_ptr<T>& singleVal : val)
    stream << singleVal;
  return stream;
}

template<class T> QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<T>>& val)
{
  int32_t vecSize;
  val.clear();
  stream >> vecSize;
  val.reserve(vecSize);

  while(vecSize--)
  {
    std::shared_ptr<T> tempVal = std::make_shared<T>();
    stream >> tempVal;
    val.push_back(tempVal);
  }
  return stream;
}*/

template<class Key, class T> QDataStream &operator<<(QDataStream& stream, const std::map<Key, T>& table)
{
  stream << static_cast<int32_t>(table.size());
  for(auto const& [key, val] : table)
  {
    stream << key;
    stream << val;
  }
  return stream;
}

template<class Key, class T> QDataStream &operator>>(QDataStream& stream, std::map<Key, T>& table)
{
  int32_t vecSize;
  table.clear();
  stream >> vecSize;
  Key key;
  T value;

  while(vecSize--)
  {
    stream >> key;
    stream >> value;
    table[key] = value;
  }
  return stream;
}
