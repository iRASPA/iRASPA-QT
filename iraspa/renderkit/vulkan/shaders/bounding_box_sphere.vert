#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec4 instancePosition;
layout(location = 3) in vec4 instanceScale;

layout(location = 0) out vec3 outN;
layout(location = 1) out vec3 outL;

void main()
{
  vec4 pos = instancePosition + instanceScale * vertexPosition;
  outN = (frameUniforms.normalMatrix * vertexNormal).xyz;
  vec4 P = frameUniforms.viewMatrix * pos;
  outL = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
  gl_Position = frameUniforms.mvpMatrix * pos;
}
