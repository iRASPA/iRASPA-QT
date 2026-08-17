/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Nucleic-acid helix rendering via C1' centerline (UCSF Chimera convention).
 ********************************************************************************************************************/

#pragma once

#include <QColor>
#include <QDataStream>
#include <cstdint>
#include <set>
#include <vector>
#include "dnaribbonstructureeditor.h"
#include "rkrenderkitprotocols.h"
#include "dnabackbone.h"
#include "proteinnucleicacidcartoon.h"
#include "rkribbonmesh.h"
#include "skatomtreecontroller.h"

class DNARibbonMixin: public DNARibbonStructureEditor, public RKRenderRibbonSource
{
public:
  DNARibbonMixin();
  DNARibbonMixin(const DNARibbonMixin &other);

  void cloneRibbonStateFrom(const DNARibbonMixin &other);

  const DNABackbone &dnaBackbone() const { return _dnaBackbone; }
  int nucleotideResidueCount() const { return _dnaBackbone.nucleotideResidueCount(); }

  // DNARibbonStructureEditor
  bool drawRibbon() const override;
  void setDrawRibbon(bool value) override;
  double ribbonScaleFactor() const override;
  void setRibbonScaleFactor(double value) override;
  int ribbonSubdivisionsPerSegment() const override;
  void setRibbonSubdivisionsPerSegment(int value) override;
  int ribbonCrossSectionRingResolution() const override;
  void setRibbonCrossSectionRingResolution(int value) override;

  NucleicAcidBackboneStyle nucleicAcidBackboneStyle() const override;
  void setNucleicAcidBackboneStyle(NucleicAcidBackboneStyle value) override;
  NucleicAcidTraceMode nucleicAcidTraceMode() const override;
  void setNucleicAcidTraceMode(NucleicAcidTraceMode value) override;
  NucleicAcidRingMode nucleicAcidRingMode() const override;
  void setNucleicAcidRingMode(NucleicAcidRingMode value) override;
  NucleicAcidLadderMode nucleicAcidLadderMode() const override;
  void setNucleicAcidLadderMode(NucleicAcidLadderMode value) override;
  double nucleicAcidOvalLength() const override;
  void setNucleicAcidOvalLength(double value) override;
  double nucleicAcidOvalWidth() const override;
  void setNucleicAcidOvalWidth(double value) override;
  double nucleicAcidRingWidth() const override;
  void setNucleicAcidRingWidth(double value) override;
  double nucleicAcidLadderRadius() const override;
  void setNucleicAcidLadderRadius(double value) override;
  double nucleicAcidDumbbellLength() const override;
  void setNucleicAcidDumbbellLength(double value) override;
  double nucleicAcidDumbbellWidth() const override;
  void setNucleicAcidDumbbellWidth(double value) override;
  double nucleicAcidDumbbellRadius() const override;
  void setNucleicAcidDumbbellRadius(double value) override;

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
  void readCocoaRibbonAppearance(QDataStream &stream);

protected:
  virtual SKAtomTreeController &ribbonAtomTreeController() = 0;
  virtual const SKAtomTreeController &ribbonAtomTreeController() const = 0;
  virtual double3 ribbonContentShift() const = 0;

  DNABackbone _dnaBackbone;
  RKRibbonMesh _ribbonMesh;

  bool _drawRibbon = true;
  double _ribbonScaleFactor = 1.0;
  int _ribbonSubdivisionsPerSegment = 24;
  int _ribbonCrossSectionRingResolution = 32;
  NucleicAcidBackboneStyle _nucleicAcidBackboneStyle = NucleicAcidBackboneStyle::oval;
  NucleicAcidTraceMode _nucleicAcidTraceMode = NucleicAcidTraceMode::phosphateMode4;
  NucleicAcidRingMode _nucleicAcidRingMode = NucleicAcidRingMode::filledPlanes;
  NucleicAcidLadderMode _nucleicAcidLadderMode = NucleicAcidLadderMode::rungs;
  double _nucleicAcidOvalLength = 1.35;
  double _nucleicAcidOvalWidth = 0.25;
  double _nucleicAcidRingWidth = 0.1;
  double _nucleicAcidLadderRadius = 0.15;
  double _nucleicAcidDumbbellLength = 1.0;
  double _nucleicAcidDumbbellWidth = 0.15;
  double _nucleicAcidDumbbellRadius = 0.3;
  bool _ribbonHDR = true;
  double _ribbonHDRExposure = 2.5;
  double _ribbonHue = 1.0;
  double _ribbonSaturation = 0.5;
  double _ribbonValue = 1.0;
  bool _ribbonAmbientOcclusion = true;
  QColor _ribbonAmbientColor = QColor(255, 255, 255);
  QColor _ribbonDiffuseColor = QColor(255, 255, 255);
  QColor _ribbonSpecularColor = QColor(255, 255, 255);
  double _ribbonAmbientIntensity = 0.2;
  double _ribbonDiffuseIntensity = 1.0;
  double _ribbonSpecularIntensity = 1.0;
  double _ribbonShininess = 4.0;
  int _ribbonAmbientOcclusionPatchNumber = 1;
  int _ribbonAmbientOcclusionPatchSize = 16;
  int _ribbonAmbientOcclusionTextureSize = 256;
  int _ribbonAmbientOcclusionTextureWidth = 2048;
  int _ribbonAmbientOcclusionTextureHeight = 64;
  int _ribbonAmbientOcclusionStripHeight = 64;
};
