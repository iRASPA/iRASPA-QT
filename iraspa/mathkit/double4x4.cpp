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

#include "double4x4.h"
#include <cmath>
#include "double3x3.h"

#include <iostream>
#include <ostream>

double4x4::double4x4(double3x3 a)
{
   m11=a.m11; m21=a.m21; m31=a.m31; m41=0.0;
   m12=a.m12; m22=a.m22; m32=a.m32; m42=0.0;
   m13=a.m13; m23=a.m23; m33=a.m33; m43=0.0;
   m14=0.0;   m24=0.0;   m34=0.0;   m44=1.0;
}

double4x4 double4x4::operator * (const double4x4& b) const
{
  double4x4 r;

  r.m11 = m11 * b.m11 + m12 * b.m21 + m13 * b.m31 + m14 * b.m41;
  r.m21 = m21 * b.m11 + m22 * b.m21 + m23 * b.m31 + m24 * b.m41;
  r.m31 = m31 * b.m11 + m32 * b.m21 + m33 * b.m31 + m34 * b.m41;
  r.m41 = m41 * b.m11 + m42 * b.m21 + m43 * b.m31 + m44 * b.m41;

  r.m12 = m11 * b.m12 + m12 * b.m22 + m13 * b.m32 + m14 * b.m42;
  r.m22 = m21 * b.m12 + m22 * b.m22 + m23 * b.m32 + m24 * b.m42;
  r.m32 = m31 * b.m12 + m32 * b.m22 + m33 * b.m32 + m34 * b.m42;
  r.m42 = m41 * b.m12 + m42 * b.m22 + m43 * b.m32 + m44 * b.m42;

  r.m13 = m11 * b.m13 + m12 * b.m23 + m13 * b.m33 + m14 * b.m43;
  r.m23 = m21 * b.m13 + m22 * b.m23 + m23 * b.m33 + m24 * b.m43;
  r.m33 = m31 * b.m13 + m32 * b.m23 + m33 * b.m33 + m34 * b.m43;
  r.m43 = m41 * b.m13 + m42 * b.m23 + m43 * b.m33 + m44 * b.m43;

  r.m14 = m11 * b.m14 + m12 * b.m24 + m13 * b.m34 + m14 * b.m44;
  r.m24 = m21 * b.m14 + m22 * b.m24 + m23 * b.m34 + m24 * b.m44;
  r.m34 = m31 * b.m14 + m32 * b.m24 + m33 * b.m34 + m34 * b.m44;
  r.m44 = m41 * b.m14 + m42 * b.m24 + m43 * b.m34 + m44 * b.m44;

  return r;
}

 double4 double4x4::operator * (const double4& b) const
 {
   double4 r;

   r[0] = m11 * b[0] + m12 * b[1] + m13 * b[2] + m14 * b[3];
   r[1] = m21 * b[0] + m22 * b[1] + m23 * b[2] + m24 * b[3];
   r[2] = m31 * b[0] + m32 * b[1] + m33 * b[2] + m34 * b[3];
   r[3] = m41 * b[0] + m42 * b[1] + m43 * b[2] + m44 * b[3];

   return r;
 }


bool double4x4::operator==(const double4x4& b) const
{
  bool equal = true;
  const double epsilon = 1e-8;

  for (int i = 0; i < 16 && equal; i++)
    equal = fabs(m[i] - b.m[i]) <= epsilon;

  return equal;
}

bool double4x4::operator!=(const double4x4& b) const
{
  return !operator==(b);
}


double4x4 const double4x4::transpose(void)
{
  double4x4 res;

  res.m11=m11; res.m12=m21; res.m13=m31; res.m14=m41;
  res.m21=m12; res.m22=m22; res.m23=m32; res.m24=m42;
  res.m31=m13; res.m32=m32; res.m33=m33; res.m34=m43;
  res.m41=m14; res.m42=m42; res.m43=m34; res.m44=m44;

  return res;
}

