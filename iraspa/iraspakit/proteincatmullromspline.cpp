/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinCatmullRomSpline.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteincatmullromspline.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

ProteinCatmullRomSpline::ProteinCatmullRomSpline(std::vector<double3> controlPoints,
                                                  std::vector<double3> orientationVectors):
  _controlPoints(std::move(controlPoints)),
  _orientationVectors(std::move(orientationVectors)),
  _arcLengthCache(buildArcLengthCache(_controlPoints))
{
  if (_controlPoints.size() != _orientationVectors.size())
  {
    throw std::invalid_argument("ProteinCatmullRomSpline control point count must match orientation vector count");
  }
  if (_controlPoints.size() < 2)
  {
    throw std::invalid_argument("ProteinCatmullRomSpline requires at least two control points");
  }
}

int ProteinCatmullRomSpline::numberOfControlPoints() const
{
  return static_cast<int>(_controlPoints.size());
}

double ProteinCatmullRomSpline::arcLength(double t) const
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

double ProteinCatmullRomSpline::parameterFromArcLength(double targetLength) const
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

double3 ProteinCatmullRomSpline::evaluate(double t) const
{
  if (t <= 0.0) return _controlPoints.front();
  if (t >= 1.0) return _controlPoints.back();

  const int segmentCount = static_cast<int>(_controlPoints.size()) - 1;
  const double scaled = t * static_cast<double>(segmentCount);
  const int segmentIndex = std::min(static_cast<int>(scaled), segmentCount - 1);
  const double localU = scaled - static_cast<double>(segmentIndex);
  return evaluateSegmentPosition(segmentIndex, localU);
}

double3 ProteinCatmullRomSpline::evaluateOrientation(double t) const
{
  if (t <= 0.0) return double3::normalize(_orientationVectors.front());
  if (t >= 1.0) return double3::normalize(_orientationVectors.back());

  const int segmentCount = static_cast<int>(_controlPoints.size()) - 1;
  const double scaled = t * static_cast<double>(segmentCount);
  const int segmentIndex = std::min(static_cast<int>(scaled), segmentCount - 1);
  const double localU = scaled - static_cast<double>(segmentIndex);

  double3 orientation = evaluateSegmentOrientation(segmentIndex, localU);
  const double3 tangent = derivative(t);
  orientation = -double3::cross(tangent, double3::cross(tangent, orientation));
  const double orientationLength = orientation.length();
  if (orientationLength > 1.0e-12)
  {
    return orientation / orientationLength;
  }
  return orientation;
}

double3 ProteinCatmullRomSpline::derivative(double t) const
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

double3 ProteinCatmullRomSpline::controlPointAt(int index) const
{
  if (index < 0) return _controlPoints.front();
  if (index >= static_cast<int>(_controlPoints.size())) return _controlPoints.back();
  return _controlPoints[index];
}

double3 ProteinCatmullRomSpline::orientationVectorAt(int index) const
{
  if (index < 0) return _orientationVectors.front();
  if (index >= static_cast<int>(_orientationVectors.size())) return _orientationVectors.back();
  return _orientationVectors[index];
}

double3 ProteinCatmullRomSpline::evaluateSegmentPosition(int segmentIndex, double u) const
{
  return catmullRomPoint(controlPointAt(segmentIndex - 1),
                         controlPointAt(segmentIndex),
                         controlPointAt(segmentIndex + 1),
                         controlPointAt(segmentIndex + 2),
                         u);
}

double3 ProteinCatmullRomSpline::evaluateSegmentOrientation(int segmentIndex, double u) const
{
  return catmullRomPoint(orientationVectorAt(segmentIndex - 1),
                         orientationVectorAt(segmentIndex),
                         orientationVectorAt(segmentIndex + 1),
                         orientationVectorAt(segmentIndex + 2),
                         u);
}

double3 ProteinCatmullRomSpline::catmullRomPoint(const double3 &p0,
                                                 const double3 &p1,
                                                 const double3 &p2,
                                                 const double3 &p3,
                                                 double u)
{
  const double u2 = u * u;
  const double u3 = u2 * u;
  const double3 term0 = 2.0 * p1;
  const double3 term1 = (-p0 + p2) * u;
  const double3 term2 = (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * u2;
  const double3 term3 = (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * u3;
  return 0.5 * (term0 + term1 + term2 + term3);
}

std::vector<double> ProteinCatmullRomSpline::buildArcLengthCache(const std::vector<double3> &controlPoints)
{
  const int numberOfSamples = std::max(64, static_cast<int>(controlPoints.size()) * 8);
  std::vector<double> cache = {0.0};
  const double dt = 1.0 / static_cast<double>(numberOfSamples);
  const int segmentCount = static_cast<int>(controlPoints.size()) - 1;

  auto pointAt = [&](int index) -> double3
  {
    if (index < 0) return controlPoints.front();
    if (index >= static_cast<int>(controlPoints.size())) return controlPoints.back();
    return controlPoints[index];
  };

  auto evaluateAtGlobalT = [&](double t) -> double3
  {
    if (t <= 0.0) return controlPoints.front();
    if (t >= 1.0) return controlPoints.back();
    const double scaled = t * static_cast<double>(segmentCount);
    const int segmentIndex = std::min(static_cast<int>(scaled), segmentCount - 1);
    const double localU = scaled - static_cast<double>(segmentIndex);
    return catmullRomPoint(pointAt(segmentIndex - 1),
                           pointAt(segmentIndex),
                           pointAt(segmentIndex + 1),
                           pointAt(segmentIndex + 2),
                           localU);
  };

  for (int i = 0; i < numberOfSamples; ++i)
  {
    const double t1 = static_cast<double>(i) * dt;
    const double t2 = static_cast<double>(i + 1) * dt;
    cache.push_back(cache.back() + (evaluateAtGlobalT(t2) - evaluateAtGlobalT(t1)).length());
  }
  return cache;
}
