/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "dnaribbonmixin.h"
#include "proteinribbonsegmentsupport.h"
#include "proteinribbonmesh.h"
#include "proteinribbonmeshparameters.h"
#include "proteinnucleicacidcartoon.h"
#include "skasymmetricatom.h"

namespace
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> leafAtoms(const SKAtomTreeController &controller)
  {
    const std::vector<std::shared_ptr<SKAtomTreeNode>> leaves = controller.flattenedLeafNodes();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms;
    atoms.reserve(leaves.size());
    for (const std::shared_ptr<SKAtomTreeNode> &node : leaves)
    {
      atoms.push_back(node->representedObject());
    }
    return atoms;
  }
}

DNARibbonMixin::DNARibbonMixin() = default;

DNARibbonMixin::DNARibbonMixin(const DNARibbonMixin &other)
{
  cloneRibbonStateFrom(other);
}

void DNARibbonMixin::cloneRibbonStateFrom(const DNARibbonMixin &other)
{
  _dnaBackbone = other._dnaBackbone;
  _ribbonMesh = other._ribbonMesh;
  _drawRibbon = other._drawRibbon;
  _ribbonScaleFactor = other._ribbonScaleFactor;
  _ribbonSubdivisionsPerSegment = other._ribbonSubdivisionsPerSegment;
  _ribbonCrossSectionRingResolution = other._ribbonCrossSectionRingResolution;
  _nucleicAcidBackboneStyle = other._nucleicAcidBackboneStyle;
  _nucleicAcidTraceMode = other._nucleicAcidTraceMode;
  _nucleicAcidRingMode = other._nucleicAcidRingMode;
  _nucleicAcidLadderMode = other._nucleicAcidLadderMode;
  _nucleicAcidOvalLength = other._nucleicAcidOvalLength;
  _nucleicAcidOvalWidth = other._nucleicAcidOvalWidth;
  _nucleicAcidRingWidth = other._nucleicAcidRingWidth;
  _nucleicAcidLadderRadius = other._nucleicAcidLadderRadius;
  _nucleicAcidDumbbellLength = other._nucleicAcidDumbbellLength;
  _nucleicAcidDumbbellWidth = other._nucleicAcidDumbbellWidth;
  _nucleicAcidDumbbellRadius = other._nucleicAcidDumbbellRadius;
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

bool DNARibbonMixin::drawRibbon() const { return _drawRibbon; }
void DNARibbonMixin::setDrawRibbon(bool value) { _drawRibbon = value; }
double DNARibbonMixin::ribbonScaleFactor() const { return _ribbonScaleFactor; }
void DNARibbonMixin::setRibbonScaleFactor(double value) { _ribbonScaleFactor = value; }
int DNARibbonMixin::ribbonSubdivisionsPerSegment() const { return _ribbonSubdivisionsPerSegment; }
void DNARibbonMixin::setRibbonSubdivisionsPerSegment(int value) { _ribbonSubdivisionsPerSegment = value; }
int DNARibbonMixin::ribbonCrossSectionRingResolution() const { return _ribbonCrossSectionRingResolution; }
void DNARibbonMixin::setRibbonCrossSectionRingResolution(int value) { _ribbonCrossSectionRingResolution = value; }

NucleicAcidBackboneStyle DNARibbonMixin::nucleicAcidBackboneStyle() const { return _nucleicAcidBackboneStyle; }
void DNARibbonMixin::setNucleicAcidBackboneStyle(NucleicAcidBackboneStyle value) { _nucleicAcidBackboneStyle = value; }
NucleicAcidTraceMode DNARibbonMixin::nucleicAcidTraceMode() const { return _nucleicAcidTraceMode; }
void DNARibbonMixin::setNucleicAcidTraceMode(NucleicAcidTraceMode value) { _nucleicAcidTraceMode = value; }
NucleicAcidRingMode DNARibbonMixin::nucleicAcidRingMode() const { return _nucleicAcidRingMode; }
void DNARibbonMixin::setNucleicAcidRingMode(NucleicAcidRingMode value) { _nucleicAcidRingMode = value; }
NucleicAcidLadderMode DNARibbonMixin::nucleicAcidLadderMode() const { return _nucleicAcidLadderMode; }
void DNARibbonMixin::setNucleicAcidLadderMode(NucleicAcidLadderMode value) { _nucleicAcidLadderMode = value; }
double DNARibbonMixin::nucleicAcidOvalLength() const { return _nucleicAcidOvalLength; }
void DNARibbonMixin::setNucleicAcidOvalLength(double value) { _nucleicAcidOvalLength = value; }
double DNARibbonMixin::nucleicAcidOvalWidth() const { return _nucleicAcidOvalWidth; }
void DNARibbonMixin::setNucleicAcidOvalWidth(double value) { _nucleicAcidOvalWidth = value; }
double DNARibbonMixin::nucleicAcidRingWidth() const { return _nucleicAcidRingWidth; }
void DNARibbonMixin::setNucleicAcidRingWidth(double value) { _nucleicAcidRingWidth = value; }
double DNARibbonMixin::nucleicAcidLadderRadius() const { return _nucleicAcidLadderRadius; }
void DNARibbonMixin::setNucleicAcidLadderRadius(double value) { _nucleicAcidLadderRadius = value; }
double DNARibbonMixin::nucleicAcidDumbbellLength() const { return _nucleicAcidDumbbellLength; }
void DNARibbonMixin::setNucleicAcidDumbbellLength(double value) { _nucleicAcidDumbbellLength = value; }
double DNARibbonMixin::nucleicAcidDumbbellWidth() const { return _nucleicAcidDumbbellWidth; }
void DNARibbonMixin::setNucleicAcidDumbbellWidth(double value) { _nucleicAcidDumbbellWidth = value; }
double DNARibbonMixin::nucleicAcidDumbbellRadius() const { return _nucleicAcidDumbbellRadius; }
void DNARibbonMixin::setNucleicAcidDumbbellRadius(double value) { _nucleicAcidDumbbellRadius = value; }

bool DNARibbonMixin::ribbonHDR() const { return _ribbonHDR; }
void DNARibbonMixin::setRibbonHDR(bool value) { _ribbonHDR = value; }
double DNARibbonMixin::ribbonHDRExposure() const { return _ribbonHDRExposure; }
void DNARibbonMixin::setRibbonHDRExposure(double value) { _ribbonHDRExposure = value; }
double DNARibbonMixin::ribbonHue() const { return _ribbonHue; }
void DNARibbonMixin::setRibbonHue(double value) { _ribbonHue = value; }
double DNARibbonMixin::ribbonSaturation() const { return _ribbonSaturation; }
void DNARibbonMixin::setRibbonSaturation(double value) { _ribbonSaturation = value; }
double DNARibbonMixin::ribbonValue() const { return _ribbonValue; }
void DNARibbonMixin::setRibbonValue(double value) { _ribbonValue = value; }

bool DNARibbonMixin::ribbonAmbientOcclusion() const { return _ribbonAmbientOcclusion; }
void DNARibbonMixin::setRibbonAmbientOcclusion(bool value) { _ribbonAmbientOcclusion = value; }
QColor DNARibbonMixin::ribbonAmbientColor() const { return _ribbonAmbientColor; }
void DNARibbonMixin::setRibbonAmbientColor(const QColor &value) { _ribbonAmbientColor = value; }
QColor DNARibbonMixin::ribbonDiffuseColor() const { return _ribbonDiffuseColor; }
void DNARibbonMixin::setRibbonDiffuseColor(const QColor &value) { _ribbonDiffuseColor = value; }
QColor DNARibbonMixin::ribbonSpecularColor() const { return _ribbonSpecularColor; }
void DNARibbonMixin::setRibbonSpecularColor(const QColor &value) { _ribbonSpecularColor = value; }
double DNARibbonMixin::ribbonAmbientIntensity() const { return _ribbonAmbientIntensity; }
void DNARibbonMixin::setRibbonAmbientIntensity(double value) { _ribbonAmbientIntensity = value; }
double DNARibbonMixin::ribbonDiffuseIntensity() const { return _ribbonDiffuseIntensity; }
void DNARibbonMixin::setRibbonDiffuseIntensity(double value) { _ribbonDiffuseIntensity = value; }
double DNARibbonMixin::ribbonSpecularIntensity() const { return _ribbonSpecularIntensity; }
void DNARibbonMixin::setRibbonSpecularIntensity(double value) { _ribbonSpecularIntensity = value; }
double DNARibbonMixin::ribbonShininess() const { return _ribbonShininess; }
void DNARibbonMixin::setRibbonShininess(double value) { _ribbonShininess = value; }

void DNARibbonMixin::applyFancyRibbonAppearance()
{
  applyFancyDnaRibbonAppearanceDefault(*this);
}

void DNARibbonMixin::rebuildBackboneStructure()
{
  _dnaBackbone = DNABackbone::build(leafAtoms(ribbonAtomTreeController()));
}

void DNARibbonMixin::rebuildBackbone()
{
  if (!_drawRibbon) { return; }
  rebuildBackboneStructure();
  rebuildRibbonMesh();
}

void DNARibbonMixin::rebuildRibbonMesh()
{
  if (!_drawRibbon) { return; }
  const std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = leafAtoms(ribbonAtomTreeController());
  const int atomCount = static_cast<int>(atoms.size());
  const int residueCount = nucleotideResidueCount();
  const ProteinRibbonMeshParameters meshParameters =
    dnaRibbonMeshParameters(*this).effectiveForStructure(atomCount, residueCount);
  ProteinRibbonMeshParameters nucleicParameters = meshParameters;
  nucleicParameters.nucleicAcidRendering = true;
  nucleicParameters.nucleicAcidBackboneStyle = _nucleicAcidBackboneStyle;
  nucleicParameters.nucleicAcidTraceMode = _nucleicAcidTraceMode;
  nucleicParameters.nucleicAcidRingMode = _nucleicAcidRingMode;
  nucleicParameters.nucleicAcidLadderMode = _nucleicAcidLadderMode;
  nucleicParameters.nucleicAcidOvalLength = _nucleicAcidOvalLength;
  nucleicParameters.nucleicAcidOvalWidth = _nucleicAcidOvalWidth;
  nucleicParameters.nucleicAcidRingWidth = _nucleicAcidRingWidth;
  nucleicParameters.nucleicAcidLadderRadius = _nucleicAcidLadderRadius;
  nucleicParameters.nucleicAcidDumbbellLength = _nucleicAcidDumbbellLength;
  nucleicParameters.nucleicAcidDumbbellWidth = _nucleicAcidDumbbellWidth;
  nucleicParameters.nucleicAcidDumbbellRadius = _nucleicAcidDumbbellRadius;
  nucleicParameters = nucleicParameters.clamped();
  _ribbonMesh = ProteinRibbonMeshBuilder::buildNucleicAcidRibbon(_dnaBackbone,
                                                                 atoms,
                                                                 _ribbonScaleFactor,
                                                                 ribbonContentShift(),
                                                                 nucleicParameters);
  _ribbonAmbientOcclusionStripHeight = meshParameters.crossSectionRingResolution;
}

std::vector<RKVertex> DNARibbonMixin::renderRibbonVertices() const
{
  return _ribbonMesh.vertices;
}

std::vector<uint32_t> DNARibbonMixin::renderRibbonIndices() const
{
  return _ribbonMesh.indices;
}

int DNARibbonMixin::ribbonNumberOfVertices() const
{
  return static_cast<int>(_ribbonMesh.vertices.size());
}

int DNARibbonMixin::ribbonNumberOfIndices() const
{
  return static_cast<int>(_ribbonMesh.indices.size());
}

std::vector<RKRibbonChainDrawRange> DNARibbonMixin::ribbonChainDrawRanges() const
{
  return _ribbonMesh.chainDrawRanges;
}

std::vector<RKRibbonChainDrawRange> DNARibbonMixin::ribbonSegmentDrawRanges() const
{
  return _ribbonMesh.segmentDrawRanges;
}

std::vector<RKRibbonChainDrawRange> DNARibbonMixin::ribbonResidueDrawRanges() const
{
  return _ribbonMesh.residueDrawRanges;
}

std::vector<RKRibbonChainDrawRange> DNARibbonMixin::ribbonDrawRangesForEncoding() const
{
  return _ribbonMesh.chainDrawRanges;
}

// A DNA strand is drawn as one double helix per chain: its draw ranges follow the backbone, not the
// atom-tree groups, so the ribbon is shown or hidden per chain and never per segment or residue.
bool DNARibbonMixin::ribbonUsesSegmentVisibility() const
{
  return false;
}

bool DNARibbonMixin::ribbonUsesResidueVisibility() const
{
  return false;
}

bool DNARibbonMixin::isRibbonSegmentDrawRangeVisible([[maybe_unused]] int index) const
{
  return true;
}

bool DNARibbonMixin::isRibbonResidueDrawRangeVisible([[maybe_unused]] int index) const
{
  return true;
}

std::set<int> DNARibbonMixin::renderSelectedRibbonSegmentDrawRangeIndices() const
{
  return ProteinRibbonSegmentSupport::selectedSegmentDrawRangeIndices(const_cast<SKAtomTreeController&>(ribbonAtomTreeController()));
}

std::set<int> DNARibbonMixin::renderSelectedRibbonResidueDrawRangeIndices() const
{
  return ProteinRibbonSegmentSupport::selectedResidueDrawRangeIndices(const_cast<SKAtomTreeController&>(ribbonAtomTreeController()));
}

int DNARibbonMixin::ribbonNumberOfChains() const
{
  return _ribbonMesh.numberOfChains();
}

int DNARibbonMixin::ribbonNumberOfRings() const
{
  return _ribbonMesh.numberOfRings();
}

int DNARibbonMixin::ribbonMaxSplineSampleCount() const
{
  return _ribbonMesh.maxSplineSampleCount();
}

int DNARibbonMixin::ribbonAmbientOcclusionPatchNumber() const { return _ribbonAmbientOcclusionPatchNumber; }
int DNARibbonMixin::ribbonAmbientOcclusionPatchSize() const { return _ribbonAmbientOcclusionPatchSize; }
int DNARibbonMixin::ribbonAmbientOcclusionTextureSize() const { return _ribbonAmbientOcclusionTextureSize; }
int DNARibbonMixin::ribbonAmbientOcclusionTextureWidth() const { return _ribbonAmbientOcclusionTextureWidth; }
int DNARibbonMixin::ribbonAmbientOcclusionTextureHeight() const { return _ribbonAmbientOcclusionTextureHeight; }
int DNARibbonMixin::ribbonAmbientOcclusionStripHeight() const { return _ribbonAmbientOcclusionStripHeight; }
void DNARibbonMixin::setRibbonAmbientOcclusionPatchNumber(int value) { _ribbonAmbientOcclusionPatchNumber = value; }
void DNARibbonMixin::setRibbonAmbientOcclusionPatchSize(int value) { _ribbonAmbientOcclusionPatchSize = value; }
void DNARibbonMixin::setRibbonAmbientOcclusionTextureSize(int value) { _ribbonAmbientOcclusionTextureSize = value; }
void DNARibbonMixin::setRibbonAmbientOcclusionTextureWidth(int value) { _ribbonAmbientOcclusionTextureWidth = value; }
void DNARibbonMixin::setRibbonAmbientOcclusionTextureHeight(int value) { _ribbonAmbientOcclusionTextureHeight = value; }
void DNARibbonMixin::setRibbonAmbientOcclusionStripHeight(int value) { _ribbonAmbientOcclusionStripHeight = value; }

float3 DNARibbonMixin::ribbonCoilColor() const
{
  return float3(1.0f, 1.0f, 0.0f);
}

float3 DNARibbonMixin::ribbonHelixColor() const
{
  return float3(1.0f, 0.0f, 0.0f);
}

float3 DNARibbonMixin::ribbonSheetColor() const
{
  return float3(0.5f, 0.5f, 0.5f);
}

void DNARibbonMixin::writeCocoaRibbonAppearance(QDataStream &stream) const
{
  stream << _ribbonScaleFactor;
  stream << static_cast<qint64>(_ribbonSubdivisionsPerSegment);
  stream << static_cast<qint64>(_ribbonCrossSectionRingResolution);
  stream << static_cast<qint64>(_nucleicAcidBackboneStyle);
  stream << static_cast<qint64>(_nucleicAcidTraceMode);
  stream << static_cast<qint64>(_nucleicAcidRingMode);
  stream << static_cast<qint64>(_nucleicAcidLadderMode);
  stream << _nucleicAcidOvalLength;
  stream << _nucleicAcidOvalWidth;
  stream << _nucleicAcidRingWidth;
  stream << _nucleicAcidLadderRadius;
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
}

void DNARibbonMixin::readCocoaRibbonAppearance(QDataStream &stream)
{
  stream >> _ribbonScaleFactor;
  qint64 subdivisions = 0;
  qint64 ringResolution = 0;
  stream >> subdivisions;
  stream >> ringResolution;
  _ribbonSubdivisionsPerSegment = static_cast<int>(subdivisions);
  _ribbonCrossSectionRingResolution = static_cast<int>(ringResolution);
  qint64 backboneStyleValue = 0;
  qint64 traceModeValue = 0;
  qint64 ringModeValue = 0;
  qint64 ladderModeValue = 0;
  stream >> backboneStyleValue;
  stream >> traceModeValue;
  stream >> ringModeValue;
  stream >> ladderModeValue;
  _nucleicAcidBackboneStyle = static_cast<NucleicAcidBackboneStyle>(backboneStyleValue);
  _nucleicAcidTraceMode = static_cast<NucleicAcidTraceMode>(traceModeValue);
  _nucleicAcidRingMode = static_cast<NucleicAcidRingMode>(ringModeValue);
  _nucleicAcidLadderMode = static_cast<NucleicAcidLadderMode>(ladderModeValue);
  stream >> _nucleicAcidOvalLength;
  stream >> _nucleicAcidOvalWidth;
  stream >> _nucleicAcidRingWidth;
  stream >> _nucleicAcidLadderRadius;
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

void DNARibbonMixin::writeRibbonState(QDataStream &stream, qint64 versionNumber) const
{
  if (versionNumber < 3) { return; }

  stream << _drawRibbon;
  stream << _ribbonScaleFactor;
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
  stream << _ribbonSubdivisionsPerSegment;
  stream << _ribbonCrossSectionRingResolution;
  stream << static_cast<qint64>(_nucleicAcidBackboneStyle);
  stream << static_cast<qint64>(_nucleicAcidTraceMode);
  stream << static_cast<qint64>(_nucleicAcidRingMode);
  stream << static_cast<qint64>(_nucleicAcidLadderMode);
  stream << _nucleicAcidOvalLength;
  stream << _nucleicAcidOvalWidth;
  stream << _nucleicAcidRingWidth;
  stream << _nucleicAcidLadderRadius;
  if (versionNumber >= 4)
  {
    stream << _nucleicAcidDumbbellLength;
    stream << _nucleicAcidDumbbellWidth;
    stream << _nucleicAcidDumbbellRadius;
  }
}

void DNARibbonMixin::readRibbonState(QDataStream &stream, qint64 versionNumber)
{
  if (versionNumber < 3) { return; }

  stream >> _drawRibbon;
  stream >> _ribbonScaleFactor;
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
  stream >> _ribbonSubdivisionsPerSegment;
  stream >> _ribbonCrossSectionRingResolution;
  qint64 backboneStyleValue = 0;
  stream >> backboneStyleValue;
  _nucleicAcidBackboneStyle = static_cast<NucleicAcidBackboneStyle>(backboneStyleValue);
  qint64 traceModeValue = 0;
  stream >> traceModeValue;
  _nucleicAcidTraceMode = static_cast<NucleicAcidTraceMode>(traceModeValue);
  qint64 ringModeValue = 0;
  stream >> ringModeValue;
  _nucleicAcidRingMode = static_cast<NucleicAcidRingMode>(ringModeValue);
  qint64 ladderModeValue = 0;
  stream >> ladderModeValue;
  _nucleicAcidLadderMode = static_cast<NucleicAcidLadderMode>(ladderModeValue);
  stream >> _nucleicAcidOvalLength;
  stream >> _nucleicAcidOvalWidth;
  stream >> _nucleicAcidRingWidth;
  stream >> _nucleicAcidLadderRadius;
  if (versionNumber >= 4)
  {
    stream >> _nucleicAcidDumbbellLength;
    stream >> _nucleicAcidDumbbellWidth;
    stream >> _nucleicAcidDumbbellRadius;
  }
}
