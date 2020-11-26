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

#include <vector>
#include "rkrenderuniforms.h"

class CubeGeometry
{
public:
  CubeGeometry();
  std::vector<RKVertex> vertices() {return _vertexes;}
  std::vector<short> indices() {return _indices;}
private:
  int _numberOfIndices = 34;
  int _numberOfVertexes = 24;

  std::vector<short> _indices =
    {
    0,  1,  2,  3,  3,      // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
    4,  4,  5,  6,  7,  7,  // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
    8,  8,  9, 10, 11, 11,  // Face 2 - triangle strip ( v8,  v9, v10, v11)
    12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
    16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
    20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
    };

  std::vector<RKVertex> _vertexes =
  {
    // face 0 (front)
    RKVertex(float4(0.0, 0.0, 1.0, 1.0), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4(1.0, 0.0, 1.0, 1.0), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4(0.0, 1.0, 1.0, 1.0), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4(1.0, 1.0, 1.0, 1.0), float4(0.0, 0.0, 1.0, 0.0), float2()),

    // face 1 (right)
    RKVertex(float4(1.0, 0.0, 1.0, 1.0), float4(1.0, 0.0, 0.0, 0.0), float2()),
    RKVertex(float4(1.0, 0.0, 0.0, 1.0), float4(1.0, 0.0, 0.0, 0.0), float2()),
    RKVertex(float4(1.0, 1.0, 1.0, 1.0), float4(1.0, 0.0, 0.0, 0.0), float2()),
    RKVertex(float4(1.0, 1.0, 0.0, 1.0), float4(1.0, 0.0, 0.0, 0.0), float2()),

    // face 2 (back)
    RKVertex(float4(1.0, 0.0, 0.0, 1.0), float4(0.0, 0.0, -1.0, 0.0), float2()),
    RKVertex(float4(0.0, 0.0, 0.0, 1.0), float4(0.0, 0.0, -1.0, 0.0), float2()),
    RKVertex(float4(1.0, 1.0, 0.0, 1.0), float4(0.0, 0.0, -1.0, 0.0), float2()),
    RKVertex(float4(0.0, 1.0, 0.0, 1.0), float4(0.0, 0.0, -1.0, 0.0), float2()),

    // face 3 (left)
    RKVertex(float4(0.0, 0.0, 0.0, 1.0), float4(-1.0, 0.0, 0.0, 0.0), float2()),
    RKVertex(float4(0.0, 0.0, 1.0, 1.0), float4(-1.0, 0.0, 0.0, 0.0), float2()),
    RKVertex(float4(0.0, 1.0, 0.0, 1.0), float4(-1.0, 0.0, 0.0, 0.0), float2()),
    RKVertex(float4(0.0, 1.0, 1.0, 1.0), float4(-1.0, 0.0, 0.0, 0.0), float2()),

    // face 4 (bottom)
    RKVertex(float4(0.0, 0.0, 0.0, 1.0), float4(0.0, -1.0, 0.0, 0.0), float2()),
    RKVertex(float4(1.0, 0.0, 0.0, 1.0), float4(0.0, -1.0, 0.0, 0.0), float2()),
    RKVertex(float4(0.0, 0.0, 1.0, 1.0), float4(0.0, -1.0, 0.0, 0.0), float2()),
    RKVertex(float4(1.0, 0.0, 1.0, 1.0), float4(0.0, -1.0, 0.0, 0.0), float2()),

    // face 5 (top)
    RKVertex(float4(0.0, 1.0, 1.0, 1.0), float4(0.0, 1.0, 0.0, 0.0), float2()),
    RKVertex(float4(1.0, 1.0, 1.0, 1.0), float4(0.0, 1.0, 0.0, 0.0), float2()),
    RKVertex(float4(0.0, 1.0, 0.0, 1.0), float4(0.0, 1.0, 0.0, 0.0), float2()),
    RKVertex(float4(1.0, 1.0, 0.0, 1.0), float4(0.0, 1.0, 0.0, 0.0), float2())
  };
};

