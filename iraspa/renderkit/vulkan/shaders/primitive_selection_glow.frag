#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec3 inN;
layout(location = 1) in vec3 inModelN;
layout(location = 2) in vec3 inL;
layout(location = 3) in vec3 inV;

layout(location = 0) out vec4 unusedSceneColor;
layout(location = 1) out vec4 vFragColor;

void main()
{
  unusedSceneColor = vec4(0.0);
  vec3 N = normalize(inN);
  vec3 L = normalize(inL);
  vec3 V = normalize(inV);
  vec3 R = reflect(-L, N);

  vec4 ambient = structureUniforms.primitiveAmbientFrontSide;
  vec4 diffuse = max(dot(N, L), 0.0) * structureUniforms.primitiveDiffuseFrontSide;
  vec4 specular = pow(max(dot(R, V), 0.0), structureUniforms.primitiveShininessFrontSide) * structureUniforms.primitiveSpecularFrontSide;
  vec4 color = vec4(ambient.xyz + diffuse.xyz + specular.xyz, 1.0);
  if (structureUniforms.primitiveFrontSideHDR != 0)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.primitiveFrontSideHDRExposure);
    vLdrColor.a = 1.0;
    color = vLdrColor;
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.primitiveHue;
  hsv.y = hsv.y * structureUniforms.primitiveSaturation;
  hsv.z = hsv.z * structureUniforms.primitiveValue;
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.primitiveSelectionIntensity;
  vFragColor = bloomLevel * vec4(hsv2rgb(hsv), 1.0);
}
