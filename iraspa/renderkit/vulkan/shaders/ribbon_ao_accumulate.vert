#version 450
#extension GL_GOOGLE_include_directive : require
#include "ao_uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec2 vertexST;

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec3 outWorldNormal;

void main()
{
  vec2 clipPos = vertexST * 2.0 - 1.0;
  gl_Position = vec4(clipPos.x, -clipPos.y, 0.0, 1.0);
  outWorldPosition = (structureUniforms.modelMatrix * vertexPosition).xyz;
  outWorldNormal = normalize(mat3(structureUniforms.modelMatrix) * vertexNormal.xyz);
}
