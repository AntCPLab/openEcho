#include "Protocols/ProtocolSet.h"
#include "Protocols/ProtocolSetup.h"

#include "Protocols/AstraShare.h"
#include "GC/AstraSecret.h"
#include "GC/EchoSecret.h"

#include "Networking/MyCryptoPlayer.h"

#include "Processor/Machine.hpp"
#include "Processor/RingMachine.hpp"
#include "Protocols/ReplicatedMC.hpp"
#include "Protocols/AstraInput.hpp"
#include "Protocols/AstraPrep.hpp"
#include "Protocols/Astra.hpp"
#include "Protocols/Trio.hpp"
#include "GC/ShareSecret.hpp"

#include "Protocols/EchoShare.h"

int main(int argc, const char** argv) {
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <playerno>" << endl;
        exit(1);
    }

    int my_number = atoi(argv[1]);
    int port_base = 12345;

    Names N(my_number, 3, "localhost", port_base);
    MyCryptoPlayer P(N);

    using T = GC::EchoSecret<EchoShare<BitVec>>;
    BinaryProtocolSetup<T> setup_off(P);
    BinaryProtocolSet<T> set_off(P, setup_off);

    auto& MC = set_off.output;

    T x1 = T::constant(3565201799, P.my_num(), {});
    T y1 = T::constant(2649716257, P.my_num(), {});

    cout << x1 << endl;
    cout << y1 << endl;

    T z1 = x1 + y1;

    vector<T> shares = {x1, y1, z1};
    MC.init_open(P);
    MC.prepare_open(x1);
    MC.prepare_open(y1);
    MC.prepare_open(z1);
    MC.exchange(P);

    cout << "x1 = " << MC.finalize_raw() << endl;
    cout << "y1 = " << MC.finalize_raw() << endl;
    cout << "z1 = " << MC.finalize_raw() << endl;

    MC.Check(P);

    return 0;
}