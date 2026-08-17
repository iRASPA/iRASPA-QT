#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 instancePosition;
layout(location = 1) in vec4 instanceScale;
layout(location = 2) in vec4 vertexPosition;
layout(location = 3) in vec4 instanceTexCoords;

layout(location = 0) out vec4 outEyePosition;
layout(location = 1) out vec2 outTexcoords;
layout(location = 2) out vec4 outSphereRadius;

void main()
{
  vec4 sphereRadius = structureUniforms.atomScaleFactor * instanceScale;
  vec4 c = structureUniforms.atomAnnotationTextScaling * vertexPosition;
  vec4 d = instanceTexCoords;

  vec4 pos = frameUniforms.viewMatrix * structureUniforms.modelMatrix * instancePosition;

  vec2 cornerOffset;
  vec2 texOffset;
  if (gl_VertexIndex == 0)
  {
    cornerOffset = vec2(c.x, -c.y);
    texOffset = vec2(d.x, d.y);
  }
  else if (gl_VertexIndex == 1)
  {
    cornerOffset = vec2(c.x, -c.y - c.w);
    texOffset = vec2(d.x, d.y + d.w);
  }
  else if (gl_VertexIndex == 2)
  {
    cornerOffset = vec2(c.x + c.z, -c.y);
    texOffset = vec2(d.x + d.z, d.y);
  }
  else
  {
    cornerOffset = vec2(c.x + c.z, -c.y - c.w);
    texOffset = vec2(d.x + d.z, d.y + d.w);
  }

  pos.xy += cornerOffset + structureUniforms.atomAnnotationTextDisplacement.xy;
  gl_Position = frameUniforms.projectionMatrix * vec4(pos.xyz, 1.0);
  outEyePosition = frameUniforms.viewMatrix * structureUniforms.modelMatrix * instancePosition;
  outTexcoords = texOffset;
  outSphereRadius = sphereRadius;
}
