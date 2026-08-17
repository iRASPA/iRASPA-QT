/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinAtomTreeBuilder.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteinatomtreebuilder.h"
#include "proteinbackbone.h"
#include "proteinribbonsegmentsupport.h"
#include "proteinribbonsecondarystructure.h"
#include "skelement.h"
#include "skasymmetricatom.h"
#include <QString>
#include <map>
#include <set>
#include <algorithm>
#include <utility>

namespace
{
  struct ResidueKey
  {
    QChar chainIdentifier;
    qint64 residueSequenceNumber;
    QChar codeForInsertionOfResidues;

    bool operator<(const ResidueKey &other) const
    {
      if (chainIdentifier != other.chainIdentifier) return chainIdentifier < other.chainIdentifier;
      if (residueSequenceNumber != other.residueSequenceNumber) return residueSequenceNumber < other.residueSequenceNumber;
      return codeForInsertionOfResidues < other.codeForInsertionOfResidues;
    }

    bool operator==(const ResidueKey &other) const
    {
      return chainIdentifier == other.chainIdentifier
          && residueSequenceNumber == other.residueSequenceNumber
          && codeForInsertionOfResidues == other.codeForInsertionOfResidues;
    }
  };

  struct ResidueBucket
  {
    QString residueName;
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms;
  };

  struct SecondaryStructureSegment
  {
    ProteinRibbonSecondaryStructure structureType;
    QChar chainIdentifier;
    std::vector<ResidueKey> residueKeys;
  };

  std::vector<std::shared_ptr<SKAsymmetricAtom>> leafAtoms(const SKAtomTreeController &controller)
  {
    const std::vector<std::shared_ptr<SKAtomTreeNode>> leaves = controller.flattenedLeafNodes();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms;
    atoms.reserve(leaves.size());
    for (const std::shared_ptr<SKAtomTreeNode> &node : leaves)
    {
      atoms.push_back(node->representedObject());
    }
    return atoms;
  }

  bool haveSameShape(const std::vector<std::shared_ptr<SKAtomTreeNode>> &left,
                     const std::vector<std::shared_ptr<SKAtomTreeNode>> &right)
  {
    if (left.size() != right.size()) return false;
    for (size_t index = 0; index < left.size(); ++index)
    {
      const std::shared_ptr<SKAtomTreeNode> &leftNode = left[index];
      const std::shared_ptr<SKAtomTreeNode> &rightNode = right[index];
      if (leftNode->groupKind() != rightNode->groupKind()) return false;
      if (leftNode->isGroup())
      {
        if (leftNode->displayName() != rightNode->displayName()) return false;
        if (!haveSameShape(leftNode->childNodes(), rightNode->childNodes())) return false;
      }
      else if (leftNode->representedObject() != rightNode->representedObject())
      {
        return false;
      }
    }
    return true;
  }

  std::set<QString> collectHiddenGroupPaths(const std::vector<std::shared_ptr<SKAtomTreeNode>> &nodes, const QString &prefix)
  {
    std::set<QString> hiddenPaths;
    for (const std::shared_ptr<SKAtomTreeNode> &node : nodes)
    {
      if (!node->isGroup()) continue;
      const QString path = prefix + QStringLiteral("/") + node->displayName();
      if (!node->representedObject()->isVisible())
      {
        hiddenPaths.insert(path);
      }
      const std::set<QString> childPaths = collectHiddenGroupPaths(node->childNodes(), path);
      hiddenPaths.insert(childPaths.begin(), childPaths.end());
    }
    return hiddenPaths;
  }

  void applyHiddenGroupPaths(const std::vector<std::shared_ptr<SKAtomTreeNode>> &nodes,
                             const QString &prefix,
                             const std::set<QString> &hiddenPaths)
  {
    for (const std::shared_ptr<SKAtomTreeNode> &node : nodes)
    {
      if (!node->isGroup()) continue;
      const QString path = prefix + QStringLiteral("/") + node->displayName();
      if (hiddenPaths.find(path) != hiddenPaths.end())
      {
        node->representedObject()->setVisibility(false);
      }
      applyHiddenGroupPaths(node->childNodes(), path, hiddenPaths);
    }
  }

