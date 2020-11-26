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

#include <cmath>
#include <QDataStream>

union double2
{
  double v[2];
  struct {double x, y;};

  inline double2(double x=0,double y=0):x(x),y(y) {};
  inline double & operator [] (int i) { return v[i]; }
  inline const double & operator [] (int i) const { return v[i]; }


  inline double2 operator + (const double2& rhs) const
  {
    return double2(x+rhs.x,y+rhs.y);
  }

  inline double2 operator - (const double2& rhs) const
  {
    return double2(x-rhs.x,y-rhs.y);
  }

  inline double2 operator * (const double2& rhs) const
  {
    return double2(x*rhs.x, y*rhs.y);
  }

  inline double2& operator += (const double2& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  inline double2& operator -= (const double2& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  inline double DotProduct(const double2 &v1, const double2 &v2)
  {
    return v1.x*v2.x+v1.y*v2.y;
  }

  friend QDataStream &operator<<(QDataStream &, const double2 &);
  friend QDataStream &operator>>(QDataStream &, double2 &);
};
