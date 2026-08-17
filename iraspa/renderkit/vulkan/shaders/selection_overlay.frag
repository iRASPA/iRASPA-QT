#version 450

layout(location = 0) in vec2 texcoord;
layout(location = 0) out vec4 vFragColor;

layout(push_constant) uniform Overlay
{
  vec4 rect;
  float dashed;
  float enabled;
  float pixelScale;
  float pad;
} overlay;

void main()
{
  if (overlay.enabled < 0.5)
  {
    discard;
  }

  vec2 p = gl_FragCoord.xy;
  vec2 mn = min(overlay.rect.xy, overlay.rect.zw);
  vec2 mx = max(overlay.rect.xy, overlay.rect.zw);
  if (p.x < mn.x || p.x > mx.x || p.y < mn.y || p.y > mx.y)
  {
    discard;
  }

  float scale = max(overlay.pixelScale, 1.0);
  float border = 2.0 * scale;
  bool onBorder = p.x < mn.x + border || p.x > mx.x - border || p.y < mn.y + border || p.y > mx.y - border;
  if (onBorder)
  {
    if (overlay.dashed > 0.5)
    {
      float t = (p.x < mn.x + border || p.x > mx.x - border) ? p.y : p.x;
      float period = 9.0 * scale;
      if (mod(t, period) >= 6.0 * scale)
      {
        discard;
      }
    }
    vFragColor = vec4(0.25, 0.25, 0.25, 1.0);
    return;
  }

  vFragColor = vec4(0.827, 0.827, 0.827, 100.0 / 255.0);
}
