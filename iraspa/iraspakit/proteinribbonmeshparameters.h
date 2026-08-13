/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonMeshParameters.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include "proteinribbonsplinetype.h"
#include "proteinnucleicacidcartoon.h"

struct ProteinRibbonMeshParameters
{
  ProteinRibbonSplineType splineType = ProteinRibbonSplineType::bSpline;
  int subdivisionsPerSegment = 24;
  int crossSectionRingResolution = 32;
  double coilRadiusScale = 0.35;
  double ribbonWidthClamp = 0.125;
  bool nucleicAcidRendering = false;
  NucleicAcidBackboneStyle nucleicAcidBackboneStyle = NucleicAcidBackboneStyle::oval;
  NucleicAcidTraceMode nucleicAcidTraceMode = NucleicAcidTraceMode::phosphateMode4;
  NucleicAcidRingMode nucleicAcidRingMode = NucleicAcidRingMode::off;
  NucleicAcidLadderMode nucleicAcidLadderMode = NucleicAcidLadderMode::off;
  /// PyMOL cartoon_oval_length (long axis, widthDirection).
  double nucleicAcidOvalLength = 1.35;
  /// PyMOL cartoon_oval_width (short axis, faceNormal / orientation).
  double nucleicAcidOvalWidth = 0.25;
  /// PyMOL cartoon_ring_width.
  double nucleicAcidRingWidth = 0.1;
  /// PyMOL cartoon_ladder_radius.
  double nucleicAcidLadderRadius = 0.15;
  int nucleicAcidLadderSegments = 8;
  /// PyMOL cartoon_dumbbell_length / width / radius.
  double nucleicAcidDumbbellLength = 1.0;
  double nucleicAcidDumbbellWidth = 0.15;
  double nucleicAcidDumbbellRadius = 0.3;
  double sheetArrowLengthExtent = 1.5;
  double sheetArrowWingPosition = 1.0;
  double sheetArrowPeakWidthFactor = 2.5;
  int normalSmoothingRadius = 4;

  ProteinRibbonMeshParameters() = default;

  ProteinRibbonMeshParameters(ProteinRibbonSplineType splineType,
                              int subdivisionsPerSegment,
                              int crossSectionRingResolution,
                              double coilRadiusScale,
                              double ribbonWidthClamp,
                              double sheetArrowLengthExtent,
                              double sheetArrowWingPosition,
                              double sheetArrowPeakWidthFactor,
                              int normalSmoothingRadius);

  static ProteinRibbonMeshParameters defaultParameters();
  static ProteinRibbonMeshParameters adaptiveCaps(int atomCount, int residueCount);
  ProteinRibbonMeshParameters effectiveForStructure(int atomCount, int residueCount) const;
  static ProteinRibbonMeshParameters forImportedStructure(int atomCount, int residueCount);
  ProteinRibbonMeshParameters clamped() const;
};
