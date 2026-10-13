#include "Tools/Hash.h"
#include "Tools/octetStream.h"
#include "Tools/time-func.h"
#include "Math/BitVec.h"
#include "Math/Z2k.hpp"
#include "Tools/FastVector.h"
#include "Math/gf28.h"

template<>
inline void IntBase<unsigned short>::randomize(PRNG& G)
{
  a = (G.get_uchar() << 8) | G.get_uchar();
}

template<>
inline void IntBase<unsigned int>::randomize(PRNG& G)
{
  a = G.get_uint();
}

template<>
inline void IntBase<unsigned long>::randomize(PRNG& G)
{
  a = G.get_word();
}

void hash_test(size_t N = 1048576) {
    Hash hash;
    Blake3 blake3;
    const size_t N_BYTES = N;
    octetStream os;
    os.append_random(N_BYTES);

    RunningTimer timer;
    hash.update(os);
    timer.stop();
    cout << "Hashing " << N_BYTES << " bytes took " << timer.elapsed() << " seconds" << endl;

    timer.reset();
    timer.start();
    blake3.update(os);
    timer.stop();

    cout << "Blake3 hashing " << N_BYTES << " bytes took " << timer.elapsed() << " seconds" << endl;
}

template <class T>
void pack_test(size_t N = 1048576) {
    octetStream os;
    os.reset_write_head();

    // using T = Z2<64>;
    PRNG G;
    const octet seed[SEED_SIZE] = "deadbeefdeadbee";
    G.SetSeed(seed);

    vector<T> data(N);
    for (size_t i = 0; i < N; i ++) {
        data[i].randomize(G);
    }

    os.reserve(N * sizeof(T));
    RunningTimer timer;
    for (size_t i = 0; i < N; i ++) {
        data[i].pack(os);
    }
    timer.stop();

    cout << "Pack " << N << " Z64 costs " << timer.elapsed() << " seconds." << endl;
    cout << os.hash() << ", " << os.get_length() << endl;
}

template <class T>
void pack_test2(size_t N = 1048576) {
    octetStream os;
    os.reset_write_head();

    PRNG G;
    const octet seed[SEED_SIZE] = "deadbeefdeadbee";
    G.SetSeed(seed);

    octet* data = new octet[N * T::size()];
    G.get_octets(data, N * T::size());
    os.reserve(N * T::size());

    RunningTimer timer;
    os.append_no_resize(data, N * T::size());
    timer.stop();

    cout << "Pack " << N << " octets costs " << timer.elapsed() << " seconds." << endl;
    cout << os.hash() << ", " << os.get_length() << endl;

    delete[] data;
}

template <class T>
void pack_test3(size_t N = 1048576) {
    octetStream os;
    os.reset_write_head();

    PRNG G;
    const octet seed[SEED_SIZE] = "deadbeefdeadbee";
    G.SetSeed(seed);

    octet* ptr = os.append(N * T::size());
    vector<T> data(N);
    for (size_t i = 0; i < N; i ++) {
        data[i].randomize(G);
    }

    RunningTimer timer;
    for (size_t i = 0; i < N; i ++) {
        memcpy(ptr + i * T::size(), data[i].get_ptr(), T::size());
    }

    timer.stop();

    cout << "Pack " << N << " octets costs " << timer.elapsed() << " seconds." << endl;
    cout << os.hash() << ", " << os.get_length() << endl;
}

template <class T>
void unpack_test(size_t N = 1048576) {
    octetStream os;
    os.reset_write_head();

    SeededPRNG G;

    os.append_random(N * sizeof(T));
    vector<T> data;

    os.reserve(N * sizeof(T));
    RunningTimer timer;
    for (size_t i = 0; i < N; i ++) {
        data[i].unpack(os);
    }
    timer.stop();

    T a = 0;
    for (size_t i = 0; i < N; i ++) {
        a += data[i];
    }

    cout << "Unpack " << N << " BitVec costs " << timer.elapsed() << " seconds." << endl;
    cout << a << endl;
}

template <class T>
void random_test(size_t N = 1048576) {
    // using T = Z2<55>;
    SeededPRNG G;

    T tot = 0;
    RunningTimer timer;
    for (size_t i = 0; i < N; i ++) {
        tot += G.get<T>();
    }
    timer.stop();
    cout << tot << endl;

    cout << "Random " << N << " " << typeid(T).name() << "costs " << timer.elapsed() << " seconds." << endl;
}

