#version 450
#extension GL_GOOGLE_include_directive : require
#include "ao_uniforms.glsl"

layout(location = 0) in vec4 inEyePosition;

void main()
{
  vec4 pos = shadowUniforms.projectionMatrix * inEyePosition;
  gl_FragDepth = 0.5 * (pos.z / pos.w) + 0.5;
}
