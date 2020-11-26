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

#include <QDebug>
#include <QModelIndex>
#include <vector>


class IndexPath
{
public:
    IndexPath();
    IndexPath(const size_t index);
    static IndexPath indexPath(QModelIndex index);
    size_t& operator[] (const size_t index);
    const size_t& operator[] (const size_t index) const;
    inline size_t lastIndex() const {if (!_path.empty()) return _path.back(); return 0;}
    const IndexPath operator+(const IndexPath& rhs);
    void increaseValueAtLastIndex();
    void decreaseValueAtLastIndex();
    size_t size();
    bool empty() const {return _path.empty();}
    IndexPath appending(size_t index);
    IndexPath removingLastIndex() const;

    bool operator<( const IndexPath& otherObject ) const;
    bool operator>( const IndexPath& otherObject ) const;
    bool operator==( const IndexPath& otherObject ) const;
private:
    std::vector<size_t> _path;
    friend QDebug operator<<(QDebug debug, const IndexPath &c);
};



