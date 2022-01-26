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

#include <mathkit.h>
#include <QColor>
#include <memory>

// Uniform                                  binding point
// RKTransformationUniforms                 0
// RKStructureUniforms                      1
// RKIsosurfaceUniforms                     2
// RKLightsUniforms                         3
//
// RKGlobalAxesUniforms                     5
// RKStructureUniforms (ambient occlusion)  6
// ShadowUniformBlock  (ambient occlusion)  7

class RKRenderObject;
class RKRenderDataSource;


enum class RKEnergySurfaceType: qint64
{
  isoSurface = 0, volumeRendering = 1, multiple_values = 2
};

enum class RKPredefinedVolumeRenderingTransferFunction: qint64
{
  RASPA_PES = 0,
  CoolWarmDiverging = 1,
  Xray = 2,
  Gray = 3,
  Rainbow = 4,
  Turbo = 5,
  Gnuplot = 6,
  Spectral = 7,
  Cool = 8,
  Viridis = 9,
  Plasma = 10,
  Inferno = 11,
  Magma = 12,
  Cividis = 13,
  Spring = 14,
  Summer = 15,
  Autumn = 16,
  Winter = 17,
  Reds = 18,
  Blues = 19,
  Greens = 20,
  Purples = 21,
  Oranges = 22,
  multiple_values = 23
};

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

struct RKVertex
{
  RKVertex():position(float4()),normal(float4()), st(float2()) {}
  RKVertex(float4 pos, float4 norm, float2 c): position(pos), normal(norm), st(c) {}

  float4 position;
  float4 normal;
  float2 st;
  float2 pad = float2();
};

struct RKPrimitiveVertex
{
  RKPrimitiveVertex(float4 pos, float4 norm, float4 c, float2 uv): position(pos), normal(norm), color(c), st(uv) {}
  float4 position;
  float4 normal;
  float4 color;
  float2 st;
  float2 pad;
};

struct RKTextVertex
{
  float4 position  = float4();
  float4 st = float4();
};

struct RKBondVertex
{
  float4 position1;
  float4 position2;
};

struct RKInPerInstanceAttributesText
{
  float4 position = float4();
  float4 scale = float4();
  float4 vertexCoordinatesData = float4();
  float4 textureCoordinatesData = float4();

  RKInPerInstanceAttributesText(float4 p, float4 s, float4 v, float4 t): position(p), scale(s), vertexCoordinatesData(v), textureCoordinatesData(t) {}
};

struct RKInPerInstanceAttributesAtoms
{
  float4 position = float4();
  float4 ambient = float4();
  float4 diffuse = float4();
  float4 specular = float4();
  float4 scale = float4();
  int32_t tag = int32_t();
  RKInPerInstanceAttributesAtoms(float4 p, float4 a, float4 d, float4 s, float4 sc, int32_t tag): position(p), ambient(a), diffuse(d), specular(s), scale(sc), tag(tag) {}
};

struct RKInPerInstanceAttributesBonds
{
  float4 position1 = float4();
  float4 position2 = float4();
  float4 color1 = float4();
  float4 color2 = float4();
  float4 scale = float4();
  int32_t tag  = int32_t();
  int32_t type  = int32_t();
  RKInPerInstanceAttributesBonds() {}
  RKInPerInstanceAttributesBonds(float4 pos1, float4 pos2, float4 c1, float4 c2, float4 sc, int32_t tag, int32_t type): position1(pos1), position2(pos2), color1(c1), color2(c2), scale(sc), tag(tag), type(type) {}
};


// Note: must be aligned at vector-length (16-bytes boundaries, 4 Floats of 4 bytes)
// current number of bytes: 512 bytes

struct RKTransformationUniforms
{
  float4x4 projectionMatrix = float4x4(1.0);
  float4x4 modelViewMatrix = float4x4(1.0);
  float4x4 mvpMatrix = float4x4();
  float4x4 shadowMatrix = float4x4();
  float4x4 projectionMatrixInverse = float4x4();
  float4x4 viewMatrixInverse = float4x4();
  float4x4 normalMatrix = float4x4();