  bool hasResidueChild(const std::shared_ptr<SKAtomTreeNode> &node)
  {
    for (const std::shared_ptr<SKAtomTreeNode> &child : node->childNodes())
    {
      if (ProteinRibbonSegmentSupport::isResidueGroupNode(child)) return true;
    }
    return false;
  }

  bool hasChainOrderedSegmentHierarchy(const std::vector<std::shared_ptr<SKAtomTreeNode>> &rootNodes)
  {
    for (const std::shared_ptr<SKAtomTreeNode> &rootNode : rootNodes)
    {
      if (!rootNode->isGroup()) continue;
      if (ProteinRibbonSegmentSupport::isChainGroupNode(rootNode))
      {
        for (const std::shared_ptr<SKAtomTreeNode> &segmentNode : rootNode->childNodes())
        {
          if (!segmentNode->isGroup()) continue;
          if (hasResidueChild(segmentNode)) return true;
        }
      }
      else if (hasResidueChild(rootNode))
      {
        return true;
      }
    }
    return false;
  }

  ResidueKey residueKey(const std::shared_ptr<SKAsymmetricAtom> &atom)
  {
    return {atom->chainIdentifier(), atom->residueSequenceNumber(), atom->codeForInsertionOfResidues()};
  }

  ResidueKey residueKey(QChar chainIdentifier, const ProteinBackboneResidue &residue)
  {
    return {chainIdentifier, residue.residueSequenceNumber, residue.codeForInsertionOfResidues};
  }

  bool hasResidueIdentity(const std::shared_ptr<SKAsymmetricAtom> &atom)
  {
    return !atom->residueName().trimmed().isEmpty() || atom->residueSequenceNumber() != 0;
  }

  bool atomSortOrder(const std::shared_ptr<SKAsymmetricAtom> &lhs, const std::shared_ptr<SKAsymmetricAtom> &rhs)
  {
    const QString leftName = lhs->displayName().toUpper();
    const QString rightName = rhs->displayName().toUpper();
    if (leftName != rightName) return leftName < rightName;
    return lhs->elementIdentifier() < rhs->elementIdentifier();
  }

  QString trimmedResidueName(const QString &residueName)
  {
    const QString trimmed = residueName.trimmed();
    return trimmed.isEmpty() ? QString("RES") : trimmed;
  }

  QString residueNumberLabel(const ResidueKey &key)
  {
    if (key.codeForInsertionOfResidues != QChar(' '))
    {
      return QString::number(key.residueSequenceNumber) + key.codeForInsertionOfResidues;
    }
    return QString::number(key.residueSequenceNumber);
  }

  QString chainDisplayName(QChar chainIdentifier)
  {
    if (chainIdentifier != QChar(' ')) return QString("Chain %1").arg(chainIdentifier);
    return QString("Chain");
  }

  std::shared_ptr<SKAtomTreeNode> makeGroupNode(const QString &displayName, SKAtomTreeGroupKind groupKind)
  {
    auto containerAtom = std::make_shared<SKAsymmetricAtom>(displayName, 0);
    containerAtom->setDisplayName(displayName);
    containerAtom->setColor(QColor(0, 0, 0));
    containerAtom->setDrawRadius(0.0);
    auto node = std::make_shared<SKAtomTreeNode>(containerAtom);
    node->setDisplayName(displayName);
    node->setGroupKind(groupKind);
    return node;
  }

