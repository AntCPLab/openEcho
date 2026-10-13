#ifndef PROTOCOLS_COVERTMC_H_
#define PROTOCOLS_COVERTMC_H_

#include "Protocols/MaliciousRepMC.h"
#include "Tools/FastVector.h"

template <class T>
class CovertABFLMC : public MaliciousRepMC<T> {
    using clear = typename T::clear;
    
    HashMaliciousRepMC<T> _impl;

    FastVector<array<T, 3>> mult_triples;
    FastVector<array<T, 3>> tmp_triples;

    FastVector<clear> random_buffer;
    PointerVector<clear> add_shares, opens;
    PointerVector<T> shares, zeros;

    unique_ptr<ReplicatedBase> prng_protocol;

public:

    Player* P;

    explicit CovertABFLMC(HashMaliciousRepMC<T>& impl) : MaliciousRepMC<T>(), _impl(impl) {}
    CovertABFLMC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) : CovertABFLMC()
    { (void)_; (void)__; (void)___; }
    
    // emulate Direct_MAC_Check
    CovertABFLMC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0) : CovertABFLMC()
    { (void)_; (void)__; (void)___; (void)____; }

    CovertABFLMC() : _impl() {}

    ~CovertABFLMC() {
        Check(*P);
    }

    void init(Player& P) {
        if (!prng_protocol) {
            prng_protocol = make_unique<ReplicatedBase>(P);
        }
        this->P = &P;
    }

    void POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        _impl.POpen(values, S, P);
    }
    void POpen_End(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        _impl.POpen_End(values, S, P);
    }

    virtual typename T::open_type finalize_raw() {
        return _impl.finalize_raw();
    }

    void CheckFor(const typename T::open_type& value, const vector<T>& shares, const Player& P) {
        _impl.CheckFor(value, shares, P);
    }

    void add_triple(const array<T, 3>& triple) {
        mult_triples.push_back(triple);
    }

    void Check(const Player& P);
    void MaybeCheck(const Player& P) {
        if (mult_triples.size() > (size_t) OnlineOptions::singleton.batch_size) {
            Check(P);
        }
    }

    CovertABFLMC& get_part_MC() {
        return *this;
    }
};

#endif