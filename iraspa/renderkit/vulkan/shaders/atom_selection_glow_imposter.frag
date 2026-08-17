#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "selection_imposter.glsl"

layout(location = 0) out vec4 unusedSceneColor;
layout(location = 1) out vec4 vFragColor;

void main()
{
  vec3 N;
  vec3 modelN;
  if (!reconstructSelectionImposter(N, modelN))
  {
    discard;
  }

  unusedSceneColor = vec4(0.0);
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.atomSelectionIntensity;
  vec3 color = bloomLevel * (inAmbient.xyz + inDiffuse.xyz);
  vFragColor = vec4(color, bloomLevel);
}
