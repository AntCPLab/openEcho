#ifndef MATH_SHORTINTEGER_H_
#define MATH_SHORTINTEGER_H_

#include "Integer.h"
#include "Math/FixedVec.h"

template <class U>
class ShortInteger : public IntBase<U>
{

public:
    typedef ShortInteger value_type;
    typedef ShortInteger clear;
    typedef void Square;

    typedef ShortInteger next;

    static char type_char() { return 'S'; }
    static string type_short() { return "SR"; }
    static DataFieldType field_type() { return DATA_INT; }

    static void reqbl(int n);

    static const false_type invertible;

    static const int N_BITS = sizeof(U) * 8;
    static const int N_BYTES = sizeof(U);
	static const int MAX_EDABITS = N_BITS;
	static const int MAX_N_BITS = N_BITS;
    static const int N_WORDS = (N_BYTES + sizeof(mp_limb_t) - 1)
			/ sizeof(mp_limb_t);
	static const int N_LIMB_BITS = 8 * sizeof(mp_limb_t);

    static int size() { return N_BYTES; }
	static int size_in_limbs() { return N_WORDS; }
	static int size_in_bits() { return size() * 8; }
	static int length() { return n_bits(); }
	static int n_bits() { return N_BITS; }
	static int t() { return 0; }

    template <int X, int L>
    static ShortInteger convert_unsigned(const gfp_<X, L> &other);
    template <int K>
    static ShortInteger convert_unsigned(const Z2<K> &other);

    static ShortInteger power_of_two(bool bit, int exp)
    {
        ShortInteger res;
        res.a = bit ? (U(1 << exp)) : 0;
        return res;
    }

    ShortInteger() { this->a = 0; }
    ShortInteger(const U& a) { this->a = a; }
    ShortInteger(const bigint &x) { *this = (x > 0) ? x.get_ui() : -x.get_ui(); }
    template <int K>
    ShortInteger(const Z2<K> &x) : ShortInteger(x.get_limb(0)) {}
    template <int K>
    ShortInteger(const SignedZ2<K> &x)
    {
        if (K < N_BITS and x.negative())
            this->a = -(-x).get_limb(0);
        else
            this->a = x.get_limb(0);
    }
    //   template<int X, int L>
    //   ShortInteger(const gfp_<X, L>& x);
    ShortInteger(int128 x) : ShortInteger(x.get_lower()) {}
    template <class T>
    ShortInteger(const IntBase<T> &x) : ShortInteger(x.get()) {}
    template <class T>
    ShortInteger(const gf2n_<T> &x) : ShortInteger(x.get()) {}

    ShortInteger(PRNG& G) {
        randomize(G);
    }

    void convert_destroy(bigint &other) { *this = other.get_si(); }

    ShortInteger operator+(const ShortInteger &other) const { return this->a + other.a; }
    ShortInteger operator-(const ShortInteger &other) const { return this->a - other.a; }
    ShortInteger operator*(const ShortInteger &other) const { return this->a * other.a; }
    ShortInteger operator/(const ShortInteger &other) const { return this->a / other.a; }

    ShortInteger operator>>=(const int &other) { this->a >>= other; return *this; }

    bool operator<(const ShortInteger &other) const { return this->a < other.a; }
    bool operator<=(const ShortInteger &other) const { return this->a <= other.a; }
    bool operator>(const ShortInteger &other) const { return this->a > other.a; }
    bool operator>=(const ShortInteger &other) const { return this->a >= other.a; }
    bool operator==(const ShortInteger &other) const { return this->a == other.a; }

    ShortInteger operator+=(const ShortInteger &other) { return this->a += other.a; }
    ShortInteger operator-=(const ShortInteger &other) { return this->a -= other.a; }
    ShortInteger operator*=(const ShortInteger &other) { return this->a *= other.a; }

    ShortInteger lazy_add(const ShortInteger &other) const
    {
        ShortInteger res;
        res.a = this->a + other.a;
        return res;
    }

    ShortInteger lazy_mul(const ShortInteger &other) const
    {
        ShortInteger res;
        res.a = this->a * other.a;
        return res;
    }

    friend unsigned int &operator+=(unsigned int &x, const ShortInteger &other) { return x += other.a; }

    ShortInteger operator-() const { return -(this->a); }

    void pack(octetStream &os, int n_bits = -1) const {
        if (n_bits < 0)
            n_bits = size_in_bits();
        octet* ptr = os.append(size_in_bits() / 8);
        memcpy(ptr, &this->a, size_in_bits() / 8);
    }
    void unpack(octetStream &os, int n_bits = -1) {
        if (n_bits < 0)
            n_bits = size_in_bits();
        octet* ptr = os.consume(n_bits / 8);
        memcpy(&this->a, ptr, n_bits / 8);
    }

    void randomize(PRNG &G, int n_bits = -1);
};

template <class U>
const false_type ShortInteger<U>::invertible;

template<class T, class U, int L>
FixedVec<T, L> operator*(const FixedVec<T, L>& b, const ShortInteger<U>& a)
{
    FixedVec<T, L> res;
    for (size_t i = 0; i < L; i++)
        res[i] = b[i] * a;
    return res;
}

template <>
inline void ShortInteger<unsigned char>::randomize(PRNG &G, int n_bits)
{
    (void) n_bits;
    a = G.get_uchar();
}

template <>
inline void ShortInteger<unsigned short>::randomize(PRNG &G, int n_bits)
{
    (void) n_bits;
    a = (G.get_uchar() << 8) | G.get_uchar();
}

template <>
inline void ShortInteger<unsigned int>::randomize(PRNG &G, int n_bits)
{
    (void) n_bits;
    a = G.get_uint();
}

template <>
inline void ShortInteger<unsigned long>::randomize(PRNG &G, int n_bits)
{
    (void) n_bits;
    a = G.get_word();
}

#endif