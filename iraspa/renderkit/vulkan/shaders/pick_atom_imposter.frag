#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 inEyePosition;
layout(location = 1) in vec2 inTexcoords;
layout(location = 2) flat in vec4 inInstancePosition;
layout(location = 3) in vec3 inFragPos;
layout(location = 4) flat in vec3 inFragCenter;
layout(location = 5) flat in vec4 inSphereRadius;
layout(location = 6) flat in int inInstanceId;
layout(location = 7) flat in float inIsPerspective;

layout(location = 0) out uvec4 vFragColor;

void main()
{
  if (inInstancePosition.w < 0.0)
  {
    discard;
  }

  vec3 N;
  vec4 screenPos;
  if (inIsPerspective > 0.5)
  {
    vec3 rij = -inFragCenter;
    vec3 vij = inFragPos;
    float A = dot(vij, vij);
    float B = dot(rij, vij);
    float C = dot(rij, rij) - inSphereRadius.z * inSphereRadius.z;
    float argument = B * B - A * C;
    if (argument < 0.0)
    {
      discard;
    }
    float t = -C / (B - sqrt(argument));
    vec3 hit = t * vij;
    N = normalize(hit - inFragCenter);
    screenPos = frameUniforms.projectionMatrix * vec4(hit, 1.0);
  }
  else
  {
    float x = inTexcoords.x;
    float y = inTexcoords.y;
    float zz = 1.0 - x * x - y * y;
    if (zz <= 0.0)
    {
      discard;
    }
    float z = sqrt(zz);
    N = vec3(x, y, z);
    vec4 pos = inEyePosition;
    pos.z += inSphereRadius.z * z;
    screenPos = frameUniforms.projectionMatrix * pos;
  }

  gl_FragDepth = screenPos.z / screenPos.w;

  if (structureUniforms.clipAtomsAtUnitCell != 0)
  {
    mat4 aoTransform = transpose(frameUniforms.normalMatrix * structureUniforms.modelMatrix);
    vec3 vertexPosition = (aoTransform * (inSphereRadius * vec4(N, 1.0))).xyz;
    vec4 objectPosition = vec4(inInstancePosition.xyz + vertexPosition, 1.0);
    if (clippedByUnitCellPlanes(objectPosition))
    {
      discard;
    }
  }

  vFragColor = uvec4(1u, uint(structureUniforms.sceneIdentifier), uint(structureUniforms.MovieIdentifier), uint(inInstanceId));
}
