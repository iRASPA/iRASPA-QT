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

#include "float3x3.h"
#include "float4x4.h"
 
#include <cmath>
#include <qmath.h>

#define sqr( x ) (( x )*( x ))
#define SIGN(a,b) ((b)>=0.0?fabs(a):-fabs(a))
#define MAX(x,y) (((x)>(y))?(x):(y))

// dlambda_limit, below which two lambdas are relatively equal
float flambda_limit = 1.0E-3f;
float fiszero_limit = 1.0E-20f;


float3x3::float3x3(float4x4 a)
{
   m11=a.m11; m21=a.m21; m31=a.m31;
   m12=a.m12; m22=a.m22; m32=a.m32;
   m13=a.m13; m23=a.m23; m33=a.m33;
}

float3x3::float3x3(simd_quatd q)
{
   float sqw = q.r*q.r;
   float sqx = q.ix*q.ix;
   float sqy = q.iy*q.iy;
   float sqz = q.iz*q.iz;

   // invs (inverse square length) is only required if quaternion is not already normalised
   float invs = 1 / (sqx + sqy + sqz + sqw);
   m11 = ( sqx - sqy - sqz + sqw)*invs ; // since sqw + sqx + sqy + sqz =1/invs*invs
   m22 = (-sqx + sqy - sqz + sqw)*invs ;
   m33 = (-sqx - sqy + sqz + sqw)*invs ;

   float tmp1 = q.ix*q.iy;
   float tmp2 = q.iz*q.r;
   m21 = 2.0 * (tmp1 + tmp2)*invs ;
   m12 = 2.0 * (tmp1 - tmp2)*invs ;

   tmp1 = q.ix*q.iz;
   tmp2 = q.iy*q.r;
   m31 = 2.0 * (tmp1 - tmp2)*invs ;
   m13 = 2.0 * (tmp1 + tmp2)*invs ;
   tmp1 = q.iy*q.iz;
   tmp2 = q.ix*q.r;
   m32 = 2.0 * (tmp1 + tmp2)*invs ;
   m23 = 2.0 * (tmp1 - tmp2)*invs ;
}

float float3x3::determinant(void)
{
  float determinant =
   + m11 * (m22 * m33 - m23 * m32)
   - m12 * (m21 * m33 - m23 * m31)
   + m13 * (m21 * m32 - m22 * m31);

   return determinant;
}

float float3x3::trace(void)
{
  return m11+m22+m33;
}

float3x3 const float3x3::inverse()
{
   float determinant =
   + m11 * (m22 * m33 - m23 * m32)
   - m12 * (m21 * m33 - m23 * m31)
   + m13 * (m21 * m32 - m22 * m31);

   float3x3 inverse;
   inverse.m11 = + (m22 * m33 - m32 * m23)/determinant;
   inverse.m21 = - (m21 * m33 - m31 * m23)/determinant;
   inverse.m31 = + (m21 * m32 - m31 * m22)/determinant;
   inverse.m12 = - (m12 * m33 - m32 * m13)/determinant;
   inverse.m22 = + (m11 * m33 - m31 * m13)/determinant;
   inverse.m32 = - (m11 * m32 - m31 * m12)/determinant;
   inverse.m13 = + (m12 * m23 - m22 * m13)/determinant;
   inverse.m23 = - (m11 * m23 - m21 * m13)/determinant;
   inverse.m33 = + (m11 * m22 - m21 * m12)/determinant;

   return inverse;
}

