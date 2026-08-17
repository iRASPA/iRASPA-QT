#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 instancePosition;
layout(location = 2) in vec4 instanceScale;
layout(location = 3) in vec4 instanceAmbientColor;
layout(location = 4) in vec4 instanceDiffuseColor;
layout(location = 5) in vec4 instanceSpecularColor;

layout(location = 0) out vec4 outEyePosition;
layout(location = 1) out vec2 outTexcoords;
layout(location = 2) flat out vec4 outInstancePosition;
layout(location = 3) flat out vec4 outAmbient;
layout(location = 4) flat out vec4 outDiffuse;
layout(location = 5) flat out vec4 outSpecular;
layout(location = 6) out vec3 outFragPos;
layout(location = 7) flat out vec3 outFragCenter;
layout(location = 8) out vec3 outL;
layout(location = 9) out vec3 outV;
layout(location = 10) flat out vec4 outSphereRadius;
layout(location = 11) flat out float outK1;
layout(location = 12) flat out float outK2;

void main()
{
  if (structureUniforms.colorAtomsWithBondColor != 0)
  {
    outAmbient = lightUniforms.lights[0].ambient * structureUniforms.bondAmbientColor;
    outDiffuse = lightUniforms.lights[0].diffuse * structureUniforms.bondDiffuseColor;
    outSpecular = lightUniforms.lights[0].specular * structureUniforms.bondSpecularColor;
  }
  else
  {
    outAmbient = lightUniforms.lights[0].ambient * structureUniforms.atomAmbientColor * instanceAmbientColor;
    outDiffuse = lightUniforms.lights[0].diffuse * structureUniforms.atomDiffuseColor * instanceDiffuseColor;
    outSpecular = lightUniforms.lights[0].specular * structureUniforms.atomSpecularColor * instanceSpecularColor;
  }

  outInstancePosition = instancePosition;
  outTexcoords = vertexPosition.xy;
  outSphereRadius = structureUniforms.atomScaleFactor * instanceScale;
  outEyePosition = frameUniforms.viewMatrix * structureUniforms.modelMatrix * instancePosition;
  outFragCenter = outEyePosition.xyz;
  outL = (lightUniforms.lights[0].position - outEyePosition * lightUniforms.lights[0].position.w).xyz;
  outV = -outEyePosition.xyz;

  int patchNumber = max(structureUniforms.ambientOcclusionPatchNumber, 1);
  outK1 = float(gl_InstanceIndex % patchNumber);
  outK2 = float(gl_InstanceIndex / patchNumber);

  if (instancePosition.w < 0.0)
  {
    outFragPos = vec3(0.0);
    gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
    return;
  }

  vec4 pos2 = outEyePosition;
  pos2.xy += 1.5 * outSphereRadius.xy * vertexPosition.xy;
  outFragPos = pos2.xyz;
  gl_Position = frameUniforms.projectionMatrix * pos2;
}
