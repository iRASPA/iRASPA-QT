#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec2 vertexST;
layout(location = 3) in vec2 vertexPad;

layout(location = 0) flat out int outSegmentIndex;
layout(location = 1) flat out int outResidueIndex;

void main()
{
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * vertexPosition;
  outSegmentIndex = int(vertexNormal.w);
  outResidueIndex = int(vertexPad.y);
}
