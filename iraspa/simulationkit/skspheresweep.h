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
#include <utility>
#include <mathkit.h>

class SKSphereSweep
{
public:
  struct Ball
  {
    double3 center;
    double radius{};
    int3 imageShift{0, 0, 0};
    int atomIndex{0};
  };

  struct Arc
  {
    int otherBall{0};
    int3 otherImage{0, 0, 0};
    double3 otherCenter;
    double theta0{0.0};
    double theta1{0.0};
    bool reversed{false};
    int concavePatchIndex{-1};
    int saddlePatchIndex{-1};
  };

  struct Cycle
  {
    std::vector<Arc> arcs;
    double3 interiorPoint;
  };

  struct AtomFace
  {
    int atomIndex{0};
    double3 center;
    double radius{0.0};
    std::vector<Cycle> cycles;
    double area{0.0};
    double solidAngle{0.0};
    double volumeWedge{0.0};
    double volumeDirect{0.0};
  };

  struct SaddlePatch
  {
    int atomI{0};
    int atomJ{0};
    int3 imageShiftJ{0, 0, 0};
    double3 centerI;
    double radiusI{0.0};
    double3 centerJ;
    double radiusJ{0.0};
    double3 torusCenter;
    double3 axis;
    double majorRadius{0.0};
    double phiMin{0.0};
    double phiMax{0.0};
    double psiI{0.0};
    double psiJ{0.0};
    std::vector<std::pair<double, double>> arcs;
    double area{0.0};
  };

  struct ConcaveCorner
  {
    int atomI{0};
    int atomJ{0};
    int atomK{0};
    int3 imageShiftJ{0, 0, 0};
    int3 imageShiftK{0, 0, 0};
    double3 probeCenter;
    double3 touchI;
    double3 touchJ;
    double3 touchK;
    double area{0.0};
    int multiplicity{1};
  };

  struct Decomposition
  {
    double probeRadius{0.0};
    std::vector<AtomFace> atomFaces;
    std::vector<SaddlePatch> saddlePatches;
    std::vector<ConcaveCorner> concaveCorners;
    double totalArea{0.0};
    double totalVolumeWedge{0.0};
    double totalVolumeDirect{0.0};
    double totalSESArea{0.0};
    double totalContactArea{0.0};
    double totalTorusArea{0.0};
    double totalConcaveArea{0.0};
  };

  static void makeCircleFrame(const double3 &axisUnit, double3 &uOut, double3 &vOut);
  static double wrapAngle0To2Pi(double angle);
  static double positiveAngleDifference(double fromAngle, double toAngle);

  static Decomposition compute(const std::vector<std::pair<double3, double>> &atoms,
                               const double3x3 &latticeVectors,
                               bool isPeriodic,
                               double probeRadius = 0.0);

  static std::vector<float4> triangulate(const Decomposition &decomposition,
                                         const double3x3 &inverseCell,
                                         bool includeExtendedPatches = true,
                                         int maxSubdivisionDepth = 3);
};
