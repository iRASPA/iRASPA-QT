#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "ribbon_selection.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec2 vertexST;
layout(location = 3) in vec2 vertexPad;
layout(location = 4) in vec2 vertexStripeST;

layout(location = 0) out vec3 outN;
layout(location = 1) out vec3 outL;
layout(location = 2) out vec2 outStripeST;

void main()
{
  vec4 pos = vec4(ribbonSelectionExpandedPosition(vertexPosition, vertexNormal, 0.45), 1.0);
  vec3 localNormal = normalize(vertexNormal.xyz);

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
  outN = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vec4(localNormal, 0.0)).xyz;
  outStripeST = vertexStripeST;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;
  outL = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
}
