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

#include "axessystemdefaultgeometry.h"
#include "arrowxgeometry.h"
#include "arrowygeometry.h"
#include "arrowzgeometry.h"
#include "nspheregeometry.h"
#include "cubeprimitivegeometry.h"

AxesSystemDefaultGeometry::AxesSystemDefaultGeometry(RKGlobalAxes::CenterType center, double centerRadius,
                                                     float4 centerColor, double arrowHeight, double arrowRadius,
                                                     float4 arrowColorX, float4 arrowColorY, float4 arrowColorZ,
                                                     double tipHeight, double tipRadius,
                                                     float4 tipColorX, float4 tipColorY, float4 tipColorZ,
                                                     bool tipVisibility, double aspectRatio, int sectorCount)
{

  NSphereGeometry sphere = NSphereGeometry(centerRadius * 1.2, centerColor, 4);
  CubePrimitiveGeometry cube = CubePrimitiveGeometry(centerRadius, centerColor);
  switch(center)
  {
  case RKGlobalAxes::CenterType::sphere:
    std::copy(sphere.vertices().begin(), sphere.vertices().end(), std::back_inserter(_vertices));
    std::copy(sphere.indices().begin(), sphere.indices().end(), std::back_inserter(_indices));
      break;
  case RKGlobalAxes::CenterType::cube:
    std::copy(cube.vertices().begin(), cube.vertices().end(), std::back_inserter(_vertices));
    std::copy(cube.indices().begin(), cube.indices().end(), std::back_inserter(_indices));
    break;
  }

  size_t axisXoffset = _vertices.size();
  ArrowXGeometry axisX = ArrowXGeometry(centerRadius, arrowHeight, arrowRadius, arrowColorX, tipHeight, tipRadius,
                                        tipColorX, tipVisibility, aspectRatio, sectorCount);
  std::copy(axisX.vertices().begin(), axisX.vertices().end(), std::back_inserter(_vertices));
  std::transform(axisX.indices().begin(), axisX.indices().end(),  std::back_inserter(_indices),
                  [axisXoffset](short index) -> short
                               {return short(index + axisXoffset);});

  size_t axisYoffset = _vertices.size();
  ArrowYGeometry axisY = ArrowYGeometry(centerRadius, arrowHeight, arrowRadius, arrowColorY, tipHeight, tipRadius,
                                        tipColorY, tipVisibility, aspectRatio, sectorCount);
  std::copy(axisY.vertices().begin(), axisY.vertices().end(), std::back_inserter(_vertices));
  std::transform(axisY.indices().begin(), axisY.indices().end(),  std::back_inserter(_indices),
                  [axisYoffset](short index) -> short
                               {return short(index + axisYoffset);});

  size_t axisZoffset = _vertices.size();
  ArrowZGeometry axisZ = ArrowZGeometry(centerRadius, arrowHeight, arrowRadius, arrowColorZ, tipHeight, tipRadius,
                                        tipColorZ, tipVisibility, aspectRatio, sectorCount);
  std::copy(axisZ.vertices().begin(), axisZ.vertices().end(), std::back_inserter(_vertices));
  std::transform(axisZ.indices().begin(), axisZ.indices().end(),  std::back_inserter(_indices),
                  [axisZoffset](short index) -> short
                               {return short(index + axisZoffset);});

  _numberOfVertices = _vertices.size();
  _numberOfIndices = _indices.size();
}
