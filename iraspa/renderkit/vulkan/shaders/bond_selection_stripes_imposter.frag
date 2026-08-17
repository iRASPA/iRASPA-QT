#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "bond_imposter.glsl"

layout(location = 0) flat in vec4 inColor1;
layout(location = 1) flat in vec4 inColor2;
layout(location = 2) flat in vec2 inMixParam;
layout(location = 3) in vec3 inFragPos;
layout(location = 4) flat in vec3 inPointA;
layout(location = 5) flat in vec3 inPointB;
layout(location = 6) flat in float inRadius;
layout(location = 7) flat in vec3 inAxisX;
layout(location = 8) flat in vec3 inAxisZ;

layout(location = 0) out vec4 vFragColor;

void main()
{
  bool orthographic = (frameUniforms.projectionMatrix[3][3] > 0.5);
  vec3 ro = orthographic ? vec3(inFragPos.xy, 0.0) : vec3(0.0);
  vec3 rd = orthographic ? vec3(0.0, 0.0, -1.0) : normalize(inFragPos);

  vec3 N;
  float ct;
  mat4 toStructure = structureUniforms.inverseModelMatrix * frameUniforms.viewMatrixInverse;
  float t = (structureUniforms.clipBondsAtUnitCell != 0)
                ? bondImposterClippedIntersect(ro, rd, inPointA, inPointB, inRadius, toStructure, N, ct)
                : bondImposterIntersect(ro, rd, inPointA, inPointB, inRadius, N, ct);
  if (t < 0.0)
  {
    discard;
  }

  vec3 pos = ro + t * rd;
  vec4 screenPos = frameUniforms.projectionMatrix * vec4(pos, 1.0);
  gl_FragDepth = screenPos.z / screenPos.w;

  vec3 axisPos = mix(inPointA, inPointB, ct);
  vec3 pr = (pos - axisPos) / inRadius;
  vec3 t1 = vec3(dot(pr, inAxisX), ct, dot(pr, inAxisZ));
  vec2 st = vec2(0.5 + 0.5 * atan(t1.x, t1.z) / 3.141592653589793, t1.y);
  float uDensity = structureUniforms.bondSelectionStripesDensity;
  float frequency = structureUniforms.bondSelectionStripesFrequency;
  if (fract(st.x * frequency) >= uDensity && fract(st.y * frequency) >= uDensity)
  {
    discard;
  }

  vec3 L = normalize((lightUniforms.lights[0].position - vec4(pos, 1.0) * lightUniforms.lights[0].position.w).xyz);
  vec4 color = max(dot(N, L), 0.0) * vec4(1.0, 1.0, 0.0, 1.0);
  if (structureUniforms.bondHDR != 0)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.bondHDRExposure);
    color = vLdrColor;
  }

  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.bondSelectionIntensity;
  vFragColor = vec4(color.xyz * bloomLevel, bloomLevel);
}
