#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(set = 1, binding = 0) uniform sampler2D fontAtlasTexture;

layout(location = 0) in vec4 inEyePosition;
layout(location = 1) in vec2 inTexcoords;
layout(location = 2) in vec4 inSphereRadius;
layout(location = 3) flat in int inInstanceID;

layout(location = 0) out vec4 vFragColor;

void main()
{
  vec4 pos = inEyePosition;
  pos.z += inSphereRadius.z + globalAxesUniforms.textDisplacement[inInstanceID].z;
  pos = frameUniforms.axesProjectionMatrix * pos;
  gl_FragDepth = pos.z / pos.w;

  vec4 color = globalAxesUniforms.textColor[inInstanceID];
  float edgeDistance = 0.5;
  float sampleDistance = texture(fontAtlasTexture, inTexcoords).r;
  float edgeWidth = length(vec2(dFdx(sampleDistance), dFdy(sampleDistance)));
  float insideness = smoothstep(edgeDistance - edgeWidth, edgeDistance + edgeWidth, sampleDistance);
  vFragColor = vec4(color.rgb * insideness, insideness);
}
