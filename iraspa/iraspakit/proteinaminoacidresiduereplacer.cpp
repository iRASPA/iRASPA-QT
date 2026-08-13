/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinAminoAcidResidueReplacer.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteinaminoacidresiduereplacer.h"
#include "proteinribbonsegmentsupport.h"
#include "ribbonstructureeditor.h"
#include "structure.h"
#include "protein.h"
#include "proteincrystal.h"
#include "atomviewer.h"
#include "bondviewer.h"
#include "atomstructureviewer.h"
#include "skaminoacididealgeometry.h"
#include "skelement.h"
#include <QDataStream>
#include <QBuffer>
#include <algorithm>

namespace
{
  std::optional<QString> normalizedResidueCode(const QString &residueName)
  {
    if (residueName == QString("MSE") || residueName == QString("SEC")) return QString("MET");
    if (residueName == QString("HSD") || residueName == QString("HSE") || residueName == QString("HSP")) return QString("HIS");
    if (residueName == QString("CYX") || residueName == QString("CYM")) return QString("CYS");
    if (residueName == QString("ASH")) return QString("ASP");
    if (residueName == QString("GLH")) return QString("GLU");
    return std::nullopt;
  }

  double drawRadiusForStructure(const std::shared_ptr<Structure> &structure, qint64 elementId)
  {
    if (AtomStructureViewer *viewer = dynamic_cast<AtomStructureViewer *>(structure.get()))
    {
      switch (viewer->atomRepresentationType())
      {
      case AtomStructureViewer::RepresentationType::vdw:
        return PredefinedElements::predefinedElements[elementId]._VDWRadius;
      case AtomStructureViewer::RepresentationType::sticks_and_balls:
        return PredefinedElements::predefinedElements[elementId]._covalentRadius;
      case AtomStructureViewer::RepresentationType::unity:
        return viewer->atomScaleFactor();
      }
    }
    return PredefinedElements::predefinedElements[elementId]._covalentRadius;
  }

  std::optional<std::tuple<double3, double3, double3>> backbonePositions(const std::vector<std::shared_ptr<SKAtomTreeNode>> &atomNodes)
  {
    std::optional<double3> nitrogenPosition;
    std::optional<double3> alphaCarbonPosition;
    std::optional<double3> carbonylPosition;

    for (const std::shared_ptr<SKAtomTreeNode> &atomNode : atomNodes)
    {
      const std::shared_ptr<SKAsymmetricAtom> atom = atomNode->representedObject();
      const std::optional<SKBackboneAtomRole> role = backboneAtomRole(atom);
      if (role.has_value())
      {
        switch (*role)
        {
        case SKBackboneAtomRole::nitrogen: nitrogenPosition = atom->position(); break;
        case SKBackboneAtomRole::alphaCarbon: alphaCarbonPosition = atom->position(); break;
        case SKBackboneAtomRole::carbonylCarbon: carbonylPosition = atom->position(); break;
        default: break;
        }
      }
      else
      {
        const QString atomName = atom->displayName().trimmed().toUpper();
        if (atomName == QString("N")) nitrogenPosition = atom->position();
        if (atomName == QString("CA")) alphaCarbonPosition = atom->position();
        if (atomName == QString("C")) carbonylPosition = atom->position();
      }
    }

    if (!nitrogenPosition.has_value() || !alphaCarbonPosition.has_value() || !carbonylPosition.has_value()) return std::nullopt;
    return std::make_tuple(*nitrogenPosition, *alphaCarbonPosition, *carbonylPosition);
  }

