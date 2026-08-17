/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit Protein.swift ribbon state (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <QColor>
#include <QDataStream>
#include <cstdint>
#include <set>
#include <vector>
#include "ribbonstructureeditor.h"
#include "rkrenderkitprotocols.h"
#include "proteinbackbone.h"
#include "proteinribboncolorset.h"
#include "proteinribbonrepresentationstyle.h"
#include "proteinribbonsecondarystructuremethod.h"
#include "proteinribbonsplinetype.h"
#include "rkribbonmesh.h"
#include "skatomtreecontroller.h"

class ProteinRibbonMixin: public ProteinRibbonStructureEditor, public RKRenderRibbonSource
{
public:
  ProteinRibbonMixin();
  ProteinRibbonMixin(const ProteinRibbonMixin &other);

  void cloneRibbonStateFrom(const ProteinRibbonMixin &other);

  const ProteinBackbone &backbone() const { return _backbone; }

  // ProteinRibbonStructureEditor
  bool drawRibbon() const override;
  void setDrawRibbon(bool value) override;
  double ribbonScaleFactor() const override;
  void setRibbonScaleFactor(double value) override;
  ProteinRibbonColorSet ribbonColorSet() const override;
  void setRibbonColorSet(ProteinRibbonColorSet value) override;
  ProteinRibbonRepresentationStyle ribbonRepresentationStyle() const override;
  void setRibbonRepresentationStyle(ProteinRibbonRepresentationStyle value) override;
  ProteinRibbonSecondaryStructureMethod ribbonSecondaryStructureMethod() const override;
  void setRibbonSecondaryStructureMethod(ProteinRibbonSecondaryStructureMethod value) override;
  ProteinRibbonSplineType ribbonSplineType() const override;
  void setRibbonSplineType(ProteinRibbonSplineType value) override;
  int ribbonSubdivisionsPerSegment() const override;
  void setRibbonSubdivisionsPerSegment(int value) override;
  int ribbonCrossSectionRingResolution() const override;
  void setRibbonCrossSectionRingResolution(int value) override;
  double ribbonCoilRadiusScale() const override;
  void setRibbonCoilRadiusScale(double value) override;
  double ribbonWidthClamp() const override;
  void setRibbonWidthClamp(double value) override;
  double ribbonSheetArrowLengthExtent() const override;
  void setRibbonSheetArrowLengthExtent(double value) override;
  double ribbonSheetArrowWingPosition() const override;
  void setRibbonSheetArrowWingPosition(double value) override;
  double ribbonSheetArrowPeakWidthFactor() const override;
  void setRibbonSheetArrowPeakWidthFactor(double value) override;
  int ribbonNormalSmoothingRadius() const override;
  void setRibbonNormalSmoothingRadius(int value) override;

  void rebuildRibbonMesh() override;
  void rebuildBackbone() override;
  void rebuildBackboneStructure() override;

  bool ribbonHDR() const override;
  void setRibbonHDR(bool value) override;
  double ribbonHDRExposure() const override;
  void setRibbonHDRExposure(double value) override;
  double ribbonHue() const override;
  void setRibbonHue(double value) override;
  double ribbonSaturation() const override;
  void setRibbonSaturation(double value) override;
  double ribbonValue() const override;
  void setRibbonValue(double value) override;

  bool ribbonAmbientOcclusion() const override;
  void setRibbonAmbientOcclusion(bool value) override;
  QColor ribbonAmbientColor() const override;
  void setRibbonAmbientColor(const QColor &value) override;
  QColor ribbonDiffuseColor() const override;
  void setRibbonDiffuseColor(const QColor &value) override;
  QColor ribbonSpecularColor() const override;
  void setRibbonSpecularColor(const QColor &value) override;
  double ribbonAmbientIntensity() const override;
  void setRibbonAmbientIntensity(double value) override;
  double ribbonDiffuseIntensity() const override;
  void setRibbonDiffuseIntensity(double value) override;
  double ribbonSpecularIntensity() const override;
  void setRibbonSpecularIntensity(double value) override;
  double ribbonShininess() const override;
  void setRibbonShininess(double value) override;

  void applyFancyRibbonAppearance() override;
  void recheckRibbonRepresentationStyle() override;

  // RKRenderRibbonSource
  std::vector<RKVertex> renderRibbonVertices() const override;
  std::vector<uint32_t> renderRibbonIndices() const override;
  int ribbonNumberOfVertices() const override;
  int ribbonNumberOfIndices() const override;
  std::vector<RKRibbonChainDrawRange> ribbonChainDrawRanges() const override;
  std::vector<RKRibbonChainDrawRange> ribbonSegmentDrawRanges() const override;
  std::vector<RKRibbonChainDrawRange> ribbonResidueDrawRanges() const override;
  std::vector<RKRibbonChainDrawRange> ribbonDrawRangesForEncoding() const override;
  bool ribbonUsesSegmentVisibility() const override;
  bool ribbonUsesResidueVisibility() const override;
  bool isRibbonSegmentDrawRangeVisible(int index) const override;
  bool isRibbonResidueDrawRangeVisible(int index) const override;
  std::set<int> renderSelectedRibbonSegmentDrawRangeIndices() const override;
  std::set<int> renderSelectedRibbonResidueDrawRangeIndices() const override;
  int ribbonNumberOfChains() const override;
  int ribbonNumberOfRings() const override;
  int ribbonMaxSplineSampleCount() const override;
  int ribbonAmbientOcclusionPatchNumber() const override;
  int ribbonAmbientOcclusionPatchSize() const override;
  int ribbonAmbientOcclusionTextureSize() const override;
  int ribbonAmbientOcclusionTextureWidth() const override;
  int ribbonAmbientOcclusionTextureHeight() const override;
  int ribbonAmbientOcclusionStripHeight() const override;
  void setRibbonAmbientOcclusionPatchNumber(int value) override;
  void setRibbonAmbientOcclusionPatchSize(int value) override;
  void setRibbonAmbientOcclusionTextureSize(int value) override;
  void setRibbonAmbientOcclusionTextureWidth(int value) override;
  void setRibbonAmbientOcclusionTextureHeight(int value) override;
  void setRibbonAmbientOcclusionStripHeight(int value) override;
  float3 ribbonCoilColor() const override;
  float3 ribbonHelixColor() const override;
  float3 ribbonSheetColor() const override;

  void writeRibbonState(QDataStream &stream, qint64 versionNumber) const;
  void readRibbonState(QDataStream &stream, qint64 versionNumber);
  void writeCocoaRibbonAppearance(QDataStream &stream) const;
  void readCocoaRibbonAppearance(QDataStream &stream, qint64 versionNumber);

  enum class RibbonPickAction
  {
    replaceResidue,
    toggleResidue,
    toggleSecondaryStructureSegment
  };

  /// Apply a ribbon pick buffer (segment/residue indices). Returns false if no node matched.
  bool applyRibbonPick(int segmentIndex, int residueIndex, RibbonPickAction action, bool selectSegment);

protected:
  virtual SKAtomTreeController &ribbonAtomTreeController() = 0;
  virtual const SKAtomTreeController &ribbonAtomTreeController() const = 0;
  virtual double3 ribbonContentShift() const = 0;

  /// Which ribbon pieces the atom tree currently hides. Resolving this means walking the tree, so it
  /// is cached against the atom visibility generation rather than recomputed for every draw call.
  struct RibbonVisibilityCache
  {
    qint64 generation = -1;
    size_t residueDrawRangeCount = 0;
    size_t segmentDrawRangeCount = 0;
    bool usesResidueVisibility = false;
    bool usesSegmentVisibility = false;
    std::vector<uint8_t> residueVisibility;
    std::vector<uint8_t> segmentVisibility;
    std::vector<RKRibbonChainDrawRange> encodingDrawRanges;
  };

  const RibbonVisibilityCache &ribbonVisibilityCache() const;
  void invalidateRibbonVisibilityCache();

  mutable RibbonVisibilityCache _ribbonVisibilityCache;

  ProteinBackbone _backbone;
  RKRibbonMesh _ribbonMesh;

  bool _drawRibbon = true;
  double _ribbonScaleFactor = 1.2;
  ProteinRibbonColorSet _ribbonColorSet = ProteinRibbonColorSet::standardAcademic;
  ProteinRibbonRepresentationStyle _ribbonRepresentationStyle = ProteinRibbonRepresentationStyle::defaultStyle;
  ProteinRibbonSecondaryStructureMethod _ribbonSecondaryStructureMethod = ProteinRibbonSecondaryStructureMethod::stride;
  ProteinRibbonSplineType _ribbonSplineType = ProteinRibbonSplineType::bSpline;
  int _ribbonSubdivisionsPerSegment = 24;
  int _ribbonCrossSectionRingResolution = 32;
  double _ribbonCoilRadiusScale = 0.35;
  double _ribbonWidthClamp = 0.125;
  double _ribbonSheetArrowLengthExtent = 1.5;
  double _ribbonSheetArrowWingPosition = 1.0;
  double _ribbonSheetArrowPeakWidthFactor = 2.5;
  int _ribbonNormalSmoothingRadius = 4;
  bool _ribbonHDR = true;
  double _ribbonHDRExposure = 1.5;
  double _ribbonHue = 1.0;
  double _ribbonSaturation = 1.0;
  double _ribbonValue = 1.0;
  bool _ribbonAmbientOcclusion = false;
  QColor _ribbonAmbientColor = QColor(255, 255, 255);
  QColor _ribbonDiffuseColor = QColor(255, 255, 255);
  QColor _ribbonSpecularColor = QColor(255, 255, 255);
  double _ribbonAmbientIntensity = 0.2;
  double _ribbonDiffuseIntensity = 1.0;
  double _ribbonSpecularIntensity = 1.0;
  double _ribbonShininess = 6.0;
  int _ribbonAmbientOcclusionPatchNumber = 1;
  int _ribbonAmbientOcclusionPatchSize = 16;
  int _ribbonAmbientOcclusionTextureSize = 256;
  int _ribbonAmbientOcclusionTextureWidth = 2048;
  int _ribbonAmbientOcclusionTextureHeight = 64;
  int _ribbonAmbientOcclusionStripHeight = 64;
};
