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

#include "octahedrongeometry.h"

OctahedronGeometry::OctahedronGeometry(double r, float4 color)
{
  _vertices = std::vector<RKPrimitiveVertex>{
      RKPrimitiveVertex(float4(0.0, 0.0, r, 0.0), float4(0.0, 0.0, 1.0, 0.0), color, float2()),
      RKPrimitiveVertex(float4(0.0, 0.0, -r, 0.0), float4(0.0, 0.0, -1.0, 0.0), color, float2()),
      RKPrimitiveVertex(float4(r / sqrt(2.0), -r / sqrt(2.0), 0.0, 0.0), float4(1.0 / sqrt(2.0), -1.0 / sqrt(2.0), 0.0, 0.0),  color, float2()),
      RKPrimitiveVertex(float4(-r / sqrt(2.0), -r / sqrt(2.0), 0.0, 0.0), float4(-1.0 / sqrt(2.0), -1.0 / sqrt(2.0), 0.0, 0.0),  color, float2()),
      RKPrimitiveVertex(float4(-r / sqrt(2.0), r / sqrt(2.0), 0.0, 0.0), float4(-1.0 / sqrt(2.0), 1.0 / sqrt(2.0), 0.0, 0.0),  color, float2()),
      RKPrimitiveVertex(float4(r / sqrt(2.0), r / sqrt(2.0), 0.0, 0.0), float4(1.0 / sqrt(2.0), 1.0 / sqrt(2.0), 0.0, 0.0),  color, float2())
    };
}
