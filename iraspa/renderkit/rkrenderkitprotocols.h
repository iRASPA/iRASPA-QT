/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#pragma once

#include <QString>
#include <QColor>
#include <QDate>
#include <QImage>
#include <vector>
#include <mathkit.h>
#include <foundationkit.h>
#include <symmetrykit.h>
#include "rkfontatlas.h"

#include "rklight.h"
#include "rkcamera.h"

// forward declaration
struct RKInPerInstanceAttributesAtoms;
struct RKInPerInstanceAttributesBonds;
struct RKInPerInstanceAttributesText;


enum class RKBackgroundType: qint64
{
  color = 0, linearGradient = 1, radialGradient = 2, image = 3
};

enum class RKBondColorMode: qint64
{
  uniform = 0, split = 1, smoothed_split = 2, multiple_values = 3
};

enum class RKRenderQuality: qint64
{
  low = 0, medium = 1, high = 2, picture = 3
};

enum class RKImageQuality: qint64
{
  rgb_8_bits = 0, rgb_16_bits = 1, cmyk_8_bits = 2, cmyk_16_bits = 3
};

enum class RKImageDPI: qint64
{
  dpi_72 = 0, dpi_75 = 1, dpi_150 = 2, dpi_300 = 3, dpi_600 = 4, dpi_1200 = 5
};

enum class RKImageDimensions: qint64
{
  physical = 0, pixels = 1
};

enum class RKImageUnits: qint64
{
  inch = 0, cm = 1
};

enum class RKSelectionStyle: qint64
{
  None = 0, WorleyNoise3D = 1, striped = 2, glow = 3, multiple_values = 4
};

enum class RKTextStyle: qint64
{
  flatBillboard = 0, multiple_values = 1
};

enum class RKTextEffect: qint64
{
  none = 0, glow = 1, pulsate = 2, squiggle = 3, multiple_values = 4
};

enum class RKTextType: qint64
{
  none = 0, displayName = 1, identifier = 2, chemicalElement = 3, forceFieldType = 4, position = 5, charge = 6, multiple_values = 7
};

enum class RKTextAlignment: qint64
{
  center = 0, left = 1, right = 2, top = 3, bottom = 4, topLeft = 5, topRight = 6, bottomLeft = 7, bottomRight = 8, multiple_values = 9
};



class RKRenderStructure
{
public:
  virtual ~RKRenderStructure() = 0;
  virtual void computeBonds() = 0;

  virtual QString displayName() const = 0;

  virtual bool isVisible() const = 0;
  virtual bool hasSelectedAtoms() const = 0;

  virtual std::vector<double3> atomPositions() const = 0;
  virtual std::vector<double3> bondPositions() const = 0;

  virtual std::vector<double2> potentialParameters() const = 0;

  virtual std::vector<RKInPerInstanceAttributesAtoms> renderAtoms() const = 0;

  virtual bool clipAtomsAtUnitCell() const = 0;
  virtual bool clipBondsAtUnitCell() const = 0;

  virtual std::vector<RKInPerInstanceAttributesText> renderTextData() const = 0;
  virtual RKTextType renderTextType() const = 0;
  virtual QString renderTextFont() const = 0;
  virtual RKTextAlignment renderTextAlignment() const = 0;
  virtual RKTextStyle renderTextStyle() const = 0;
  virtual QColor renderTextColor() const = 0;
  virtual double renderTextScaling() const = 0;
  virtual double3 renderTextOffset() const = 0;

  virtual std::vector<RKInPerInstanceAttributesAtoms> renderSelectedAtoms() const = 0;
  virtual RKSelectionStyle atomSelectionStyle() const = 0;
  virtual double atomSelectionIntensity() const = 0;
  virtual double atomSelectionScaling() const = 0;
  virtual double atomSelectionFrequency() const = 0;
  virtual double atomSelectionDensity() const = 0;
  virtual double atomSelectionStripesDensity() const = 0;
  virtual double atomSelectionStripesFrequency() const = 0;
  virtual double atomSelectionWorleyNoise3DFrequency() const = 0;
  virtual double atomSelectionWorleyNoise3DJitter() const = 0;

  virtual std::vector<RKInPerInstanceAttributesBonds> renderInternalBonds() const = 0;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderExternalBonds() const = 0;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderSelectedInternalBonds() const = 0;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderSelectedExternalBonds() const = 0;

