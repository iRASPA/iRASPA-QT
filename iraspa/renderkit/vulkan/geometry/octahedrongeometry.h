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
#include "mathkit.h"

class OctahedronGeometry
{
public:
  OctahedronGeometry(double r, float4 color);
  const std::vector<RKPrimitiveVertex> vertices() const {return _vertices;}
  const std::vector<short> indices() const {return _indices;}
private:
  std::vector<short> _indices{0, 3, 4,      // Face 0 - triangle ( v0,  v3,  v3)
                              0, 4, 5,      // Face 1 - triangle ( v0,  v4,  v5)
                              0, 5, 2,      // Face 2 - triangle ( v0,  v5,  v2)
                              0, 2, 3,      // Face 3 - triangle ( v0,  v2,  v3)
                              1, 4, 3,      // Face 4 - triangle ( v1,  v4,  v3)
                              1, 5, 4,      // Face 5 - triangle ( v1,  v5,  v4)
                              1, 2, 5,      // Face 6 - triangle ( v1,  v2,  v5)
                              1, 3, 2};     // Face 7 - triangle ( v1,  v3,  v2)
  std::vector<RKPrimitiveVertex> _vertices{};
};

