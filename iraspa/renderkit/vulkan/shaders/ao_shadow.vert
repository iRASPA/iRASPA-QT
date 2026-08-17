#version 450
#extension GL_GOOGLE_include_directive : require
#include "ao_uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 instancePosition;
layout(location = 2) in vec4 instanceScale;

layout(location = 0) out float outSphereRadius;
layout(location = 1) out vec2 outTexcoord;
layout(location = 2) out vec4 outEyePosition;

void main()
{
  float sphereRadius = structureUniforms.atomScaleFactor * instanceScale.x;
  vec4 eyePosition = shadowUniforms.viewMatrix * structureUniforms.modelMatrix * instancePosition;
  outEyePosition = eyePosition;
  outSphereRadius = sphereRadius;
  outTexcoord = vertexPosition.xy;
  vec4 pos = eyePosition;
  pos.xy += sphereRadius * vertexPosition.xy;
  gl_Position = shadowUniforms.projectionMatrix * pos;
  // OpenGL NDC Z [-1,1] -> Vulkan clip Z [0,w] for rasterization only (fragment overrides depth).
  gl_Position.z = 0.5 * gl_Position.z + 0.5 * gl_Position.w;
}
