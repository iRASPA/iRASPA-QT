/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "dnabackbone.h"
#include "sknucleotide.h"
#include <algorithm>
#include <map>

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
  };

  std::optional<DNABackboneAtomRole> dnaBackboneAtomRoleForName(const QString &atomName)
  {
    const QString name = SKNucleotide::normalizedAtomName(atomName);
    if (name == QStringLiteral("P")) return DNABackboneAtomRole::phosphate;
    if (name == QStringLiteral("O5'")) return DNABackboneAtomRole::o5Prime;
    if (name == QStringLiteral("C5'")) return DNABackboneAtomRole::c5Prime;
    if (name == QStringLiteral("C4'")) return DNABackboneAtomRole::c4Prime;
    if (name == QStringLiteral("O4'")) return DNABackboneAtomRole::o4Prime;
    if (name == QStringLiteral("C1'")) return DNABackboneAtomRole::c1Prime;
    if (name == QStringLiteral("C2'")) return DNABackboneAtomRole::c2Prime;
    if (name == QStringLiteral("C3'")) return DNABackboneAtomRole::c3Prime;
    if (name == QStringLiteral("O3'")) return DNABackboneAtomRole::o3Prime;
    return std::nullopt;
  }

  struct MutableResidue
  {
    QString residueName;
    std::shared_ptr<SKAsymmetricAtom> phosphate;
    std::shared_ptr<SKAsymmetricAtom> o5Prime;
    std::shared_ptr<SKAsymmetricAtom> c5Prime;
    std::shared_ptr<SKAsymmetricAtom> c4Prime;
    std::shared_ptr<SKAsymmetricAtom> o4Prime;
    std::shared_ptr<SKAsymmetricAtom> c1Prime;
    std::shared_ptr<SKAsymmetricAtom> c2Prime;
    std::shared_ptr<SKAsymmetricAtom> c3Prime;
    std::shared_ptr<SKAsymmetricAtom> o3Prime;
  };

  void assignRole(MutableResidue &residue, DNABackboneAtomRole role, const std::shared_ptr<SKAsymmetricAtom> &atom)
  {
    switch (role)
    {
    case DNABackboneAtomRole::phosphate: residue.phosphate = atom; break;
    case DNABackboneAtomRole::o5Prime: residue.o5Prime = atom; break;
    case DNABackboneAtomRole::c5Prime: residue.c5Prime = atom; break;
    case DNABackboneAtomRole::c4Prime: residue.c4Prime = atom; break;
    case DNABackboneAtomRole::o4Prime: residue.o4Prime = atom; break;
    case DNABackboneAtomRole::c1Prime: residue.c1Prime = atom; break;
    case DNABackboneAtomRole::c2Prime: residue.c2Prime = atom; break;
    case DNABackboneAtomRole::c3Prime: residue.c3Prime = atom; break;
    case DNABackboneAtomRole::o3Prime: residue.o3Prime = atom; break;
    }
  }

  DNABackboneResidue makeResidue(const ResidueKey &key, const MutableResidue &mutableResidue)
  {
    return DNABackboneResidue(mutableResidue.residueName,
                              key.residueSequenceNumber,
                              key.codeForInsertionOfResidues,
                              mutableResidue.phosphate,
                              mutableResidue.o5Prime,
                              mutableResidue.c5Prime,
                              mutableResidue.c4Prime,
                              mutableResidue.o4Prime,
                              mutableResidue.c1Prime,
                              mutableResidue.c2Prime,
                              mutableResidue.c3Prime,
                              mutableResidue.o3Prime);
  }
}

std::optional<DNABackboneAtomRole> dnaBackboneAtomRole(const std::shared_ptr<SKAsymmetricAtom> &atom)
{
  if (!atom || !SKNucleotide::isNucleotideResidueName(atom->residueName())) return std::nullopt;
  return dnaBackboneAtomRoleForName(atom->displayName());
}

