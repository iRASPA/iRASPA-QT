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

class UnitCubeGeometry
{
public:
  UnitCubeGeometry();
  const std::vector<RKVertex>& vertices() {return _vertexes;}
  const std::vector<short>& indices() {return _indices;}
private:

  std::vector<short> _indices =
  {
    0,5,4,5,0,1,3,7,6,3,6,2,7,4,6,6,4,5,2,1,3,3,1,0,3,0,7,7,0,4,6,5,2,2,5,1
  };

  std::vector<RKVertex> _vertexes =
  {
    RKVertex(float4(-0.5f,-0.5f,-0.5f, 1.0f), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4( 0.5f,-0.5f,-0.5f, 1.0f), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4( 0.5f, 0.5f,-0.5f, 1.0f), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4(-0.5f, 0.5f,-0.5f, 1.0f), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4(-0.5f,-0.5f, 0.5f, 1.0f), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4( 0.5f,-0.5f, 0.5f, 1.0f), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4( 0.5f, 0.5f, 0.5f, 1.0f), float4(0.0, 0.0, 1.0, 0.0), float2()),
    RKVertex(float4(-0.5f, 0.5f, 0.5f, 1.0f), float4(0.0, 0.0, 1.0, 0.0), float2())
  };
};