template <class T>
void mul_test(size_t N = 1048576) {
    SeededPRNG G;
    vector<T> x(N), y(N), z(N);

    for (size_t i = 0; i < N; i ++) {
        x[i] = G.get_uchar();
        y[i] = G.get_uchar();
    }

    RunningTimer timer;
    for (size_t i = 0; i < N; i ++) {
        z[i] = x[i] * y[i];
    }
    timer.stop();

    T z1 = 0;
    for (size_t i = 0; i < N; i ++) {
        z1 += z[i];
    }

    cout << z1 << endl;

    cout << "Multiply " << N << " pair of Z2^" << sizeof(T) * 8 << " costs " << timer.elapsed() << " seconds." << endl;
}

void mul_vec_test_gf28(size_t N = 1 << 24) {
    SeededPRNG G;
    vector<uint8_t> x(N), y(N), z(N);
    G.get_octets((octet*)x.data(), N);
    G.get_octets((octet*)y.data(), N);

    RunningTimer timer;
    gf28::mul_vec(x.data(), y.data(), z.data(), N);
    timer.stop();

    uint8_t acc = 0;
    for (size_t i = 0; i < N; i ++) acc ^= z[i];
    cout << (unsigned)acc << endl;
    cout << "Multiply (vec) " << N << " pair of GF(2^8) costs " << timer.elapsed() << " seconds." << endl;
}

template <class T>
void and_test(size_t N = 1048576) {
    SeededPRNG G;
    vector<T> x(N), y(N), z(N);

    for (size_t i = 0; i < N; i ++) {
        x[i].randomize(G);
        y[i].randomize(G);
    }

    RunningTimer timer;
    for (size_t i = 0; i < N; i ++) {
        z[i] = x[i] & y[i];
    }
    timer.stop();

    T z1 = 0;
    for (size_t i = 0; i < N; i ++) {
        z1 ^= z[i];
    }

    cout << z1 << endl;

    cout << "And " << N << " pair of Z2^" << sizeof(T) * 8 << " costs " << timer.elapsed() << " seconds." << endl;
}

void and_test(size_t N = 1048576) {
    using T = BitVec;
    SeededPRNG G;
    vector<T> x(N), y(N), z(N);

    for (size_t i = 0; i < N; i ++) {
        x[i].randomize(G);
        y[i].randomize(G);
    }

    RunningTimer timer;
    for (size_t i = 0; i < N; i ++) {
        z[i] = x[i] * y[i];
    }
    timer.stop();

    T z1 = 0;
    for (size_t i = 0; i < N; i ++) {
        z1 += z[i];
    }

    cout << z1 << endl;

    cout << "And " << N << " pair of BitVec costs " << timer.elapsed() << " seconds." << endl;
}

void xor_test(size_t N = 1048576) {
    using T = BitVec;
    SeededPRNG G;
    vector<T> x(N), y(N), z(N);

    for (size_t i = 0; i < N; i ++) {
        x[i].randomize(G);
        y[i].randomize(G);
    }

    RunningTimer timer;
    for (size_t i = 0; i < N; i ++) {
        z[i] = x[i] + y[i];
    }
    timer.stop();

    T z1 = 0;
    for (size_t i = 0; i < N; i ++) {
        z1 += z[i];
    }

    cout << z1 << endl;

    cout << "Xor " << N << " pair of BitVec costs " << timer.elapsed() << " seconds." << endl;
}

void push_back_test(size_t N = 1048576) {
    using T = BitVec;
    SeededPRNG G;
    T data = G.get<T>();

    RunningTimer timer;
    vector<T> data2(N / 2);
    for (size_t i = 0; i < N; i ++) {
        data2.push_back(data);
    }

    timer.stop();

    cout << "Push-back " << N << " BitVec costs " << timer.elapsed() << " seconds." << endl;
}

void array_test(size_t N = 1048576) {
    using T = BitVec;
    SeededPRNG G;
    T data = G.get<T>();

    RunningTimer timer;
    T* data2 = new T[N];
    for (size_t i = 0; i < N; i ++) {
        data2[i] = data;
    }
    timer.stop();

    cout << "Array " << N << " BitVec costs " << timer.elapsed() << " seconds." << endl;
}

void fast_vector_test(size_t N = 1048576) {
    using T = BitVec;
    SeededPRNG G;
    T data = G.get<T>();

    RunningTimer timer;
    FastVector<T> data2(N / 2);
    for (size_t i = 0; i < N; i ++) {
        data2.push_back(data);
    }
    timer.stop();

    cout << "FastVector " << N << " BitVec costs " << timer.elapsed() << " seconds." << endl;
}

