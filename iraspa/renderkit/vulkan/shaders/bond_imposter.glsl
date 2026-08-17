// Helpers for ray-traced bond-cylinder imposters (ported from the OpenGL/Metal
// implementation). Each (sub-)cylinder is drawn as a view-aligned hull of three
// quads (18 vertices generated from gl_VertexIndex): a quad at each end-cap plane
// and a quad on the camera-facing tangent plane. The fragment shader ray-traces
// the analytic capped cylinder in eye space and writes the exact surface depth.
// Works for both orthographic and perspective cameras.

const vec3 bondImposterHullOffsets[18] = vec3[18](
  // cap quad at endpoint A
  vec3(-1.0, -1.0, -1.0),
  vec3( 1.0, -1.0, -1.0),
  vec3(-1.0, -1.0,  1.0),
  vec3( 1.0, -1.0,  1.0),
  vec3(-1.0, -1.0,  1.0),
  vec3( 1.0, -1.0, -1.0),
  // camera-facing quad
  vec3(-1.0, -1.0,  1.0),
  vec3( 1.0, -1.0,  1.0),
  vec3(-1.0,  1.0,  1.0),
  vec3( 1.0,  1.0,  1.0),
  vec3(-1.0,  1.0,  1.0),
  vec3( 1.0, -1.0,  1.0),
  // cap quad at endpoint B
  vec3(-1.0,  1.0,  1.0),
  vec3( 1.0,  1.0,  1.0),
  vec3(-1.0,  1.0, -1.0),
  vec3( 1.0,  1.0, -1.0),
  vec3(-1.0,  1.0, -1.0),
  vec3( 1.0,  1.0,  1.0)
);

// displacement (in mesh x,z of the cylinder) of the sub-cylinders of double/triple bonds,
// matching CappedCylinderDouble/TripleBondGeometry
vec2 bondImposterSubCylinderOffset(int type, int sub)
{
  if (type == 1)  // double bond
  {
    return vec2((sub == 0) ? -1.0 : 1.0, 0.0);
  }
  else if (type == 3)  // triple bond
  {
    float dz = 0.5 * sqrt(3.0);
    return (sub == 0) ? vec2(-1.0, -dz) : ((sub == 1) ? vec2(1.0, -dz) : vec2(0.0, dz));
  }
  return vec2(0.0, 0.0);
}

// builds the hull vertex position in eye space for a cylinder from a to b
vec3 bondImposterHullPosition(vec3 a, vec3 b, float radius, int vid, bool orthographic)
{
  vec3 vHalf = 0.5 * (b - a);
  vec3 center = a + vHalf;

  // direction from the camera towards the bond
  vec3 e = orthographic ? vec3(0.0, 0.0, -1.0) : center;

  vec3 u = cross(vHalf, e);
  if (dot(u, u) < 1.0e-8) u = cross(vHalf, vec3(0.0, 1.0, 0.0));
  if (dot(u, u) < 1.0e-8) u = cross(vHalf, vec3(1.0, 0.0, 0.0));
  u = normalize(u);
  vec3 w = normalize(cross(u, normalize(vHalf)));
  if (dot(w, e) > 0.0) w = -w;  // make w point towards the camera

  vec3 coords = bondImposterHullOffsets[vid % 18];
  return center + radius * (coords.x * u + coords.z * w) + coords.y * vHalf;
}

// returns the entry and exit ray parameters of the capped cylinder from a to b
// (a convex solid, so the ray intersects its surface exactly twice or not at all)
bool bondImposterIntersectInterval(vec3 ro, vec3 rd, vec3 a, vec3 b, float r, out float tEnter, out float tExit)
{
  vec3 ba = b - a;
  vec3 oc = ro - a;
  float baba = dot(ba, ba);
  float bard = dot(ba, rd);
  float baoc = dot(ba, oc);

  float tMin = -1.0e30;
  float tMax = 1.0e30;
  tEnter = 0.0;
  tExit = 0.0;

  // infinite cylinder around the axis
  float k2 = baba - bard * bard;
  float k1 = baba * dot(oc, rd) - baoc * bard;
  float k0 = baba * dot(oc, oc) - baoc * baoc - r * r * baba;
  if (k2 > 1.0e-8)
  {
    float h = k1 * k1 - k2 * k0;
    if (h < 0.0) return false;
    h = sqrt(h);
    tMin = max(tMin, (-k1 - h) / k2);
    tMax = min(tMax, (-k1 + h) / k2);
  }
  else if (k0 > 0.0)
  {
    return false;  // ray parallel to the axis and outside the radius
  }

  // slab between the two end-cap planes
  if (abs(bard) > 1.0e-8)
  {
    float t0 = (0.0 - baoc) / bard;
    float t1 = (baba - baoc) / bard;
    tMin = max(tMin, min(t0, t1));
    tMax = min(tMax, max(t0, t1));
  }
  else if (baoc < 0.0 || baoc > baba)
  {
    return false;  // ray parallel to the caps and outside the slab
  }

  if (tMin > tMax) return false;
  tEnter = tMin;
  tExit = tMax;
  return true;
}

