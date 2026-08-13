in VS_OUT
{
  vec3 N;
  vec3 L;
  vec3 V;
  vec3 Model_N;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
} fs_in;

out vec4 vFragColor;

void main()
{
  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L);
  vec3 V = normalize(fs_in.V);
  vec3 R = reflect(-L, N);

  vec4 ambient = vec4(fs_in.ambient, 1.0);
  vec4 diffuse = max(dot(N, L), 0.0) * vec4(fs_in.diffuse, 1.0);
  vec4 specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.ribbonShininess) * vec4(fs_in.specular, 1.0);

  vec3 t1 = fs_in.Model_N;
  float frequency = structureUniforms.atomSelectionWorleyNoise3DFrequency;
  float jitter = structureUniforms.atomSelectionWorleyNoise3DJitter;
  vec2 F = cellular3D(frequency * vec3(t1.x, t1.z, t1.y), jitter);
  float n = F.y - F.x;

  vec4 color = n * (ambient + diffuse + specular);
  if (structureUniforms.ribbonHDR)
  {
    color = 1.0 - exp2(-color * structureUniforms.ribbonHDRExposure);
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.ribbonHue;
  hsv.y = hsv.y * structureUniforms.ribbonSaturation;
  hsv.z = hsv.z * structureUniforms.ribbonValue;
  float bloomLevel = frameUniforms.bloomLevel * structureUniforms.atomSelectionIntensity;
  vFragColor = vec4(hsv2rgb(hsv) * bloomLevel, bloomLevel);
}
