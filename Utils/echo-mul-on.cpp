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

    using On = EchoShare2<64>;

    ProtocolSetup<On> setup_on(P, 0);
    ProtocolSet<On> set_on(P, setup_on);

    auto& protocol_prep = set_on.protocol;
    auto& output = set_on.output;

    On x1 = On::constant(123);
    On y1 = On::constant(456);
    On x2 = On::constant(789);
    On y2 = On::constant(101112);

    protocol_prep.init_mul();
    protocol_prep.prepare_mul(x1, y1);
    protocol_prep.prepare_mul(x2, y2);
    protocol_prep.exchange();
    On res1 = protocol_prep.finalize_mul();
    On res2 = protocol_prep.finalize_mul();
    protocol_prep.init_mul();
    protocol_prep.prepare_mul(res1, res2);
    protocol_prep.exchange();
    On res = protocol_prep.finalize_mul();


    cout << "Online multiplication result: " << res[0] << ", " << res[1] << endl;
    // vector<SignedZ2<64>> revealed;
    // vector<On> shares = {res};
    // output.POpen(revealed, shares, P);
    output.init_open(P);
    output.prepare_open(res);
    output.exchange(P);
    auto revealed = output.finalize_open();

    cout << "Revealed result: " << revealed << endl;

    set_on.check();
    
    return 0;
}