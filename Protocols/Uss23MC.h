#ifndef PROTOCOLS_USS23MC_H_
#define PROTOCOLS_USS23MC_H_

#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/MaliciousRep3Share.h"

template <class T>
class Uss23MC : public MaliciousRepMC<T>
{
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

    void reset() {
        hash.reset();
        needs_checking = false;
    }
    
    Uss23MC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) : Uss23MC()
    { (void)_; (void)__; (void)___; }
    
    Uss23MC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0) : Uss23MC()
    { (void)_; (void)__; (void)___; (void)____; }

    Uss23MC() {
        reset();
    }
    ~Uss23MC() {
        if (needs_checking)
        {
            cerr << endl << "SECURITY BUG: insufficient checking" << endl;
            terminate();
        }
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

    void Check(const Player& P) {
        if (needs_checking)
        {
            CODE_LOCATION
            vector<octetStream> os(P.num_players());
            hash.final(os[P.my_num()]);
            reset();
            P.Broadcast_Receive(os);
            for (int i = 0; i < P.num_players(); i++)
                if (os[i] != os[P.my_num()]) {
                    // throw mac_fail("check hash mismatch");
                    // cout << "Check hash mismatch." << endl;
                }
        }
    }

    Uss23MC& get_part_MC()
    {
        return *this;
    }
};

#endif