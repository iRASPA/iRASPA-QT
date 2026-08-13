/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinAminoAcidResidueReplacer.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <optional>
#include <vector>
#include <memory>
#include <QString>
#include "skatomtreecontroller.h"
#include "skbondsetcontroller.h"
#include "skcolorsets.h"
#include <simulationkit.h>

class Structure;
class Protein;
class ProteinCrystal;

struct ProteinAminoAcidResidueReplacer
{
  static bool isProteinStructure(const std::shared_ptr<Structure> &structure);
  static bool isKnownAminoAcidResidueName(const QString &residueName);
  static bool isAminoAcidResidueGroupNode(const std::shared_ptr<SKAtomTreeNode> &node);

  struct ResidueContext
  {
    std::shared_ptr<SKAtomTreeNode> residueNode;
    std::vector<std::shared_ptr<SKAtomTreeNode>> atomNodes;
  };

  static std::optional<ResidueContext> residueContext(const std::shared_ptr<SKAtomTreeNode> &clickedNode,
                                                      SKAtomTreeController &controller);
  static std::optional<QString> currentResidueCode(const std::vector<std::shared_ptr<SKAtomTreeNode>> &atomNodes);

  static std::optional<std::pair<std::shared_ptr<SKAtomTreeController>, std::shared_ptr<SKBondSetController>>> snapshotAtomBondState(const std::shared_ptr<Structure> &structure);

  static bool replaceResidue(const std::shared_ptr<Structure> &structure,
                             const std::shared_ptr<SKAtomTreeNode> &residueNode,
                             const std::vector<std::shared_ptr<SKAtomTreeNode>> &atomNodes,
                             const QString &newResidueCode,
                             SKColorSets &colorSets,
                             ForceFieldSets &forceFieldSets);
};
