/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinBackbone.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <QString>
#include <QChar>
#include <vector>
#include <memory>
#include <optional>
#include <skasymmetricatom.h>
#include "skelement.h"

std::optional<SKBackboneAtomRole> backboneAtomRoleForType(const QString &type);
std::optional<SKBackboneAtomRole> backboneAtomRole(const std::shared_ptr<SKAsymmetricAtom> &atom);
bool isBackboneAtomType(const QString &type);

struct ProteinBackboneResidue
{
  QString residueName;
  qint64 residueSequenceNumber = 0;
  QChar codeForInsertionOfResidues = QChar(' ');
  std::shared_ptr<SKAsymmetricAtom> nitrogen;
  std::shared_ptr<SKAsymmetricAtom> alphaCarbon;
  std::shared_ptr<SKAsymmetricAtom> carbonylCarbon;
  std::shared_ptr<SKAsymmetricAtom> carbonylOxygen;

  ProteinBackboneResidue() = default;
  ProteinBackboneResidue(QString residueName,
                         qint64 residueSequenceNumber,
                         QChar codeForInsertionOfResidues,
                         std::shared_ptr<SKAsymmetricAtom> nitrogen,
                         std::shared_ptr<SKAsymmetricAtom> alphaCarbon,
                         std::shared_ptr<SKAsymmetricAtom> carbonylCarbon,
                         std::shared_ptr<SKAsymmetricAtom> carbonylOxygen);

  std::vector<std::shared_ptr<SKAsymmetricAtom>> backboneAtoms() const;
};

struct ProteinBackboneChain
{
  QChar chainIdentifier = QChar(' ');
  std::vector<ProteinBackboneResidue> residues;

  ProteinBackboneChain() = default;
  ProteinBackboneChain(QChar chainIdentifier, std::vector<ProteinBackboneResidue> residues);
};

struct ProteinBackbone
{
  std::vector<ProteinBackboneChain> chains;

  ProteinBackbone() = default;
  explicit ProteinBackbone(std::vector<ProteinBackboneChain> chains);

  int alphaCarbonResidueCount() const;
  static ProteinBackbone build(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms);
};
