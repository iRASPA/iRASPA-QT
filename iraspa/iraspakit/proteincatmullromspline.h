/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinCatmullRomSpline.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <vector>
#include <mathkit.h>
#include "proteinribboncenterlinepath.h"

class ProteinCatmullRomSpline: public ProteinRibbonCenterlinePath
{
public:
  ProteinCatmullRomSpline(std::vector<double3> controlPoints,
                          std::vector<double3> orientationVectors);

  int numberOfControlPoints() const override;
  double arcLength(double t) const override;
  double parameterFromArcLength(double targetLength) const override;
  double3 evaluate(double t) const override;
  double3 evaluateOrientation(double t) const override;
  double3 derivative(double t) const override;

private:
  double3 controlPointAt(int index) const;
  double3 orientationVectorAt(int index) const;
  double3 evaluateSegmentPosition(int segmentIndex, double u) const;
  double3 evaluateSegmentOrientation(int segmentIndex, double u) const;

  static double3 catmullRomPoint(const double3 &p0,
                                 const double3 &p1,
                                 const double3 &p2,
                                 const double3 &p3,
                                 double u);
  static std::vector<double> buildArcLengthCache(const std::vector<double3> &controlPoints);

  std::vector<double3> _controlPoints;
  std::vector<double3> _orientationVectors;
  std::vector<double> _arcLengthCache;
};
