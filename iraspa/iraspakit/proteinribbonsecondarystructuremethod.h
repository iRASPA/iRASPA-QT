/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonSecondaryStructureMethod.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <QString>
#include "sksecondarystructure.h"

enum class ProteinRibbonSecondaryStructureMethod
{
  stride,
  dss,
  dssp,
  psea,
  sequoia,
  segno
};

inline QString proteinRibbonSecondaryStructureMethodDisplayName(ProteinRibbonSecondaryStructureMethod method)
{
  switch (method)
  {
  case ProteinRibbonSecondaryStructureMethod::stride: return QString("STRIDE");
  case ProteinRibbonSecondaryStructureMethod::dss: return QString("DSS");
  case ProteinRibbonSecondaryStructureMethod::dssp: return QString("DSSP");
  case ProteinRibbonSecondaryStructureMethod::psea: return QString("P-SEA");
  case ProteinRibbonSecondaryStructureMethod::sequoia: return QString("Sequoia");
  case ProteinRibbonSecondaryStructureMethod::segno: return QString("SEGNO");
  }
  return QString();
}

inline ProteinRibbonSecondaryStructureMethod proteinRibbonSecondaryStructureMethodFromRawValue(const QString &value)
{
  if (value == QStringLiteral("DSS")) return ProteinRibbonSecondaryStructureMethod::dss;
  if (value == QStringLiteral("DSSP")) return ProteinRibbonSecondaryStructureMethod::dssp;
  if (value == QStringLiteral("P-SEA")) return ProteinRibbonSecondaryStructureMethod::psea;
  if (value == QStringLiteral("Sequoia")) return ProteinRibbonSecondaryStructureMethod::sequoia;
  if (value == QStringLiteral("SEGNO")) return ProteinRibbonSecondaryStructureMethod::segno;
  return ProteinRibbonSecondaryStructureMethod::stride;
}

inline QString proteinRibbonSecondaryStructureMethodRawValue(ProteinRibbonSecondaryStructureMethod method)
{
  return proteinRibbonSecondaryStructureMethodDisplayName(method);
}

inline SKSecondaryStructureAssignmentMethod proteinRibbonSecondaryStructureAssignmentMethod(ProteinRibbonSecondaryStructureMethod method)
{
  switch (method)
  {
  case ProteinRibbonSecondaryStructureMethod::stride: return SKSecondaryStructureAssignmentMethod::stride;
  case ProteinRibbonSecondaryStructureMethod::dss: return SKSecondaryStructureAssignmentMethod::dss;
  case ProteinRibbonSecondaryStructureMethod::dssp: return SKSecondaryStructureAssignmentMethod::dssp;
  case ProteinRibbonSecondaryStructureMethod::psea: return SKSecondaryStructureAssignmentMethod::psea;
  case ProteinRibbonSecondaryStructureMethod::sequoia: return SKSecondaryStructureAssignmentMethod::sequoia;
  case ProteinRibbonSecondaryStructureMethod::segno: return SKSecondaryStructureAssignmentMethod::segno;
  }
  return SKSecondaryStructureAssignmentMethod::stride;
}
