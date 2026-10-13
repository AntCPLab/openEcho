#ifndef PROTOCOLS_COVERTMC_HPP_
#define PROTOCOLS_COVERTMC_HPP_

#include "Protocols/CovertMC.h"

template <class T>
inline void shuffle(T& triples, size_t vector_size, PRNG& G) {
    for (int i = vector_size - 1; i > 0; i --) {
        int index = G.get_uint(i + 1);
        swap(triples[i], triples[index]);
    }
}

template <class T>
void CovertABFLMC<T>::Check(const Player& P) {
    _impl.Check(P);

    size_t size = mult_triples.size();
    size_t num_generate = ceil(OnlineOptions::singleton.deterrent * size);

    if (num_generate == 0) {
        mult_triples.clear();
        return ;
    }

    GlobalPRNG global_prng(P);

    auto& shared_prngs = prng_protocol->shared_prngs;

    tmp_triples.resize(2 * num_generate);
    array<octetStream, 2> os;
    os[0].reserve((num_generate + 1) * sizeof(clear));
    random_buffer.reserve((num_generate + 1) * 6);
    add_shares.reserve(num_generate + 1);

    shared_prngs[0].get_octets((octet*)random_buffer.data(), (num_generate + 1) * sizeof(clear) * 3);
    shared_prngs[1].get_octets((octet*)(random_buffer.data() + (num_generate + 1) * 3), (num_generate + 1) * sizeof(clear) * 3);

    clear* ptr1 = random_buffer.data();
    clear* ptr2 = ptr1 + (num_generate + 1) * 3;
    octet* os_ptr = os[0].append((num_generate + 1) * sizeof(clear));
    clear* ptr = (clear*) os_ptr;

    for (size_t i = 0; i < num_generate + 1; i ++) {
        T& x = tmp_triples[i][0];
        T& y = tmp_triples[i][1];
        T& z = tmp_triples[i][2];

        x[0] = ptr1[0]; y[0] = ptr1[1];
        x[1] = ptr2[0]; y[1] = ptr2[1];

        ptr[i] = x[0] * (y[0] + y[1]) + x[1] * y[0] + ptr1[2] - ptr2[2];
        ptr1 += 3; ptr2 += 3;
        z[0] = ptr[i];
    }

    P.pass_around(os[0], os[1], 1);
    ptr1 = (clear*) os[1].get_data();

    for (size_t i = 0; i < num_generate + 1; i ++) {
        T& z = tmp_triples[i][2];
        z[1] = ptr1[0];
        ptr1 ++;
    }

    int random_index = global_prng.get_uint((int)(num_generate + 1));
    array<T, 3> open_triple = tmp_triples[random_index];
    tmp_triples.erase(random_index);

    shuffle(mult_triples, mult_triples.size(), global_prng);

    shares.resize(3 + 2 * num_generate);
    opens.reserve(3 + 2 * num_generate);

    shares[2 * num_generate] = open_triple[0];
    shares[2 * num_generate + 1] = open_triple[1];
    shares[2 * num_generate + 2] = open_triple[2];

    for (size_t i = 0; i < num_generate; i ++) {
        shares[i * 2] = mult_triples[i][0] - tmp_triples[i][0];
        shares[i * 2 + 1] = mult_triples[i][1] - tmp_triples[i][1];
    }

    POpen(opens, shares, P);

    if (opens[2 * num_generate] * opens[2 * num_generate + 1] != opens[2 * num_generate + 2]) {
        throw runtime_error("CovertABFLMC: Opened values do not match");
    }

    zeros.resize(num_generate);

    for (size_t i = 0; i < num_generate; i ++) {
        const auto& [x, y, z] = mult_triples[i];
        const auto& [a, b, c] = tmp_triples[i];
        const auto& rho = opens[i * 2];
        const auto& sigma = opens[i * 2 + 1];

        zeros[i] = rho * b + sigma * a + T::constant(rho * sigma, P.my_num(), {}) + c - z;
    }

    this->CheckFor(0, zeros, P);

    mult_triples.clear();
}

#endif