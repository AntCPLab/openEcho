#include "Machines/maximal.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <playerno>" << endl;
        exit(1);
    }

    int my_number = atoi(argv[1]);
    Names N(my_number, 3, "localhost", 9999);
    CryptoPlayer P(N);

    using T = Rep3Share2<64>;
    ProtocolSetup<T> setup(P, 0);
    ProtocolSet<T> set(P, setup);

    auto& protocol = set.protocol;

    T a = protocol.get_random();
    T b = protocol.get_random();
    T c = protocol.get_random();
    T d = protocol.get_random();

    protocol.init_mul();
    protocol.init_dotprod();

    protocol.prepare_dotprod(c, d);
    protocol.prepare_mul(a, b);
    protocol.next_dotprod();

    protocol.exchange();

    cout << protocol.finalize_mul() << endl;
    cout << protocol.finalize_dotprod(1) << endl;

    return 0;
}