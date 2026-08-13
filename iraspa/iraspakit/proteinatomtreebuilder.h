/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinAtomTreeBuilder.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <vector>
#include <memory>
#include "skatomtreecontroller.h"
#include "proteinribbonsecondarystructuremethod.h"

struct ProteinAtomTreeBuilder
{
  static bool applyHierarchyIfNeeded(SKAtomTreeController &controller,
                                     ProteinRibbonSecondaryStructureMethod secondaryStructureMethod = ProteinRibbonSecondaryStructureMethod::stride);
  static std::vector<std::shared_ptr<SKAtomTreeNode>> build(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms,
                                                            ProteinRibbonSecondaryStructureMethod secondaryStructureMethod = ProteinRibbonSecondaryStructureMethod::stride);
};
