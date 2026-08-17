#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec4 vertexColor;

layout(location = 0) out vec3 outN;
layout(location = 1) out vec3 outL;
layout(location = 2) out vec4 outDiffuse;

void main()
{
  vec4 scale = vec4(globalAxesUniforms.axesScale, globalAxesUniforms.axesScale, globalAxesUniforms.axesScale, 1.0);
  vec4 pos = scale * vertexPosition + vec4(0.0, 0.0, 0.0, 1.0);
  outN = (frameUniforms.normalMatrix * vertexNormal).xyz;
  outDiffuse = vertexColor;
  vec4 P = frameUniforms.axesViewMatrix * pos;
  outL = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
  gl_Position = frameUniforms.axesMvpMatrix * pos;
}
