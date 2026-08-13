in vec4 vertexPosition;
in vec4 vertexNormal;
in vec2 vertexST;
in vec2 vertexPad;
in vec2 vertexStripeST;

out VS_OUT
{
  vec3 N;
  vec3 L;
  vec3 V;
  vec3 Model_N;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
} vs_out;

void main()
{
  vec3 expandedPosition = ribbonSelectionExpandedPosition(vertexPosition, vertexNormal, 0.2);
  vec4 pos = vec4(expandedPosition, 1.0);
  vec3 localNormal = normalize(vertexNormal.xyz);

  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vec4(localNormal, 0.0)).xyz;
  vs_out.Model_N = localNormal;

  vec3 baseColor = vec3(1.0, 1.0, 0.0);
  vs_out.ambient = (lightUniforms.lights[0].ambient * structureUniforms.ribbonAmbientColor * vec4(baseColor, 1.0)).xyz;
  vs_out.diffuse = (lightUniforms.lights[0].diffuse * structureUniforms.ribbonDiffuseColor * vec4(baseColor, 1.0)).xyz;
  vs_out.specular = (lightUniforms.lights[0].specular * structureUniforms.ribbonSpecularColor).xyz;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;
  vs_out.L = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
  vs_out.V = -P.xyz;
}