double4x4 const double4x4::inverse(const double4x4& a)
{
   double4x4 inverse;

   double d01 = ( a.m13 * a.m24 - a.m14 * a.m23 );
   double d02 = ( a.m13 * a.m34 - a.m14 * a.m33 );
   double d12 = ( a.m23 * a.m34 - a.m24 * a.m33 );
   double d13 = ( a.m23 * a.m44 - a.m24 * a.m43 );
   double d23 = ( a.m33 * a.m44 - a.m34 * a.m43 );
   double d30 = ( a.m43 * a.m14 - a.m44 * a.m13 );

   const double cell0  =  ( a.m22 * d23 - a.m32 * d13 + a.m42 * d12 );
   const double cell4  = -( a.m12 * d23 + a.m32 * d30 + a.m42 * d02 );
   const double cell8  =  ( a.m12 * d13 + a.m22 * d30 + a.m42 * d01 );
   const double cell12 = -( a.m12 * d12 - a.m22 * d02 + a.m32 * d01 );

   const double determinant = a.m11 * cell0 + a.m21 * cell4 + a.m31 * cell8 + a.m41 * cell12;
   if(fabs( determinant ) < 1e-8)
   {
     std::cout << "Error";
   }

   inverse.m11 = cell0;
   inverse.m12 = cell4;
   inverse.m13 = cell8;
   inverse.m14 = cell12;

   inverse.m21 = -( a.m21 * d23 - a.m31 * d13 + a.m41 * d12 );
   inverse.m22 =  ( a.m11 * d23 + a.m31 * d30 + a.m41 * d02 );
   inverse.m23 = -( a.m11 * d13 + a.m21 * d30 + a.m41 * d01 );
   inverse.m24 =  ( a.m11 * d12 - a.m21 * d02 + a.m31 * d01 );

   d01 = a.m11 * a.m22 - a.m12 * a.m21;
   d02 = a.m11 * a.m32 - a.m12 * a.m31;
   d12 = a.m21 * a.m32 - a.m22 * a.m31;
   d13 = a.m21 * a.m42 - a.m22 * a.m41;
   d23 = a.m31 * a.m42 - a.m32 * a.m41;
   d30 = a.m41 * a.m12 - a.m42 * a.m11;

   inverse.m31 =  ( a.m24 * d23 - a.m34 * d13 + a.m44 * d12 );
   inverse.m32 = -( a.m14 * d23 + a.m34 * d30 + a.m44 * d02 );
   inverse.m33 =  ( a.m14 * d13 + a.m24 * d30 + a.m44 * d01 );
   inverse.m34 = -( a.m14 * d12 - a.m24 * d02 + a.m34 * d01 );

   inverse.m41 = -( a.m23 * d23 - a.m33 * d13 + a.m43 * d12 );
   inverse.m42 =  ( a.m13 * d23 + a.m33 * d30 + a.m43 * d02 );
   inverse.m43 = -( a.m13 * d13 + a.m23 * d30 + a.m43 * d01 );
   inverse.m44 =  ( a.m13 * d12 - a.m23 * d02 + a.m33 * d01 );

   const double invDeterminant = 1.0/ determinant;
   inverse.m11*=invDeterminant; inverse.m21*=invDeterminant; inverse.m31*=invDeterminant; inverse.m41*=invDeterminant;
   inverse.m12*=invDeterminant; inverse.m22*=invDeterminant; inverse.m32*=invDeterminant; inverse.m42*=invDeterminant;
   inverse.m13*=invDeterminant; inverse.m23*=invDeterminant; inverse.m33*=invDeterminant; inverse.m43*=invDeterminant;
   inverse.m14*=invDeterminant; inverse.m24*=invDeterminant; inverse.m34*=invDeterminant; inverse.m44*=invDeterminant;
   return inverse;
}

