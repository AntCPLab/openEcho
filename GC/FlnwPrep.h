#ifndef PROTOCOLS_FLNWPREP_H_
#define PROTOCOLS_FLNWPREP_H_

#include "Protocols/MalRepRingPrep.h"
#include "Protocols/ReplicatedPrep.h"
#include "Protocols/MaliciousRepMC.hpp"
#include "Processor/OnlineOptions.hpp"
#include "Protocols/Replicated.hpp"
#include "GC/ShiftableTripleBuffer.h"
#include <memory>

namespace GC {

template <class T>
class FlnwPrep : public BufferPrep<T>, ShiftableTripleBuffer<T> {

    vector<array<T, 3>> local_triples;
    HashMaliciousRepMC<T> MC;

    Timer prep_timer;

public:
    FlnwPrep(SubProcessor<T>*, DataPositions& usage) :
            BufferPrep<T>(usage) {
    }

    FlnwPrep(DataPositions& usage, int = -1) :
            BufferPrep<T>(usage) {
    }

    ~FlnwPrep() {
    }

    void set_protocol(typename T::Protocol& protocol)
    {
        this->P = &protocol.P;
    }

    void buffer_triples();

    // void get_triples(vector<array<T, 3>>& target_triples, size_t num, int n_bits) {
    //     prep_timer.start();
    //     (void) n_bits;
    //     target_triples.resize(num);
    //     if (this->triples.size() >= num) {
    //         memcpy(target_triples.data(), this->triples.data() + (this->triples.size() - num), num * sizeof(array<T, 3>));
    //         this->triples.resize(this->triples.size() - num);
    //     }
    //     else if (this->triples.size() == 0) {
    //         buffer_num_triples(num, target_triples);
    //     }
    //     else {
    //         size_t batch_size = OnlineOptions::singleton.binary_batch_size;
    //         if (num <= batch_size) {
    //             size_t pre_size = this->triples.size();
    //             memcpy(target_triples.data(), this->triples.data(), pre_size * sizeof(array<T, 3>));
    //             buffer_num_triples(batch_size, this->triples);
    //             memcpy(target_triples.data() + pre_size, this->triples.data() + (batch_size - (num - pre_size)), (num - pre_size) * sizeof(array<T, 3>));
    //             this->triples.resize(batch_size - (num - pre_size));
    //         }
    //         else {
    //             buffer_num_triples(num, target_triples);
    //         }
    //     }
    //     prep_timer.stop();
    // }

    void get_triples(array<T, 3>* target_triples, size_t num, int n_bits);

    array<T, 3> get_triple_no_count(int n_bits)
    {
        return ShiftableTripleBuffer<T>::get_triple_no_count(n_bits);
    }

    void get(Dtype type, T* data)
    {
        BufferPrep<T>::get(type, data);
    }
};

}

#endif