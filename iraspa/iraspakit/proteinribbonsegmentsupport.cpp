/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonSegmentSupport.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteinribbonsegmentsupport.h"
#include <algorithm>
#include <unordered_map>

namespace
{
  /// Node and outline ancestors must be visible. The controller hidden root is ignored.
  bool isRibbonHierarchyNodeVisible(const std::shared_ptr<SKAtomTreeNode> &node)
  {
    for (std::shared_ptr<SKAtomTreeNode> current = node; current; current = current->parent())
    {
      if (!current->parent()) break;
      if (std::shared_ptr<SKAsymmetricAtom> atom = current->representedObject())
      {
        if (!atom->isVisible()) return false;
      }
    }
    return true;
  }

  std::unordered_map<int, std::shared_ptr<SKAtomTreeNode>> leafNodesByTag(SKAtomTreeController &controller)
  {
    const std::vector<std::shared_ptr<SKAtomTreeNode>> leafNodes = controller.flattenedLeafNodes();
    std::unordered_map<int, std::shared_ptr<SKAtomTreeNode>> nodesByTag;
    nodesByTag.reserve(leafNodes.size());
    for (const std::shared_ptr<SKAtomTreeNode> &leafNode : leafNodes)
    {
      nodesByTag[static_cast<int>(leafNode->representedObject()->tag())] = leafNode;
    }
    return nodesByTag;
  }

  std::vector<std::shared_ptr<SKAsymmetricAtom>> backboneAlphaCarbonAtoms(const ProteinBackbone &backbone)
  {
    std::vector<std::shared_ptr<SKAsymmetricAtom>> alphaCarbons;
    for (const ProteinBackboneChain &chain : backbone.chains)
    {
      for (const ProteinBackboneResidue &residue : chain.residues)
      {
        if (residue.alphaCarbon) alphaCarbons.push_back(residue.alphaCarbon);
      }
    }
    return alphaCarbons;
  }
}

ProteinRibbonResidueSegment::ProteinRibbonResidueSegment(QChar chainIdentifier,
                                                         ProteinRibbonSecondaryStructure structureType,
                                                         int firstResidueIndex,
                                                         int lastResidueIndex):
  chainIdentifier(chainIdentifier),
  structureType(structureType),
  firstResidueIndex(firstResidueIndex),
  lastResidueIndex(lastResidueIndex)
{
}

std::vector<ProteinRibbonResidueSegment> ProteinRibbonSegmentSupport::residueSegments(const ProteinBackboneChain &chain,
                                                                                      double3 contentShift,
                                                                                      ProteinRibbonSecondaryStructureMethod secondaryStructureMethod)
{
  std::vector<ProteinBackboneResidue> residues;
  for (const ProteinBackboneResidue &residue : chain.residues)
  {
    if (residue.alphaCarbon) residues.push_back(residue);
  }
  if (residues.empty()) return {};

  const std::vector<ProteinRibbonSecondaryStructure> assignment =
    ProteinRibbonSecondaryStructureAssigner::assign(chain, contentShift, secondaryStructureMethod);
  if (assignment.empty()) return {};
  return residueSegments(assignment, chain.chainIdentifier);
}

std::vector<ProteinRibbonResidueSegment> ProteinRibbonSegmentSupport::residueSegments(const std::vector<ProteinRibbonSecondaryStructure> &assignment,
                                                                                      QChar chainIdentifier)
{
  if (assignment.empty()) return {};

  std::vector<ProteinRibbonResidueSegment> segments;
  ProteinRibbonSecondaryStructure currentType = assignment.front();
  int runStart = 0;

  auto appendRun = [&](int runEnd)
  {
    if (runEnd < runStart) return;
    segments.emplace_back(chainIdentifier, currentType, runStart, runEnd);
  };

  for (size_t index = 1; index < assignment.size(); ++index)
  {
    if (assignment[index] != currentType)
    {
      appendRun(static_cast<int>(index) - 1);
      currentType = assignment[index];
      runStart = static_cast<int>(index);
    }
  }
  appendRun(static_cast<int>(assignment.size()) - 1);
  return segments;
}

