#version 450

layout(set = 0, binding = 0) uniform sampler2D backgroundTexture;

layout(location = 0) in vec2 texcoord;
layout(location = 0) out vec4 vFragColor;

void main()
{
  vFragColor = texture(backgroundTexture, texcoord);
}
