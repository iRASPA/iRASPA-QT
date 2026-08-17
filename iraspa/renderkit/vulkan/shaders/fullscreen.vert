#version 450

layout(location = 0) in vec4 position;
layout(location = 0) out vec2 texcoord;

void main()
{
  gl_Position = position;
  texcoord = position.xy * vec2(0.5) + vec2(0.5);
}