float3x3 const float3x3::inverse(const float3x3& a)
{
   float determinant =
   + a.m11 * (a.m22 * a.m33 - a.m23 * a.m32)
   - a.m12 * (a.m21 * a.m33 - a.m23 * a.m31)
   + a.m13 * (a.m21 * a.m32 - a.m22 * a.m31);

   float3x3 inverse;
   inverse.m11 = + (a.m22 * a.m33 - a.m32 * a.m23)/determinant;
   inverse.m21 = - (a.m21 * a.m33 - a.m31 * a.m23)/determinant;
   inverse.m31 = + (a.m21 * a.m32 - a.m31 * a.m22)/determinant;
   inverse.m12 = - (a.m12 * a.m33 - a.m32 * a.m13)/determinant;
   inverse.m22 = + (a.m11 * a.m33 - a.m31 * a.m13)/determinant;
   inverse.m32 = - (a.m11 * a.m32 - a.m31 * a.m12)/determinant;
   inverse.m13 = + (a.m12 * a.m23 - a.m22 * a.m13)/determinant;
   inverse.m23 = - (a.m11 * a.m23 - a.m21 * a.m13)/determinant;
   inverse.m33 = + (a.m11 * a.m22 - a.m21 * a.m12)/determinant;

   return inverse;
}


float3x3 const float3x3::transpose(void)
{
  float3x3 res;

  res.m11=m11; res.m12=m21; res.m13=m31;
  res.m21=m12; res.m22=m22; res.m23=m32;
  res.m31=m13; res.m32=m23; res.m33=m33;

  return res;
}

float3x3 float3x3::inversetranpose()
{
  float determinant =
  + m11 * (m22 * m33 - m23 * m32)
  - m12 * (m21 * m33 - m23 * m31)
  + m13 * (m21 * m32 - m22 * m31);

  float3x3 inverse;
  inverse.m11 = + (m22 * m33 - m32 * m23)/determinant;
  inverse.m12 = - (m21 * m33 - m31 * m23)/determinant;
  inverse.m13 = + (m21 * m32 - m31 * m22)/determinant;
  inverse.m21 = - (m12 * m33 - m32 * m13)/determinant;
  inverse.m22 = + (m11 * m33 - m31 * m13)/determinant;
  inverse.m23 = - (m11 * m32 - m31 * m12)/determinant;
  inverse.m31 = + (m12 * m23 - m22 * m13)/determinant;
  inverse.m32 = - (m11 * m23 - m21 * m13)/determinant;
  inverse.m33 = + (m11 * m22 - m21 * m12)/determinant;

  return inverse;
}

float trunc_sqrt( float x )
{ return( x <= 0.0 ? 0.0 : sqrt( x ) );
}

float trunc_acos( float x )
{ if( x >= 1.0 )
    return 0.0;
  if( x <= -1.0 )
    return float(M_PI);
  return acos( x );
}

float sign( float x )
{ return( x < 0.0 ? -1.0 : 1.0 );
}

float angle( float x, float y )
{ if( x == 0.0 )
    return( y == 0.0 ? 0.0 : 0.5 * M_PI * sign( y ) );
  return( x < 0.0 ? atan( y / x ) + M_PI * sign( y )
                  : atan( y / x ) );
}

void float3x3::solve_lambdas(float3 &res)
{ float p, q, b, t, delta;
  b = m[0] + m[4] + m[8];
  t = sqr( m[1] ) + sqr( m[2] ) + sqr( m[5] );
  p = 0.5 * ( sqr( m[0] - m[4] ) + sqr( m[0] - m[8] )
      + sqr( m[4] - m[8] ) );
  p += 3.0 * t;
  q = 18.0 * ( m[0] * m[4] * m[8] + 3.0 * m[1] * m[2] * m[5] );
  q += 2.0 * ( m[0] * sqr( m[0] ) + m[4] * sqr( m[4] ) +
               m[8] * sqr( m[8] ) );
  q += 9.0 * b * t;
  q -= 3.0 * ( m[0] + m[4] ) * ( m[0] + m[8] ) *
             ( m[4] + m[8] );
  q -= 27.0 * ( m[0] * sqr( m[5] ) + m[4] * sqr( m[2] ) +
                m[8] * sqr( m[1] ) );
  if( p < fiszero_limit )
    res[0] = res[1] = res[2] = b / 3.0;
  else
  { delta = trunc_acos( 0.5 * q / sqrt( p * sqr( p ) ) );
    p = 2.0 * sqrt( p );
// Changing the order in result yields different angles but identical matrix
    res[0] = ( b + p * cos( delta / 3.0 ) ) / 3.0;
    res[1] = ( b + p * cos( ( delta + 2.0 * M_PI ) / 3.0 ) ) / 3.0;
    res[2] = ( b + p * cos( ( delta - 2.0 * M_PI ) / 3.0 ) ) / 3.0;
  };
}

