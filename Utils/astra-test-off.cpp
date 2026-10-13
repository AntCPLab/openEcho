#include "Protocols/ProtocolSet.h"
#include "Protocols/ProtocolSetup.h"

#include "Protocols/AstraShare.h"
#include "GC/AstraSecret.h"

#include "Processor/Machine.hpp"
#include "Processor/RingMachine.hpp"
#include "Protocols/ReplicatedMC.hpp"
#include "Protocols/AstraInput.hpp"
#include "Protocols/AstraPrep.hpp"
#include "Protocols/Astra.hpp"
#include "Protocols/Trio.hpp"
#include "GC/ShareSecret.hpp"


// #include "Machines/maximal.hpp"

int main(int argc, const char** argv) {
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <playerno>" << endl;
        exit(1);
    }

    int my_number = atoi(argv[1]);
    int port_base = 12345;

    Names N(my_number, 3, "127.0.0.1", port_base);
    CryptoPlayer P(N);

    using Off = AstraPrepShare2<64>;

    ProtocolSetup<Off> setup_off(P, 0);
    ProtocolSet<Off> set_off(P, setup_off);

    auto& protocol_prep = set_off.protocol;

    Off x(123);
    Off y(456);

    protocol_prep.init_mul();
    protocol_prep.prepare_mul(x, y);
    protocol_prep.exchange();
    Off res = protocol_prep.finalize_mul();

    cout << "Offline multiplication result: " << res[0] << ", " << res[1] << endl;
    
    return 0;
}