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

#include <QDataStream>
#include <iostream>
#include <memory>
#include <tuple>
#include "skasymmetricatom.h"


class SKAsymmetricBond: public std::enable_shared_from_this<SKAsymmetricBond>
{
public:
  enum class SKBondType: qint64
  {
    singleBond = 0, doubleBond = 1, partialDoubleBond = 2, tripleBond = 3
  };

  struct KeyHash {
   std::size_t operator()(const std::shared_ptr<SKAsymmetricBond>& k) const
   {
       size_t first = std::hash<std::shared_ptr<SKAsymmetricAtom>>{}(k->atom1());
       size_t second = std::hash<std::shared_ptr<SKAsymmetricAtom>>{}(k->atom2());
       size_t lhs = std::min(first,second);
       size_t rhs = std::max(first,second);
       return lhs ^ rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
   }
  };

  struct KeyEqual {
   bool operator()(const std::shared_ptr<SKAsymmetricBond>& lhs, const std::shared_ptr<SKAsymmetricBond>& rhs) const
   {
       return lhs->atom1().get() == rhs->atom1().get() && lhs->atom2().get() == rhs->atom2().get();
   }
  };
  SKAsymmetricBond() {}
  SKAsymmetricBond(std::shared_ptr<SKAsymmetricAtom> a, std::shared_ptr<SKAsymmetricAtom> b);
  bool operator==(SKAsymmetricBond const& rhs) const;
  inline std::shared_ptr<SKAsymmetricAtom> atom1() const {return this->_atom1.lock();}
  inline std::shared_ptr<SKAsymmetricAtom> atom2() const {return this->_atom2.lock();}
  std::vector<std::shared_ptr<SKBond>>& copies()  {return _copies;}
  bool isVisible() {return _isVisible;}
  void setIsVisible(bool v) {_isVisible = v;}
  SKBondType getBondType() {return _bondType;}
  void setBondType(SKBondType type) {_bondType = type;}
  int getTag1() {return _tag1;}
  int getTag2() {return _tag2;}
  void setAtom1(std::shared_ptr<SKAsymmetricAtom> a) {_atom1 = a;}
  void setAtom2(std::shared_ptr<SKAsymmetricAtom> a) {_atom2 = a;}
  void setAsymmetricIndex(int index) {_asymmetricIndex = index;}
  int asymmetricIndex() {return _asymmetricIndex;}
private:

  std::weak_ptr<SKAsymmetricAtom> _atom1;
  std::weak_ptr<SKAsymmetricAtom> _atom2;
  qint64 _tag1;
  qint64 _tag2;
  std::vector<std::shared_ptr<SKBond>> _copies;
  bool _isVisible;
  SKBondType _bondType;
  int _asymmetricIndex = 0;
  [[maybe_unused]] int _bondOrder = 0;

  friend QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<SKAsymmetricBond>>& val);
  friend QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<SKAsymmetricBond>>& val);

  friend QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<SKBond>>& val);
  friend QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<SKBond>>& val);

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<SKAsymmetricBond> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<SKAsymmetricBond> &);
};

