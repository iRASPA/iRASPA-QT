#version 450

layout(set = 0, binding = 0) uniform sampler2DMS depthMS;

void main()
{
  gl_FragDepth = texelFetch(depthMS, ivec2(gl_FragCoord.xy), 0).r;
}
