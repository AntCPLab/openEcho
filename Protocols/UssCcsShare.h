#ifndef PROTOCOLS_USSCCSSHARE_H_
#define PROTOCOLS_USSCCSSHARE_H_

#include "Rep3Share.h"
#include "SemiShare.h"
#include "Ccs24.hpp"
#include "Ccs24MC.hpp"
#include "ReplicatedInput.hpp"
#include "GC/Uss23Secret.h"

namespace GC {
    class Uss23Secret;
}

template<class T>
class UssCcsShare : public Rep3Share<T>
{
    typedef Rep3Share<T> super;
    typedef UssCcsShare This;

public:
    typedef Ccs24<UssCcsShare<T>> Protocol;
    typedef Ccs24MC<UssCcsShare<T>> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<UssCcsShare<T>> Input;
    typedef SpecificPrivateOutput<UssCcsShare> PrivateOutput;
    typedef ReplicatedPrep<UssCcsShare> LivePrep;
    typedef Rep3Share<T> Honest;
    typedef T random_type;
    typedef This Scalar;

    typedef GC::Uss23Secret bit_type;

    // indicate security relevance of field size
    typedef T mac_key_type;

    const static bool expensive = true;
    static const bool has_trunc_pr = false;
    static const bool malicious = true;

    static const int OnlineSecurity = 0;
    static constexpr int CCS_RING_GAP = 64;
    static constexpr int CCS_LAMBDA = 40;

    static string type_short()
    {
        return "R" + string(1, T::type_char());
    }
    static string type_string()
    {
        return "UssCcs " + super::type_string();
    }

    UssCcsShare()
    {
    }
    UssCcsShare(const T& other, int my_num, T alphai = {}) :
            super(other, my_num, alphai)
    {
    }
    template<class U>
    UssCcsShare(const U& other) : super(other)
    {
    }

    static UssCcsShare get_zero() {
        UssCcsShare res;
        res.assign_zero();
        return res;
    }
};

#endif