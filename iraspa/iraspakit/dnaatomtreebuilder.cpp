/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "dnaatomtreebuilder.h"
#include "dnabackbone.h"
#include "proteinribbonsegmentsupport.h"
#include "sknucleotide.h"
#include <map>
#include <set>
#include <algorithm>

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

  void replaceRootNodes(SKAtomTreeController &controller, const std::vector<std::shared_ptr<SKAtomTreeNode>> &nodes)
  {
    for (const std::shared_ptr<SKAtomTreeNode> &root : controller.rootNodes())
    {
      controller.removeNode(root);
    }
    for (const std::shared_ptr<SKAtomTreeNode> &node : nodes)
    {
      controller.appendToRootnodes(node);
    }
  }

  bool hasChainHelixHierarchy(const std::vector<std::shared_ptr<SKAtomTreeNode>> &rootNodes)
  {
    for (const std::shared_ptr<SKAtomTreeNode> &rootNode : rootNodes)
    {
      if (!rootNode->isGroup()) continue;
      if (ProteinRibbonSegmentSupport::isChainGroupNode(rootNode))
      {
        for (const std::shared_ptr<SKAtomTreeNode> &helixNode : rootNode->childNodes())
        {
          if (!helixNode->isGroup()) continue;
          for (const std::shared_ptr<SKAtomTreeNode> &child : helixNode->childNodes())
          {
            if (child->isGroup()) return true;
          }
        }
      }
      else if (ProteinRibbonSegmentSupport::isDNAHelixGroupNode(rootNode))
      {
        for (const std::shared_ptr<SKAtomTreeNode> &child : rootNode->childNodes())
        {
          if (child->isGroup()) return true;
        }
      }
    }
    return false;
  }

  ResidueKey residueKey(const std::shared_ptr<SKAsymmetricAtom> &atom)
  {
    return {atom->chainIdentifier(), atom->residueSequenceNumber(), atom->codeForInsertionOfResidues()};
  }

  ResidueKey residueKey(QChar chainIdentifier, const DNABackboneResidue &residue)
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
    return trimmed.isEmpty() ? QStringLiteral("NUC") : trimmed;
  }

  QString residueDisplayName(const ResidueKey &key, const ResidueBucket &bucket)
  {
    const QString namePart = trimmedResidueName(bucket.residueName);
    QString label = QStringLiteral("%1 %2").arg(namePart).arg(key.residueSequenceNumber);
    if (key.codeForInsertionOfResidues != QChar(' ')) label += key.codeForInsertionOfResidues;
    return label;
  }

  QString chainDisplayName(QChar chainIdentifier)
  {
    if (chainIdentifier != QChar(' ')) return QStringLiteral("Chain %1").arg(chainIdentifier);
    return QStringLiteral("Chain");
  }

  QString helixDisplayName(const DNABackboneChain &chain, const std::map<ResidueKey, ResidueBucket> &residuesByKey)
  {
    if (chain.residues.empty()) return QStringLiteral("DNA helix");
    const ResidueKey firstKey = residueKey(chain.chainIdentifier, chain.residues.front());
    const ResidueKey lastKey = residueKey(chain.chainIdentifier, chain.residues.back());
    const QString firstName = trimmedResidueName(residuesByKey.at(firstKey).residueName);
    const QString lastName = trimmedResidueName(residuesByKey.at(lastKey).residueName);
    if (firstKey == lastKey)
    {
      return QStringLiteral("DNA helix (%1 %2)").arg(firstName).arg(firstKey.residueSequenceNumber);
    }
    return QStringLiteral("DNA helix (%1 %2–%3 %4)")
      .arg(firstName)
      .arg(firstKey.residueSequenceNumber)
      .arg(lastName)
      .arg(lastKey.residueSequenceNumber);
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
}

bool DNAAtomTreeBuilder::applyHierarchyIfNeeded(SKAtomTreeController &controller)
{
  const std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = controller.flattenedObjects();
  if (atoms.empty()) return false;
  if (DNABackbone::build(atoms).chains.empty()) return false;
  if (hasChainHelixHierarchy(controller.rootNodes())) return false;
  replaceRootNodes(controller, build(atoms));
  return true;
}

