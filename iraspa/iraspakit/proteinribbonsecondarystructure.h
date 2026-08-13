/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonSecondaryStructure.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <mathkit.h>
#include <vector>
#include <optional>
#include <memory>
#include "sksecondarystructure.h"
#include "proteinbackbone.h"
#include "proteinribbonsecondarystructuremethod.h"

enum class ProteinRibbonSecondaryStructure
{
  coil,
  helix,
  sheet
};

inline float3 proteinRibbonSecondaryStructureRibbonColor(ProteinRibbonSecondaryStructure structure)
{
  switch (structure)
  {
  case ProteinRibbonSecondaryStructure::coil: return float3(0.0f, 0.0f, 1.0f);
  case ProteinRibbonSecondaryStructure::helix: return float3(0.0f, 1.0f, 0.0f);
  case ProteinRibbonSecondaryStructure::sheet: return float3(1.0f, 0.0f, 1.0f);
  }
  return float3();
}

inline ProteinRibbonSecondaryStructure proteinRibbonSecondaryStructureFromSK(SKSecondaryStructureType type)
{
  switch (type)
  {
  case SKSecondaryStructureType::coil: return ProteinRibbonSecondaryStructure::coil;
  case SKSecondaryStructureType::helix: return ProteinRibbonSecondaryStructure::helix;
  case SKSecondaryStructureType::sheet: return ProteinRibbonSecondaryStructure::sheet;
  }
  return ProteinRibbonSecondaryStructure::coil;
}

struct ProteinRibbonSecondaryStructureAssigner
{
  static std::vector<ProteinRibbonSecondaryStructure> assign(const ProteinBackboneChain &chain,
                                                             double3 contentShift,
                                                             ProteinRibbonSecondaryStructureMethod method = ProteinRibbonSecondaryStructureMethod::stride);
  static std::vector<ProteinRibbonSecondaryStructure> assign(const ProteinBackboneChain &chain,
                                                             double3 contentShift,
                                                             SKSecondaryStructureAssignmentMethod method);
  static std::vector<ProteinRibbonSecondaryStructure> assign(const std::vector<double3> &centers);
  static ProteinRibbonSecondaryStructure interpolate(ProteinRibbonSecondaryStructure a,
                                                     ProteinRibbonSecondaryStructure b,
                                                     double t);
  static bool isSheetLeadingEdge(int residueIndex, const std::vector<ProteinRibbonSecondaryStructure> &assignment);
  static bool isSheetTrailingEdge(int residueIndex, const std::vector<ProteinRibbonSecondaryStructure> &assignment);
};
