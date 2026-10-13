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
#include "Protocols/EchoInput.h"

#include "Protocols/AstraInput.hpp"
#include "Protocols/AstraPrep.hpp"
#include "Protocols/Astra.hpp"
#include "Protocols/AstraMC.hpp"
#include "Protocols/Trio.hpp"

#include "Protocols/EchoShare.h"
#include "Protocols/UssCcsRingShare.h"

int N = (1 << 20);
int R;

void print_comm(Player& P);

template <class T>
void run(Player& P);

template <class T>
void run2(Player& P);

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
        "1", // Default.
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
    int port_base = 9999;
    int num_players;
    if (protocol == "astra-on" || protocol == "on-mac") {
        num_players = 2;
    }
    else {
        num_players = 3;
    }
    Names N(my_number, num_players, host, port_base);
    
    if (protocol == "astra") {
        using T = GC::AstraSecret<AstraPrepShare<BitVec>>;
        CryptoPlayer P(N);
        run<T>(P);
    }
    else if (protocol == "astra-on") {
        using T = GC::AstraSecret<AstraShare<BitVec>>;
        CryptoPlayer P(N);
        run<T>(P);
    }
    else if (protocol == "off-mac") {
        using T = EchoPrepShare<ShortInteger<uint8_t>>;
        MyCryptoPlayer P2(N);
        run2<T>(P2);
    }
    else if (protocol == "on-mac") {
        using T = EchoShare<ShortInteger<uint8_t>>;
        MyCryptoPlayer P2(N);
        run2<T>(P2);
    }
    else if (protocol == "offline") {
        using T = GC::EchoSecret<EchoPrepShare<BitVec>>;
        MyCryptoPlayer P2(N);
        run<T>(P2);
    }
    else if (protocol == "online") {
        using T = GC::EchoSecret<EchoShare<BitVec>>;
        MyCryptoPlayer P2(N);
        run<T>(P2);
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

template <class T>
void run(Player& P) {
    BinaryProtocolSetup<T> setup(P);
    DataPositions usage(P.num_players());
    typename T::LivePrep prep(usage);
    GC::ShareThread<T> thread(prep, P, setup.get_mac_key());
    typename T::MAC_Check& output(*thread.MC);
    typename T::Protocol& protocol(*thread.protocol);
    typename T::Input input(output, prep, P, &protocol);

    GlobalPRNG G(P);

    vector<T> x(N), y(N);
    for (int i = 0; i < N; i++) {
        x[i].randomize(G);
        y[i].randomize(G);
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
    protocol.check();
    output.Check(P);

    timer.stop();
    print_comm(P);
    cout << "Compute " << N << "*" << R << " 64-bit and-gates takes " << timer.elapsed() << " seconds" << endl;
    cout << "Without file IO: " << timer.elapsed() - P.total_comm()["Preprocessing transmission"].get_stats().timer.elapsed() << " seconds" << endl;
}


template <class T>
void run2(Player& P) {

    cout << "Running protocol: " << typeid(T).name() << endl;

    ProtocolSetup<T> setup(P);
    ProtocolSet<T> set(P, setup);

    auto& protocol = set.protocol;

    GlobalPRNG G(P);

    vector<T> x(N * R), y(N * R);
    for (int i = 0; i < N * R; i++) {
        x[i].randomize(G);
        y[i].randomize(G);
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
        for (int i = 0; i < N; i++)
            protocol.prepare_mul(x[i + _ * N], y[i + _ * N]);
        now = timer.elapsed();
        prepare_counter += now - last;
        last = now;

        protocol.exchange();
        now = timer.elapsed();
        cout << "\tThis exchange: " << now - last << " seconds" << endl;
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

    set.check();

    timer.stop();
    cpu_timer.stop();

    print_comm(P);
    cout << "Compute " << N << "*" <<  R << " and-gates takes " << timer.elapsed() << " seconds" << endl;
    cout << "CPU time: " << cpu_timer.elapsed() << " seconds" << endl;
    cout << "File IO: " << P.total_comm()["Preprocessing transmission"].get_stats().timer.elapsed() << " seconds" << endl;
}