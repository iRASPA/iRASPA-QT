/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonSecondaryStructure.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteinribbonsecondarystructure.h"

namespace
{
  SKStrideBackboneChain makeBackboneChain(const ProteinBackboneChain &chain, double3 contentShift)
  {
    SKStrideBackboneChain backboneChain;
    backboneChain.chainIdentifier = chain.chainIdentifier;
    for (const ProteinBackboneResidue &residue : chain.residues)
    {
      if (!residue.alphaCarbon) continue;
      SKStrideBackboneResidue strideResidue;
      strideResidue.residueName = residue.residueName;
      if (residue.nitrogen) strideResidue.nitrogen = residue.nitrogen->position() + contentShift;
      if (residue.alphaCarbon) strideResidue.alphaCarbon = residue.alphaCarbon->position() + contentShift;
      if (residue.carbonylCarbon) strideResidue.carbonylCarbon = residue.carbonylCarbon->position() + contentShift;
      if (residue.carbonylOxygen) strideResidue.carbonylOxygen = residue.carbonylOxygen->position() + contentShift;
      backboneChain.residues.push_back(strideResidue);
    }
    return backboneChain;
  }
}

std::vector<ProteinRibbonSecondaryStructure> ProteinRibbonSecondaryStructureAssigner::assign(const ProteinBackboneChain &chain,
                                                                                             double3 contentShift,
                                                                                             ProteinRibbonSecondaryStructureMethod method)
{
  return assign(chain, contentShift, proteinRibbonSecondaryStructureAssignmentMethod(method));
}

std::vector<ProteinRibbonSecondaryStructure> ProteinRibbonSecondaryStructureAssigner::assign(const ProteinBackboneChain &chain,
                                                                                             double3 contentShift,
                                                                                             SKSecondaryStructureAssignmentMethod method)
{
  const SKStrideBackboneChain backboneChain = makeBackboneChain(chain, contentShift);
  const std::vector<SKSecondaryStructureType> assignment = SKSecondaryStructureAssigner::assign(backboneChain, method);
  std::vector<ProteinRibbonSecondaryStructure> result;
  result.reserve(assignment.size());
  for (SKSecondaryStructureType type : assignment)
  {
    result.push_back(proteinRibbonSecondaryStructureFromSK(type));
  }
  return result;
}

std::vector<ProteinRibbonSecondaryStructure> ProteinRibbonSecondaryStructureAssigner::assign(const std::vector<double3> &centers)
{
  return std::vector<ProteinRibbonSecondaryStructure>(centers.size(), ProteinRibbonSecondaryStructure::coil);
}

ProteinRibbonSecondaryStructure ProteinRibbonSecondaryStructureAssigner::interpolate(ProteinRibbonSecondaryStructure a,
                                                                                     ProteinRibbonSecondaryStructure b,
                                                                                     double t)
{
  if (t < 0.5) return a;
  return b;
}

bool ProteinRibbonSecondaryStructureAssigner::isSheetLeadingEdge(int residueIndex, const std::vector<ProteinRibbonSecondaryStructure> &assignment)
{
  if (residueIndex < 0 || residueIndex >= static_cast<int>(assignment.size())) return false;
  if (assignment[residueIndex] != ProteinRibbonSecondaryStructure::sheet) return false;
  return residueIndex == 0 || assignment[residueIndex - 1] != ProteinRibbonSecondaryStructure::sheet;
}

bool ProteinRibbonSecondaryStructureAssigner::isSheetTrailingEdge(int residueIndex, const std::vector<ProteinRibbonSecondaryStructure> &assignment)
{
  if (residueIndex < 0 || residueIndex >= static_cast<int>(assignment.size())) return false;
  if (assignment[residueIndex] != ProteinRibbonSecondaryStructure::sheet) return false;
  return residueIndex == static_cast<int>(assignment.size()) - 1 || assignment[residueIndex + 1] != ProteinRibbonSecondaryStructure::sheet;
}
