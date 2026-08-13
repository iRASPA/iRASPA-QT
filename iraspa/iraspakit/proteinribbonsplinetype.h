/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonSplineType.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <QString>

enum class ProteinRibbonSplineType
{
  bSpline,
  catmullRom
};

inline QString proteinRibbonSplineTypeDisplayName(ProteinRibbonSplineType type)
{
  switch (type)
  {
  case ProteinRibbonSplineType::bSpline: return QString("B-Spline");
  case ProteinRibbonSplineType::catmullRom: return QString("Catmull-Rom");
  }
  return QString();
}

inline QString proteinRibbonSplineTypeRawValue(ProteinRibbonSplineType type)
{
  return proteinRibbonSplineTypeDisplayName(type);
}

inline ProteinRibbonSplineType proteinRibbonSplineTypeFromRawValue(const QString &value)
{
  if (value == QStringLiteral("Catmull-Rom")) return ProteinRibbonSplineType::catmullRom;
  return ProteinRibbonSplineType::bSpline;
}
