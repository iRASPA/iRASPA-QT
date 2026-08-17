#version 450

layout(set = 0, binding = 0) uniform sampler2D s_texture;

layout(location = 0) in vec2 v_texCoord;
layout(location = 1) in vec2 v_blurTexCoords[14];
layout(location = 0) out vec4 vFragColor;

void main()
{
  vFragColor = vec4(0.0);
  vFragColor += texture(s_texture, v_blurTexCoords[0]) * 0.0044299121055113265;
  vFragColor += texture(s_texture, v_blurTexCoords[1]) * 0.00895781211794;
  vFragColor += texture(s_texture, v_blurTexCoords[2]) * 0.0215963866053;
  vFragColor += texture(s_texture, v_blurTexCoords[3]) * 0.0443683338718;
  vFragColor += texture(s_texture, v_blurTexCoords[4]) * 0.0776744219933;
  vFragColor += texture(s_texture, v_blurTexCoords[5]) * 0.115876621105;
  vFragColor += texture(s_texture, v_blurTexCoords[6]) * 0.147308056121;
  vFragColor += texture(s_texture, v_texCoord) * 0.159576912161;
  vFragColor += texture(s_texture, v_blurTexCoords[7]) * 0.147308056121;
  vFragColor += texture(s_texture, v_blurTexCoords[8]) * 0.115876621105;
  vFragColor += texture(s_texture, v_blurTexCoords[9]) * 0.0776744219933;
  vFragColor += texture(s_texture, v_blurTexCoords[10]) * 0.0443683338718;
  vFragColor += texture(s_texture, v_blurTexCoords[11]) * 0.0215963866053;
  vFragColor += texture(s_texture, v_blurTexCoords[12]) * 0.00895781211794;
  vFragColor += texture(s_texture, v_blurTexCoords[13]) * 0.0044299121055113265;
}
