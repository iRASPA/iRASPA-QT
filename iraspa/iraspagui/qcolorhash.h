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

#include <QColor>
#include <QDebug>
#include <QSet>

#include <unordered_set>

// hash function for QColor for use in QSet / QHash
QT_BEGIN_NAMESPACE
uint qHash(const QColor &c)
{
    return qHash(c.rgba());
}
QT_END_NAMESPACE

// hash function for QColor for std::unordered_set
namespace std {
template<> struct hash<QColor>
{
    using argument_type = QColor;
    using result_type = size_t;
    result_type operator()(const argument_type &c) const
    {
        return hash<QRgb>()(c.rgba());
    }
};
} // namespace std