  std::shared_ptr<SKAtomTreeNode> makeAtomTreeNode(const QString &atomName,
                                                   const QString &residueCode,
                                                   const double3 &position,
                                                   const std::shared_ptr<SKAsymmetricAtom> &templateAtom,
                                                   const std::shared_ptr<SKAsymmetricAtom> &preservedAtom,
                                                   const std::shared_ptr<Structure> &structure,
                                                   SKColorSets &colorSets,
                                                   ForceFieldSets &forceFieldSets)
  {
    const QString definitionKey = residueCode + QString("+") + atomName.trimmed().toUpper();
    const auto elementIt = PredefinedElements::residueDefinitionsElement.find(definitionKey);
    const QString elementSymbol = elementIt != PredefinedElements::residueDefinitionsElement.end()
      ? elementIt->second
      : (preservedAtom ? preservedAtom->uniqueForceFieldName() : QString("C"));
    const auto atomicNumberIt = PredefinedElements::atomicNumberData.find(elementSymbol);
    const qint64 elementIdentifier = atomicNumberIt != PredefinedElements::atomicNumberData.end()
      ? atomicNumberIt->second
      : (preservedAtom ? preservedAtom->elementIdentifier() : 6);
    const QString uniqueForceFieldName = PredefinedElements::predefinedElements[elementIdentifier]._chemicalSymbol;

    const SKColorSet *colorSet = colorSets[structure->atomColorSchemeIdentifier()];
    QColor color = preservedAtom ? preservedAtom->color() : QColor(0, 0, 0);
    if (colorSet)
    {
      if (const QColor *schemeColor = (*colorSet)[uniqueForceFieldName]) color = *schemeColor;
    }

    const double drawRadius = drawRadiusForStructure(structure, elementIdentifier);
    ForceFieldSet *forceField = forceFieldSets[structure->atomForceFieldIdentifier()];
    double bondDistanceCriteria = preservedAtom ? preservedAtom->bondDistanceCriteria() : 1.0;
    if (forceField)
    {
      if (ForceFieldType *forceFieldType = (*forceField)[uniqueForceFieldName])
      {
        bondDistanceCriteria = forceFieldType->userDefinedRadius();
      }
    }

    auto atom = std::make_shared<SKAsymmetricAtom>(atomName, static_cast<int>(elementIdentifier));
    atom->setDisplayName(atomName);
    atom->setUniqueForceFieldName(uniqueForceFieldName);
    atom->setPosition(position);
    atom->setCharge(preservedAtom ? preservedAtom->charge() : 0.0);
    atom->setColor(color);
    atom->setDrawRadius(drawRadius);
    atom->setBondDistanceCriteria(bondDistanceCriteria);
    atom->setOccupancy(preservedAtom ? preservedAtom->occupancy() : templateAtom->occupancy());
    atom->setResidueName(residueCode);
    atom->setChainIdentifier(templateAtom->chainIdentifier().toLatin1());
    atom->setResidueSequenceNumber(templateAtom->residueSequenceNumber());
    atom->setCodeForInsertionOfResidues(templateAtom->codeForInsertionOfResidues().toLatin1());
    atom->setSegmentIdentifier(templateAtom->segmentIdentifier());
    const auto typeIt = PredefinedElements::residueDefinitionsType.find(definitionKey);
    atom->backBoneAtom(typeIt != PredefinedElements::residueDefinitionsType.end() && isBackboneAtomType(typeIt->second));

    if (Protein *protein = dynamic_cast<Protein *>(structure.get()))
    {
      protein->expandSymmetry(atom);
    }
    else if (ProteinCrystal *proteinCrystal = dynamic_cast<ProteinCrystal *>(structure.get()))
    {
      proteinCrystal->expandSymmetry(atom);
    }
    return std::make_shared<SKAtomTreeNode>(atom);
  }

  void updateResidueGroupDisplayName(const std::shared_ptr<SKAtomTreeNode> &residueNode,
                                     const QString &residueCode,
                                     const std::shared_ptr<SKAsymmetricAtom> &templateAtom)
  {
    QString label = QString("%1 %2").arg(residueCode).arg(templateAtom->residueSequenceNumber());
    if (templateAtom->codeForInsertionOfResidues() != QChar(' '))
    {
      label += templateAtom->codeForInsertionOfResidues();
    }
    residueNode->setDisplayName(label);
    residueNode->representedObject()->setDisplayName(label);
  }
}

bool ProteinAminoAcidResidueReplacer::isProteinStructure(const std::shared_ptr<Structure> &structure)
{
  return std::dynamic_pointer_cast<Protein>(structure) || std::dynamic_pointer_cast<ProteinCrystal>(structure);
}

bool ProteinAminoAcidResidueReplacer::isKnownAminoAcidResidueName(const QString &residueName)
{
  const QString trimmedName = residueName.trimmed().toUpper();
  if (SKAminoAcidIdealGeometry::idealCoordinates(trimmedName).has_value()) return true;
  const std::optional<QString> normalized = normalizedResidueCode(trimmedName);
  return normalized.has_value() && SKAminoAcidIdealGeometry::idealCoordinates(*normalized).has_value();
}

bool ProteinAminoAcidResidueReplacer::isAminoAcidResidueGroupNode(const std::shared_ptr<SKAtomTreeNode> &node)
{
  if (!node || !node->isGroup()) return false;
  const std::vector<std::shared_ptr<SKAtomTreeNode>> atomNodes = node->descendantLeafNodes();
  if (atomNodes.empty()) return false;
  const std::shared_ptr<SKAsymmetricAtom> referenceAtom = atomNodes.front()->representedObject();
  if (!isKnownAminoAcidResidueName(referenceAtom->residueName())) return false;
  return std::all_of(atomNodes.begin(), atomNodes.end(), [&](const std::shared_ptr<SKAtomTreeNode> &child)
  {
    const std::shared_ptr<SKAsymmetricAtom> atom = child->representedObject();
    return atom->chainIdentifier() == referenceAtom->chainIdentifier()
        && atom->residueSequenceNumber() == referenceAtom->residueSequenceNumber()
        && atom->codeForInsertionOfResidues() == referenceAtom->codeForInsertionOfResidues()
        && isKnownAminoAcidResidueName(atom->residueName());
  });
}

