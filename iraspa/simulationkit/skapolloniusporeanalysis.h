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
#include "skapolloniusdiagram.h"

class SKApolloniusPoreAnalysis
{
public:
  struct InaccessiblePocket
  {
    std::vector<std::size_t> vertexIndices;
    double3 fractionalCenter;
    double3 cartesianCenter;
    double maxIncludedSphereDiameter{0.0};
    double boundingRadius{0.0};
    std::vector<std::pair<double3, double>> sphereUnion;
  };

  struct ChannelComponent
  {
    std::vector<std::size_t> vertexIndices;
    int dimensionality{0};
    double maxIncludedSphereDiameter{0.0};
    double limitingFreeSphereDiameter{0.0};
  };

  struct PoreTopologyResult
  {
    double probeRadius{0.0};
    double globalDi{0.0};
    double globalDf{0.0};
    double globalDif{0.0};
    double dfAlongA{0.0};
    double dfAlongB{0.0};
    double dfAlongC{0.0};
    int channelDimensionality{0};
    std::vector<ChannelComponent> channels;
    std::vector<InaccessiblePocket> inaccessiblePockets;
    std::vector<std::pair<double3, double>> blockingSpheres;
  };

  static PoreTopologyResult analyze(const SKApolloniusDiagram::ApolloniusNetwork &network,
                                    const double3x3 &latticeVectors,
                                    double probeRadius,
                                    bool isPeriodic = true,
                                    const std::vector<std::pair<double3, double>> &atoms = {});

  static std::vector<std::pair<double3, double>> computeBlockingSpheres(
      const std::vector<std::pair<double3, double>> &atoms,
      const double3x3 &latticeVectors,
      double probeRadius,
      bool isPeriodic = true,
      double gridSpacing = 0.2);
};
