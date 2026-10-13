#include "Protocols/MalRepRingOptions.cpp"
#include "Machines/maximal.hpp"

#define RING_SIZE 64

void print_comm(Player& P) {
    NamedCommStats stats = P.total_comm();
    Bundle<octetStream> bundle(P);
    bundle.mine.store(stats.sent);
    P.Broadcast_Receive_no_stats(bundle);
    size_t global = 0;
    for (auto& os : bundle)
        global += os.get_int(8);
    cerr << "Global data sent = " << global / 1048576.0 << " MB (all parties)" << endl;
}

void dabit_benchmark(Player& P) {
    OnlineOptions::singleton.bucket_size = 3;
    OnlineOptions::singleton.batch_size = 1 << 20;

    using T = SpdzWiseRingShare<106, 40>;
    MixedProtocolSetup<T> setup(P);
    MixedProtocolSet<T> set(P, setup);

    auto& prep = set.preprocessing;

    vector<dabit<T>> dbits(OnlineOptions::singleton.batch_size);
    for (int i = 0; i < OnlineOptions::singleton.batch_size; i ++) {
        prep.get_dabit(dbits[i].first, dbits[i].second);
    }

    set.check();
    print_comm(P);
}

void edabit_benchmark(Player& P) {
    OnlineOptions::singleton.bucket_size = 3;
    OnlineOptions::singleton.batch_size = 1000000;
    const vector<pair<int, size_t>> requirements = {
        {10, 2048},
        {12, 20121577},
        {31, 20123625}
    };

    using T = SpdzWiseRingShare<64, 40>;
    MixedProtocolSetup<T> setup(P);
    MixedProtocolSet<T> set(P, setup);

    auto& prep = set.preprocessing;

    for (auto& [n_bits, size] : requirements) {
        vector<edabit<T>> eb(size);
        if (size < 1000000) {
            OnlineOptions::singleton.batch_size = size;
        } else {
            OnlineOptions::singleton.batch_size = 1000000;
        }
        RunningTimer timer;
        for (size_t i = 0; i < size; i ++) {
            try {
                prep.get_edabit_no_count(true, n_bits, eb[i]);
            } catch (out_of_range& e) {
                cout << "Error: " << e.what() << endl;
                cout << "i = " << i << ", n_bits = " << n_bits << ", size = " << size << endl;
                throw e;
            }
        }
        timer.stop();
        cout << "Generating " << size << " edabits of size " << n_bits << " takes " << timer.elapsed() << " seconds." << endl;
    }
    set.check();
    print_comm(P);
}

/*
    For each multiplication:
        Offline: P0 -> P2 three elements.
        Online: P1 <-> P2 one element.
    
    Global 5 elements per semi multiplication.
*/

void randbit_benchmark(Player& P) {
    OnlineOptions::singleton.bucket_size = 3;
    OnlineOptions::singleton.batch_size = 1048576;
    OnlineOptions::singleton.q_size = 16;
    OnlineOptions::singleton.lambda = 40;


    size_t number = 865311779;

    using T = SpdzWiseRingShare<64, 46>;
    MixedProtocolSetup<T> setup(P);
    MixedProtocolSet<T> set(P, setup);

    auto& prep = set.preprocessing;

    vector<T> randbits(OnlineOptions::singleton.batch_size);
    SeededPRNG local_prng;

    Timer timer;
    timer.start();

    for (size_t _ = 0; _ < ceil(number / (double) OnlineOptions::singleton.batch_size); _++) {
        for (int i = 0; i < OnlineOptions::singleton.batch_size; i ++) {
            randbits[i] = prep.get_bit();
        }
    }
    
    set.check();
    timer.stop();

    cout << "Generate " << number << " random bits takes " << timer.elapsed() << " seconds." << endl;
    print_comm(P);
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <playerno>" << endl;
        exit(1);
    }

    int port_base = 12345;
    int my_number = atoi(argv[1]);
    Names N(my_number, 3, "127.0.0.1", port_base);
    CryptoPlayer P(N);
    OnlineOptions::singleton.verbose = true;

    randbit_benchmark(P);
    return 0;
}