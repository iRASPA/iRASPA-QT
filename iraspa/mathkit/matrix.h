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
#include <vector>
#include <optional>
#include "ringmatrix.h"

class Matrix
{
public:
  Matrix(int rows, int columns, int initialValue);

  int rows() const {return _rows;}
  int columns() const {return _columns;}

  double& operator() (int row, int col) {return _grid[row*_columns + col];}
  double  operator() (int row, int col) const {return _grid[row*_columns + col];}

  friend Matrix operator*(const Matrix &v1, const Matrix &v2);
  friend QDebug operator<<(QDebug debug, const Matrix &m);
private:
  int _rows;
  int _columns;
  std::vector<double> _grid;
};

inline Matrix operator*(const RingMatrix &a, const Matrix &b)
{
  Matrix results = Matrix(a.rows(), b.columns(), 0);
  for(int i=0;i<a.rows();i++)
  {
    for(int j=0;j<b.columns();j++)
    {
      double v = 0;
      for(int k=0;k<a.columns();k++)
      {
        v += a(i,k) * b(k,j);
      }
      results(i,j) = v;
    }
  }
  return results;
}

inline Matrix operator*(const Matrix &a, const RingMatrix &b)
{
  Matrix results = Matrix(a.rows(), b.columns(), 0);
  for(int i=0;i<a.rows();i++)
  {
    for(int j=0;j<b.columns();j++)
    {
      double v = 0;
      for(int k=0;k<a.columns();k++)
      {
        v += a(i,k) * b(k,j);
      }
      results(i,j) = v;
    }
  }
  return results;
}
