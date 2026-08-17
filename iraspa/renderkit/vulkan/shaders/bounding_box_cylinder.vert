#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec4 instancePosition1;
layout(location = 3) in vec4 instancePosition2;
layout(location = 4) in vec4 instanceScale;

layout(location = 0) out vec3 outN;
layout(location = 1) out vec3 outL;

void main()
{
  vec4 scale = instanceScale;
  vec4 pos = scale * vec4(vertexPosition.xyz, 1.0);
  vec4 pos1 = instancePosition1;
  vec4 pos2 = instancePosition2;
  vec3 dr = (pos1 - pos2).xyz;
  float bondLength = length(dr);

  scale.x = 1.0;
  scale.y = bondLength;
  scale.z = 1.0;
  scale.w = 1.0;

  dr = normalize(dr);
  vec3 v1;
  if ((dr.z != 0.0) && (-dr.x != dr.y))
    v1 = normalize(vec3(-dr.y - dr.z, dr.x, dr.x));
  else
    v1 = normalize(vec3(dr.z, dr.z, -dr.x - dr.y));
  vec3 v2 = normalize(cross(dr, v1));
  mat4 orientationMatrix = mat4(vec4(-v1.x, -v1.y, -v1.z, 0.0),
                                vec4(-dr.x, -dr.y, -dr.z, 0.0),
                                vec4(-v2.x, -v2.y, -v2.z, 0.0),
                                vec4(0.0, 0.0, 0.0, 1.0));

  outN = (frameUniforms.normalMatrix * orientationMatrix * vertexNormal).xyz;
  vec4 worldPos = vec4((orientationMatrix * (scale * pos) + pos1).xyz, 1.0);
  vec4 P = frameUniforms.viewMatrix * worldPos;
  outL = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
  gl_Position = frameUniforms.mvpMatrix * worldPos;
}
