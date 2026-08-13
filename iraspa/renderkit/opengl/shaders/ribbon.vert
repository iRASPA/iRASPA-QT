in vec4 vertexPosition;
in vec4 vertexNormal;
in vec2 vertexST;
in vec2 vertexPad;

out VS_OUT
{
  vec3 N;
  vec3 L;
  vec3 V;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec2 aoUV;
  vec3 baseColor;
} vs_out;

vec3 ribbonColorForStructureType(float structureType)
{
  if (structureType < 0.5) { return structureUniforms.ribbonCoilColor.xyz; }
  if (structureType < 1.5) { return structureUniforms.ribbonHelixColor.xyz; }
  if (structureType < 2.5) { return structureUniforms.ribbonSheetColor.xyz; }
  // PyMOL DNA cartoon colors (cartoon_nucleic_acid_color + per-residue base rings).
  if (structureType < 3.5) { return vec3(1.0, 1.0, 0.0); }       // yellow backbone
  if (structureType < 4.5) { return vec3(1.0, 1.0, 0.0); }       // DA / A
  if (structureType < 5.5) { return vec3(1.0, 0.0, 0.0); }       // DC / C
  if (structureType < 6.5) { return vec3(0.4, 0.4, 0.4); }       // DG / G (gray40)
  if (structureType < 7.5) { return vec3(0.6, 1.0, 0.94); }     // DT / T, U (palecyan)
  return structureUniforms.ribbonCoilColor.xyz;
}

void main()
{
  vec4 pos = vertexPosition;
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
  vec3 localNormal = vertexNormal.xyz;
  vs_out.N = (frameUniforms.normalMatrix * structureUniforms.modelMatrix * vec4(localNormal, 0.0)).xyz;

  vec3 baseColor = ribbonColorForStructureType(vertexPad.x);
  vs_out.baseColor = baseColor;
  vs_out.ambient = (lightUniforms.lights[0].ambient * structureUniforms.ribbonAmbientColor * vec4(baseColor, 1.0)).xyz;
  vs_out.diffuse = (lightUniforms.lights[0].diffuse * structureUniforms.ribbonDiffuseColor * vec4(baseColor, 1.0)).xyz;
  vs_out.specular = (lightUniforms.lights[0].specular * structureUniforms.ribbonSpecularColor).xyz;
  vs_out.aoUV = vertexST;

  vec4 P = frameUniforms.viewMatrix * structureUniforms.modelMatrix * pos;
  vs_out.L = (lightUniforms.lights[0].position - P * lightUniforms.lights[0].position.w).xyz;
  vs_out.V = -P.xyz;
}
