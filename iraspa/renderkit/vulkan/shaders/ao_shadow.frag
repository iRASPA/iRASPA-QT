#version 450
#extension GL_GOOGLE_include_directive : require
#include "ao_uniforms.glsl"

layout(location = 0) in float inSphereRadius;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec4 inEyePosition;

void main()
{
  float x = inTexcoord.x;
  float y = inTexcoord.y;
  float zz = 1.0 - x * x - y * y;
  if (zz <= 0.0)
  {
    discard;
  }

  vec4 pos = shadowUniforms.projectionMatrix * inEyePosition;
  // Same as OpenGL gl_FragDepth: OpenGL NDC Z -> [0,1]. Must match RKShadowUniforms
  // ViewToOpenGLDepthTextureMatrix (0.5*z+0.5) used by textureProj — do not omit or
  // also bake this convert into projectionMatrix (that would double-apply).
  gl_FragDepth = 0.5 * (pos.z / pos.w) + 0.5;
}
