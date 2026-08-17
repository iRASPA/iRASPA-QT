#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "ribbon_selection.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec2 vertexST;
layout(location = 3) in vec2 vertexPad;
layout(location = 4) in vec2 vertexStripeST;

layout(location = 0) out vec3 outAmbient;
layout(location = 1) out vec3 outDiffuse;

void main()
{
  vec4 pos = vec4(ribbonSelectionExpandedPosition(vertexPosition, vertexNormal, 0.2), 1.0);
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;

  vec3 baseColor = vec3(1.0, 1.0, 0.0);
  outAmbient = (lightUniforms.lights[0].ambient * structureUniforms.ribbonAmbientColor * vec4(baseColor, 1.0)).xyz;
  outDiffuse = (lightUniforms.lights[0].diffuse * structureUniforms.ribbonDiffuseColor * vec4(baseColor, 1.0)).xyz;
}
