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

#include "cappedcylindertriplebondgeometry.h"
#include "cappedcylindersinglebondgeometry.h"
#include <algorithm>
#include <float4.h>

CappedCylinderTripleBondGeometry::CappedCylinderTripleBondGeometry(double r, int s)
{
  _slices = s;
  _numberOfVertexes = 3*(4 * _slices + 2);
  _numberOfIndices = 3*(12 * _slices);

  _vertexes = std::vector<RKVertex>();
  _indices = std::vector<short>();

  CappedCylinderSingleBondGeometry singleBondGeometry = CappedCylinderSingleBondGeometry(r,s);

  std::transform(singleBondGeometry.vertices().begin(), singleBondGeometry.vertices().end(), std::back_inserter(_vertexes), [](RKVertex i) {return RKVertex(i.position + float4(-1.0,0.0,-0.5*sqrt(3.0),0.0), i.normal, i.st);});
  std::transform(singleBondGeometry.vertices().begin(), singleBondGeometry.vertices().end(), std::back_inserter(_vertexes), [](RKVertex i) {return RKVertex(i.position + float4(1.0,0.0,-0.5*sqrt(3.0),0.0), i.normal, i.st);});
  std::transform(singleBondGeometry.vertices().begin(), singleBondGeometry.vertices().end(), std::back_inserter(_vertexes), [](RKVertex i) {return RKVertex(i.position + float4(0.0,0.0,0.5*sqrt(3.0),0.0), i.normal, i.st);});

  std::copy(singleBondGeometry.indices().begin(), singleBondGeometry.indices().end(), std::back_inserter(_indices));
  std::transform(singleBondGeometry.indices().begin(), singleBondGeometry.indices().end(), std::back_inserter(_indices), [=](uint16_t i) {return i + 4 * _slices + 2;});
  std::transform(singleBondGeometry.indices().begin(), singleBondGeometry.indices().end(), std::back_inserter(_indices), [=](uint16_t i) {return i + 8 * _slices + 4;});
}
