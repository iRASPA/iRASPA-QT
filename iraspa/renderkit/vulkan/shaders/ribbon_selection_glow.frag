#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec3 inAmbient;
layout(location = 1) in vec3 inDiffuse;

layout(location = 0) out vec4 unusedSceneColor;
layout(location = 1) out vec4 vFragColor;

void main()
{
  unusedSceneColor = vec4(0.0);
  vFragColor = vec4(structureUniforms.atomSelectionIntensity * (inAmbient + inDiffuse), 1.0);
}
