#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec3 inN;
layout(location = 1) in vec3 inL;
layout(location = 2) in vec3 inV;

layout(location = 0) out vec4 vFragColor;

void main()
{
  vec3 N = normalize(inN);
  vec3 L = normalize(inL);
  vec4 color = max(dot(N, L), 0.0) * structureUniforms.unitCellColor;
  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.atomHue;
  hsv.y = hsv.y * structureUniforms.atomSaturation;
  hsv.z = hsv.z * structureUniforms.atomValue;
  vFragColor = vec4(hsv2rgb(hsv), 1.0);
}