  /// PDB marks every HETATM as solvent. Polymer MODRES written as HETATM (SET, etc.) still carry the
  /// peptide backbone and stay with the chain segments; waters, ions, and ligands go under HETATM.
  /// Water/solvent-agent residue names are also accepted so a lost solvent flag cannot drop them into coils.
  bool residueIsHetatmListing(const ResidueBucket &bucket)
  {
    if (bucket.atoms.empty()) return false;

    const QString residueName = bucket.residueName.trimmed().toUpper();
    const bool namedSolventResidue = PredefinedElements::isWaterResidueName(residueName)
                                  || PredefinedElements::isSolventAgentResidueName(residueName);
    bool allAtomsSolvent = true;
    for (const std::shared_ptr<SKAsymmetricAtom> &atom : bucket.atoms)
    {
      if (!atom->solvent())
      {
        allAtomsSolvent = false;
        break;
      }
    }
    if (!allAtomsSolvent && !namedSolventResidue) return false;

    bool hasNitrogen = false;
    bool hasAlphaCarbon = false;
    bool hasCarbonyl = false;
    for (const std::shared_ptr<SKAsymmetricAtom> &atom : bucket.atoms)
    {
      const QString atomName = atom->displayName().trimmed().toUpper();
      if (atomName == QString("N")) hasNitrogen = true;
      else if (atomName == QString("CA")) hasAlphaCarbon = true;
      else if (atomName == QString("C")) hasCarbonyl = true;
    }
    // Polymer MODRES written as HETATM still carry the peptide triad.
    return !(hasNitrogen && hasAlphaCarbon && hasCarbonyl);
  }

  QString residueDisplayName(const ResidueKey &key, const ResidueBucket &bucket)
  {
    const QString namePart = trimmedResidueName(bucket.residueName);
    QString label = QString("%1 %2").arg(namePart).arg(key.residueSequenceNumber);
    if (key.codeForInsertionOfResidues != QChar(' ')) label += key.codeForInsertionOfResidues;
    return label;
  }

  std::map<ResidueKey, ProteinRibbonSecondaryStructure> assignSecondaryStructure(const ProteinBackbone &backbone,
                                                                                 ProteinRibbonSecondaryStructureMethod secondaryStructureMethod)
  {
    std::map<ResidueKey, ProteinRibbonSecondaryStructure> assignmentByResidue;
    for (const ProteinBackboneChain &chain : backbone.chains)
    {
      std::vector<ProteinBackboneResidue> residuesWithAlphaCarbon;
      for (const ProteinBackboneResidue &residue : chain.residues)
      {
        if (residue.alphaCarbon) residuesWithAlphaCarbon.push_back(residue);
      }
      const std::vector<ProteinRibbonSecondaryStructure> assignment =
        ProteinRibbonSecondaryStructureAssigner::assign(chain, double3(0.0, 0.0, 0.0), secondaryStructureMethod);
      for (size_t index = 0; index < residuesWithAlphaCarbon.size() && index < assignment.size(); ++index)
      {
        assignmentByResidue[residueKey(chain.chainIdentifier, residuesWithAlphaCarbon[index])] = assignment[index];
      }
    }
    return assignmentByResidue;
  }

  QString segmentDisplayName(const SecondaryStructureSegment &segment, const std::map<ResidueKey, ResidueBucket> &residuesByKey)
  {
    QString typeLabel;
    switch (segment.structureType)
    {
    case ProteinRibbonSecondaryStructure::helix: typeLabel = QString("Alpha-helix"); break;
    case ProteinRibbonSecondaryStructure::sheet: typeLabel = QString("Beta-sheet"); break;
    case ProteinRibbonSecondaryStructure::coil: typeLabel = QString("Coil"); break;
    }
    if (segment.residueKeys.empty()) return typeLabel;
    const ResidueKey &firstKey = segment.residueKeys.front();
    const ResidueKey &lastKey = segment.residueKeys.back();
    const QString firstName = trimmedResidueName(residuesByKey.at(firstKey).residueName);
    const QString lastName = trimmedResidueName(residuesByKey.at(lastKey).residueName);
    const QString firstNumberLabel = residueNumberLabel(firstKey);
    const QString lastNumberLabel = residueNumberLabel(lastKey);
    if (firstKey == lastKey) return QString("%1 (%2 %3)").arg(typeLabel, firstName, firstNumberLabel);
    return QString("%1 (%2 %3–%4 %5)").arg(typeLabel, firstName, firstNumberLabel, lastName, lastNumberLabel);
  }

