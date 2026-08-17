#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec2 texcoord;
layout(location = 0) out vec4 vFragColor;

float Sphere(vec2 p, float s)
{
  return length(p) - s;
}

float RoundedBox(vec2 p, vec2 b, float r)
{
  return length(max(abs(p) - b, 0.0)) - r;
}

float Rectangle(vec2 uv, vec2 pos, vec2 size)
{
  return (step(pos.x, uv.x) - step(pos.x + size.x, uv.x)) * (step(pos.y - size.y, uv.y) - step(pos.y, uv.y));
}

void main()
{
  float alpha = globalAxesUniforms.axesBackgroundColor.w;

  switch (globalAxesUniforms.axesBackGroundStyle)
  {
    case 0:
      alpha = 0.0;
    case 1:
      if (Sphere(texcoord - vec2(0.5, 0.5), 0.5) > 0.0)
        alpha = 0.0;
    case 2:
      break;
    case 3:
      if (RoundedBox(texcoord - vec2(0.5, 0.5), vec2(0.3, 0.3), 0.2) > 0.0)
        alpha = 0.0;
      break;
    case 4:
      if (max(-Sphere(texcoord - vec2(0.5, 0.5), 0.48), Sphere(texcoord - vec2(0.5, 0.5), 0.5)) > 0.0)
        alpha = 0.0;
      break;
    case 5:
      if (Rectangle(texcoord - vec2(0.5, 0.5), vec2(-0.48, 0.48), vec2(0.96, 0.96)) > 0.0)
        alpha = 0.0;
      break;
    case 6:
      if (max(-RoundedBox(texcoord - vec2(0.5, 0.5), vec2(0.30, 0.30), 0.17),
              RoundedBox(texcoord - vec2(0.5, 0.5), vec2(0.3, 0.3), 0.2)) > 0.0)
        alpha = 0.0;
    default:
      break;
  }

  vFragColor = vec4(globalAxesUniforms.axesBackgroundColor.xyz * alpha, alpha);
}
