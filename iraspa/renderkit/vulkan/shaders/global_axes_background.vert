#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 0) out vec2 texcoord;

void main()
{
  gl_Position = vec4(vertexPosition.x, -vertexPosition.y, vertexPosition.z, vertexPosition.w);
  texcoord = vertexPosition.xy * vec2(0.5) + vec2(0.5);
}
