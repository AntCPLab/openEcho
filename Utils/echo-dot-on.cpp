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

    On x3 = On::constant(131415);
    On y3 = On::constant(161718);
    On x4 = On::constant(192021);
    On y4 = On::constant(222324);

    protocol_prep.init_dotprod();
    protocol_prep.prepare_dotprod(x1, y1);
    protocol_prep.prepare_dotprod(x2, y2);
    protocol_prep.next_dotprod();
    protocol_prep.prepare_dotprod(x3, y3);
    protocol_prep.prepare_dotprod(x4, y4);
    protocol_prep.next_dotprod();

    protocol_prep.exchange();

    On res1 = protocol_prep.finalize_dotprod(2);
    On res2 = protocol_prep.finalize_dotprod(2);

    output.init_open(P);
    output.prepare_open(res1);
    output.prepare_open(res2);
    output.exchange(P);

    cout << "Revealed result1: " << output.finalize_open() << endl;
    cout << "Revealed result2: " << output.finalize_open() << endl;

    set_on.check();
    
    return 0;
}