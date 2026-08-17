#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "worley.glsl"
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
  vec3 V = normalize(inV);
  vec3 R = reflect(-L, N);

  vec4 ambient = inAmbient;
  vec4 diffuse = max(dot(N, L), 0.0) * inDiffuse;
  vec4 specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.atomShininess) * inSpecular;

  float frequency = structureUniforms.atomSelectionWorleyNoise3DFrequency;
  float jitter = structureUniforms.atomSelectionWorleyNoise3DJitter;
  vec2 F = cellular3D(frequency * modelN, jitter);
  float n = F.y - F.x;

  vec4 color = n * (ambient + diffuse + specular);
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
