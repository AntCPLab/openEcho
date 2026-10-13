#ifndef PROTOCOLS_USS23_H_
#define PROTOCOLS_USS23_H_

#include "Replicated.hpp"
#include "BinaryCheck.hpp"
#include "Processor/Data_Files.h"

#include <thread>
#include <atomic>
#include <fstream>
#include <chrono>
#include "Tools/SafeQueue.h"

struct StatusData {
    DZKProof proof;
    size_t node_id;
    Field **mask_ss_prev, **mask_ss_next;
    Field input_mask_prev, input_mask_next;
    size_t sz;

    StatusData() {}
    StatusData(DZKProof proof, size_t node_id, Field **mask_ss_prev,
            Field **mask_ss_next, Field input_mask_prev,
            Field input_mask_next, size_t sz) :
        proof(proof), node_id(node_id), mask_ss_prev(mask_ss_prev),
        mask_ss_next(mask_ss_next), input_mask_prev(input_mask_prev),
        input_mask_next(input_mask_next), sz(sz) {}
    
};

using Uss23ShareTypeBlock = MyPair<uint64_t, uint64_t>;

struct ShareTupleBlock {
public:
    Uss23ShareTypeBlock input1, input2, result, rho;

    ShareTupleBlock(): input1(), input2(), result(), rho() {}
};

template <class T>
class Uss23 : public ProtocolBase<T>
{
    typedef Replicated<T> Honest;

    array<octetStream, 2> os;
    typename T::MAC_Check* MC;
    Preprocessing<T>* prep;
    typename T::clear dotprod_share;
    PointerVector<typename T::clear> add_shares;

    PointerVector<ShareTupleBlock> share_tuple_blocks;
    size_t idx_input, idx_rho, idx_result;
    size_t share_tuple_block_size;
    const size_t ZOOM_RATE = 2;

    size_t MAX_LAYER_SIZE = 64000000;

    const Field two_inverse = Mersenne::inverse(2);
    const Field neg_one = Mersenne::PR - 1;
    const Field neg_two = Mersenne::PR - 2;
    const Field neg_two_inverse = Mersenne::neg(two_inverse);

    StatusData *status_queue;
    vector<typename T::open_type> opened;

    PointerVector<typename T::clear> uids;

    std::mutex check_lock;
    size_t check_id;

    WaitQueue<int> cv;

    size_t local_counter, status_counter, status_pointer;
    WaitSize wait_size;
    Field sid;

    vector<std::thread> check_threads, verify_threads;

    array<octetStream, 2> proof_os, vermsg_os;
    size_t verify_index;
    mutex verify_lock;
    VerMsg *vermsgs;
    WaitQueue<u_char> verify_queue;
    WaitSize verify_tag;
    std::atomic<bool> check_passed{true};

    PointerVector<Uss23ShareTypeBlock> shareTypeBlockBuffer;

public:

    mutable array<PRNG, 2> shared_prngs;
    Player& P;
    vector<array<PRNG, 2>> check_prngs;
    PRNG global_prng;

    Uss23(Player& P);
    Uss23(const ReplicatedBase& other): ReplicatedBase(other) {}
    Uss23(Player& P, array<PRNG, 2>& prngs):
        ReplicatedBase(P, prngs) {}
    
    ~Uss23() {
        for (int i = 0; i < OnlineOptions::singleton.thread_number; i ++) {
            cv.push(-1);
        }
        
        for (auto &each_thread: check_threads) {
            each_thread.join();
        }

        if (local_counter > 0) {
            Check_one(status_pointer, local_counter);
            status_counter ++;
        }

        if (status_counter > 0) {
            try {
                verify();
            }
            catch (const std::exception& error) {
                cerr << error.what() << endl;
                std::terminate();
            }
        }
        
        for (int i = 0; i < OnlineOptions::singleton.thread_number; i ++) {
            verify_queue.push(0);
        }

        for (auto &each_thread: verify_threads) {
            each_thread.join();
        }

        cout << "USS23 check passed? " << check_passed.load() << endl;
    }

    Uss23 branch() {
        typename T::Protocol res(P);
        res.prep = prep;
        res.MC = MC;
        res.init_mul();
        return res;
    }

    void init(Preprocessing<T>& prep, typename T::MAC_Check& MC) {
        this->prep = &prep;
        this->MC = &MC;
    }

    static void assign(T& share, const typename T::clear& value, int my_num)
    {
        Honest::assign(share, value, my_num);
    }

    void init_mul();
    void prepare_mul(const T& x, const T& y, int n = -1);
    void exchange();
    T finalize_mul(int n = -1);

    DZKProof _prove(
        size_t node_id,
        Field** masks,
        size_t batch_size, 
        Field sid,
        Field left_input_mask,
        Field right_input_mask
    );

    VerMsg _gen_vermsg(
        DZKProof proof, 
        size_t node_id,
        Field** masks_ss,
        size_t batch_size, 
        Field sid,
        size_t prover_ID,
        size_t party_ID,
        Field input_mask
    );

    bool _verify(
        DZKProof proof, 
        VerMsg other_vermsg, 
        size_t node_id,
        Field** masks_ss,
        size_t batch_size, 
        Field sid,
        size_t prover_ID,
        size_t party_ID,
        Field input_mask
    );

    void Check_one(size_t node_id, int size = -1);
    void verify();
    void thread_handler();
    void verify_thread_handler();
    size_t get_n_relevant_players() { return P.num_players() - 1; }
    void verify_part1(int prev_number, int my_number);
    void verify_part2(int next_number, int my_number);
};

#endif