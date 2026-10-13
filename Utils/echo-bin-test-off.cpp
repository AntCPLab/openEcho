#include "Protocols/ProtocolSet.h"
#include "Protocols/ProtocolSetup.h"

#include "Protocols/AstraShare.h"
#include "GC/AstraSecret.h"
#include "GC/EchoSecret.h"

#include "Networking/MyCryptoPlayer.h"

#include "Processor/Machine.hpp"
#include "Processor/RingMachine.hpp"
#include "Protocols/ReplicatedMC.hpp"
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
    MyCryptoPlayer P(N);

    using Off = GC::EchoSecret<EchoPrepShare<BitVec>>;

    BinaryProtocolSetup<Off> setup(P);
    DataPositions usage(P.num_players());
    typename Off::LivePrep prep(usage);
    GC::ShareThread<Off> thread(prep, P, setup.get_mac_key());
    typename Off::MAC_Check& output(*thread.MC);
    typename Off::Protocol& protocol(*thread.protocol);
    typename Off::Input input(output, prep, P, &protocol);

    Off x1 = Off::constant(3565201799, P.my_num(), {});
    Off y1 = Off::constant(2649716257, P.my_num(), {});
    Off x2 = Off::constant(2631361689, P.my_num(), {});
    Off y2 = Off::constant(3285601113, P.my_num(), {});

    protocol.init_mul();
    protocol.prepare_mul(x1, y1, 32);
    protocol.prepare_mul(x2, y2, 32);
    protocol.exchange();
    Off res1 = protocol.finalize_mul();
    Off res2 = protocol.finalize_mul();
    protocol.init_mul();
    protocol.prepare_mul(res1, res2, 32);
    protocol.exchange();
    Off res = protocol.finalize_mul();

    cout << "Offline multiplication result: " << res[0] << ", " << res[1] << endl;
    vector<typename Off::clear> revealed;
    vector<Off> shares = {res};
    output.POpen(revealed, shares, P);
    cout << "Revealed result: " << revealed[0] << endl;

    protocol.check();
    output.Check(P);

    return 0;
}