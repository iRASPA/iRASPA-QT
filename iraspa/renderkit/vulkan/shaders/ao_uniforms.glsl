layout(std140, set = 0, binding = 0) uniform StructureUniformBlock
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
} structureUniforms;

layout(std140, set = 0, binding = 1) uniform ShadowUniformBlock
{
  mat4 projectionMatrix;
  mat4 viewMatrix;
  mat4 shadowMatrix;
  mat4 normalMatrix;
} shadowUniforms;
