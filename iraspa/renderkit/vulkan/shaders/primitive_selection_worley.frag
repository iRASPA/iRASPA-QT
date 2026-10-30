#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "worley.glsl"

layout(location = 0) in vec3 inN;
layout(location = 1) in vec3 inModelN;
layout(location = 2) in vec3 inL;
layout(location = 3) in vec3 inV;

layout(location = 0) out vec4 vFragColor;

void main()
{
  vec3 N = normalize(inN);
  vec3 L = normalize(inL);
  vec3 V = normalize(inV);
  vec3 R = reflect(-L, N);

  float frequency = structureUniforms.primitiveSelectionWorleyNoise3DFrequency;
  float jitter = structureUniforms.primitiveSelectionWorleyNoise3DJitter;
  vec2 F = cellular3D(frequency * inModelN, jitter);
  float n = F.y - F.x;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 color;
  if (gl_FrontFacing)
  {
    ambient = lightUniforms.lights[0].ambient * structureUniforms.primitiveAmbientFrontSide;
    diffuse = max(dot(N, L), 0.0) * lightUniforms.lights[0].diffuse * structureUniforms.primitiveDiffuseFrontSide;
    specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.primitiveShininessFrontSide) * lightUniforms.lights[0].specular * structureUniforms.primitiveSpecularFrontSide;
    color = vec4(n * (ambient.xyz + diffuse.xyz + specular.xyz), n);
    if (structureUniforms.primitiveFrontSideHDR != 0)
    {
      vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.primitiveFrontSideHDRExposure);
      vLdrColor.a = n;
      color = vLdrColor;
    }
  }
  else
  {
    ambient = lightUniforms.lights[0].ambient * structureUniforms.primitiveAmbientBackSide;
    diffuse = max(dot(-N, L), 0.0) * lightUniforms.lights[0].diffuse * structureUniforms.primitiveDiffuseBackSide;
    specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.primitiveShininessBackSide) * lightUniforms.lights[0].specular * structureUniforms.primitiveSpecularBackSide;
    color = vec4(n * (ambient.xyz + diffuse.xyz + specular.xyz), n);
    if (structureUniforms.primitiveBackSideHDR != 0)
    {
      vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.primitiveBackSideHDRExposure);
      vLdrColor.a = n;
      color = vLdrColor;
    }
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.primitiveHue;
  hsv.y = hsv.y * structureUniforms.primitiveSaturation;
  hsv.z = hsv.z * structureUniforms.primitiveValue;
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.primitiveSelectionIntensity;
  vFragColor = vec4(hsv2rgb(hsv) * bloomLevel * color.a, bloomLevel * color.a);
}
