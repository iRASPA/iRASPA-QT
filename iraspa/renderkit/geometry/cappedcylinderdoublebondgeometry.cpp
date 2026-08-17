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

#include "cappedcylinderdoublebondgeometry.h"
#include "cappedcylindersinglebondgeometry.h"
#include <QDebug>

CappedCylinderDoubleBondGeometry::CappedCylinderDoubleBondGeometry(double r, int s): _singleBondGeometry(CappedCylinderSingleBondGeometry(r,s))
{
  _slices = s;
  _numberOfVertexes = 2*(4 * _slices + 2);
  _numberOfIndices = 2*(12 * _slices);

  _vertexes = std::vector<RKVertex>{};
  _indices = std::vector<short>{};

  std::transform(_singleBondGeometry.vertices().begin(), _singleBondGeometry.vertices().end(), std::back_inserter(_vertexes), [](const RKVertex &i) {return RKVertex(i.position + float4(-1.0,0.0,0.0,0.0), i.normal, i.st);});
  std::transform(_singleBondGeometry.vertices().begin(), _singleBondGeometry.vertices().end(), std::back_inserter(_vertexes), [](const RKVertex &i) {return RKVertex(i.position + float4(1.0,0.0,0.0,0.0), i.normal, i.st);});

  std::copy(_singleBondGeometry.indices().begin(), _singleBondGeometry.indices().end(), std::back_inserter(_indices));
  std::transform(_singleBondGeometry.indices().begin(), _singleBondGeometry.indices().end(), std::back_inserter(_indices), [=](const short &i) {return i + 4 * _slices + 2;});
}
