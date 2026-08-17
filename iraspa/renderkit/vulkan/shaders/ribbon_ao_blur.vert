#version 450

layout(location = 0) in vec4 position;
layout(location = 0) out vec2 texCoord;

void main()
{
  gl_Position = position;
  texCoord = position.xy * vec2(0.5) + vec2(0.5);
}
