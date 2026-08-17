layout(std140, set = 0, binding = 0) uniform FrameUniformBlock
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

  vec4 cameraPosition;
  vec4 padvector4;
  float numberOfMultiSamplePoints;
  float padInt1;
  float padInt2;
  float padInt3;
  float bloomLevel;
  float bloomPulse;
  float padFloat1;
  float padFloat2;
} frameUniforms;

layout(std140, set = 0, binding = 1) uniform StructureUniformBlock
{
  int sceneIdentifier;
  int MovieIdentifier;
  float atomScaleFactor;
  int numberOfMultiSamplePoints;

  int ambientOcclusion;
  int ambientOcclusionPatchNumber;
  float ambientOcclusionPatchSize;
  float ambientOcclusionInverseTextureSize;

  float atomHue;
  float atomSaturation;
  float atomValue;
  int structureIdentifier;

  int atomHDR;
  float atomHDRExposure;
  float atomSelectionIntensity;
  int clipAtomsAtUnitCell;

  vec4 atomAmbientColor;
  vec4 atomDiffuseColor;
  vec4 atomSpecularColor;
  float atomShininess;

  float bondHue;
  float bondSaturation;
  float bondValue;

  int bondHDR;
  float bondHDRExposure;
  float bondSelectionIntensity;
  int clipBondsAtUnitCell;

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

  vec4 clipPlaneTop;
  vec4 clipPlaneBottom;
  vec4 clipPlaneFront;
  vec4 clipPlaneBack;

  mat4 modelMatrix;
  mat4 inverseModelMatrix;
  mat4 boxMatrix;
  mat4 inverseBoxMatrix;
  float atomSelectionStripesDensity;
  float atomSelectionStripesFrequency;
  float atomSelectionWorleyNoise3DFrequency;
  float atomSelectionWorleyNoise3DJitter;

  vec4 atomAnnotationTextDisplacement;
  vec4 atomAnnotationTextColor;
  float atomAnnotationTextScaling;
  float atomSelectionScaling;
  float bondSelectionScaling;
  int colorAtomsWithBondColor;

  mat4 transformationMatrix;
  mat4 transformationNormalMatrix;

  vec4 primitiveAmbientFrontSide;
  vec4 primitiveDiffuseFrontSide;
  vec4 primitiveSpecularFrontSide;
  int primitiveFrontSideHDR;
  float primitiveFrontSideHDRExposure;
  float primitiveOpacity;
  float primitiveShininessFrontSide;

  vec4 primitiveAmbientBackSide;
  vec4 primitiveDiffuseBackSide;
  vec4 primitiveSpecularBackSide;
  int primitiveBackSideHDR;
  float primitiveBackSideHDRExposure;
  float pad6;
  float primitiveShininessBackSide;

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
  int isUnity;
  float pad7;

  float primitiveHue;
  float primitiveSaturation;
  float primitiveValue;
  float pad8;

  vec4 localAxisPosition;
  vec4 numberOfReplicas;
  vec4 ribbonCoilColor;
  vec4 ribbonHelixColor;
  vec4 ribbonSheetColor;
  int ribbonHDR;
  float ribbonHDRExposure;
  float ribbonHue;
  float ribbonSaturation;
  float ribbonValue;
  int ribbonAmbientOcclusion;
  float padRibbon1;
  float ribbonShininess;
  float padRibbon2;
  float padRibbon3;
  float padRibbon4;
  float padRibbon5;
  vec4 ribbonAmbientColor;
  vec4 ribbonDiffuseColor;
  vec4 ribbonSpecularColor;
} structureUniforms;

layout(std140, set = 0, binding = 2) uniform IsosurfaceUniformBlock
{
  mat4 unitCellMatrix;
  mat4 inverseUnitCellMatrix;
  mat4 unitCellNormalMatrix;

  mat4 boxMatrix;
  mat4 inverseBoxMatrix;

  vec4 ambientFrontSide;
  vec4 diffuseFrontSide;
  vec4 specularFrontSide;
  int frontHDR;
  float frontHDRExposure;
  float transparencyThreshold;
  float shininessFrontSide;

  vec4 ambientBackSide;
  vec4 diffuseBackSide;
  vec4 specularBackSide;
  int backHDR;
  float backHDRExposure;
  int transferFunctionIndex;
  float shininessBackSide;

  float hue;
  float saturation;
  float value;
  float stepLength;

  vec4 scaleToEncompassing;
  vec4 pad5;
  vec4 pad6;
} isosurfaceUniforms;

struct Light
{
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 spotDirection;
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

layout(std140, set = 0, binding = 3) uniform LightsUniformBlock
{
  Light lights[4];
} lightUniforms;

layout(std140, set = 0, binding = 4) uniform RibbonAODebugUniformBlock
{
  int mode;
  int textureWidth;
  int textureHeight;
  int patchNumber;
  float patchSize;
  float inverseTextureSize;
  int fastInteractionShading;
  int viewportWidth;
  int viewportHeight;
} ribbonAODebug;

layout(std140, set = 0, binding = 5) uniform GlobalAxesUniformBlock
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

bool clippedByUnitCellPlanes(vec4 objectPosition)
{
  return dot(structureUniforms.clipPlaneLeft, objectPosition) < 0.0
      || dot(structureUniforms.clipPlaneRight, objectPosition) < 0.0
      || dot(structureUniforms.clipPlaneTop, objectPosition) < 0.0
      || dot(structureUniforms.clipPlaneBottom, objectPosition) < 0.0
      || dot(structureUniforms.clipPlaneFront, objectPosition) < 0.0
      || dot(structureUniforms.clipPlaneBack, objectPosition) < 0.0;
}