  virtual std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const = 0;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const = 0;

  virtual int numberOfAtoms() const = 0;
  virtual int numberOfInternalBonds() const = 0;
  virtual int numberOfExternalBonds() const = 0;

  virtual bool drawUnitCell() const = 0;
  virtual bool drawAtoms() const = 0;
  virtual bool drawBonds() const = 0;

  virtual simd_quatd orientation() const = 0;
  virtual double3 origin() const = 0;

  // material properties
  virtual QColor atomAmbientColor() const = 0;
  virtual QColor atomDiffuseColor() const = 0;
  virtual QColor atomSpecularColor() const = 0;
  virtual double atomAmbientIntensity() const = 0;
  virtual double atomDiffuseIntensity() const = 0;
  virtual double atomSpecularIntensity() const = 0;
  virtual double atomShininess() const = 0;

  virtual std::shared_ptr<SKCell> cell() const = 0;
  virtual SKBoundingBox boundingBox() const = 0;
  virtual SKBoundingBox transformedBoundingBox() const = 0;
  virtual void expandSymmetry() = 0;
  virtual void reComputeBoundingBox() = 0;

  //virtual atomCacheAmbientOcclusionTexture: [CUnsignedChar] {get set}

  virtual bool hasExternalBonds() const = 0;

  virtual bool isUnity() const = 0;

  virtual double atomHue() const = 0;
  virtual double atomSaturation() const = 0;
  virtual double atomValue() const = 0;

  virtual bool colorAtomsWithBondColor() const = 0;
  virtual double atomScaleFactor() const = 0;
  virtual bool atomAmbientOcclusion() const = 0;
  virtual int atomAmbientOcclusionPatchNumber() const = 0;
  virtual void setAtomAmbientOcclusionPatchNumber(int) = 0;
  virtual int atomAmbientOcclusionPatchSize() const = 0;
  virtual void setAtomAmbientOcclusionPatchSize(int) = 0;
  virtual int atomAmbientOcclusionTextureSize() const = 0;
  virtual void setAtomAmbientOcclusionTextureSize(int) = 0;

  virtual bool atomHDR() const = 0;
  virtual double atomHDRExposure() const = 0;

  virtual bool bondAmbientOcclusion() const = 0;
  virtual QColor bondAmbientColor() const = 0;
  virtual QColor bondDiffuseColor() const = 0;
  virtual QColor bondSpecularColor() const = 0;
  virtual double bondAmbientIntensity() const = 0;
  virtual double bondDiffuseIntensity() const = 0;
  virtual double bondSpecularIntensity() const = 0;
  virtual double bondShininess() const = 0;

  virtual double bondScaleFactor() const = 0;
  virtual RKBondColorMode bondColorMode() const = 0;

  virtual bool bondHDR() const = 0;
  virtual double bondHDRExposure() const = 0;

  virtual double bondHue() const = 0;
  virtual double bondSaturation() const = 0;
  virtual double bondValue() const = 0;

  virtual RKSelectionStyle bondSelectionStyle() const = 0;
  virtual double bondSelectionIntensity() const = 0;
  virtual double bondSelectionScaling() const = 0;
  virtual double bondSelectionFrequency() const = 0;
  virtual double bondSelectionDensity() const = 0;
  virtual double bondSelectionStripesDensity() const = 0;
  virtual double bondSelectionStripesFrequency() const = 0;
  virtual double bondSelectionWorleyNoise3DFrequency() const = 0;
  virtual double bondSelectionWorleyNoise3DJitter() const = 0;

  // unit cell
  virtual double unitCellScaleFactor() const = 0;
  virtual QColor unitCellDiffuseColor() const = 0;
  virtual double unitCellDiffuseIntensity() const = 0;


  // adsorption surface
  virtual std::vector<double3> atomUnitCellPositions() const = 0;
  virtual bool drawAdsorptionSurface() const = 0;
  virtual double adsorptionSurfaceOpacity() const = 0;
  virtual double adsorptionSurfaceIsoValue() const = 0;
  virtual double adsorptionSurfaceMinimumValue() const = 0;
  virtual void setAdsorptionSurfaceMinimumValue(double value) = 0;
  virtual int adsorptionSurfaceSize() const = 0;
  virtual double2 adsorptionSurfaceProbeParameters() const = 0;

