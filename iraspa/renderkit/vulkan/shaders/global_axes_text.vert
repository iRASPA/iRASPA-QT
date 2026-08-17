#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 instancePosition;
layout(location = 1) in vec4 instanceScale;
layout(location = 2) in vec4 vertexPosition;
layout(location = 3) in vec4 instanceTexCoords;
layout(location = 4) in vec4 instanceAxisId;

layout(location = 0) out vec4 outEyePosition;
layout(location = 1) out vec2 outTexcoords;
layout(location = 2) out vec4 outSphereRadius;
layout(location = 3) flat out int outInstanceID;

void main()
{
  int axisId = int(instanceAxisId.x + 0.5);
  vec4 sphereRadius = globalAxesUniforms.textScale[axisId] * instanceScale;
  vec4 c = globalAxesUniforms.textScale[axisId] * (instanceScale * vertexPosition);
  vec4 d = instanceTexCoords;

  vec4 textPosition = instancePosition;
  float scale = globalAxesUniforms.axesScale + 2.0 * globalAxesUniforms.centerScale + globalAxesUniforms.textOffset
                + globalAxesUniforms.textScale[axisId];
  textPosition.xyz *= scale;
  vec4 pos = frameUniforms.axesViewMatrix * textPosition;

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

  pos.xy += cornerOffset + globalAxesUniforms.textDisplacement[axisId].xy;
  gl_Position = frameUniforms.axesProjectionMatrix * vec4(pos.xyz, 1.0);
  outEyePosition = frameUniforms.axesViewMatrix * textPosition;
  outTexcoords = texOffset;
  outSphereRadius = sphereRadius;
  outInstanceID = axisId;
}
