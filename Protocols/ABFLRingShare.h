#ifndef PROTOCOLS_ABFLRINGSHARE_H_
#define PROTOCOLS_ABFLRINGSHARE_H_

#include "ABFLShare.h"

#include "Math/Z2k.hpp"
#include "GC/square64.h"

template<int K>
class ABFLRingShare : public ABFLShare<Z2<K>>
{
    typedef Z2<K> T;
    typedef ABFLRingShare This;

public:
    typedef ABFL<ABFLRingShare> Protocol;
    typedef ABFLMC<ABFLRingShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<ABFLRingShare> Input;
    typedef SpecificPrivateOutput<This> PrivateOutput;
    typedef ReplicatedPrep<ABFLRingShare> LivePrep;
    typedef Replicated<T> Honest;

    typedef ABFLRingShare Scalar;
    typedef SignedZ2<K> clear;

    typedef GC::ABFLSecret bit_type;

    static const bool has_split = false;

    ABFLRingShare()
    {
    }
    template<class U>
    ABFLRingShare(const FixedVec<U, 2>& other)
    {
        FixedVec<T, 2>::operator=(other);
    }
};


#endif