void double4x4::inverse()
{
   double4x4 inverse;

   double d01 = ( m13 * m24 - m14 * m23 );
   double d02 = ( m13 * m34 - m14 * m33 );
   double d12 = ( m23 * m34 - m24 * m33 );
   double d13 = ( m23 * m44 - m24 * m43 );
   double d23 = ( m33 * m44 - m34 * m43 );
   double d30 = ( m43 * m14 - m44 * m13 );

   const double cell0  =  ( m22 * d23 - m32 * d13 + m42 * d12 );
   const double cell4  = -( m12 * d23 + m32 * d30 + m42 * d02 );
   const double cell8  =  ( m12 * d13 + m22 * d30 + m42 * d01 );
   const double cell12 = -( m12 * d12 - m22 * d02 + m32 * d01 );

   const double determinant = m11 * cell0 + m21 * cell4 + m31 * cell8 + m41 * cell12;
   if(fabs( determinant ) < 1e-8)
   {
     std::cout << "Error";
   }

   inverse.m11 = cell0;
   inverse.m12 = cell4;
   inverse.m13 = cell8;
   inverse.m14 = cell12;

   inverse.m21 = -( m21 * d23 - m31 * d13 + m41 * d12 );
   inverse.m22 =  ( m11 * d23 + m31 * d30 + m41 * d02 );
   inverse.m23 = -( m11 * d13 + m21 * d30 + m41 * d01 );
   inverse.m24 =  ( m11 * d12 - m21 * d02 + m31 * d01 );

   d01 = m11 * m22 - m12 * m21;
   d02 = m11 * m32 - m12 * m31;
   d12 = m21 * m32 - m22 * m31;
   d13 = m21 * m42 - m22 * m41;
   d23 = m31 * m42 - m32 * m41;
   d30 = m41 * m12 - m42 * m11;

   inverse.m31 =  ( m24 * d23 - m34 * d13 + m44 * d12 );
   inverse.m32 = -( m14 * d23 + m34 * d30 + m44 * d02 );
   inverse.m33 =  ( m14 * d13 + m24 * d30 + m44 * d01 );
   inverse.m34 = -( m14 * d12 - m24 * d02 + m34 * d01 );

   inverse.m41 = -( m23 * d23 - m33 * d13 + m43 * d12 );
   inverse.m42 =  ( m13 * d23 + m33 * d30 + m43 * d02 );
   inverse.m43 = -( m13 * d13 + m23 * d30 + m43 * d01 );
   inverse.m44 =  ( m13 * d12 - m23 * d02 + m33 * d01 );

   const double invDeterminant = 1.0/ determinant;
   inverse.m11*=invDeterminant; inverse.m21*=invDeterminant; inverse.m31*=invDeterminant; inverse.m41*=invDeterminant;
   inverse.m12*=invDeterminant; inverse.m22*=invDeterminant; inverse.m32*=invDeterminant; inverse.m42*=invDeterminant;
   inverse.m13*=invDeterminant; inverse.m23*=invDeterminant; inverse.m33*=invDeterminant; inverse.m43*=invDeterminant;
   inverse.m14*=invDeterminant; inverse.m24*=invDeterminant; inverse.m34*=invDeterminant; inverse.m44*=invDeterminant;
   *this=inverse;
}

 double4x4 double4x4::TransformationAroundArbitraryPoint(double4x4 m, double3 p)
{
   double3x3 R;

   R.m11=1.0-m.m11; R.m12=-m.m12;    R.m13=-m.m13;
   R.m21=-m.m21;    R.m22=1.0-m.m22; R.m23=-m.m23;
   R.m31=-m.m31;    R.m32=-m.m32;    R.m33=1.0-m.m33;

   m.m14 += R.m11 * p.x + R.m12 * p.y + R.m13 * p.z;
   m.m24 += R.m21 * p.x + R.m22 * p.y + R.m23 * p.z;
   m.m34 += R.m31 * p.x + R.m32 * p.y + R.m33 * p.z;

   return m;
}


