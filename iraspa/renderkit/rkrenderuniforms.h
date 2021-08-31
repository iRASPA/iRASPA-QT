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
//#include "rkrenderkitprotocols.h"

class RKRenderStructure;
class RKRenderDataSource;
//#include <QtOpenGL>

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

const std::string OpenGLVersionStringLiteral = R"foo(
#version 330
)foo";

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
  float4x4 viewMatrix = float4x4(1.0);
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
  float numberOfMultiSamplePoints = 8.0;
  float intPad1;
  float intPad2;
  float intPad3;
  float bloomLevel = 1.0;
  float bloomPulse = 1.0;
  float padFloat1 = 0.0;
  float padFloat2 = 0.0;
  float4 padVector3 = float4();
  float4 padvector4 = float4();

  RKTransformationUniforms() {};
  RKTransformationUniforms(double4x4 projectionMatrix, double4x4 modelViewMatrix, double4x4 axesProjectionMatrix, double4x4 axesModelViewMatrix, double bloomLevel, double bloomPulse, int multiSampling);
};

const std::string  OpenGLFrameUniformBlockStringLiteral = R"foo(
layout (std140) uniform FrameUniformBlock
{
  mat4 projectionMatrix;
  mat4 viewMatrix;
  mat4 mvpMatrix;
  mat4 shadowMatrix;
  mat4 projectionMatrixInverse;
  mat4 viewMatrixInverse;
  mat4 normalMatrix;

  mat4 axesProjectionMatrix;
  mat4 axesViewMatrix;
  mat4 axesMvpMatrix;
  mat4 padMatrix;

  float numberOfMultiSamplePoints;
  float padInt1;
  float padInt2;
  float padInt3;
  float bloomLevel;
  float bloomPulse;
  float padFloat1;
  float padFloat2;
  vec4 padvector3;
  vec4 padvector4;
} frameUniforms;
)foo";

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

  float4x4 modelMatrix = float4x4(double4x4());

  //----------------------------------------  384 bytes boundary

  float4x4 boxMatrix = float4x4();
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

  //----------------------------------------  512 bytes boundary

  float4x4 transformationMatrix = float4x4();
  float4x4 transformationNormalMatrix = float4x4();

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

  //----------------------------------------  768 bytes boundary

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
  float pad9;

  float4x4 pad10 = 0.0f;
  float4x4 pad11 = 0.0f;
  float4x4 pad12 = 0.0f;

  RKStructureUniforms() {}
  RKStructureUniforms(size_t sceneIdentifier, size_t movieIdentifier, std::shared_ptr<RKRenderStructure> structure);
  RKStructureUniforms(size_t sceneIdentifier, size_t movieIdentifier, std::shared_ptr<RKRenderStructure> structure, double4x4 inverseModelMatrix);
};

const std::string  OpenGLStructureUniformBlockStringLiteral = R"foo(
layout (std140) uniform StructureUniformBlock
{
    int sceneIdentifier;
    int MovieIdentifier;
    float atomScaleFactor;
    int numberOfMultiSamplePoints;

    bool ambientOcclusion;
    int ambientOcclusionPatchNumber;
    float ambientOcclusionPatchSize;
    float ambientOcclusionInverseTextureSize;

    float atomHue;
    float atomSaturation;
    float atomValue;
    float pad111;

    bool atomHDR;
    float atomHDRExposure;
    float atomSelectionIntensity;
    bool clipAtomsAtUnitCell;

    vec4 atomAmbientColor;
    vec4 atomDiffuseColor;
    vec4 atomSpecularColor;
    float atomShininess;

    float bondHue;
    float bondSaturation;
    float bondValue;

    //----------------------------------------  128 bytes boundary

    bool bondHDR;
    float bondHDRExposure;
    float bondSelectionIntensity;
    bool clipBondsAtUnitCell;

    vec4 bondAmbientColor;
    vec4 bondDiffuseColor;
    vec4 bondSpecularColor;

    float bondShininess;
    float bondScaling;
    int bondColorMode;
    float unitCellScaling;
    vec4 unitCellColor;

    vec4 clipPlaneLeft;
    vec4 clipPlaneRight;

    //----------------------------------------  256 bytes boundary

    vec4 clipPlaneTop;
    vec4 clipPlaneBottom;
    vec4 clipPlaneFront;
    vec4 clipPlaneBack;

    mat4 modelMatrix;

    //----------------------------------------  384 bytes boundary

    mat4 boxMatrix;
    float atomSelectionStripesDensity;
    float atomSelectionStripesFrequency;
    float atomSelectionWorleyNoise3DFrequency;
    float atomSelectionWorleyNoise3DJitter;

    vec4 atomAnnotationTextDisplacement;
    vec4 atomAnnotationTextColor;
    float atomAnnotationTextScaling;
    float atomSelectionScaling;
    float bondSelectionScaling;
    bool colorAtomsWithBondColor;

    //----------------------------------------  512 bytes boundary

    mat4 transformationMatrix;
    mat4 transformationNormalMatrix;

    vec4 primitiveAmbientFrontSide;
    vec4 primitiveDiffuseFrontSide;
    vec4 primitiveSpecularFrontSide;
    bool primitiveFrontSideHDR;
    float primitiveFrontSideHDRExposure;
    float primitiveOpacity;
    float primitiveShininessFrontSide;

    vec4 primitiveAmbientBackSide;
    vec4 primitiveDiffuseBackSide;
    vec4 primitiveSpecularBackSide;
    bool primitiveBackSideHDR;
    float primitiveBackSideHDRExposure;
    float pad6;
    float primitiveShininessBackSide;

    //----------------------------------------  768 bytes boundary

    float bondSelectionStripesDensity;
    float bondSelectionStripesFrequency;
    float bondSelectionWorleyNoise3DFrequency;
    float bondSelectionWorleyNoise3DJitter;

    float primitiveSelectionStripesDensity;
    float primitiveSelectionStripesFrequency;
    float primitiveSelectionWorleyNoise3DFrequency;
    float primitiveSelectionWorleyNoise3DJitter;

    float primitiveSelectionScaling;
    float primitiveSelectionIntensity;
    float pad7;
    float pad8;

    float primitiveHue;
    float primitiveSaturation;
    float primitiveValue;
    float pad9;

    mat4 pad10;
    mat4 pad11;
    mat4 pad12;
} structureUniforms;
)foo";

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

