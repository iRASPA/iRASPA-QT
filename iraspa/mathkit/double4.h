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

union double4
{
  double v[4];
  struct {double x, y, z, w;};

  double4(double x=0,double y=0, double z=0, double w=0):x(x),y(y),z(z),w(w) {}
  //explicit double4(double v1=0.0, double v2=0.0, double v3=0.0, double v4=0.0);
  inline double & operator [] (int i) { return v[i]; }
  inline const double & operator [] (int i) const { return v[i]; }

  void normalise();
  static double dot(const double4 &v1, const double4 &v2);
  static double4 cross(const double4 &v1, const double4 &v2);
  static double4 cross(const double4 &v1, const double4 &v2, const double4 &v3);

  friend double4 operator*(const double4 &v1, const double4 &v2);
  friend double4 operator*(const double4 &v, double value);
  friend double4 operator*(double value, const double4 &v);

  friend QDataStream &operator<<(QDataStream &, const double4 &);
  friend QDataStream &operator>>(QDataStream &, double4 &);
};
