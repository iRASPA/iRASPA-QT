in vec4 vertexPosition;
in vec4 vertexNormal;
in vec2 vertexST;
in vec2 vertexPad;
in vec2 vertexStripeST;

out VS_OUT
{
  vec3 N;
  vec3 L;
  vec2 stripeST;
} vs_out;

void main()
{
  vec3 expandedPosition = ribbonSelectionExpandedPosition(vertexPosition, vertexNormal, 0.45);
  vec4 pos = vec4(expandedPosition, 1.0);
  vec3 localNormal = normalize(vertexNormal.xyz);

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vec4(localNormal, 0.0)).xyz;
  vs_out.stripeST = vertexStripeST;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;
  vs_out.L = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
}
