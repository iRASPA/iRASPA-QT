#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(set = 1, binding = 0) uniform sampler2D ambientOcclusionTexture;

layout(location = 0) in vec4 inEyePosition;
layout(location = 1) in vec2 inTexcoords;
layout(location = 2) flat in vec4 inInstancePosition;
layout(location = 3) flat in vec4 inAmbient;
layout(location = 4) flat in vec4 inDiffuse;
layout(location = 5) flat in vec4 inSpecular;
layout(location = 6) in vec3 inFragPos;
layout(location = 7) flat in vec3 inFragCenter;
layout(location = 8) in vec3 inL;
layout(location = 9) in vec3 inV;
layout(location = 10) flat in vec4 inSphereRadius;
layout(location = 11) flat in float inK1;
layout(location = 12) flat in float inK2;

layout(location = 0) out vec4 vFragColor;

vec2 textureCoordinateForSphereSurfacePosition(vec3 sphereSurfacePosition)
{
  vec3 absoluteSphereSurfacePosition = abs(sphereSurfacePosition);
  float d = absoluteSphereSurfacePosition.x + absoluteSphereSurfacePosition.y + absoluteSphereSurfacePosition.z;
  return (sphereSurfacePosition.z > 0.0) ? sphereSurfacePosition.xy / d
                                         : sign(sphereSurfacePosition.xy) * (1.0 - absoluteSphereSurfacePosition.yx / d);
}

void main()
{
  if (inInstancePosition.w < 0.0)
  {
    discard;
  }

  float x = inTexcoords.x;
  float y = inTexcoords.y;
  float zz = 1.0 - x * x - y * y;
  if (zz <= 0.0)
  {
    discard;
  }

  float z = sqrt(zz);
  vec3 N = vec3(x, y, z);
  mat4 aoTransform = transpose(frameUniforms.normalMatrix * structureUniforms.modelMatrix);

  if (structureUniforms.clipAtomsAtUnitCell != 0)
  {
    vec3 vertexPosition = (aoTransform * (inSphereRadius * vec4(x, y, z, 1.0))).xyz;
    vec4 position = vec4(inInstancePosition.xyz + vertexPosition, 1.0);
    if (clippedByUnitCellPlanes(position))
    {
      discard;
    }
  }

  vec4 pos = inEyePosition;
  pos.z += inSphereRadius.z * z;
  pos = frameUniforms.projectionMatrix * pos;
  gl_FragDepth = pos.z / pos.w;

  vec3 L = normalize(inL);
  vec3 V = normalize(inV);
  vec3 R = reflect(-L, N);

  vec3 ambient = inAmbient.xyz;
  vec3 diffuse = max(dot(N, L), 0.0) * inDiffuse.xyz;
  vec3 specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.atomShininess) * inSpecular.xyz;

  float ao = 1.0;
  if (structureUniforms.ambientOcclusion != 0)
  {
    float patchSize = structureUniforms.ambientOcclusionPatchSize;
    vec3 t1 = (aoTransform * vec4(N, 1.0)).xyz;
    vec2 m2 = (vec2(patchSize * (inK1 + 0.5), patchSize * (inK2 + 0.5)) + 0.5 * (patchSize - 1.0) * textureCoordinateForSphereSurfacePosition(t1))
              * structureUniforms.ambientOcclusionInverseTextureSize;
    ao = texture(ambientOcclusionTexture, m2).r;
  }

  vec4 color = vec4(ao * (ambient + diffuse + specular), 1.0);
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
  vFragColor = vec4(hsv2rgb(hsv), 1.0);
}
