#include "Math/gf28.h"
#include "Tools/cpu_support.h"

const true_type gf28::characteristic_two;
const true_type gf28::invertible;

gf28& gf28::mul(const gf28& x, const gf28& y)
{
#if defined(__GFNI__)
  __m128i xa = _mm_set1_epi8((char)x.get());
  __m128i ya = _mm_set1_epi8((char)y.get());
  __m128i r  = _mm_gf2p8mul_epi8(xa, ya);
  a = (uint8_t)_mm_cvtsi128_si32(r);
  return *this;
#else
  a = mul_software(x.get(), y.get());
  return *this;
#endif
}

gf28 gf28::invert() const
{
  if (a == 0) throw division_by_zero();
  // Fermat's little theorem in GF(2^8): a^(2^8-2) = a^254
  // Use square-and-multiply with gfni/software mul
  auto mul8 = [&](uint8_t x, uint8_t y) -> uint8_t {
#if defined(__GFNI__)
    __m128i xa = _mm_set1_epi8((char)x);
    __m128i ya = _mm_set1_epi8((char)y);
    __m128i r  = _mm_gf2p8mul_epi8(xa, ya);
    return (uint8_t)_mm_cvtsi128_si32(r);
#else
    return mul_software(x, y);
#endif
  };

  uint8_t x = a;
  // exponentiate to 254: x^(254)
  // Compute x^2, x^4, x^8, x^16, x^32, x^64, x^128
  uint8_t x2 = mul8(x, x);
  uint8_t x4 = mul8(x2, x2);
  uint8_t x8 = mul8(x4, x4);
  uint8_t x16 = mul8(x8, x8);
  uint8_t x32 = mul8(x16, x16);
  uint8_t x64 = mul8(x32, x32);
  uint8_t x128 = mul8(x64, x64);
  // 254 = 128 + 64 + 32 + 16 + 8 + 4 + 2
  uint8_t res = x128;
  res = mul8(res, x64);
  res = mul8(res, x32);
  res = mul8(res, x16);
  res = mul8(res, x8);
  res = mul8(res, x4);
  res = mul8(res, x2);
  return gf28(res);
}

void gf28::mul_vec(const uint8_t* x, const uint8_t* y, uint8_t* z, size_t n)
{
#if defined(__GFNI__)
  size_t i = 0;
  for (; i + 16 <= n; i += 16)
  {
    __m128i xa = _mm_loadu_si128((const __m128i*)(x + i));
    __m128i ya = _mm_loadu_si128((const __m128i*)(y + i));
    __m128i r  = _mm_gf2p8mul_epi8(xa, ya);
    _mm_storeu_si128((__m128i*)(z + i), r);
  }
  for (; i < n; ++i)
    z[i] = mul_software(x[i], y[i]);
#else
  for (size_t i = 0; i < n; ++i)
    z[i] = mul_software(x[i], y[i]);
#endif
}

void gf28::mul_vec_scalar(const uint8_t* x, uint8_t y, uint8_t* z, size_t n)
{
#if defined(__GFNI__)
  __m128i ya = _mm_set1_epi8((char)y);
  size_t i = 0;
  for (; i + 16 <= n; i += 16)
  {
    __m128i xa = _mm_loadu_si128((const __m128i*)(x + i));
    __m128i r  = _mm_gf2p8mul_epi8(xa, ya);
    _mm_storeu_si128((__m128i*)(z + i), r);
  }
  for (; i < n; ++i)
    z[i] = mul_software(x[i], y);
#else
  for (size_t i = 0; i < n; ++i)
    z[i] = mul_software(x[i], y);
#endif
}

uint8_t gf28::dot(const uint8_t* x, const uint8_t* y, size_t n)
{
#if defined(__GFNI__)
  __m128i acc = _mm_setzero_si128();
  size_t i = 0;
  for (; i + 16 <= n; i += 16)
  {
    __m128i xv = _mm_loadu_si128((const __m128i*)(x + i));
    __m128i yv = _mm_loadu_si128((const __m128i*)(y + i));
    __m128i r  = _mm_gf2p8mul_epi8(xv, yv);
    acc = _mm_xor_si128(acc, r);
  }
  // horizontal XOR reduction
  acc ^= _mm_srli_si128(acc, 8);
  acc ^= _mm_srli_si128(acc, 4);
  acc ^= _mm_srli_si128(acc, 2);
  acc ^= _mm_srli_si128(acc, 1);
  uint8_t res = (uint8_t)_mm_cvtsi128_si32(acc);
  for (; i < n; ++i) res ^= mul_software(x[i], y[i]);
  return res;
#else
  uint8_t res = 0;
  for (size_t i = 0; i < n; ++i)
    res ^= mul_software(x[i], y[i]);
  return res;
#endif
}


