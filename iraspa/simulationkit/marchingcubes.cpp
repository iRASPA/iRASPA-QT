/**
 * @file    MarchingCubes.cpp
 * @author  Thomas Lewiner <thomas.lewiner@polytechnique.org>
 * @author  Math Dept, PUC-Rio
 * @version 0.2
 * @date    12/08/2002
 *
 * @brief   MarchingCubes Algorithm with modifications by Samuel H. Wilks <sw463@cam.ac.uk>
 */
//________________________________________________


#include <math.h>
#include <time.h>
#include <memory.h>
#include <stdlib.h>
#include <float.h>
#include <cmath>
#include <limits>
#include <iostream>
#include "marchingcubes.h"
#include "lookuptable.h"


//_____________________________________________________________________________
// print cube for debug
void print_cube(float *cube)
{
  //Rcpp::Rcout << "\t";
  //  for(int i=0; i < 8; ++i) {
  //    Rcpp::Rcout << cube[i] << " ";
  //  }
  //  Rcpp::Rcout << "\n";
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// Constructor
MarchingCubes::MarchingCubes( const int size_x /*= -1*/, const int size_y /*= -1*/, const int size_z /*= -1*/ ) :
//-----------------------------------------------------------------------------
  _originalMC(false),
  _size_x(size_x),
  _size_y(size_y),
  _size_z(size_z)
{}
//_____________________________________________________________________________



//_____________________________________________________________________________
// main algorithm
void MarchingCubes::run( real iso )
//-----------------------------------------------------------------------------
{

  compute_intersection_points( iso ) ;

  for( _k = 0 ; _k < _size_z-1 ; _k++ )
  for( _j = 0 ; _j < _size_y-1 ; _j++ )
  for( _i = 0 ; _i < _size_x-1 ; _i++ )
  {
        float cube[8];
    _lut_entry = 0 ;
    for( int p = 0 ; p < 8 ; ++p )
    {
            cube[p] = get_data(int3{_i+((p^(p>>1))&1), _j+((p>>1)&1), _k+((p>>2)&1)}) - iso ;
            if( std::abs( cube[p] ) < std::numeric_limits<float>::epsilon() ) {
                cube[p] = std::numeric_limits<float>::epsilon() ;
            }
            if( cube[p] > 0 ) _lut_entry += 1 << p ;
    }

    process_cube(cube) ;
  }

}
//_____________________________________________________________________________



//_____________________________________________________________________________
// init temporary structures (must set sizes before call)
void MarchingCubes::init_temps()
//-----------------------------------------------------------------------------
{
    _data.resize(_size_x * _size_y * _size_z);
    _x_verts.resize(_size_x * _size_y * _size_z);
    _y_verts.resize(_size_x * _size_y * _size_z);
    _z_verts.resize(_size_x * _size_y * _size_z);

    memset( _x_verts.data(), -1, _x_verts.size() * sizeof( int ) ) ;
  memset( _y_verts.data(), -1, _y_verts.size() * sizeof( int ) ) ;
  memset( _z_verts.data(), -1, _z_verts.size() * sizeof( int ) ) ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// init all structures (must set sizes before call)
void MarchingCubes::init_all ()
//-----------------------------------------------------------------------------
{
  init_temps();
}
//_____________________________________________________________________________



//_____________________________________________________________________________
//_____________________________________________________________________________


//_____________________________________________________________________________
// Compute the intersection points
void MarchingCubes::compute_intersection_points( real iso )
//-----------------------------------------------------------------------------
{
    for(_k=0; _k < _size_z; ++_k) {
        for(_j=0; _j < _size_y; ++_j) {
            for(_i=0; _i < _size_x; ++_i) {

                auto grid_coord = int3{_i, _j, _k};

                float cube[8];
                cube[0] = get_data(grid_coord) - iso ;
                if( _i < _size_x - 1 ) cube[1] = get_data(int3{_i+1, _j, _k}) - iso ;
                else                   cube[1] = cube[0] ;

                if( _j < _size_y - 1 ) cube[3] = get_data(int3{_i, _j+1, _k}) - iso ;
                else                   cube[3] = cube[0] ;

                if( _k < _size_z - 1 ) cube[4] = get_data(int3{_i, _j ,_k+1}) - iso ;
                else                   cube[4] = cube[0] ;

                if( std::abs( cube[0] ) < std::numeric_limits<float>::epsilon() ) cube[0] = std::numeric_limits<float>::epsilon() ;
                if( std::abs( cube[1] ) < std::numeric_limits<float>::epsilon() ) cube[1] = std::numeric_limits<float>::epsilon() ;
                if( std::abs( cube[3] ) < std::numeric_limits<float>::epsilon() ) cube[3] = std::numeric_limits<float>::epsilon() ;
                if( std::abs( cube[4] ) < std::numeric_limits<float>::epsilon() ) cube[4] = std::numeric_limits<float>::epsilon() ;

                if( cube[0] < 0 )
                {
                    if( cube[1] > 0 ) set_x_vert( add_vertex(grid_coord, int3{1, 0, 0}, 1, cube), _i,_j,_k ) ;
                    if( cube[3] > 0 ) set_y_vert( add_vertex(grid_coord, int3{0, 1, 0}, 3, cube), _i,_j,_k ) ;
                    if( cube[4] > 0 ) set_z_vert( add_vertex(grid_coord, int3{0, 0, 1}, 4, cube), _i,_j,_k ) ;
                }
                else
                {
                    if( cube[1] < 0 ) set_x_vert( add_vertex(grid_coord, int3{1, 0, 0}, 1, cube), _i,_j,_k ) ;
                    if( cube[3] < 0 ) set_y_vert( add_vertex(grid_coord, int3{0, 1, 0}, 3, cube), _i,_j,_k ) ;
                    if( cube[4] < 0 ) set_z_vert( add_vertex(grid_coord, int3{0, 0, 1}, 4, cube), _i,_j,_k ) ;
                }
            }
        }
    }
}
//_____________________________________________________________________________





//_____________________________________________________________________________
// tests if the components of the tesselation of the cube should be connected by the interior of an ambiguous face
// Test a face
// if face>0 return true if the face contains a part of the surface
bool test_face( schar face, float *cube ) {
  static int corner_lookup[6][4] = {
        {0, 4, 5, 1},
        {1, 5, 6, 2},
        {2, 6, 7, 3},
        {3, 7, 4, 0},
        {0, 3, 2, 1},
        {4, 7, 6, 5}
    };

    int idx = std::abs(face) - 1;
    auto corners = corner_lookup[idx];
    auto A = cube[corners[0]];
    auto B = cube[corners[1]];
    auto C = cube[corners[2]];
    auto D = cube[corners[3]];

    if(std::abs(A * C - B * D) < std::numeric_limits<float>::epsilon()) {
    return face >= 0 ;
    }

    // face and A invert signs
  return face * A * ( A * C - B * D ) >= 0.f ;
}





//_____________________________________________________________________________
// Test the interior of a cube
// if s == 7, return true  if the interior is empty
// if s ==-7, return false if the interior is empty
bool MarchingCubes::test_interior( schar s, float *cube )
//-----------------------------------------------------------------------------
{
  real t, At=0, Bt=0, Ct=0, Dt=0, a, b ;
  char  test =  0 ;
  char  edge = -1 ; // reference edge of the triangulation

  switch( _case )
  {
  case  4 :
  case 10 :
    a = ( cube[4] - cube[0] ) * ( cube[6] - cube[2] ) - ( cube[7] - cube[3] ) * ( cube[5] - cube[1] ) ;
    b =  cube[2] * ( cube[4] - cube[0] ) + cube[0] * ( cube[6] - cube[2] )
             - cube[1] * ( cube[7] - cube[3] ) - cube[3] * ( cube[5] - cube[1] ) ;
    t = - b / (2*a) ;
    if( t<0 || t>1 ) return s>0 ;

    At = cube[0] + ( cube[4] - cube[0] ) * t ;
    Bt = cube[3] + ( cube[7] - cube[3] ) * t ;
    Ct = cube[2] + ( cube[6] - cube[2] ) * t ;
    Dt = cube[1] + ( cube[5] - cube[1] ) * t ;
    break ;

  case  6 :
  case  7 :
  case 12 :
  case 13 :
    switch( _case )
    {
    case  6 : edge = test6 [_config][2] ; break ;
    case  7 : edge = test7 [_config][4] ; break ;
    case 12 : edge = test12[_config][3] ; break ;
    case 13 : edge = tiling13_5_1[_config][_subconfig][0] ; break ;
    }
    switch( edge )
    {
    case  0 :
      t  = cube[0] / ( cube[0] - cube[1] ) ;
      At = 0 ;
      Bt = cube[3] + ( cube[2] - cube[3] ) * t ;
      Ct = cube[7] + ( cube[6] - cube[7] ) * t ;
      Dt = cube[4] + ( cube[5] - cube[4] ) * t ;
      break ;
    case  1 :
      t  = cube[1] / ( cube[1] - cube[2] ) ;
      At = 0 ;
      Bt = cube[0] + ( cube[3] - cube[0] ) * t ;
      Ct = cube[4] + ( cube[7] - cube[4] ) * t ;
      Dt = cube[5] + ( cube[6] - cube[5] ) * t ;
      break ;
    case  2 :
      t  = cube[2] / ( cube[2] - cube[3] ) ;
      At = 0 ;
      Bt = cube[1] + ( cube[0] - cube[1] ) * t ;
      Ct = cube[5] + ( cube[4] - cube[5] ) * t ;
      Dt = cube[6] + ( cube[7] - cube[6] ) * t ;
      break ;
    case  3 :
      t  = cube[3] / ( cube[3] - cube[0] ) ;
      At = 0 ;
      Bt = cube[2] + ( cube[1] - cube[2] ) * t ;
      Ct = cube[6] + ( cube[5] - cube[6] ) * t ;
      Dt = cube[7] + ( cube[4] - cube[7] ) * t ;
      break ;
    case  4 :
      t  = cube[4] / ( cube[4] - cube[5] ) ;
      At = 0 ;
      Bt = cube[7] + ( cube[6] - cube[7] ) * t ;
      Ct = cube[3] + ( cube[2] - cube[3] ) * t ;
      Dt = cube[0] + ( cube[1] - cube[0] ) * t ;
      break ;
    case  5 :
      t  = cube[5] / ( cube[5] - cube[6] ) ;
      At = 0 ;
      Bt = cube[4] + ( cube[7] - cube[4] ) * t ;
      Ct = cube[0] + ( cube[3] - cube[0] ) * t ;
      Dt = cube[1] + ( cube[2] - cube[1] ) * t ;
      break ;
    case  6 :
      t  = cube[6] / ( cube[6] - cube[7] ) ;
      At = 0 ;
      Bt = cube[5] + ( cube[4] - cube[5] ) * t ;
      Ct = cube[1] + ( cube[0] - cube[1] ) * t ;
      Dt = cube[2] + ( cube[3] - cube[2] ) * t ;
      break ;
    case  7 :
      t  = cube[7] / ( cube[7] - cube[4] ) ;
      At = 0 ;
      Bt = cube[6] + ( cube[5] - cube[6] ) * t ;
      Ct = cube[2] + ( cube[1] - cube[2] ) * t ;
      Dt = cube[3] + ( cube[0] - cube[3] ) * t ;
      break ;
    case  8 :
      t  = cube[0] / ( cube[0] - cube[4] ) ;
      At = 0 ;
      Bt = cube[3] + ( cube[7] - cube[3] ) * t ;
      Ct = cube[2] + ( cube[6] - cube[2] ) * t ;
      Dt = cube[1] + ( cube[5] - cube[1] ) * t ;
      break ;
    case  9 :
      t  = cube[1] / ( cube[1] - cube[5] ) ;
      At = 0 ;
      Bt = cube[0] + ( cube[4] - cube[0] ) * t ;
      Ct = cube[3] + ( cube[7] - cube[3] ) * t ;
      Dt = cube[2] + ( cube[6] - cube[2] ) * t ;
      break ;
    case 10 :
      t  = cube[2] / ( cube[2] - cube[6] ) ;
      At = 0 ;
      Bt = cube[1] + ( cube[5] - cube[1] ) * t ;
      Ct = cube[0] + ( cube[4] - cube[0] ) * t ;
      Dt = cube[3] + ( cube[7] - cube[3] ) * t ;
      break ;
    case 11 :
      t  = cube[3] / ( cube[3] - cube[7] ) ;
      At = 0 ;
      Bt = cube[2] + ( cube[6] - cube[2] ) * t ;
      Ct = cube[1] + ( cube[5] - cube[1] ) * t ;
      Dt = cube[0] + ( cube[4] - cube[0] ) * t ;
      break ;
    default : std::cout << " Invalid edge " << edge << "\n";  print_cube(cube) ;  break ;
    }
    break ;

  default : std::cout << " Invalid ambiguous case " << _case << "\n";  print_cube(cube) ;  break ;
  }

  if( At >= 0 ) test ++ ;
  if( Bt >= 0 ) test += 2 ;
  if( Ct >= 0 ) test += 4 ;
  if( Dt >= 0 ) test += 8 ;
  switch( test )
  {
  case  0 : return s>0 ;
  case  1 : return s>0 ;
  case  2 : return s>0 ;
  case  3 : return s>0 ;
  case  4 : return s>0 ;
  case  5 : if( At * Ct - Bt * Dt <  std::numeric_limits<float>::epsilon() ) return s>0 ; break ;
  case  6 : return s>0 ;
  case  7 : return s<0 ;
  case  8 : return s>0 ;
  case  9 : return s>0 ;
  case 10 : if( At * Ct - Bt * Dt >= std::numeric_limits<float>::epsilon() ) return s>0 ; break ;
  case 11 : return s<0 ;
  case 12 : return s>0 ;
  case 13 : return s<0 ;
  case 14 : return s<0 ;
  case 15 : return s<0 ;
  }

  return s < 0;
}
//_____________________________________________________________________________




//_____________________________________________________________________________
// Process a unit cube
void MarchingCubes::process_cube(float *cube)
//-----------------------------------------------------------------------------
{
  if( _originalMC )
  {
    char nt = 0 ;
    while( casesClassic[_lut_entry][3*nt] != -1 ) nt++ ;
    add_triangle( casesClassic[_lut_entry], nt ) ;
    return ;
  }

  int   v12 = -1 ;
  _case   = cases[_lut_entry][0] ;
  _config = cases[_lut_entry][1] ;
  _subconfig = 0 ;

  switch( _case )
  {
  case  0 :
    break ;

  case  1 :
    add_triangle( tiling1[_config], 1 ) ;
    break ;

  case  2 :
    add_triangle( tiling2[_config], 2 ) ;
    break ;

  case  3 :
    if( test_face( test3[_config], cube) )
      add_triangle( tiling3_2[_config], 4 ) ; // 3.2
    else
      add_triangle( tiling3_1[_config], 2 ) ; // 3.1
    break ;

  case  4 :
    if( test_interior(test4[_config], cube))
      add_triangle( tiling4_1[_config], 2 ) ; // 4.1.1
    else
      add_triangle( tiling4_2[_config], 6 ) ; // 4.1.2
    break ;

  case  5 :
    add_triangle( tiling5[_config], 3 ) ;
    break ;

  case  6 :
    if( test_face( test6[_config][0], cube) )
      add_triangle( tiling6_2[_config], 5 ) ; // 6.2
    else
    {
      if( test_interior( test6[_config][1], cube) )
        add_triangle( tiling6_1_1[_config], 3 ) ; // 6.1.1
      else
      {
        v12 = add_c_vertex() ;
        add_triangle( tiling6_1_2[_config], 9 , v12) ; // 6.1.2
      }
    }
    break ;

  case  7 :
    if( test_face( test7[_config][0], cube ) ) _subconfig +=  1 ;
    if( test_face( test7[_config][1], cube ) ) _subconfig +=  2 ;
    if( test_face( test7[_config][2], cube ) ) _subconfig +=  4 ;
    switch( _subconfig )
      {
      case 0 :
        add_triangle( tiling7_1[_config], 3 ) ; break ;
      case 1 :
        add_triangle( tiling7_2[_config][0], 5 ) ; break ;
      case 2 :
        add_triangle( tiling7_2[_config][1], 5 ) ; break ;
      case 3 :
        v12 = add_c_vertex() ;
        add_triangle( tiling7_3[_config][0], 9, v12 ) ; break ;
      case 4 :
        add_triangle( tiling7_2[_config][2], 5 ) ; break ;
      case 5 :
        v12 = add_c_vertex() ;
        add_triangle( tiling7_3[_config][1], 9, v12 ) ; break ;
      case 6 :
        v12 = add_c_vertex() ;
        add_triangle( tiling7_3[_config][2], 9, v12 ) ; break ;
      case 7 :
        if( test_interior( test7[_config][3], cube) )
          add_triangle( tiling7_4_2[_config], 9 ) ;
        else
          add_triangle( tiling7_4_1[_config], 5 ) ;
        break ;
      };
    break ;

  case  8 :
    add_triangle( tiling8[_config], 2 ) ;
    break ;

  case  9 :
    add_triangle( tiling9[_config], 4 ) ;
    break ;

  case 10 :
    if( test_face( test10[_config][0], cube) )
    {
      if( test_face( test10[_config][1], cube) )
        add_triangle( tiling10_1_1_[_config], 4 ) ; // 10.1.1
      else
      {
        v12 = add_c_vertex() ;
        add_triangle( tiling10_2[_config], 8, v12 ) ; // 10.2
      }
    }
    else
    {
      if( test_face( test10[_config][1], cube) )
      {
        v12 = add_c_vertex() ;
        add_triangle( tiling10_2_[_config], 8, v12 ) ; // 10.2
      }
      else
      {
        if( test_interior( test10[_config][2], cube) )
          add_triangle( tiling10_1_1[_config], 4 ) ; // 10.1.1
        else
          add_triangle( tiling10_1_2[_config], 8 ) ; // 10.1.2
      }
    }
    break ;

  case 11 :
    add_triangle( tiling11[_config], 4 ) ;
    break ;

  case 12 :
    if( test_face( test12[_config][0], cube) )
    {
      if( test_face( test12[_config][1], cube) )
        add_triangle( tiling12_1_1_[_config], 4 ) ; // 12.1.1
      else
      {
        v12 = add_c_vertex() ;
        add_triangle( tiling12_2[_config], 8, v12 ) ; // 12.2
      }
    }
    else
    {
      if( test_face( test12[_config][1], cube) )
      {
        v12 = add_c_vertex() ;
        add_triangle( tiling12_2_[_config], 8, v12 ) ; // 12.2
      }
      else
      {
        if( test_interior( test12[_config][2], cube) )
          add_triangle( tiling12_1_1[_config], 4 ) ; // 12.1.1
        else
          add_triangle( tiling12_1_2[_config], 8 ) ; // 12.1.2
      }
    }
    break ;

  case 13 :
    if( test_face( test13[_config][0], cube ) ) _subconfig +=  1 ;
    if( test_face( test13[_config][1], cube ) ) _subconfig +=  2 ;
    if( test_face( test13[_config][2], cube ) ) _subconfig +=  4 ;
    if( test_face( test13[_config][3], cube ) ) _subconfig +=  8 ;
    if( test_face( test13[_config][4], cube ) ) _subconfig += 16 ;
    if( test_face( test13[_config][5], cube ) ) _subconfig += 32 ;
    switch( subconfig13[_subconfig] )
    {
      case 0 :/* 13.1 */
        add_triangle( tiling13_1[_config], 4 ) ; break ;

      case 1 :/* 13.2 */
        add_triangle( tiling13_2[_config][0], 6 ) ; break ;
      case 2 :/* 13.2 */
        add_triangle( tiling13_2[_config][1], 6 ) ; break ;
      case 3 :/* 13.2 */
        add_triangle( tiling13_2[_config][2], 6 ) ; break ;
      case 4 :/* 13.2 */
        add_triangle( tiling13_2[_config][3], 6 ) ; break ;
      case 5 :/* 13.2 */
        add_triangle( tiling13_2[_config][4], 6 ) ; break ;
      case 6 :/* 13.2 */
        add_triangle( tiling13_2[_config][5], 6 ) ; break ;

      case 7 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][0], 10, v12 ) ; break ;
      case 8 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][1], 10, v12 ) ; break ;
      case 9 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][2], 10, v12 ) ; break ;
      case 10 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][3], 10, v12 ) ; break ;
      case 11 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][4], 10, v12 ) ; break ;
      case 12 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][5], 10, v12 ) ; break ;
      case 13 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][6], 10, v12 ) ; break ;
      case 14 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][7], 10, v12 ) ; break ;
      case 15 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][8], 10, v12 ) ; break ;
      case 16 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][9], 10, v12 ) ; break ;
      case 17 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][10], 10, v12 ) ; break ;
      case 18 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3[_config][11], 10, v12 ) ; break ;

      case 19 :/* 13.4 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_4[_config][0], 12, v12 ) ; break ;
      case 20 :/* 13.4 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_4[_config][1], 12, v12 ) ; break ;
      case 21 :/* 13.4 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_4[_config][2], 12, v12 ) ; break ;
      case 22 :/* 13.4 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_4[_config][3], 12, v12 ) ; break ;

      case 23 :/* 13.5 */
        _subconfig = 0 ;
        if( test_interior( test13[_config][6], cube ) )
          add_triangle( tiling13_5_1[_config][0], 6 ) ;
        else
          add_triangle( tiling13_5_2[_config][0], 10 ) ;
        break ;
      case 24 :/* 13.5 */
        _subconfig = 1 ;
        if( test_interior( test13[_config][6], cube ) )
          add_triangle( tiling13_5_1[_config][1], 6 ) ;
        else
          add_triangle( tiling13_5_2[_config][1], 10 ) ;
        break ;
      case 25 :/* 13.5 */
        _subconfig = 2 ;
        if( test_interior( test13[_config][6], cube ) )
          add_triangle( tiling13_5_1[_config][2], 6 ) ;
        else
          add_triangle( tiling13_5_2[_config][2], 10 ) ;
        break ;
      case 26 :/* 13.5 */
        _subconfig = 3 ;
        if( test_interior( test13[_config][6], cube ) )
          add_triangle( tiling13_5_1[_config][3], 6 ) ;
        else
          add_triangle( tiling13_5_2[_config][3], 10 ) ;
        break ;

      case 27 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][0], 10, v12 ) ; break ;
      case 28 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][1], 10, v12 ) ; break ;
      case 29 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][2], 10, v12 ) ; break ;
      case 30 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][3], 10, v12 ) ; break ;
      case 31 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][4], 10, v12 ) ; break ;
      case 32 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][5], 10, v12 ) ; break ;
      case 33 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][6], 10, v12 ) ; break ;
      case 34 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][7], 10, v12 ) ; break ;
      case 35 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][8], 10, v12 ) ; break ;
      case 36 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][9], 10, v12 ) ; break ;
      case 37 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][10], 10, v12 ) ; break ;
      case 38 :/* 13.3 */
        v12 = add_c_vertex() ;
        add_triangle( tiling13_3_[_config][11], 10, v12 ) ; break ;

      case 39 :/* 13.2 */
        add_triangle( tiling13_2_[_config][0], 6 ) ; break ;
      case 40 :/* 13.2 */
        add_triangle( tiling13_2_[_config][1], 6 ) ; break ;
      case 41 :/* 13.2 */
        add_triangle( tiling13_2_[_config][2], 6 ) ; break ;
      case 42 :/* 13.2 */
        add_triangle( tiling13_2_[_config][3], 6 ) ; break ;
      case 43 :/* 13.2 */
        add_triangle( tiling13_2_[_config][4], 6 ) ; break ;
      case 44 :/* 13.2 */
        add_triangle( tiling13_2_[_config][5], 6 ) ; break ;

      case 45 :/* 13.1 */
        add_triangle( tiling13_1_[_config], 4 ) ; break ;

      default :
                std::cout << "Marching Cubes: Impossible case 13?\n";  print_cube(cube) ;
      }
      break ;

  case 14 :
    add_triangle( tiling14[_config], 4 ) ;
    break ;
  };
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// Adding triangles
void MarchingCubes::add_triangle( const char* trig, char n, int v12 ) {
    int i = 0;
    while(i < 3 * n) {
        int tv[3];

        for(int t=0; t < 3; ++t, ++i) {
            switch(trig[i]) {
                case  0 : tv[t] = get_x_vert( _i , _j , _k ) ; break ;
                case  1 : tv[t] = get_y_vert(_i+1, _j , _k ) ; break ;
                case  2 : tv[t] = get_x_vert( _i ,_j+1, _k ) ; break ;
                case  3 : tv[t] = get_y_vert( _i , _j , _k ) ; break ;
                case  4 : tv[t] = get_x_vert( _i , _j ,_k+1) ; break ;
                case  5 : tv[t] = get_y_vert(_i+1, _j ,_k+1) ; break ;
                case  6 : tv[t] = get_x_vert( _i ,_j+1,_k+1) ; break ;
                case  7 : tv[t] = get_y_vert( _i , _j ,_k+1) ; break ;
                case  8 : tv[t] = get_z_vert( _i , _j , _k ) ; break ;
                case  9 : tv[t] = get_z_vert(_i+1, _j , _k ) ; break ;
                case 10 : tv[t] = get_z_vert(_i+1,_j+1, _k ) ; break ;
                case 11 : tv[t] = get_z_vert( _i ,_j+1, _k ) ; break ;
                case 12 : tv[t] = v12 ; break ;
                default : break ;
            }

            if( tv[t] == -1 ) {
              std::cout << "Marching Cubes: invalid triangle " << (ntrigs() + 1) << "\n";
                //print_cube() ;
            }
        }

        _triangles.push_back(Triangle{tv[0], tv[1], tv[2]});
    }
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// Calculating gradient

real MarchingCubes::get_x_grad( const int i, const int j, const int k ) const
//-----------------------------------------------------------------------------
{
  if(i > 0) {
        if(i < _size_x - 1 ) {
            return ( get_data(int3{i+1, j, k}) - get_data(int3{i-1, j, k })) / 2 ;
        }
        else {
      return get_data(int3{i, j, k}) - get_data(int3{i-1, j, k}) ;
        }
  }
    else {
    return get_data(int3{i+1, j, k}) - get_data(int3{i, j, k}) ;
    }
}
//-----------------------------------------------------------------------------

real MarchingCubes::get_y_grad( const int i, const int j, const int k ) const
//-----------------------------------------------------------------------------
{
  if(j > 0) {
        if (j < _size_y - 1) {
            return ( get_data(int3{i, j+1, k}) - get_data(int3{i, j-1, k}) ) / 2 ;
        }
        else {
      return get_data(int3{i, j, k}) - get_data(int3{i, j-1, k});
        }
  }
    else {
    return get_data(int3{i, j+1, k}) - get_data(int3{i, j, k});
    }
}
//-----------------------------------------------------------------------------

real MarchingCubes::get_z_grad( const int i, const int j, const int k ) const
//-----------------------------------------------------------------------------
{
  if(k > 0) {
        if(k < _size_z - 1) {
      return ( get_data(int3{i, j, k+1}) - get_data(int3{i, j, k-1}) ) / 2 ;
        }
        else {
      return get_data(int3{i, j, k}) - get_data(int3{i, j, k-1}) ;
        }
  }
    else {
    return get_data(int3{i, j, k+1}) - get_data(int3{i, j, k}) ;
    }
}
//_____________________________________________________________________________


//_____________________________________________________________________________
// Adding vertices

int MarchingCubes::add_vertex(const int3 &grid_coord, const int3 &dir, int corner, float *cube) {
    auto u = cube[0] / (cube[0] - cube[corner]);

  double grid_coord_x = grid_coord.x;
  double grid_coord_y = grid_coord.y;
  double grid_coord_z = grid_coord.z;

  double dir_x = dir.x;
  double dir_y = dir.y;
  double dir_z = dir.z;

  double3 pos = double3{ grid_coord_x, grid_coord_y, grid_coord_z } + double3{ dir_x, dir_y, dir_z } * u;

    int3 grid_coord2 = grid_coord + dir;

    int grid_coord2_x = grid_coord2.x;
    int grid_coord2_y = grid_coord2.y;
    int grid_coord2_z = grid_coord2.z;

    auto nx = (1-u)*get_x_grad(grid_coord_x, grid_coord_y, grid_coord_z) + u * get_x_grad(grid_coord2_x, grid_coord2_y, grid_coord2_z);
    auto ny = (1-u)*get_y_grad(grid_coord_x, grid_coord_y, grid_coord_z) + u * get_y_grad(grid_coord2_x, grid_coord2_y, grid_coord2_z);
    auto nz = (1-u)*get_z_grad(grid_coord_x, grid_coord_y, grid_coord_z) + u * get_z_grad(grid_coord2_x, grid_coord2_y, grid_coord2_z);

    //double3 n = arma::normalise(double3{nx, ny, nz});
    double3 n = double3{nx, ny, nz}.normalise();

    float pos_x = pos.x;
    float pos_y = pos.y;
    float pos_z = pos.z;

    float n_x = n.x;
    float n_y = n.y;
    float n_z = n.z;

    _vertices.push_back(Vertex{pos_x, pos_y, pos_z, n_x, n_y, n_z});
    return _vertices.size() - 1;
}

int MarchingCubes::add_c_vertex()
//-----------------------------------------------------------------------------
{
  auto u = float{0.f};
  auto pos = double3(0,0,0);
  auto n = double3(0,0,0);

  // Computes the average of the intersection points of the cube
    // x-face
    for(auto t : {0, 1}) {
        for(auto s : {0, 1}) {
            auto vid = get_x_vert( _i , _j + s , _k + t ) ;
            if( vid != -1 ) {
                ++u ;
                const Vertex &v = _vertices[vid];
                pos += double3{v.x, v.y, v.z};
                n += double3{v.nx, v.ny, v.nz};
            }
        }
    }

    // y-face
    for(auto t : {0, 1}) {
        for(auto s : {0, 1}) {
            auto vid = get_y_vert( _i + t , _j , _k + s ) ;
            if( vid != -1 ) {
                ++u ;
                const Vertex &v = _vertices[vid];
                pos += double3{v.x, v.y, v.z};
                n += double3{v.nx, v.ny, v.nz};
            }
        }
    }

    // z-face
    for(auto t : {0, 1}) {
        for(auto s : {0, 1}) {
            auto vid = get_z_vert( _i + s , _j + t , _k ) ;
            if( vid != -1 ) {
                ++u ;
                const Vertex &v = _vertices[vid];
                pos += double3{v.x, v.y, v.z};
                n += double3{v.nx, v.ny, v.nz};
            }
        }
    }

    pos.x *= 1.0f/u;
    pos.y *= 1.0f/u;
    pos.z *= 1.0f/u;
    //n = arma::normalise(n);
    n = n.normalise();

    float pos_x = pos.x;
    float pos_y = pos.y;
    float pos_z = pos.z;

    float n_x = n.x;
    float n_y = n.y;
    float n_z = n.z;

    _vertices.push_back(Vertex{pos_x, pos_y, pos_z, n_x, n_y, n_z});
  return _vertices.size() - 1;
}
//_____________________________________________________________________________