  std::shared_ptr<SKAtomTreeNode> makeResidueNode(const ResidueKey &key, const ResidueBucket &bucket)
  {
    const std::shared_ptr<SKAtomTreeNode> residueNode = makeGroupNode(residueDisplayName(key, bucket), SKAtomTreeGroupKind::residue);
    std::vector<std::shared_ptr<SKAsymmetricAtom>> sortedAtoms = bucket.atoms;
    std::sort(sortedAtoms.begin(), sortedAtoms.end(), atomSortOrder);
    for (const std::shared_ptr<SKAsymmetricAtom> &atom : sortedAtoms)
    {
      std::make_shared<SKAtomTreeNode>(atom)->appendToParent(residueNode);
    }
    return residueNode;
  }

  std::shared_ptr<SKAtomTreeNode> makeSegmentNode(const SecondaryStructureSegment &segment,
                                                  const std::map<ResidueKey, ResidueBucket> &residuesByKey)
  {
    const std::shared_ptr<SKAtomTreeNode> segmentNode = makeGroupNode(segmentDisplayName(segment, residuesByKey),
                                                                      SKAtomTreeGroupKind::secondaryStructureSegment);
    for (const ResidueKey &key : segment.residueKeys)
    {
      const auto bucketIt = residuesByKey.find(key);
      if (bucketIt == residuesByKey.end()) continue;
      makeResidueNode(key, bucketIt->second)->appendToParent(segmentNode);
    }
    return segmentNode;
  }

  std::shared_ptr<SKAtomTreeNode> makeHetatmGroupNode(const std::vector<ResidueKey> &keys,
                                                      const std::map<ResidueKey, ResidueBucket> &residuesByKey)
  {
    const std::shared_ptr<SKAtomTreeNode> hetatmNode = makeGroupNode(QString("HETATM"), SKAtomTreeGroupKind::hetatm);
    for (const ResidueKey &key : keys)
    {
      const auto bucketIt = residuesByKey.find(key);
      if (bucketIt == residuesByKey.end()) continue;
      makeResidueNode(key, bucketIt->second)->appendToParent(hetatmNode);
    }
    return hetatmNode;
  }

  void recordPlacedAtoms(const std::shared_ptr<SKAtomTreeNode> &node, std::set<const SKAsymmetricAtom*> &placedAtoms)
  {
    if (!node->isGroup())
    {
      placedAtoms.insert(node->representedObject().get());
      return;
    }
    for (const std::shared_ptr<SKAtomTreeNode> &child : node->childNodes())
    {
      recordPlacedAtoms(child, placedAtoms);
    }
  }

