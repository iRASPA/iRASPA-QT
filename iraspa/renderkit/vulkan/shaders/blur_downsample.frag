#version 450

layout(location = 0) in vec2 texcoord;
layout(location = 0) out vec4 vFragColor;

layout(set = 0, binding = 0) uniform sampler2DMS originalTexture;

void main()
{
  vec2 tmp = floor(vec2(textureSize(originalTexture)) * texcoord);
  vFragColor = texelFetch(originalTexture, ivec2(tmp), 0);
}
