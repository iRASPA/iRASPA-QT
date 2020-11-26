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
#include "cappedcylindersinglebondgeometry.h"

CappedCylinderSingleBondGeometry::CappedCylinderSingleBondGeometry(double r, int s)
{
  _slices = s;
  _numberOfVertexes = 4 * _slices + 2;
  _numberOfIndices = 12 * _slices;

  _vertexes = std::vector<RKVertex>();
  _indices = std::vector<short>();

  _vertexes.reserve(_numberOfVertexes);
  _vertexes.resize(_numberOfVertexes);
  _indices.reserve(_numberOfIndices);
  _indices.resize(_numberOfIndices);

  double delta = 2.0 * M_PI / double(_slices);

  int index = 0;
  for(int i=0;i<_slices;i++)
  {
    double cosTheta = cos(delta * double(i));
    double sinTheta = sin(delta * double(i));

    float4 position1 = float4(float(r * cosTheta), 0.0, float(r * sinTheta), 0.0);
    float4 normal1 = float4(float(cosTheta), 0.0, float(sinTheta), 0.0);
    _vertexes[index] = RKVertex(position1, normal1, float2());
    index += 1;

    float4 normal2 = float4(float(cosTheta), 0.0, float(sinTheta), 0.0);
    float4 position2 = float4(float(r * cosTheta), 1.0, float(r * sinTheta), 0.0);
    _vertexes[index] = RKVertex(position2, normal2, float2());
    index += 1;
  }

  // first cap
  // ==========================================================================

  float4 position_cap1 = float4(0.0, 0.0, 0.0, 0.0);
  float4 normal_cap1 = float4(0.0, -1.0, 0.0, 0.0);
  _vertexes[index] = RKVertex(position_cap1, normal_cap1, float2());
  int ref_cap_1 = index;
  index += 1;

  for(int i=0;i<_slices;i++)
  {
    double cosTheta = r * cos(delta * double(i));
    double sinTheta = r * sin(delta * double(i));
    float4 position_cap1 = float4(float(cosTheta), 0.0, float(sinTheta), 0.0);
    float4 normal_cap1 = float4(0.0, -1.0, 0.0, 0.0);
    _vertexes[index] = RKVertex(position_cap1, normal_cap1, float2());
    index += 1;
  }

  // second cap
  // ==========================================================================

  float4 position_cap2 = float4(0.0, 1.0, 0.0, 0.0);
  float4 normal_cap2 = float4(0.0, 1.0, 0.0, 0.0);
  _vertexes[index] = RKVertex(position_cap2, normal_cap2, float2());
  int ref_cap_2 = index;
  index += 1;

  for(int i=0;i<_slices;i++)
  {
    double cosTheta = r * cos(delta * double(i));
    double sinTheta = r * sin(delta * double(i));
    float4 position_cap2 = float4(float(cosTheta), 1.0, float(sinTheta), 0.0);
    float4 normal_cap2 = float4(0.0, 1.0, 0.0, 0.0);
    _vertexes[index] = RKVertex(position_cap2, normal_cap2, float2());
    index += 1;
  }

  index = 0;
  for(int i=0;i<_slices;i++)
  {
    _indices[index]=short((2 * i) % (2 * _slices));
    index += 1;
    _indices[index]=short((2 * i + 1) % (2 * _slices));
    index += 1;
    _indices[index]=short((2 * i + 2) % (2 * _slices));
    index += 1;
    _indices[index]=short((2 * i + 2) % (2 * _slices));
    index += 1;
    _indices[index]=short((2 * i + 1) % (2 * _slices));
    index += 1;
    _indices[index]=short((2 * i + 3) % (2 * _slices));
    index += 1;
  }

  // first cap
  // ==========================================================================

  for(int i=0;i<_slices;i++)
  {
    _indices[index]=short(ref_cap_1);
    index += 1;
    _indices[index]=short(ref_cap_1 + 1 + ((i) % _slices));
    index += 1;
    _indices[index]=short(ref_cap_1 + 1 + ((i + 1) % _slices));
    index += 1;
  }

  // second cap
  // ==========================================================================

  for(int i=0;i<_slices;i++)
  {
    _indices[index]=short(ref_cap_2);
    index += 1;
    _indices[index]=short(ref_cap_2 + 1 + ((i + 1) % _slices));
    index += 1;
    _indices[index]=short(ref_cap_2 + 1 + ((i) % _slices));
    index += 1;
  }
}
