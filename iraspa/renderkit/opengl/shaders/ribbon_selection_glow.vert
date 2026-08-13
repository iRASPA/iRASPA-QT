in vec4 vertexPosition;
in vec4 vertexNormal;
in vec2 vertexST;
in vec2 vertexPad;
in vec2 vertexStripeST;

out VS_OUT
{
  vec3 ambient;
  vec3 diffuse;
} vs_out;

void main()
{
  vec3 expandedPosition = ribbonSelectionExpandedPosition(vertexPosition, vertexNormal, 0.2);
  vec4 pos = vec4(expandedPosition, 1.0);
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;

  vec3 baseColor = vec3(1.0, 1.0, 0.0);
  vs_out.ambient = (lightUniforms.lights[0].ambient * structureUniforms.ribbonAmbientColor * vec4(baseColor, 1.0)).xyz;
  vs_out.diffuse = (lightUniforms.lights[0].diffuse * structureUniforms.ribbonDiffuseColor * vec4(baseColor, 1.0)).xyz;
}
