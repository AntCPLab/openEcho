#include "Protocols/ProtocolSet.h"
#include "Protocols/ProtocolSetup.h"

#include "Protocols/AstraShare.h"
#include "GC/AstraSecret.h"

#include "Processor/Machine.hpp"
#include "Processor/RingMachine.hpp"
#include "Protocols/ReplicatedMC.hpp"
#include "Protocols/AstraInput.h"
#include "Protocols/AstraPrep.hpp"
#include "Protocols/Astra.hpp"
#include "Protocols/Trio.hpp"
#include "GC/ShareSecret.hpp"

#include "Protocols/EchoShare.h"


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

    using Off = EchoPrepShare2<64>;

    ProtocolSetup<Off> setup_off(P, 0);
    ProtocolSet<Off> set_off(P, setup_off);

    auto& protocol_prep = set_off.protocol;
    auto& output = set_off.output;

    Off x1 = Off::constant(123);
    Off y1 = Off::constant(456);
    Off x2 = Off::constant(789);
    Off y2 = Off::constant(101112);

    protocol_prep.init_mul();
    protocol_prep.prepare_mul(x1, y1);
    protocol_prep.prepare_mul(x2, y2);
    protocol_prep.exchange();
    Off res1 = protocol_prep.finalize_mul();
    Off res2 = protocol_prep.finalize_mul();
    protocol_prep.init_mul();
    protocol_prep.prepare_mul(res1, res2);
    protocol_prep.exchange();
    Off res = protocol_prep.finalize_mul();

    cout << "Offline multiplication result: " << res[0] << ", " << res[1] << endl;
    output.init_open(P);
    output.prepare_open(res);
    output.exchange(P);
    auto revealed = output.finalize_open();
    cout << "Revealed result: " << revealed << endl;

    set_off.check();

    return 0;
}