#ifndef GC_COVERTSECRET_H_
#define GC_COVERTSECRET_H_

#include "GC/MaliciousRepSecret.h"
#include "Protocols/CovertProtocol.h"
#include "Protocols/CovertMC.h"

#include "Protocols/BGIN19.h"

namespace GC {

class CovertABFLSecret : public MalRepSecretBase<CovertABFLSecret>
{
    typedef CovertABFLSecret This;
    typedef MalRepSecretBase<This> super;

public:
    typedef CovertABFLProtocol<CovertABFLSecret> Protocol;
    typedef CovertABFLMC<CovertABFLSecret> MC;
    typedef MC MAC_Check;
    typedef MC DefaultMC;
    typedef SmallMalRepSecret small_type;

    CovertABFLSecret() {}
    template<class T>
    CovertABFLSecret(const T& other) : super(other) {}

    static MC* new_mc(typename super::mac_key_type)
    {
        return new MC;
    }

};

class BGIN19Secret : public MalRepSecretBase<BGIN19Secret>
{
    typedef BGIN19Secret This;
    typedef MalRepSecretBase<This> super;

public:
    typedef BGIN19Protocol<BGIN19Secret> Protocol;
    typedef BGIN19MC<BGIN19Secret> MC;
    typedef MC MAC_Check;
    typedef MC DefaultMC;
    typedef SmallMalRepSecret small_type;

    BGIN19Secret() {}
    template<class T>
    BGIN19Secret(const T& other) : super(other) {}

    static MC* new_mc(typename super::mac_key_type)
    {
        return new MC;
    }

};

}

#endif