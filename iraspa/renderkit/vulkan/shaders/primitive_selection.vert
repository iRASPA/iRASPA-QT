#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec4 instancePosition;
layout(location = 3) in vec4 instanceScale;

layout(location = 0) out vec3 outN;
layout(location = 1) out vec3 outModelN;
layout(location = 2) out vec3 outL;
layout(location = 3) out vec3 outV;

void main()
{
  vec4 scale = structureUniforms.primitiveSelectionScaling * instanceScale;
  vec4 pos = structureUniforms.transformationMatrix * (scale * vertexPosition) + instancePosition;

  outN = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * structureUniforms.transformationNormalMatrix * vertexNormal).xyz;
  outModelN = vertexPosition.xyz;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;
  outL = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
  outV = -P.xyz;

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
}
