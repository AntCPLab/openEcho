#ifndef PROTOCOLS_CCS24MC_H_
#define PROTOCOLS_CCS24MC_H_

#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/MaliciousRep3Share.h"
#include "Tools/FastVector.h"

template <class T>
class Ccs24MC : public MaliciousRepMC<T>
{
    using ExtendedOffline = Z2<52 + T::CCS_RING_GAP>;

    octetStream to_send[2], to_receive[2];
    PointerVector<T> secrets;
    array<PRNG, 2> shared_prngs;

    FastVector<ExtendedOffline> rx, ry, rz1, rz2;
    FastVector<size_t> vector_sizes;
    size_t buffer_size;
    
    typedef MaliciousRepMC<T> super;

    Hash hash;
    octetStream os;
    bool needs_checking;
    void update() {
        hash.update(os);
        needs_checking = true;
    }
    void finalize(const vector<typename T::open_type>& values) {
        os.reset_write_head();
        os.reserve(values.size() * T::open_type::size());
        for (auto& value : values)
            value.pack(os);
        update();
    }

public:

    Player* P;

    void reset() {
        hash.reset();
        needs_checking = false;
    }

    Ccs24MC() {
        rx.reserve(1 << 28);
        ry.reserve(1 << 28);
        rz1.reserve(1 << 25);
        rz2.reserve(1 << 25);
        vector_sizes.reserve(1 << 25);
        buffer_size = 0;
    }
    Ccs24MC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0)
        : Ccs24MC()
    {  
        (void) _;
        (void) __;
        (void) ___;
    }

    // emulate Direct_MAC_Check
    Ccs24MC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0): 
        Ccs24MC()
    { 
        (void) _;
        (void) __;
        (void) ___;
        (void) ____;
    }

    ~Ccs24MC()
    {
        if (P) {
            MaybeCheck(*P, true);
        }
    }

    Ccs24MC& get_part_MC()
    {
        return *this;
    }

    void POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        ReplicatedMC<T>::POpen(values, S, P);
        finalize(values);
    }
    void POpen_End(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        ReplicatedMC<T>::POpen_End(values, S, P);
        finalize(values);
    }

    virtual typename T::open_type finalize_raw() {
        auto res = ReplicatedMC<T>::finalize_raw();
        os.reset_write_head();
        res.pack(os);
        update();
        return res;
    }

    void CheckFor(const typename T::open_type& value, const vector<T>& check_shares, const Player& P) {
        os.reset_write_head();
        for (auto& share : check_shares)
        {
            typename T::value_type shares[3];
            for (int i = 0; i < 3; i++)
            {
                int j = (i + P.my_num()) % 3;
                if (j < 2)
                    shares[i] = share[j];
                else
                    shares[i] = value - share.sum();
            }
            for (auto& x : shares)
                x.pack(os);
        }
        update();
        Check(P);
    }

    inline void add_rx(typename T::clear rx_)
    {
        ExtendedOffline rx_data = reinterpret_cast<ExtendedOffline&>(rx_);
        rx.push_back(rx_data);
    }

    inline void add_ry(typename T::clear ry_)
    {
        ExtendedOffline ry_data = reinterpret_cast<ExtendedOffline&>(ry_);
        ry.push_back(ry_data);
    }

    inline void add_rz1(typename T::clear z1_)
    {
        ExtendedOffline z1_data = reinterpret_cast<ExtendedOffline&>(z1_);
        rz1.push_back(z1_data);
    }

    inline void add_rz2(typename T::clear z2_)
    {
        ExtendedOffline z2_data = reinterpret_cast<ExtendedOffline&>(z2_);
        rz2.push_back(z2_data);
    }

    inline void add_size(size_t size)
    {
        vector_sizes.push_back(size);
        buffer_size += size;
    }

    void init(Player& P) {
        octet seed1[16] = "0123456789abcde";
        octet seed2[16] = "edcba9876543210";
        octet seed3[16] = "02468ace13579bd";

        if (P.my_num() == 0) {
            shared_prngs[0].SetSeed(seed1);
            shared_prngs[1].SetSeed(seed3);
        }
        else if (P.my_num() == 1) {
            shared_prngs[0].SetSeed(seed2);
            shared_prngs[1].SetSeed(seed1);
        }
        else {
            shared_prngs[0].SetSeed(seed3);
            shared_prngs[1].SetSeed(seed2);
        }

        this->P = &P;
    }
    
    void Check(const Player& P) {
        MaybeCheck(P, false);
    }
    void MaybeCheck(const Player& P, bool final = false);
};

#endif