#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec4 inEyePosition;
layout(location = 1) in vec2 inTexcoords;
layout(location = 2) flat in vec4 inInstancePosition;
layout(location = 3) flat in vec4 inAmbient;
layout(location = 4) flat in vec4 inDiffuse;
layout(location = 5) flat in vec4 inSpecular;
layout(location = 6) in vec3 inFragPos;
layout(location = 7) flat in vec3 inFragCenter;
layout(location = 8) in vec3 inL;
layout(location = 9) in vec3 inV;
layout(location = 10) flat in vec4 inSphereRadius;
layout(location = 11) flat in float inIsPerspective;

layout(location = 0) out vec4 vFragColor;

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

  mat4 aoTransform = transpose(frameUniforms.normalMatrix * structureUniforms.modelMatrix);
  vec3 modelN = (aoTransform * vec4(N, 1.0)).xyz;
  vec2 st = vec2(0.5 + 0.5 * atan(modelN.z, modelN.x) / 3.141592653589793, 0.5 - asin(modelN.y) / 3.141592653589793);
  if (fract(st.x * 8.0) >= 0.125 && fract(st.y * 8.0) >= 0.125)
  {
    discard;
  }

  vec3 L = normalize(inL);
  vec3 ambient = inAmbient.xyz;
  vec3 diffuse = max(dot(N, L), 0.0) * inDiffuse.xyz;
  vFragColor = vec4((ambient + diffuse) * 0.8, 0.8);
}