int solve_type( float3 lambdas)
{
  int i1, i2, isum = 0;
  float t, lambdasum = 0.0;

  for( int i=0; i<3; i++ )
    lambdasum += sqr( lambdas[i] );
  lambdasum = sqrt( lambdasum );
  for( int i=0; i<2; i++ )
   for( int j=i+1; j<3; j++ )
   { t = fabs( lambdas[i] - lambdas[j] );
     if( lambdasum > fiszero_limit )
       t /= lambdasum;
     if( t < flambda_limit )
     { isum++;
       i1 = i;
       i2 = j;
     };
   };
  if( isum == 0 )
    return 2;
  if( isum >= 2 )
    return 0;
  t = 0.5 * ( lambdas[i1] + lambdas[i2] );
  lambdas[2] = lambdas[3-i1-i2];
  lambdas[0] = lambdas[1] = t;
  return 1;
}


void float3x3::solve_angles_0( float3 &res, float3 lambdas )
{
  Q_UNUSED(lambdas);
  res[0] = 0.0;
  res[1] = 0.0;
  res[2] = 0.0;
}

void float3x3::solve_angles_1( float3 &res, float3 lambdas)
{
  float phi1a, phi1b, phi2, absdif, delta = 1.0E10;
  float g12, g21, t1, t2;
  t1 = lambdas[0] - lambdas[2];
  t2 = m[0] - lambdas[2];
  phi2 = trunc_acos( trunc_sqrt( t2 / t1 ) ); // + pi for symmetry
  g12 = 0.5 * t1 * sin( 2.0 * phi2 );
  g21 = t2;
  t1 = angle( m[4] - m[8], -2.0 * m[5] );
  t2 = sin( t1 );
  t1 = cos( t1 );
  phi1b = 0.5 * angle( g21 * t1, -g21 * t2 );
  t1 = angle( m[1], -1.0 * m[2] );
  t2 = sin( t1 );
  t1 = cos( t1 );
  bool big = sqr( m[4] - m[8] ) + sqr( 2.0 * m[5] )
            > sqr( m[1] ) + sqr( m[2] );
  for( int i=0; i<2; i++ )
  { phi1a = angle( g12 * t2, g12 * t1 );
    absdif = fabs( phi1a - phi1b );
    if( absdif < delta )
    { delta = absdif;
      res[0] = big ? phi1b : phi1a;
      res[1] = phi2;
    };
    phi2 = -phi2;
    g12 = -g12;
  };
  res[2] = 0.0;
}

