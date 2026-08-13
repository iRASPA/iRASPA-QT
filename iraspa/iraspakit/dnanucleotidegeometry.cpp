/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "dnanucleotidegeometry.h"
#include "dnabackbone.h"
#include "sknucleotide.h"
#include <algorithm>
#include <cmath>
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

  struct MutableResidue
  {
    QString residueName;
    std::map<QString, std::shared_ptr<SKAsymmetricAtom>> atomsByName;
  };

  std::shared_ptr<SKAsymmetricAtom> atomNamed(const MutableResidue &residue, const QString &name)
  {
    const auto it = residue.atomsByName.find(SKNucleotide::normalizedAtomName(name));
    if (it == residue.atomsByName.end()) return nullptr;
    return it->second;
  }

  void collectRingAtoms(const MutableResidue &mutableResidue,
                        const std::vector<QString> &names,
                        std::vector<std::shared_ptr<SKAsymmetricAtom>> &out)
  {
    out.clear();
    out.reserve(names.size());
    for (const QString &name : names)
    {
      const std::shared_ptr<SKAsymmetricAtom> atom = atomNamed(mutableResidue, name);
      if (atom) out.push_back(atom);
    }
  }

  double3 shiftedPosition(const std::shared_ptr<SKAsymmetricAtom> &atom, double3 contentShift)
  {
    const double3 position = atom->position();
    return position + contentShift;
  }
}

std::vector<double3> DNANucleotideResidueGeometry::riboseRingPositions(double3 contentShift) const
{
  std::vector<double3> positions;
  positions.reserve(riboseRingAtoms.size());
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : riboseRingAtoms)
  {
    positions.push_back(shiftedPosition(atom, contentShift));
  }
  return positions;
}

std::vector<double3> DNANucleotideResidueGeometry::baseRingPositions(double3 contentShift) const
{
  std::vector<double3> positions;
  positions.reserve(baseRingAtoms.size());
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : baseRingAtoms)
  {
    positions.push_back(shiftedPosition(atom, contentShift));
  }
  return positions;
}

std::optional<double3> DNANucleotideResidueGeometry::c1PrimePosition(double3 contentShift) const
{
  if (!c1Prime) return std::nullopt;
  return shiftedPosition(c1Prime, contentShift);
}

std::optional<double3> DNANucleotideResidueGeometry::baseAnchorPosition(double3 contentShift) const
{
  if (!baseAnchor) return std::nullopt;
  return shiftedPosition(baseAnchor, contentShift);
}

std::optional<double3> DNANucleotideResidueGeometry::phosphatePosition(double3 contentShift) const
{
  if (!phosphate) return std::nullopt;
  return shiftedPosition(phosphate, contentShift);
}

DNANucleotideGeometry DNANucleotideGeometry::build(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms)
{
  std::map<ResidueKey, MutableResidue> residuesByKey;
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : atoms)
  {
    if (!atom || !SKNucleotide::isNucleotideResidueName(atom->residueName())) continue;
    const ResidueKey key{atom->chainIdentifier(), atom->residueSequenceNumber(), atom->codeForInsertionOfResidues()};
    MutableResidue &mutableResidue = residuesByKey[key];
    if (mutableResidue.residueName.isEmpty()) mutableResidue.residueName = atom->residueName();
    mutableResidue.atomsByName[SKNucleotide::normalizedAtomName(atom->displayName())] = atom;
  }

  DNANucleotideGeometry geometry;
  geometry.residues.reserve(residuesByKey.size());
  int globalIndex = 0;
  for (const auto &entry : residuesByKey)
  {
    const MutableResidue &mutableResidue = entry.second;
    DNANucleotideResidueGeometry residue;
    residue.chainIdentifier = entry.first.chainIdentifier;
    residue.residueSequenceNumber = entry.first.residueSequenceNumber;
    residue.codeForInsertionOfResidues = entry.first.codeForInsertionOfResidues;
    residue.residueName = mutableResidue.residueName;
    residue.globalResidueIndex = globalIndex++;
    residue.baseKind = SKNucleotideBase::baseKindFromResidueName(residue.residueName);
    residue.c1Prime = atomNamed(mutableResidue, QStringLiteral("C1'"));
    residue.phosphate = atomNamed(mutableResidue, QStringLiteral("P"));
    const QString anchorName = SKNucleotideBase::baseAnchorAtomName(residue.baseKind);
    if (!anchorName.isEmpty()) residue.baseAnchor = atomNamed(mutableResidue, anchorName);
    collectRingAtoms(mutableResidue, SKNucleotideBase::riboseRingAtomNames(), residue.riboseRingAtoms);
    collectRingAtoms(mutableResidue, SKNucleotideBase::baseRingAtomNames(residue.baseKind), residue.baseRingAtoms);
    geometry.residues.push_back(std::move(residue));
  }
  return geometry;
}

void DNANucleotideGeometry::assignGlobalResidueIndicesFromBackbone(DNANucleotideGeometry &geometry,
                                                                   const DNABackbone &backbone)
{
  int globalIndex = 0;
  for (const DNABackboneChain &chain : backbone.chains)
  {
    for (const DNABackboneResidue &backboneResidue : chain.residues)
    {
      if (!backboneResidue.ribbonCenterAtom()) continue;
      for (DNANucleotideResidueGeometry &geometryResidue : geometry.residues)
      {
        if (geometryResidue.chainIdentifier != chain.chainIdentifier) continue;
        if (geometryResidue.residueSequenceNumber != backboneResidue.residueSequenceNumber) continue;
        if (geometryResidue.codeForInsertionOfResidues != backboneResidue.codeForInsertionOfResidues) continue;
        geometryResidue.globalResidueIndex = globalIndex;
        break;
      }
      ++globalIndex;
    }
  }
}

std::vector<DNANucleotideBasePair> DNANucleotideGeometry::detectWatsonCrickPairs(const DNANucleotideGeometry &geometry)
{
  std::vector<DNANucleotideBasePair> pairs;
  const double minPairDistance = 7.0;
  const double maxPairDistance = 16.5;

  for (size_t indexA = 0; indexA < geometry.residues.size(); ++indexA)
  {
    const DNANucleotideResidueGeometry &residueA = geometry.residues[indexA];
    if (!residueA.baseAnchor) continue;
    const double3 anchorA = residueA.baseAnchor->position();
    std::optional<size_t> bestPartner;
    double bestDistanceSquared = maxPairDistance * maxPairDistance;

    for (size_t indexB = indexA + 1; indexB < geometry.residues.size(); ++indexB)
    {
      const DNANucleotideResidueGeometry &residueB = geometry.residues[indexB];
      if (residueB.chainIdentifier == residueA.chainIdentifier) continue;
      if (!residueB.baseAnchor) continue;
      if (!SKNucleotideBase::areWatsonCrickComplementary(residueA.residueName, residueB.residueName)) continue;
      const double3 delta = residueB.baseAnchor->position() - anchorA;
      const double distanceSquared = delta.length_squared();
      const double distance = std::sqrt(distanceSquared);
      if (distance < minPairDistance || distance > maxPairDistance) continue;
      if (distanceSquared < bestDistanceSquared)
      {
        bestDistanceSquared = distanceSquared;
        bestPartner = indexB;
      }
    }

    if (bestPartner.has_value())
    {
      pairs.push_back({static_cast<int>(indexA), static_cast<int>(*bestPartner)});
    }
  }
  return pairs;
}
