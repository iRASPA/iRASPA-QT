/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonMeshParameters.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteinribbonmeshparameters.h"
#include <algorithm>

ProteinRibbonMeshParameters::ProteinRibbonMeshParameters(ProteinRibbonSplineType splineType,
                                                         int subdivisionsPerSegment,
                                                         int crossSectionRingResolution,
                                                         double coilRadiusScale,
                                                         double ribbonWidthClamp,
                                                         double sheetArrowLengthExtent,
                                                         double sheetArrowWingPosition,
                                                         double sheetArrowPeakWidthFactor,
                                                         int normalSmoothingRadius):
  splineType(splineType),
  subdivisionsPerSegment(subdivisionsPerSegment),
  crossSectionRingResolution(crossSectionRingResolution),
  coilRadiusScale(coilRadiusScale),
  ribbonWidthClamp(ribbonWidthClamp),
  sheetArrowLengthExtent(sheetArrowLengthExtent),
  sheetArrowWingPosition(sheetArrowWingPosition),
  sheetArrowPeakWidthFactor(sheetArrowPeakWidthFactor),
  normalSmoothingRadius(normalSmoothingRadius)
{
}

ProteinRibbonMeshParameters ProteinRibbonMeshParameters::defaultParameters()
{
  return ProteinRibbonMeshParameters();
}

ProteinRibbonMeshParameters ProteinRibbonMeshParameters::adaptiveCaps([[maybe_unused]] int atomCount,
                                                                      [[maybe_unused]] int residueCount)
{
  // Cocoa no longer size-caps mesh settings; keep the helper as default clamps for any legacy callers.
  return defaultParameters().clamped();
}

ProteinRibbonMeshParameters ProteinRibbonMeshParameters::effectiveForStructure([[maybe_unused]] int atomCount,
                                                                               [[maybe_unused]] int residueCount) const
{
  // Runtime mesh settings: user/import values with hard clamps only (no structure-size caps).
  return clamped();
}

ProteinRibbonMeshParameters ProteinRibbonMeshParameters::forImportedStructure([[maybe_unused]] int atomCount,
                                                                              [[maybe_unused]] int residueCount)
{
  // Match Cocoa: imported proteins use Default mesh params (arrow length 1.5, peak 2.5).
  return defaultParameters().clamped();
}

ProteinRibbonMeshParameters ProteinRibbonMeshParameters::clamped() const
{
  ProteinRibbonMeshParameters copy = *this;
  copy.subdivisionsPerSegment = std::min(std::max(copy.subdivisionsPerSegment, 1), 128);
  copy.crossSectionRingResolution = std::min(std::max(copy.crossSectionRingResolution, 2), 128);
  copy.coilRadiusScale = std::min(std::max(copy.coilRadiusScale, 0.05), 2.0);
  copy.ribbonWidthClamp = std::min(std::max(copy.ribbonWidthClamp, 0.01), 1.0);
  copy.sheetArrowLengthExtent = std::min(std::max(copy.sheetArrowLengthExtent, 0.5), 10.0);
  copy.sheetArrowWingPosition = std::min(std::max(copy.sheetArrowWingPosition, 0.1), 5.0);
  copy.sheetArrowPeakWidthFactor = std::min(std::max(copy.sheetArrowPeakWidthFactor, 1.0), 10.0);
  copy.normalSmoothingRadius = std::min(std::max(copy.normalSmoothingRadius, 0), 16);
  copy.nucleicAcidOvalLength = std::min(std::max(copy.nucleicAcidOvalLength, 0.2), 3.0);
  copy.nucleicAcidOvalWidth = std::min(std::max(copy.nucleicAcidOvalWidth, 0.05), 1.0);
  copy.nucleicAcidDumbbellLength = std::min(std::max(copy.nucleicAcidDumbbellLength, 0.2), 3.0);
  copy.nucleicAcidDumbbellWidth = std::min(std::max(copy.nucleicAcidDumbbellWidth, 0.02), 1.0);
  copy.nucleicAcidDumbbellRadius = std::min(std::max(copy.nucleicAcidDumbbellRadius, 0.05), 1.5);
  copy.nucleicAcidRingWidth = std::min(std::max(copy.nucleicAcidRingWidth, 0.02), 1.0);
  copy.nucleicAcidLadderRadius = std::min(std::max(copy.nucleicAcidLadderRadius, 0.02), 1.0);
  copy.nucleicAcidLadderSegments = std::min(std::max(copy.nucleicAcidLadderSegments, 4), 32);
  return copy;
}
