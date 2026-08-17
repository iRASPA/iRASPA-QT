#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "worley.glsl"
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

  vec3 L = normalize((lightUniforms.lights[0].position - vec4(pos, 1.0) * lightUniforms.lights[0].position.w).xyz);
  vec3 V = normalize(-pos);
  vec3 R = reflect(-L, N);

  vec4 ambient = lightUniforms.lights[0].ambient * structureUniforms.bondAmbientColor;
  vec4 diffuse = max(dot(N, L), 0.0) * lightUniforms.lights[0].diffuse * structureUniforms.bondDiffuseColor;
  vec4 specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.bondShininess)
                  * lightUniforms.lights[0].specular * structureUniforms.bondSpecularColor;

  float frequency = structureUniforms.bondSelectionWorleyNoise3DFrequency;
  float jitter = structureUniforms.bondSelectionWorleyNoise3DJitter;
  vec2 F = cellular3D(frequency * vec3(t1.x, 2.0 * t1.y, t1.z), jitter);
  float n = F.y - F.x;

  vec4 color = n * (ambient + diffuse + specular);
  if (structureUniforms.bondHDR != 0)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.bondHDRExposure);
    vLdrColor.a = 1.0;
    color = vLdrColor;
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.bondHue;
  hsv.y = hsv.y * structureUniforms.bondSaturation;
  hsv.z = hsv.z * structureUniforms.bondValue;
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.bondSelectionIntensity;
  vFragColor = vec4(hsv2rgb(hsv) * bloomLevel, bloomLevel);
}
