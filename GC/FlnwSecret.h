#ifndef GC_FLNWSECRET_H
#define GC_FLNWSECRET_H

#include "Protocols/FlnwProtocol.hpp"
#include "MaliciousRepSecret.h"
#include "GC/FlnwPrep.hpp"

namespace GC {

class FlnwSecret : public MalRepSecretBase<FlnwSecret>
{
    typedef MalRepSecretBase<FlnwSecret> super;
    typedef FlnwSecret This;

public:
    typedef FlnwProtocol<FlnwSecret> Protocol;
    typedef MaliciousRepMC<FlnwSecret> MC;
    typedef MC MAC_Check;
    typedef HashMaliciousRepMC<FlnwSecret> DefaultMC;
    typedef FlnwPrep<FlnwSecret> LivePrep;

    typedef FlnwSecret small_type;

    static const bool expensive_triples = false;
    static const bool malicious = true;

    FlnwSecret() {}
    template<class T>
    FlnwSecret(const T& other) : super(other) {}

    static MC* new_mc(mac_key_type) { return new DefaultMC; }
    static FlnwSecret get_zero() {
        FlnwSecret res;
        res[0].assign_zero();
        res[1].assign_zero();
        return res;
    }
};

}

#endif