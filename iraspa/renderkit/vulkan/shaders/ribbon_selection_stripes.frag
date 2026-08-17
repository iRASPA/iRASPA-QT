#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec3 inN;
layout(location = 1) in vec3 inL;
layout(location = 2) in vec2 inStripeST;

layout(location = 0) out vec4 vFragColor;

void main()
{
  vec3 N = normalize(inN);
  vec3 L = normalize(inL);
  vec4 color = max(dot(N, L), 0.0) * vec4(1.0, 1.0, 0.0, 1.0);

  vec2 stripeST = inStripeST;
  float uDensity = structureUniforms.atomSelectionStripesDensity;
  float frequency = structureUniforms.atomSelectionStripesFrequency;

  float bandAlong = smoothstep(0.0, 0.06, stripeST.x) * smoothstep(0.0, 0.06, 1.0 - stripeST.x);
  float bandAround = smoothstep(0.0, 0.10, stripeST.y) * smoothstep(0.0, 0.10, 1.0 - stripeST.y);
  float bandMask = bandAlong * bandAround;
  if (bandMask < 0.01)
  {
    discard;
  }

  float stripeU = fract(stripeST.x * frequency);
  float stripeV = fract(stripeST.y * frequency);
  bool inStripe = (stripeU < uDensity) != (stripeV < uDensity);
  if (!inStripe)
  {
    discard;
  }

  color *= bandMask;

  if (structureUniforms.ribbonHDR != 0)
  {
    color = 1.0 - exp2(-color * structureUniforms.ribbonHDRExposure);
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.ribbonHue;
  hsv.y = hsv.y * structureUniforms.ribbonSaturation;
  hsv.z = hsv.z * structureUniforms.ribbonValue;
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.atomSelectionIntensity;
  vFragColor = vec4(hsv2rgb(hsv) * bloomLevel, bloomLevel);
}
