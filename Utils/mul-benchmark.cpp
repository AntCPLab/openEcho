#define ECHO_OFFLINE
#define DATA_RING 52
#define ONLINE_SECURITY 5
#define BASE_RING (DATA_RING + ONLINE_SECURITY)
#define OFFLINE_SECURITY 12

#include "Protocols/MalRepRingOptions.cpp"

#include "Machines/maximal.hpp"

#include "Math/ShortInteger.h"

#include "Protocols/AstraShare.h"
#include "GC/AstraSecret.h"

#include "Protocols/AstraInput.hpp"
#include "Protocols/AstraPrep.hpp"
#include "Protocols/Astra.hpp"
#include "Protocols/AstraMC.hpp"
#include "Protocols/Trio.hpp"

#include "Protocols/EchoShare.h"
#include "Protocols/UssCcsRingShare.h"
#include "Protocols/PostSacriRepRingShare.h"

#include "Protocols/CovertMC.hpp"
#include "Protocols/BGIN19MC.hpp"
#include "Protocols/CovertShare.h"

#include "Networking/MyCryptoPlayer.h"

#include <array>
#include <fstream>
#include <string>
#include <sched.h>

using namespace std;

int N_gates = 1048576;
int R;

template <class T>
void run(Player& P);

void print_mem_usage(int round) {
    ifstream status_file("/proc/self/status");
    string line;
    cout << "[Memory] Round " << round << ": ";
    while (getline(status_file, line)) {
        if (line.find("VmRSS") != string::npos) {
            cout << line << endl;
            break;
        }
    }
}

int main(int argc, const char** argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <playerno> <semi/offline/online/mal/spdz/ccs>" << endl;
        exit(1);
    }

    ez::ezOptionParser opt;
    opt.syntax = "./mul-benchmark.x <playerno> <semi/offline/online/mal/spdz/ccs>\n";
    opt.example = "./mul-benchmark.x 0 offline\n";

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
        "1048576",
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
        "8",
        0,
        1,
        0,
        "CCS compression parameter",
        "-q",
        "--q-size"
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
        to_string(40).c_str(), // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        ("Statistical security parameter (default: " + to_string(40)
                + ")").c_str(), // Help description.
        "-S", // Flag token.
        "--security" // Flag token.
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
    opt.get("-q")->getInt(OnlineOptions::singleton.q_size);
    opt.get("-lm")->getInt(OnlineOptions::singleton.lambda);
    opt.get("-n")->getInt(N_gates);
    OnlineOptions::singleton.live_prep = not opt.get("-F")->isSet;
    opt.get("-S")->getInt(OnlineOptions::singleton.security_parameter);

    OnlineOptions::singleton.options.push_back("nofile");

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

    if (not opt.isSet("-lm") and not opt.isSet("--lambda")) {
        if (protocol == "ccs")
            OnlineOptions::singleton.lambda =
                    UssCcsRingShare<52>::CCS_LAMBDA;
        else if (protocol == "cov-dzkp")
            OnlineOptions::singleton.lambda =
                    DzkpRingShare<52>::CCS_LAMBDA;
    }

    int port_base = 9999;
    int num_players;
    if (protocol == "astra-on") {
        num_players = 2;
    }
    else {
        num_players = 3;
    }
    Names N(my_number, num_players, host, port_base);
    
    if (protocol == "semi") {
        CryptoPlayer P(N);
        run<Rep3Share2<52>>(P);
    }
    else if (protocol == "offline") {
        MyCryptoPlayer P(N);
        // run<EchoPrepShare<ShortInteger<uint8_t>>>(P);
        run<EchoPrepShare2<64>>(P);
    }
    else if (protocol == "online") {
        MyCryptoPlayer P(N);
        // run<EchoShare<ShortInteger<uint8_t>>>(P);
        run<EchoShare2<64>>(P);
    }
    else if (protocol == "mal") {
        CryptoPlayer P(N);
        run<MalRepRingShare<52, 40>>(P);
    }
    else if (protocol == "spdz") {
        CryptoPlayer P(N);
        run<SpdzWiseRingShare<52, 46>>(P);
    }
    else if (protocol == "ccs") {
        CryptoPlayer P(N);
        run<UssCcsRingShare<52>>(P);
    }
    else if (protocol == "astra-off") {
        CryptoPlayer P(N);
        // run<AstraPrepShare<ShortInteger<uint8_t>>>(P);
        run<AstraPrepShare2<52>>(P);
    }
    else if (protocol == "astra-on") {
        CryptoPlayer P(N);
        // run<AstraShare<ShortInteger<uint8_t>>>(P);
        run<AstraShare2<52>>(P);
    }
    else if (protocol == "cov-mac") {
        CryptoPlayer P(N);
        run<CovertMacRingShare<52, 12>>(P);
    }
    else if (protocol == "cov-dzkp") {
        CryptoPlayer P(N);
        run<DzkpRingShare<52>>(P);
    }
    else if (protocol == "ps") {
        CryptoPlayer P(N);
        run<PostSacriRepRingShare<52, 40>>(P);
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
}

