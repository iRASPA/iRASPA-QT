#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 instancePosition;
layout(location = 2) in int instanceTag;

layout(location = 0) flat out int outInstanceId;

void main()
{
  outInstanceId = instanceTag;
  vec4 pos = instancePosition + structureUniforms.transformationMatrix * vertexPosition;
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
}
