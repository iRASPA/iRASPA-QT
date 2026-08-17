#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec2 texcoord;
layout(location = 0) out vec4 vFragColor;

layout(set = 1, binding = 0) uniform sampler2DMS originalTexture;
layout(set = 1, binding = 1) uniform sampler2D blurredTexture;

void main()
{
  vec2 tmp = floor(vec2(textureSize(originalTexture)) * texcoord);
  vec4 vColor = vec4(0.0);
  int sampleCount = max(int(frameUniforms.numberOfMultiSamplePoints), 1);
  for (int i = 0; i < 8; ++i)
  {
    if (i < sampleCount)
    {
      vColor += texelFetch(originalTexture, ivec2(tmp), i);
    }
  }
  vFragColor = vColor / float(sampleCount) + frameUniforms.bloomPulse * frameUniforms.bloomLevel * texture(blurredTexture, texcoord);
}