std::vector<ProteinRibbonResidueSegment> ProteinRibbonSegmentSupport::residueSegments(const ProteinBackbone &backbone,
                                                                                      double3 contentShift,
                                                                                      ProteinRibbonSecondaryStructureMethod secondaryStructureMethod)
{
  std::vector<ProteinRibbonResidueSegment> segments;
  for (const ProteinBackboneChain &chain : backbone.chains)
  {
    const auto chainSegments = residueSegments(chain, contentShift, secondaryStructureMethod);
    segments.insert(segments.end(), chainSegments.begin(), chainSegments.end());
  }
  return segments;
}

bool ProteinRibbonSegmentSupport::isChainGroupNode(const std::shared_ptr<SKAtomTreeNode> &node)
{
  return node && node->groupKind() == SKAtomTreeGroupKind::chain;
}

bool ProteinRibbonSegmentSupport::isSecondaryStructureSegmentNode(const std::shared_ptr<SKAtomTreeNode> &node)
{
  return node && node->groupKind() == SKAtomTreeGroupKind::secondaryStructureSegment;
}

bool ProteinRibbonSegmentSupport::isHetatmGroupNode(const std::shared_ptr<SKAtomTreeNode> &node)
{
  return node && node->groupKind() == SKAtomTreeGroupKind::hetatm;
}

bool ProteinRibbonSegmentSupport::isResidueGroupNode(const std::shared_ptr<SKAtomTreeNode> &node)
{
  return node && node->groupKind() == SKAtomTreeGroupKind::residue;
}

bool ProteinRibbonSegmentSupport::isDNAHelixGroupNode(const std::shared_ptr<SKAtomTreeNode> &node)
{
  return node && node->groupKind() == SKAtomTreeGroupKind::dnaHelix;
}

bool ProteinRibbonSegmentSupport::isProteinHierarchyGroupNode(const std::shared_ptr<SKAtomTreeNode> &node)
{
  if (isChainGroupNode(node) || isSecondaryStructureSegmentNode(node)) return true;
  // A residue of the polymer is swept into the ribbon; one under HETATM has only its atoms.
  if (!isResidueGroupNode(node)) return false;
  return isSecondaryStructureSegmentNode(node->parent());
}

bool ProteinRibbonSegmentSupport::isRibbonHierarchyGroupNode(const std::shared_ptr<SKAtomTreeNode> &node)
{
  if (isProteinHierarchyGroupNode(node)) return true;
  if (isDNAHelixGroupNode(node)) return true;
  if (!isResidueGroupNode(node)) return false;
  return isDNAHelixGroupNode(node->parent());
}

std::vector<std::shared_ptr<SKAtomTreeNode>> ProteinRibbonSegmentSupport::orderedSegmentTreeNodes(SKAtomTreeController &controller)
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> segmentNodes;
  for (const std::shared_ptr<SKAtomTreeNode> &rootNode : controller.rootNodes())
  {
    if (!rootNode->isGroup()) continue;
    if (isChainGroupNode(rootNode))
    {
      for (const std::shared_ptr<SKAtomTreeNode> &childNode : rootNode->childNodes())
      {
        if (isSecondaryStructureSegmentNode(childNode)) segmentNodes.push_back(childNode);
      }
    }
    else if (isSecondaryStructureSegmentNode(rootNode))
    {
      segmentNodes.push_back(rootNode);
    }
  }
  return segmentNodes;
}

bool ProteinRibbonSegmentSupport::segmentTreeNodesAlignWithDrawRanges(SKAtomTreeController &controller, int drawRangeCount)
{
  const std::vector<std::shared_ptr<SKAtomTreeNode>> segmentNodes = orderedSegmentTreeNodes(controller);
  return !segmentNodes.empty() && static_cast<int>(segmentNodes.size()) == drawRangeCount;
}

std::vector<std::shared_ptr<SKAtomTreeNode>> ProteinRibbonSegmentSupport::orderedResidueTreeNodes(SKAtomTreeController &controller)
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> residueNodes;
  for (const std::shared_ptr<SKAtomTreeNode> &segmentNode : orderedSegmentTreeNodes(controller))
  {
    for (const std::shared_ptr<SKAtomTreeNode> &childNode : segmentNode->childNodes())
    {
      if (isResidueGroupNode(childNode)) residueNodes.push_back(childNode);
    }
  }
  return residueNodes;
}

bool ProteinRibbonSegmentSupport::residueTreeNodesAlignWithDrawRanges(SKAtomTreeController &controller, int drawRangeCount)
{
  const std::vector<std::shared_ptr<SKAtomTreeNode>> residueNodes = orderedResidueTreeNodes(controller);
  return !residueNodes.empty() && static_cast<int>(residueNodes.size()) == drawRangeCount;
}