void force_synchronize(Player& P) {
    octetStream os[2];
    os[0].reset_write_head();
    P.pass_around(os[0], os[1], 1);
    P.pass_around(os[0], os[1], -1);
}

template<class T>
struct IsDzkpRingShare : false_type
{
};

template<int K>
struct IsDzkpRingShare<DzkpRingShare<K>> : true_type
{
};

template<class T>
void randomizeBenchmarkShare(T& value, GlobalPRNG& prng, int myNumber)
{
    if constexpr (IsDzkpRingShare<T>::value)
    {
        std::array<typename T::clear, 3> shares;
        for (auto& share : shares)
            share.randomize(prng);
        value[0] = shares[myNumber];
        value[1] = shares[(myNumber + 2) % 3];
    }
    else
        value.randomize(prng);
}

template <class T>
void run(Player& P) {

    cout << "Running protocol: " << typeid(T).name() << endl;

    using clear = typename T::clear;
    int n_bits = clear::N_BITS;

    ProtocolSetup<T> setup(P);
    ProtocolSet<T> set(P, setup);

    auto& protocol = set.protocol;
    protocol.set_fast_mode(true);

    GlobalPRNG G(P);

    vector<T> x(N_gates * R), y(N_gates * R);
    for (int i = 0; i < N_gates * R; i++) {
        randomizeBenchmarkShare(x[i], G, P.my_num());
        randomizeBenchmarkShare(y[i], G, P.my_num());
    }

    force_synchronize(P);

    Timer timer;
    Timer cpu_timer(CLOCK_PROCESS_CPUTIME_ID);
    timer.start();
    cpu_timer.start();

    double prepare_counter = 0, exchange_counter = 0, finalize_counter = 0;
    T z;

    for (int _ = 0; _ < R; _ ++) {
        protocol.init_mul();
        double last = timer.elapsed(), now;
        for (int i = 0; i < N_gates; i++)
            protocol.prepare_mul(x[i + _ * N_gates], y[i + _ * N_gates], n_bits);
        now = timer.elapsed();
        prepare_counter += now - last;
        last = now;

        protocol.exchange();
        now = timer.elapsed();
        cout << "\tThis exchange: " << now - last << " seconds" << endl;
        exchange_counter += now - last;
        last = now;

        for (int i = 0; i < N_gates; i++)
            z += protocol.finalize_mul(n_bits);
        now = timer.elapsed();
        finalize_counter += now - last;
    }

    cout << "Prepare costs " << prepare_counter << " seconds" << endl;
    cout << "Exchange costs " << exchange_counter << " seconds" << endl;
    cout << "Finalize costs " << finalize_counter << " seconds" << endl;

    set.check();

    timer.stop();
    cpu_timer.stop();

    print_comm(P);
    cout << "Compute " << N_gates << "*" <<  R << " rounds mul-gates takes " << timer.elapsed() << " seconds" << endl;
    cout << "CPU time: " << cpu_timer.elapsed() << " seconds" << endl;
    cout << "File IO: " << P.total_comm()["Preprocessing transmission"].get_stats().timer.elapsed() << " seconds" << endl;
}