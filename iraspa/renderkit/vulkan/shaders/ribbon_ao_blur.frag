#version 450

layout(set = 0, binding = 0) uniform sampler2D sourceTexture;

layout(push_constant) uniform PushConstants
{
  vec2 inverseTextureSize;
  vec2 axis;
} push;

layout(location = 0) in vec2 texCoord;
layout(location = 0) out float vFragColor;

void main()
{
  const float weights[8] = float[](
      0.159576912161, 0.147308056121, 0.115876621105, 0.0776744219933,
      0.0443683338718, 0.0215963866053, 0.00895781211794, 0.0044299121055113265);
  const float horizontalSteps[7] = float[](8.0, 16.0, 24.0, 32.0, 40.0, 48.0, 56.0);
  const float verticalSteps[7] = float[](1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);

  float sum = texture(sourceTexture, texCoord).r * weights[0];
  vec2 texel = push.inverseTextureSize * push.axis;
  for (int i = 0; i < 7; ++i)
  {
    float stepSize = (push.axis.x > 0.5) ? horizontalSteps[i] : verticalSteps[i];
    vec2 offset = texel * stepSize;
    sum += texture(sourceTexture, texCoord - offset).r * weights[i + 1];
    sum += texture(sourceTexture, texCoord + offset).r * weights[i + 1];
  }
  vFragColor = sum;
}