void ProteinRibbonSegmentSupport::setGroupRibbonVisibility(const std::shared_ptr<SKAtomTreeNode> &node, bool isVisible)
{
  if (!node) return;
  node->representedObject()->setVisibility(isVisible);
}

void ProteinRibbonSegmentSupport::setGroupAtomsVisibility(const std::shared_ptr<SKAtomTreeNode> &node, bool isVisible)
{
  if (!node) return;
  for (const std::shared_ptr<SKAtomTreeNode> &leaf : node->descendantLeafNodes())
  {
    leaf->representedObject()->setVisibility(isVisible);
  }
}

void ProteinRibbonSegmentSupport::setGroupVisibility(const std::shared_ptr<SKAtomTreeNode> &node, bool isVisible)
{
  if (!node) return;
  node->representedObject()->setVisibility(isVisible);
  for (const std::shared_ptr<SKAtomTreeNode> &descendant : node->descendantNodes())
  {
    descendant->representedObject()->setVisibility(isVisible);
  }
  setGroupAtomsVisibility(node, isVisible);
}

std::optional<bool> ProteinRibbonSegmentSupport::groupAtomsVisibilityState(const std::shared_ptr<SKAtomTreeNode> &node)
{
  if (!node) return std::nullopt;
  bool sawVisible = false;
  bool sawHidden = false;
  for (const std::shared_ptr<SKAtomTreeNode> &leaf : node->descendantLeafNodes())
  {
    if (leaf->representedObject()->isVisible()) sawVisible = true;
    else sawHidden = true;
    if (sawVisible && sawHidden) return std::nullopt;
  }
  return sawVisible || !sawHidden;
}

bool ProteinRibbonSegmentSupport::isRibbonSegmentVisible(const std::shared_ptr<SKAtomTreeNode> &node)
{
  return node && isRibbonHierarchyNodeVisible(node);
}

bool ProteinRibbonSegmentSupport::isRibbonResidueVisible(const std::shared_ptr<SKAtomTreeNode> &node)
{
  return node && isRibbonHierarchyNodeVisible(node);
}

std::shared_ptr<SKAtomTreeNode> ProteinRibbonSegmentSupport::treeNodeForSegment(int segmentIndex, SKAtomTreeController &controller)
{
  const std::vector<std::shared_ptr<SKAtomTreeNode>> segmentNodes = orderedSegmentTreeNodes(controller);
  if (segmentIndex < 0 || segmentIndex >= static_cast<int>(segmentNodes.size())) return nullptr;
  return segmentNodes[segmentIndex];
}

std::shared_ptr<SKAtomTreeNode> ProteinRibbonSegmentSupport::treeNodeForResidue(int residueIndex, SKAtomTreeController &controller)
{
  const std::vector<std::shared_ptr<SKAtomTreeNode>> residueNodes = orderedResidueTreeNodes(controller);
  if (residueIndex < 0 || residueIndex >= static_cast<int>(residueNodes.size())) return nullptr;
  return residueNodes[residueIndex];
}

std::shared_ptr<SKAtomTreeNode> ProteinRibbonSegmentSupport::treeNodeForRibbonPick(int segmentIndex,
                                                                                   int residueIndex,
                                                                                   bool selectSegment,
                                                                                   SKAtomTreeController &controller)
{
  if (selectSegment) return treeNodeForSegment(segmentIndex, controller);
  return treeNodeForResidue(residueIndex, controller);
}

