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
#include "skatomcopy.h"

class SKBond: public std::enable_shared_from_this<SKBond>
{
public:
    enum class BoundaryType: qint64
    {
        internal = 0, external = 1
    };
    SKBond();
    SKBond(std::shared_ptr<SKAtomCopy> a, std::shared_ptr<SKAtomCopy> b, BoundaryType type=BoundaryType::internal);
    inline std::shared_ptr<SKAtomCopy> atom1() const {return this->_atom1.lock();}
    inline std::shared_ptr<SKAtomCopy> atom2() const {return this->_atom2.lock();}
    BoundaryType boundaryType() {return _boundaryType;}
    bool operator==(SKBond const& rhs) const;
    double bondLength();
    void setAtoms(std::shared_ptr<SKAtomCopy>, std::shared_ptr<SKAtomCopy>);
    int getTag1() {return _tag1;}
    int getTag2() {return _tag2;}
private:
    std::weak_ptr<SKAtomCopy> _atom1;
    std::weak_ptr<SKAtomCopy> _atom2;
    qint64 _tag1;
    qint64 _tag2;
    BoundaryType _boundaryType = BoundaryType::internal;

    friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<SKBond> &);
    friend QDataStream &operator>>(QDataStream &, std::shared_ptr<SKBond> &);
};

