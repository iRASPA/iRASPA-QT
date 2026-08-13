/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonCenterlinePath.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <mathkit.h>

class ProteinRibbonCenterlinePath
{
public:
  virtual ~ProteinRibbonCenterlinePath() = default;

  virtual int numberOfControlPoints() const = 0;
  virtual double arcLength(double t) const = 0;
  virtual double parameterFromArcLength(double targetLength) const = 0;
  virtual double3 evaluate(double t) const = 0;
  virtual double3 evaluateOrientation(double t) const = 0;
  virtual double3 derivative(double t) const = 0;
};
