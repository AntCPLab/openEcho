#ifndef GC_ECHOSECRET_H_
#define GC_ECHOSECRET_H_

#include "AstraSecret.h"
#include "Protocols/EchoMC.hpp"
#include "EchoBinPrep.hpp"

template <class T>
class EchoPrepBinProtocol;

namespace GC {

template <class U>
class EchoSecret : public U, public AstraSecretBase<EchoSecret<U>> {
    using This = EchoSecret<U>;
    using super = U;

public:
    using part_type = This;
    using small_type = This;
    
    typedef typename super::template mc_template<This> MC;
    using MAC_Check = MC;

    typedef EchoBinPrep<This> LivePrep;
    typedef typename super::template protocol_template<This> Protocol;
    typedef typename super::template input_template<This> Input;

    static const bool is_real = super::is_real;
    static const bool symmetric = super::symmetric;
    static const int default_length = super::clear::N_BITS;

    static bool real_shares(const Player& P)
    {
        return super::real_shares(P);
    }

    static MC* new_mc(typename super::mac_key_type)
    {
        return new MC;
    }

    static This constant(const typename super::clear& value, int my_num,
            typename super::mac_key_type, int = -1)
    {
        (void) my_num;
        This res;
        res[0] = value;
        res[1] = value;
        return res;
    }

    EchoSecret()
    {
    }
    template <class T>
    EchoSecret(const T& other) :
            super(other)
    {
    }

    void load_clear(int n, const Integer& x)
    {
        this->check_length(n, x);
        (*this)[0] = x;
        (*this)[1] = x;
    }

    BitVec local_mul(const EchoSecret& other) const {
        return (*this)[0] * other.sum() + (*this)[1] * other[0];
    }

    void bitcom(StackedVector<This>& S, const vector<int>& regs)
    {
        plain_bitcom(*this, S, regs);
    }

    void bitdec(StackedVector<This>& S, const vector<int>& regs) const
    {
        plain_bitdec(*this, S, regs);
    }
};
    
}


#endif