std::vector<std::shared_ptr<SKAtomTreeNode>> DNAAtomTreeBuilder::build(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms)
{
  const DNABackbone backbone = DNABackbone::build(atoms);

  std::map<ResidueKey, ResidueBucket> residuesByKey;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> orphanAtoms;
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : atoms)
  {
    if (!hasResidueIdentity(atom))
    {
      orphanAtoms.push_back(atom);
      continue;
    }
    const ResidueKey key = residueKey(atom);
    if (residuesByKey.find(key) == residuesByKey.end())
    {
      residuesByKey[key] = {atom->residueName(), {}};
    }
    residuesByKey[key].atoms.push_back(atom);
  }

  const bool useChainLevel = backbone.chains.size() > 1;
  std::vector<std::shared_ptr<SKAtomTreeNode>> rootNodes;

  for (const DNABackboneChain &chain : backbone.chains)
  {
    if (chain.residues.empty()) continue;

    std::shared_ptr<SKAtomTreeNode> chainNode;
    if (useChainLevel)
    {
      chainNode = makeGroupNode(chainDisplayName(chain.chainIdentifier), SKAtomTreeGroupKind::chain);
      rootNodes.push_back(chainNode);
    }

    const std::shared_ptr<SKAtomTreeNode> helixNode = makeGroupNode(helixDisplayName(chain, residuesByKey), SKAtomTreeGroupKind::dnaHelix);
    for (const DNABackboneResidue &residue : chain.residues)
    {
      const ResidueKey key = residueKey(chain.chainIdentifier, residue);
      const auto bucketIt = residuesByKey.find(key);
      if (bucketIt == residuesByKey.end()) continue;

      const std::shared_ptr<SKAtomTreeNode> residueNode = makeGroupNode(residueDisplayName(key, bucketIt->second), SKAtomTreeGroupKind::residue);
      std::vector<std::shared_ptr<SKAsymmetricAtom>> sortedAtoms = bucketIt->second.atoms;
      std::sort(sortedAtoms.begin(), sortedAtoms.end(), atomSortOrder);
      for (const std::shared_ptr<SKAsymmetricAtom> &atom : sortedAtoms)
      {
        std::make_shared<SKAtomTreeNode>(atom)->appendToParent(residueNode);
      }
      residueNode->appendToParent(helixNode);
    }

    if (chainNode) helixNode->appendToParent(chainNode);
    else rootNodes.push_back(helixNode);
  }

  std::set<ResidueKey> assignedKeys;
  for (const DNABackboneChain &chain : backbone.chains)
  {
    for (const DNABackboneResidue &residue : chain.residues)
    {
      assignedKeys.insert(residueKey(chain.chainIdentifier, residue));
    }
  }

  std::vector<ResidueKey> unassignedKeys;
  for (const auto &entry : residuesByKey)
  {
    if (!assignedKeys.count(entry.first) && SKNucleotide::isNucleotideResidueName(entry.second.residueName))
    {
      unassignedKeys.push_back(entry.first);
    }
  }
  std::sort(unassignedKeys.begin(), unassignedKeys.end());
  if (!unassignedKeys.empty())
  {
    const std::shared_ptr<SKAtomTreeNode> otherNode = makeGroupNode(QStringLiteral("Other nucleotides"), SKAtomTreeGroupKind::otherNucleotides);
    for (const ResidueKey &key : unassignedKeys)
    {
      const ResidueBucket &bucket = residuesByKey.at(key);
      const std::shared_ptr<SKAtomTreeNode> residueNode = makeGroupNode(residueDisplayName(key, bucket), SKAtomTreeGroupKind::residue);
      std::vector<std::shared_ptr<SKAsymmetricAtom>> sortedAtoms = bucket.atoms;
      std::sort(sortedAtoms.begin(), sortedAtoms.end(), atomSortOrder);
      for (const std::shared_ptr<SKAsymmetricAtom> &atom : sortedAtoms)
      {
        std::make_shared<SKAtomTreeNode>(atom)->appendToParent(residueNode);
      }
      residueNode->appendToParent(otherNode);
    }
    rootNodes.push_back(otherNode);
  }

  if (!orphanAtoms.empty())
  {
    const std::shared_ptr<SKAtomTreeNode> otherNode = makeGroupNode(QStringLiteral("Other"), SKAtomTreeGroupKind::other);
    std::vector<std::shared_ptr<SKAsymmetricAtom>> sortedOrphans = orphanAtoms;
    std::sort(sortedOrphans.begin(), sortedOrphans.end(), atomSortOrder);
    for (const std::shared_ptr<SKAsymmetricAtom> &atom : sortedOrphans)
    {
      std::make_shared<SKAtomTreeNode>(atom)->appendToParent(otherNode);
    }
    rootNodes.push_back(otherNode);
  }

  return rootNodes;
}