  std::map<QChar, std::vector<SecondaryStructureSegment>> buildSegmentsByChain(const ProteinBackbone &backbone,
                                                                             const std::map<ResidueKey, ResidueBucket> &residuesByKey,
                                                                             const std::map<ResidueKey, ProteinRibbonSecondaryStructure> &secondaryStructureByResidue,
                                                                             ProteinRibbonSecondaryStructureMethod secondaryStructureMethod)
  {
    (void)secondaryStructureMethod;
    std::map<QChar, std::vector<SecondaryStructureSegment>> segmentsByChain;
    std::set<ResidueKey> assignedKeys;

    for (const ProteinBackboneChain &chain : backbone.chains)
    {
      std::vector<ProteinBackboneResidue> residuesWithAlphaCarbon;
      for (const ProteinBackboneResidue &residue : chain.residues)
      {
        if (residue.alphaCarbon) residuesWithAlphaCarbon.push_back(residue);
      }

      std::vector<ProteinRibbonSecondaryStructure> assignment;
      assignment.reserve(residuesWithAlphaCarbon.size());
      for (const ProteinBackboneResidue &residue : residuesWithAlphaCarbon)
      {
        const ResidueKey key = residueKey(chain.chainIdentifier, residue);
        const auto it = secondaryStructureByResidue.find(key);
        assignment.push_back(it != secondaryStructureByResidue.end() ? it->second : ProteinRibbonSecondaryStructure::coil);
      }

      const std::vector<ProteinRibbonResidueSegment> runs = ProteinRibbonSegmentSupport::residueSegments(assignment, chain.chainIdentifier);
      std::vector<SecondaryStructureSegment> chainSegments;
      for (const ProteinRibbonResidueSegment &run : runs)
      {
        std::vector<ResidueKey> keys;
        for (int index = run.firstResidueIndex; index <= run.lastResidueIndex && index < static_cast<int>(residuesWithAlphaCarbon.size()); ++index)
        {
          const ResidueKey key = residueKey(chain.chainIdentifier, residuesWithAlphaCarbon[index]);
          if (residuesByKey.find(key) == residuesByKey.end()) continue;
          keys.push_back(key);
        }
        if (keys.empty()) continue;
        chainSegments.push_back({run.structureType, run.chainIdentifier, keys});
        assignedKeys.insert(keys.begin(), keys.end());
      }
      if (!chainSegments.empty()) segmentsByChain[chain.chainIdentifier] = chainSegments;
    }

    std::vector<ResidueKey> unassignedKeys;
    for (const auto &entry : residuesByKey)
    {
      if (!assignedKeys.count(entry.first) && !residueIsHetatmListing(entry.second))
      {
        unassignedKeys.push_back(entry.first);
      }
    }
    std::sort(unassignedKeys.begin(), unassignedKeys.end());
    for (const ResidueKey &key : unassignedKeys)
    {
      const auto it = secondaryStructureByResidue.find(key);
      const ProteinRibbonSecondaryStructure structureType = it != secondaryStructureByResidue.end() ? it->second : ProteinRibbonSecondaryStructure::coil;
      segmentsByChain[key.chainIdentifier].push_back({structureType, key.chainIdentifier, {key}});
    }
    return segmentsByChain;
  }
}

bool ProteinAtomTreeBuilder::applyHierarchyIfNeeded(SKAtomTreeController &controller,
                                                    ProteinRibbonSecondaryStructureMethod secondaryStructureMethod)
{
  const std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = leafAtoms(controller);
  if (atoms.empty()) return false;
  if (ProteinBackbone::build(atoms).chains.empty()) return false;

  std::vector<std::shared_ptr<SKAtomTreeNode>> rebuilt = build(atoms, secondaryStructureMethod);
  if (hasChainOrderedSegmentHierarchy(controller.rootNodes()) &&
      haveSameShape(controller.rootNodes(), rebuilt))
  {
    return false;
  }

  const std::set<QString> hiddenGroupPaths = collectHiddenGroupPaths(controller.rootNodes(), QString());
  controller.setRootNodes(std::move(rebuilt));
  if (!hiddenGroupPaths.empty())
  {
    applyHiddenGroupPaths(controller.rootNodes(), QString(), hiddenGroupPaths);
  }
  controller.setTags();
  return true;
}