  virtual double adsorptionSurfaceHue() const = 0;
  virtual double adsorptionSurfaceSaturation() const = 0;
  virtual double adsorptionSurfaceValue() const = 0;

  virtual bool adsorptionSurfaceFrontSideHDR() const = 0;
  virtual double adsorptionSurfaceFrontSideHDRExposure() const = 0;
  virtual QColor adsorptionSurfaceFrontSideAmbientColor() const = 0;
  virtual QColor adsorptionSurfaceFrontSideDiffuseColor() const = 0;
  virtual QColor adsorptionSurfaceFrontSideSpecularColor() const = 0;
  virtual double adsorptionSurfaceFrontSideDiffuseIntensity() const = 0;
  virtual double adsorptionSurfaceFrontSideAmbientIntensity() const = 0;
  virtual double adsorptionSurfaceFrontSideSpecularIntensity() const = 0;
  virtual double adsorptionSurfaceFrontSideShininess() const = 0;

  virtual bool adsorptionSurfaceBackSideHDR() const = 0;
  virtual double adsorptionSurfaceBackSideHDRExposure() const = 0;
  virtual QColor adsorptionSurfaceBackSideAmbientColor() const = 0;
  virtual QColor adsorptionSurfaceBackSideDiffuseColor() const = 0;
  virtual QColor adsorptionSurfaceBackSideSpecularColor() const = 0;
  virtual double adsorptionSurfaceBackSideDiffuseIntensity() const = 0;
  virtual double adsorptionSurfaceBackSideAmbientIntensity() const = 0;
  virtual double adsorptionSurfaceBackSideSpecularIntensity() const = 0;
  virtual double adsorptionSurfaceBackSideShininess() const = 0;

  virtual void recomputeDensityProperties() = 0;
  virtual double2 frameworkProbeParameters() const = 0;
  virtual void setStructureHeliumVoidFraction(double value) = 0;
  virtual void setStructureNitrogenSurfaceArea(double value) = 0;

  virtual std::vector<RKInPerInstanceAttributesText> atomTextData(RKFontAtlas *fontAtlas) const = 0;
};

class RKRenderPrimitiveObjectsSource
{
public:
  virtual simd_quatd primitiveOrientation() const = 0;
  virtual double3x3 primitiveTransformationMatrix() const = 0;

  virtual double primitiveOpacity() const = 0;
  virtual bool primitiveIsCapped() const = 0;
  virtual bool primitiveIsFractional() const = 0;
  virtual int primitiveNumberOfSides() const = 0;
  virtual double primitiveThickness() const = 0;

  virtual RKSelectionStyle primitiveSelectionStyle() const = 0;
  virtual double primitiveSelectionIntensity() const = 0;
  virtual double primitiveSelectionScaling() const = 0;
  virtual double primitiveSelectionStripesDensity() const = 0;
  virtual double primitiveSelectionStripesFrequency() const = 0;
  virtual double primitiveSelectionWorleyNoise3DFrequency() const = 0;
  virtual double primitiveSelectionWorleyNoise3DJitter() const = 0;

  virtual double primitiveHue() const = 0;
  virtual double primitiveSaturation() const = 0;
  virtual double primitiveValue() const = 0;

  virtual bool primitiveFrontSideHDR() const = 0;
  virtual double primitiveFrontSideHDRExposure() const = 0;
  virtual QColor primitiveFrontSideAmbientColor() const = 0;
  virtual QColor primitiveFrontSideDiffuseColor() const = 0;
  virtual QColor primitiveFrontSideSpecularColor() const = 0;
  virtual double primitiveFrontSideDiffuseIntensity() const = 0;
  virtual double primitiveFrontSideAmbientIntensity() const = 0;
  virtual double primitiveFrontSideSpecularIntensity() const = 0;
  virtual double primitiveFrontSideShininess() const = 0;

  virtual bool primitiveBackSideHDR() const = 0;
  virtual double primitiveBackSideHDRExposure() const = 0;
  virtual QColor primitiveBackSideAmbientColor() const = 0;
  virtual QColor primitiveBackSideDiffuseColor() const = 0;
  virtual QColor primitiveBackSideSpecularColor() const = 0;
  virtual double primitiveBackSideDiffuseIntensity() const = 0;
  virtual double primitiveBackSideAmbientIntensity() const = 0;
  virtual double primitiveBackSideSpecularIntensity() const = 0;
  virtual double primitiveBackSideShininess() const = 0;
};

