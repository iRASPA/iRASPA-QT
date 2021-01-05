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

#include "double3x3.h"
#include "double4x4.h"

#include <cmath>
#include <qmath.h>
#if defined (Q_OS_OSX)
  #include <Accelerate/Accelerate.h>
#endif

#define sqr( x ) (( x )*( x ))
#define SIGN(a,b) ((b)>=0.0?fabs(a):-fabs(a))

// dlambda_limit, below which two lambdas are relatively equal
double dlambda_limit = 1.0E-3;
double iszero_limit = 1.0E-20;


double3x3::double3x3(double4x4 a)
{
   m11=a.m11; m21=a.m21; m31=a.m31;
   m12=a.m12; m22=a.m22; m32=a.m32;
   m13=a.m13; m23=a.m23; m33=a.m33;
}

double3x3::double3x3(simd_quatd q)
{
   double sqw = q.r*q.r;
   double sqx = q.ix*q.ix;
   double sqy = q.iy*q.iy;
   double sqz = q.iz*q.iz;

   // invs (inverse square length) is only required if quaternion is not already normalised
   double invs = 1.0 / (sqx + sqy + sqz + sqw);
   m11 = ( sqx - sqy - sqz + sqw)*invs ; // since sqw + sqx + sqy + sqz =1/invs*invs
   m22 = (-sqx + sqy - sqz + sqw)*invs ;
   m33 = (-sqx - sqy + sqz + sqw)*invs ;

   double tmp1 = q.ix*q.iy;
   double tmp2 = q.iz*q.r;
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

double double3x3::determinant(void)
{
  double determinant =
   + m11 * (m22 * m33 - m23 * m32)
   - m12 * (m21 * m33 - m23 * m31)
   + m13 * (m21 * m32 - m22 * m31);

   return determinant;
}

double3x3::double3x3(double lattice[3][3])
{
  m11 = lattice[0][0]; m21 = lattice[1][0]; m31 = lattice[2][0];
  m12 = lattice[0][1]; m22 = lattice[1][1]; m32 = lattice[2][1];
  m13 = lattice[0][2]; m23 = lattice[1][2]; m33 = lattice[2][2];
}

double double3x3::trace(void)
{
  return m11+m22+m33;
}

double3x3 const double3x3::inverse()
{
   double determinant =
   + m11 * (m22 * m33 - m23 * m32)
   - m12 * (m21 * m33 - m23 * m31)
   + m13 * (m21 * m32 - m22 * m31);

   double3x3 inverse;
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

double3x3 const double3x3::inverse(const double3x3& a)
{
   double determinant =
   + a.m11 * (a.m22 * a.m33 - a.m23 * a.m32)
   - a.m12 * (a.m21 * a.m33 - a.m23 * a.m31)
   + a.m13 * (a.m21 * a.m32 - a.m22 * a.m31);

   double3x3 inverse;
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

double3x3 double3x3::transpose(const double3x3& right)
{
    double3x3 res;

    res.m11=right.m11; res.m12=right.m21; res.m13=right.m31;
    res.m21=right.m12; res.m22=right.m22; res.m23=right.m32;
    res.m31=right.m13; res.m32=right.m23; res.m33=right.m33;

    return res;
}


double3x3 const double3x3::transpose(void) const
{
  double3x3 res;

  res.m11=m11; res.m12=m21; res.m13=m31;
  res.m21=m12; res.m22=m22; res.m23=m32;
  res.m31=m13; res.m32=m23; res.m33=m33;

  return res;
}

double3x3 double3x3::inversetranpose()
{
  double determinant =
  + m11 * (m22 * m33 - m23 * m32)
  - m12 * (m21 * m33 - m23 * m31)
  + m13 * (m21 * m32 - m22 * m31);

  double3x3 inverse;
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


double3x3 double3x3::operator * (const double3x3& b) const
{
  double3x3 r;


  r.m11 = m11 * b.m11 + m12 * b.m21 + m13 * b.m31;
  r.m21 = m21 * b.m11 + m22 * b.m21 + m23 * b.m31;
  r.m31 = m31 * b.m11 + m32 * b.m21 + m33 * b.m31;

  r.m12 = m11 * b.m12 + m12 * b.m22 + m13 * b.m32;
  r.m22 = m21 * b.m12 + m22 * b.m22 + m23 * b.m32;
  r.m32 = m31 * b.m12 + m32 * b.m22 + m33 * b.m32;

  r.m13 = m11 * b.m13 + m12 * b.m23 + m13 * b.m33;
  r.m23 = m21 * b.m13 + m22 * b.m23 + m23 * b.m33;
  r.m33 = m31 * b.m13 + m32 * b.m23 + m33 * b.m33;


  return r;
}

double3 double3x3::operator * (const double3& b) const
 {
   double3 r;

   r.x = m11 * b.x + m12 * b.y + m13 * b.z;
   r.y = m21 * b.x + m22 * b.y + m23 * b.z;
   r.z = m31 * b.x + m32 * b.y + m33 * b.z;

   return r;
 }



double trunc_sqrt( double x )
{ return( x <= 0.0 ? 0.0 : sqrt( x ) );
}

double trunc_acos( double x )
{ if( x >= 1.0 )
    return 0.0;
  if( x <= -1.0 )
    return M_PI;
  return acos( x );
}

double sign( double x )
{ return( x < 0.0 ? -1.0 : 1.0 );
}

double angle( double x, double y )
{ if( x == 0.0 )
    return( y == 0.0 ? 0.0 : 0.5 * M_PI * sign( y ) );
  return( x < 0.0 ? atan( y / x ) + M_PI * sign( y )
                  : atan( y / x ) );
}

void double3x3::solve_lambdas(double3 &res)
{ double p, q, b, t, delta;
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
  if( p < iszero_limit )
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

int solve_type( double3 lambdas)
{
  int i1, i2, isum = 0;
  double t, lambdasum = 0.0;

  for( int i=0; i<3; i++ )
    lambdasum += sqr( lambdas[i] );
  lambdasum = sqrt( lambdasum );
  for( int i=0; i<2; i++ )
   for( int j=i+1; j<3; j++ )
   { t = fabs( lambdas[i] - lambdas[j] );
     if( lambdasum > iszero_limit )
       t /= lambdasum;
     if( t < dlambda_limit )
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


void double3x3::solve_angles_0( double3 &res, [[maybe_unused]] double3 lambdas )
{
  res[0] = 0.0;
  res[1] = 0.0;
  res[2] = 0.0;
}

void double3x3::solve_angles_1( double3 &res, double3 lambdas)
{
  double phi1a, phi1b, phi2, absdif, delta = 1.0E10;
  double g12, g21, t1, t2;
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

void double3x3::solve_angles_2( double3 &res, double3 lambdas)
{
  double phi1a, phi1b, phi2, phi3, v, w, absdif, delta = 1.0E10;
  double g11, g12, g21, g22, t1, t2, t3, t4;
  t1 = lambdas[0] - lambdas[1];
  t2 = lambdas[1] - lambdas[2];
  t3 = lambdas[2] - lambdas[0];
  t4 = m[0] - lambdas[2];
  v = sqr( m[1] ) + sqr( m[2] );
  v += t4 * ( m[0] + t3 - lambdas[1] );
  v /= t2 * t3;
  if( fabs( v ) < iszero_limit ) w = 1.0;
  else w = ( t4 - t2 * v ) / ( t1 * v );
  phi2 = trunc_acos( trunc_sqrt( v ) ); // + pi for symmetry
  phi3 = trunc_acos( trunc_sqrt( w ) ); // + pi for symmetry
  g11 = 0.5 * t1 * cos( phi2 ) * sin( 2.0 * phi3 );
  g12 = 0.5 * ( t1 * w + t2 ) * sin( 2.0 * phi2 );
  g21 = t1 * ( 1.0 + ( v - 2.0 ) * w ) + t2 * v;
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
    if( absdif < delta )
    { delta = absdif;
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

#if defined (Q_OS_OSX)
void double3x3::EigenSystemSymmetric(double3 &eigenvalues, double3x3 &eigenvectors)
{
  char decompositionJobV = 'V';
  char upload = 'U';
  std::vector<double> matrix = std::vector<double>{ax, ay, az, bx, by, bz, cx, cy, cz};
  std::vector<double> work(9*3);
  int lwork = 9 * 3;
  std::vector<double> e = std::vector<double>(3);
  int error = 0;
  int N = 3;
  int M = 3;

  dsyev_(&decompositionJobV, &upload, &M, matrix.data(), &N, e.data(), work.data(), &lwork, &error);

  eigenvalues = double3(e[2],e[1],e[0]);
  double3 v1,v2,v3;
  v1 = double3(matrix[0],matrix[1],matrix[2]);
  v2 = double3(matrix[3],matrix[4],matrix[5]);
  v3 = double3(matrix[6],matrix[7],matrix[8]);
  eigenvectors = double3x3(v1,v2,v3);
  if(eigenvectors.determinant()<0)
  {
    eigenvectors = double3x3(v1,v3,v2);
  }
}

#else
void double3x3::EigenSystemSymmetric(double3 &eigenvalues, double3x3 &eigenvectors)
{
  double3 angles;
  double3 cs,sn;
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

  double3 v1,v2,v3;
  v1 = double3(eigenvectors.m11,eigenvectors.m12,eigenvectors.m13);
  v2 = double3(eigenvectors.m21,eigenvectors.m22,eigenvectors.m23);
  v3 = double3(eigenvectors.m31,eigenvectors.m32,eigenvectors.m33);
  eigenvectors = double3x3(v1,v2,v3);
  if(eigenvectors.determinant()<0)
  {
    eigenvectors = double3x3(v1,v3,v2);
  }
}
#endif


// Assume [U,D,V] = SVD(A). Then
// U = eigenvectors(A'A), V = eigenvectors(AA'), and D = sqrt(eigenvalues(A));
void double3x3::SingularValueDecompositionSymmetric(double3x3 &U,double3 &D,double3x3 &V)
{
   double3x3 a;

   a=(this->transpose()) * (*this);
   a.EigenSystemSymmetric(D,U);

   // for a symmetric matrix V=U
   V=U;

   D[0]=sqrt(D[0]); D[1]=sqrt(D[1]); D[2]=sqrt(D[2]);
}


static double PYTHAG(double a, double b)
{
    double at = fabs(a), bt = fabs(b), ct, result;

    if (at > bt)       { ct = bt / at; result = at * sqrt(1.0 + ct * ct); }
    else if (bt > 0.0) { ct = at / bt; result = bt * sqrt(1.0 + ct * ct); }
    else result = 0.0;
    return(result);
}


// given matrix M:n×m that performs some linear transformation, the SVD decomposes that transformation into three steps: U:m×m, D:m×n, V:n×n, such that M=U×D×V^T
// V^T is the rotation to a more suitable axis system
// D is the scaling done by M, D is a rectangular diagonal, with d_{ii}>d_{jj},i<j
// The values of matrix D state the importance of each scaling. If the last values are low, they can be discarded without much loss.
// U is the rotation back to the initial axis system
// The m columns of U are called the left singular vectors. The n columns of V are called the right singular vectors.

// The right singular vectors are eigenvectors of M^T×M
// The left singular vectors are eigenvectors of M×M^T
// The non-zero values of D are the square root of the eigenvalues of M×M^T and M^T×M are called the singular values
// These singular values can be seen as the semiaxes of an ellipsoid where the scaling is done.
// U and V are unitary matrixes, i.e. U×U^T=I and V×V^T=I.

//  matrix=a*double3x3(w[0],0,0, 0,w[1],0, 0,0,w[2])*v.transpose();
void double3x3::SingularValueDecomposition(double3x3 &a,double3 &w,double3x3 &v)
{
    int flag, i, its, j, jj, k, l, nm;
    double c, f, h, s, x, y, z;
    double anorm = 0.0, g = 0.0, scale = 0.0;
    double *rv1;

    int n=3;
    int m=3;

    a=*this;


    if (m < n)
    {
        fprintf(stderr, "#rows must be > #cols \n");
        return;
    }

    rv1 = (double *)malloc((unsigned int) n*sizeof(double));

/* Householder reduction to bidiagonal form */
    for (i = 0; i < n; i++)
    {
        /* left-hand reduction */
        l = i + 1;
        rv1[i] = scale * g;
        g = s = scale = 0.0;
        if (i < m)
        {
            for (k = i; k < m; k++)
                scale += fabs((double)a.mm[k][i]);
            if (scale)
            {
                for (k = i; k < m; k++)
                {
                    a.mm[k][i] = (float)((double)a.mm[k][i]/scale);
                    s += ((double)a.mm[k][i] * (double)a.mm[k][i]);
                }
                f = (double)a.mm[i][i];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a.mm[i][i] = (float)(f - g);
                if (i != n - 1)
                {
                    for (j = l; j < n; j++)
                    {
                        for (s = 0.0, k = i; k < m; k++)
                            s += ((double)a.mm[k][i] * (double)a.mm[k][j]);
                        f = s / h;
                        for (k = i; k < m; k++)
                            a.mm[k][j] += (float)(f * (double)a.mm[k][i]);
                    }
                }
                for (k = i; k < m; k++)
                    a.mm[k][i] = (float)((double)a.mm[k][i]*scale);
            }
        }
        w[i] = (float)(scale * g);

        /* right-hand reduction */
        g = s = scale = 0.0;
        if (i < m && i != n - 1)
        {
            for (k = l; k < n; k++)
                scale += fabs((double)a.mm[i][k]);
            if (scale)
            {
                for (k = l; k < n; k++)
                {
                    a.mm[i][k] = (float)((double)a.mm[i][k]/scale);
                    s += ((double)a.mm[i][k] * (double)a.mm[i][k]);
                }
                f = (double)a.mm[i][l];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a.mm[i][l] = (float)(f - g);
                for (k = l; k < n; k++)
                    rv1[k] = (double)a.mm[i][k] / h;
                if (i != m - 1)
                {
                    for (j = l; j < m; j++)
                    {
                        for (s = 0.0, k = l; k < n; k++)
                            s += ((double)a.mm[j][k] * (double)a.mm[i][k]);
                        for (k = l; k < n; k++)
                            a.mm[j][k] += (float)(s * rv1[k]);
                    }
                }
                for (k = l; k < n; k++)
                    a.mm[i][k] = (float)((double)a.mm[i][k]*scale);
            }
        }
        anorm = std::max(anorm, (fabs((double)w[i]) + fabs(rv1[i])));
    }

    /* accumulate the right-hand transformation */
    for (i = n - 1; i >= 0; i--)
    {
        if (i < n - 1)
        {
            if (g)
            {
                for (j = l; j < n; j++)
                    v.mm[j][i] = (float)(((double)a.mm[i][j] / (double)a.mm[i][l]) / g);
                    /* double division to avoid underflow */
                for (j = l; j < n; j++)
                {
                    for (s = 0.0, k = l; k < n; k++)
                        s += ((double)a.mm[i][k] * (double)v.mm[k][j]);
                    for (k = l; k < n; k++)
                        v.mm[k][j] += (float)(s * (double)v.mm[k][i]);
                }
            }
            for (j = l; j < n; j++)
                v.mm[i][j] = v.mm[j][i] = 0.0;
        }
        v.mm[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }

    /* accumulate the left-hand transformation */
    for (i = n - 1; i >= 0; i--)
    {
        l = i + 1;
        g = (double)w[i];
        if (i < n - 1)
            for (j = l; j < n; j++)
                a.mm[i][j] = 0.0;
        if (g)
        {
            g = 1.0 / g;
            if (i != n - 1)
            {
                for (j = l; j < n; j++)
                {
                    for (s = 0.0, k = l; k < m; k++)
                        s += ((double)a.mm[k][i] * (double)a.mm[k][j]);
                    f = (s / (double)a.mm[i][i]) * g;
                    for (k = i; k < m; k++)
                        a.mm[k][j] += (float)(f * (double)a.mm[k][i]);
                }
            }
            for (j = i; j < m; j++)
                a.mm[j][i] = (float)((double)a.mm[j][i]*g);
        }
        else
        {
            for (j = i; j < m; j++)
                a.mm[j][i] = 0.0;
        }
        ++a.mm[i][i];
    }

    /* diagonalize the bidiagonal form */
    for (k = n - 1; k >= 0; k--)
    {                             /* loop over singular values */
        for (its = 0; its < 30; its++)
        {                         /* loop over allowed iterations */
            flag = 1;
            for (l = k; l >= 0; l--)
            {                     /* test for splitting */
                nm = l - 1;
                if (fabs(rv1[l]) + anorm == anorm)
                {
                    flag = 0;
                    break;
                }
                if (fabs((double)w[nm]) + anorm == anorm)
                    break;
            }
            if (flag)
            {
                c = 0.0;
                s = 1.0;
                for (i = l; i <= k; i++)
                {
                    f = s * rv1[i];
                    if (fabs(f) + anorm != anorm)
                    {
                        g = (double)w[i];
                        h = PYTHAG(f, g);
                        w[i] = (float)h;
                        h = 1.0 / h;
                        c = g * h;
                        s = (- f * h);
                        for (j = 0; j < m; j++)
                        {
                            y = (double)a.mm[j][nm];
                            z = (double)a.mm[j][i];
                            a.mm[j][nm] = (float)(y * c + z * s);
                            a.mm[j][i] = (float)(z * c - y * s);
                        }
                    }
                }
            }
            z = (double)w[k];
            if (l == k)
            {                  /* convergence */
                if (z < 0.0)
                {              /* make singular value nonnegative */
                    w[k] = (float)(-z);
                    for (j = 0; j < n; j++)
                        v.mm[j][k] = (-v.mm[j][k]);
                }
                break;
            }
            if (its >= 30) {
                free((void*) rv1);
                fprintf(stderr, "No convergence after 30,000! iterations \n");
                return;
            }

            /* shift from bottom 2 x 2 minor */
            x = (double)w[l];
            nm = k - 1;
            y = (double)w[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = PYTHAG(f, 1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;

            /* next QR transformation */
            c = s = 1.0;
            for (j = l; j <= nm; j++)
            {
                i = j + 1;
                g = rv1[i];
                y = (double)w[i];
                h = s * g;
                g = c * g;
                z = PYTHAG(f, h);
                rv1[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y = y * c;
                for (jj = 0; jj < n; jj++)
                {
                    x = (double)v.mm[jj][j];
                    z = (double)v.mm[jj][i];
                    v.mm[jj][j] = (float)(x * c + z * s);
                    v.mm[jj][i] = (float)(z * c - x * s);
                }
                z = PYTHAG(f, h);
                w[j] = (float)z;
                if (z)
                {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = (c * g) + (s * y);
                x = (c * y) - (s * g);
                for (jj = 0; jj < m; jj++)
                {
                    y = (double)a.mm[jj][j];
                    z = (double)a.mm[jj][i];
                    a.mm[jj][j] = (float)(y * c + z * s);
                    a.mm[jj][i] = (float)(z * c - y * s);
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            w[k] = (float)x;
        }
    }
    free((void*) rv1);
}

std::ostream& operator<<(std::ostream& out, const double3x3& vec) // output
{
  out << vec.m[0] <<  vec.m[1] << vec.m[2];
  out << vec.m[3] <<  vec.m[4] << vec.m[5];
  out << vec.m[6] <<  vec.m[7] << vec.m[8];
  return out;
}

QDebug operator<<(QDebug debug, const double3x3 &m)
{
  debug << "[(" << m.m[0] << ", " << m.m[1] << ", " << m.m[2] << "), ("
                << m.m[3] << ", " << m.m[4] << ", " << m.m[5] << "), ("
                << m.m[6] << ", " << m.m[7] << ", " << m.m[8] << ")]";
  return debug;
}

QDataStream &operator<<(QDataStream &stream, const double3x3 &v)
{
  stream << v.ax;
  stream << v.ay;
  stream << v.az;

  stream << v.bx;
  stream << v.by;
  stream << v.bz;

  stream << v.cx;
  stream << v.cy;
  stream << v.cz;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, double3x3 &v)
{
  stream >> v.ax;
  stream >> v.ay;
  stream >> v.az;

  stream >> v.bx;
  stream >> v.by;
  stream >> v.bz;

  stream >> v.cx;
  stream >> v.cy;
  stream >> v.cz;

  return stream;
}

