#pragma once

#ifdef HAS_DX
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
#endif

#ifdef HAS_DX
struct SVertex
{
  float x, y, z;
  XMFLOAT4 col;
  float u, v;
  float u2, v2;
};
#else
struct SVertex
{
  float x, y, z;
  unsigned char r, g, b, a;
  float u, v;
};
#endif
