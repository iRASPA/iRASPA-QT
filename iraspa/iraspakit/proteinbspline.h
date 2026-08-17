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
  static double3 evaluatePoint(double t,
                               const std::vector<double3> &controlPoints,
                               int degree,
                               const std::vector<double> &knots);
  /// Binary search for the knot span index such that knots[span] <= u < knots[span+1].
  static int findSpan(double u, int degree, const std::vector<double> &knots, int controlPointCount);
  /// Non-zero basis functions N_{span-degree,degree} … N_{span,degree} (Piegl & Tiller, Alg. A2.2).
  /// Only degree+1 of them are non-zero, so evaluation never walks the whole control-point list.
  static void basisFunctions(int span, double u, int degree, const std::vector<double> &knots, double *basis);
  static std::vector<double> computeKnots(int numberOfControlPoints, int degree);
  static void computeDerivativeCurve(const std::vector<double3> &controlPoints,
                                     int degree,
                                     const std::vector<double> &knots,
                                     std::vector<double3> &derivativeControlPoints,
                                     std::vector<double> &derivativeKnots);
  static std::vector<double> buildArcLengthCache(const std::vector<double3> &controlPoints,
                                                 int degree,
                                                 const std::vector<double> &knots);

  static constexpr int maximumDegree = 16;

  std::vector<double3> _controlPoints;
  std::vector<double3> _orientationVectors;
  int _degree;
  std::vector<double> _knots;
  /// Control points of the analytic derivative curve: Q_i = degree * (P_{i+1} - P_i) / (u_{i+degree+1} - u_{i+1}).
  std::vector<double3> _derivativeControlPoints;
  /// Knot vector of the derivative curve (degree - 1): the original knots with the first and last dropped.
  std::vector<double> _derivativeKnots;
  std::vector<double> _arcLengthCache;
};
