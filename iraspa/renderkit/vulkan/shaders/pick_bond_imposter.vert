#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"
#include "bond_imposter.glsl"

layout(location = 0) in vec4 instancePosition1;
layout(location = 1) in vec4 instancePosition2;
layout(location = 2) in vec4 instanceColor1;
layout(location = 3) in vec4 instanceColor2;
layout(location = 4) in vec4 instanceScale;
layout(location = 5) in int instanceType;
layout(location = 6) in int instanceTag;

layout(location = 0) out vec3 outFragPos;
layout(location = 1) flat out vec3 outPointA;
layout(location = 2) flat out vec3 outPointB;
layout(location = 3) flat out float outRadius;
layout(location = 4) flat out int outInstanceId;

void main()
{
  vec4 pos1 = instancePosition1;
  vec4 pos2 = instancePosition2;
  outInstanceId = instanceTag;

  int type = (structureUniforms.isUnity != 0) ? 0 : instanceType;
  vec2 offset = bondImposterSubCylinderOffset(type, gl_VertexIndex / 18);

  vec3 dr = normalize((pos1 - pos2).xyz);
  vec3 v1 = normalize(abs(dr.x) > abs(dr.z) ? vec3(-dr.y, dr.x, 0.0) : vec3(0.0, -dr.z, dr.y));
  vec3 v2 = normalize(cross(dr, v1));

  vec3 displacement = structureUniforms.bondScaling * (offset.x * (-v1) + offset.y * (-v2));
  float radius = structureUniforms.bondScaling;

  mat4 mv = frameUniforms.viewMatrix * structureUniforms.modelMatrix;
  vec3 a = (mv * vec4(pos1.xyz + displacement, 1.0)).xyz;
  vec3 b = (mv * vec4(pos2.xyz + displacement, 1.0)).xyz;

  bool orthographic = (frameUniforms.projectionMatrix[3][3] > 0.5);
  vec3 posEye = bondImposterHullPosition(a, b, radius, gl_VertexIndex, orthographic);

  outFragPos = posEye;
  outPointA = a;
  outPointB = b;
  outRadius = radius;
  gl_Position = frameUniforms.projectionMatrix * vec4(posEye, 1.0);

  if (pos1.w < 0.0 || pos2.w < 0.0)
  {
    gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
  }
}
