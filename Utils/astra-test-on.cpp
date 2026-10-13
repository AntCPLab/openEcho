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
#include "Protocols/AstraMC.hpp"
#include "Protocols/Trio.hpp"
#include "GC/ShareSecret.hpp"


int main(int argc, const char** argv) {
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <playerno>" << endl;
        exit(1);
    }

    int my_number = atoi(argv[1]);
    int port_base = 12345;

    Names N(my_number, 2, "127.0.0.1", port_base);
    CryptoPlayer P(N);

    using On = AstraShare2<64>;

    ProtocolSetup<On> setup_on(P, 0);
    ProtocolSet<On> set_on(P, setup_on);

    auto& protocol_online = set_on.protocol;

    On x(123);
    On y(456);

    protocol_online.init_mul();
    protocol_online.prepare_mul(x, y);
    protocol_online.exchange();
    On res = protocol_online.finalize_mul();

    cout << "Online multiplication result: " << res[0] << ", " << res[1] << endl;

    return 0;
}