#ifndef GC_ABFLSECRET_H
#define GC_ABFLSECRET_H

#include "Protocols/ABFL.hpp"
#include "MaliciousRepSecret.h"
#include "Protocols/ABFLMC.hpp"

namespace GC {

class ABFLSecret : public MalRepSecretBase<ABFLSecret>
{
    typedef MalRepSecretBase<ABFLSecret> super;
    typedef ABFLSecret This;

public:
    typedef ABFL<ABFLSecret> Protocol;
    typedef ABFLMC<ABFLSecret> MC;
    typedef MC MAC_Check;
    typedef MAC_Check DefaultMC;

    typedef ABFLSecret small_type;

    static const bool expensive_triples = false;
    static const bool malicious = true;

    ABFLSecret() {}
    template<class T>
    ABFLSecret(const T& other) : super(other) {}

    static MC* new_mc(mac_key_type) { return new MC; }
    static ABFLSecret get_zero() {
        ABFLSecret res;
        res[0].assign_zero();
        res[1].assign_zero();
        return res;
    }
};

}

#endif