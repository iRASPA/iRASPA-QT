#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec2 vertexST;
layout(location = 3) in vec2 vertexPad;

layout(location = 0) out vec3 outN;
layout(location = 1) out vec3 outL;
layout(location = 2) out vec3 outV;
layout(location = 3) out vec3 outAmbient;
layout(location = 4) out vec3 outDiffuse;
layout(location = 5) out vec3 outSpecular;
layout(location = 6) out vec2 outAoUV;
layout(location = 7) out vec3 outBaseColor;

vec3 ribbonColorForStructureType(float structureType)
{
  if (structureType < 0.5) { return structureUniforms.ribbonCoilColor.xyz; }
  if (structureType < 1.5) { return structureUniforms.ribbonHelixColor.xyz; }
  if (structureType < 2.5) { return structureUniforms.ribbonSheetColor.xyz; }
  if (structureType < 3.5) { return vec3(1.0, 1.0, 0.0); }
  if (structureType < 4.5) { return vec3(1.0, 1.0, 0.0); }
  if (structureType < 5.5) { return vec3(1.0, 0.0, 0.0); }
  if (structureType < 6.5) { return vec3(0.4, 0.4, 0.4); }
  if (structureType < 7.5) { return vec3(0.6, 1.0, 0.94); }
  return structureUniforms.ribbonCoilColor.xyz;
}

void main()
{
  vec4 pos = vertexPosition;
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
  vec3 localNormal = vertexNormal.xyz;
  outN = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vec4(localNormal, 0.0)).xyz;

  vec3 baseColor = ribbonColorForStructureType(vertexPad.x);
  outBaseColor = baseColor;
  outAmbient = (lightUniforms.lights[0].ambient * structureUniforms.ribbonAmbientColor * vec4(baseColor, 1.0)).xyz;
  outDiffuse = (lightUniforms.lights[0].diffuse * structureUniforms.ribbonDiffuseColor * vec4(baseColor, 1.0)).xyz;
  outSpecular = (lightUniforms.lights[0].specular * structureUniforms.ribbonSpecularColor).xyz;
  outAoUV = vertexST;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;
  outL = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
  outV = -P.xyz;
}
