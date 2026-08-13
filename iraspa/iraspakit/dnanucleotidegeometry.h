/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include <QString>
#include <QChar>
#include <vector>
#include <memory>
#include <optional>
#include <skasymmetricatom.h>
#include <sknucleotidebase.h>
#include "mathkit.h"

struct DNABackbone;

struct DNANucleotideResidueGeometry
{
  QChar chainIdentifier = QChar(' ');
  qint64 residueSequenceNumber = 0;
  QChar codeForInsertionOfResidues = QChar(' ');
  QString residueName;
  int globalResidueIndex = -1;

  SKNucleotideBaseKind baseKind = SKNucleotideBaseKind::unknown;
  std::shared_ptr<SKAsymmetricAtom> c1Prime;
  std::shared_ptr<SKAsymmetricAtom> phosphate;
  std::shared_ptr<SKAsymmetricAtom> baseAnchor;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> riboseRingAtoms;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> baseRingAtoms;

  std::vector<double3> riboseRingPositions(double3 contentShift) const;
  std::vector<double3> baseRingPositions(double3 contentShift) const;
  std::optional<double3> c1PrimePosition(double3 contentShift) const;
  std::optional<double3> baseAnchorPosition(double3 contentShift) const;
  std::optional<double3> phosphatePosition(double3 contentShift) const;
};

struct DNANucleotideBasePair
{
  int residueGeometryIndexA = -1;
  int residueGeometryIndexB = -1;
};

struct DNANucleotideGeometry
{
  std::vector<DNANucleotideResidueGeometry> residues;

  static DNANucleotideGeometry build(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms);
  static void assignGlobalResidueIndicesFromBackbone(DNANucleotideGeometry &geometry, const DNABackbone &backbone);
  static std::vector<DNANucleotideBasePair> detectWatsonCrickPairs(const DNANucleotideGeometry &geometry);
};
