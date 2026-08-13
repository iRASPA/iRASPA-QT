uniform sampler2D ambientOcclusionTexture;
uniform sampler2D ambientOcclusionRawTexture;

in VS_OUT
{
  vec3 N;
  vec3 L;
  vec3 V;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec2 aoUV;
  vec3 baseColor;
} fs_in;

out vec4 vFragColor;

float sampleRibbonAmbientOcclusion(vec2 uv, vec2 inverseTexSize)
{
  float sum = 0.0;
  float weight = 0.0;
  for (int dy = -1; dy <= 1; dy++)
  {
    for (int dx = -1; dx <= 1; dx++)
    {
      vec2 offset = vec2(float(dx), float(dy)) * inverseTexSize;
      float value = texture(ambientOcclusionTexture, uv + offset).r;
      float tapWeight = (dx == 0 && dy == 0) ? 2.0 : 1.0;
      sum += value * tapWeight;
      weight += tapWeight;
    }
  }
  return sum / max(weight, 1.0);
}

vec2 ribbonAmbientOcclusionUV(vec2 st)
{
  return vec2(st.x, 1.0 - st.y);
}

void main()
{
  vec2 inverseTextureSize = vec2(1.0 / max(float(ribbonAODebug.textureWidth), 1.0),
                                 1.0 / max(float(ribbonAODebug.textureHeight), 1.0));
  vec2 aoUV = ribbonAmbientOcclusionUV(fs_in.aoUV);
  float aoSample = 1.0;
  if (structureUniforms.ribbonAmbientOcclusion && ribbonAODebug.fastInteractionShading == 0)
  {
    aoSample = sampleRibbonAmbientOcclusion(aoUV, inverseTextureSize);
  }

  if (ribbonAODebug.mode != 0)
  {
    if (ribbonAODebug.mode == 1)
    {
      vFragColor = vec4(aoUV.x, aoUV.y, 0.0, 1.0);
      return;
    }
    if (ribbonAODebug.mode == 2)
    {
      vFragColor = vec4(aoSample, aoSample, aoSample, 1.0);
      return;
    }
    if (ribbonAODebug.mode == 3)
    {
      vec2 texelCoord = aoUV * vec2(float(ribbonAODebug.textureWidth), float(ribbonAODebug.textureHeight));
      ivec2 checker = ivec2(floor(texelCoord));
      float c = float((checker.x + checker.y) & 1);
      vFragColor = vec4(c, c, c, 1.0);
      return;
    }
    if (ribbonAODebug.mode == 4)
    {
      vFragColor = vec4(fs_in.baseColor, 1.0);
      return;
    }
    if (ribbonAODebug.mode == 5)
    {
      vec2 screenUV = gl_FragCoord.xy / vec2(max(float(ribbonAODebug.viewportWidth), 1.0),
                                             max(float(ribbonAODebug.viewportHeight), 1.0));
      if (screenUV.y > 0.88)
      {
        float band = screenUV.x * 3.0;
        if (band < 1.0)
        {
          vFragColor = vec4(structureUniforms.ribbonCoilColor.rgb, 1.0);
          return;
        }
        if (band < 2.0)
        {
          vFragColor = vec4(structureUniforms.ribbonHelixColor.rgb, 1.0);
          return;
        }
        vFragColor = vec4(structureUniforms.ribbonSheetColor.rgb, 1.0);
        return;
      }
      vFragColor = vec4(fs_in.baseColor, 1.0);
      return;
    }
    if (ribbonAODebug.mode == 6)
    {
      float atlasValue = texture(ambientOcclusionTexture, aoUV).r;
      vFragColor = vec4(atlasValue, 0.0, 0.0, 1.0);
      return;
    }
    if (ribbonAODebug.mode == 7)
    {
      float atlasValue = texture(ambientOcclusionRawTexture, aoUV).r;
      vFragColor = vec4(atlasValue, 0.0, 0.0, 1.0);
      return;
    }
  }

  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L);
  vec3 V = normalize(fs_in.V);
  vec3 R = reflect(-L, N);

  vec3 ambient = fs_in.ambient;
  vec3 diffuse = max(dot(N, L), 0.0) * fs_in.diffuse;
  vec3 specular = pow(max(dot(R, V), 0.0), lightUniforms.lights[0].shininess + structureUniforms.ribbonShininess) * fs_in.specular;

  float ao = 1.0;
  if (structureUniforms.ribbonAmbientOcclusion && ribbonAODebug.fastInteractionShading == 0)
  {
    ao = aoSample;
  }

  vec4 color = vec4(ao * (ambient + diffuse + specular), 1.0);
  if (structureUniforms.ribbonHDR)
  {
    vec4 vLdrColor = 1.0 - exp2(-color * structureUniforms.ribbonHDRExposure);
    vLdrColor.a = 1.0;
    color = vLdrColor;
  }

  vec3 hsv = rgb2hsv(color.xyz);
  hsv.x = hsv.x * structureUniforms.ribbonHue;
  hsv.y = hsv.y * structureUniforms.ribbonSaturation;
  hsv.z = hsv.z * structureUniforms.ribbonValue;
  vFragColor = vec4(hsv2rgb(hsv), 1.0);
}