std::vector<std::shared_ptr<SKAtomTreeNode>> ProteinAtomTreeBuilder::build(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms,
                                                                           ProteinRibbonSecondaryStructureMethod secondaryStructureMethod)
{
  const ProteinBackbone backbone = ProteinBackbone::build(atoms);
  const std::map<ResidueKey, ProteinRibbonSecondaryStructure> secondaryStructureByResidue =
    assignSecondaryStructure(backbone, secondaryStructureMethod);

  std::map<ResidueKey, ResidueBucket> residuesByKey;
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : atoms)
  {
    if (!hasResidueIdentity(atom)) continue;
    const ResidueKey key = residueKey(atom);
    if (residuesByKey.find(key) == residuesByKey.end())
    {
      residuesByKey[key] = {atom->residueName(), {}};
    }
    residuesByKey[key].atoms.push_back(atom);
  }

  std::map<ResidueKey, ResidueBucket> polymerResiduesByKey;
  std::map<QChar, std::vector<ResidueKey>> hetatmKeysByChain;
  for (const auto &entry : residuesByKey)
  {
    if (residueIsHetatmListing(entry.second)) hetatmKeysByChain[entry.first.chainIdentifier].push_back(entry.first);
    else polymerResiduesByKey[entry.first] = entry.second;
  }

  const std::map<QChar, std::vector<SecondaryStructureSegment>> segmentsByChain =
    buildSegmentsByChain(backbone, polymerResiduesByKey, secondaryStructureByResidue, secondaryStructureMethod);

  std::vector<QChar> chainOrder;
  std::set<QChar> seenChains;
  for (const ProteinBackboneChain &chain : backbone.chains)
  {
    if (seenChains.insert(chain.chainIdentifier).second) chainOrder.push_back(chain.chainIdentifier);
  }
  // Polymer residues can exist on chains the ribbon backbone never saw (incomplete residues without an
  // alpha-carbon). Keep those chains so a hierarchy rebuild never drops atoms — dropped atoms leave stale
  // bond tags and crash on archive copy/decode.
  for (const auto &entry : polymerResiduesByKey)
  {
    if (seenChains.insert(entry.first.chainIdentifier).second) chainOrder.push_back(entry.first.chainIdentifier);
  }
  for (const auto &entry : hetatmKeysByChain)
  {
    if (seenChains.insert(entry.first).second) chainOrder.push_back(entry.first);
  }

  const bool useChainLevel = chainOrder.size() > 1;

  std::vector<std::shared_ptr<SKAtomTreeNode>> rootNodes;
  std::set<const SKAsymmetricAtom*> placedAtoms;
  for (QChar chainIdentifier : chainOrder)
  {
    const auto segmentsIt = segmentsByChain.find(chainIdentifier);
    const auto hetatmIt = hetatmKeysByChain.find(chainIdentifier);
    const bool hasSegments = segmentsIt != segmentsByChain.end() && !segmentsIt->second.empty();
    const bool hasHetatm = hetatmIt != hetatmKeysByChain.end() && !hetatmIt->second.empty();
    if (!hasSegments && !hasHetatm) continue;

    std::shared_ptr<SKAtomTreeNode> chainNode;
    if (useChainLevel)
    {
      chainNode = makeGroupNode(chainDisplayName(chainIdentifier), SKAtomTreeGroupKind::chain);
      rootNodes.push_back(chainNode);
    }

    if (hasSegments)
    {
      for (const SecondaryStructureSegment &segment : segmentsIt->second)
      {
        const std::shared_ptr<SKAtomTreeNode> segmentNode = makeSegmentNode(segment, polymerResiduesByKey);
        recordPlacedAtoms(segmentNode, placedAtoms);
        if (chainNode) segmentNode->appendToParent(chainNode);
        else rootNodes.push_back(segmentNode);
      }
    }

    if (hasHetatm)
    {
      const std::shared_ptr<SKAtomTreeNode> hetatmNode = makeHetatmGroupNode(hetatmIt->second, residuesByKey);
      recordPlacedAtoms(hetatmNode, placedAtoms);
      if (chainNode) hetatmNode->appendToParent(chainNode);
      else rootNodes.push_back(hetatmNode);
    }
  }

  std::vector<std::shared_ptr<SKAsymmetricAtom>> leftoverAtoms;
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : atoms)
  {
    if (!placedAtoms.count(atom.get())) leftoverAtoms.push_back(atom);
  }

  if (!leftoverAtoms.empty())
  {
    const std::shared_ptr<SKAtomTreeNode> otherNode = makeGroupNode(QString("Other"), SKAtomTreeGroupKind::other);
    std::sort(leftoverAtoms.begin(), leftoverAtoms.end(), atomSortOrder);
    for (const std::shared_ptr<SKAsymmetricAtom> &atom : leftoverAtoms)
    {
      std::make_shared<SKAtomTreeNode>(atom)->appendToParent(otherNode);
    }
    rootNodes.push_back(otherNode);
  }

  return rootNodes;
}
