#ifndef PROTOCOLS_ECHOPREP_HPP_
#define PROTOCOLS_ECHOPREP_HPP_

#include "Processor/OnlineOptions.h"
#include "Protocols/EchoPrep.h"

// template <class T>
// inline void partial_shuffle(T& triples, size_t vector_size, size_t shuffle_size, PRNG& G) {
//     for (size_t i = vector_size - shuffle_size; i < vector_size; i ++) {
//         int idx = G.get_uint(i);
//         swap(triples[i], triples[idx]);
//     }
// }

template <class T>
void EchoPrepPrep<T>::buffer_bits() {
    // size_t batch_size = OnlineOptions::singleton.batch_size;
    // size_t batch_size = 10602472;

    if (OnlineOptions::singleton.has_option("nobits")) {
        // Debug memory issue
        this->bits.resize(OnlineOptions::singleton.bit_batch_size);
        return ;
    }

    size_t batch_size = OnlineOptions::singleton.bit_batch_size;
    SeededPRNG local_prng;
    Player& P = *(this->P);
    int my_num = P.my_num();
    auto& protocol = *honest;
    auto& shared_prngs = prng_protocol->shared_prngs;

    auto& bits = this->bits;
    bits.resize(batch_size);

    size_t random_size = batch_size / 8;
    random_buffer.resize(random_size + 1 + batch_size * clear::N_BYTES);
    shares.resize(batch_size * 2);

    for (auto& o : os) {
        o.reset_write_head();
    }

    if (my_num == 0) {
        local_prng.get_octets(random_buffer.data(), random_size + 1);
        shared_prngs[1].get_octets(random_buffer.data() + random_size + 1, batch_size * clear2::N_BYTES);
        clear2* random_ptr = reinterpret_cast<clear2*>(random_buffer.data() + random_size + 1);
        clear2* os_ptr = reinterpret_cast<clear2*>(os[0].append(batch_size * clear2::N_BYTES));
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i][0] = clear2((random_buffer[i / 8] >> (i % 8)) & 1) - random_ptr[i];
            shares[i][1] = random_ptr[i];
            os_ptr[i] = shares[i][0];
        }
        P.exchange(1, os[0], os[1]);
        os_ptr = reinterpret_cast<clear2*>(os[1].get_data());
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i + batch_size][0] = os_ptr[i];
        }
    }
    else if (my_num == 1) {
        local_prng.get_octets(random_buffer.data(), random_size + 1);
        shared_prngs[0].get_octets(random_buffer.data() + random_size + 1, batch_size * clear2::N_BYTES);
        clear2* random_ptr = reinterpret_cast<clear2*>(random_buffer.data() + random_size + 1);
        clear2* os_ptr = reinterpret_cast<clear2*>(os[0].append(batch_size * clear2::N_BYTES));
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i + batch_size][1] = clear2((random_buffer[i / 8] >> (i % 8)) & 1) - random_ptr[i];
            shares[i + batch_size][0] = random_ptr[i];
            os_ptr[i] = shares[i + batch_size][1];
        }
        P.exchange(0, os[0], os[1]);
        os_ptr = reinterpret_cast<clear2*>(os[1].get_data());
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i][1] = os_ptr[i];
        }
    }
    else {
        random_buffer.resize(batch_size * 2 * clear2::N_BYTES);
        shared_prngs[0].get_octets(random_buffer.data(), batch_size * clear2::N_BYTES);
        shared_prngs[1].get_octets(random_buffer.data() + batch_size * clear2::N_BYTES, batch_size * clear::N_BYTES);
        clear2* random_ptr = reinterpret_cast<clear2*>(random_buffer.data());
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i][0] = random_ptr[i];
            shares[i + batch_size][1] = random_ptr[i + batch_size];
        }
    }

    protocol.init_mul();
    for (size_t i = 0; i < batch_size; i ++) {
        protocol.prepare_mul(shares[i] * 2, shares[i + batch_size]);
    }
    protocol.exchange();

    for (size_t i = 0; i < batch_size; i ++) {
        bits[i] = shares[i] + shares[i + batch_size] - protocol.finalize_mul();
    }

    mac_bits.resize(batch_size);
    T2 one = T2::constant(1, my_num, {});

    random_buffer.resize(batch_size * clear2::N_BYTES);
    global_prng->get_octets(random_buffer.data(), batch_size * clear2::N_BYTES);
    clear2* ptr = reinterpret_cast<clear2*>(random_buffer.data());

    protocol.init_dotprod();
    for (size_t i = 0; i < batch_size; i ++) {
        const auto& bit = shares[i];
        protocol.prepare_mul(bit, mac_key);
        protocol.prepare_dotprod(one - bit, bit * ptr[i]);
    }
    protocol.next_dotprod();
    protocol.exchange();
    for (size_t i = 0; i < batch_size; i++) {
        mac_bits[i] = protocol.finalize_mul();
    }
    T2 ip = protocol.finalize_dotprod(batch_size);
    protocol.init_dotprod();
    for (size_t i = 0; i < batch_size; i++) {
        protocol.prepare_dotprod(mac_bits[i] * ptr[i], one - shares[i]);
    }
    protocol.next_dotprod();
    protocol.prepare_mul(ip, mac_key);
    protocol.exchange();
    T2 mac_ip = protocol.finalize_dotprod(batch_size);
    T2 zero = protocol.finalize_mul() - mac_ip;
    (void) zero;
    if (my_num == 2) {
        for (size_t i = 0; i < batch_size; i++) {
            swap(bits[i][0], bits[i][1]);
        }
    }

    os_prep.clear();
    octet* os_prep_ptr = reinterpret_cast<octet*>(os_prep.append(batch_size * sizeof(T)));
    memcpy(os_prep_ptr, bits.data(), sizeof(T) * batch_size);
    echo_protocol->store(os_prep);
}

template <class T>
void EchoPrep<T>::buffer_bits() {
    if (OnlineOptions::singleton.has_option("nobits")) {
        // Debug memory issue
        this->bits.resize(OnlineOptions::singleton.bit_batch_size);
        return ;
    }

    size_t batch_size = OnlineOptions::singleton.bit_batch_size;
    auto& bits = this->bits;
    bits.resize(batch_size);
    if (OnlineOptions::singleton.has_option("nofile")) {
        return ;
    }
    os_prep.reserve(batch_size * sizeof(T));
    echo_protocol->read(os_prep);

    memcpy(bits.data(), os_prep.get_data(), batch_size * sizeof(T));
}

#endif