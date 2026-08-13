/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    PyMOL nucleic-acid cartoon settings (cartoon_nucleic_acid_mode, cCartoon_* cross-sections).
 ********************************************************************************************************************/

#pragma once

#include <QString>

/// PyMOL cCartoon_oval / tube / dumbbell / rect backbone cross-section for DNA/RNA.
enum class NucleicAcidBackboneStyle
{
  oval = 0,
  tube,
  dumbbell,
  rect
};

/// PyMOL cartoon_nucleic_acid_mode: phosphate trace (0/2/4) vs C3' trace (1).
enum class NucleicAcidTraceMode
{
  phosphateMode4 = 4,
  c3PrimeMode1 = 1
};

/// PyMOL cartoon_ring_mode 1: flat filled ribose/base ring planes.
enum class NucleicAcidRingMode
{
  off = 0,
  filledPlanes = 1
};

/// PyMOL cartoon_ladder_mode 1: glycosidic/backbone-to-base and Watson-Crick rungs.
enum class NucleicAcidLadderMode
{
  off = 0,
  rungs = 1
};

inline QString nucleicAcidBackboneStyleDisplayName(NucleicAcidBackboneStyle style)
{
  switch (style)
  {
  case NucleicAcidBackboneStyle::oval: return QStringLiteral("Oval");
  case NucleicAcidBackboneStyle::tube: return QStringLiteral("Tube");
  case NucleicAcidBackboneStyle::dumbbell: return QStringLiteral("Dumbbell");
  case NucleicAcidBackboneStyle::rect: return QStringLiteral("Rect");
  }
  return QString();
}

inline QString nucleicAcidTraceModeDisplayName(NucleicAcidTraceMode mode)
{
  switch (mode)
  {
  case NucleicAcidTraceMode::phosphateMode4: return QStringLiteral("Phosphate (P)");
  case NucleicAcidTraceMode::c3PrimeMode1: return QStringLiteral("C3′");
  }
  return QString();
}
