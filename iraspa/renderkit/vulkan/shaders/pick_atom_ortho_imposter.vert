#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 instancePosition;
layout(location = 2) in vec4 instanceScale;
layout(location = 3) in int instanceTag;

layout(location = 0) out vec4 outEyePosition;
layout(location = 1) out vec2 outTexcoords;
layout(location = 2) flat out vec4 outInstancePosition;
layout(location = 3) out vec3 outFragPos;
layout(location = 4) flat out vec3 outFragCenter;
layout(location = 5) flat out vec4 outSphereRadius;
layout(location = 6) flat out int outInstanceId;
layout(location = 7) flat out float outIsPerspective;

void main()
{
  outInstanceId = instanceTag;
  outInstancePosition = instancePosition;
  outTexcoords = vertexPosition.xy;
  outSphereRadius = structureUniforms.atomScaleFactor * instanceScale;
  outIsPerspective = 0.0;
  outEyePosition = frameUniforms.viewMatrix * structureUniforms.modelMatrix * instancePosition;
  outFragCenter = outEyePosition.xyz;

  if (instancePosition.w < 0.0)
  {
    outFragPos = vec3(0.0);
    gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
    return;
  }

  vec4 pos2 = outEyePosition;
  pos2.xy += outSphereRadius.xy * vertexPosition.xy;
  outFragPos = pos2.xyz;
  gl_Position = frameUniforms.projectionMatrix * pos2;
}