std::set<std::shared_ptr<SKAtomTreeNode>> ProteinRibbonSegmentSupport::filterResidueTreeNodes(SKAtomTreeController &controller,
                                                                                              const ProteinBackbone &backbone,
                                                                                              double3 contentShift,
                                                                                              simd_quatd orientation,
                                                                                              double3 boundingBoxCenter,
                                                                                              double3 origin,
                                                                                              const std::function<bool(double3)> &filter)
{
  const std::vector<std::shared_ptr<SKAtomTreeNode>> residueNodes = orderedResidueTreeNodes(controller);
  const std::vector<std::shared_ptr<SKAsymmetricAtom>> alphaCarbons = backboneAlphaCarbonAtoms(backbone);
  if (residueNodes.size() != alphaCarbons.size()) return {};

  const double4x4 rotationMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(orientation), boundingBoxCenter);
  std::set<std::shared_ptr<SKAtomTreeNode>> selectedNodes;

  for (size_t index = 0; index < residueNodes.size(); ++index)
  {
    if (!isRibbonResidueVisible(residueNodes[index])) continue;
    const std::shared_ptr<SKAsymmetricAtom> alphaCarbon = alphaCarbons[index];
    bool isInside = false;
    for (const std::shared_ptr<SKAtomCopy> &copy : alphaCarbon->copies())
    {
      if (copy->type() != SKAtomCopy::AtomCopyType::copy) continue;
      const double3 position = copy->position() + contentShift;
      const double4 transformed = rotationMatrix * double4(position.x, position.y, position.z, 1.0);
      const double3 absoluteCartesianPosition = double3(transformed.x, transformed.y, transformed.z) + origin;
      if (filter(absoluteCartesianPosition))
      {
        isInside = true;
        break;
      }
    }
    if (isInside) selectedNodes.insert(residueNodes[index]);
  }
  return selectedNodes;
}

std::set<std::shared_ptr<SKAtomTreeNode>> ProteinRibbonSegmentSupport::filterSegmentTreeNodes(SKAtomTreeController &controller,
                                                                                              const ProteinBackbone &backbone,
                                                                                              double3 contentShift,
                                                                                              simd_quatd orientation,
                                                                                              double3 boundingBoxCenter,
                                                                                              double3 origin,
                                                                                              const std::function<bool(double3)> &filter)
{
  const std::set<std::shared_ptr<SKAtomTreeNode>> selectedResidues =
    filterResidueTreeNodes(controller, backbone, contentShift, orientation, boundingBoxCenter, origin, filter);
  std::set<std::shared_ptr<SKAtomTreeNode>> selectedSegments;
  for (const std::shared_ptr<SKAtomTreeNode> &residueNode : selectedResidues)
  {
    const std::shared_ptr<SKAtomTreeNode> parentNode = residueNode->parent();
    if (parentNode && isSecondaryStructureSegmentNode(parentNode))
    {
      selectedSegments.insert(parentNode);
    }
  }
  return selectedSegments;
}

std::set<int> ProteinRibbonSegmentSupport::selectedSegmentDrawRangeIndices(SKAtomTreeController &controller)
{
  const std::vector<std::shared_ptr<SKAtomTreeNode>> segmentNodes = orderedSegmentTreeNodes(controller);
  std::set<int> indices;
  for (const std::shared_ptr<SKAtomTreeNode> &selectedNode : controller.selectedTreeNodes())
  {
    if (isChainGroupNode(selectedNode))
    {
      for (const std::shared_ptr<SKAtomTreeNode> &childNode : selectedNode->childNodes())
      {
        if (!isSecondaryStructureSegmentNode(childNode)) continue;
        const auto it = std::find(segmentNodes.begin(), segmentNodes.end(), childNode);
        if (it != segmentNodes.end()) indices.insert(static_cast<int>(std::distance(segmentNodes.begin(), it)));
      }
    }
    else if (isSecondaryStructureSegmentNode(selectedNode))
    {
      const auto it = std::find(segmentNodes.begin(), segmentNodes.end(), selectedNode);
      if (it != segmentNodes.end()) indices.insert(static_cast<int>(std::distance(segmentNodes.begin(), it)));
    }
  }
  return indices;
}

std::set<int> ProteinRibbonSegmentSupport::selectedResidueDrawRangeIndices(SKAtomTreeController &controller)
{
  const std::vector<std::shared_ptr<SKAtomTreeNode>> residueNodes = orderedResidueTreeNodes(controller);
  std::set<int> indices;
  for (const std::shared_ptr<SKAtomTreeNode> &selectedNode : controller.selectedTreeNodes())
  {
    if (isResidueGroupNode(selectedNode))
    {
      const auto it = std::find(residueNodes.begin(), residueNodes.end(), selectedNode);
      if (it != residueNodes.end()) indices.insert(static_cast<int>(std::distance(residueNodes.begin(), it)));
    }
    else if (selectedNode->isLeaf())
    {
      const std::shared_ptr<SKAtomTreeNode> residueNode = enclosingResidueGroupNode(selectedNode);
      if (!residueNode) continue;
      const auto it = std::find(residueNodes.begin(), residueNodes.end(), residueNode);
      if (it != residueNodes.end()) indices.insert(static_cast<int>(std::distance(residueNodes.begin(), it)));
    }
  }
  return indices;
}

