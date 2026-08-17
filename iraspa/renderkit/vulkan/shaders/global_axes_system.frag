#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec3 inN;
layout(location = 1) in vec3 inL;
layout(location = 2) in vec4 inDiffuse;

layout(location = 0) out vec4 vFragColor;

void main()
{
  vec3 N = normalize(inN);
  vec3 L = normalize(inL);
  vec4 color = max(dot(N, L), 0.0) * inDiffuse;
  vec4 vLdrColor = 1.0 - exp2(-color * 1.5);
  vLdrColor.a = 1.0;
  vFragColor = vec4(vLdrColor.xyz, 1.0);
}