class RKRenderCrystalPrimitiveEllipsoidObjectsSource: public RKRenderPrimitiveObjectsSource
{
public:
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderCrystalPrimitiveEllipsoidObjects() const = 0;
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderSelectedCrystalPrimitiveEllipsoidObjects() const = 0;
};

class RKRenderCrystalPrimitiveCylinderObjectsSource: public RKRenderPrimitiveObjectsSource
{
public:
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderCrystalPrimitiveCylinderObjects() const = 0;
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderSelectedCrystalPrimitiveCylinderObjects() const = 0;
};

class RKRenderCrystalPrimitivePolygonalPrimsObjectsSource: public RKRenderPrimitiveObjectsSource
{
public:
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderCrystalPrimitivePolygonalPrismObjects() const = 0;
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderSelectedCrystalPrimitivePolygonalPrismObjects() const = 0;
};

class RKRenderPrimitiveEllipsoidObjectsSource: public RKRenderPrimitiveObjectsSource
{
public:
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderPrimitiveEllipsoidObjects() const = 0;
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderSelectedPrimitiveEllipsoidObjects() const = 0;
};

class RKRenderPrimitiveCylinderObjectsSource: public RKRenderPrimitiveObjectsSource
{
public:
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderPrimitiveCylinderObjects() const = 0;
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderSelectedPrimitiveCylinderObjects() const = 0;
};

class RKRenderPrimitivePolygonalPrimsObjectsSource: public RKRenderPrimitiveObjectsSource
{
public:
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderPrimitivePolygonalPrismObjects() const = 0;
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderSelectedPrimitivePolygonalPrismObjects() const = 0;
};

class RKRenderDataSource
{
public:
  virtual std::shared_ptr<RKCamera> camera() const = 0;
  virtual std::vector<size_t> numberOfScenes() const = 0;
  virtual int numberOfMovies(int sceneIndex) const = 0;

  virtual std::vector<std::shared_ptr<RKLight>>& renderLights() = 0;

  virtual std::vector<RKInPerInstanceAttributesAtoms> renderMeasurementPoints() const = 0;
  virtual std::vector<RKRenderStructure> renderMeasurementStructure() const = 0;

  virtual SKBoundingBox renderBoundingBox() const = 0;

  virtual bool hasSelectedObjects() const = 0;

  virtual RKBackgroundType renderBackgroundType() const = 0;
  virtual QColor renderBackgroundColor() const = 0;
  virtual const QImage renderBackgroundCachedImage() = 0;

  virtual int renderImageNumberOfPixels() const = 0;
  virtual double renderImagePhysicalSizeInInches() const = 0;

  virtual bool showBoundingBox() const = 0;
  virtual std::vector<RKInPerInstanceAttributesAtoms> renderBoundingBoxSpheres() const = 0;
  virtual std::vector<RKInPerInstanceAttributesBonds> renderBoundingBoxCylinders() const = 0;
};

class RKRenderViewController
{
 public:
  virtual void redraw() = 0;
  virtual void redrawWithQuality(RKRenderQuality quality) = 0;

  virtual void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures) = 0;
  virtual void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source) = 0;
  virtual void reloadData() = 0;
  virtual void reloadData(RKRenderQuality ambientOcclusionQuality) = 0;
  virtual void reloadAmbientOcclusionData() = 0;
  virtual void reloadRenderData() = 0;
  virtual void reloadSelectionData() = 0;
  virtual void reloadRenderMeasurePointsData() = 0;
  virtual void reloadBoundingBoxData() = 0;

  virtual void reloadBackgroundImage() = 0;

  virtual void invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderStructure>> structures) = 0;
  virtual void invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderStructure>> structures) = 0;
  virtual void computeHeliumVoidFraction(std::vector<std::shared_ptr<RKRenderStructure>> structures) = 0;
  virtual void computeNitrogenSurfaceArea(std::vector<std::shared_ptr<RKRenderStructure>> structures) = 0;

  virtual void updateTransformUniforms() = 0;
  virtual void updateStructureUniforms() = 0;
  virtual void updateIsosurfaceUniforms() = 0;
  virtual void updateLightUniforms() = 0;

  virtual void updateVertexArrays() = 0;

  virtual QImage renderSceneToImage(int width, int height, RKRenderQuality quality) = 0;

  virtual std::array<int,4> pickTexture(int x, int y, int width, int height) = 0;
};
