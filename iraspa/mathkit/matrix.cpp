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

#include "matrix.h"

Matrix::Matrix(int rows, int columns, int initialValue): _rows(rows), _columns(columns), _grid(rows * columns)
{
  std::fill(_grid.begin(), _grid.end(), initialValue);
}

Matrix operator*(const Matrix &lhs, const Matrix &rhs)
{
  Matrix results = Matrix(lhs._rows, rhs._columns, 0);
  for(int i=0;i<lhs._rows;i++)
  {
    for(int j=0;j<rhs._columns;j++)
    {
      double v = 0;
      for(int k=0;k<lhs.columns();k++)
      {
        v += lhs(i,k) * rhs(k,j);
      }
      results(i,j) = v;
    }
  }
  return results;
}

QDebug operator<<(QDebug debug, const Matrix &m)
{
  debug << "\n{\n";
  for(int i=0;i<m._rows;i++)
  {
    for(int j=0;j<m._columns;j++)
    {
      debug << m(i,j) << " ";
    }
    debug << "\n";
  }
  debug << "}";
  return debug;
}
