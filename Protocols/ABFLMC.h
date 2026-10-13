#ifndef PROTOCOLS_ABFLMC_H_
#define PROTOCOLS_ABFLMC_H_

#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/MaliciousRep3Share.h"
#include "Tools/FastVector.h"

template <class T>
class ABFLMC : public MaliciousRepMC<T>
{
    octetStream to_send[2], to_receive[2];
    PointerVector<T> secrets;
    FastVector<array<T, 3>> to_be_checked;

    typedef MaliciousRepMC<T> super;
    HashMaliciousRepMC<T> _impl;

    bool checking;

    vector<array<T, 3>> triple_buffer;
public:

    Player* P;

    explicit ABFLMC(HashMaliciousRepMC<T>& impl) : MaliciousRepMC<T>(), _impl(impl) {}
    ABFLMC() : _impl() {}
    ABFLMC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) : ABFLMC()
    {  
        (void) _; (void) __; (void) ___;
    }

    // emulate Direct_MAC_Check
    ABFLMC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0): ABFLMC()
    { 
        (void) _; (void) __; (void) ___; (void) ____;
    }

    ~ABFLMC()
    {
        if (P) {
            MaybeCheck(*P, true);
        }
    }

    void init(Player& P) {
        this->P = &P;
    }

    ABFLMC& get_part_MC()
    {
        return *this;
    }

    void add_triple(T a, T b, T c)
    {
        to_be_checked.push_back({a, b, c});
    }

    void add_triple(array<T, 3> triple)
    {
        to_be_checked.push_back(triple);
    }

    void add_triples(array<T, 3>* triples, size_t size)
    {
        to_be_checked.resize(to_be_checked.size() + size);
        memcpy(to_be_checked.data() + to_be_checked.size() - size, triples, size * sizeof(array<T, 3>));
    }

    void Check(const Player& P);
    void MaybeCheck(const Player& P, bool final = false);

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
};

#endif