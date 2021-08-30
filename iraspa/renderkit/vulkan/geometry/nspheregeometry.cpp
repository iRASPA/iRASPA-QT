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

#include "nspheregeometry.h"
#include "octahedrongeometry.h"

NSphereGeometry::NSphereGeometry(double r, float4 color, int iterations)
{
  const OctahedronGeometry octahedron = OctahedronGeometry(1.0, color);
  const std::vector<short> octahedronIndices = octahedron.indices();
  const std::vector<RKPrimitiveVertex> octahedronVertices = octahedron.vertices();

  std::transform(octahedronIndices.begin(), octahedronIndices.end(),  std::back_inserter(_vertices),
                  [octahedronVertices](short index) -> RKPrimitiveVertex
                    {return octahedronVertices[index];});

  for(int l=0;l<iterations;l++)
  {
    int currentNumberOfTriangles = _vertices.size() / 3;
    for(int i=0;i<currentNumberOfTriangles;i++)
    {
      float3 pos1 = float3(0.5 * (_vertices[3*i].position.x + _vertices[3*i+1].position.x),
                           0.5 * (_vertices[3*i].position.y + _vertices[3*i+1].position.y),
                           0.5 * (_vertices[3*i].position.z + _vertices[3*i+1].position.z)).normalise();

      float3 pos2 = float3(0.5 * (_vertices[3*i+1].position.x + _vertices[3*i+2].position.x),
                           0.5 * (_vertices[3*i+1].position.y + _vertices[3*i+2].position.y),
                           0.5 * (_vertices[3*i+1].position.z + _vertices[3*i+2].position.z)).normalise();

      float3 pos3 = float3(0.5 * (_vertices[3*i+2].position.x + _vertices[3*i].position.x),
                           0.5 * (_vertices[3*i+2].position.y + _vertices[3*i].position.y),
                           0.5 * (_vertices[3*i+2].position.z + _vertices[3*i].position.z)).normalise();

      _vertices.push_back(_vertices[3*i]);
      _vertices.push_back(RKPrimitiveVertex(float4(pos1.x,pos1.y,pos1.z,0.0),float4(pos1.x, pos1.y, pos1.z, 0.0), color, float2()));
      _vertices.push_back(RKPrimitiveVertex(float4(pos3.x,pos3.y,pos3.z,0.0), float4(pos3.x, pos3.y, pos3.z, 0.0), color, float2()));

      _vertices.push_back(RKPrimitiveVertex(float4(pos1.x,pos1.y,pos1.z,0.0), float4(pos1.x, pos1.y, pos1.z, 0.0), color, float2()));
      _vertices.push_back(_vertices[3*i+1]);
      _vertices.push_back(RKPrimitiveVertex(float4(pos2.x, pos2.y, pos2.z, 0.0), float4(pos2.x, pos2.y, pos2.z, 0.0), color, float2()));

      _vertices.push_back(RKPrimitiveVertex(float4(pos2.x, pos2.y, pos2.z, 0.0), float4(pos2.x, pos2.y, pos2.z, 0.0), color, float2()));
      _vertices.push_back(_vertices[3*i+2]);
      _vertices.push_back(RKPrimitiveVertex(float4(pos3.x, pos3.y, pos3.z, 0.0), float4(pos3.x, pos3.y, pos3.z, 0.0), color, float2()));

      _vertices[3*i] = RKPrimitiveVertex(float4(pos1.x, pos1.y, pos1.z, 0.0), float4(pos1.x, pos1.y, pos1.z, 0.0), color, float2());
      _vertices[3*i+1] = RKPrimitiveVertex(float4(pos2.x, pos2.y, pos2.z, 0.0), float4(pos2.x, pos2.y, pos2.z, 0.0), color, float2());
      _vertices[3*i+2] = RKPrimitiveVertex(float4(pos3.x, pos3.y, pos3.z, 0.0), float4(pos3.x, pos3.y, pos3.z, 0.0), color, float2());
     }
  }

  for(size_t i=0;i<_vertices.size();i++)
  {
    _indices.push_back(i);
  }

  for(size_t i=0;i<_vertices.size();i++)
  {
    _vertices[i].position.x = r;
    _vertices[i].position.y = r;
    _vertices[i].position.x = r;
  }

  _numberOfVertices  = _vertices.size();
  _numberOfIndices = _indices.size();
}
