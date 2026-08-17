#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec3 inN;
layout(location = 1) in vec3 inModelN;
layout(location = 2) in vec3 inL;
layout(location = 3) in vec3 inV;

layout(location = 0) out vec4 vFragColor;

void main()
{
  vec3 N = normalize(inN);
  vec3 L = normalize(inL);
  vec4 color = max(dot(N, L), 0.0) * vec4(1.0, 1.0, 0.0, 1.0);

  vec3 t1 = inModelN;
  vec2 st = vec2(0.5 + 0.5 * atan(t1.z, t1.x) / 3.141592653589793, 0.5 - asin(t1.y) / 3.141592653589793);
  float uDensity = structureUniforms.primitiveSelectionStripesDensity;
  float frequency = structureUniforms.primitiveSelectionStripesFrequency;
  if (fract(st.x * frequency) >= uDensity && fract(st.y * frequency) >= uDensity)
  {
    discard;
  }

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