std::vector<std::shared_ptr<SKAtomTreeNode>> ProteinRibbonSegmentSupport::residueGroupNodes(const std::shared_ptr<SKAtomTreeNode> &segmentNode)
{
  if (!isSecondaryStructureSegmentNode(segmentNode)) return {};
  std::vector<std::shared_ptr<SKAtomTreeNode>> nodes;
  for (const std::shared_ptr<SKAtomTreeNode> &child : segmentNode->childNodes())
  {
    if (isResidueGroupNode(child)) nodes.push_back(child);
  }
  return nodes;
}

bool ProteinRibbonSegmentSupport::isSecondaryStructureSegmentSelected(const std::shared_ptr<SKAtomTreeNode> &segmentNode,
                                                                      const std::set<std::shared_ptr<SKAtomTreeNode>> &selectedNodes)
{
  if (!isSecondaryStructureSegmentNode(segmentNode)) return false;
  if (selectedNodes.count(segmentNode)) return true;
  const std::vector<std::shared_ptr<SKAtomTreeNode>> residues = residueGroupNodes(segmentNode);
  for (const std::shared_ptr<SKAtomTreeNode> &residue : residues)
  {
    if (selectedNodes.count(residue)) return true;
  }
  return false;
}

std::shared_ptr<SKAtomTreeNode> ProteinRibbonSegmentSupport::enclosingResidueGroupNode(const std::shared_ptr<SKAtomTreeNode> &leafNode)
{
  for (std::shared_ptr<SKAtomTreeNode> node = leafNode ? leafNode->parent() : nullptr; node; node = node->parent())
  {
    if (isResidueGroupNode(node)) return node;
  }
  return nullptr;
}

std::shared_ptr<SKAtomTreeNode> ProteinRibbonSegmentSupport::enclosingSecondaryStructureSegmentNode(const std::shared_ptr<SKAtomTreeNode> &node)
{
  for (std::shared_ptr<SKAtomTreeNode> candidate = node; candidate; candidate = candidate->parent())
  {
    if (isSecondaryStructureSegmentNode(candidate)) return candidate;
  }
  return nullptr;
}

std::vector<uint8_t> ProteinRibbonSegmentSupport::visibilityMaskForNodes(const std::vector<std::shared_ptr<SKAtomTreeNode>> &nodes)
{
  std::vector<uint8_t> visible;
  visible.reserve(nodes.size());
  for (const std::shared_ptr<SKAtomTreeNode> &node : nodes)
  {
    visible.push_back(!node || isRibbonHierarchyNodeVisible(node) ? 1 : 0);
  }
  return visible;
}

ProteinRibbonSegmentSupport::RibbonVisibilityMasks ProteinRibbonSegmentSupport::visibilityMasks(const std::vector<int> &residueAlphaCarbonTags,
                                                                                               const std::vector<int> &segmentAlphaCarbonTags,
                                                                                               SKAtomTreeController &controller)
{
  RibbonVisibilityMasks masks;
  if (residueAlphaCarbonTags.empty() && segmentAlphaCarbonTags.empty()) return masks;

  const std::unordered_map<int, std::shared_ptr<SKAtomTreeNode>> nodesByTag = leafNodesByTag(controller);

  auto maskForTags = [&nodesByTag](const std::vector<int> &tags,
                                   const std::function<std::shared_ptr<SKAtomTreeNode>(const std::shared_ptr<SKAtomTreeNode>&)> &enclosing)
  {
    std::vector<uint8_t> visible;
    visible.reserve(tags.size());
    for (int tag : tags)
    {
      const auto it = tag >= 0 ? nodesByTag.find(tag) : nodesByTag.end();
      const std::shared_ptr<SKAtomTreeNode> node = it != nodesByTag.end() ? enclosing(it->second) : nullptr;
      visible.push_back(!node || isRibbonHierarchyNodeVisible(node) ? 1 : 0);
    }
    return visible;
  };

  masks.residues = maskForTags(residueAlphaCarbonTags, enclosingResidueGroupNode);
  masks.segments = maskForTags(segmentAlphaCarbonTags, enclosingSecondaryStructureSegmentNode);
  return masks;
}
