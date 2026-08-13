/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit Protein.swift ribbon state (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteinribbonmixin.h"
#include "proteinribbonsegmentsupport.h"
#include "proteinatomtreebuilder.h"
#include "proteinribbonmesh.h"
#include "proteinribbonmeshparameters.h"
#include "proteinribbonsegmentsupport.h"
#include "skasymmetricatom.h"

ProteinRibbonMixin::ProteinRibbonMixin() = default;

ProteinRibbonMixin::ProteinRibbonMixin(const ProteinRibbonMixin &other)
{
  cloneRibbonStateFrom(other);
}

void ProteinRibbonMixin::cloneRibbonStateFrom(const ProteinRibbonMixin &other)
{
  _backbone = other._backbone;
  _ribbonMesh = other._ribbonMesh;
  _drawRibbon = other._drawRibbon;
  _ribbonScaleFactor = other._ribbonScaleFactor;
  _ribbonColorSet = other._ribbonColorSet;
  _ribbonRepresentationStyle = other._ribbonRepresentationStyle;
  _ribbonSecondaryStructureMethod = other._ribbonSecondaryStructureMethod;
  _ribbonSplineType = other._ribbonSplineType;
  _ribbonSubdivisionsPerSegment = other._ribbonSubdivisionsPerSegment;
  _ribbonCrossSectionRingResolution = other._ribbonCrossSectionRingResolution;
  _ribbonCoilRadiusScale = other._ribbonCoilRadiusScale;
  _ribbonWidthClamp = other._ribbonWidthClamp;
  _ribbonSheetArrowLengthExtent = other._ribbonSheetArrowLengthExtent;
  _ribbonSheetArrowWingPosition = other._ribbonSheetArrowWingPosition;
  _ribbonSheetArrowPeakWidthFactor = other._ribbonSheetArrowPeakWidthFactor;
  _ribbonNormalSmoothingRadius = other._ribbonNormalSmoothingRadius;
  _ribbonHDR = other._ribbonHDR;
  _ribbonHDRExposure = other._ribbonHDRExposure;
  _ribbonHue = other._ribbonHue;
  _ribbonSaturation = other._ribbonSaturation;
  _ribbonValue = other._ribbonValue;
  _ribbonAmbientOcclusion = other._ribbonAmbientOcclusion;
  _ribbonAmbientColor = other._ribbonAmbientColor;
  _ribbonDiffuseColor = other._ribbonDiffuseColor;
  _ribbonSpecularColor = other._ribbonSpecularColor;
  _ribbonAmbientIntensity = other._ribbonAmbientIntensity;
  _ribbonDiffuseIntensity = other._ribbonDiffuseIntensity;
  _ribbonSpecularIntensity = other._ribbonSpecularIntensity;
  _ribbonShininess = other._ribbonShininess;
  _ribbonAmbientOcclusionPatchNumber = other._ribbonAmbientOcclusionPatchNumber;
  _ribbonAmbientOcclusionPatchSize = other._ribbonAmbientOcclusionPatchSize;
  _ribbonAmbientOcclusionTextureSize = other._ribbonAmbientOcclusionTextureSize;
  _ribbonAmbientOcclusionTextureWidth = other._ribbonAmbientOcclusionTextureWidth;
  _ribbonAmbientOcclusionTextureHeight = other._ribbonAmbientOcclusionTextureHeight;
  _ribbonAmbientOcclusionStripHeight = other._ribbonAmbientOcclusionStripHeight;
}

bool ProteinRibbonMixin::drawRibbon() const { return _drawRibbon; }
void ProteinRibbonMixin::setDrawRibbon(bool value) { _drawRibbon = value; }
double ProteinRibbonMixin::ribbonScaleFactor() const { return _ribbonScaleFactor; }
void ProteinRibbonMixin::setRibbonScaleFactor(double value) { _ribbonScaleFactor = value; }
ProteinRibbonColorSet ProteinRibbonMixin::ribbonColorSet() const { return _ribbonColorSet; }
void ProteinRibbonMixin::setRibbonColorSet(ProteinRibbonColorSet value) { _ribbonColorSet = value; }
ProteinRibbonRepresentationStyle ProteinRibbonMixin::ribbonRepresentationStyle() const { return _ribbonRepresentationStyle; }
void ProteinRibbonMixin::setRibbonRepresentationStyle(ProteinRibbonRepresentationStyle value) { _ribbonRepresentationStyle = value; }
ProteinRibbonSecondaryStructureMethod ProteinRibbonMixin::ribbonSecondaryStructureMethod() const { return _ribbonSecondaryStructureMethod; }
void ProteinRibbonMixin::setRibbonSecondaryStructureMethod(ProteinRibbonSecondaryStructureMethod value) { _ribbonSecondaryStructureMethod = value; }
ProteinRibbonSplineType ProteinRibbonMixin::ribbonSplineType() const { return _ribbonSplineType; }
void ProteinRibbonMixin::setRibbonSplineType(ProteinRibbonSplineType value) { _ribbonSplineType = value; }
int ProteinRibbonMixin::ribbonSubdivisionsPerSegment() const { return _ribbonSubdivisionsPerSegment; }
void ProteinRibbonMixin::setRibbonSubdivisionsPerSegment(int value) { _ribbonSubdivisionsPerSegment = value; }
int ProteinRibbonMixin::ribbonCrossSectionRingResolution() const { return _ribbonCrossSectionRingResolution; }
void ProteinRibbonMixin::setRibbonCrossSectionRingResolution(int value) { _ribbonCrossSectionRingResolution = value; }
double ProteinRibbonMixin::ribbonCoilRadiusScale() const { return _ribbonCoilRadiusScale; }
void ProteinRibbonMixin::setRibbonCoilRadiusScale(double value) { _ribbonCoilRadiusScale = value; }
double ProteinRibbonMixin::ribbonWidthClamp() const { return _ribbonWidthClamp; }
void ProteinRibbonMixin::setRibbonWidthClamp(double value) { _ribbonWidthClamp = value; }
double ProteinRibbonMixin::ribbonSheetArrowLengthExtent() const { return _ribbonSheetArrowLengthExtent; }
void ProteinRibbonMixin::setRibbonSheetArrowLengthExtent(double value) { _ribbonSheetArrowLengthExtent = value; }
double ProteinRibbonMixin::ribbonSheetArrowWingPosition() const { return _ribbonSheetArrowWingPosition; }
void ProteinRibbonMixin::setRibbonSheetArrowWingPosition(double value) { _ribbonSheetArrowWingPosition = value; }
double ProteinRibbonMixin::ribbonSheetArrowPeakWidthFactor() const { return _ribbonSheetArrowPeakWidthFactor; }
void ProteinRibbonMixin::setRibbonSheetArrowPeakWidthFactor(double value) { _ribbonSheetArrowPeakWidthFactor = value; }
int ProteinRibbonMixin::ribbonNormalSmoothingRadius() const { return _ribbonNormalSmoothingRadius; }
void ProteinRibbonMixin::setRibbonNormalSmoothingRadius(int value) { _ribbonNormalSmoothingRadius = value; }

bool ProteinRibbonMixin::ribbonHDR() const { return _ribbonHDR; }
void ProteinRibbonMixin::setRibbonHDR(bool value) { _ribbonHDR = value; }
double ProteinRibbonMixin::ribbonHDRExposure() const { return _ribbonHDRExposure; }
void ProteinRibbonMixin::setRibbonHDRExposure(double value) { _ribbonHDRExposure = value; }
double ProteinRibbonMixin::ribbonHue() const { return _ribbonHue; }
void ProteinRibbonMixin::setRibbonHue(double value) { _ribbonHue = value; }
double ProteinRibbonMixin::ribbonSaturation() const { return _ribbonSaturation; }
void ProteinRibbonMixin::setRibbonSaturation(double value) { _ribbonSaturation = value; }
double ProteinRibbonMixin::ribbonValue() const { return _ribbonValue; }
void ProteinRibbonMixin::setRibbonValue(double value) { _ribbonValue = value; }

bool ProteinRibbonMixin::ribbonAmbientOcclusion() const { return _ribbonAmbientOcclusion; }
void ProteinRibbonMixin::setRibbonAmbientOcclusion(bool value) { _ribbonAmbientOcclusion = value; }
QColor ProteinRibbonMixin::ribbonAmbientColor() const { return _ribbonAmbientColor; }
void ProteinRibbonMixin::setRibbonAmbientColor(const QColor &value) { _ribbonAmbientColor = value; }
QColor ProteinRibbonMixin::ribbonDiffuseColor() const { return _ribbonDiffuseColor; }
void ProteinRibbonMixin::setRibbonDiffuseColor(const QColor &value) { _ribbonDiffuseColor = value; }
QColor ProteinRibbonMixin::ribbonSpecularColor() const { return _ribbonSpecularColor; }
void ProteinRibbonMixin::setRibbonSpecularColor(const QColor &value) { _ribbonSpecularColor = value; }
double ProteinRibbonMixin::ribbonAmbientIntensity() const { return _ribbonAmbientIntensity; }
void ProteinRibbonMixin::setRibbonAmbientIntensity(double value) { _ribbonAmbientIntensity = value; }
double ProteinRibbonMixin::ribbonDiffuseIntensity() const { return _ribbonDiffuseIntensity; }
void ProteinRibbonMixin::setRibbonDiffuseIntensity(double value) { _ribbonDiffuseIntensity = value; }
double ProteinRibbonMixin::ribbonSpecularIntensity() const { return _ribbonSpecularIntensity; }
void ProteinRibbonMixin::setRibbonSpecularIntensity(double value) { _ribbonSpecularIntensity = value; }
double ProteinRibbonMixin::ribbonShininess() const { return _ribbonShininess; }
void ProteinRibbonMixin::setRibbonShininess(double value) { _ribbonShininess = value; }

void ProteinRibbonMixin::applyFancyRibbonAppearance()
{
  applyFancyRibbonAppearanceDefault(*this);
  recheckRibbonRepresentationStyle();
}

void ProteinRibbonMixin::recheckRibbonRepresentationStyle()
{
  ::recheckRibbonRepresentationStyle(*this);
}

void ProteinRibbonMixin::rebuildBackboneStructure()
{
  const std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = ribbonAtomTreeController().flattenedObjects();
  _backbone = ProteinBackbone::build(atoms);
}

void ProteinRibbonMixin::rebuildBackbone()
{
  if (!_drawRibbon) { return; }
  rebuildBackboneStructure();
  rebuildRibbonMesh();
}

void ProteinRibbonMixin::rebuildRibbonMesh()
{
  if (!_drawRibbon) { return; }
  const int atomCount = static_cast<int>(ribbonAtomTreeController().flattenedLeafNodes().size());
  const int residueCount = _backbone.alphaCarbonResidueCount();
  const ProteinRibbonMeshParameters meshParameters =
    ribbonMeshParameters(*this).effectiveForStructure(atomCount, residueCount);
  _ribbonMesh = ProteinRibbonMeshBuilder::build(_backbone,
                                                _ribbonScaleFactor,
                                                ribbonContentShift(),
                                                meshParameters,
                                                _ribbonSecondaryStructureMethod);
  _ribbonAmbientOcclusionStripHeight = meshParameters.crossSectionRingResolution;
}

std::vector<RKVertex> ProteinRibbonMixin::renderRibbonVertices() const
{
  return _ribbonMesh.vertices;
}

std::vector<uint32_t> ProteinRibbonMixin::renderRibbonIndices() const
{
  return _ribbonMesh.indices;
}

int ProteinRibbonMixin::ribbonNumberOfVertices() const
{
  return static_cast<int>(_ribbonMesh.vertices.size());
}

int ProteinRibbonMixin::ribbonNumberOfIndices() const
{
  return static_cast<int>(_ribbonMesh.indices.size());
}

std::vector<RKRibbonChainDrawRange> ProteinRibbonMixin::ribbonChainDrawRanges() const
{
  return _ribbonMesh.chainDrawRanges;
}

std::vector<RKRibbonChainDrawRange> ProteinRibbonMixin::ribbonSegmentDrawRanges() const
{
  return _ribbonMesh.segmentDrawRanges;
}

std::vector<RKRibbonChainDrawRange> ProteinRibbonMixin::ribbonResidueDrawRanges() const
{
  return _ribbonMesh.residueDrawRanges;
}

bool ProteinRibbonMixin::ribbonUsesSegmentVisibility() const
{
  if (!_ribbonMesh.segmentAlphaCarbonTags.empty())
  {
    return _ribbonMesh.segmentAlphaCarbonTags.size() == _ribbonMesh.segmentDrawRanges.size();
  }
  return ProteinRibbonSegmentSupport::segmentTreeNodesAlignWithDrawRanges(const_cast<SKAtomTreeController&>(ribbonAtomTreeController()),
                                                                          static_cast<int>(_ribbonMesh.segmentDrawRanges.size()));
}

bool ProteinRibbonMixin::ribbonUsesResidueVisibility() const
{
  if (!_ribbonMesh.residueAlphaCarbonTags.empty())
  {
    return _ribbonMesh.residueAlphaCarbonTags.size() == _ribbonMesh.residueDrawRanges.size();
  }
  return ProteinRibbonSegmentSupport::residueTreeNodesAlignWithDrawRanges(const_cast<SKAtomTreeController&>(ribbonAtomTreeController()),
                                                                          static_cast<int>(_ribbonMesh.residueDrawRanges.size()));
}

bool ProteinRibbonMixin::isRibbonSegmentDrawRangeVisible(int index) const
{
  if (!ribbonUsesSegmentVisibility()) { return true; }
  SKAtomTreeController &controller = const_cast<SKAtomTreeController&>(ribbonAtomTreeController());
  if (!_ribbonMesh.segmentAlphaCarbonTags.empty())
  {
    if (index < 0 || index >= static_cast<int>(_ribbonMesh.segmentAlphaCarbonTags.size())) { return true; }
    const int tag = _ribbonMesh.segmentAlphaCarbonTags[static_cast<size_t>(index)];
    const std::shared_ptr<SKAtomTreeNode> segmentNode = ProteinRibbonSegmentSupport::segmentTreeNodeForAtomTag(tag, controller);
    if (!segmentNode) { return true; }
    return ProteinRibbonSegmentSupport::isRibbonSegmentVisible(segmentNode);
  }
  const std::vector<std::shared_ptr<SKAtomTreeNode>> segmentNodes =
    ProteinRibbonSegmentSupport::orderedSegmentTreeNodes(controller);
  if (index < 0 || index >= static_cast<int>(segmentNodes.size())) { return true; }
  return ProteinRibbonSegmentSupport::isRibbonSegmentVisible(segmentNodes[static_cast<size_t>(index)]);
}

bool ProteinRibbonMixin::isRibbonResidueDrawRangeVisible(int index) const
{
  if (!ribbonUsesResidueVisibility()) { return true; }
  SKAtomTreeController &controller = const_cast<SKAtomTreeController&>(ribbonAtomTreeController());
  if (!_ribbonMesh.residueAlphaCarbonTags.empty())
  {
    if (index < 0 || index >= static_cast<int>(_ribbonMesh.residueAlphaCarbonTags.size())) { return true; }
    const int tag = _ribbonMesh.residueAlphaCarbonTags[static_cast<size_t>(index)];
    const std::shared_ptr<SKAtomTreeNode> residueNode = ProteinRibbonSegmentSupport::residueTreeNodeForAtomTag(tag, controller);
    if (!residueNode) { return true; }
    return ProteinRibbonSegmentSupport::isRibbonResidueVisible(residueNode);
  }
  const std::vector<std::shared_ptr<SKAtomTreeNode>> residueNodes =
    ProteinRibbonSegmentSupport::orderedResidueTreeNodes(controller);
  if (index < 0 || index >= static_cast<int>(residueNodes.size())) { return true; }
  return ProteinRibbonSegmentSupport::isRibbonResidueVisible(residueNodes[static_cast<size_t>(index)]);
}

std::set<int> ProteinRibbonMixin::renderSelectedRibbonSegmentDrawRangeIndices() const
{
  return ProteinRibbonSegmentSupport::selectedSegmentDrawRangeIndices(const_cast<SKAtomTreeController&>(ribbonAtomTreeController()));
}

std::set<int> ProteinRibbonMixin::renderSelectedRibbonResidueDrawRangeIndices() const
{
  return ProteinRibbonSegmentSupport::selectedResidueDrawRangeIndices(const_cast<SKAtomTreeController&>(ribbonAtomTreeController()));
}

int ProteinRibbonMixin::ribbonNumberOfChains() const
{
  return _ribbonMesh.numberOfChains();
}

int ProteinRibbonMixin::ribbonNumberOfRings() const
{
  return _ribbonMesh.numberOfRings();
}

int ProteinRibbonMixin::ribbonMaxSplineSampleCount() const
{
  return _ribbonMesh.maxSplineSampleCount();
}

int ProteinRibbonMixin::ribbonAmbientOcclusionPatchNumber() const { return _ribbonAmbientOcclusionPatchNumber; }
int ProteinRibbonMixin::ribbonAmbientOcclusionPatchSize() const { return _ribbonAmbientOcclusionPatchSize; }
int ProteinRibbonMixin::ribbonAmbientOcclusionTextureSize() const { return _ribbonAmbientOcclusionTextureSize; }
int ProteinRibbonMixin::ribbonAmbientOcclusionTextureWidth() const { return _ribbonAmbientOcclusionTextureWidth; }
int ProteinRibbonMixin::ribbonAmbientOcclusionTextureHeight() const { return _ribbonAmbientOcclusionTextureHeight; }
int ProteinRibbonMixin::ribbonAmbientOcclusionStripHeight() const { return _ribbonAmbientOcclusionStripHeight; }
void ProteinRibbonMixin::setRibbonAmbientOcclusionPatchNumber(int value) { _ribbonAmbientOcclusionPatchNumber = value; }
void ProteinRibbonMixin::setRibbonAmbientOcclusionPatchSize(int value) { _ribbonAmbientOcclusionPatchSize = value; }
void ProteinRibbonMixin::setRibbonAmbientOcclusionTextureSize(int value) { _ribbonAmbientOcclusionTextureSize = value; }
void ProteinRibbonMixin::setRibbonAmbientOcclusionTextureWidth(int value) { _ribbonAmbientOcclusionTextureWidth = value; }
void ProteinRibbonMixin::setRibbonAmbientOcclusionTextureHeight(int value) { _ribbonAmbientOcclusionTextureHeight = value; }
void ProteinRibbonMixin::setRibbonAmbientOcclusionStripHeight(int value) { _ribbonAmbientOcclusionStripHeight = value; }

float3 ProteinRibbonMixin::ribbonCoilColor() const
{
  return proteinRibbonColorSetCoilColor(_ribbonColorSet);
}

float3 ProteinRibbonMixin::ribbonHelixColor() const
{
  return proteinRibbonColorSetHelixColor(_ribbonColorSet);
}

float3 ProteinRibbonMixin::ribbonSheetColor() const
{
  return proteinRibbonColorSetSheetColor(_ribbonColorSet);
}

void ProteinRibbonMixin::writeCocoaRibbonAppearance(QDataStream &stream) const
{
  stream << proteinRibbonColorSetRawValue(_ribbonColorSet);
  stream << _ribbonHDR;
  stream << _ribbonHDRExposure;
  stream << _ribbonHue;
  stream << _ribbonSaturation;
  stream << _ribbonValue;
  stream << _ribbonAmbientOcclusion;
  stream << _ribbonAmbientColor;
  stream << _ribbonDiffuseColor;
  stream << _ribbonSpecularColor;
  stream << _ribbonAmbientIntensity;
  stream << _ribbonDiffuseIntensity;
  stream << _ribbonSpecularIntensity;
  stream << _ribbonShininess;
  stream << proteinRibbonSplineTypeRawValue(_ribbonSplineType);
  stream << static_cast<qint64>(_ribbonSubdivisionsPerSegment);
  stream << static_cast<qint64>(_ribbonCrossSectionRingResolution);
  stream << _ribbonCoilRadiusScale;
  stream << _ribbonWidthClamp;
  stream << _ribbonSheetArrowLengthExtent;
  stream << _ribbonSheetArrowWingPosition;
  stream << _ribbonSheetArrowPeakWidthFactor;
  stream << static_cast<qint64>(_ribbonNormalSmoothingRadius);
  stream << proteinRibbonRepresentationStyleRawValue(_ribbonRepresentationStyle);
  stream << proteinRibbonSecondaryStructureMethodRawValue(_ribbonSecondaryStructureMethod);
}

void ProteinRibbonMixin::readCocoaRibbonAppearance(QDataStream &stream, qint64 versionNumber)
{
  if (versionNumber < 3) { return; }

  QString colorSetIdentifier;
  stream >> colorSetIdentifier;
  _ribbonColorSet = proteinRibbonColorSetFromRawValue(colorSetIdentifier);

  if (versionNumber >= 4)
  {
    stream >> _ribbonHDR;
    stream >> _ribbonHDRExposure;
    stream >> _ribbonHue;
    stream >> _ribbonSaturation;
    stream >> _ribbonValue;
    stream >> _ribbonAmbientOcclusion;
    stream >> _ribbonAmbientColor;
    stream >> _ribbonDiffuseColor;
    stream >> _ribbonSpecularColor;
    stream >> _ribbonAmbientIntensity;
    stream >> _ribbonDiffuseIntensity;
    stream >> _ribbonSpecularIntensity;
    stream >> _ribbonShininess;
  }

  if (versionNumber >= 5)
  {
    QString splineTypeIdentifier;
    stream >> splineTypeIdentifier;
    _ribbonSplineType = proteinRibbonSplineTypeFromRawValue(splineTypeIdentifier);
    qint64 subdivisions = 0;
    qint64 ringResolution = 0;
    qint64 normalSmoothingRadius = 0;
    stream >> subdivisions;
    stream >> ringResolution;
    _ribbonSubdivisionsPerSegment = static_cast<int>(subdivisions);
    _ribbonCrossSectionRingResolution = static_cast<int>(ringResolution);
    stream >> _ribbonCoilRadiusScale;
    stream >> _ribbonWidthClamp;
    stream >> _ribbonSheetArrowLengthExtent;
    stream >> _ribbonSheetArrowWingPosition;
    stream >> _ribbonSheetArrowPeakWidthFactor;
    stream >> normalSmoothingRadius;
    _ribbonNormalSmoothingRadius = static_cast<int>(normalSmoothingRadius);
  }

  if (versionNumber >= 6)
  {
    QString representationStyleIdentifier;
    stream >> representationStyleIdentifier;
    _ribbonRepresentationStyle = proteinRibbonRepresentationStyleFromRawValue(representationStyleIdentifier);
  }
  else if (versionNumber >= 4)
  {
    _ribbonRepresentationStyle = _ribbonAmbientOcclusion ? ProteinRibbonRepresentationStyle::fancy
                                                         : ProteinRibbonRepresentationStyle::defaultStyle;
  }

  if (versionNumber >= 7)
  {
    QString secondaryStructureMethodIdentifier;
    stream >> secondaryStructureMethodIdentifier;
    _ribbonSecondaryStructureMethod = proteinRibbonSecondaryStructureMethodFromRawValue(secondaryStructureMethodIdentifier);
  }

  migrateLegacySheetArrowDefaultsIfNeeded(*this);
}

void ProteinRibbonMixin::writeRibbonState(QDataStream &stream, qint64 versionNumber) const
{
  if (versionNumber < 3) { return; }

  stream << _drawRibbon;
  stream << _ribbonScaleFactor;
  stream << static_cast<qint64>(_ribbonColorSet);
  stream << _ribbonHDR;
  stream << _ribbonHDRExposure;
  stream << _ribbonHue;
  stream << _ribbonSaturation;
  stream << _ribbonValue;
  stream << _ribbonAmbientOcclusion;
  stream << _ribbonAmbientColor;
  stream << _ribbonDiffuseColor;
  stream << _ribbonSpecularColor;
  stream << _ribbonAmbientIntensity;
  stream << _ribbonDiffuseIntensity;
  stream << _ribbonSpecularIntensity;
  stream << _ribbonShininess;
  stream << static_cast<qint64>(_ribbonSplineType);
  stream << _ribbonSubdivisionsPerSegment;
  stream << _ribbonCrossSectionRingResolution;
  stream << _ribbonCoilRadiusScale;
  stream << _ribbonWidthClamp;
  stream << _ribbonSheetArrowLengthExtent;
  stream << _ribbonSheetArrowWingPosition;
  stream << _ribbonSheetArrowPeakWidthFactor;
  stream << _ribbonNormalSmoothingRadius;
  stream << static_cast<qint64>(_ribbonRepresentationStyle);
  stream << static_cast<qint64>(_ribbonSecondaryStructureMethod);
}

void ProteinRibbonMixin::readRibbonState(QDataStream &stream, qint64 versionNumber)
{
  if (versionNumber < 3) { return; }

  stream >> _drawRibbon;
  stream >> _ribbonScaleFactor;
  qint64 colorSetValue = 0;
  stream >> colorSetValue;
  _ribbonColorSet = static_cast<ProteinRibbonColorSet>(colorSetValue);
  stream >> _ribbonHDR;
  stream >> _ribbonHDRExposure;
  stream >> _ribbonHue;
  stream >> _ribbonSaturation;
  stream >> _ribbonValue;
  stream >> _ribbonAmbientOcclusion;
  stream >> _ribbonAmbientColor;
  stream >> _ribbonDiffuseColor;
  stream >> _ribbonSpecularColor;
  stream >> _ribbonAmbientIntensity;
  stream >> _ribbonDiffuseIntensity;
  stream >> _ribbonSpecularIntensity;
  stream >> _ribbonShininess;
  qint64 splineTypeValue = 0;
  stream >> splineTypeValue;
  _ribbonSplineType = static_cast<ProteinRibbonSplineType>(splineTypeValue);
  stream >> _ribbonSubdivisionsPerSegment;
  stream >> _ribbonCrossSectionRingResolution;
  stream >> _ribbonCoilRadiusScale;
  stream >> _ribbonWidthClamp;
  stream >> _ribbonSheetArrowLengthExtent;
  stream >> _ribbonSheetArrowWingPosition;
  stream >> _ribbonSheetArrowPeakWidthFactor;
  stream >> _ribbonNormalSmoothingRadius;
  qint64 representationStyleValue = 0;
  stream >> representationStyleValue;
  _ribbonRepresentationStyle = static_cast<ProteinRibbonRepresentationStyle>(representationStyleValue);
  qint64 secondaryStructureMethodValue = 0;
  stream >> secondaryStructureMethodValue;
  _ribbonSecondaryStructureMethod = static_cast<ProteinRibbonSecondaryStructureMethod>(secondaryStructureMethodValue);
  migrateLegacySheetArrowDefaultsIfNeeded(*this);
}

static void insertTreeNodeSelection(SKAtomTreeController &controller, const std::shared_ptr<SKAtomTreeNode> &node)
{
  if (node)
  {
    controller.insertSelectionIndexPath(node->indexPath());
  }
}

static void removeTreeNodeSelection(SKAtomTreeController &controller, const std::shared_ptr<SKAtomTreeNode> &node)
{
  if (node)
  {
    controller.removeSelectionIndexPath(node->indexPath());
  }
}

bool ProteinRibbonMixin::applyRibbonPick(int segmentIndex, int residueIndex, RibbonPickAction action, bool selectSegment)
{
  SKAtomTreeController &controller = ribbonAtomTreeController();
  const std::shared_ptr<SKAtomTreeNode> treeNode =
    ProteinRibbonSegmentSupport::treeNodeForRibbonPick(segmentIndex, residueIndex, selectSegment, controller);
  if (!treeNode)
  {
    return false;
  }

  switch (action)
  {
  case RibbonPickAction::replaceResidue:
    controller.clearSelection();
    insertTreeNodeSelection(controller, treeNode);
    break;

  case RibbonPickAction::toggleResidue:
  {
    const AtomSelectionIndexPaths selection = controller.selectionIndexPaths();
    const IndexPath indexPath = treeNode->indexPath();
    if (selection.second.count(indexPath) > 0)
    {
      removeTreeNodeSelection(controller, treeNode);
    }
    else
    {
      insertTreeNodeSelection(controller, treeNode);
    }
    break;
  }

  case RibbonPickAction::toggleSecondaryStructureSegment:
  {
    if (!ProteinRibbonSegmentSupport::isSecondaryStructureSegmentNode(treeNode))
    {
      return false;
    }
    const std::vector<std::shared_ptr<SKAtomTreeNode>> residueNodes =
      ProteinRibbonSegmentSupport::residueGroupNodes(treeNode);

    if (ProteinRibbonSegmentSupport::isSecondaryStructureSegmentSelected(treeNode, controller.selectedTreeNodes()))
    {
      removeTreeNodeSelection(controller, treeNode);
      for (const std::shared_ptr<SKAtomTreeNode> &residueNode : residueNodes)
      {
        removeTreeNodeSelection(controller, residueNode);
      }
    }
    else
    {
      for (const std::shared_ptr<SKAtomTreeNode> &residueNode : residueNodes)
      {
        removeTreeNodeSelection(controller, residueNode);
      }
      insertTreeNodeSelection(controller, treeNode);
    }
    break;
  }
  }

  return true;
}
