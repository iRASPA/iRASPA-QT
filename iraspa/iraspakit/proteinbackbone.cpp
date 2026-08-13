/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinBackbone.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteinbackbone.h"
#include "skelement.h"
#include <map>
#include <algorithm>

std::optional<SKBackboneAtomRole> backboneAtomRoleForType(const QString &type)
{
  const QString trimmed = type.trimmed();
  if (trimmed == QString("NH1")) return SKBackboneAtomRole::nitrogen;
  if (trimmed == QString("CH1E") || trimmed == QString("CH2G")) return SKBackboneAtomRole::alphaCarbon;
  if (trimmed == QString("C")) return SKBackboneAtomRole::carbonylCarbon;
  if (trimmed == QString("O")) return SKBackboneAtomRole::carbonylOxygen;
  return std::nullopt;
}

std::optional<SKBackboneAtomRole> backboneAtomRole(const std::shared_ptr<SKAsymmetricAtom> &atom)
{
  if (!atom) return std::nullopt;
  const QString key = atom->residueName().trimmed().toUpper() + QString("+") + atom->displayName().trimmed().toUpper();
  const auto typeIt = PredefinedElements::residueDefinitionsType.find(key);
  if (typeIt == PredefinedElements::residueDefinitionsType.end()) return std::nullopt;
  return backboneAtomRoleForType(typeIt->second);
}

bool isBackboneAtomType(const QString &type)
{
  return backboneAtomRoleForType(type).has_value();
}

ProteinBackboneResidue::ProteinBackboneResidue(QString residueName,
                                               qint64 residueSequenceNumber,
                                               QChar codeForInsertionOfResidues,
                                               std::shared_ptr<SKAsymmetricAtom> nitrogen,
                                               std::shared_ptr<SKAsymmetricAtom> alphaCarbon,
                                               std::shared_ptr<SKAsymmetricAtom> carbonylCarbon,
                                               std::shared_ptr<SKAsymmetricAtom> carbonylOxygen):
  residueName(std::move(residueName)),
  residueSequenceNumber(residueSequenceNumber),
  codeForInsertionOfResidues(codeForInsertionOfResidues),
  nitrogen(std::move(nitrogen)),
  alphaCarbon(std::move(alphaCarbon)),
  carbonylCarbon(std::move(carbonylCarbon)),
  carbonylOxygen(std::move(carbonylOxygen))
{
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> ProteinBackboneResidue::backboneAtoms() const
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms;
  if (nitrogen) atoms.push_back(nitrogen);
  if (alphaCarbon) atoms.push_back(alphaCarbon);
  if (carbonylCarbon) atoms.push_back(carbonylCarbon);
  if (carbonylOxygen) atoms.push_back(carbonylOxygen);
  return atoms;
}

ProteinBackboneChain::ProteinBackboneChain(QChar chainIdentifier, std::vector<ProteinBackboneResidue> residues):
  chainIdentifier(chainIdentifier), residues(std::move(residues))
{
}

ProteinBackbone::ProteinBackbone(std::vector<ProteinBackboneChain> chains): chains(std::move(chains))
{
}

int ProteinBackbone::alphaCarbonResidueCount() const
{
  int count = 0;
  for (const ProteinBackboneChain &chain : chains)
  {
    for (const ProteinBackboneResidue &residue : chain.residues)
    {
      if (residue.alphaCarbon) ++count;
    }
  }
  return count;
}

ProteinBackbone ProteinBackbone::build(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms)
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
  };

  struct MutableResidue
  {
    QString residueName;
    std::shared_ptr<SKAsymmetricAtom> nitrogen;
    std::shared_ptr<SKAsymmetricAtom> alphaCarbon;
    std::shared_ptr<SKAsymmetricAtom> carbonylCarbon;
    std::shared_ptr<SKAsymmetricAtom> carbonylOxygen;
  };

  std::vector<QChar> chainOrder;
  std::map<QChar, std::map<ResidueKey, MutableResidue>> residuesByChain;
  std::map<QChar, std::vector<ResidueKey>> residueOrderByChain;

  for (const std::shared_ptr<SKAsymmetricAtom> &atom : atoms)
  {
    const std::optional<SKBackboneAtomRole> role = backboneAtomRole(atom);
    if (!role.has_value()) continue;
    atom->backBoneAtom(true);

    const ResidueKey key{atom->chainIdentifier(), atom->residueSequenceNumber(), atom->codeForInsertionOfResidues()};
    if (residuesByChain.find(atom->chainIdentifier()) == residuesByChain.end())
    {
      chainOrder.push_back(atom->chainIdentifier());
      residuesByChain[atom->chainIdentifier()] = {};
      residueOrderByChain[atom->chainIdentifier()] = {};
    }

    auto &residueMap = residuesByChain[atom->chainIdentifier()];
    if (residueMap.find(key) == residueMap.end())
    {
      residueOrderByChain[atom->chainIdentifier()].push_back(key);
      residueMap[key] = MutableResidue{atom->residueName(), nullptr, nullptr, nullptr, nullptr};
    }

    MutableResidue &residue = residueMap[key];
    switch (*role)
    {
    case SKBackboneAtomRole::nitrogen:
      residue.nitrogen = atom;
      break;
    case SKBackboneAtomRole::alphaCarbon:
      residue.alphaCarbon = atom;
      break;
    case SKBackboneAtomRole::carbonylCarbon:
      residue.carbonylCarbon = atom;
      break;
    case SKBackboneAtomRole::carbonylOxygen:
    {
      const QString atomName = atom->displayName().trimmed().toUpper();
      if (residue.carbonylOxygen)
      {
        const QString existingName = residue.carbonylOxygen->displayName().trimmed().toUpper();
        if (existingName != QString("O") || atomName != QString("OXT"))
        {
          residue.carbonylOxygen = atom;
        }
      }
      else
      {
        residue.carbonylOxygen = atom;
      }
      break;
    }
    }
  }

  ProteinBackbone backbone;
  for (const QChar chainId : chainOrder)
  {
    const auto mapIt = residuesByChain.find(chainId);
    const auto orderIt = residueOrderByChain.find(chainId);
    if (mapIt == residuesByChain.end() || orderIt == residueOrderByChain.end()) continue;

    std::vector<ResidueKey> sortedKeys = orderIt->second;
    std::sort(sortedKeys.begin(), sortedKeys.end(), [](const ResidueKey &a, const ResidueKey &b)
    {
      if (a.residueSequenceNumber != b.residueSequenceNumber) return a.residueSequenceNumber < b.residueSequenceNumber;
      return a.codeForInsertionOfResidues < b.codeForInsertionOfResidues;
    });

    std::vector<ProteinBackboneResidue> residues;
    for (const ResidueKey &key : sortedKeys)
    {
      const auto residueIt = mapIt->second.find(key);
      if (residueIt == mapIt->second.end()) continue;
      const MutableResidue &mutableResidue = residueIt->second;
      residues.emplace_back(mutableResidue.residueName,
                            key.residueSequenceNumber,
                            key.codeForInsertionOfResidues,
                            mutableResidue.nitrogen,
                            mutableResidue.alphaCarbon,
                            mutableResidue.carbonylCarbon,
                            mutableResidue.carbonylOxygen);
    }
    backbone.chains.emplace_back(chainId, std::move(residues));
  }

  return backbone;
}