// ray-traces a capped cylinder from a to b; returns the ray parameter t (or a negative
// value when there is no intersection) and sets the surface normal and the fraction
// ct (0 at a, 1 at b) along the axis
float bondImposterIntersect(vec3 ro, vec3 rd, vec3 a, vec3 b, float r, out vec3 N, out float ct)
{
  vec3 ba = b - a;
  vec3 oc = ro - a;
  float baba = dot(ba, ba);
  float bard = dot(ba, rd);
  float baoc = dot(ba, oc);
  float k2 = baba - bard * bard;
  float k1 = baba * dot(oc, rd) - baoc * bard;
  float k0 = baba * dot(oc, oc) - baoc * baoc - r * r * baba;
  float h = k1 * k1 - k2 * k0;
  N = vec3(0.0);
  ct = 0.0;
  if (h < 0.0) return -1.0;
  h = sqrt(h);
  float t = (-k1 - h) / k2;

  // body of the cylinder
  float y = baoc + t * bard;
  if (y > 0.0 && y < baba)
  {
    N = (oc + t * rd - ba * y / baba) / r;
    ct = y / baba;
    return t;
  }

  // flat end-caps
  t = (((y < 0.0) ? 0.0 : baba) - baoc) / bard;
  if (abs(k1 + k2 * t) >= h) return -1.0;
  N = ba * sign(y) / sqrt(baba);
  ct = (y < 0.0) ? 0.0 : 1.0;
  return t;
}

// ray-traces the capped cylinder from a to b clipped by the six unit-cell planes,
// analytically generating the flat caps at the cell boundary (this replaces the
// stencil-based cap pass of the triangle-mesh path)
float bondImposterClippedIntersect(vec3 ro, vec3 rd, vec3 a, vec3 b, float r, mat4 toStructure,
                                   out vec3 N, out float ct)
{
  vec3 ba = b - a;
  vec3 oc = ro - a;
  float baba = dot(ba, ba);
  float bard = dot(ba, rd);
  float baoc = dot(ba, oc);
  float k2 = baba - bard * bard;
  float k1 = baba * dot(oc, rd) - baoc * bard;
  float k0 = baba * dot(oc, oc) - baoc * baoc - r * r * baba;

  float tmin = -1.0e30;
  float tmax = 1.0e30;
  int entryType = -1;

  if (k2 > 1.0e-6 * baba)
  {
    float h = k1 * k1 - k2 * k0;
    if (h < 0.0)
    {
      N = vec3(0.0);
      ct = 0.0;
      return -1.0;
    }
    h = sqrt(h);
    tmin = (-k1 - h) / k2;
    tmax = (-k1 + h) / k2;
    entryType = 0;
  }
  else if (k0 > 0.0)
  {
    N = vec3(0.0);
    ct = 0.0;
    return -1.0;
  }

  if (abs(bard) > 1.0e-6)
  {
    float tCapA = (0.0 - baoc) / bard;
    float tCapB = (baba - baoc) / bard;
    float tEnter = min(tCapA, tCapB);
    float tExit = max(tCapA, tCapB);
    if (tEnter > tmin)
    {
      tmin = tEnter;
      entryType = 1;
    }
    tmax = min(tmax, tExit);
  }
  else if (baoc < 0.0 || baoc > baba)
  {
    N = vec3(0.0);
    ct = 0.0;
    return -1.0;
  }

  vec4 planes[6] = vec4[6](structureUniforms.clipPlaneLeft, structureUniforms.clipPlaneRight,
                           structureUniforms.clipPlaneTop, structureUniforms.clipPlaneBottom,
                           structureUniforms.clipPlaneFront, structureUniforms.clipPlaneBack);
  vec4 so = toStructure * vec4(ro, 1.0);
  vec4 sd = toStructure * vec4(rd, 0.0);
  for (int i = 0; i < 6; i++)
  {
    float f0 = dot(planes[i], so);
    float df = dot(planes[i], sd);
    if (abs(df) < 1.0e-8)
    {
      if (f0 < 0.0)
      {
        N = vec3(0.0);
        ct = 0.0;
        return -1.0;
      }
    }
    else
    {
      float tp = -f0 / df;
      if (df > 0.0)
      {
        if (tp > tmin)
        {
          tmin = tp;
          entryType = 2 + i;
        }
      }
      else
      {
        tmax = min(tmax, tp);
      }
    }
  }

  if (tmax < tmin || tmin < 0.0 || entryType < 0)
  {
    N = vec3(0.0);
    ct = 0.0;
    return -1.0;
  }

  float t = tmin;
  float y = baoc + t * bard;
  ct = clamp(y / baba, 0.0, 1.0);

  if (entryType == 0)
  {
    N = (oc + t * rd - ba * y / baba) / r;
  }
  else if (entryType == 1)
  {
    N = (y < 0.5 * baba) ? -ba / sqrt(baba) : ba / sqrt(baba);
  }
  else
  {
    vec4 planeEye = transpose(toStructure) * planes[entryType - 2];
    N = -normalize(planeEye.xyz);
  }
  return t;
}
