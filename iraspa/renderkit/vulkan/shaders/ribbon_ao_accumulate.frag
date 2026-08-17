#version 450
#extension GL_GOOGLE_include_directive : require
#include "ao_uniforms.glsl"

layout(set = 1, binding = 0) uniform sampler2D shadowMapTexture;

layout(push_constant) uniform PushConstants
{
  float weight;
} push;

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in vec3 inWorldNormal;

layout(location = 0) out float vFragColor;

void main()
{
  vec4 shadowCoordinate = shadowUniforms.shadowMatrix * vec4(inWorldPosition, 1.0);
  vec4 shadowPos = shadowCoordinate / shadowCoordinate.w;
  vec4 viewNormal = shadowUniforms.viewMatrix * vec4(normalize(inWorldNormal), 0.0);
  float normalWeight = max(viewNormal.z, 0.0);
  if (normalWeight < 1.0e-4)
  {
    discard;
  }
  float mapDepth = texture(shadowMapTexture, shadowPos.xy).r;
  float visibility = step(shadowPos.z, mapDepth);
  vFragColor = push.weight * normalWeight * visibility;
}
