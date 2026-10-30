/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "dnaribbonstructureeditor.h"
#include "proteinribbonsplinetype.h"

ProteinRibbonMeshParameters dnaRibbonMeshParameters(const DNARibbonStructureEditor &editor)
{
  ProteinRibbonMeshParameters parameters(ProteinRibbonSplineType::bSpline,
                                         editor.ribbonSubdivisionsPerSegment(),
                                         editor.ribbonCrossSectionRingResolution(),
                                         0.35,
                                         0.125,
                                         1.5,
                                         1.0,
                                         2.5,
                                         4);
  parameters.nucleicAcidRendering = true;
  parameters.nucleicAcidBackboneStyle = editor.nucleicAcidBackboneStyle();
  parameters.nucleicAcidTraceMode = editor.nucleicAcidTraceMode();
  parameters.nucleicAcidRingMode = editor.nucleicAcidRingMode();
  parameters.nucleicAcidLadderMode = editor.nucleicAcidLadderMode();
  parameters.nucleicAcidOvalLength = editor.nucleicAcidOvalLength();
  parameters.nucleicAcidOvalWidth = editor.nucleicAcidOvalWidth();
  parameters.nucleicAcidRingWidth = editor.nucleicAcidRingWidth();
  parameters.nucleicAcidLadderRadius = editor.nucleicAcidLadderRadius();
  parameters.nucleicAcidDumbbellLength = editor.nucleicAcidDumbbellLength();
  parameters.nucleicAcidDumbbellWidth = editor.nucleicAcidDumbbellWidth();
  parameters.nucleicAcidDumbbellRadius = editor.nucleicAcidDumbbellRadius();
  return parameters.clamped();
}

void setDnaRibbonMeshParameters(DNARibbonStructureEditor &editor, const ProteinRibbonMeshParameters &parameters)
{
  const ProteinRibbonMeshParameters clamped = parameters.clamped();
  editor.setRibbonSubdivisionsPerSegment(clamped.subdivisionsPerSegment);
  editor.setRibbonCrossSectionRingResolution(clamped.crossSectionRingResolution);
  editor.setNucleicAcidBackboneStyle(clamped.nucleicAcidBackboneStyle);
  editor.setNucleicAcidTraceMode(clamped.nucleicAcidTraceMode);
  editor.setNucleicAcidRingMode(clamped.nucleicAcidRingMode);
  editor.setNucleicAcidLadderMode(clamped.nucleicAcidLadderMode);
  editor.setNucleicAcidOvalLength(clamped.nucleicAcidOvalLength);
  editor.setNucleicAcidOvalWidth(clamped.nucleicAcidOvalWidth);
  editor.setNucleicAcidRingWidth(clamped.nucleicAcidRingWidth);
  editor.setNucleicAcidLadderRadius(clamped.nucleicAcidLadderRadius);
  editor.setNucleicAcidDumbbellLength(clamped.nucleicAcidDumbbellLength);
  editor.setNucleicAcidDumbbellWidth(clamped.nucleicAcidDumbbellWidth);
  editor.setNucleicAcidDumbbellRadius(clamped.nucleicAcidDumbbellRadius);
}

void applyDefaultDnaRibbonAppearance(DNARibbonStructureEditor &editor)
{
  editor.setRibbonHDR(true);
  editor.setRibbonHDRExposure(1.5);
  editor.setRibbonHue(1.0);
  editor.setRibbonSaturation(1.0);
  editor.setRibbonValue(1.0);
  editor.setRibbonAmbientOcclusion(false);
  editor.setRibbonEdgeCueing(RKEdgeCueing::off);
  editor.setRibbonAmbientColor(QColor(255, 255, 255));
  editor.setRibbonDiffuseColor(QColor(255, 255, 255));
  editor.setRibbonSpecularColor(QColor(255, 255, 255));
  editor.setRibbonAmbientIntensity(0.2);
  editor.setRibbonDiffuseIntensity(1.0);
  editor.setRibbonSpecularIntensity(1.0);
  editor.setRibbonShininess(6.0);
}

void applyFancyDnaRibbonAppearanceDefault(DNARibbonStructureEditor &editor)
{
  editor.setRibbonHDR(true);
  editor.setRibbonHDRExposure(2.5);
  editor.setRibbonHue(1.0);
  editor.setRibbonSaturation(0.5);
  editor.setRibbonValue(1.0);
  editor.setRibbonAmbientOcclusion(true);
  editor.setRibbonEdgeCueing(RKEdgeCueing::off);
  editor.setRibbonAmbientColor(QColor(255, 255, 255));
  editor.setRibbonDiffuseColor(QColor(255, 255, 255));
  editor.setRibbonSpecularColor(QColor(255, 255, 255));
  editor.setRibbonAmbientIntensity(0.2);
  editor.setRibbonDiffuseIntensity(1.0);
  editor.setRibbonSpecularIntensity(1.0);
  editor.setRibbonShininess(4.0);
}

void applyIllustrativeDnaRibbonAppearanceDefault(DNARibbonStructureEditor &editor)
{
  editor.setRibbonHDR(true);
  editor.setRibbonHDRExposure(2.5);
  editor.setRibbonHue(1.0);
  editor.setRibbonSaturation(0.5);
  editor.setRibbonValue(1.0);
  editor.setRibbonAmbientOcclusion(true);
  editor.setRibbonEdgeCueing(RKEdgeCueing::contoursAndHalos);
  editor.setRibbonAmbientColor(QColor(255, 255, 255));
  editor.setRibbonDiffuseColor(QColor(255, 255, 255));
  editor.setRibbonSpecularColor(QColor(255, 255, 255));
  editor.setRibbonAmbientIntensity(0.2);
  editor.setRibbonDiffuseIntensity(1.0);
  editor.setRibbonSpecularIntensity(1.0);
  editor.setRibbonShininess(4.0);
}
