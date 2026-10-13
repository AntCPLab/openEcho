#ifndef PROTOCOLS_FLNWPROTOCOL_H_
#define PROTOCOLS_FLNWPROTOCOL_H_

using namespace std;

#include "Replicated.h"
#include "Processor/Data_Files.h"
#include "Tools/CodeLocations.h"
#include "Tools/FastVector.h"

template <class T>
class FlnwProtocol : public ReplicatedBase, public ProtocolBase<T> {
    using clear = typename T::clear;
    array<octetStream, 2> os;
    IteratorVector<array<T, 2>> mult_shares;
    IteratorVector<T> results;
    vector<array<T, 3>> triples;

    Preprocessing<T>* prep;
    typename T::MAC_Check* MC;

    unsigned bit_length = 0;

public:
    static const bool uses_triples = true;

    Player& P;
    FlnwProtocol(Player& P) : ReplicatedBase(P), prep(0), P(P) {
        mult_shares.reserve(1 << 20);
    }

    void init(Preprocessing<T>& prep, typename T::MAC_Check& MC) {
        this->prep = &prep;
        this->MC = &MC;
        this->prep->set_protocol(*this);
    }

    typename T::Protocol branch() {
        typename T::Protocol res(P);
        res.prep = prep;
        res.MC = MC;
        res.init_mul();
        return res;
    }

    void init_mul() {
        mult_shares.clear();
        results.clear();

        for (auto& o : os) {
            o.reset_write_head();
        }
    }
    void prepare_mul(const T& x, const T& y, int n = -1) {
        bit_length = max(bit_length, (unsigned) n);
        mult_shares.push_back({x, y});
    }
    void exchange();
    T finalize_mul(int n = -1) {
        (void) n;
        return results.next();
    }

    int get_n_relevant_players() { return 3; }

    double randomness_time() {
        return 0;
    }

    T get_random() {
        return ReplicatedBase::get_random<typename T::open_type>();
    }
};

#endif