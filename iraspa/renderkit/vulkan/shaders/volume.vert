#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;

layout(location = 0) out vec3 outUV;
layout(location = 1) out vec3 outPosition;

void main()
{
  vec4 pos = structureUniforms.modelMatrix * structureUniforms.boxMatrix * vertexPosition;
  outPosition = pos.xyz;
  outUV = vertexPosition.xyz;
  gl_Position = frameUniforms.mvpMatrix * pos;
}