void EchoP0(size_t N = 1048576) {
    using T = Z2<64>;
    SeededPRNG G;
    vector<T> mul_x(3 * N), mul_y(3 * N);
    T z;
    vector<T> randoms(6 * N);
    vector<T> pack_data(3 * N);
    FastVector<T> fast_vector(6 * N);

    octetStream os;

    for (size_t i = 0; i < 3 * N; i ++) {
        mul_x[i].randomize(G);
        mul_y[i].randomize(G);
    }
    for (size_t i = 5 * N; i < 6 * N; i ++) {
        randoms[i].randomize(G);
    }
    for (size_t i = 0; i < 3 * N; i ++) {
        pack_data[i].randomize(G);
    }

    double now, last = 0;
    RunningTimer timer;

    for (size_t i = 0; i < 3 * N; i ++) {
        z += mul_x[i] * mul_y[i];
    }

    now = timer.elapsed();
    cout << "Mul costs " << (now - last) << " seconds." << endl;
    last = now;

    for (size_t i = 0; i < 5 * N; i ++) {
        randoms[i].randomize(G);
    }

    now = timer.elapsed();
    cout << "Random costs " << (now - last) << " seconds." << endl;
    last = now;

    os.reserve(5 * N * T::size());

    for (size_t i = 0; i < 3 * N; i ++) {
        pack_data[i].pack(os);
    }

    now = timer.elapsed();
    cout << "Pack costs " << (now - last) << " seconds." << endl;
    last = now;

    for (size_t i = 0; i < 6 * N; i ++) {
        fast_vector.push_back(randoms[i]);
    }

    timer.stop();
    now = timer.elapsed();
    cout << "FastVector push_back costs " << (now - last) << " seconds." << endl;

    cout << "EchoP0: " << timer.elapsed() << " seconds." << endl;

    for (size_t i = 0; i < 6 * N; i ++) {
        z += randoms[i];
    }
    cout << z << os.hash() << endl;
    Hash hash;
    hash.update(fast_vector.data(), fast_vector.size() * T::size());
    cout << "FastVector hash: " << hash.final() << endl;
}

void AstraP0(size_t N = 1048576) {
    using T = Z2<64>;
    SeededPRNG G;
    vector<T> mul_x(N), mul_y(N);
    T z;
    vector<T> randoms(3 * N);
    vector<T> pack_data(N);
    octetStream os;

    for (size_t i = 0; i < N; i ++) {
        mul_x[i].randomize(G);
        mul_y[i].randomize(G);
    }

    double now, last = 0;
    RunningTimer timer;

    for (size_t i = 0; i < N; i ++) {
        z += mul_x[i] * mul_y[i];
    }

    now = timer.elapsed();
    cout << "Mul costs " << (now - last) << " seconds." << endl;
    last = now;

    for (size_t i = 0; i < 3 * N; i ++) {
        randoms[i].randomize(G);
    }

    now = timer.elapsed();
    cout << "Random costs " << (now - last) << " seconds." << endl;
    last = now;

    os.reserve(N * T::size());
    for (size_t i = 0; i < N; i ++) {
        pack_data[i].pack(os);
    }

    now = timer.elapsed();
    timer.stop();
    cout << "Pack costs " << (now - last) << " seconds." << endl;

    cout << "AstraP0: " << timer.elapsed() << " seconds." << endl;

    for (size_t i = 0; i < 3 * N; i ++) {
        z += randoms[i];
    }
    cout << z << os.hash() << endl;
}

void bool_test(size_t N = 1048576) {
    SeededPRNG G;
    vector<bool> x(N), y(N), z(N);

    for (size_t i = 0; i < N; i ++) {
        x[i] = G.get_bit();
        y[i] = G.get_bit();
    }

    RunningTimer timer;
    for (size_t i = 0; i < N; i ++) {
        z[i] = (x[i] * y[i]);
    }
    timer.stop();

    bool z1 = 0;
    for (size_t i = 0; i < N; i ++) {
        z1 ^= z[i];
    }

    cout << z1 << endl;

    cout << "And " << N << " pair of bits costs " << timer.elapsed() << " seconds." << endl;
}

int main() {
    size_t N = 1 << 24;

    // Specialized GF(2^8)
    // using gf28 = gf2n_short;
    gf28::reset();
    gf28::init_field(8);

    // Scalar per-element benchmark
    mul_test<gf28>(N);

    // Vectorized benchmark to exploit GFNI parallelism
    mul_vec_test_gf28(N);
    mul_test<Z2<64>>(N);

    return 0;
}