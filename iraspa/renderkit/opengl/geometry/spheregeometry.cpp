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
#include "spheregeometry.h"

SphereGeometry::SphereGeometry(double r, int s)
{
  double theta, phi, sinTheta, sinPhi, cosTheta, cosPhi;
  float4 position, normal;

  _slices = s;
  _numberOfIndices = 2 * (s + 1) * s;
  _numberOfVertexes = (s + 1) * s;

  _vertexes = std::vector<RKVertex>();
  _indices = std::vector<short>();
  _vertexes.reserve(_numberOfVertexes);
  _vertexes.reserve(_numberOfIndices);

  for(int stackNumber=0;stackNumber<=_slices;stackNumber++)
  {
    for(int sliceNumber=0;sliceNumber<_slices;sliceNumber++)
    {
      theta = double(stackNumber) * M_PI / double(_slices);
      phi = double(sliceNumber) * 2.0 * M_PI / double(_slices);
      sinTheta = sin(theta);
      sinPhi = sin(phi);
      cosTheta = cos(theta);
      cosPhi = cos(phi);
      normal = float4(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta, 0.0);
      position = float4(r * cosPhi * sinTheta, r * sinPhi * sinTheta, r * cosTheta, 0.0);
      _vertexes.push_back(RKVertex(position, normal, float2()));
    }
  }

  for(int stackNumber=0;stackNumber<_slices;stackNumber++)
  {
    for(int sliceNumber=0;sliceNumber<=_slices;sliceNumber++)
    {
      _indices.push_back(short((stackNumber * _slices) + (sliceNumber % _slices)));
      _indices.push_back(short(((stackNumber + 1) * _slices) + (sliceNumber % _slices)));
    }
  }
}
