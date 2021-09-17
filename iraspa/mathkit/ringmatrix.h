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
#include <any>
#include "int3x3.h"
#include <variant>

class RingMatrix
{
public:
  RingMatrix(int rows, int columns, int initialValue);
  RingMatrix(int rows, int columns, std::vector<std::vector<int>> data);
  RingMatrix(int3x3 t1,int3x3 t2,int3x3 t3);

  int rows() const {return _rows;}
  int columns() const {return _columns;}

  static RingMatrix createRandomRingMatrix(int rows, int columns, int limit);

  RingMatrix transposed();

  int& operator() (int row, int col) {return _grid[row*_columns + col];}
  int  operator() (int row, int col) const {return _grid[row*_columns + col];}
  static RingMatrix identity(int size);
  static RingMatrix zeros(int rows, int columns);
  static RingMatrix ones(int rows, int columns);

  inline bool operator==(const RingMatrix& b) const
  {
    if(this->_rows != b._rows) return false;
    if(this->_columns != b._columns) return false;

    for(int i=0;i<this->_rows;i++)
    {
      for(int j=0;j<b._columns;j++)
      {
        if((*this)(i,j) != b(i,j)) return false;
      }
    }
    return true;
  }

  std::tuple<RingMatrix,RingMatrix,std::vector<int>> HermiteNormalForm();
  std::tuple<RingMatrix, RingMatrix, RingMatrix> SmithNormalForm();
private:
  int _rows;
  int _columns;
  std::vector<int> _grid;

  const static int HNF_C_Iwaniec;

  void swapColumns(int a, int b);

  RingMatrix submatrix(int startRow, int startColumn, int numberOfRows, int numberOfColumns);
  void assignSubmatrix(int startRow, int startColumn, RingMatrix replacement);
  std::tuple<RingMatrix,RingMatrix,RingMatrix> ColumnReduction(RingMatrix A1, int col_1, int col_2, int row_start);
  std::pair<std::vector<int>,std::vector<int>> Conditioning(RingMatrix A, int col_1, int col_2, int row_start);
  std::variant<std::vector<int>, int> Algorithm_6_14(int a, int b, int N, std::vector<int> Nfact);
  std::vector<int> RemovedDuplicates( std::vector<int> array);

  void Smith_Theorem5(RingMatrix &A, RingMatrix &U, RingMatrix &V, int col);
  void Smith_Theorem8(RingMatrix &A, RingMatrix &U, RingMatrix &V, int row, int r);
  std::vector<int> Algorithm_6_15(int a, std::vector<int> bi, int N);

  friend QDebug operator<<(QDebug debug, const RingMatrix &m);
};

inline RingMatrix operator*(const RingMatrix &a, const RingMatrix &b)
{
  RingMatrix results = RingMatrix(a.rows(), b.columns(), 0);
  for(int i=0;i<a.rows();i++)
  {
    for(int j=0;j<b.columns();j++)
    {
      int v = 0;
      for(int k=0;k<a.columns();k++)
      {
        v += a(i,k) * b(k,j);
      }
      results(i,j) = v;
    }
  }
  return results;
}
