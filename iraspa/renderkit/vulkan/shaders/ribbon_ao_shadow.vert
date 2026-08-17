#version 450
#extension GL_GOOGLE_include_directive : require
#include "ao_uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;

layout(location = 0) out vec4 outEyePosition;

void main()
{
  outEyePosition = shadowUniforms.viewMatrix * structureUniforms.modelMatrix * vertexPosition;
  gl_Position = shadowUniforms.projectionMatrix * outEyePosition;
  gl_Position.z = 0.5 * gl_Position.z + 0.5 * gl_Position.w;
}
