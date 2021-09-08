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

  inline double & operator [] (int i) { return v[i]; }
  inline const double & operator [] (int i) const { return v[i]; }

  void normalise();
  inline static double dot(const double4 &v1, const double4 &v2) { return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z+v1.w*v2.w;}

  // http://www.gamedev.net/topic/456301-cross-product-vector-4d/
  inline static double4 cross(const double4 &v1, const double4 &v2) {return double4(v1.y*v2.z - v1.z* v2.y, -v1.x*v2.z + v1.z*v2.x, v1.x*v2.y - v1.y*v2.x, 0.0);}
  inline static double4 cross(const double4 &v1, const double4 &v2, const double4 &v3)
  {
    return double4(v1.y*(v2.z*v3.w - v3.z*v2.w) - v1.z*(v2.y*v3.w - v3.y*v2.w) + v1.w*(v2.y*v3.z - v3.y*v2.z),
                   -v1.x*(v2.z*v3.w - v3.z*v2.w) + v1.z*(v2.x*v3.w - v3.x*v2.w) - v1.w*(v2.x*v3.z - v3.x*v2.z),
                   v1.x*(v2.y*v3.w - v3.y*v2.w) - v1.y*(v2.x*v3.w - v3.x*v2.w) + v1.w*(v2.x*v3.y - v3.x*v2.y),
                   -v1.x*(v2.y*v3.z - v3.y*v2.z) + v1.y*(v2.x*v3.z - v3.x*v2.z) - v1.z*(v2.x*v3.y - v3.x*v2.y));
  }

  double4 operator-() const {return double4(-this->x, -this->y, -this->z, -this->w);}
  double4& operator+=(const double4 &b) {this->x += b.x, this->y += b.y, this->z += b.z; this->w += b.w; return *this;}
  double4& operator-=(const double4 &b) {this->x -= b.x, this->y -= b.y, this->z -= b.z; this->w += b.w; return *this;}

  friend QDataStream &operator<<(QDataStream &, const double4 &);
  friend QDataStream &operator>>(QDataStream &, double4 &);
};

inline double4 operator*(const double4 &a, const double4 &b)
{
   return double4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}

inline double4 operator*(const double4 &a, const double &b)
{
  return double4(a.x * b, a.y * b, a.z * b, a.w * b);
}

inline double4 operator*(const double &a, const double4 &b)
{
  return double4(a*b.x, a*b.y, a*b.z, a*b.w);
}
