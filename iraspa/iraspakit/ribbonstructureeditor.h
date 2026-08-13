/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit RibbonStructureEditor.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <QColor>
#include "proteinribboncolorset.h"
#include "proteinribbonrepresentationstyle.h"
#include "proteinribbonsecondarystructuremethod.h"
#include "proteinribbonsplinetype.h"
#include "proteinribbonmeshparameters.h"

class ProteinRibbonStructureEditor
{
public:
  virtual ~ProteinRibbonStructureEditor() = default;

  virtual bool drawRibbon() const = 0;
  virtual void setDrawRibbon(bool value) = 0;
  virtual double ribbonScaleFactor() const = 0;
  virtual void setRibbonScaleFactor(double value) = 0;
  virtual ProteinRibbonColorSet ribbonColorSet() const = 0;
  virtual void setRibbonColorSet(ProteinRibbonColorSet value) = 0;
  virtual ProteinRibbonRepresentationStyle ribbonRepresentationStyle() const = 0;
  virtual void setRibbonRepresentationStyle(ProteinRibbonRepresentationStyle value) = 0;
  virtual ProteinRibbonSecondaryStructureMethod ribbonSecondaryStructureMethod() const = 0;
  virtual void setRibbonSecondaryStructureMethod(ProteinRibbonSecondaryStructureMethod value) = 0;
  virtual ProteinRibbonSplineType ribbonSplineType() const = 0;
  virtual void setRibbonSplineType(ProteinRibbonSplineType value) = 0;
  virtual int ribbonSubdivisionsPerSegment() const = 0;
  virtual void setRibbonSubdivisionsPerSegment(int value) = 0;
  virtual int ribbonCrossSectionRingResolution() const = 0;
  virtual void setRibbonCrossSectionRingResolution(int value) = 0;
  virtual double ribbonCoilRadiusScale() const = 0;
  virtual void setRibbonCoilRadiusScale(double value) = 0;
  virtual double ribbonWidthClamp() const = 0;
  virtual void setRibbonWidthClamp(double value) = 0;
  virtual double ribbonSheetArrowLengthExtent() const = 0;
  virtual void setRibbonSheetArrowLengthExtent(double value) = 0;
  virtual double ribbonSheetArrowWingPosition() const = 0;
  virtual void setRibbonSheetArrowWingPosition(double value) = 0;
  virtual double ribbonSheetArrowPeakWidthFactor() const = 0;
  virtual void setRibbonSheetArrowPeakWidthFactor(double value) = 0;
  virtual int ribbonNormalSmoothingRadius() const = 0;
  virtual void setRibbonNormalSmoothingRadius(int value) = 0;

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
  virtual void recheckRibbonRepresentationStyle() = 0;
};

ProteinRibbonMeshParameters ribbonMeshParameters(const ProteinRibbonStructureEditor &editor);
void setRibbonMeshParameters(ProteinRibbonStructureEditor &editor, const ProteinRibbonMeshParameters &parameters);
void migrateLegacySheetArrowDefaultsIfNeeded(ProteinRibbonStructureEditor &editor);
void applyDefaultRibbonAppearance(ProteinRibbonStructureEditor &editor);
void applyFancyRibbonAppearanceDefault(ProteinRibbonStructureEditor &editor);
void applyRibbonRepresentationStyle(ProteinRibbonStructureEditor &editor, ProteinRibbonRepresentationStyle style);
bool matchesDefaultRibbonAppearance(const ProteinRibbonStructureEditor &editor);
bool matchesFancyRibbonAppearance(const ProteinRibbonStructureEditor &editor);
void recheckRibbonRepresentationStyle(ProteinRibbonStructureEditor &editor);

class AtomViewer;
void rebuildRibbonSecondaryStructureHierarchy(ProteinRibbonStructureEditor &editor, AtomViewer &atomViewer);
