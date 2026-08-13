/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include <QString>
#include <vector>

enum class SKNucleotideBaseKind
{
  unknown,
  adenine,
  cytosine,
  guanine,
  thymine,
  uracil
};

/// Ribbon vertex pad.x codes for PyMOL-style nucleic acid coloring (see ribbon.vert).
constexpr float kNucleicBackboneStructureType = 3.0f;
constexpr float kNucleicAdenineStructureType = 4.0f;
constexpr float kNucleicCytosineStructureType = 5.0f;
constexpr float kNucleicGuanineStructureType = 6.0f;
constexpr float kNucleicThymineStructureType = 7.0f;

namespace SKNucleotideBase
{
  SKNucleotideBaseKind baseKindFromResidueName(const QString &residueName);
  /// Maps base kind to ribbon shader structure type (backbone uses yellow regardless of base).
  float vertexStructureTypeCode(SKNucleotideBaseKind baseKind, bool backbone = false);
  bool areWatsonCrickComplementary(SKNucleotideBaseKind a, SKNucleotideBaseKind b);
  bool areWatsonCrickComplementary(const QString &residueNameA, const QString &residueNameB);

  /// PyMOL ring_finder 1: ribose ring atom names in bond order.
  std::vector<QString> riboseRingAtomNames();
  /// Base ring atom names in bond order (pyrimidine 6-ring or purine 9-ring subset present in PDB).
  std::vector<QString> baseRingAtomNames(SKNucleotideBaseKind baseKind);
  /// Glycosidic anchor: N1 (pyrimidine) or N9 (purine).
  QString baseAnchorAtomName(SKNucleotideBaseKind baseKind);
}
