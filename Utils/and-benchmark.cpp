#define ECHO_OFFLINE
#define CRYPTOPLAYER
#define BASE_RING 64
#define DATA_RING 52
#define ONLINE_SECURITY 3
#define OFFLINE_SECURITY 12

#include "Machines/maximal.hpp"
#include "Protocols/Rep3Share2k.h"
#include "GC/ShareSecret.h"
#include "GC/MaliciousRepSecret.h"
#include "GC/Uss23Secret.h"
#include "Networking/MyCryptoPlayer.h"

#include "Math/ShortInteger.h"

#include "Protocols/AstraShare.h"
#include "GC/AstraSecret.h"
#include "GC/EchoSecret.h"

#include "Protocols/AstraInput.h"
#include "Protocols/AstraPrep.hpp"
#include "Protocols/Astra.hpp"
#include "Protocols/AstraMC.hpp"
#include "Protocols/Trio.hpp"

#include "Protocols/CovertMC.hpp"
#include "Protocols/BGIN19MC.hpp"
#include "Protocols/CovertShare.h"

#include "Protocols/EchoShare.h"
#include "Protocols/UssCcsRingShare.h"
#include "GC/CovertSecret.h"
#include "Protocols/ABFLRingShare.h"
#include "GC/FlnwSecret.h"

#include <array>

int N = (1 << 20);
int R;

void print_comm(Player& P);
template <class T>
void run(Player& P);

int main(int argc, const char** argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <playerno> <semi/offline/online/mal/uss>" << endl;
        exit(1);
    }

    ez::ezOptionParser opt;
    opt.syntax = "./and-benchmark.x <playerno> <semi/offline/online/mal>\n";
    opt.example = "./and-benchmark.x 0 offline\n";

    opt.add(
        "", 
        0, 
        0, 
        0, 
        "WAN mode (default: false)",
        "-w",
        "--wan"
    );
    opt.add(
        "localhost",
        0,
        1,
        0,
        "IP address of the server",
        "-h",
        "--host"
    );
    opt.add(
        "10000",
        0,
        1,
        0,
        "Batch size",
        "-b",
        "--batch-size"
    );
    opt.add(
        "4",
        0,
        1,
        0,
        "Buckct size",
        "-B",
        "--bucket-size"
    );
    opt.add(
        "1",
        0,
        1,
        0,
        "Rounds",
        "-r",
        "--rounds"
    );
    opt.add(
        "2", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Repeat times in CCS", // Help description.
        "-lm", // Flag token.
        "--lambda" // Flag token.
    );
    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Preprocessing from files", // Help description.
        "-F", // Flag token.
        "--file-preprocessing" // Flag token.
    );
    opt.add(
        "1048576",
        0,
        1,
        0,
        "Number of multiplication gates per round",
        "-n",
        "--num-gates"
    );
    opt.add(
        "1048576", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Batch size for binary ciruit verification", // Help description.
        "-bb", // Flag token.
        "--binary-batchsize" // Flag token.
    );
    opt.add(
        "64", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Max number of proofs per communication", // Help description.
        "-ms", // Flag token.
        "--max-status" // Flag token.
    );

    opt.parse(argc, argv);

    string host;

    if (opt.isSet("-w")) {
        OnlineOptions::singleton.wan = true;
    }
    opt.get("--host")->getString(host);
    opt.get("--batch-size")->getInt(OnlineOptions::singleton.batch_size);
    opt.get("--bucket-size")->getInt(OnlineOptions::singleton.bucket_size);
    opt.get("--rounds")->getInt(R);
    opt.get("-lm")->getInt(OnlineOptions::singleton.lambda);
    OnlineOptions::singleton.live_prep = not opt.get("-F")->isSet;
    opt.get("-n")->getInt(N);
    opt.get("-bb")->getInt(OnlineOptions::singleton.binary_batch_size);

    opt.get("-ms")->getInt(OnlineOptions::singleton.max_status);
    OnlineOptions::singleton.thread_number = 1;


    int my_number;
    string protocol;
    

    if (opt.firstArgs.size() > 1) {
        my_number = atoi(opt.firstArgs[1]->c_str());
        protocol = *(opt.firstArgs[2]);
    }
    else {
        my_number = atoi(argv[1]);
        protocol = argv[2];
    }
    int num_players;
    if (protocol == "astra") {
        num_players = 2;
    }
    else {
        num_players = 3;
    }
    int port_base = 9999;
    Names N(my_number, num_players, host, port_base);
    
    if (protocol == "semi") {
        using T = GC::SemiHonestRepSecret;
        CryptoPlayer P(N);
        run<T>(P);
    }
    else if (protocol == "mal") {
        using T = GC::MaliciousRepSecret;
        CryptoPlayer P(N);
        run<T>(P);
    }
    else if (protocol == "uss") {
        // Parameters used by Li et al. (USENIX Security 2023):
        // lambda = 32 in the optimized first round and 8 afterwards.
        OnlineOptions::singleton.k_size = 32;
        OnlineOptions::singleton.k2_size = 8;
        using T = GC::Uss23Secret;
        CryptoPlayer P(N);
        run<T>(P);
    }
    else if (protocol == "cov-trip") {
        using T = GC::CovertABFLSecret;
        CryptoPlayer P(N);
        run<T>(P);
    }
    else if (protocol == "cov-dzkp") {
        using T = GC::BGIN19Secret;
        CryptoPlayer P(N);
        run<T>(P);
    }
    else if (protocol == "abfl") {
        using T = GC::ABFLSecret;
        CryptoPlayer P(N);
        run<T>(P);
    }
    else {
        cerr << "Unknown protocol: " << protocol << endl;
        exit(1);
    }

    return 0;
}

