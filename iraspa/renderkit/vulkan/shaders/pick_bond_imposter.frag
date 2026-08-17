#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "bond_imposter.glsl"

layout(location = 0) in vec3 inFragPos;
layout(location = 1) flat in vec3 inPointA;
layout(location = 2) flat in vec3 inPointB;
layout(location = 3) flat in float inRadius;
layout(location = 4) flat in int inInstanceId;

layout(location = 0) out uvec4 vFragColor;

void main()
{
  bool orthographic = (frameUniforms.projectionMatrix[3][3] > 0.5);
  vec3 ro = orthographic ? vec3(inFragPos.xy, 0.0) : vec3(0.0);
  vec3 rd = orthographic ? vec3(0.0, 0.0, -1.0) : normalize(inFragPos);

  vec3 N;
  float ct;
  float t = bondImposterIntersect(ro, rd, inPointA, inPointB, inRadius, N, ct);
  if (t < 0.0)
  {
    discard;
  }

  vec3 pos = ro + t * rd;
  vec4 screenPos = frameUniforms.projectionMatrix * vec4(pos, 1.0);
  gl_FragDepth = screenPos.z / screenPos.w;

  vFragColor = uvec4(2u, uint(structureUniforms.sceneIdentifier), uint(structureUniforms.MovieIdentifier), uint(inInstanceId));
}
