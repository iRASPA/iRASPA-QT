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
#include <ostream>
#include <QDataStream>
#include <QDebug>

#include "int3.h"
#include "bool3.h"

union double3
{
  double v[3];
  struct {double x, y, z;};

  double3(double x=0,double y=0, double z=0):x(x),y(y),z(z) {}
  double3(int3 a):x(double(a.x)),y(double(a.y)),z(double(a.z)) {}
  inline double & operator [] (int i) { return v[i]; }
  inline const double & operator [] (int i) const { return v[i]; }
  double3 operator + (const double3& right) const;
  double3 operator - (const double3& right) const;
  double length() {return sqrt(x*x+y*y+z*z);}
  double length() const {return sqrt(x*x+y*y+z*z);}
  double3 normalise();
  double3 fract();

  static double3 randomVectorOnUnitSphere();

  static double3 fract(double3 const &v);
  static double dot(const double3 &v1, const double3 &v2);
  static double3 max(const double3 &v1, const double3 &v2);
  static double3 min(const double3 &v1, const double3 &v2);
  static double3 cross(const double3 &v1, const double3 &v2);
  static double3 normalize(const double3 &v);

  static inline double3 flip(double3 v, bool3 flip, double3 boundary) {return double3(flip.x ? boundary.x - v.x : v.x,
                                                                                      flip.y ? boundary.y - v.y : v.y,
                                                                                      flip.z ? boundary.z - v.z : v.z);}

  friend double3 operator*(const double3 &v1, const double3 &v2);
  friend double3 operator/(const double3 &v1, const double3 &v2);
  friend double3 operator*(const double3 &v, double value);
  friend double3 operator*(double value, const double3 &v);
  friend double3 operator/(const double3 &v, double value);
  friend double3 operator/(double value, const double3 &v);
  friend std::ostream& operator<<(std::ostream& out, const double3& vec) ;

  friend QDataStream &operator<<(QDataStream &, const double3 &);
  friend QDataStream &operator>>(QDataStream &, double3 &);

  friend QDebug operator<<(QDebug debug, const double3 &m);

  struct KeyHash{
      size_t operator()(const double3 &k) const{
      size_t h1 = std::hash<double>()(k.x);
      size_t h2 = std::hash<double>()(k.y);
      size_t h3 = std::hash<double>()(k.z);
      return (h1 ^ (h2 << 1)) ^ h3;
      }
  };

  struct KeyEqual{
    bool operator()( const double3& lhs, const double3& rhs ) const{
      return (std::fabs(lhs.x - rhs.x) < 1e-3) && std::fabs(lhs.y - rhs.y) < 1e-3 && std::fabs(lhs.z == rhs.z) < 1e-3;
    }
  };
};

