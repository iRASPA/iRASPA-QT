/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinBSpline.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteinbspline.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

ProteinBSpline::ProteinBSpline(std::vector<double3> controlPoints,
                               std::vector<double3> orientationVectors,
                               int degree):
  _controlPoints(std::move(controlPoints)),
  _orientationVectors(std::move(orientationVectors)),
  _degree(std::min(degree, static_cast<int>(_controlPoints.size()) - 1))
{
  if (_controlPoints.size() != _orientationVectors.size())
  {
    throw std::invalid_argument("ProteinBSpline control point count must match orientation vector count");
  }
  if (_controlPoints.size() < 2)
  {
    throw std::invalid_argument("ProteinBSpline requires at least two control points");
  }
  _knots = computeKnots(static_cast<int>(_controlPoints.size()), _degree);
  _arcLengthCache = buildArcLengthCache(_controlPoints, _degree, _knots);
}

int ProteinBSpline::numberOfControlPoints() const
{
  return static_cast<int>(_controlPoints.size());
}

double ProteinBSpline::arcLength(double t) const
{
  if (_arcLengthCache.empty()) return 0.0;
  if (t <= 0.0) return 0.0;
  if (t >= 1.0) return _arcLengthCache.back();

  const double index = t * static_cast<double>(_arcLengthCache.size() - 1);
  const int i0 = static_cast<int>(index);
  const int i1 = std::min(i0 + 1, static_cast<int>(_arcLengthCache.size()) - 1);
  const double fraction = index - static_cast<double>(i0);
  return _arcLengthCache[i0] * (1.0 - fraction) + _arcLengthCache[i1] * fraction;
}

double ProteinBSpline::parameterFromArcLength(double targetLength) const
{
  if (_arcLengthCache.empty()) return 0.0;
  const double totalLength = _arcLengthCache.back();
  if (targetLength <= 0.0) return 0.0;
  if (targetLength >= totalLength) return 1.0;

  const auto it = std::lower_bound(_arcLengthCache.begin(), _arcLengthCache.end(), targetLength);
  const int index = static_cast<int>(std::distance(_arcLengthCache.begin(), it));
  if (index == 0) return 0.0;

  const double lengthBefore = _arcLengthCache[index - 1];
  const double lengthAfter = _arcLengthCache[index];
  const double fraction = (targetLength - lengthBefore) / (lengthAfter - lengthBefore);
  const double tBefore = static_cast<double>(index - 1) / static_cast<double>(_arcLengthCache.size() - 1);
  const double tAfter = static_cast<double>(index) / static_cast<double>(_arcLengthCache.size() - 1);
  return tBefore + fraction * (tAfter - tBefore);
}

double3 ProteinBSpline::evaluate(double t) const
{
  return evaluatePoint(t, _controlPoints, _degree, _knots);
}

double3 ProteinBSpline::evaluateOrientation(double t) const
{
  if (t <= 0.0) return double3::normalize(_orientationVectors.front());
  if (t >= 1.0) return double3::normalize(_orientationVectors.back());

  double3 orientation = double3(0.0, 0.0, 0.0);
  for (int i = 0; i < static_cast<int>(_controlPoints.size()); ++i)
  {
    orientation += basisFunction(i, _degree, t) * _orientationVectors[i];
  }

  const double3 tangent = derivative(t);
  orientation = -double3::cross(tangent, double3::cross(tangent, orientation));
  const double orientationLength = orientation.length();
  if (orientationLength > 1.0e-12)
  {
    return orientation / orientationLength;
  }
  return orientation;
}

double3 ProteinBSpline::derivative(double t) const
{
  const double epsilon = 1.0e-5;
  if (t <= epsilon)
  {
    return (evaluate(t + epsilon) - evaluate(t)) / epsilon;
  }
  if (t >= 1.0 - epsilon)
  {
    return (evaluate(t) - evaluate(t - epsilon)) / epsilon;
  }
  return (evaluate(t + epsilon) - evaluate(t - epsilon)) / (2.0 * epsilon);
}

double ProteinBSpline::basisFunction(int i, int p, double u) const
{
  return basisFunction(i, p, u, _knots);
}

double3 ProteinBSpline::evaluatePoint(double t,
                                      const std::vector<double3> &controlPoints,
                                      int degree,
                                      const std::vector<double> &knots)
{
  if (t <= 0.0) return controlPoints.front();
  if (t >= 1.0) return controlPoints.back();

  double3 point = double3(0.0, 0.0, 0.0);
  for (int i = 0; i < static_cast<int>(controlPoints.size()); ++i)
  {
    point += basisFunction(i, degree, t, knots) * controlPoints[i];
  }
  return point;
}

double ProteinBSpline::basisFunction(int i, int p, double u, const std::vector<double> &knots)
{
  if (p == 0)
  {
    return (u >= knots[i] && u < knots[i + 1]) ? 1.0 : 0.0;
  }

  double left = 0.0;
  double right = 0.0;
  if (knots[i + p] != knots[i])
  {
    left = (u - knots[i]) / (knots[i + p] - knots[i]) * basisFunction(i, p - 1, u, knots);
  }
  if (knots[i + p + 1] != knots[i + 1])
  {
    right = (knots[i + p + 1] - u) / (knots[i + p + 1] - knots[i + 1]) * basisFunction(i + 1, p - 1, u, knots);
  }
  return left + right;
}

std::vector<double> ProteinBSpline::computeKnots(int numberOfControlPoints, int degree)
{
  const int m = numberOfControlPoints + degree + 1;
  std::vector<double> knots(m, 0.0);
  for (int i = 0; i <= degree; ++i) knots[i] = 0.0;
  if (numberOfControlPoints > degree + 1)
  {
    for (int i = degree + 1; i < numberOfControlPoints; ++i)
    {
      knots[i] = static_cast<double>(i - degree) / static_cast<double>(numberOfControlPoints - degree);
    }
  }
  for (int i = numberOfControlPoints; i < m; ++i) knots[i] = 1.0;
  return knots;
}

std::vector<double> ProteinBSpline::buildArcLengthCache(const std::vector<double3> &controlPoints,
                                                        int degree,
                                                        const std::vector<double> &knots)
{
  const int numberOfSamples = std::max(64, static_cast<int>(controlPoints.size()) * 4);
  std::vector<double> cache = {0.0};
  const double dt = 1.0 / static_cast<double>(numberOfSamples);
  for (int i = 0; i < numberOfSamples; ++i)
  {
    const double t1 = static_cast<double>(i) * dt;
    const double t2 = static_cast<double>(i + 1) * dt;
    const double3 p1 = evaluatePoint(t1, controlPoints, degree, knots);
    const double3 p2 = evaluatePoint(t2, controlPoints, degree, knots);
    cache.push_back(cache.back() + (p2 - p1).length());
  }
  return cache;
}
