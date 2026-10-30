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

#include <cstddef>
#include <vector>
#include <utility>
#include <mathkit.h>

class SKVoronoi
{
public:
  enum class VoronoiType
  {
    Standard,
    Radical
  };

  struct VoronoiFace
  {
    std::vector<std::size_t> vertexIndices;
    std::size_t neighborAtomIndex{0};
    int3 periodicShift{0, 0, 0};
    double3 normal;
  };

  struct VoronoiCell
  {
    std::vector<double3> vertices;
    std::vector<VoronoiFace> faces;
    double volume{0.0};
  };

  struct VoronoiEdge
  {
    std::size_t startVertex{0};
    std::size_t endVertex{0};
    int3 periodicShift{0, 0, 0};
    double maxFreeSphereRadius{0.0};
  };

  struct VoronoiNode
  {
    double3 cartesianPosition;
    double3 fractionalPosition;
    double maxFreeSphereRadius{0.0};
  };

  struct VoronoiNetwork
  {
    std::vector<VoronoiNode> nodes;
    std::vector<VoronoiEdge> edges;
    std::vector<VoronoiCell> cells;
  };

  static VoronoiCell computeCell(std::size_t atomIndex,
                                 const std::vector<std::pair<double3, double>> &atoms,
                                 const double3x3 &latticeVectors,
                                 bool isPeriodic = true,
                                 VoronoiType type = VoronoiType::Standard,
                                 double boundingBoxPadding = 10.0);

  static std::vector<VoronoiCell> computeAllCells(const std::vector<std::pair<double3, double>> &atoms,
                                                  const double3x3 &latticeVectors,
                                                  bool isPeriodic = true,
                                                  VoronoiType type = VoronoiType::Standard,
                                                  double boundingBoxPadding = 10.0);

  static VoronoiNetwork computeNetwork(const std::vector<std::pair<double3, double>> &atoms,
                                       const double3x3 &latticeVectors,
                                       bool isPeriodic = true,
                                       VoronoiType type = VoronoiType::Radical,
                                       double boundingBoxPadding = 10.0);
};