const std::string  OpenGLShadowUniformBlockStringLiteral = R"foo(
layout (std140) uniform ShadowUniformBlock
{
  mat4 projectionMatrix;
  mat4 viewMatrix;
  mat4 shadowMatrix;
  mat4 normalMatrix;
} shadowUniforms;
)foo";

struct RKIsosurfaceUniforms
{
  float4x4 unitCellMatrix = float4x4();
  float4x4 unitCellNormalMatrix = float4x4();

  //----------------------------------------  128 bytes boundary

  float4 ambientFrontSide = float4(0.0f, 0.0f, 0.0f, 1.0f);
  float4 diffuseFrontSide = float4(0.588235f, 0.670588f, 0.729412f, 1.0f);
  float4 specularFrontSide = float4(1.0f, 1.0f, 1.0f, 1.0f);
  int32_t frontHDR = true;
  float frontHDRExposure = 1.5;
  float pad1 = 0.0;
  float shininessFrontSide = 4.0;

  float4 ambientBackSide = float4(0.0f, 0.0f, 0.0f, 1.0f);
  float4 diffuseBackSide = float4(0.588235f, 0.670588f, 0.729412f, 1.0f);
  float4 specularBackSide = float4(0.9f, 0.9f, 0.9f, 1.0f);
  int32_t backHDR = true;
  float backHDRExposure = 1.5;
  float pad2 = 0.0;
  float shininessBackSide = 4.0;

  //----------------------------------------  256 bytes boundary

  float hue;
  float saturation;
  float value;
  float pad3;

  float4 pad4;
  float4 pad5;
  float4 pad6;
  float4x4 pad7;

  //----------------------------------------  384 bytes boundary

  float4x4 pad8;
  float4x4 pad9;

  RKIsosurfaceUniforms();
  RKIsosurfaceUniforms(std::shared_ptr<RKRenderStructure> structure);
};

const std::string  OpenGLIsosurfaceUniformBlockStringLiteral = R"foo(
layout (std140) uniform IsosurfaceUniformBlock
{
  mat4 unitCellMatrix;
  mat4 unitCellNormalMatrix;

  vec4 ambientFrontSide;
  vec4 diffuseFrontSide;
  vec4 specularFrontSide;
  bool frontHDR;
  float frontHDRExposure;
  float pad1;
  float shininessFrontSide;

  vec4 ambientBackSide;
  vec4 diffuseBackSide;
  vec4 specularBackSide;
  bool backHDR;
  float backHDRExposure;
  float pad2;
  float shininessBackSide;

  float hue;
  float saturation;
  float value;
  float pad3;

  vec4 pad4;
  vec4 pad5;
  vec4 pad6;
  mat4 pad7;

  mat4 pad8;
  mat4 pad9;
} isosurfaceUniforms;
)foo";

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

const std::string  OpenGLLightUniformBlockStringLiteral = R"foo(
struct Light
{
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  vec4  spotDirection;

