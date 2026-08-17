#version 450
#extension GL_GOOGLE_include_directive : require
#include "uniforms.glsl"

layout(location = 0) in vec3 inUV;
layout(location = 1) in vec3 inPosition;

layout(location = 0) out vec4 vFragColor;

layout(set = 1, binding = 0) uniform sampler3D volume;
layout(set = 1, binding = 1) uniform sampler2D depthTexture;
layout(set = 1, binding = 2) uniform sampler2DArray transferFunction;

struct Ray
{
  vec3 origin;
  vec3 direction;
};

struct AABB
{
  vec3 top;
  vec3 bottom;
};

void ray_box_intersection(Ray ray, AABB box, out float t_0, out float t_1)
{
  vec3 direction_inv = 1.0 / ray.direction;
  vec3 t_top = direction_inv * (box.top - ray.origin);
  vec3 t_bottom = direction_inv * (box.bottom - ray.origin);
  vec3 t_min = min(t_top, t_bottom);
  vec2 t = max(t_min.xx, t_min.yz);
  t_0 = max(0.0, max(t.x, t.y));
  vec3 t_max = max(t_top, t_bottom);
  t = min(t_max.xx, t_max.yz);
  t_1 = min(t.x, t.y);
}

const int numSamples = 8192;

void main()
{
  vec3 ambient, diffuse, specular;
  vec3 numberOfReplicas = structureUniforms.numberOfReplicas.xyz;
  vec3 direction = normalize(inPosition.xyz - frameUniforms.cameraPosition.xyz);
  vec4 dir = vec4(direction.x, direction.y, direction.z, 0.0);
  vec3 ray_direction = (structureUniforms.inverseBoxMatrix * structureUniforms.inverseModelMatrix * dir).xyz;

  vec3 ray_origin = inUV;

  float stepLength = isosurfaceUniforms.stepLength / numberOfReplicas.z;

  float t_0, t_1;
  Ray casting_ray = Ray(ray_origin, ray_direction);
  AABB bounding_box = AABB(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0));
  ray_box_intersection(casting_ray, bounding_box, t_0, t_1);

  vec3 ray_start = ray_origin + ray_direction * t_0;
  vec3 ray_stop = ray_origin + ray_direction * t_1;

  vec3 ray = ray_stop - ray_start;
  float ray_length = length(ray);
  vec3 step_vector = stepLength * ray / ray_length;

  float depth = texelFetch(depthTexture, ivec2(gl_FragCoord.xy), 0).r;
  float newDepth = 1.0;
  mat4 m = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * isosurfaceUniforms.boxMatrix;

  vec4 scaleToEncompassing = isosurfaceUniforms.scaleToEncompassing;

  vec4 colour = vec4(0.0, 0.0, 0.0, 0.0);
  vec3 position = ray_start;
  for (int i = 0; i < numSamples && ray_length > 0.0 && colour.a < 1.0; i++)
  {
    vec4 values = texture(volume, numberOfReplicas * (scaleToEncompassing.xyz * position));
    vec3 normal = normalize((structureUniforms.modelMatrix * transpose(structureUniforms.inverseBoxMatrix) * vec4(values.gba, 0.0)).rgb);
    vec4 c = texture(transferFunction, vec3(values.r, 0.5, float(isosurfaceUniforms.transferFunctionIndex)));

    c.a = smoothstep(isosurfaceUniforms.transparencyThreshold, 1.0, c.a);

    vec3 R = reflect(-direction, normal);
    ambient = vec3(0.1, 0.1, 0.1);
    float dotProduct = dot(normal, direction);

    if (dotProduct < 0.0)
    {
      ambient = isosurfaceUniforms.ambientBackSide.rgb;
      diffuse = vec3(max(abs(dotProduct), 0.0)) * isosurfaceUniforms.diffuseBackSide.rgb;
      specular = vec3(pow(max(dot(R, direction), 0.0), isosurfaceUniforms.shininessBackSide)) * isosurfaceUniforms.specularBackSide.rgb;
      vec3 totalColor = (ambient + diffuse + specular).rgb;

      if (isosurfaceUniforms.backHDR != 0)
      {
        totalColor = 1.0 - exp2(-totalColor * isosurfaceUniforms.backHDRExposure);
      }

      c.a = 1.0 - pow(1.0 - c.a, stepLength * 2000.0);
      colour.rgb += (1.0 - colour.a) * c.a * c.rgb * totalColor.rgb;
      colour.a += (1.0 - colour.a) * c.a;
    }
    else
    {
      ambient = isosurfaceUniforms.ambientFrontSide.rgb;
      diffuse = vec3(max(abs(dotProduct), 0.0)) * isosurfaceUniforms.diffuseFrontSide.rgb;
      specular = vec3(pow(max(dot(R, direction), 0.0), isosurfaceUniforms.shininessFrontSide)) * isosurfaceUniforms.specularFrontSide.rgb;
      vec3 totalColor = (ambient + diffuse + specular).rgb;

      if (isosurfaceUniforms.frontHDR != 0)
      {
        totalColor = 1.0 - exp2(-totalColor * isosurfaceUniforms.frontHDRExposure);
      }

      c.a = 1.0 - pow(1.0 - c.a, stepLength * 2000.0);
      colour.rgb += (1.0 - colour.a) * c.a * c.rgb * totalColor.rgb;
      colour.a += (1.0 - colour.a) * c.a;
    }

    position = position + step_vector;
    ray_length -= stepLength;

    vec4 clipPosition = m * vec4(position, 1.0);
    newDepth = clipPosition.z / clipPosition.w;
    if (newDepth > depth)
    {
      break;
    }
  }

  if (colour.a < 0.5)
  {
    newDepth = depth;
  }

  gl_FragDepth = newDepth;

  vec3 hsv = rgb2hsv(colour.xyz);
  hsv.x = hsv.x * isosurfaceUniforms.hue;
  hsv.y = hsv.y * isosurfaceUniforms.saturation;
  hsv.z = hsv.z * isosurfaceUniforms.value;
  vFragColor = vec4(hsv2rgb(hsv) * isosurfaceUniforms.diffuseFrontSide.w * colour.a,
                    isosurfaceUniforms.diffuseFrontSide.w * colour.a);
}
