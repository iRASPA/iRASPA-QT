#version 450
#extension GL_GOOGLE_include_directive : require
#include "ao_uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 instancePosition;
layout(location = 2) in vec4 instanceScale;

layout(location = 0) out vec4 outAtomCenterPosition;
layout(location = 1) out vec2 outTexcoords;
layout(location = 2) out vec4 outSphereRadius;

void main()
{
  outAtomCenterPosition = structureUniforms.modelMatrix * instancePosition;
  outSphereRadius = structureUniforms.atomScaleFactor * instanceScale;

  int patchNumber = max(structureUniforms.ambientOcclusionPatchNumber, 1);
  float patchSize = structureUniforms.ambientOcclusionPatchSize;
  int iid = gl_InstanceIndex;
  float k1 = iid % patchNumber;
  float k2 = iid / patchNumber;

  vec2 offset = vec2(patchSize, patchSize) * vec2(k1, k2) * structureUniforms.ambientOcclusionInverseTextureSize;
  vec2 origin = offset * 2.0 - 1.0;
  float tmp = 2.0 * patchSize * structureUniforms.ambientOcclusionInverseTextureSize;

  outTexcoords = vertexPosition.xy;
  gl_Position = vec4(origin + tmp * (vertexPosition.xy * 0.5 + vec2(0.5)), 0.0, 1.0);
}
