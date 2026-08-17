#ifndef MC_COMMON_GLSL
#define MC_COMMON_GLSL

const ivec3 cubeOffsets[8] = ivec3[](
  ivec3(0, 0, 0),
  ivec3(1, 0, 0),
  ivec3(0, 0, 1),
  ivec3(1, 0, 1),
  ivec3(0, 1, 0),
  ivec3(1, 1, 0),
  ivec3(0, 1, 1),
  ivec3(1, 1, 1)
);

const int offsets3[72] = int[](
  0,0,0, 1,0,0,
  1,0,0, 1,0,1,
  1,0,1, 0,0,1,
  0,0,1, 0,0,0,
  0,1,0, 1,1,0,
  1,1,0, 1,1,1,
  1,1,1, 0,1,1,
  0,1,1, 0,1,0,
  0,0,0, 0,1,0,
  1,0,0, 1,1,0,
  1,0,1, 1,1,1,
  0,0,1, 0,1,1
);

int wrap1(int v, int d)
{
  int r = v % d;
  return r < 0 ? r + d : r;
}

ivec3 wrapCoord(ivec3 p, ivec3 dimensions)
{
  return ivec3(wrap1(p.x, dimensions.x), wrap1(p.y, dimensions.y), wrap1(p.z, dimensions.z));
}

#endif
