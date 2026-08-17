#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "selection_imposter.glsl"

layout(location = 0) out vec4 vFragColor;

void main()
{
  vec3 N;
  vec3 modelN;
  if (!reconstructSelectionImposter(N, modelN))
  {
    discard;
  }

  vec3 L = normalize(inL);
  vec4 color = max(dot(N, L), 0.0) * vec4(1.0, 1.0, 0.0, 1.0);

  vec2 st = vec2(0.5 + 0.5 * atan(modelN.z, modelN.x) / 3.141592653589793, 0.5 - asin(modelN.y) / 3.141592653589793);
  float uDensity = structureUniforms.atomSelectionStripesDensity;
  float frequency = structureUniforms.atomSelectionStripesFrequency;
  if (fract(st.x * frequency) >= uDensity && fract(st.y * frequency) >= uDensity)
  {
    discard;
  }

  if (structureUniforms.atomHDR != 0)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.atomHDRExposure);
    vLdrColor.a = 1.0;
    color = vLdrColor;
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.atomHue;
  hsv.y = hsv.y * structureUniforms.atomSaturation;
  hsv.z = hsv.z * structureUniforms.atomValue;
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.atomSelectionIntensity;
  vFragColor = vec4(hsv2rgb(hsv) * bloomLevel, bloomLevel);
}