  float4x4 axesProjectionMatrix = float4x4();
  float4x4 axesViewMatrix = float4x4();
  float4x4 axesMvpMatrix = float4x4();
  float4x4 padMatrix = float4x4();

  // moved 'numberOfMultiSamplePoints' to here (for downsampling when no structures are present)
  float4 cameraPosition = float4();
  float4 padvector4 = float4();
  float numberOfMultiSamplePoints = 8.0;
  float intPad1;
  float intPad2;
  float intPad3;
  float bloomLevel = 1.0;
  float bloomPulse = 1.0;
  float padFloat1 = 0.0;
  float padFloat2 = 0.0;

  RKTransformationUniforms() {};
  RKTransformationUniforms(double4x4 projectionMatrix, double4x4 modelViewMatrix, double4x4 modelMatrix, double4x4 viewMatrix, double4x4 axesProjectionMatrix, double4x4 axesModelViewMatrix, bool isOrthographic, double bloomLevel, double bloomPulse, int multiSampling);
};


// IMPORTANT: must be aligned on 256-bytes boundaries
// current number of bytes: 512 bytes
struct RKStructureUniforms
{
  int32_t sceneIdentifier = 0;
  int32_t MovieIdentifier = 0;
  float atomScaleFactor = 1.0f;
  int32_t numberOfMultiSamplePoints = 8;

  int32_t ambientOcclusion = false;
  int32_t ambientOcclusionPatchNumber = 64;
  float ambientOcclusionPatchSize = 16.0f;
  float ambientOcclusionInverseTextureSize = float(1.0/1024.0);

  float atomHue = 1.0f;
  float atomSaturation = 1.0f;
  float atomValue = 1.0f;
  float pad111 = 1.0f;

  int32_t atomHDR = true;
  float atomHDRExposure = 1.5f;
  float atomSelectionIntensity = 0.5f;
  int32_t clipAtomsAtUnitCell = false;

  float4 atomAmbient = float4(1.0, 1.0, 1.0, 1.0);
  float4 atomDiffuse = float4(1.0, 1.0, 1.0, 1.0);
  float4 atomSpecular = float4(1.0, 1.0, 1.0, 1.0);
  float atomShininess = 4.0;

  float bondHue = 0.0f;
  float bondSaturation = 0.0f;
  float bondValue = 0.0f;

  //----------------------------------------  128 bytes boundary

  int32_t bondHDR = true;
  float bondHDRExposure = 1.5f;
  float bondSelectionIntensity = 1.0f;
  int32_t clipBondsAtUnitCell = false;


  float4 bondAmbientColor = float4(1.0, 1.0, 1.0, 1.0);
  float4 bondDiffuseColor = float4(1.0, 1.0, 1.0, 1.0);
  float4 bondSpecularColor = float4(1.0, 1.0, 1.0, 1.0);

  float bondShininess = 4.0f;
  float bondScaling = 1.0f;
  int32_t bondColorMode = 0;

  float unitCellScaling = 1.0f;
  float4 unitCellDiffuseColor = float4(1.0, 1.0, 1.0, 1.0);

  float4 clipPlaneLeft = float4(1.0, 1.0, 1.0, 1.0);
  float4 clipPlaneRight = float4(1.0, 1.0, 1.0, 1.0);

  //----------------------------------------  256 bytes boundary

  float4 clipPlaneTop = float4(1.0, 1.0, 1.0, 1.0);
  float4 clipPlaneBottom = float4(1.0, 1.0, 1.0, 1.0);

  float4 clipPlaneFront = float4(1.0, 1.0, 1.0, 1.0);
  float4 clipPlaneBack = float4(1.0, 1.0, 1.0, 1.0);

  float4x4 modelMatrix = float4x4(0.0f);

