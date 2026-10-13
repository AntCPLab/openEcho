#ifndef PROTOCOLS_FLNWPROTOCOL_HPP_
#define PROTOCOLS_FLNWPROTOCOL_HPP_

#include "Protocols/FlnwProtocol.h"

template <class T>
void FlnwProtocol<T>::exchange() {
    size_t round_size = mult_shares.size();
    results.resize(round_size);
    triples.resize(round_size);
    prep->get_triples(triples.data(), round_size, bit_length);

    clear* __restrict os_data = reinterpret_cast<clear*>(os[0].append(round_size * 2 * sizeof(clear)));

    for (size_t i = 0; i < round_size; ++i) {
        // Cache frequently accessed data to improve locality
        const auto& a = triples[i][0];
        const auto& b = triples[i][1];

        // Cache mult_shares data - access both elements together for better cache utilization
        const auto& x = mult_shares[i][0];
        const auto& y = mult_shares[i][1];
        
        // Combine operations to reduce temporary calculations
        const size_t os_idx = i * 2;
        os_data[os_idx] = (x - a)[1];
        os_data[os_idx + 1] = (y - b)[1];
    }

    P.pass_around(os[0], os[1], 1);
    const clear* __restrict recv_data = reinterpret_cast<const clear*>(os[1].get_data());

    int my_num = P.my_num();

    const auto cached_alphai = MC->get_alphai();
    for (size_t i = 0; i < round_size; ++i) {
        auto& result = results[i];

        const auto& a = triples[i][0];
        const auto& b = triples[i][1];
        const auto& c = triples[i][2];

        // Cache mult_shares access
        const auto& x = mult_shares[i][0];
        const auto& y = mult_shares[i][1];
        
        // Pre-compute difference operations to reduce repeated calculations
        const auto x_minus_a = x - a;
        const auto y_minus_b = y - b;
        
        const size_t recv_idx = i * 2;
        const clear rho = recv_data[recv_idx] + x_minus_a.sum();
        const clear sigma = recv_data[recv_idx + 1] + y_minus_b.sum();
        
        // Combine operations to reduce temporary variables and improve instruction pipeline
        const clear rho_sigma = rho * sigma;
        result += rho * b + sigma * a + T::constant(rho_sigma, my_num, cached_alphai) + c;
    }

    results.reset();
    mult_shares.clear();
}

#endif