DNABackboneResidue::DNABackboneResidue(QString residueName,
                                       qint64 residueSequenceNumber,
                                       QChar codeForInsertionOfResidues,
                                       std::shared_ptr<SKAsymmetricAtom> phosphate,
                                       std::shared_ptr<SKAsymmetricAtom> o5Prime,
                                       std::shared_ptr<SKAsymmetricAtom> c5Prime,
                                       std::shared_ptr<SKAsymmetricAtom> c4Prime,
                                       std::shared_ptr<SKAsymmetricAtom> o4Prime,
                                       std::shared_ptr<SKAsymmetricAtom> c1Prime,
                                       std::shared_ptr<SKAsymmetricAtom> c2Prime,
                                       std::shared_ptr<SKAsymmetricAtom> c3Prime,
                                       std::shared_ptr<SKAsymmetricAtom> o3Prime):
  residueName(std::move(residueName)),
  residueSequenceNumber(residueSequenceNumber),
  codeForInsertionOfResidues(codeForInsertionOfResidues),
  phosphate(std::move(phosphate)),
  o5Prime(std::move(o5Prime)),
  c5Prime(std::move(c5Prime)),
  c4Prime(std::move(c4Prime)),
  o4Prime(std::move(o4Prime)),
  c1Prime(std::move(c1Prime)),
  c2Prime(std::move(c2Prime)),
  c3Prime(std::move(c3Prime)),
  o3Prime(std::move(o3Prime))
{
}

std::shared_ptr<SKAsymmetricAtom> DNABackboneResidue::ribbonCenterAtom() const
{
  if (phosphate) return phosphate;
  if (c1Prime) return c1Prime;
  if (c4Prime) return c4Prime;
  return nullptr;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> DNABackboneResidue::backboneAtoms() const
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms;
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : {phosphate, o5Prime, c5Prime, c4Prime, o4Prime, c1Prime, c3Prime, o3Prime})
  {
    if (atom) atoms.push_back(atom);
  }
  return atoms;
}

DNABackboneChain::DNABackboneChain(QChar chainIdentifier, std::vector<DNABackboneResidue> residues):
  chainIdentifier(chainIdentifier),
  residues(std::move(residues))
{
}

int DNABackbone::nucleotideResidueCount() const
{
  int count = 0;
  for (const DNABackboneChain &chain : chains)
  {
    for (const DNABackboneResidue &residue : chain.residues)
    {
      if (residue.ribbonCenterAtom()) ++count;
    }
  }
  return count;
}

DNABackbone DNABackbone::build(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms)
{
  std::vector<QChar> chainOrder;
  std::map<QChar, std::map<ResidueKey, MutableResidue>> residuesByChain;
  std::map<QChar, std::vector<ResidueKey>> residueOrderByChain;

  for (const std::shared_ptr<SKAsymmetricAtom> &atom : atoms)
  {
    if (!SKNucleotide::isNucleotideResidueName(atom->residueName())) continue;
    const std::optional<DNABackboneAtomRole> role = dnaBackboneAtomRole(atom);
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
      residueMap[key] = MutableResidue{atom->residueName(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    }

    assignRole(residueMap[key], *role, atom);
  }

  DNABackbone backbone;
  for (const QChar chainId : chainOrder)
  {
    const auto mapIt = residuesByChain.find(chainId);
    const auto orderIt = residueOrderByChain.find(chainId);
    if (mapIt == residuesByChain.end() || orderIt == residueOrderByChain.end()) continue;

    std::vector<ResidueKey> sortedKeys = orderIt->second;
    std::sort(sortedKeys.begin(), sortedKeys.end());

    std::vector<DNABackboneResidue> residues;
    for (const ResidueKey &key : sortedKeys)
    {
      const auto residueIt = mapIt->second.find(key);
      if (residueIt == mapIt->second.end()) continue;
      DNABackboneResidue residue = makeResidue(key, residueIt->second);
      if (residue.ribbonCenterAtom()) residues.push_back(std::move(residue));
    }
    if (!residues.empty())
    {
      backbone.chains.emplace_back(chainId, std::move(residues));
    }
  }
  return backbone;
}

ProteinBackbone DNABackbone::toProteinBackbone() const
{
  std::vector<ProteinBackboneChain> proteinChains;
  proteinChains.reserve(chains.size());
  for (const DNABackboneChain &chain : chains)
  {
    std::vector<ProteinBackboneResidue> proteinResidues;
    proteinResidues.reserve(chain.residues.size());
    for (const DNABackboneResidue &residue : chain.residues)
    {
      if (!residue.ribbonCenterAtom()) continue;
      proteinResidues.emplace_back(residue.residueName,
                                   residue.residueSequenceNumber,
                                   residue.codeForInsertionOfResidues,
                                   residue.c2Prime ? residue.c2Prime : residue.o4Prime,
                                   residue.ribbonCenterAtom(),
                                   residue.c3Prime ? residue.c3Prime : residue.c5Prime,
                                   residue.c2Prime ? residue.c2Prime : residue.o3Prime);
    }
    if (!proteinResidues.empty())
    {
      proteinChains.emplace_back(chain.chainIdentifier, std::move(proteinResidues));
    }
  }
  return ProteinBackbone(std::move(proteinChains));
}