  //----------------------------------------  384 bytes boundary

  float4x4 inverseModelMatrix = float4x4(double4x4());
  float4x4 boxMatrix = float4x4();

  //----------------------------------------  512 bytes boundary

  float4x4 inverseBoxMatrix = float4x4(double4x4());
  float atomSelectionStripesDensity = 0.25f;
  float atomSelectionStripesFrequency = 12.0f;
  float atomSelectionWorleyNoise3DFrequency = 2.0f;
  float atomSelectionWorleyNoise3DJitter = 0.0f;

  float4 atomAnnotationTextDisplacement = float4();
  float4 atomAnnotationTextColor = float4(0.0,0.0,0.0,1.0);
  float atomAnnotationTextScaling = 1.0f;
  float atomSelectionScaling = 1.0f;
  float bondSelectionScaling = 1.25f;
  int32_t colorAtomsWithBondColor = false;

  //----------------------------------------  640 bytes boundary

  float4x4 primitiveTransformationMatrix = float4x4();
  float4x4 primitiveTransformationNormalMatrix = float4x4();

  //----------------------------------------  768 bytes boundary

  float4 primitiveAmbientFrontSide = float4(0.0,0.0,0.0,1.0);
  float4 primitiveDiffuseFrontSide = float4(0.0,0.0,0.0,1.0);
  float4 primitiveSpecularFrontSide = float4(0.0,0.0,0.0,1.0);
  int32_t primitiveFrontSideHDR = true;
  float primitiveFrontSideHDRExposure = 1.5f;
  float primitiveOpacity = 1.0f;
  float primitiveShininessFrontSide = 4.0f;

  float4 primitiveAmbientBackSide = float4(0.0,0.0,0.0,1.0);
  float4 primitiveDiffuseBackSide = float4(0.0,0.0,0.0,1.0);
  float4 primitiveSpecularBackSide = float4(0.0,0.0,0.0,1.0);
  int32_t primitiveBackSideHDR = true;
  float primitiveBackSideHDRExposure = 1.5f;
  float pad6 = 0.0f;
  float primitiveShininessBackSide = 4.0f;

  //----------------------------------------  896 bytes boundary

  float bondSelectionStripesDensity = 0.25f;
  float bondSelectionStripesFrequency = 12.0f;
  float bondSelectionWorleyNoise3DFrequency = 2.0f;
  float bondSelectionWorleyNoise3DJitter = 1.0f;

  float primitiveSelectionStripesDensity = 0.25f;
  float primitiveSelectionStripesFrequency = 12.0f;
  float primitiveSelectionWorleyNoise3DFrequency = 2.0f;
  float primitiveSelectionWorleyNoise3DJitter = 1.0f;

  float primitiveSelectionScaling = 1.01f;
  float primitiveSelectionIntensity = 0.8f;
  float pad7 = 0.0f;
  float pad8 = 0.0f;

  float primitiveHue = 1.0f;
  float primitiveSaturation = 1.0f;
  float primitiveValue = 1.0f;
  float pad9 = 0.0f;

  float4 localAxisPosition = float4(0.0f,0.0f,0.0f,0.0f);
  float4 numberOfReplicas = float4(0.0f,0.0f,0.0f,0.0f);
  float4 pad11 = float4(0.0f,0.0f,0.0f,0.0f);
  float4 pad12 = float4(0.0f,0.0f,0.0f,0.0f);


  RKStructureUniforms() {}
  RKStructureUniforms(size_t sceneIdentifier, size_t movieIdentifier, std::shared_ptr<RKRenderObject> structure);
  RKStructureUniforms(size_t sceneIdentifier, size_t movieIdentifier, std::shared_ptr<RKRenderObject> structure, double4x4 inverseModelMatrix);
};


// IMPORTANT: must be aligned on 256-bytes boundaries
// current number of bytes: 256 bytes
struct RKShadowUniforms
{
  float4x4 projectionMatrix = float4x4();
  float4x4 viewMatrix = float4x4();
  float4x4 shadowMatrix = float4x4();
  float4x4 normalMatrix = float4x4();

