#ifndef GC_USS23SECRET_H
#define GC_USS23SECRET_H

#include "MaliciousRepSecret.h"
#include "Protocols/Uss23.hpp"
#include "Protocols/Uss23MC.h"
#include "Protocols/ReplicatedInput.h"

namespace GC {

class Uss23Secret : public MalRepSecretBase<Uss23Secret>
{
    typedef MalRepSecretBase<Uss23Secret> super;
    typedef Uss23Secret This;

public:
    typedef Uss23<Uss23Secret> Protocol;
    typedef Uss23MC<Uss23Secret> MC;
    typedef MC MAC_Check;
    typedef MAC_Check DefaultMC;
    typedef ReplicatedInput<This> Input;

    typedef Uss23Secret small_type;
    typedef BitVec clear;
    typedef This bit_type;
    typedef This part_type;

    static const bool expensive_triples = false;
    static const bool malicious = true;

    Uss23Secret() {}
    template<class T>
    Uss23Secret(const T& other) : super(other) {}

    static MC* new_mc(mac_key_type) { return new MC; }
    static Uss23Secret get_zero() {
        Uss23Secret res;
        res[0].assign_zero();
        res[1].assign_zero();
        return res;
    }
};

}

#endif