  float constantAttenuation;
  float linearAttenuation;
  float quadraticAttenuation;
  float spotCutoff;

  float spotExponent;
  float shininess;
  float pad1;
  float pad2;
  float pad3;
  float pad4;
  float pad5;
  float pad6;
};

layout (std140) uniform LightsUniformBlock
{
  Light lights[4];
} lightUniforms;
)foo";

const std::string  OpenGLRGBHSVStringLiteral = R"foo(
vec3 rgb2hsv(vec3 c)
{
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
)foo";

const std::string  OpenGLWorleyNoise3DStringLiteral = R"foo(

vec3 mod289(vec3 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x)
{
  return mod289(((x*34.0)+1.0)*x);
}

vec2 cellular3D(vec3 P, float jitter)
{
  #define K 0.142857142857
  #define Ko 0.428571428571
  #define K2 0.020408163265306
  #define Kz 0.166666666667
  #define Kzo 0.416666666667

  vec3 Pi = mod(floor(P), 289.0);
  vec3 Pf = fract(P) - 0.5;

  vec3 Pfx = Pf.x + vec3(1.0, 0.0, -1.0);
  vec3 Pfy = Pf.y + vec3(1.0, 0.0, -1.0);
  vec3 Pfz = Pf.z + vec3(1.0, 0.0, -1.0);

  vec3 p = permute(Pi.x + vec3(-1.0, 0.0, 1.0));
  vec3 p1 = permute(p + Pi.y - 1.0);
  vec3 p2 = permute(p + Pi.y);
  vec3 p3 = permute(p + Pi.y + 1.0);

  vec3 p11 = permute(p1 + Pi.z - 1.0);
  vec3 p12 = permute(p1 + Pi.z);
  vec3 p13 = permute(p1 + Pi.z + 1.0);

  vec3 p21 = permute(p2 + Pi.z - 1.0);
  vec3 p22 = permute(p2 + Pi.z);
  vec3 p23 = permute(p2 + Pi.z + 1.0);

  vec3 p31 = permute(p3 + Pi.z - 1.0);
  vec3 p32 = permute(p3 + Pi.z);
  vec3 p33 = permute(p3 + Pi.z + 1.0);

  vec3 ox11 = fract(p11*K) - Ko;
  vec3 oy11 = mod(floor(p11*K), 7.0)*K - Ko;
  vec3 oz11 = floor(p11*K2)*Kz - Kzo; // p11 < 289 guaranteed

  vec3 ox12 = fract(p12*K) - Ko;
  vec3 oy12 = mod(floor(p12*K), 7.0)*K - Ko;
  vec3 oz12 = floor(p12*K2)*Kz - Kzo;

  vec3 ox13 = fract(p13*K) - Ko;
  vec3 oy13 = mod(floor(p13*K), 7.0)*K - Ko;
  vec3 oz13 = floor(p13*K2)*Kz - Kzo;

  vec3 ox21 = fract(p21*K) - Ko;
  vec3 oy21 = mod(floor(p21*K), 7.0)*K - Ko;
  vec3 oz21 = floor(p21*K2)*Kz - Kzo;

  vec3 ox22 = fract(p22*K) - Ko;
  vec3 oy22 = mod(floor(p22*K), 7.0)*K - Ko;
  vec3 oz22 = floor(p22*K2)*Kz - Kzo;

  vec3 ox23 = fract(p23*K) - Ko;
  vec3 oy23 = mod(floor(p23*K), 7.0)*K - Ko;
  vec3 oz23 = floor(p23*K2)*Kz - Kzo;

  vec3 ox31 = fract(p31*K) - Ko;
  vec3 oy31 = mod(floor(p31*K), 7.0)*K - Ko;
  vec3 oz31 = floor(p31*K2)*Kz - Kzo;

  vec3 ox32 = fract(p32*K) - Ko;
  vec3 oy32 = mod(floor(p32*K), 7.0)*K - Ko;
  vec3 oz32 = floor(p32*K2)*Kz - Kzo;

  vec3 ox33 = fract(p33*K) - Ko;
  vec3 oy33 = mod(floor(p33*K), 7.0)*K - Ko;
  vec3 oz33 = floor(p33*K2)*Kz - Kzo;

  vec3 dx11 = Pfx + jitter*ox11;
  vec3 dy11 = Pfy.x + jitter*oy11;
  vec3 dz11 = Pfz.x + jitter*oz11;

  vec3 dx12 = Pfx + jitter*ox12;
  vec3 dy12 = Pfy.x + jitter*oy12;
  vec3 dz12 = Pfz.y + jitter*oz12;

  vec3 dx13 = Pfx + jitter*ox13;
  vec3 dy13 = Pfy.x + jitter*oy13;
  vec3 dz13 = Pfz.z + jitter*oz13;

  vec3 dx21 = Pfx + jitter*ox21;
  vec3 dy21 = Pfy.y + jitter*oy21;
  vec3 dz21 = Pfz.x + jitter*oz21;

  vec3 dx22 = Pfx + jitter*ox22;
  vec3 dy22 = Pfy.y + jitter*oy22;
  vec3 dz22 = Pfz.y + jitter*oz22;

  vec3 dx23 = Pfx + jitter*ox23;
  vec3 dy23 = Pfy.y + jitter*oy23;
  vec3 dz23 = Pfz.z + jitter*oz23;

  vec3 dx31 = Pfx + jitter*ox31;
  vec3 dy31 = Pfy.z + jitter*oy31;
  vec3 dz31 = Pfz.x + jitter*oz31;

  vec3 dx32 = Pfx + jitter*ox32;
  vec3 dy32 = Pfy.z + jitter*oy32;
  vec3 dz32 = Pfz.y + jitter*oz32;

  vec3 dx33 = Pfx + jitter*ox33;
  vec3 dy33 = Pfy.z + jitter*oy33;
  vec3 dz33 = Pfz.z + jitter*oz33;

  vec3 d11 = dx11 * dx11 + dy11 * dy11 + dz11 * dz11;
  vec3 d12 = dx12 * dx12 + dy12 * dy12 + dz12 * dz12;
  vec3 d13 = dx13 * dx13 + dy13 * dy13 + dz13 * dz13;
  vec3 d21 = dx21 * dx21 + dy21 * dy21 + dz21 * dz21;
  vec3 d22 = dx22 * dx22 + dy22 * dy22 + dz22 * dz22;
  vec3 d23 = dx23 * dx23 + dy23 * dy23 + dz23 * dz23;
  vec3 d31 = dx31 * dx31 + dy31 * dy31 + dz31 * dz31;
  vec3 d32 = dx32 * dx32 + dy32 * dy32 + dz32 * dz32;
  vec3 d33 = dx33 * dx33 + dy33 * dy33 + dz33 * dz33;

  // Sort out the two smallest distances (F1, F2)
#if 0
  // Cheat and sort out only F1
  vec3 d1 = min(min(d11,d12), d13);
  vec3 d2 = min(min(d21,d22), d23);
  vec3 d3 = min(min(d31,d32), d33);
  vec3 d = min(min(d1,d2), d3);
  d.x = min(min(d.x,d.y),d.z);
  return sqrt(d.xx); // F1 duplicated, no F2 computed
#else
  // Do it right and sort out both F1 and F2
  vec3 d1a = min(d11, d12);
  d12 = max(d11, d12);
  d11 = min(d1a, d13); // Smallest now not in d12 or d13
  d13 = max(d1a, d13);
  d12 = min(d12, d13); // 2nd smallest now not in d13
  vec3 d2a = min(d21, d22);
  d22 = max(d21, d22);
  d21 = min(d2a, d23); // Smallest now not in d22 or d23
  d23 = max(d2a, d23);
  d22 = min(d22, d23); // 2nd smallest now not in d23
  vec3 d3a = min(d31, d32);
  d32 = max(d31, d32);
  d31 = min(d3a, d33); // Smallest now not in d32 or d33
  d33 = max(d3a, d33);
  d32 = min(d32, d33); // 2nd smallest now not in d33
  vec3 da = min(d11, d21);
  d21 = max(d11, d21);
  d11 = min(da, d31); // Smallest now in d11
  d31 = max(da, d31); // 2nd smallest now not in d31
  d11.xy = (d11.x < d11.y) ? d11.xy : d11.yx;
  d11.xz = (d11.x < d11.z) ? d11.xz : d11.zx; // d11.x now smallest
  d12 = min(d12, d21); // 2nd smallest now not in d21
  d12 = min(d12, d22); // nor in d22
  d12 = min(d12, d31); // nor in d31
  d12 = min(d12, d32); // nor in d32
  d11.yz = min(d11.yz,d12.xy); // nor in d12.yz
  d11.y = min(d11.y,d12.z); // Only two more to go
  d11.y = min(d11.y,d11.z); // Done! (Phew!)
  return sqrt(d11.xy); // F1, F2
#endif
}
)foo";


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

const std::string  OpenGLGlobalAxesUniformBlockStringLiteral = R"foo(
layout (std140) uniform GlobalAxesUniformBlock
{
  vec4 axesBackgroundColor;
  vec4 textColor[3];
  vec4 textDisplacement[3];
  int axesBackGroundStyle;
  float axesScale;
  float centerScale;
  float textOffset;
  vec4 textScale;
} globalAxesUniforms;
)foo";