  RKShadowUniforms();
  RKShadowUniforms(double4x4 projectionMatrix, double4x4 viewMatrix, double4x4 modelMatrix);
};


struct RKIsosurfaceUniforms
{
  float4x4 unitCellMatrix = float4x4();
  float4x4 inverseUnitCellMatrix = float4x4();
  float4x4 unitCellNormalMatrix = float4x4();

  float4x4 boxMatrix = float4x4();
  float4x4 inverseBoxMatrix = float4x4();

  //----------------------------------------  128 bytes boundary

  float4 ambientFrontSide = float4(0.0f, 0.0f, 0.0f, 1.0f);
  float4 diffuseFrontSide = float4(0.588235f, 0.670588f, 0.729412f, 1.0f);
  float4 specularFrontSide = float4(1.0f, 1.0f, 1.0f, 1.0f);
  int32_t frontHDR = true;
  float frontHDRExposure = 1.5;
  float transparencyThreshold = 0.0;
  float shininessFrontSide = 4.0;

  float4 ambientBackSide = float4(0.0f, 0.0f, 0.0f, 1.0f);
  float4 diffuseBackSide = float4(0.588235f, 0.670588f, 0.729412f, 1.0f);
  float4 specularBackSide = float4(0.9f, 0.9f, 0.9f, 1.0f);
  int32_t backHDR = true;
  float backHDRExposure = 1.5;
  int32_t transferFunctionIndex;
  float shininessBackSide = 4.0;

  //----------------------------------------  256 bytes boundary

  float hue;
  float saturation;
  float value;
  float stepLength = 0.001;

  float4 scaleToEncompassing;
  float4 pad5;
  float4 pad6;


  //----------------------------------------  384 bytes boundary



  RKIsosurfaceUniforms();
  RKIsosurfaceUniforms(std::shared_ptr<RKRenderObject> structure);
};



struct RKLightUniform
{
  float4 position = float4(0.0, 0.0, 100.0, 0.0);  // w=0 directional light, w=1.0 positional light
  float4 ambient = float4(1.0, 1.0, 1.0, 1.0);
  float4 diffuse = float4(1.0, 1.0, 1.0, 1.0);
  float4 specular = float4(1.0, 1.0, 1.0, 1.0);

  float4 spotDirection = float4(1.0, 1.0, 1.0, 0.0);
  float constantAttenuation = 1.0;
  float linearAttenuation = 1.0;
  float quadraticAttenuation = 1.0;
  float spotCutoff = 1.0;

  float spotExponent = 1.0;
  float shininess = 4.0;
  float pad1 = 0.0;
  float pad2 = 0.0;

  float pad3 = 0.0;
  float pad4 = 0.0;
  float pad5 = 0.0;
  float pad6 = 0.0;

  RKLightUniform();
  RKLightUniform(std::shared_ptr<RKRenderDataSource> project, int lightIndex);
};

struct RKLightsUniforms
{
  RKLightsUniforms() {}
  RKLightsUniforms(std::shared_ptr<RKRenderDataSource> project);
  std::vector<RKLightUniform> lights{RKLightUniform(),RKLightUniform(),RKLightUniform(),RKLightUniform()};
};

struct RKGlobalAxesUniforms
{
  float4 axesBackgroundColor = float4(0.8f,0.8f,0.8f,0.05f);
  float3x4 textColor = float3x4(float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
  float3x4 textDisplacement = float3x4(float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
  int32_t axesBackGroundStyle = 1;
  float axesScale = 5.0f;
  float centerScale = 0.0f;
  float textOffset = 0.0f;
  float4 textScale = float4(1.0f, 1.0f, 1.0f, 1.0f);

  RKGlobalAxesUniforms(std::shared_ptr<RKRenderDataSource> project);
};
