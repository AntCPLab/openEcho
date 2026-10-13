#ifndef PROTOCOLS_USS23_HPP_
#define PROTOCOLS_USS23_HPP_

#include "Uss23.h"

namespace {

inline Field uss23WeightedBitSum(uint64_t bits) {
    const Field low = bits & 0xffffffffULL;
    const Field high = bits >> 32;
    return Mersenne::add(low, high);
}

inline Field uss23WeightedProductCore(uint64_t base, uint64_t e, uint64_t f) {
    const auto base_e = base & e;
    const auto base_f = base & f;
    const auto base_e_f = base_e & f;

    Field result = uss23WeightedBitSum(base);
    result = Mersenne::sub(result,
            Mersenne::mul(2, uss23WeightedBitSum(base_e)));
    result = Mersenne::sub(result,
            Mersenne::mul(2, uss23WeightedBitSum(base_f)));
    result = Mersenne::add(result,
            Mersenne::mul(4, uss23WeightedBitSum(base_e_f)));
    return result;
}

inline Field uss23ExpectedOutput(size_t bit_count) {
    constexpr uint64_t WEIGHT_SUM_PER_BLOCK =
            2 * ((uint64_t(1) << 32) - 1);
    const size_t full_blocks = bit_count / BLOCK_SIZE;
    const size_t remainder = bit_count % BLOCK_SIZE;

    uint128_t weight_sum =
            static_cast<uint128_t>(full_blocks) * WEIGHT_SUM_PER_BLOCK;
    if (remainder <= 32)
        weight_sum += (uint64_t(1) << remainder) - 1;
    else
        weight_sum += ((uint64_t(1) << 32) - 1)
                + ((uint64_t(1) << (remainder - 32)) - 1);

    const Field reduced_weight_sum = Mersenne::modp_128(weight_sum);
    return Mersenne::neg(Mersenne::mul(
            Mersenne::inverse(2), reduced_weight_sum));
}

inline size_t uss23ProofRoundCount(
        size_t bit_count, size_t first_compression,
        size_t recursive_compression) {
    size_t columns =
            (bit_count + first_compression - 1) / first_compression;
    columns = ((columns + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
    columns = (4 * columns + recursive_compression - 1)
            / recursive_compression;

    size_t rounds = 1;
    do {
        rounds++;
        if (columns == 1)
            break;
        columns = (columns + recursive_compression - 1)
                / recursive_compression;
    } while (true);
    return rounds;
}

inline bool uss23HasValidProofShape(
        const DZKProof& proof, size_t bit_count,
        size_t first_compression, size_t recursive_compression) {
    const size_t rounds = uss23ProofRoundCount(
            bit_count, first_compression, recursive_compression);
    if (proof.p_evals_masked.size() != rounds)
        return false;
    if (proof.p_evals_masked.front().size()
            != 2 * first_compression - 1)
        return false;
    for (size_t i = 1; i < rounds; i++) {
        const size_t expected_size = i + 1 == rounds
                ? 2 * recursive_compression + 1
                : 2 * recursive_compression - 1;
        if (proof.p_evals_masked[i].size() != expected_size)
            return false;
    }
    return true;
}

} // namespace

template <class T>
Uss23<T>::Uss23(Player& P) : P(P) {
    assert(P.num_players() == 3);
    assert(OnlineOptions::singleton.thread_number > 0);
    assert(OnlineOptions::singleton.max_status > 0);
    if (OnlineOptions::singleton.binary_batch_size <= 0
            || OnlineOptions::singleton.binary_batch_size % BLOCK_SIZE != 0) {
        throw invalid_argument("USS23 binary batch size must be a positive multiple of 64");
    }
    if (OnlineOptions::singleton.k_size < 8
            || OnlineOptions::singleton.k_size > 32
            || OnlineOptions::singleton.k_size % 8 != 0
            || OnlineOptions::singleton.k2_size < 2
            || OnlineOptions::singleton.k2_size > 32
            || BLOCK_SIZE % OnlineOptions::singleton.k2_size != 0) {
        throw invalid_argument("invalid USS23 compression parameters");
    }

    if (not P.is_encrypted()) {
        insecure("unencrypted communication");
    }

    status_queue = new StatusData[OnlineOptions::singleton.max_status];
    
    shared_prngs[0].ReSeed();
	octetStream os;
	os.append(shared_prngs[0].get_seed(), SEED_SIZE);
	P.send_relative(1, os);
	P.receive_relative(-1, os);
	shared_prngs[1].SetSeed(os.get_data());

    global_prng.SeedGlobally(P);
    
    sid = Mersenne::randomize(global_prng);

    check_prngs.resize(OnlineOptions::singleton.max_status);

    for (auto &prngs: check_prngs) {
        prngs[0].SetSeed(shared_prngs[0]);
        prngs[1].SetSeed(shared_prngs[1]);
    }

    for (int i = 0; i < OnlineOptions::singleton.thread_number; i ++) {
        check_threads.push_back(std::thread(&Uss23<T>::thread_handler, this));
        verify_threads.push_back(std::thread(&Uss23<T>::verify_thread_handler, this));
    }

    this->local_counter = 0;
    this->status_counter = 0;
    this->status_pointer = 0;

    wait_size.set_target(OnlineOptions::singleton.max_status);

    idx_input = idx_result = idx_rho = 0;
    // works for binary_batch_size % BLOCK_SIZE = 0
    // share_tuple_block_size = OnlineOptions::singleton.binary_batch_size * OnlineOptions::singleton.max_status * ZOOM_RATE / BLOCK_SIZE; // key bug
    size_t total_batch_size = OnlineOptions::singleton.binary_batch_size * OnlineOptions::singleton.max_status;
    share_tuple_block_size = (MAX_LAYER_SIZE > total_batch_size ? MAX_LAYER_SIZE : total_batch_size) * ZOOM_RATE / BLOCK_SIZE;

    // cout << "Using tuple size: " << share_tuple_block_size << endl;

    share_tuple_blocks.reserve(share_tuple_block_size);

    vermsgs = new VerMsg[OnlineOptions::singleton.max_status];
}

template <class T>
void Uss23<T>::init_mul() {
    for (auto& o : os)
        o.reset_write_head();
    add_shares.clear();

    shareTypeBlockBuffer.clear();
}

template <class T>
void Uss23<T>::prepare_mul(const T& x, const T& y, int n) {
    typename T::value_type add_share = x[0] * (y[0] + y[1]) + x[1] * y[0];

    typename T::value_type tmp[2];
    for (int i = 0; i < 2; i++)
        tmp[i].randomize(shared_prngs[i], n);

    const int bit_count = n == -1 ? BLOCK_SIZE : n;
    if (bit_count < 0 || bit_count > BLOCK_SIZE)
        throw invalid_argument("invalid USS23 SIMD width");
    const uint64_t mask = bit_count == BLOCK_SIZE ? ~uint64_t(0)
            : (uint64_t(1) << bit_count) - 1;

    shareTypeBlockBuffer.push_back(Uss23ShareTypeBlock(
            static_cast<uint64_t>(x[0].get()) & mask,
            static_cast<uint64_t>(x[1].get()) & mask));
    shareTypeBlockBuffer.push_back(Uss23ShareTypeBlock(
            static_cast<uint64_t>(y[0].get()) & mask,
            static_cast<uint64_t>(y[1].get()) & mask));
    shareTypeBlockBuffer.push_back(Uss23ShareTypeBlock(
            static_cast<uint64_t>(tmp[0].get()) & mask,
            static_cast<uint64_t>(tmp[1].get()) & mask));

    add_share = add_share + tmp[0] - tmp[1];
    add_share.pack(os[0], n);
    add_shares.push_back(add_share);
}

template <class T>
void Uss23<T>::exchange() {
    os[0].append(0);
    if (os[0].get_length() > 0)
        P.pass_around(os[0], os[1], 1);
}

template <class T>
T Uss23<T>::finalize_mul(int n) {
    T result;
    result[0] = add_shares.next();
    result[1].unpack(os[1], n);

    ShareTupleBlock block;
    block.input1 = shareTypeBlockBuffer.next();
    block.input2 = shareTypeBlockBuffer.next();
    block.rho = shareTypeBlockBuffer.next();
    const int bit_count = n == -1 ? BLOCK_SIZE : n;
    const uint64_t mask = bit_count == BLOCK_SIZE ? ~uint64_t(0)
            : (uint64_t(1) << bit_count) - 1;
    block.result = Uss23ShareTypeBlock(
            static_cast<uint64_t>(result[0].get()) & mask,
            static_cast<uint64_t>(result[1].get()) & mask);
    share_tuple_blocks.push_back(block);

    // Every stored tuple is one 64-lane SIMD block. Unused lanes are
    // explicitly zeroed above and are therefore safe to include.
    this->local_counter += BLOCK_SIZE;

    while (local_counter >= (size_t) OnlineOptions::singleton.binary_batch_size) {
        local_counter -= OnlineOptions::singleton.binary_batch_size;     
        
        Check_one(status_pointer);

        status_counter ++;
        status_pointer ++;

        if (status_counter == (size_t) OnlineOptions::singleton.max_status) {
            verify();
        }
    }

    return result;
}

template <class T>
void Uss23<T>::thread_handler() {

    int _ = -1;
    while (true) { 
        if (!cv.pop_dont_stop(_)) {
            continue;
        }

        if (_ == -1) {            
            break;
        }

        Check_one(_);
    }
    return ;
}

template <class T>
void Uss23<T>::verify_part1(int prev_number, int my_number) {
    DZKProof proof;
    verify_lock.lock();
    int i = verify_index ++;
    proof.unpack(proof_os[1]);
    verify_lock.unlock();

    size_t sz = status_queue[i].sz;

    vermsgs[i] = _gen_vermsg(proof, status_queue[i].node_id,
            status_queue[i].mask_ss_prev, sz, sid, prev_number,
            my_number, status_queue[i].input_mask_prev);

    ++ verify_tag;
    
}
template <class T>
void Uss23<T>::verify_part2(int next_number, int my_number) {
    
    VerMsg received_vermsg;
    DZKProof proof;
    
    verify_lock.lock();
    received_vermsg.unpack(vermsg_os[1]);
    proof.unpack(proof_os[1]);
    int i = verify_index ++;
    verify_lock.unlock();

    size_t sz = status_queue[i].sz;
    bool res = _verify(proof, received_vermsg,
            status_queue[i].node_id, status_queue[i].mask_ss_next,
            sz, sid, next_number, my_number,
            status_queue[i].input_mask_next);

    if (!res) {
        check_passed = false;
    }

    ++ verify_tag;
}

template <class T>
void Uss23<T>::verify_thread_handler() {
    u_char data = 0;
    int my_number = P.my_real_num();
    int prev_number = my_number == 0 ? 2 : my_number - 1;
    int next_number = my_number == 2 ? 0 : my_number + 1;

    while (true) { 
        if (!verify_queue.pop_dont_stop(data)) {
            continue;
        }

        if (data == 0) {
            break;
        }

        else if (data == 1) {
            verify_part1(prev_number, my_number);
        }

        else if (data == 2) {
            verify_part2(next_number, my_number);
        }
    }
}

template <class T>
void Uss23<T>::verify() {

    // ofstream outfile;
    // outfile.open("logs/Verify_" + to_string(P.my_real_num()), ios::app);
    
    if (status_counter == 0) {
        return ;
    }

    for (auto& o : proof_os) {
        o.clear();
    }

    for (auto& o : vermsg_os) {
        o.clear();
    }

    size_t size = status_counter;

    // outfile << "Verify with size " << size << endl;

    verify_index = 0;
    verify_tag.reset();
    verify_tag.set_target(size);

    // auto cp0 = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < size; i ++) {
        DZKProof proof = status_queue[i].proof;   
        proof.pack(proof_os[0]);
    }

    // this->check_comm += proof_os[0].get_length();
    P.pass_around(proof_os[0], proof_os[1], 1);

    // auto cp1 = std::chrono::high_resolution_clock::now();
    // outfile << "Exchange proof1 uses " << (cp1 - cp0).count() / 1e6 << "ms." << endl;

    for (size_t i = 0; i < size; i ++) {
        verify_queue.push(1);
    }

    verify_tag.wait();
    verify_tag.reset();
    verify_index = 0;

    // auto cp2 = std::chrono::high_resolution_clock::now();
    // outfile << "Gen vermsg uses " << (cp2 - cp1).count() / 1e6 << "ms." << endl;

    for (size_t i = 0; i < size; i ++) {
        vermsgs[i].pack(vermsg_os[0]);
    }

    proof_os[1].reset_write_head();

    // this->check_comm += proof_os[0].get_length();
    // this->check_comm += vermsg_os[0].get_length();
  
    P.pass_around(proof_os[0], proof_os[1], -1);
    P.pass_around(vermsg_os[0], vermsg_os[1], 1);

    // auto cp3 = std::chrono::high_resolution_clock::now();
    // outfile << "Exchange vermsg uses " << (cp3 - cp2).count() / 1e6 << "ms." << endl;

    for (size_t i = 0; i < size; i ++) {
        verify_queue.push(2);
    }

    verify_tag.wait();

    // auto cp4 = std::chrono::high_resolution_clock::now();
    // outfile << "Verify uses " << (cp4 - cp3).count() / 1e6 << "ms." << endl;

    status_counter = 0;
    status_pointer = 0;
    wait_size.reset();
    share_tuple_blocks.clear();
    if (!check_passed.load()) {
        throw runtime_error("USS23 verification failed");
    }
}

template <class T>
void Uss23<T>::Check_one(size_t node_id, int size) {

    // ofstream outfile;
    // outfile.open("logs/CheckOne_" + to_string(P.my_real_num()), ios::app);

    // outfile << "Entering Check_one, node_id = " << node_id << endl;

    // auto cp0 = std::chrono::high_resolution_clock::now();

    if (size == 0)  return ;
    size_t ms = OnlineOptions::singleton.max_status;

    if (size == -1) size = OnlineOptions::singleton.binary_batch_size;

    size_t sz = size;
    size_t k = OnlineOptions::singleton.k_size;
    size_t k2 = OnlineOptions::singleton.k2_size;
    size_t cnt = uss23ProofRoundCount(sz, k, k2);

    // outfile << "Check one with size " << sz << endl; 

    Field **masks, **mask_ss_next, **mask_ss_prev;

    masks = new Field*[cnt];
    mask_ss_next = new Field*[cnt];
    mask_ss_prev = new Field*[cnt];

    masks[0] = new Field[2 * k - 1];
    mask_ss_next[0] = new Field[2 * k - 1];
    mask_ss_prev[0] = new Field[2 * k - 1];

    
    for (size_t j = 0; j < 2 * k - 1; j ++) {
        mask_ss_next[0][j] = Mersenne::randomize(check_prngs[node_id % ms][1]);
        mask_ss_prev[0][j] = Mersenne::randomize(check_prngs[node_id % ms][0]);
        masks[0][j] = Mersenne::add(mask_ss_next[0][j], mask_ss_prev[0][j]);
    }
        
    for (size_t i = 1; i < cnt; i++) {
        const size_t row_size = i + 1 == cnt
                ? 2 * k2 + 1 : 2 * k2 - 1;
        masks[i] = new Field[row_size];
        mask_ss_next[i] = new Field[row_size];
        mask_ss_prev[i] = new Field[row_size];

        for (size_t j = 0; j < row_size; j ++) {
            mask_ss_next[i][j] = Mersenne::randomize(check_prngs[node_id % ms][1]);
            mask_ss_prev[i][j] = Mersenne::randomize(check_prngs[node_id % ms][0]);
            masks[i][j] = Mersenne::add(mask_ss_next[i][j], mask_ss_prev[i][j]);
        }
    }
    // outfile << "Prepare data uses " << (cp2 - cp1_5).count() / 1e6 << "ms." << endl;

    const Field input_mask_prev =
            Mersenne::randomize(check_prngs[node_id % ms][1]);
    const Field input_mask_next =
            Mersenne::randomize(check_prngs[node_id % ms][0]);
    DZKProof proof = _prove(node_id, masks, sz, sid,
            input_mask_next, input_mask_prev);

    // outfile << "in Check_one, pushing status_queue, ID: " << node_id << endl;
    status_queue[node_id % ms] = StatusData(proof,
                                       node_id,
                                       mask_ss_next,
                                       mask_ss_prev,
                                       input_mask_prev,
                                       input_mask_next,
                                       sz);

    ++wait_size;

    // outfile << "Finish check" << endl;
}


template <class _T>
DZKProof Uss23<_T>::_prove(
    size_t node_id,
    Field** masks,
    size_t batch_size, 
    Field sid,
    Field left_input_mask,
    Field right_input_mask
) {
    size_t k = OnlineOptions::singleton.k_size; 
    size_t k2 = OnlineOptions::singleton.k2_size;

    vector<vector<Field>> p_evals_masked;
    size_t k_max = max(k, k2 + 1);
    // Evaluations of polynomial p(X)
    Field* eval_p_poly = new Field[2 * k_max - 1];  

    Field** base = new Field*[k_max - 1];
    for (size_t i = 0; i < k_max - 1; i++) {
        base[i] = new Field[k_max];
    }

    Field** eval_result = new Field*[k_max];
    for(size_t i = 0; i < k_max; i++) {
        eval_result[i] = new Field[k_max]();
    }

    Field* eval_base = new Field[k_max];

    // ===============================  First Round  ===============================

    #ifdef TIMING
        auto start = std::chrono::high_resolution_clock::now();
    #endif

    // Vectors of masked evaluations of polynomial p(X)
    size_t T = ((batch_size - 1) / k + 1) * k;
    size_t s = (T - 1) / k + 1;

    // Transcript
    PrimeFieldTranscript transcript_hash;
    transcript_hash.append_one_msg(sid);


    ShareTupleBlock quarter_k_blocks[k];
    // works for binary_batch_size % BLOCK_SIZE = 0
    size_t start_point = (node_id % (ZOOM_RATE * OnlineOptions::singleton.max_status)) * OnlineOptions::singleton.binary_batch_size / BLOCK_SIZE;
    size_t block_cols_num = (s - 1) / BLOCK_SIZE + 1;
    size_t cur_quarter_k_blocks_id = 0;
    size_t total_blocks_num = (batch_size - 1) / BLOCK_SIZE + 1;
    // assuming k % 4 = 0
    size_t quarter_k = k / 4;

    size_t padded_s = block_cols_num * BLOCK_SIZE;
    s = padded_s;

    for (size_t block_col_id = 0; block_col_id < block_cols_num * 4; block_col_id ++) {

        // fetch k/4 tuple_blocks, containing k / 4 * BLOCKSIZE bit tuples
        const size_t remaining = cur_quarter_k_blocks_id < total_blocks_num
                ? total_blocks_num - cur_quarter_k_blocks_id : 0;
        const size_t copy_count = min(quarter_k, remaining);
        if (copy_count != 0)
            memcpy(quarter_k_blocks,
                    share_tuple_blocks.data() + start_point
                            + cur_quarter_k_blocks_id,
                    sizeof(ShareTupleBlock) * copy_count);
        for (size_t i = copy_count; i < quarter_k; i++)
            quarter_k_blocks[i] = ShareTupleBlock();
        
        for (size_t i = 0; i < quarter_k; i++) {
            const auto& row_block = quarter_k_blocks[i];
            const uint64_t a = row_block.input1.first;
            const uint64_t c = row_block.input2.first;
            const uint64_t e =
                    (a & c) ^ row_block.result.first ^ row_block.rho.first;
            const uint64_t row_factors[4] = {
                    i < copy_count ? a & c : 0,
                    i < copy_count ? c : 0,
                    i < copy_count ? a : 0,
                    i < copy_count ? ~uint64_t(0) : 0};

            for (size_t j = 0; j < quarter_k; j++) {
                const auto& col_block = quarter_k_blocks[j];
                const uint64_t b = col_block.input2.second;
                const uint64_t d = col_block.input1.second;
                const uint64_t f = col_block.rho.second;
                const uint64_t col_factors[4] = {
                        j < copy_count ? b & d : 0,
                        j < copy_count ? d : 0,
                        j < copy_count ? b : 0,
                        j < copy_count ? ~uint64_t(0) : 0};

                for (size_t row_entry_id = 0; row_entry_id < 4;
                        row_entry_id++) {
                    for (size_t col_entry_id = 0; col_entry_id < 4;
                            col_entry_id++) {
                        const uint64_t base =
                                row_factors[row_entry_id]
                                & col_factors[col_entry_id];
                        Field contribution =
                                uss23WeightedProductCore(base, e, f);
                        if (row_entry_id == 0)
                            contribution = Mersenne::neg(
                                    Mersenne::mul(2, contribution));
                        else if (row_entry_id == 3)
                            contribution = Mersenne::mul(
                                    neg_two_inverse, contribution);

                        auto& destination =
                                eval_result[i * 4 + row_entry_id]
                                           [j * 4 + col_entry_id];
                        destination =
                                Mersenne::add(destination, contribution);
                    }
                }
            }
        }
        cur_quarter_k_blocks_id += quarter_k;
    }

    for(size_t i = 0; i < k; i++) {
        eval_p_poly[i] = eval_result[i][i];
    }

    Lagrange::get_bases(k, base);
    for(size_t i = 0; i < k - 1; i++) {
        eval_p_poly[i + k] = 0;
        for(size_t j = 0; j < k; j++) {
            for (size_t l = 0; l < k; l++) {
                eval_p_poly[i + k] = Mersenne::add(eval_p_poly[i + k], Mersenne::mul(base[i][j], Mersenne::mul(eval_result[j][l], base[i][l])));
            }
        }
    }

    #ifdef TIMING
        auto end = std::chrono::high_resolution_clock::now();
        cout << "First round (compute p evals) uses: " << (end - start).count() / 1e6 << " ms" << endl;

        start = std::chrono::high_resolution_clock::now();
    #endif

    uint16_t cnt = 0;

    vector<Field> ss(2 * k - 1);       
    for(size_t i = 0; i < 2 * k - 1; i++) {           
        ss[i] = Mersenne::sub(eval_p_poly[i], masks[cnt][i]);
    }
    p_evals_masked.push_back(ss);

    transcript_hash.append_msges(ss);
    Field r = transcript_hash.get_challenge(k);

    Lagrange::evaluate_bases(k, r, eval_base);

    s *= 4;
    size_t s0 = s;
    // use k2 as the compression parameter from the second round
    s = (s - 1) / k2 + 1;

    Field **input_left, **input_right;
    input_left = new Field*[k2];
    input_right = new Field*[k2];

    for(size_t i = 0; i < k2; i++) {
        input_left[i] = new Field[s]();
        input_right[i] = new Field[s]();
    }

    size_t index = 0;
    cur_quarter_k_blocks_id = 0;

    // new matrix: total size = s * 4 = 80w, number of rows: k2 = 8, number of cols: 80w / 8 = 10w

    // generate two lookup tables
    // size_t bits_num = quarter_k / 2 * 3;

    const size_t half_blocks = quarter_k / 2;
    const size_t bits_num = half_blocks * 3;
    const size_t table_size = size_t(1) << bits_num;
    vector<Field> input_left_table1(table_size);
    vector<Field> input_left_table2(table_size);
    vector<Field> input_right_table1(table_size);
    vector<Field> input_right_table2(table_size);

    for (size_t state = 0; state < table_size; state++) {
        Field left_sum1 = 0, left_sum2 = 0;
        Field right_sum1 = 0, right_sum2 = 0;

        for (size_t j = 0; j < half_blocks; j++) {
            const bool a_or_b = (state >> (j * 3)) & 1;
            const bool c_or_d = (state >> (j * 3 + 1)) & 1;
            const bool e_or_f = (state >> (j * 3 + 2)) & 1;
            const size_t base1 = 4 * j;
            const size_t base2 = 4 * (j + half_blocks);

            const auto accumulate = [&](Field& left, Field& right,
                    size_t base_index) {
                if (a_or_b && c_or_d) {
                    const Field left_coefficient =
                            e_or_f ? 2 : neg_two;
                    const Field right_coefficient =
                            e_or_f ? neg_one : 1;
                    left = Mersenne::add(left, Mersenne::mul(
                            left_coefficient, eval_base[base_index]));
                    right = Mersenne::add(right, Mersenne::mul(
                            right_coefficient, eval_base[base_index]));
                }
                if (c_or_d) {
                    const Field coefficient = e_or_f ? neg_one : 1;
                    left = Mersenne::add(left, Mersenne::mul(
                            coefficient, eval_base[base_index + 1]));
                    right = Mersenne::add(right, Mersenne::mul(
                            coefficient, eval_base[base_index + 1]));
                }
                if (a_or_b) {
                    const Field coefficient = e_or_f ? neg_one : 1;
                    left = Mersenne::add(left, Mersenne::mul(
                            coefficient, eval_base[base_index + 2]));
                    right = Mersenne::add(right, Mersenne::mul(
                            coefficient, eval_base[base_index + 2]));
                }
                left = Mersenne::add(left, Mersenne::mul(
                        e_or_f ? two_inverse : neg_two_inverse,
                        eval_base[base_index + 3]));
                right = Mersenne::add(right, Mersenne::mul(
                        e_or_f ? neg_one : 1,
                        eval_base[base_index + 3]));
            };

            accumulate(left_sum1, right_sum1, base1);
            accumulate(left_sum2, right_sum2, base2);
        }
        input_left_table1[state] = left_sum1;
        input_left_table2[state] = left_sum2;
        input_right_table1[state] = right_sum1;
        input_right_table2[state] = right_sum2;
    }

    #ifdef TIMING
        end = std::chrono::high_resolution_clock::now();
        cout << "First round (generate lookup tables) uses: " << (end - start).count() / 1e6 << " ms" << endl;

        start = std::chrono::high_resolution_clock::now();
    #endif

    // Batchsize = 640w, total_blocks_num = 640w/64 = 10w, fetching k/4 = 8 blocks per time, needs 12500 times
    // k = 32, s = 640w/32 = 20w, block_cols_num = 20w/64 = 3125, totally 3125 cols of blocks, 3125 * 4 = 12500

    vector<uint64_t> bit_blocks_left1(bits_num);
    vector<uint64_t> bit_blocks_left2(bits_num);
    vector<uint64_t> bit_blocks_right1(bits_num);
    vector<uint64_t> bit_blocks_right2(bits_num);

    for (size_t block_col_id = 0; block_col_id < block_cols_num * 4; block_col_id ++) {

        // fetch k/4 tuple_blocks, containing k / 4 * BLOCKSIZE bit tuples
        const size_t remaining = cur_quarter_k_blocks_id < total_blocks_num
                ? total_blocks_num - cur_quarter_k_blocks_id : 0;
        const size_t copy_count = min(quarter_k, remaining);
        if (copy_count != 0)
            memcpy(quarter_k_blocks,
                    share_tuple_blocks.data() + start_point
                            + cur_quarter_k_blocks_id,
                    sizeof(ShareTupleBlock) * copy_count);
        for (size_t i = copy_count; i < quarter_k; i++)
            quarter_k_blocks[i] = ShareTupleBlock();

        // i = 0, 1, 2, 3
        for (size_t i = 0; i < quarter_k / 2; i++) {
            ShareTupleBlock cur_block = quarter_k_blocks[i];

            bit_blocks_left1[i * 3] = cur_block.input1.first;
            bit_blocks_left1[i * 3 + 1] = cur_block.input2.first;
            bit_blocks_left1[i * 3 + 2] = (cur_block.input1.first & cur_block.input2.first) ^ (cur_block.result.first) ^ (cur_block.rho.first);

            bit_blocks_right1[i * 3] = cur_block.input2.second;
            bit_blocks_right1[i * 3 + 1] = cur_block.input1.second;
            bit_blocks_right1[i * 3 + 2] = cur_block.rho.second;

            cur_block = quarter_k_blocks[i + quarter_k / 2];

            bit_blocks_left2[i * 3] = cur_block.input1.first;
            bit_blocks_left2[i * 3 + 1] = cur_block.input2.first;
            bit_blocks_left2[i * 3 + 2] = (cur_block.input1.first & cur_block.input2.first) ^ (cur_block.result.first) ^ (cur_block.rho.first);

            bit_blocks_right2[i * 3] = cur_block.input2.second;
            bit_blocks_right2[i * 3 + 1] = cur_block.input1.second;
            bit_blocks_right2[i * 3 + 2] = cur_block.rho.second;
        }

        Field left_padding = 0;
        Field right_padding = 0;
        for (size_t i = copy_count; i < quarter_k; i++) {
            left_padding = Mersenne::add(left_padding,
                    Mersenne::mul(
                            neg_two_inverse, eval_base[4 * i + 3]));
            right_padding = Mersenne::add(
                    right_padding, eval_base[4 * i + 3]);
        }

        for (size_t lane = 0; lane < BLOCK_SIZE; lane++) {
            const size_t current_index = index + lane;
            if (current_index >= s0)
                break;

            const size_t row = current_index / s;
            const size_t col = current_index % s;
            size_t left_id1 = 0, left_id2 = 0;
            size_t right_id1 = 0, right_id2 = 0;

            for (size_t j = 0; j < bits_num; j++) {
                left_id1 |= ((bit_blocks_left1[j] >> lane) & 1) << j;
                left_id2 |= ((bit_blocks_left2[j] >> lane) & 1) << j;
                right_id1 |= ((bit_blocks_right1[j] >> lane) & 1) << j;
                right_id2 |= ((bit_blocks_right2[j] >> lane) & 1) << j;
            }

            const Field lane_weight = uint64_t(1) << (lane % 32);
            const Field left_value = Mersenne::sub(Mersenne::add(
                    input_left_table1[left_id1],
                    input_left_table2[left_id2]), left_padding);
            input_left[row][col] =
                    Mersenne::mul(left_value, lane_weight);
            input_right[row][col] = Mersenne::sub(Mersenne::add(
                    input_right_table1[right_id1],
                    input_right_table2[right_id2]), right_padding);
        }

        index += BLOCK_SIZE;
        cur_quarter_k_blocks_id += quarter_k;
    }

    cnt++;

    #ifdef TIMING
        end = std::chrono::high_resolution_clock::now();
        cout << "First round (compute new inputs) uses: " << (end - start).count() / 1e6 << " ms" << endl;

        start = std::chrono::high_resolution_clock::now();
    #endif

    Lagrange::get_bases(k2, base);

    while(true){
        const bool final_round = s == 1;
        const size_t compression = final_round ? k2 + 1 : k2;
        if (final_round)
            Lagrange::get_bases(compression, base);

        for(size_t i = 0; i < compression; i++) {
            for(size_t j = 0; j < compression; j++) {
                if (final_round) {
                    const Field left = i == k2
                            ? left_input_mask : input_left[i][0];
                    const Field right = j == k2
                            ? right_input_mask : input_right[j][0];
                    eval_result[i][j] = Mersenne::mul(left, right);
                }
                else {
                    eval_result[i][j] = Mersenne::inner_product(
                            input_left[i], input_right[j], s);
                }
            }
        }

        for(size_t i = 0; i < compression; i++) {
            eval_p_poly[i] = eval_result[i][i];
        }

        for(size_t i = 0; i < compression - 1; i++) {
            eval_p_poly[i + compression] = 0;
            for(size_t j = 0; j < compression; j++) {
                for (size_t l = 0; l < compression; l++) {
                    eval_p_poly[i + compression] = Mersenne::add(
                            eval_p_poly[i + compression],
                            Mersenne::mul(base[i][j], Mersenne::mul(
                                    eval_result[j][l], base[i][l])));
                }
            }
        }

        vector<Field> ss(2 * compression - 1);
        for(size_t i = 0; i < ss.size(); i++) {
            ss[i] = Mersenne::sub(eval_p_poly[i], masks[cnt][i]);
        }
        p_evals_masked.push_back(ss);

        if (final_round) {
            break;
        }
        
        transcript_hash.append_msges(ss);
        Field r = transcript_hash.get_challenge(k2);

        Lagrange::evaluate_bases(k2, r, eval_base);

        s0 = s;
        s = (s - 1) / k2 + 1;
       
        for(size_t i = 0; i < k2; i++) {
            for(size_t j = 0; j < s; j++) {
                index = i * s + j;

                if (index < s0) {
                    uint128_t temp_result = 0;
                    for(size_t l = 0; l < k2; l++) {
                        temp_result += ((uint128_t) eval_base[l]) * ((uint128_t) input_left[l][index]);
                    }
                    input_left[i][j] = Mersenne::modp_128(temp_result);

                    temp_result = 0;
                    for(size_t l = 0; l < k2; l++) {
                        temp_result += ((uint128_t) eval_base[l]) * ((uint128_t) input_right[l][index]);
                    }
                    input_right[i][j] = Mersenne::modp_128(temp_result);

                }
                else {
                    input_left[i][j] = 0;
                    input_right[i][j] = 0;
                }
            }
        }
        cnt++;
    }

    #ifdef TIMING
        end = std::chrono::high_resolution_clock::now();
        cout << "Recursion uses: " << (end - start).count() / 1e6 << " ms" << endl;
    #endif

    for(size_t i = 0; i < k_max; i++) {
        delete[] eval_result[i];
    }
    delete[] eval_result;
    delete[] eval_p_poly;

    for (size_t i = 0; i < k_max - 1; i++) {
        delete[] base[i];
    }
    delete[] base;
    delete[] eval_base;

    for(size_t i = 0; i < k2; i++) {
        delete[] input_left[i];
        delete[] input_right[i];
    }

    delete[] input_left;
    delete[] input_right;

    for (size_t j = 0; j < p_evals_masked.size(); j++) {
        delete[] masks[j];
    }
    delete[] masks;

    DZKProof proof = {
        p_evals_masked,
    };
    return proof;
}

template <class _T>
VerMsg Uss23<_T>::_gen_vermsg(
    DZKProof proof, 
    size_t node_id,
    Field** masks_ss,
    size_t batch_size, 
    Field sid,
    size_t prover_ID,
    size_t party_ID,
    Field input_mask
) {
    size_t k = OnlineOptions::singleton.k_size;
    size_t k2 = OnlineOptions::singleton.k2_size;
    if (!uss23HasValidProofShape(proof, batch_size, k, k2)) {
        throw runtime_error("malformed USS23 proof");
    }

    size_t k_max = max(k, k2 + 1);

    Field* eval_base = new Field[k_max];
    Field* eval_base_2k = new Field[2 * k_max - 1];    

    // ===============================  First Round  ===============================

    size_t T = ((batch_size - 1) / k + 1) * k;
    size_t s = (T - 1) / k + 1;
    size_t len = uss23ProofRoundCount(batch_size, k, k2);
    size_t quarter_k = k / 4;

    vector<Field> b_ss(len);
    Field final_input = 0, final_result_ss = 0;

    size_t cnt = 0;

    // Transcript
    PrimeFieldTranscript transcript_hash;
    transcript_hash.append_one_msg(sid);

    transcript_hash.append_msges(proof.p_evals_masked[cnt]);

    Field out_ss = 0, sum_ss = 0;

    bool prev_party = ((int64_t)(party_ID + 1 - prover_ID)) % 3 == 0;

    if (prev_party) {
        out_ss = uss23ExpectedOutput(batch_size);
        for(size_t i = 0; i < 2 * k - 1; i++) { 
            proof.p_evals_masked[cnt][i] = Mersenne::add(proof.p_evals_masked[cnt][i], masks_ss[cnt][i]);
        } 
    } else {
        out_ss = 0;
        for(size_t i = 0; i < 2 * k - 1; i++) { 
            proof.p_evals_masked[cnt][i] = masks_ss[cnt][i];
        }
    }

    // compute random linear combination on the first k outputs using betas
    for (size_t j = 0; j < k; j++) { 
        sum_ss = Mersenne::add(sum_ss, proof.p_evals_masked[cnt][j]);
    }

    b_ss[cnt] = Mersenne::sub(sum_ss, out_ss);
    
    // new evaluations at random point r
    Field r = transcript_hash.get_challenge(k);
    Lagrange::evaluate_bases(k, r, eval_base);
    Lagrange::evaluate_bases(2 * k - 1, r, eval_base_2k);
    out_ss = Mersenne::inner_product(
            eval_base_2k, proof.p_evals_masked[cnt], 2 * k - 1);

    size_t start_point = (node_id % (ZOOM_RATE * OnlineOptions::singleton.max_status)) * OnlineOptions::singleton.binary_batch_size / BLOCK_SIZE;
    size_t block_cols_num = (s - 1) / BLOCK_SIZE + 1;
    size_t total_blocks_num = (batch_size - 1) / BLOCK_SIZE + 1;
    size_t cur_quarter_k_blocks_id = 0;
    ShareTupleBlock quarter_k_blocks[k];

    size_t padded_s = block_cols_num * BLOCK_SIZE;
    s = padded_s;

    s *= 4;
    size_t s0 = s;
    // use k2 as the compression parameter from the second round
    s = (s - 1) / k2 + 1;
    size_t index = 0;
 
    Field **input = new Field*[k2];
    for(size_t i = 0; i < k2; i++) {
        input[i] = new Field[s]();
    }

    cur_quarter_k_blocks_id = 0;

    const size_t half_blocks = quarter_k / 2;
    const size_t bits_num = half_blocks * 3;
    const size_t table_size = size_t(1) << bits_num;
    vector<Field> input_table1(table_size);
    vector<Field> input_table2(table_size);

    #ifdef TIMING
        auto start = std::chrono::high_resolution_clock::now(), end = start;
    #endif

    for (size_t state = 0; state < table_size; state++) {
        Field sum1 = 0, sum2 = 0;
        for (size_t j = 0; j < half_blocks; j++) {
            const bool first = (state >> (j * 3)) & 1;
            const bool second = (state >> (j * 3 + 1)) & 1;
            const bool third = (state >> (j * 3 + 2)) & 1;
            const size_t base1 = 4 * j;
            const size_t base2 = 4 * (j + half_blocks);

            const auto accumulate = [&](Field& sum, size_t base_index) {
                if (first && second) {
                    const Field coefficient = prev_party
                            ? (third ? neg_one : 1)
                            : (third ? 2 : neg_two);
                    sum = Mersenne::add(sum, Mersenne::mul(
                            coefficient, eval_base[base_index]));
                }
                if (second) {
                    const Field coefficient = third ? neg_one : 1;
                    sum = Mersenne::add(sum, Mersenne::mul(
                            coefficient, eval_base[base_index + 1]));
                }
                if (first) {
                    const Field coefficient = third ? neg_one : 1;
                    sum = Mersenne::add(sum, Mersenne::mul(
                            coefficient, eval_base[base_index + 2]));
                }
                const Field coefficient = prev_party
                        ? (third ? neg_one : 1)
                        : (third ? two_inverse : neg_two_inverse);
                sum = Mersenne::add(sum, Mersenne::mul(
                        coefficient, eval_base[base_index + 3]));
            };

            accumulate(sum1, base1);
            accumulate(sum2, base2);
        }
        input_table1[state] = sum1;
        input_table2[state] = sum2;
    }

    #ifdef TIMING
            end = std::chrono::high_resolution_clock::now();
            cout << "First round (generate lookup tables) uses: " << (end - start).count() / 1e6 << " ms" << endl;
            start = std::chrono::high_resolution_clock::now();
    #endif

    vector<uint64_t> bit_blocks1(bits_num);
    vector<uint64_t> bit_blocks2(bits_num);
    for (size_t block_col_id = 0;
            block_col_id < block_cols_num * 4; block_col_id++) {
        const size_t remaining = cur_quarter_k_blocks_id < total_blocks_num
                ? total_blocks_num - cur_quarter_k_blocks_id : 0;
        const size_t copy_count = min(quarter_k, remaining);
        if (copy_count != 0)
            memcpy(quarter_k_blocks,
                    share_tuple_blocks.data() + start_point
                            + cur_quarter_k_blocks_id,
                    sizeof(ShareTupleBlock) * copy_count);
        for (size_t i = copy_count; i < quarter_k; i++)
            quarter_k_blocks[i] = ShareTupleBlock();

        for (size_t i = 0; i < half_blocks; i++) {
            auto block = quarter_k_blocks[i];
            if (prev_party) {
                bit_blocks1[i * 3] = block.input2.first;
                bit_blocks1[i * 3 + 1] = block.input1.first;
                bit_blocks1[i * 3 + 2] = block.rho.first;
            }
            else {
                bit_blocks1[i * 3] = block.input1.second;
                bit_blocks1[i * 3 + 1] = block.input2.second;
                bit_blocks1[i * 3 + 2] =
                        (block.input1.second & block.input2.second)
                        ^ block.result.second ^ block.rho.second;
            }

            block = quarter_k_blocks[i + half_blocks];
            if (prev_party) {
                bit_blocks2[i * 3] = block.input2.first;
                bit_blocks2[i * 3 + 1] = block.input1.first;
                bit_blocks2[i * 3 + 2] = block.rho.first;
            }
            else {
                bit_blocks2[i * 3] = block.input1.second;
                bit_blocks2[i * 3 + 1] = block.input2.second;
                bit_blocks2[i * 3 + 2] =
                        (block.input1.second & block.input2.second)
                        ^ block.result.second ^ block.rho.second;
            }
        }

        Field padding = 0;
        for (size_t i = copy_count; i < quarter_k; i++) {
            const Field coefficient =
                    prev_party ? 1 : neg_two_inverse;
            padding = Mersenne::add(padding, Mersenne::mul(
                    coefficient, eval_base[4 * i + 3]));
        }

        for (size_t lane = 0; lane < BLOCK_SIZE; lane++) {
            const size_t current_index = index + lane;
            if (current_index >= s0)
                break;
            const size_t row = current_index / s;
            const size_t col = current_index % s;
            size_t lookup1 = 0, lookup2 = 0;
            for (size_t j = 0; j < bits_num; j++) {
                lookup1 |= ((bit_blocks1[j] >> lane) & 1) << j;
                lookup2 |= ((bit_blocks2[j] >> lane) & 1) << j;
            }

            Field value = Mersenne::sub(Mersenne::add(
                    input_table1[lookup1], input_table2[lookup2]),
                    padding);
            if (!prev_party)
                value = Mersenne::mul(
                        value, uint64_t(1) << (lane % 32));
            input[row][col] = value;
        }

        index += BLOCK_SIZE;
        cur_quarter_k_blocks_id += quarter_k;
    }

    cnt++;

    #ifdef TIMING
        end = std::chrono::high_resolution_clock::now();
        cout << "First round (compute new inputs) uses: " << (end - start).count() / 1e6 << " ms" << endl;

        start = std::chrono::high_resolution_clock::now();
    #endif

    while(true)
    {
        const bool final_round = s == 1;
        const size_t compression = final_round ? k2 + 1 : k2;
        const size_t proof_row_size = 2 * compression - 1;
        transcript_hash.append_msges(proof.p_evals_masked[cnt]);

        if(prev_party) {
            for(size_t i = 0; i < proof_row_size; i++) {
                proof.p_evals_masked[cnt][i] = Mersenne::add(proof.p_evals_masked[cnt][i], masks_ss[cnt][i]);
            } 
        } else {
            for(size_t i = 0; i < proof_row_size; i++) {
                proof.p_evals_masked[cnt][i] = masks_ss[cnt][i];
            }
        }
        sum_ss = 0;
        for(size_t j = 0; j < k2; j++) { 
            sum_ss = Mersenne::add(
                    sum_ss, proof.p_evals_masked[cnt][j]);
        }
        b_ss[cnt] = Mersenne::sub(sum_ss, out_ss);

        r = transcript_hash.get_challenge(k2);
        Lagrange::evaluate_bases(
                proof_row_size, r, eval_base_2k);
        uint128_t temp_result = 0;
        for(size_t i = 0; i < proof_row_size; i++) {
            temp_result += (uint128_t)eval_base_2k[i] * (uint128_t)proof.p_evals_masked[cnt][i];
        }
        out_ss = Mersenne::modp_128(temp_result);

        if(final_round) {
            Lagrange::evaluate_bases(compression, r, eval_base);
            
            for(size_t i = 0; i < k2; i++) {
                final_input = Mersenne::add(final_input,
                        Mersenne::mul(eval_base[i], input[i][0]));
            }
            final_input = Mersenne::add(final_input,
                    Mersenne::mul(eval_base[k2], input_mask));
            final_result_ss = out_ss;

            break;
        }

        Lagrange::evaluate_bases(k2, r, eval_base);
        s0 = s;
        s = (s - 1) / k2 + 1;
        for(size_t i = 0; i < k2; i++) {
            for(size_t j = 0; j < s; j++) {
                index = i * s + j;
                if (index < s0) {
                    uint128_t temp_result = 0;
                    for(size_t l = 0; l < k2; l++) {
                        temp_result += (uint128_t)eval_base[l] * (uint128_t)input[l][index];
                    }
                    input[i][j] = Mersenne::modp_128(temp_result);
                }
                else {
                    input[i][j] = 0;
                }
            }
        }

        cnt++;
    }

    #ifdef TIMING
        end = std::chrono::high_resolution_clock::now();
        cout << "Recursion uses: " << (end - start).count() / 1e6 << " ms" << endl;
    #endif

    delete[] eval_base;
    delete[] eval_base_2k;

    for (size_t i = 0; i < k2; i++)
        delete[] input[i];
    delete[] input;

    for (size_t j = 0; j < proof.p_evals_masked.size(); j++) {
        delete[] masks_ss[j];
    }
    delete[] masks_ss;

    VerMsg vermsg(
        b_ss,
        final_input,
        final_result_ss
    );

    return vermsg;
}

template <class _T>
bool Uss23<_T>::_verify(
    DZKProof proof, 
    VerMsg other_vermsg, 
    size_t node_id,
    Field** masks_ss,
    size_t batch_size, 
    Field sid,
    size_t prover_ID,
    size_t party_ID,
    Field input_mask
) {
    VerMsg self_vermsg = _gen_vermsg(proof, node_id, masks_ss,
            batch_size, sid, prover_ID, party_ID, input_mask);

    if (self_vermsg.b_ss.size() != other_vermsg.b_ss.size())
        return false;
    for(size_t i = 0; i < self_vermsg.b_ss.size(); i++) {
        const Field b = Mersenne::add(
                self_vermsg.b_ss[i], other_vermsg.b_ss[i]);
        if (b != 0) {
            cerr << "USS23 verification failed in round " << i
                 << ": b=" << b << endl;
            return false;
        }
    }

    const Field result = Mersenne::mul(
            self_vermsg.final_input, other_vermsg.final_input);
    const Field claimed_result = Mersenne::add(
            self_vermsg.final_result_ss, other_vermsg.final_result_ss);
    if (Mersenne::sub(result, claimed_result) != 0) {
        cerr << "USS23 final multiplication check failed" << endl;
        return false;
    }

    return true;
}


#endif