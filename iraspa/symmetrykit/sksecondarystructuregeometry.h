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

#include <mathkit.h>
#include <optional>
#include <vector>
#include "sksecondarystructure.h"

struct SKSecondaryStructureGeometry
{
  struct LocalAxisFrame
  {
    double3 origin;
    double3 direction;
  };

  struct PhiPsiDegrees
  {
    double phi = 0.0;
    double psi = 0.0;
  };

  static std::vector<std::optional<double3>> alphaCarbonPositions(const SKStrideBackboneChain &chain);
  static std::vector<std::optional<double3>> alphaCarbonPositionsWithBreaks(const SKStrideBackboneChain &chain,
                                                                            double breakDistance = 4.5);
  static std::vector<PhiPsiDegrees> phiPsiDegrees(const SKStrideBackboneChain &chain);
  static std::optional<LocalAxisFrame> localAxisFrame(int index, const std::vector<double3> &positions);
  static double axisRadius(const double3 &position, const LocalAxisFrame &frame);
  static double axisTau(const double3 &position, const LocalAxisFrame &frame);
  static std::vector<bool> maskConsecutive(const std::vector<bool> &mask, int minimumLength);
  static std::vector<bool> extendRegion(const std::vector<bool> &base, const std::vector<bool> &extensionMask);
  static double distance(const double3 &a, const double3 &b);
  static double angle(const double3 &p0, const double3 &p1, const double3 &p2);
  static double torsionAngle(const double3 &p0, const double3 &p1, const double3 &p2, const double3 &p3);
  static double3 normalize(const double3 &vector);
  static std::optional<double3> localHelixAxis(int index, const std::vector<double3> &positions);
  static double helixRadius(int index, const std::vector<double3> &positions, const double3 &axisOrigin, const double3 &axis);
  static double tauAngle(const double3 &position, const double3 &axisOrigin, const double3 &axis);
  static bool inRange(double value, double lower, double upper);
  static bool inWrappedRange(double value, double lower, double upper);
  static double acutePeptidePlaneAngle(const double3 &firstNitrogen,
                                       const double3 &firstAlphaCarbon,
                                       const double3 &firstCarbonylCarbon,
                                       const double3 &secondNitrogen,
                                       const double3 &secondAlphaCarbon,
                                       const double3 &secondCarbonylCarbon);
};
