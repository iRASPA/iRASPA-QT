in VS_OUT
{
  vec3 ambient;
  vec3 diffuse;
} fs_in;

out vec4 vFragColor;

void main()
{
  vFragColor = vec4(structureUniforms.atomSelectionIntensity * (fs_in.ambient + fs_in.diffuse), 1.0);
}