void float3x3::solve_angles_2( float3 &res, float3 lambdas)
{
  float phi1a, phi1b, phi2, phi3, v1, w1, absdif, small_delta = 1.0E10;
  float g11, g12, g21, g22, t1, t2, t3, t4;
  t1 = lambdas[0] - lambdas[1];
  t2 = lambdas[1] - lambdas[2];
  t3 = lambdas[2] - lambdas[0];
  t4 = m[0] - lambdas[2];
  v1 = sqr( m[1] ) + sqr( m[2] );
  v1 += t4 * ( m[0] + t3 - lambdas[1] );
  v1 /= t2 * t3;
  if( fabs( v1 ) < fiszero_limit ) w1 = 1.0;
  else w1 = ( t4 - t2 * v1 ) / ( t1 * v1 );
  phi2 = trunc_acos( trunc_sqrt( v1 ) ); // + pi for symmetry
  phi3 = trunc_acos( trunc_sqrt( w1 ) ); // + pi for symmetry
  g11 = 0.5 * t1 * cos( phi2 ) * sin( 2.0 * phi3 );
  g12 = 0.5 * ( t1 * w1 + t2 ) * sin( 2.0 * phi2 );
  g21 = t1 * ( 1.0 + ( v1 - 2.0 ) * w1 ) + t2 * v1;
  g22 = t1 * sin( phi2 ) * sin( 2.0 * phi3 );
  t1 = angle( m[1], -1.0 * m[2] );
  t3 = angle(m[4] - m[8], -2.0 *m[5] );
  t2 = sin( t1 );
  t1 = cos( t1 );
  t4 = sin( t3 );
  t3 = cos( t3 );
  bool big = sqr( m[4] - m[8] ) + sqr( 2.0 * m[5] )
            > sqr( m[1] ) + sqr( m[2] );
  for( int i=0; i<4; i++ )
  { phi1a = angle( g11 * t1 + g12 * t2, -g11 * t2 + g12 * t1 );
    phi1b = 0.5 * angle( g21 * t3 + g22 * t4, -g21 * t4 + g22 * t3 );
    absdif = fabs( phi1a - phi1b );
    if( absdif < small_delta )
    { small_delta = absdif;
      res[0] = big ? phi1b : phi1a;
      res[1] = phi2;
      res[2] = phi3;
    };
    phi3 = -phi3;
    g11 = -g11;
    g22 = -g22;
    if( i == 1 )
    { phi2 = -phi2;
      g12 = -g12;
      g22 = -g22;
    };
  };
}


void float3x3::EigenSystemSymmetric(float3 &eigenvalues,float3x3 &eigenvectors)
{
  float3 angles;
  float3 cs,sn;
  int n_all_lambdas_equal = 0, n_two_lambdas_equal = 0,
    n_all_lambdas_different = 0;

  // compute eigenvalues
  solve_lambdas(eigenvalues);

  // compute anti-clockwise Euler angles
  switch( solve_type( eigenvalues ) )
  {
    case 0:
      solve_angles_0( angles, eigenvalues);
      n_all_lambdas_equal++;
      break;
    case 1:
      solve_angles_1( angles, eigenvalues);
      n_two_lambdas_equal++;
      break;
    case 2:
      solve_angles_2( angles, eigenvalues);
      n_all_lambdas_different++;
      break;
  };

  // compute eigenvectors R(angle1).R(angle2).R(angle3)
  // eigenvectors are orthonormal to machine-precision
  sn[0]=sin(angles[0]); sn[1]=sin(angles[1]); sn[2]=sin(angles[2]);
  cs[0]=cos(angles[0]); cs[1]=cos(angles[1]); cs[2]=cos(angles[2]);

  // first eigenvector
  eigenvectors.m11=cs[1]*cs[2];          eigenvectors.m12=cs[2]*sn[0]*sn[1]+cs[0]*sn[2];   eigenvectors.m13=-cs[0]*cs[2]*sn[1]+sn[0]*sn[2];

  // second eigenvector
  eigenvectors.m21=-cs[1]*sn[2];         eigenvectors.m22=cs[0]*cs[2]-sn[0]*sn[1]*sn[2];   eigenvectors.m23=cs[2]*sn[0]+cs[0]*sn[1]*sn[2];

  // third eigenvector
  eigenvectors.m31=sn[1];                eigenvectors.m32=-cs[1]*sn[0];                    eigenvectors.m33=cs[0]*cs[1];

}


std::ostream& operator<<(std::ostream& out, const float3x3& vec) // output
{
  out << vec.m[0] <<  vec.m[1] << vec.m[2];
  out << vec.m[3] <<  vec.m[4] << vec.m[5];
  out << vec.m[6] <<  vec.m[7] << vec.m[8];
  return out;
}


