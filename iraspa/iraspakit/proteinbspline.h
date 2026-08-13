/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinBSpline.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <vector>
#include <mathkit.h>
#include "proteinribboncenterlinepath.h"

class ProteinBSpline: public ProteinRibbonCenterlinePath
{
public:
  ProteinBSpline(std::vector<double3> controlPoints,
                 std::vector<double3> orientationVectors,
                 int degree = 3);

  int numberOfControlPoints() const override;
  double arcLength(double t) const override;
  double parameterFromArcLength(double targetLength) const override;
  double3 evaluate(double t) const override;
  double3 evaluateOrientation(double t) const override;
  double3 derivative(double t) const override;

private:
  double basisFunction(int i, int p, double u) const;

  static double3 evaluatePoint(double t,
                               const std::vector<double3> &controlPoints,
                               int degree,
                               const std::vector<double> &knots);
  static double basisFunction(int i, int p, double u, const std::vector<double> &knots);
  static std::vector<double> computeKnots(int numberOfControlPoints, int degree);
  static std::vector<double> buildArcLengthCache(const std::vector<double3> &controlPoints,
                                                 int degree,
                                                 const std::vector<double> &knots);

  std::vector<double3> _controlPoints;
  std::vector<double3> _orientationVectors;
  int _degree;
  std::vector<double> _knots;
  std::vector<double> _arcLengthCache;
};
