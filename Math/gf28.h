#ifndef MATH_GF28_H_
#define MATH_GF28_H_

#include <cstdint>
#include <iostream>
using namespace std;

#include "Math/ValueInterface.h"
#include "Math/field_types.h"
#include "Tools/random.h"
#include "Tools/octetStream.h"
#include "Tools/intrinsics.h"

class gf28 : public ValueInterface
{
  uint8_t a;

  static inline uint8_t mul_software(uint8_t x, uint8_t y)
  {
    // GF(2^8) with AES polynomial x^8 + x^4 + x^3 + x + 1 (0x11B)
    uint8_t res = 0;
    while (y)
      {
        if (y & 1)
          res ^= x;
        uint8_t hi = x & 0x80;
        x <<= 1;
        if (hi)
          x ^= 0x1B;
        y >>= 1;
      }
    return res;
  }

public:
  typedef gf28 value_type;
  typedef gf28 next;
  typedef gf28 Scalar;

  static const true_type characteristic_two;
  static const true_type invertible;

  static int length() { return 8; }
  static int size() { return 1; }
  static int size_in_bits() { return 8; }
  static DataFieldType field_type() { return DATA_GF2N; }
  static char type_char() { return '2'; }
  static string type_short() { return "2"; }
  static string type_string() { return "gf28"; }
  static bool allows(Dtype type) { return type <= DATA_INVERSE; }

  static void reset() {}
  static void init_field(int = 8) {}
  static void init_default(int, bool = false) {}
  static void specification(octetStream& os)
  {
    os.store((int)size());
    os.store(length());
  }

  gf28() : a(0) {}
  gf28(uint8_t v) : a(v) {}
  gf28(int v) : a((uint8_t)v) {}
  gf28(unsigned long v) : a((uint8_t)v) {}

  uint8_t get() const { return a; }
  const void* get_ptr() const { return &a; }

  void assign_zero() { a = 0; }
  void assign_one() { a = 1; }
  void assign(const void* ptr) { a = *(const uint8_t*)ptr; }
  void normalize() {}

  int is_zero() const { return a == 0; }
  int is_one() const { return a == 1; }

  // Addition/Subtraction over GF(2): XOR
  void add(const gf28& x, const gf28& y) { a = x.a ^ y.a; }
  void sub(const gf28& x, const gf28& y) { a = x.a ^ y.a; }

  gf28 operator+(const gf28& x) const { return gf28(a ^ x.a); }
  gf28 operator-(const gf28& x) const { return gf28(a ^ x.a); }
  gf28& operator+=(const gf28& x) { a ^= x.a; return *this; }
  gf28& operator-=(const gf28& x) { a ^= x.a; return *this; }

  gf28& mul(const gf28& x, const gf28& y);
  gf28 operator*(const gf28& x) const { gf28 r; return r.mul(*this, x); }
  gf28& operator*=(const gf28& x) { return mul(*this, x); }

  gf28 invert() const;
  gf28 operator/(const gf28& x) const { return *this * x.invert(); }

  // Bitwise ops
  gf28 operator&(const gf28& x) const { return gf28(a & x.a); }
  gf28 operator^(const gf28& x) const { return gf28(a ^ x.a); }
  gf28 operator|(const gf28& x) const { return gf28(a | x.a); }
  gf28 operator~() const { return gf28(~a); }

  void randomize(PRNG& G, int n = -1) { (void)n; a = G.get_uchar(); }
  void almost_randomize(PRNG& G) { randomize(G); }
  void force_to_bit() { a &= 1; }

  void pack(octetStream& o, int n = -1) const { (void)n; o.append((octet*)&a, 1); }
  void unpack(octetStream& o, int n = -1) { (void)n; o.consume((octet*)&a, 1); }

  void output(ostream& s, bool human) const
  {
    if (human) s << hex << (unsigned)a << dec; else s.write((char*)&a, 1);
  }
  void input(istream& s, bool human)
  {
    if (human) { unsigned tmp; s >> hex >> tmp >> dec; a = (uint8_t)tmp; }
    else s.read((char*)&a, 1);
  }

  friend ostream& operator<<(ostream& s, const gf28& x)
  {
    x.output(s, true);
    return s;
  }
  friend istream& operator>>(istream& s, gf28& x)
  {
    x.input(s, true);
    return s;
  }

  bool operator==(const gf28& y) const { return a == y.a; }
  bool operator!=(const gf28& y) const { return a != y.a; }

  // Batch APIs to exploit GFNI parallelism fully
  static void mul_vec(const uint8_t* x, const uint8_t* y, uint8_t* z, size_t n);
  static void mul_vec_scalar(const uint8_t* x, uint8_t y, uint8_t* z, size_t n);

  // Streaming dot product: returns \sum_i x[i]*y[i] in GF(2^8)
  static uint8_t dot(const uint8_t* x, const uint8_t* y, size_t n);
};

#endif /* MATH_GF28_H_ */

