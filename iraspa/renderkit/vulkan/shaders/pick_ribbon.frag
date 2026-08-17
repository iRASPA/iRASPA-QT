#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) flat in int inSegmentIndex;
layout(location = 1) flat in int inResidueIndex;
layout(location = 0) out uvec4 vFragColor;

void main()
{
  vFragColor = uvec4(3u, uint(structureUniforms.structureIdentifier), uint(inSegmentIndex), uint(inResidueIndex));
}