std::optional<ProteinAminoAcidResidueReplacer::ResidueContext> ProteinAminoAcidResidueReplacer::residueContext(const std::shared_ptr<SKAtomTreeNode> &clickedNode,
                                                                                                                SKAtomTreeController &controller)
{
  auto makeContext = [](const std::shared_ptr<SKAtomTreeNode> &residueNode,
                        const std::vector<std::shared_ptr<SKAtomTreeNode>> &atomNodes) -> std::optional<ResidueContext>
  {
    if (atomNodes.empty()) return std::nullopt;
    if (!isKnownAminoAcidResidueName(atomNodes.front()->representedObject()->residueName())) return std::nullopt;
    return ResidueContext{residueNode, atomNodes};
  };

  if (ProteinRibbonSegmentSupport::isResidueGroupNode(clickedNode) || isAminoAcidResidueGroupNode(clickedNode))
  {
    return makeContext(clickedNode, clickedNode->descendantLeafNodes());
  }

  if (const std::shared_ptr<SKAtomTreeNode> residueNode = ProteinRibbonSegmentSupport::enclosingResidueGroupNode(clickedNode))
  {
    return makeContext(residueNode, residueNode->descendantLeafNodes());
  }

  if (clickedNode && clickedNode->isGroup())
  {
    for (std::shared_ptr<SKAtomTreeNode> node = clickedNode->parent(); node; node = node->parent())
    {
      if (ProteinRibbonSegmentSupport::isResidueGroupNode(node) || isAminoAcidResidueGroupNode(node))
      {
        return makeContext(node, node->descendantLeafNodes());
      }
    }
  }

  if (!clickedNode || !clickedNode->isLeaf()) return std::nullopt;
  const std::shared_ptr<SKAsymmetricAtom> referenceAtom = clickedNode->representedObject();
  if (!isKnownAminoAcidResidueName(referenceAtom->residueName())) return std::nullopt;

  std::vector<std::shared_ptr<SKAtomTreeNode>> atomNodes;
  for (const std::shared_ptr<SKAtomTreeNode> &node : controller.flattenedLeafNodes())
  {
    const std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject();
    if (atom->chainIdentifier() == referenceAtom->chainIdentifier()
        && atom->residueSequenceNumber() == referenceAtom->residueSequenceNumber()
        && atom->codeForInsertionOfResidues() == referenceAtom->codeForInsertionOfResidues()
        && isKnownAminoAcidResidueName(atom->residueName()))
    {
      atomNodes.push_back(node);
    }
  }
  if (atomNodes.empty()) return std::nullopt;
  return ResidueContext{nullptr, atomNodes};
}

std::optional<QString> ProteinAminoAcidResidueReplacer::currentResidueCode(const std::vector<std::shared_ptr<SKAtomTreeNode>> &atomNodes)
{
  if (atomNodes.empty()) return std::nullopt;
  const QString trimmedName = atomNodes.front()->representedObject()->residueName().trimmed().toUpper();
  if (trimmedName.isEmpty()) return std::nullopt;
  return trimmedName;
}

std::optional<std::pair<std::shared_ptr<SKAtomTreeController>, std::shared_ptr<SKBondSetController>>> ProteinAminoAcidResidueReplacer::snapshotAtomBondState(const std::shared_ptr<Structure> &structure)
{
  if (!structure) return std::nullopt;
  structure->atomsTreeController()->setTags();
  structure->bondSetController()->setTags();

  QByteArray atomData;
  QBuffer atomBuffer(&atomData);
  atomBuffer.open(QIODevice::WriteOnly);
  QDataStream atomStream(&atomBuffer);
  atomStream << structure->atomsTreeController();

  QByteArray bondData;
  QBuffer bondBuffer(&bondData);
  bondBuffer.open(QIODevice::WriteOnly);
  QDataStream bondStream(&bondBuffer);
  bondStream << structure->bondSetController();

  QDataStream atomReader(atomData);
  auto atoms = std::make_shared<SKAtomTreeController>();
  atomReader >> atoms;

  QDataStream bondReader(bondData);
  auto bonds = std::make_shared<SKBondSetController>(atoms);
  bondReader >> bonds;

  return std::make_pair(atoms, bonds);
}