void print_comm(Player& P) {
    NamedCommStats stats = P.total_comm();
    Bundle<octetStream> bundle(P);
    bundle.mine.store(stats.sent);
    P.Broadcast_Receive_no_stats(bundle);
    size_t global = 0;
    for (auto& os : bundle)
        global += os.get_int(8);
    cerr << "Global data sent = " << global / 1048576.0 << " MB (all parties)" << endl;
    cout << "Transmission time: " << stats["Preprocessing transmission"].get_stats().timer.elapsed() << " seconds" << endl;
}

void force_synchronize(Player& P) {
    octetStream os[2];
    os[0].reset_write_head();
    P.pass_around(os[0], os[1], 1);
    P.pass_around(os[0], os[1], -1);
}

template<class T>
void randomizeReplicatedShare(T& value, GlobalPRNG& prng, int myNumber)
{
    std::array<typename T::clear, 3> shares;
    for (auto& share : shares)
        share.randomize(prng);
    value[0] = shares[myNumber];
    value[1] = shares[(myNumber + 2) % 3];
}

template <class T>
void run(Player& P) {
    BinaryProtocolSetup<T> setup(P);
    BinaryProtocolSet<T> set(P, setup);

    auto& protocol = set.protocol;
    protocol.set_fast_mode(true);

    GlobalPRNG G(P);

    vector<T> x(N), y(N);
    for (int i = 0; i < N; i++) {
        randomizeReplicatedShare(x[i], G, P.my_num());
        randomizeReplicatedShare(y[i], G, P.my_num());
    }
    
    force_synchronize(P);

    Timer timer;
    timer.start();

    double prepare_counter = 0, exchange_counter = 0, finalize_counter = 0;

    for (int _ = 0; _ < R; _ ++) {
        T z;
        protocol.init_mul();
        double last = timer.elapsed(), now;
        for (int i = 0; i < N; i++)
            protocol.prepare_mul(x[i], y[i]);
        now = timer.elapsed();
        prepare_counter += now - last;
        last = now;

        protocol.exchange();
        now = timer.elapsed();
        exchange_counter += now - last;
        last = now;

        for (int i = 0; i < N; i++)
            z += protocol.finalize_mul();
        now = timer.elapsed();
        finalize_counter += now - last;
    }

    cout << "Prepare costs " << prepare_counter << " seconds" << endl;
    cout << "Exchange costs " << exchange_counter << " seconds" << endl;
    cout << "Finalize costs " << finalize_counter << " seconds" << endl;

    cout << "Semi part takes " << timer.elapsed() << " seconds" << endl;
    set.check();

    timer.stop();
    print_comm(P);
    cout << "Compute " << N << "*" << R << " 64-bit and-gates takes " << timer.elapsed() << " seconds" << endl;
    cout << "Without file IO: " << timer.elapsed() - P.total_comm()["Preprocessing transmission"].get_stats().timer.elapsed() << " seconds" << endl;
}