double4x4 double4x4::Identity(void)
{
  double4x4 R;

  R.m11=1.0; R.m12=0.0; R.m13=0.0; R.m14=0.0;
  R.m21=0.0; R.m22=1.0; R.m23=0.0; R.m24=0.0;
  R.m31=0.0; R.m32=0.0; R.m33=1.0; R.m34=0.0;
  R.m41=0.0; R.m42=0.0; R.m43=0.0; R.m44=1.0;

  return R;
}


// Book: "Essential Mathematics for Games & Interactive Applications", 2nd edition, page 157
double4x4 double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4 m, double3 p)
{
   double4x4 R;

   R.m11=1.0-m.m11; R.m12=-m.m12;    R.m13=-m.m13;
   R.m21=-m.m21;    R.m22=1.0-m.m22; R.m23=-m.m23;
   R.m31=-m.m31;    R.m32=-m.m32;    R.m33=1.0-m.m33;

   m.m14 += R.m11 * p.x + R.m12 * p.y + R.m13 * p.z;
   m.m24 += R.m21 * p.x + R.m22 * p.y + R.m23 * p.z;
   m.m34 += R.m31 * p.x + R.m32 * p.y + R.m33 * p.z;

   return m;
}

double4x4 double4x4::AffinityMatrixToTransformationAroundArbitraryPointWithTranslation(double4x4 m, double3 p, double3 t)
{
   double4x4 R;

   R.m11=1.0-m.m11; R.m12=-m.m12;    R.m13=-m.m13;
   R.m21=-m.m21;    R.m22=1.0-m.m22; R.m23=-m.m23;
   R.m31=-m.m31;    R.m32=-m.m32;    R.m33=1.0-m.m33;

   m.m14 += R.m11 * p.x + R.m12 * p.y + R.m13 * p.z + t.x;
   m.m24 += R.m21 * p.x + R.m22 * p.y + R.m23 * p.z + t.y;
   m.m34 += R.m31 * p.x + R.m32 * p.y + R.m33 * p.z + t.z;

   return m;
}


double4x4 double4x4::Rotationdouble4x4FromQuaternion(simd_quatd q)
{
  double4x4 m;
  double sqw = q.r*q.r;
  double sqx = q.ix*q.ix;
  double sqy = q.iy*q.iy;
  double sqz = q.iz*q.iz;

  // invs (inverse square length) is only required if quaternion is not already normalised
  double invs = 1 / (sqx + sqy + sqz + sqw);
  m.m11 = ( sqx - sqy - sqz + sqw)*invs ; // since sqw + sqx + sqy + sqz =1/invs*invs
  m.m22 = (-sqx + sqy - sqz + sqw)*invs ;
  m.m33 = (-sqx - sqy + sqz + sqw)*invs ;
  m.m44 = 1.0;

  double tmp1 = q.ix*q.iy;
  double tmp2 = q.iz*q.r;
  m.m21 = 2.0 * (tmp1 + tmp2)*invs ;
  m.m12 = 2.0 * (tmp1 - tmp2)*invs ;
  m.m14=m.m41=0.0;

  tmp1 = q.ix*q.iz;
  tmp2 = q.iy*q.r;
  m.m31 = 2.0 * (tmp1 - tmp2)*invs ;
  m.m13 = 2.0 * (tmp1 + tmp2)*invs ;
  m.m24=m.m42=0.0;

  tmp1 = q.iy*q.iz;
  tmp2 = q.ix*q.r;
  m.m32 = 2.0 * (tmp1 + tmp2)*invs ;
  m.m23 = 2.0 * (tmp1 - tmp2)*invs ;
  m.m34=m.m43=0.0;

  return m;
}



std::ostream& operator<<(std::ostream& out, const double4x4& vec) // output
{
  out << vec.m11 <<  vec.m21 << vec.m31 << vec.m41;
  out << vec.m12 <<  vec.m22 << vec.m32 << vec.m42;
  out << vec.m13 <<  vec.m23 << vec.m33 << vec.m43;
  out << vec.m14 <<  vec.m24 << vec.m34 << vec.m44;
  return out;
}

