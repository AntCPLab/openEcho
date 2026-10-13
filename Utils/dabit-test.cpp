#include "Protocols/MalRepRingOptions.cpp"
#include "Machines/maximal.hpp"
#include "Networking/MyCryptoPlayer.h"

#include "Protocols/Trio.hpp"
#include "Protocols/EchoShare.h"

#include <fstream>
#include <string>
#include <sched.h>

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <player no>" << endl;
        return 1;
    }

    int my_num = atoi(argv[1]);
    Names N(my_num, 3, "localhost", 9999);
    CryptoPlayer P(N);

    // OnlineOptions::singleton.code_locations = true;

    using T = EchoPrepShare2<64>;

    ProtocolSetup<T> setup(P);
    ProtocolSet<T> set(P, setup);

    auto& prep = set.preprocessing;

    T a;
    typename T::bit_type b;
    prep.get_dabit(a, b);
    cout << "Received a: " << a << endl;
    cout << "Received b: " << b << endl;

    set.check();

    return 0;
}