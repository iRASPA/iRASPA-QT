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

#include <cmath>
#include "cylindergeometry.h"

CylinderGeometry::CylinderGeometry(double r, int s)
{
  _slices = s;
  _numberOfVertexes = 2 * _slices;
  _numberOfIndices = 2 * _slices + 2;

  _vertexes = std::vector<RKVertex>();
  _indices = std::vector<short>();
  _vertexes.reserve(_numberOfVertexes);
  _indices.reserve(_numberOfIndices);

  double delta = 2.0 * M_PI / double(_slices);

  for(int i=0;i<_slices;i++)
  {
    double cosTheta = cos(delta * double(i));
    double sinTheta = sin(delta * double(i));

    float4 normal1 = float4(float(cosTheta), 0.0, float(sinTheta), 0.0);
    float4 position1 = float4(float(r * cosTheta), 0.0, float(r * sinTheta), 0.0);
    _vertexes.push_back(RKVertex(position1, normal1, float2()));
    _indices.push_back(short(2*i));

    float4 normal2 = float4(float(cosTheta), 0.0, float(sinTheta), 0.0);
    float4 position2 = float4(float(r * cosTheta), 1.0, float(r * sinTheta), 0.0);
    _vertexes.push_back(RKVertex(position2, normal2, float2()));
    _indices.push_back(short(2*i+1));
  }
  _indices.push_back(short(0));
  _indices.push_back(short(1));
}
