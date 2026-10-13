#ifndef PROTOCOLS_ECHOBINPREP_H_
#define PROTOCOLS_ECHOBINPREP_H_

#include "Protocols/ReplicatedPrep.h"
#include "Protocols/MaliciousRepMC.hpp"
#include "Processor/OnlineOptions.hpp"
#include "Protocols/Replicated.hpp"

#include "RepPrep.hpp"

#include "Tools/FastVector.h"

namespace GC {

template <class T>
class EchoBinPrep : public BufferPrep<T>, ShiftableTripleBuffer<T> {
    using clear = typename T::clear;

    FastVector<clear> random_buffer;
    PointerVector<clear> add_shares;
    PointerVector<T> zeros;
    unique_ptr<ReplicatedBase> prng_protocol;
    unique_ptr<GlobalPRNG> global_prng;

    HashMaliciousRepMC<T> MC;

    unique_ptr<typename T::Protocol> protocol;
    FastVector<array<T, 3>> tmp_triples;

public:
    EchoBinPrep(SubProcessor<T>*, DataPositions& usage) :
            BufferPrep<T>(usage) {
    }

    EchoBinPrep(DataPositions& usage, int = -1) :
            BufferPrep<T>(usage) {
    }

    void set_protocol(typename T::Protocol& protocol) {
        if (not this->protocol) {
            this->protocol = make_unique<typename T::Protocol>(protocol.branch());
        }
        Player& P = protocol.P;
        prng_protocol = make_unique<ReplicatedBase>(P);
        global_prng = make_unique<GlobalPRNG>(P);
        
        this->P = &protocol.P;
    }

    void buffer_triples();

    void get(Dtype type, T* data)
    {
        BufferPrep<T>::get(type, data);
    }

    array<T, 3> get_triple_no_count(int n_bits)
    {
        return ShiftableTripleBuffer<T>::get_triple_no_count(n_bits);
    }

    void get_triples(array<T, 3>* triples, size_t num, int n_bits);
};

}

#endif