/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonRepresentationStyle.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <QString>
#include <vector>

enum class ProteinRibbonRepresentationStyle
{
  defaultStyle,
  fancy,
  custom
};

inline QString proteinRibbonRepresentationStyleDisplayName(ProteinRibbonRepresentationStyle style)
{
  switch (style)
  {
  case ProteinRibbonRepresentationStyle::defaultStyle: return QString("Default");
  case ProteinRibbonRepresentationStyle::fancy: return QString("Fancy");
  case ProteinRibbonRepresentationStyle::custom: return QString("Custom");
  }
  return QString();
}

inline ProteinRibbonRepresentationStyle proteinRibbonRepresentationStyleFromRawValue(const QString &value)
{
  if (value == QStringLiteral("Fancy")) return ProteinRibbonRepresentationStyle::fancy;
  if (value == QStringLiteral("Custom")) return ProteinRibbonRepresentationStyle::custom;
  return ProteinRibbonRepresentationStyle::defaultStyle;
}

inline QString proteinRibbonRepresentationStyleRawValue(ProteinRibbonRepresentationStyle style)
{
  return proteinRibbonRepresentationStyleDisplayName(style);
}

inline std::vector<ProteinRibbonRepresentationStyle> proteinRibbonRepresentationSelectableCases()
{
  return {ProteinRibbonRepresentationStyle::defaultStyle, ProteinRibbonRepresentationStyle::fancy};
}
