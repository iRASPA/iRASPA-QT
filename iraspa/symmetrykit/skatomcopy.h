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
#include <vector>
#include <unordered_set>
#include <mathkit.h>
#include <type_traits>
#include <foundationkit.h>

class SKAsymmetricAtom;

class SKAtomCopy
{
public:
    SKAtomCopy(): _position(), _tag(0), _type(AtomCopyType::copy), _parent() {}
    SKAtomCopy(const SKAtomCopy &atomCopy);
    SKAtomCopy(std::shared_ptr<SKAsymmetricAtom> asymmetricParentAtom, double3 position): _position(position), _tag(0), _type(AtomCopyType::copy), _parent(asymmetricParentAtom) {}
    enum class AtomCopyType: qint64
    {
      copy = 2, duplicate = 3
    };
    const std::shared_ptr<SKAsymmetricAtom> parent() const {return this->_parent.lock();}
    double3 position() const {return _position;}
    void setPosition(double3 p) {_position = p;}
    AtomCopyType type() {return _type;}
    void setType(AtomCopyType type) {_type = type;}
    qint64 tag() {return _tag;}
    void setTag(qint64 tag) {_tag = tag;}
    qint64 asymmetricIndex() {return _asymmetricIndex;}
    void setAsymmetricIndex(qint64 value) {_asymmetricIndex = value;}
private:
    qint64 _versionNumber{1};
    struct Hash
    {
      template <typename T> std::size_t operator() (T* const &p) const
      {
        return std::hash<T>()(*p);
      }
    };
    struct Compare
    {
      template <typename T> size_t operator() (T* const &a, T* const &b) const
      {
        return *a == *b;
      }
    };
    double3 _position;
    qint64 _tag;
    AtomCopyType _type;
    std::weak_ptr<SKAsymmetricAtom> _parent;
    qint64 _asymmetricIndex;

    friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<SKAtomCopy> &);
    friend QDataStream &operator>>(QDataStream &, std::shared_ptr<SKAtomCopy> &);

    friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<SKAsymmetricAtom> &);
    friend QDataStream &operator>>(QDataStream &, std::shared_ptr<SKAsymmetricAtom> &);
};
