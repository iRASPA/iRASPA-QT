/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include <vector>
#include <memory>
#include "skatomtreecontroller.h"

struct DNAAtomTreeBuilder
{
  static bool applyHierarchyIfNeeded(SKAtomTreeController &controller);
  static std::vector<std::shared_ptr<SKAtomTreeNode>> build(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms);
};
