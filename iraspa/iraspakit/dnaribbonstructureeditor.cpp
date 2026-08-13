/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "dnaribbonstructureeditor.h"
#include "proteinribbonsplinetype.h"

ProteinRibbonMeshParameters dnaRibbonMeshParameters(const DNARibbonStructureEditor &editor)
{
  return ProteinRibbonMeshParameters(ProteinRibbonSplineType::bSpline,
                                     editor.ribbonSubdivisionsPerSegment(),
                                     editor.ribbonCrossSectionRingResolution(),
                                     0.35,
                                     0.125,
                                     1.5,
                                     1.0,
                                     2.5,
                                     4);
}

void setDnaRibbonMeshParameters(DNARibbonStructureEditor &editor, const ProteinRibbonMeshParameters &parameters)
{
  editor.setRibbonSubdivisionsPerSegment(parameters.subdivisionsPerSegment);
  editor.setRibbonCrossSectionRingResolution(parameters.crossSectionRingResolution);
}

void applyDefaultDnaRibbonAppearance(DNARibbonStructureEditor &editor)
{
  editor.setRibbonHDR(true);
  editor.setRibbonHDRExposure(1.5);
  editor.setRibbonHue(1.0);
  editor.setRibbonSaturation(1.0);
  editor.setRibbonValue(1.0);
  editor.setRibbonAmbientOcclusion(false);
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
  editor.setRibbonAmbientColor(QColor(255, 255, 255));
  editor.setRibbonDiffuseColor(QColor(255, 255, 255));
  editor.setRibbonSpecularColor(QColor(255, 255, 255));
  editor.setRibbonAmbientIntensity(0.2);
  editor.setRibbonDiffuseIntensity(1.0);
  editor.setRibbonSpecularIntensity(1.0);
  editor.setRibbonShininess(4.0);
}
