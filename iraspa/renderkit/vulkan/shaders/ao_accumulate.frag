#version 450
#extension GL_GOOGLE_include_directive : require
#include "ao_uniforms.glsl"

layout(location = 0) in vec4 inAtomCenterPosition;
layout(location = 1) in vec2 inTexcoords;
layout(location = 2) in vec4 inSphereRadius;

layout(set = 1, binding = 0) uniform sampler2DShadow shadowMapTexture;

layout(push_constant) uniform PushConstants
{
  float weight;
} push;

layout(location = 0) out float vFragColor;

vec3 coordinateFromTexturePosition(vec2 texturePosition)
{
  vec2 absoluteTexturePosition = abs(texturePosition);
  float h = 1.0 - absoluteTexturePosition.s - absoluteTexturePosition.t;
  if (h >= 0.0)
  {
    return vec3(texturePosition.s, texturePosition.t, h);
  }
  return vec3(sign(texturePosition.s) * (1.0 - absoluteTexturePosition.t),
              sign(texturePosition.t) * (1.0 - absoluteTexturePosition.s), h);
}

void main()
{
  float patchSize = structureUniforms.ambientOcclusionPatchSize;
  int p = int(patchSize);
  ivec2 fc = ivec2(gl_FragCoord.xy);
  // Do NOT flip FragCoord Y. Vulkan gl_FragCoord Y increases downward (top-left origin), so
  // high V within the patch is the bottom of the patch on screen — and localY→octahedral
  // then matches OpenGL's high-FragCoord→+Y and the ModelN sampling UV math.
  ivec2 impostorSpaceCoordinate = ivec2(fc.x % p, fc.y % p);
  vec2 newImpostorSpaceCoordinate = (2.0 * vec2(impostorSpaceCoordinate) / (patchSize - 1.0) - 1.0);

  vec3 imposterXYZ = normalize(coordinateFromTexturePosition(newImpostorSpaceCoordinate));
  vec3 currentSphereSurfaceCoordinate = inSphereRadius.xyz * imposterXYZ;
  vec3 pos = currentSphereSurfaceCoordinate + inAtomCenterPosition.xyz;

  vec4 shadowCoordinate = shadowUniforms.shadowMatrix * vec4(pos, 1.0);
  vec4 normal = shadowUniforms.normalMatrix * vec4(imposterXYZ, 1.0);
  if (normal.z < 0.0)
  {
    discard;
  }

  // Do not flip shadow V. Direct3D/Metal need `1-V` because their NDC is Y-up while
  // the texture origin is top-left. Vulkan NDC is already Y-down, so rasterization
  // and UV origin cancel and OpenGL's shadowMatrix bias matches as-is.
  vFragColor = push.weight * normal.z * textureProj(shadowMapTexture, shadowCoordinate);
}
