#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec3 inN;
layout(location = 1) in vec3 inL;
layout(location = 2) in vec3 inV;

layout(location = 0) out vec4 vFragColor;

void main()
{
  vec3 N = normalize(inN);
  vec3 L = normalize(inL);
  vec3 V = normalize(inV);

  vec4 ambient;
  vec4 specular;
  vec4 diffuse;
  vec4 color;

  if (gl_FrontFacing)
  {
    vec3 R = reflect(-L, N);
    ambient = lightUniforms.lights[0].ambient * isosurfaceUniforms.ambientFrontSide;
    diffuse = max(dot(N, L), 0.0) * lightUniforms.lights[0].diffuse * isosurfaceUniforms.diffuseFrontSide;
    specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + isosurfaceUniforms.shininessFrontSide) * lightUniforms.lights[0].specular * isosurfaceUniforms.specularFrontSide;
    color = vec4(ambient.xyz + diffuse.xyz + specular.xyz, 1.0);
    if (isosurfaceUniforms.frontHDR != 0)
    {
      vec4 vLdrColor = 1.0 - exp2(-color * isosurfaceUniforms.frontHDRExposure);
      vLdrColor.a = 1.0;
      color = vLdrColor;
    }
  }
  else
  {
    vec3 R = reflect(-L, -N);
    ambient = lightUniforms.lights[0].ambient * isosurfaceUniforms.ambientBackSide;
    diffuse = max(dot(-N, L), 0.0) * lightUniforms.lights[0].diffuse * isosurfaceUniforms.diffuseBackSide;
    specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + isosurfaceUniforms.shininessBackSide) * lightUniforms.lights[0].specular * isosurfaceUniforms.specularBackSide;
    color = vec4(ambient.xyz + diffuse.xyz + specular.xyz, 1.0);
    if (isosurfaceUniforms.backHDR != 0)
    {
      vec4 vLdrColor = 1.0 - exp2(-color * isosurfaceUniforms.backHDRExposure);
      vLdrColor.a = 1.0;
      color = vLdrColor;
    }
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * isosurfaceUniforms.hue;
  hsv.y = hsv.y * isosurfaceUniforms.saturation;
  hsv.z = hsv.z * isosurfaceUniforms.value;
  float opacity = gl_FrontFacing ? isosurfaceUniforms.diffuseFrontSide.w : isosurfaceUniforms.diffuseBackSide.w;
  vFragColor = vec4(hsv2rgb(hsv) * opacity, opacity);
}
