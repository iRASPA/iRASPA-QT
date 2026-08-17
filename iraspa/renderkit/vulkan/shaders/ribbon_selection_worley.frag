#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "worley.glsl"

layout(location = 0) in vec3 inN;
layout(location = 1) in vec3 inL;
layout(location = 2) in vec3 inV;
layout(location = 3) in vec3 inModelN;
layout(location = 4) in vec3 inAmbient;
layout(location = 5) in vec3 inDiffuse;
layout(location = 6) in vec3 inSpecular;

layout(location = 0) out vec4 vFragColor;

void main()
{
  vec3 N = normalize(inN);
  vec3 L = normalize(inL);
  vec3 V = normalize(inV);
  vec3 R = reflect(-L, N);

  vec4 ambient = vec4(inAmbient, 1.0);
  vec4 diffuse = max(dot(N, L), 0.0) * vec4(inDiffuse, 1.0);
  vec4 specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.ribbonShininess) * vec4(inSpecular, 1.0);

  float frequency = structureUniforms.atomSelectionWorleyNoise3DFrequency;
  float jitter = structureUniforms.atomSelectionWorleyNoise3DJitter;
  vec2 F = cellular3D(frequency * vec3(inModelN.x, inModelN.z, inModelN.y), jitter);
  float n = F.y - F.x;

  vec4 color = n * (ambient + diffuse + specular);
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
