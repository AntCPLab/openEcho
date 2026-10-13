#ifndef PROTOCOLS_ECHOPREP_H_
#define PROTOCOLS_ECHOPREP_H_

#include "Protocols/ReplicatedPrep.h"
#include "Replicated.hpp"

#include <vector>

template <class T>
class EchoPrep : public virtual BufferPrep<T> {
    using clear = typename T::clear;
    unique_ptr<ReplicatedBase> prng_protocol;
    unique_ptr<GlobalPRNG> global_prng;

    typename T::Protocol* echo_protocol;
    octetStream os_prep;
public:
    EchoPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage)
    {
        (void) proc;
    }

    void set_protocol(typename T::Protocol& prot) {
        this->echo_protocol = &prot;
        Player& P = prot.P;
        prng_protocol = make_unique<ReplicatedBase>(P);
        global_prng = make_unique<GlobalPRNG>(P);

        this->P = &prot.P;
    }

    void buffer_bits();
    void get_dabit_no_count(T& a, typename T::bit_type& b)
    {
        if (this->bits.empty()) {
            buffer_bits();
        }
        a = this->bits.back();
        this->bits.pop_back();
        b[0] = a[0].get_bit(0);
        b[1] = a[1].get_bit(0);
        if (this->P->my_num() == 2) {
            swap(b[0], b[1]);
        }
    }

};

template <class T>
class EchoPrepPrep : public virtual BufferPrep<T> {
    using clear = typename T::clear;
    using T2 = Rep3Share2<clear::N_BYTES * 8>;
    using clear2 = typename T2::clear;

    unique_ptr<ReplicatedBase> prng_protocol;
    unique_ptr<GlobalPRNG> global_prng;

    vector<octet> random_buffer;
    vector<clear> mult_shares;
    vector<T2> mac_bits, shares;

    array<octetStream, 2> os;
    octetStream os_prep;

    unique_ptr<Replicated<T2>> honest;
    T2 mac_key;
    typename T::Protocol* echo_protocol;

public:
    EchoPrepPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage)
    {
        (void) proc;
    }

    ~EchoPrepPrep() {}

    void set_protocol(typename T::Protocol& prot) {
        this->echo_protocol = &prot;
        Player& P = prot.P;
        prng_protocol = make_unique<ReplicatedBase>(P);
        global_prng = make_unique<GlobalPRNG>(P);

        this->P = &prot.P;
        if (honest == nullptr) {
            honest = make_unique<Replicated<T2>>(P);
        }
        mac_key = honest->get_random();
    }

    void buffer_bits();

    void get_dabit_no_count(T& a, typename T::bit_type& b)
    {
        if (this->bits.empty()) {
            buffer_bits();
        }
        a = this->bits.back();
        this->bits.pop_back();
        b[0] = a[0].get_bit(0);
        b[1] = a[1].get_bit(0);
        if (this->P->my_num() == 2) {
            swap(b[0], b[1]);
        }
    }
};

#endif