/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonSegmentSupport.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <QChar>
#include <optional>
#include <set>
#include <vector>
#include <functional>
#include <memory>
#include <mathkit.h>
#include "proteinbackbone.h"
#include "proteinribbonsecondarystructure.h"
#include "proteinribbonsecondarystructuremethod.h"
#include "skatomtreecontroller.h"

struct ProteinRibbonResidueSegment
{
  QChar chainIdentifier = QChar(' ');
  ProteinRibbonSecondaryStructure structureType = ProteinRibbonSecondaryStructure::coil;
  int firstResidueIndex = 0;
  int lastResidueIndex = 0;

  ProteinRibbonResidueSegment() = default;
  ProteinRibbonResidueSegment(QChar chainIdentifier,
                              ProteinRibbonSecondaryStructure structureType,
                              int firstResidueIndex,
                              int lastResidueIndex);
};

struct ProteinRibbonSegmentSupport
{
  static std::vector<ProteinRibbonResidueSegment> residueSegments(const ProteinBackboneChain &chain,
                                                                  double3 contentShift,
                                                                  ProteinRibbonSecondaryStructureMethod secondaryStructureMethod = ProteinRibbonSecondaryStructureMethod::stride);
  static std::vector<ProteinRibbonResidueSegment> residueSegments(const std::vector<ProteinRibbonSecondaryStructure> &assignment,
                                                                  QChar chainIdentifier);
  static std::vector<ProteinRibbonResidueSegment> residueSegments(const ProteinBackbone &backbone,
                                                                  double3 contentShift,
                                                                  ProteinRibbonSecondaryStructureMethod secondaryStructureMethod = ProteinRibbonSecondaryStructureMethod::stride);

  static bool isChainGroupNode(const std::shared_ptr<SKAtomTreeNode> &node);
  static bool isSecondaryStructureSegmentNode(const std::shared_ptr<SKAtomTreeNode> &node);
  static bool isHetatmGroupNode(const std::shared_ptr<SKAtomTreeNode> &node);
  static bool isResidueGroupNode(const std::shared_ptr<SKAtomTreeNode> &node);
  static bool isDNAHelixGroupNode(const std::shared_ptr<SKAtomTreeNode> &node);
  static bool isProteinHierarchyGroupNode(const std::shared_ptr<SKAtomTreeNode> &node);
  /// Chain / helix / polymer residue groups that expose separate Atoms and Ribbon visibility.
  static bool isRibbonHierarchyGroupNode(const std::shared_ptr<SKAtomTreeNode> &node);

  static std::vector<std::shared_ptr<SKAtomTreeNode>> orderedSegmentTreeNodes(SKAtomTreeController &controller);
  static bool segmentTreeNodesAlignWithDrawRanges(SKAtomTreeController &controller, int drawRangeCount);
  static std::vector<std::shared_ptr<SKAtomTreeNode>> orderedResidueTreeNodes(SKAtomTreeController &controller);
  static bool residueTreeNodesAlignWithDrawRanges(SKAtomTreeController &controller, int drawRangeCount);

  /// The ribbon of a group and nothing else. Nesting is read back through `isRibbonResidueVisible`.
  static void setGroupRibbonVisibility(const std::shared_ptr<SKAtomTreeNode> &node, bool isVisible);
  /// The atoms under a group; switching the group is switching all of them.
  static void setGroupAtomsVisibility(const std::shared_ptr<SKAtomTreeNode> &node, bool isVisible);
  /// A group outside the ribbon hierarchy has no ribbon and draws a single box.
  static void setGroupVisibility(const std::shared_ptr<SKAtomTreeNode> &node, bool isVisible);
  /// Aggregate of descendant leaf visibility: true / false / nullopt (mixed).
  static std::optional<bool> groupAtomsVisibilityState(const std::shared_ptr<SKAtomTreeNode> &node);
  static bool isRibbonSegmentVisible(const std::shared_ptr<SKAtomTreeNode> &node);
  static bool isRibbonResidueVisible(const std::shared_ptr<SKAtomTreeNode> &node);

  static constexpr qint32 ribbonPickObjectType = 3;

  static std::shared_ptr<SKAtomTreeNode> treeNodeForSegment(int segmentIndex, SKAtomTreeController &controller);
  static std::shared_ptr<SKAtomTreeNode> treeNodeForResidue(int residueIndex, SKAtomTreeController &controller);
  static std::shared_ptr<SKAtomTreeNode> treeNodeForRibbonPick(int segmentIndex,
                                                                int residueIndex,
                                                                bool selectSegment,
                                                                SKAtomTreeController &controller);

  static std::set<std::shared_ptr<SKAtomTreeNode>> filterResidueTreeNodes(SKAtomTreeController &controller,
                                                                          const ProteinBackbone &backbone,
                                                                          double3 contentShift,
                                                                          simd_quatd orientation,
                                                                          double3 boundingBoxCenter,
                                                                          double3 origin,
                                                                          const std::function<bool(double3)> &filter);
  static std::set<std::shared_ptr<SKAtomTreeNode>> filterSegmentTreeNodes(SKAtomTreeController &controller,
                                                                          const ProteinBackbone &backbone,
                                                                          double3 contentShift,
                                                                          simd_quatd orientation,
                                                                          double3 boundingBoxCenter,
                                                                          double3 origin,
                                                                          const std::function<bool(double3)> &filter);

  static std::set<int> selectedSegmentDrawRangeIndices(SKAtomTreeController &controller);
  static std::set<int> selectedResidueDrawRangeIndices(SKAtomTreeController &controller);
  static std::vector<std::shared_ptr<SKAtomTreeNode>> residueGroupNodes(const std::shared_ptr<SKAtomTreeNode> &segmentNode);
  static bool isSecondaryStructureSegmentSelected(const std::shared_ptr<SKAtomTreeNode> &segmentNode,
                                                  const std::set<std::shared_ptr<SKAtomTreeNode>> &selectedNodes);
  static std::shared_ptr<SKAtomTreeNode> enclosingResidueGroupNode(const std::shared_ptr<SKAtomTreeNode> &leafNode);
  static std::shared_ptr<SKAtomTreeNode> enclosingSecondaryStructureSegmentNode(const std::shared_ptr<SKAtomTreeNode> &node);

  /// Resolves the residue group node for a Cα tag stored on the ribbon mesh (1:1 with residue draw ranges).
  static std::shared_ptr<SKAtomTreeNode> residueTreeNodeForAtomTag(qint64 tag, SKAtomTreeController &controller);
  /// Resolves the secondary-structure segment node for a Cα tag stored on the ribbon mesh.
  static std::shared_ptr<SKAtomTreeNode> segmentTreeNodeForAtomTag(qint64 tag, SKAtomTreeController &controller);
};
