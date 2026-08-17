#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "ribbon_selection.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec2 vertexST;
layout(location = 3) in vec2 vertexPad;
layout(location = 4) in vec2 vertexStripeST;

layout(location = 0) out vec3 outN;
layout(location = 1) out vec3 outL;
layout(location = 2) out vec3 outV;
layout(location = 3) out vec3 outModelN;
layout(location = 4) out vec3 outAmbient;
layout(location = 5) out vec3 outDiffuse;
layout(location = 6) out vec3 outSpecular;

void main()
{
  vec4 pos = vec4(ribbonSelectionExpandedPosition(vertexPosition, vertexNormal, 0.2), 1.0);
  vec3 localNormal = normalize(vertexNormal.xyz);

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
  outN = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vec4(localNormal, 0.0)).xyz;
  outModelN = localNormal;

  vec3 baseColor = vec3(1.0, 1.0, 0.0);
  outAmbient = (lightUniforms.lights[0].ambient * structureUniforms.ribbonAmbientColor * vec4(baseColor, 1.0)).xyz;
  outDiffuse = (lightUniforms.lights[0].diffuse * structureUniforms.ribbonDiffuseColor * vec4(baseColor, 1.0)).xyz;
  outSpecular = (lightUniforms.lights[0].specular * structureUniforms.ribbonSpecularColor).xyz;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;
  outL = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
  outV = -P.xyz;
}
