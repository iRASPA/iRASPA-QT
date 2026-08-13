/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include <QColor>
#include "proteinnucleicacidcartoon.h"
#include "proteinribbonmeshparameters.h"

class DNARibbonStructureEditor
{
public:
  virtual ~DNARibbonStructureEditor() = default;

  virtual bool drawRibbon() const = 0;
  virtual void setDrawRibbon(bool value) = 0;
  virtual double ribbonScaleFactor() const = 0;
  virtual void setRibbonScaleFactor(double value) = 0;
  virtual int ribbonSubdivisionsPerSegment() const = 0;
  virtual void setRibbonSubdivisionsPerSegment(int value) = 0;
  virtual int ribbonCrossSectionRingResolution() const = 0;
  virtual void setRibbonCrossSectionRingResolution(int value) = 0;

  virtual NucleicAcidBackboneStyle nucleicAcidBackboneStyle() const = 0;
  virtual void setNucleicAcidBackboneStyle(NucleicAcidBackboneStyle value) = 0;
  virtual NucleicAcidTraceMode nucleicAcidTraceMode() const = 0;
  virtual void setNucleicAcidTraceMode(NucleicAcidTraceMode value) = 0;
  virtual NucleicAcidRingMode nucleicAcidRingMode() const = 0;
  virtual void setNucleicAcidRingMode(NucleicAcidRingMode value) = 0;
  virtual NucleicAcidLadderMode nucleicAcidLadderMode() const = 0;
  virtual void setNucleicAcidLadderMode(NucleicAcidLadderMode value) = 0;
  virtual double nucleicAcidOvalLength() const = 0;
  virtual void setNucleicAcidOvalLength(double value) = 0;
  virtual double nucleicAcidOvalWidth() const = 0;
  virtual void setNucleicAcidOvalWidth(double value) = 0;
  virtual double nucleicAcidRingWidth() const = 0;
  virtual void setNucleicAcidRingWidth(double value) = 0;
  virtual double nucleicAcidLadderRadius() const = 0;
  virtual void setNucleicAcidLadderRadius(double value) = 0;
  virtual double nucleicAcidDumbbellLength() const = 0;
  virtual void setNucleicAcidDumbbellLength(double value) = 0;
  virtual double nucleicAcidDumbbellWidth() const = 0;
  virtual void setNucleicAcidDumbbellWidth(double value) = 0;
  virtual double nucleicAcidDumbbellRadius() const = 0;
  virtual void setNucleicAcidDumbbellRadius(double value) = 0;

  virtual void rebuildRibbonMesh() = 0;
  virtual void rebuildBackbone() = 0;
  virtual void rebuildBackboneStructure() = 0;

  virtual bool ribbonHDR() const = 0;
  virtual void setRibbonHDR(bool value) = 0;
  virtual double ribbonHDRExposure() const = 0;
  virtual void setRibbonHDRExposure(double value) = 0;
  virtual double ribbonHue() const = 0;
  virtual void setRibbonHue(double value) = 0;
  virtual double ribbonSaturation() const = 0;
  virtual void setRibbonSaturation(double value) = 0;
  virtual double ribbonValue() const = 0;
  virtual void setRibbonValue(double value) = 0;

  virtual bool ribbonAmbientOcclusion() const = 0;
  virtual void setRibbonAmbientOcclusion(bool value) = 0;
  virtual QColor ribbonAmbientColor() const = 0;
  virtual void setRibbonAmbientColor(const QColor &value) = 0;
  virtual QColor ribbonDiffuseColor() const = 0;
  virtual void setRibbonDiffuseColor(const QColor &value) = 0;
  virtual QColor ribbonSpecularColor() const = 0;
  virtual void setRibbonSpecularColor(const QColor &value) = 0;
  virtual double ribbonAmbientIntensity() const = 0;
  virtual void setRibbonAmbientIntensity(double value) = 0;
  virtual double ribbonDiffuseIntensity() const = 0;
  virtual void setRibbonDiffuseIntensity(double value) = 0;
  virtual double ribbonSpecularIntensity() const = 0;
  virtual void setRibbonSpecularIntensity(double value) = 0;
  virtual double ribbonShininess() const = 0;
  virtual void setRibbonShininess(double value) = 0;

  virtual void applyFancyRibbonAppearance() = 0;
};

ProteinRibbonMeshParameters dnaRibbonMeshParameters(const DNARibbonStructureEditor &editor);
void setDnaRibbonMeshParameters(DNARibbonStructureEditor &editor, const ProteinRibbonMeshParameters &parameters);
void applyDefaultDnaRibbonAppearance(DNARibbonStructureEditor &editor);
void applyFancyDnaRibbonAppearanceDefault(DNARibbonStructureEditor &editor);