bool ProteinAminoAcidResidueReplacer::replaceResidue(const std::shared_ptr<Structure> &structure,
                                                     const std::shared_ptr<SKAtomTreeNode> &residueNode,
                                                     const std::vector<std::shared_ptr<SKAtomTreeNode>> &atomNodes,
                                                     const QString &newResidueCode,
                                                     SKColorSets &colorSets,
                                                     ForceFieldSets &forceFieldSets)
{
  if (!isProteinStructure(structure)) return false;
  AtomEditor *atomEditor = dynamic_cast<AtomEditor *>(structure.get());
  BondEditor *bondEditor = dynamic_cast<BondEditor *>(structure.get());
  if (!atomEditor || !bondEditor) return false;

  const QString trimmedNewCode = newResidueCode.trimmed().toUpper();
  if (!SKAminoAcidIdealGeometry::idealCoordinates(trimmedNewCode).has_value()) return false;

  const std::optional<std::tuple<double3, double3, double3>> backbone = backbonePositions(atomNodes);
  if (!backbone.has_value()) return false;

  const auto alignedCoordinates = SKAminoAcidIdealGeometry::alignedCoordinates(trimmedNewCode,
                                                                               std::get<0>(*backbone),
                                                                               std::get<1>(*backbone),
                                                                               std::get<2>(*backbone));
  if (!alignedCoordinates.has_value()) return false;

  const std::shared_ptr<SKAsymmetricAtom> templateAtom = atomNodes.front()->representedObject();
  std::map<QString, std::shared_ptr<SKAsymmetricAtom>> preservedProperties;
  for (const std::shared_ptr<SKAtomTreeNode> &node : atomNodes)
  {
    preservedProperties[node->representedObject()->displayName().trimmed().toUpper()] = node->representedObject();
  }

  std::vector<QString> atomNames;
  atomNames.reserve(alignedCoordinates->size());
  for (const auto &entry : *alignedCoordinates) atomNames.push_back(entry.first);
  std::sort(atomNames.begin(), atomNames.end());

  std::vector<std::shared_ptr<SKAtomTreeNode>> newAtomNodes;
  for (const QString &atomName : atomNames)
  {
    const auto positionIt = alignedCoordinates->find(atomName);
    if (positionIt == alignedCoordinates->end()) continue;
    const QString preservedKey = atomName.trimmed().toUpper();
    const auto preservedIt = preservedProperties.find(preservedKey);
    const std::shared_ptr<SKAsymmetricAtom> preservedAtom = preservedIt != preservedProperties.end() ? preservedIt->second : nullptr;
    newAtomNodes.push_back(makeAtomTreeNode(atomName,
                                            trimmedNewCode,
                                            positionIt->second,
                                            templateAtom,
                                            preservedAtom,
                                            structure,
                                            colorSets,
                                            forceFieldSets));
  }
  if (newAtomNodes.empty()) return false;

  if (residueNode)
  {
    for (const std::shared_ptr<SKAtomTreeNode> &child : residueNode->childNodes())
    {
      structure->atomsTreeController()->removeNode(child);
    }
    for (size_t index = 0; index < newAtomNodes.size(); ++index)
    {
      structure->atomsTreeController()->insertNodeInParent(newAtomNodes[index], residueNode, static_cast<int>(index));
    }
    updateResidueGroupDisplayName(residueNode, trimmedNewCode, templateAtom);
  }
  else
  {
    const std::shared_ptr<SKAtomTreeNode> parentNode = atomNodes.front()->parent();
    const int insertionIndex = atomNodes.front()->indexPath().lastIndex();
    std::vector<std::shared_ptr<SKAtomTreeNode>> sortedAtomNodes = atomNodes;
    std::sort(sortedAtomNodes.begin(), sortedAtomNodes.end(), [](const std::shared_ptr<SKAtomTreeNode> &lhs, const std::shared_ptr<SKAtomTreeNode> &rhs)
    {
      return lhs->indexPath() > rhs->indexPath();
    });
    for (const std::shared_ptr<SKAtomTreeNode> &atomNode : sortedAtomNodes)
    {
      structure->atomsTreeController()->removeNode(atomNode);
    }
    for (size_t offset = 0; offset < newAtomNodes.size(); ++offset)
    {
      structure->atomsTreeController()->insertNodeInParent(newAtomNodes[offset], parentNode, insertionIndex + static_cast<int>(offset));
    }
  }

  bondEditor->computeBonds();
  structure->atomsTreeController()->setTags();
  structure->bondSetController()->setTags();
  structure->reComputeBoundingBox();

  if (ProteinRibbonStructureEditor *ribbonEditor = dynamic_cast<ProteinRibbonStructureEditor *>(structure.get()))
  {
    ribbonEditor->rebuildBackbone();
  }

  structure->setRepresentationStyle(structure->atomRepresentationStyle(), colorSets);
  structure->setRepresentationColorSchemeIdentifier(structure->atomColorSchemeIdentifier(), colorSets);
  structure->setAtomForceFieldIdentifier(structure->atomForceFieldIdentifier(), forceFieldSets);

  return true;
}
