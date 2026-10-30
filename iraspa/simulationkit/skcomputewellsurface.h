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
#include <mathkit.h>

class SKComputeWellSurface
{
public:
  static constexpr float energyScale = 0.001f;
  static constexpr float filamentReliabilityThreshold = 0.45f;
  static constexpr double filamentMinimumArea = 2.0;
  static constexpr int filamentClosingRadius = 2;

  static std::vector<float> computeWellFieldGrid(int3 size,
                                                 double2 probeParameter,
                                                 const std::vector<double3> &positions,
                                                 const std::vector<double2> &potentialParameters,
                                                 double3x3 unitCell,
                                                 int3 numberOfReplicas,
                                                 const std::vector<double4> &blockingPockets = {}) noexcept;

  static float effectiveTrimIsovalue(const std::vector<float> &field,
                                     double isovalue,
                                     int3 dimensions) noexcept;

  static std::vector<float4> constructWellSurfaceVertexBuffer(const std::vector<float> &field,
                                                              double isovalue,
                                                              int3 dimensions);

  static void refineWellSurfaceVertices(std::vector<float4> &vertexBuffer,
                                        double2 probeParameter,
                                        const std::vector<double3> &positions,
                                        const std::vector<double2> &potentialParameters,
                                        double3x3 unitCell,
                                        int3 numberOfReplicas,
                                        float isovalue,
                                        const std::vector<double4> &blockingPockets = {}) noexcept;

  static std::vector<float4> constructWellFilamentVertexBuffer(const std::vector<float> &field,
                                                               double isovalue,
                                                               int3 dimensions,
                                                               double3x3 unitCell);

  static double areaOfVertexBuffer(const std::vector<float4> &vertexBuffer,
                                   const double3x3 &unitCell) noexcept;
};
