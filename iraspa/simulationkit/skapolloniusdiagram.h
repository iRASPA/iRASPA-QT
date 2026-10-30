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

#include <array>
#include <cstddef>
#include <vector>
#include <utility>
#include <mathkit.h>

class SKApolloniusDiagram
{
public:
  enum class EdgeCurvatureType
  {
    StraightLine,
    Circle,
    Parabola,
    Ellipse,
    Hyperbola
  };

  struct ApolloniusVertex
  {
    double3 cartesianPosition;
    double3 fractionalPosition;
    double clearanceRadius{0.0};
    std::array<std::size_t, 4> generatorAtoms{};
    std::array<int3, 4> generatorShifts{};
  };

  struct ApolloniusEdge
  {
    std::size_t startVertex{0};
    std::size_t endVertex{0};
    int3 periodicShift{0, 0, 0};
    double bottleneckRadius{0.0};
    double3 bottleneckPosition;
    std::array<std::size_t, 3> generatorAtoms{};
    std::array<int3, 3> generatorShifts{};
    EdgeCurvatureType curvatureType{EdgeCurvatureType::Hyperbola};
    std::vector<double3> polylinePoints;
    std::vector<double> polylineClearances;
  };

  struct ApolloniusFace
  {
    std::size_t atomA{0};
    std::size_t atomB{0};
    int3 periodicShiftB{0, 0, 0};
    std::vector<std::size_t> vertexIndices;
    std::vector<std::size_t> edgeIndices;
    std::vector<double3> triangles;
    std::vector<double3> triangleNormals;
  };

  struct ApolloniusCell
  {
    std::size_t atomIndex{0};
    std::vector<std::size_t> faceIndices;
    std::vector<std::size_t> vertexIndices;
    double volume{0.0};
    double surfaceArea{0.0};
  };

  struct ApolloniusNetwork
  {
    std::vector<ApolloniusVertex> vertices;
    std::vector<ApolloniusEdge> edges;
    std::vector<ApolloniusFace> faces;
    std::vector<ApolloniusCell> cells;
  };

  static std::vector<std::pair<double3, double>> solveApolloniusQuadruplet(
      const std::array<double3, 4> &centers,
      const std::array<double, 4> &radii);

  static ApolloniusNetwork computeNetwork(
      const std::vector<std::pair<double3, double>> &atoms,
      const double3x3 &latticeVectors,
      bool isPeriodic = true,
      std::size_t polylineResolution = 16);
};
