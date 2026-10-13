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

    using T = EchoShare2<64>;

    MixedProtocolSetup<T> setup(P);
    MixedProtocolSet<T> set(P, setup);

    auto& protocol = set.protocol;
    auto& MC = set.output;
    auto& MCb = set.binary.output;
    auto& prep = set.preprocessing;

    using Bit = typename T::bit_type;
    using clear = typename T::clear;
    
    Bit x = Bit::constant(1, my_num, {});
    dabit<T> dabit;
    prep.get_dabit(dabit.first, dabit.second);
    Bit rb = dabit.second;
    T ra = dabit.first;

    MCb.init_open(P);
    MCb.prepare_open(rb + x);
    MCb.prepare_open(rb);
    MCb.exchange(P);
    auto res = MCb.finalize_open(); // x ^ rb
    clear res2 = res;

    T xa = T::constant(0, my_num, {});
    xa += T::constant(res2, my_num, {});    // [x ^ r]^A

    cout << "[x^r]^A = " << xa << endl;
    cout << "Dabit = " << MCb.finalize_open() << endl;

    protocol.init_mul();
    protocol.prepare_mul(xa, 2 * ra);
    protocol.exchange();
    
    T result = xa + ra;
    T result2 = protocol.finalize_mul();

    cout << "2 * x * r = " << result2 << endl;

    result -= result2;

    MC.init_open(P);
    MC.prepare_open(result);
    MC.exchange(P);
    cout << "[x]^A = " << MC.finalize_open() << endl;

    set.check();
    return 0;
}