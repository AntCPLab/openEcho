#include "Protocols/ProtocolSet.h"
#include "Protocols/ProtocolSetup.h"

#include "Machines/maximal.hpp"


#include "Tools/time-func.h"
#include "Tools/FastVector.h"

FastVector<Rep3Share2<64>> bits, shares, mac_bits;
FastVector<octet> random_buffer;

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
    P.pass_around_no_stats(os[0], os[1], 1);
    P.pass_around_no_stats(os[0], os[1], -1);
}

void generate_randbit(CryptoPlayer& P, PRNG* global_prng, size_t size) {
    size_t batch_size = size;
    SeededPRNG local_prng;
    int my_num = P.my_num();
    auto protocol = Replicated<Rep3Share2<64>>(P);
    auto& shared_prngs = protocol.shared_prngs;

    bits.resize(batch_size);
    array<octetStream, 2> os;
    size_t random_size = batch_size / 8;
    random_buffer.resize(random_size + 1 + batch_size * 8);
    shares.resize(batch_size * 2);
    mac_bits.resize(batch_size);

    using T2 = Rep3Share2<64>;
    using clear2 = T2::clear;

    T2 mac_key;
    mac_key.randomize(local_prng);

    force_synchronize(P);

    Timer timer;
    timer.start();
    cout << "Timer start." << endl;

    for (auto& o : os) {
        o.reset_write_head();
    }

    if (my_num == 0) {
        local_prng.get_octets(random_buffer.data(), random_size + 1);
        shared_prngs[1].get_octets(random_buffer.data() + random_size + 1, batch_size * clear2::N_BYTES);
        clear2* random_ptr = reinterpret_cast<clear2*>(random_buffer.data() + random_size + 1);
        clear2* os_ptr = reinterpret_cast<clear2*>(os[0].append(batch_size * clear2::N_BYTES));
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i][0] = clear2((random_buffer[i / 8] >> (i % 8)) & 1) - random_ptr[i];
            shares[i][1] = random_ptr[i];
            os_ptr[i] = shares[i][0];
        }
        P.exchange(1, os[0], os[1]);
        os_ptr = reinterpret_cast<clear2*>(os[1].get_data());
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i + batch_size][0] = os_ptr[i];
        }
    }
    else if (my_num == 1) {
        local_prng.get_octets(random_buffer.data(), random_size + 1);
        shared_prngs[0].get_octets(random_buffer.data() + random_size + 1, batch_size * clear2::N_BYTES);
        clear2* random_ptr = reinterpret_cast<clear2*>(random_buffer.data() + random_size + 1);
        clear2* os_ptr = reinterpret_cast<clear2*>(os[0].append(batch_size * clear2::N_BYTES));
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i + batch_size][1] = clear2((random_buffer[i / 8] >> (i % 8)) & 1) - random_ptr[i];
            shares[i + batch_size][0] = random_ptr[i];
            os_ptr[i] = shares[i + batch_size][1];
        }
        P.exchange(0, os[0], os[1]);
        os_ptr = reinterpret_cast<clear2*>(os[1].get_data());
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i][1] = os_ptr[i];
        }
    }
    else {
        random_buffer.resize(batch_size * 2 * clear2::N_BYTES);
        shared_prngs[0].get_octets(random_buffer.data(), batch_size * clear2::N_BYTES);
        shared_prngs[1].get_octets(random_buffer.data() + batch_size * clear2::N_BYTES, batch_size * clear2::N_BYTES);
        clear2* random_ptr = reinterpret_cast<clear2*>(random_buffer.data());
        for (size_t i = 0; i < batch_size; i ++) {
            shares[i][0] = random_ptr[i];
            shares[i + batch_size][1] = random_ptr[i + batch_size];
        }
    }

    protocol.init_mul();
    for (size_t i = 0; i < batch_size; i ++) {
        protocol.prepare_mul(shares[i] * 2, shares[i + batch_size]);
    }
    protocol.exchange();

    for (size_t i = 0; i < batch_size; i ++) {
        bits[i] = shares[i] + shares[i + batch_size] - protocol.finalize_mul();
    }

    T2 one = T2::constant(1, my_num, {});

    random_buffer.resize(batch_size * clear2::N_BYTES);
    global_prng->get_octets(random_buffer.data(), batch_size * clear2::N_BYTES);
    clear2* ptr = reinterpret_cast<clear2*>(random_buffer.data());

    protocol.init_dotprod();
    for (size_t i = 0; i < batch_size; i ++) {
        const auto& bit = shares[i];
        protocol.prepare_mul(bit, mac_key);
        protocol.prepare_dotprod(one - bit, bit * ptr[i]);
    }
    protocol.next_dotprod();
    protocol.exchange();
    for (size_t i = 0; i < batch_size; i++) {
        mac_bits[i] = protocol.finalize_mul();
    }
    T2 ip = protocol.finalize_dotprod(batch_size);
    protocol.init_dotprod();
    for (size_t i = 0; i < batch_size; i++) {
        protocol.prepare_dotprod(mac_bits[i] * ptr[i], one - shares[i]);
    }
    protocol.next_dotprod();
    protocol.prepare_mul(ip, mac_key);
    protocol.exchange();
    T2 mac_ip = protocol.finalize_dotprod(batch_size);
    T2 zero = protocol.finalize_mul() - mac_ip;
    (void) zero;
    if (my_num == 2) {
        for (size_t i = 0; i < batch_size; i++) {
            swap(bits[i][0], bits[i][1]);
        }
    }

    timer.stop();

    cout << "Generate " << size << " random bits takes " << timer.elapsed() << " seconds." << endl;
    print_comm(P);
}

int main(int argc, const char** argv) {
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <playerno>" << endl;
        exit(1);
    }

    int my_number = atoi(argv[1]);
    int port_base = 12345;

    Names N(my_number, 3, "172.25.88.179", port_base);
    CryptoPlayer P(N);

    GlobalPRNG global_prng(P);
    
    generate_randbit(P, &global_prng, 10602472);

    return 0;
}