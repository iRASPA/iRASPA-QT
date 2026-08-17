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
  _degree(std::min({degree, static_cast<int>(_controlPoints.size()) - 1, maximumDegree}))
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
  computeDerivativeCurve(_controlPoints, _degree, _knots, _derivativeControlPoints, _derivativeKnots);
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
  const double fraction = (targetLength - lengthBefore) / std::max(lengthAfter - lengthBefore, 1.0e-18);
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

  const int controlPointCount = static_cast<int>(_controlPoints.size());
  const double clampedT = std::min(std::max(t, _knots[_degree]), _knots[controlPointCount] - 1.0e-12);
  const int span = findSpan(clampedT, _degree, _knots, controlPointCount);
  double basis[maximumDegree + 1] = {0.0};
  basisFunctions(span, clampedT, _degree, _knots, basis);

  double3 orientation = double3(0.0, 0.0, 0.0);
  for (int i = 0; i <= _degree; ++i)
  {
    orientation += basis[i] * _orientationVectors[span - _degree + i];
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

/// Analytic first derivative: the derivative of a clamped B-spline of degree p is a
/// B-spline of degree p-1 over the trimmed knot vector (Piegl & Tiller, eq. 3.4).
double3 ProteinBSpline::derivative(double t) const
{
  return evaluatePoint(t, _derivativeControlPoints, _degree - 1, _derivativeKnots);
}

void ProteinBSpline::computeDerivativeCurve(const std::vector<double3> &controlPoints,
                                            int degree,
                                            const std::vector<double> &knots,
                                            std::vector<double3> &derivativeControlPoints,
                                            std::vector<double> &derivativeKnots)
{
  derivativeControlPoints.clear();
  derivativeControlPoints.reserve(controlPoints.size() - 1);
  for (size_t i = 0; i + 1 < controlPoints.size(); ++i)
  {
    const double denominator = knots[i + static_cast<size_t>(degree) + 1] - knots[i + 1];
    if (denominator > 1.0e-18)
    {
      derivativeControlPoints.push_back((static_cast<double>(degree) / denominator) * (controlPoints[i + 1] - controlPoints[i]));
    }
    else
    {
      derivativeControlPoints.push_back(double3(0.0, 0.0, 0.0));
    }
  }
  derivativeKnots.assign(knots.begin() + 1, knots.end() - 1);
}

double3 ProteinBSpline::evaluatePoint(double t,
                                      const std::vector<double3> &controlPoints,
                                      int degree,
                                      const std::vector<double> &knots)
{
  if (t <= 0.0) return controlPoints.front();
  if (t >= 1.0) return controlPoints.back();

  const int controlPointCount = static_cast<int>(controlPoints.size());
  const double clampedT = std::min(std::max(t, knots[degree]), knots[controlPointCount] - 1.0e-12);
  const int span = findSpan(clampedT, degree, knots, controlPointCount);
  double basis[maximumDegree + 1] = {0.0};
  basisFunctions(span, clampedT, degree, knots, basis);

  double3 point = double3(0.0, 0.0, 0.0);
  for (int i = 0; i <= degree; ++i)
  {
    point += basis[i] * controlPoints[span - degree + i];
  }
  return point;
}

int ProteinBSpline::findSpan(double u, int degree, const std::vector<double> &knots, int controlPointCount)
{
  if (u >= knots[controlPointCount]) return controlPointCount - 1;
  if (u <= knots[degree]) return degree;

  int low = degree;
  int high = controlPointCount;
  int mid = (low + high) / 2;
  while (u < knots[mid] || u >= knots[mid + 1])
  {
    if (u < knots[mid]) high = mid;
    else low = mid;
    mid = (low + high) / 2;
  }
  return mid;
}

void ProteinBSpline::basisFunctions(int span, double u, int degree, const std::vector<double> &knots, double *basis)
{
  double left[maximumDegree + 1] = {0.0};
  double right[maximumDegree + 1] = {0.0};
  basis[0] = 1.0;
  if (degree <= 0) return;

  for (int j = 1; j <= degree; ++j)
  {
    left[j] = u - knots[span + 1 - j];
    right[j] = knots[span + j] - u;
    double saved = 0.0;
    for (int r = 0; r < j; ++r)
    {
      const double denominator = right[r + 1] + left[j - r];
      const double temp = std::abs(denominator) > 1.0e-18 ? basis[r] / denominator : 0.0;
      basis[r] = saved + right[r + 1] * temp;
      saved = left[j - r] * temp;
    }
    basis[j] = saved;
  }
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
