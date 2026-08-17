#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) flat in int inInstanceId;
layout(location = 0) out uvec4 vFragColor;

void main()
{
  vFragColor = uvec4(1u, uint(structureUniforms.sceneIdentifier), uint(structureUniforms.MovieIdentifier), uint(inInstanceId));
}
