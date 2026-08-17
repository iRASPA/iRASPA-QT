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

layout(location = 0) out vec4 unusedSceneColor;
layout(location = 1) out vec4 vFragColor;

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

  vec3 L = normalize((lightUniforms.lights[0].position - vec4(pos, 1.0) * lightUniforms.lights[0].position.w).xyz);
  vec3 V = normalize(-pos);
  vec3 R = reflect(-L, N);

  vec4 ambient = lightUniforms.lights[0].ambient * structureUniforms.bondAmbientColor;
  vec4 specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.bondShininess)
                  * lightUniforms.lights[0].specular * structureUniforms.bondSpecularColor;
  vec4 diffuse = vec4(max(dot(N, L), 0.0));
  float tmix = clamp((ct - inMixParam.x) / max(inMixParam.y - inMixParam.x, 1.0e-6), 0.0, 1.0);
  if (structureUniforms.bondColorMode == 0)
  {
    diffuse *= lightUniforms.lights[0].diffuse * structureUniforms.bondDiffuseColor;
  }
  else if (structureUniforms.bondColorMode == 1)
  {
    diffuse *= (tmix < 0.5 ? inColor1 : inColor2);
  }
  else
  {
    diffuse *= mix(inColor1, inColor2, smoothstep(0.0, 1.0, tmix));
  }

  vec4 color = vec4(ambient.xyz + diffuse.xyz + specular.xyz, 1.0);
  if (structureUniforms.bondHDR != 0)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.bondHDRExposure);
    vLdrColor.a = 1.0;
    color = vLdrColor;
  }

  unusedSceneColor = vec4(0.0);
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.bondSelectionIntensity;
  vFragColor = vec4(color.xyz * bloomLevel, bloomLevel);
}
