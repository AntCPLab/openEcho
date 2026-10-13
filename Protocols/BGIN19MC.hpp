#ifndef PROTOCOLS_BGIN19MC_HPP_
#define PROTOCOLS_BGIN19MC_HPP_

#include "Protocols/BGIN19MC.h"
#include "Math/gf28.h"
#include "Tools/intrinsics.h"

inline size_t bgin19ProofRoundCount(size_t batchSize, size_t k, size_t k2)
{
  if (batchSize == 0)
    return 0;
  if (k < 2 || k >= 8 * sizeof(uint64_t) || k != k2
      || k2 > BLOCK_SIZE || BLOCK_SIZE % k2 != 0)
    throw invalid_argument("invalid BGIN19 compression parameters");

  size_t compressedSize = DIV_CEIL(batchSize, k);
  compressedSize = DIV_CEIL(compressedSize, BLOCK_SIZE) * BLOCK_SIZE;

  size_t rounds = 1;
  compressedSize = DIV_CEIL(2 * compressedSize, k2);
  while (true)
  {
    rounds++;
    if (compressedSize == 1)
      return rounds;
    compressedSize = DIV_CEIL(compressedSize, k2);
  }
}

gf2n_long inner_product(gf2n_long* x, gf2n_long* y, size_t length) {
  gf2n_long answer;
  int128 tmp = 0;
  for (size_t i = 0; i < length; ++i) {
    tmp ^= clmul<0>(int128(x[i].get()).a, int128(y[i].get()).a);
  }

  gf2n_long::internal_type hi,lo;
  int128 _tmp = (tmp);
  _tmp.to(lo);
  hi = 0;
  answer.reduce(hi, lo);

  return answer;
}

gf2n_long inner_product(gf2n_long* x, vector<gf2n_long> y, size_t length) {
  gf2n_long answer;
  int128 tmp = 0;
  for (size_t i = 0; i < length; ++i) {
    tmp ^= clmul<0>(int128(x[i].get()).a, int128(y[i].get()).a);
  }

  gf2n_long::internal_type hi,lo;
  int128 _tmp = (tmp);
  _tmp.to(lo);
  hi = 0;
  answer.reduce(hi, lo);

  return answer;
}

gf2n_long inner_product(gf2n_long** x, gf2n_long** y, size_t rows, size_t cols) {
  gf2n_long answer;
  int128 tmp = 0;
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      tmp ^= clmul<0>(int128(x[i][j].get()).a, int128(y[i][j].get()).a);
    }
  }

  gf2n_long::internal_type hi,lo;
  int128 _tmp = (tmp);
  _tmp.to(lo);
  hi = 0;
  answer.reduce(hi, lo);

  return answer;
}

gf2n_long inner_product(gf2n_long** x, gf2n_long* y, size_t rows, size_t cols) {
  gf2n_long answer;
  int128 tmp = 0;
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      tmp ^= clmul<0>(int128(x[i][j].get()).a, int128(y[j].get()).a);
    }
  }

  gf2n_long::internal_type hi,lo;
  int128 _tmp = (tmp);
  _tmp.to(lo);
  hi = 0;
  answer.reduce(hi, lo);

  return answer;
}

// ============================================================


gf2n_short inner_product(gf2n_short* x, gf2n_short* y, size_t length) {
  gf2n_short answer;
  int128 tmp = 0;
  for (size_t i = 0; i < length; ++i) {
    tmp ^= clmul<0>(int128(x[i].get()).a, int128(y[i].get()).a);
  }

  int128 _tmp = (tmp);
  answer.reduce(_tmp.get_upper(), _tmp.get_lower());

  return answer;
}

gf2n_short inner_product(gf2n_short* x, vector<gf2n_short> y, size_t length) {
  gf2n_short answer;
  int128 tmp = 0;
  for (size_t i = 0; i < length; ++i) {
    tmp ^= clmul<0>(int128(x[i].get()).a, int128(y[i].get()).a);
  }

  int128 _tmp = (tmp);
  answer.reduce(_tmp.get_upper(), _tmp.get_lower());

  return answer;
}

gf2n_short inner_product(gf2n_short** x, gf2n_short** y, size_t rows, size_t cols) {
  gf2n_short answer;
  int128 tmp = 0;
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      tmp ^= clmul<0>(int128(x[i][j].get()).a, int128(y[i][j].get()).a);
    }
  }

  int128 _tmp = (tmp);
  answer.reduce(_tmp.get_upper(), _tmp.get_lower());

  return answer;
}

gf2n_short inner_product(gf2n_short** x, gf2n_short* y, size_t rows, size_t cols) {
  gf2n_short answer;
  int128 tmp = 0;
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      tmp ^= clmul<0>(int128(x[i][j].get()).a, int128(y[j].get()).a);
    }
  }

  int128 _tmp = (tmp);
  answer.reduce(_tmp.get_upper(), _tmp.get_lower());

  return answer;
}

// ============================================================
// gf28 inner products with vectorized multiply and SIMD XOR reduce

inline gf28 inner_product(gf28* x, gf28* y, size_t length) {
  if (length == 0) return gf28(0);
  // Direct streaming dot without temporaries
  // Assumes gf28 is byte-sized contiguous
  const uint8_t* xb = reinterpret_cast<const uint8_t*>(&x[0]);
  const uint8_t* yb = reinterpret_cast<const uint8_t*>(&y[0]);
  uint8_t res = gf28::dot(xb, yb, length);
  return gf28(res);
}

inline gf28 inner_product(gf28* x, vector<gf28> y, size_t length) {
  if (length == 0) return gf28(0);
  const uint8_t* xb = reinterpret_cast<const uint8_t*>(&x[0]);
  const uint8_t* yb = reinterpret_cast<const uint8_t*>(y.data());
  uint8_t res = gf28::dot(xb, yb, length);
  return gf28(res);
}

inline gf28 inner_product(gf28** x, gf28** y, size_t rows, size_t cols) {
  uint8_t accum = 0;
  for (size_t r = 0; r < rows; ++r) {
    const uint8_t* xb = reinterpret_cast<const uint8_t*>(&x[r][0]);
    const uint8_t* yb = reinterpret_cast<const uint8_t*>(&y[r][0]);
    accum ^= gf28::dot(xb, yb, cols);
  }
  return gf28(accum);
}

inline gf28 inner_product(gf28** x, gf28* y, size_t rows, size_t cols) {
  uint8_t accum = 0;
  const uint8_t* yb = reinterpret_cast<const uint8_t*>(&y[0]);
  for (size_t r = 0; r < rows; ++r) {
    const uint8_t* xb = reinterpret_cast<const uint8_t*>(&x[r][0]);
    accum ^= gf28::dot(xb, yb, cols);
  }
  return gf28(accum);
}

// Variant: second "vector" indexed in the first dimension (use a column from a matrix)
inline gf28 inner_product_first_dim(const gf28* x, gf28** y, size_t rows, size_t col)
{
  if (rows == 0) return gf28(0);
  uint8_t accum = 0;
  alignas(16) uint8_t ycol_buf[256];
  const bool small = rows <= 256;
  if (small) {
    for (size_t i = 0; i < rows; ++i) ycol_buf[i] = y[i][col].get();
    const uint8_t* xb = reinterpret_cast<const uint8_t*>(&x[0]);
    accum = gf28::dot(xb, ycol_buf, rows);
  } else {
    size_t i = 0;
    const uint8_t* xb = reinterpret_cast<const uint8_t*>(&x[0]);
    while (i < rows) {
      size_t chunk = std::min((size_t)256, rows - i);
      for (size_t t = 0; t < chunk; ++t) ycol_buf[t] = y[i + t][col].get();
      accum ^= gf28::dot(xb + i, ycol_buf, chunk);
      i += chunk;
    }
  }
  return gf28(accum);
}

inline gf28 inner_product_first_dim(const gf28* x, const vector<vector<gf28>>& y, size_t col)
{
  size_t rows = y.size();
  if (rows == 0) return gf28(0);
  uint8_t accum = 0;
  alignas(16) uint8_t ycol_buf[256];
  const bool small = rows <= 256;
  if (small) {
    for (size_t i = 0; i < rows; ++i) ycol_buf[i] = y[i][col].get();
    const uint8_t* xb = reinterpret_cast<const uint8_t*>(&x[0]);
    accum = gf28::dot(xb, ycol_buf, rows);
  } else {
    size_t i = 0;
    const uint8_t* xb = reinterpret_cast<const uint8_t*>(&x[0]);
    while (i < rows) {
      size_t chunk = std::min((size_t)256, rows - i);
      for (size_t t = 0; t < chunk; ++t) ycol_buf[t] = y[i + t][col].get();
      accum ^= gf28::dot(xb + i, ycol_buf, chunk);
      i += chunk;
    }
  }
  return gf28(accum);
}

// Compute eval_p_poly[i + k] = base[i]^T * eval_result * base[i] for i in [0, k-2]
// Specialized and vectorized for gf28 and small k (e.g., k=8)
inline vector<gf28> compute_eval_p_poly_quadratic_gf28(gf28** base, gf28** eval_result, size_t k)
{
  vector<gf28> out; out.resize(k > 0 ? k - 1 : 0);
  if (k == 0) return out;

  vector<uint8_t> b(k), row(k), tmp(k), sj(k);

  auto xor_reduce = [] (const uint8_t* data, size_t n) -> uint8_t {
    __m128i acc = _mm_setzero_si128();
    size_t i = 0;
    for (; i + 16 <= n; i += 16) {
      __m128i v = _mm_loadu_si128((const __m128i*)(data + i));
      acc = _mm_xor_si128(acc, v);
    }
    alignas(16) uint8_t buf[16];
    _mm_storeu_si128((__m128i*)buf, acc);
    uint8_t s = 0;
    for (int t = 0; t < 16; ++t) s ^= buf[t];
    for (; i < n; ++i) s ^= data[i];
    return s;
  };

  for (size_t i = 0; i + 1 <= k - 1 && i < k; ++i) {
    for (size_t t = 0; t < k; ++t) b[t] = base[i][t].get();

    for (size_t j = 0; j < k; ++j) {
      for (size_t l = 0; l < k; ++l) row[l] = eval_result[j][l].get();
      gf28::mul_vec(row.data(), b.data(), tmp.data(), k);
      sj[j] = xor_reduce(tmp.data(), k);
    }

    gf28::mul_vec(b.data(), sj.data(), tmp.data(), k);
    uint8_t acc = xor_reduce(tmp.data(), k);
    out[i] = gf28(acc);
  }

  return out;
}

template <class T>
void BGIN19MC<T>::MaybeCheck(const Player& P, bool final) {
    // The command-line batch size counts packed 64-bit AND operations.
    size_t batch_size = (size_t) OnlineOptions::singleton.binary_batch_size;
    if (!final && mult_triples.size() < batch_size) {
        return ;
    }

    // while (mult_triples.size() >= batch_size) {
    //     Check_one(P, batch_size);
    //     mult_triples.remove_front(batch_size);
    // }

    // if (mult_triples.size() == 0 || !final) return ;
    // Check_one(P, mult_triples.size());

    if (mult_triples.size() != 0) {
        Check_one(P, mult_triples.size());
        mult_triples.clear();
        trip_pointer = 0;
    }
}

template <class T>
void BGIN19MC<T>::Check_one(const Player& P, size_t block_count) {

    int my_number = P.my_real_num();
    int prev_number = my_number == 0 ? 2 : my_number - 1;
    int next_number = my_number == 2 ? 0 : my_number + 1;

    // Proof dimensions count scalar bit triples, while storage uses SIMD blocks.
    if (block_count > numeric_limits<size_t>::max() / BLOCK_SIZE)
        throw overflow_error("BGIN19 SIMD batch is too large");
    size_t bit_count = block_count * BLOCK_SIZE;
    size_t k = OnlineOptions::singleton.k_size;
    size_t k2 = OnlineOptions::singleton.k2_size;
    size_t cnt = bgin19ProofRoundCount(bit_count, k, k2);

    BGIN19Field **masks, **mask_ss_next, **mask_ss_prev;

    masks = new BGIN19Field*[cnt];
    mask_ss_next = new BGIN19Field*[cnt];
    mask_ss_prev = new BGIN19Field*[cnt];

    masks[0] = new BGIN19Field[2 * k - 1];
    mask_ss_next[0] = new BGIN19Field[2 * k - 1];
    mask_ss_prev[0] = new BGIN19Field[2 * k - 1];

    for (size_t j = 0; j < 2 * k - 1; j ++) {
        mask_ss_next[0][j].randomize(prng_protocol->shared_prngs[1]);
        mask_ss_prev[0][j].randomize(prng_protocol->shared_prngs[0]);
        masks[0][j] = mask_ss_next[0][j] + mask_ss_prev[0][j];
    }

    for (size_t i = 1; i < cnt; i++) {
        masks[i] = new BGIN19Field[2 * k2 - 1];
        mask_ss_next[i] = new BGIN19Field[2 * k2 - 1];
        mask_ss_prev[i] = new BGIN19Field[2 * k2 - 1];

        for (size_t j = 0; j < 2 * k2 - 1; j ++) {
            mask_ss_next[i][j].randomize(prng_protocol->shared_prngs[1]);
            mask_ss_prev[i][j].randomize(prng_protocol->shared_prngs[0]);
            masks[i][j] = mask_ss_next[i][j] + mask_ss_prev[i][j];
        }
    }

    BGIN19DZKProof proof = _prove(masks, bit_count, sid, global_prng);

    for (int i = 0; i < 2; i ++) {
        proof_os[i].reset_write_head();
        vermsg_os[i].reset_write_head();
    }

    proof.pack(proof_os[0]);
    P.pass_around(proof_os[0], proof_os[1], 1);

    BGIN19DZKProof proof_recv;
    proof_recv.unpack(proof_os[1]);

    BGIN19VerMsg vermsg = _gen_vermsg(proof_recv, mask_ss_next, bit_count, sid,
            prev_number, my_number, global_prng);

    vermsg.pack(vermsg_os[0]);
    proof_os[1].reset_write_head();
    P.pass_around(proof_os[0], proof_os[1], -1);
    P.pass_around(vermsg_os[0], vermsg_os[1], 1);

    BGIN19VerMsg received_vermsg;
    BGIN19DZKProof complete_proof;

    received_vermsg.unpack(vermsg_os[1]);
    complete_proof.unpack(proof_os[1]);

    bool res = _verify(complete_proof, received_vermsg, mask_ss_prev, bit_count, sid,
            next_number, my_number, global_prng);
    passed &= res;
}

template <class U>
BGIN19DZKProof BGIN19MC<U>::_prove(
    BGIN19Field** masks,
    size_t batch_size, 
    BGIN19Field sid,
    PRNG prng
) {
    size_t k = OnlineOptions::singleton.k_size;
    size_t k2 = OnlineOptions::singleton.k2_size;

    // Vectors of masked evaluations of polynomial p(X)
    vector<vector<BGIN19Field>> p_evals_masked;
    size_t k_max = k > k2 ? k : k2;

    // Evaluations of polynomial p(X)
    this->ensure_matrix(this->scratch_eval_result, this->scratch_eval_result_rows, k_max, k_max);
    BGIN19Field** eval_result = this->scratch_eval_result_rows.data();
    for (size_t i = 0; i < k_max; ++i)
        for (size_t j = 0; j < k_max; ++j)
            eval_result[i][j] = 0;

    this->scratch_eval_p_poly.resize(2 * k_max - 1);
    BGIN19Field* eval_p_poly = this->scratch_eval_p_poly.data();

    // base: allocate fresh as lifecycle small and used by lagrange utils
    vector<vector<BGIN19Field>> base_vec(k_max - 1, vector<BGIN19Field>(k_max));
    vector<BGIN19Field*> base_ptrs(k_max - 1);
    for (size_t i = 0; i < k_max - 1; ++i) base_ptrs[i] = base_vec[i].data();
    BGIN19Field** base = base_ptrs.data();

    this->scratch_eval_base.resize(k_max);
    BGIN19Field* eval_base = this->scratch_eval_base.data();

    // ===============================  First Round  ===============================

    size_t T = ((batch_size - 1) / k + 1) * k;
    size_t s = (T - 1) / k + 1;

    // Transcript
    BGIN19LocalHash transcript_hash;
    transcript_hash.append_one_msg(sid);
    
    BGIN19ShareTupleBlock k_share_tuple_blocks[k];
    // works for binary_batch_size % BLOCK_SIZE = 0
    size_t block_cols_num = (s - 1) / BLOCK_SIZE + 1;
    
    size_t cur_k_blocks = 0;
    size_t total_blocks_num = (batch_size - 1) / BLOCK_SIZE + 1;

    size_t padded_s = block_cols_num * BLOCK_SIZE;

    // bs = 6400, s = 800 -> 832
    s = padded_s;

    this->scratch_thetas.resize(s);
    BGIN19Field* thetas = this->scratch_thetas.data();
    for(size_t j = 0; j < s; j++) {
        thetas[j].randomize(prng);
    }

    for (size_t block_col = 0; block_col < block_cols_num; block_col ++) {

        // fetch k tuple_blocks
        if (block_col == block_cols_num - 1 && total_blocks_num - cur_k_blocks < k) {
            memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * (total_blocks_num - cur_k_blocks));
            for (size_t i = total_blocks_num - cur_k_blocks; i < k; i++)
                k_share_tuple_blocks[i] = BGIN19ShareTupleBlock();
        }
        else {
            memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * k);
        }
        
        for(size_t i = 0; i < k; i++) { 
            BGIN19ShareTupleBlock row_tuple_block = k_share_tuple_blocks[i];
            
            for(size_t j = 0; j < k; j++) {  
                BGIN19ShareTupleBlock col_tuple_block = k_share_tuple_blocks[j];

                // x_i * y_{i-1} + y_i * x_{i-1}
                long this_value_block = (row_tuple_block.input1.first & col_tuple_block.input2.second) ^ (row_tuple_block.input2.first & col_tuple_block.input1.second);

                // iterate over set bits only
                size_t theta_off = block_col * BLOCK_SIZE;
                unsigned long long t = (unsigned long long)this_value_block;
                while (t) {
                    unsigned int l = __builtin_ctzll(t);
                    eval_result[i][j] += thetas[theta_off + l];
                    t &= t - 1;
                }
            }
        }
        cur_k_blocks += k;
    }

    for(size_t i = 0; i < k; i++) {
        eval_p_poly[i] = eval_result[i][i];
    }

    BGIN19Lagrange::get_bases(k, base);

    auto vals = compute_eval_p_poly_quadratic_gf28(base, eval_result, k);
    for(size_t i = 0; i < k - 1; i++) {
        eval_p_poly[i + k] = vals[i];
    }

    size_t cnt = 0;

    vector<BGIN19Field> ss(2 * k - 1);       
    for(size_t i = 0; i < 2 * k - 1; i++) {           
        ss[i] = eval_p_poly[i] - masks[cnt][i];
    }
    p_evals_masked.push_back(ss);


    // ===============================  Following Rounds  ===============================

    transcript_hash.append_msges(ss);
    BGIN19Field r = transcript_hash.get_challenge();

    cnt++;

    BGIN19Lagrange::evaluate_bases(k, r, eval_base);

    s *= 2;
    size_t s0 = s;
    // use k2 as the compression parameter from the second round
    s = (s - 1) / k2 + 1;
 
    this->scratch_input_left.resize(k2 * s);
    this->scratch_input_right.resize(k2 * s);
    this->ensure_matrix(this->scratch_input_left, this->scratch_input_left_rows, k2, s);
    this->ensure_matrix(this->scratch_input_right, this->scratch_input_right_rows, k2, s);
    BGIN19Field** input_left = this->scratch_input_left_rows.data();
    BGIN19Field** input_right = this->scratch_input_right_rows.data();

    size_t index = 0;
    cur_k_blocks = 0;

    uint64_t table_size = 1 << k;
    this->scratch_input_table.resize(table_size);
    BGIN19Field* input_table = this->scratch_input_table.data();

    for (uint64_t i = 0; i < table_size; i++) {
        BGIN19Field sum = 0;
        for (uint64_t j = 0; j < k; j++) {
            if ((i >> j) & 1)
                sum += eval_base[j];
        }
        input_table[i] = sum;
    }

    this->scratch_bit_blocks_left1.resize(k);
    this->scratch_bit_blocks_left2.resize(k);
    this->scratch_bit_blocks_right1.resize(k);
    this->scratch_bit_blocks_right2.resize(k);
    long* bit_blocks_left1 = this->scratch_bit_blocks_left1.data();
    long* bit_blocks_left2 = this->scratch_bit_blocks_left2.data();
    long* bit_blocks_right1 = this->scratch_bit_blocks_right1.data();
    long* bit_blocks_right2 = this->scratch_bit_blocks_right2.data();

    int bits_num = k2;
    int group_num = BLOCK_SIZE / k2;

    for (size_t block_col = 0; block_col < block_cols_num; block_col ++) {

        // fetch k tuple_blocks, containing k * BLOCKSIZE bit tuples
        if (block_col == block_cols_num - 1 && total_blocks_num - cur_k_blocks < k) {
            memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * (total_blocks_num - cur_k_blocks));
            for (size_t i = total_blocks_num - cur_k_blocks; i < k; i++)
                k_share_tuple_blocks[i] = BGIN19ShareTupleBlock();
        }
        else {
            memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * k);
        }

        for (uint64_t i = 0; i < k; i++) {

            BGIN19ShareTupleBlock cur_block = k_share_tuple_blocks[i];

            bit_blocks_left1[i] = cur_block.input1.first;
            bit_blocks_left2[i] = cur_block.input2.first;

            bit_blocks_right1[i] = cur_block.input2.second;
            bit_blocks_right2[i] = cur_block.input1.second;
        }

        // k = 8, bits_num = 8, group_num = 8
        // bit_id = 0, 1, 2, ..., 7
        for (int bit_id = 0; bit_id < bits_num; bit_id++) {
            // group_id = 0, 1, 2, ..., 7
            for (int group_id = 0; group_id < group_num; group_id++) {

                // bit_id = 0:  overall_bit_id = 0, 8, 16, 24, 32, 40, 48, 56
                // bit_id = 1:  overall_bit_id = 1, 9, 17, 25, 33, 41, 49, 57
                // ......
                // bit_id = 7:  overall_bit_id = 7, 15, 23, 31, 39, 47, 49, 63
                int overall_bit_id = group_id * bits_num + bit_id;
                int cur_index = index + overall_bit_id * 2;
                int row = cur_index / s;
                int col = cur_index % s;

                if (__builtin_expect(index >= s0, 0)) {
                    if (__builtin_expect((uint64_t)row == k2, 1)) {
                        break;
                    }
                    else {
                        input_left[row][col] = input_left[row][col + 1] = 0;
                        input_right[row][col] = input_right[row][col + 1] = 0;
                        continue;
                    }
                }

                uint64_t left_id1 = 0, left_id2 = 0, right_id1 = 0, right_id2 = 0;
                unsigned long long col_mask = 1ULL << overall_bit_id;

                // Optimized bit extraction using parallel bit operations
                for (int j = 0; j < bits_num; j++) {
                    uint64_t j_bit = 1ULL << j;
                    left_id1 |= (bit_blocks_left1[j] & (long)col_mask) ? j_bit : 0ULL;
                    left_id2 |= (bit_blocks_left2[j] & (long)col_mask) ? j_bit : 0ULL;
                    right_id1 |= (bit_blocks_right1[j] & (long)col_mask) ? j_bit : 0ULL;
                    right_id2 |= (bit_blocks_right2[j] & (long)col_mask) ? j_bit : 0ULL;
                }

                size_t theta_idx = block_col * BLOCK_SIZE + overall_bit_id;
                BGIN19Field theta_val = thetas[theta_idx];
                input_left[row][col] = input_table[left_id1] * theta_val;
                input_left[row][col + 1] = input_table[left_id2] * theta_val;
                input_right[row][col] = input_table[right_id1];
                input_right[row][col + 1] = input_table[right_id2];
            } 
        }
        
        index += BLOCK_SIZE * 2;
        cur_k_blocks += k;
    }

    BGIN19Lagrange::get_bases(k2, base);

    while(true){

        // Batch compute all inner products with better cache locality
        for(size_t i = 0; i < k2; i++) {
            const uint8_t* left_row = reinterpret_cast<const uint8_t*>(input_left[i]);
            for(size_t j = 0; j < k2; j++) {
                const uint8_t* right_row = reinterpret_cast<const uint8_t*>(input_right[j]);
                eval_result[i][j] = gf28::dot(left_row, right_row, s);
            }
        }

        for(size_t i = 0; i < k2; i++) {
            eval_p_poly[i] = eval_result[i][i];
        }

        auto vals2 = compute_eval_p_poly_quadratic_gf28(base, eval_result, k2);

        for(size_t i = 0; i < k2 - 1; i++) {
            eval_p_poly[i + k2] = vals2[i];
        }

        vector<BGIN19Field> ss(2 * k2 - 1);       
        for (size_t i = 0; i < 2 * k2 - 1; i++) {           
            ss[i] = eval_p_poly[i] - masks[cnt][i];
        }
        p_evals_masked.push_back(ss);

        if (s == 1) {
            break;
        }
        
        transcript_hash.append_msges(ss);
        r = transcript_hash.get_challenge();

        BGIN19Lagrange::evaluate_bases(k2, r, eval_base);

        s0 = s;
        s = (s - 1) / k2 + 1;
       
        for(size_t i = 0; i < k2; i++) {
            for(size_t j = 0; j < s; j++) {
                index = i * s + j;
               
                if (index < s0) {
                    input_left[i][j] = inner_product_first_dim(eval_base,
                            input_left, k2, index);
                    input_right[i][j] = inner_product_first_dim(eval_base,
                            input_right, k2, index);
                }
                else {
                    input_left[i][j] = 0;
                    input_right[i][j] = 0;
                }
            }
        }

        cnt++;
    }

    // no deletes needed for scratch buffers
    for (size_t j = 0; j < bgin19ProofRoundCount(batch_size, k, k2); j ++) {
        delete[] masks[j];
    }
    delete[] masks;

    BGIN19DZKProof proof = {
        p_evals_masked,
    };
    return proof;
}

template <class U>
BGIN19VerMsg BGIN19MC<U>::_gen_vermsg(
    BGIN19DZKProof proof, 
    BGIN19Field** masks_ss,
    size_t batch_size, 
    BGIN19Field sid,
    size_t prover_ID,
    size_t party_ID,
    PRNG prng
) {
    size_t k = OnlineOptions::singleton.k_size;
    size_t k2 = OnlineOptions::singleton.k2_size;

    size_t k_max = k > k2 ? k : k2;

    BGIN19Field* eval_base = new BGIN19Field[k_max];
    BGIN19Field* eval_base_2k = new BGIN19Field[2 * k_max - 1];    

    // ===============================  First Round  ===============================

    size_t T = ((batch_size - 1) / k + 1) * k;
    size_t s = (T - 1) / k + 1;
    size_t len = bgin19ProofRoundCount(batch_size, k, k2);

    vector<BGIN19Field> b_ss(len);
    BGIN19Field final_input = 0, final_result_ss = 0;

    // Transcript
    BGIN19LocalHash transcript_hash;
    transcript_hash.append_one_msg(sid);

    size_t cnt = 0;
    BGIN19Field out_ss = 0, sum_ss = 0;

    transcript_hash.append_msges(proof.p_evals_masked[cnt]);
    BGIN19Field r = transcript_hash.get_challenge();
    
    // recover proof
    bool prev_party = ((int64_t)(party_ID + 1 - prover_ID)) % 3 == 0;

    if (prev_party) {
        for (size_t i = 0; i < 2 * k - 1; i++) { 
            proof.p_evals_masked[cnt][i] += masks_ss[cnt][i];
        } 
    } else {
        for (size_t i = 0; i < 2 * k - 1; i++) { 
            proof.p_evals_masked[cnt][i] = masks_ss[cnt][i];
        }
    }

    // sample randomness betas
    BGIN19Field* betas = new BGIN19Field[k];
    for(size_t j = 0; j < k; j++) {
        betas[j] = 1; //.randomize(prng);
    }

    // compute random linear combination on the first k outputs using betas
    for(size_t j = 0; j < k; j++) { 
        sum_ss = inner_product(betas, proof.p_evals_masked[cnt], k);
    }

    size_t block_cols_num = (s - 1) / BLOCK_SIZE + 1;
    size_t total_blocks_num = (batch_size - 1) / BLOCK_SIZE + 1;
    size_t cur_k_blocks = 0;
    BGIN19ShareTupleBlock k_share_tuple_blocks[k];

    size_t padded_s = block_cols_num * BLOCK_SIZE;

    s = padded_s;

    BGIN19Field* thetas = new BGIN19Field[s];
    for(size_t j = 0; j < s; j++) {
        thetas[j].randomize(prng);
    }

    if (prev_party) {
        for (size_t block_col = 0; block_col < block_cols_num; block_col ++) {
            if (block_col == block_cols_num - 1 && total_blocks_num - cur_k_blocks < k) {
                memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * (total_blocks_num - cur_k_blocks));
                memset(k_share_tuple_blocks + total_blocks_num - cur_k_blocks, 0, sizeof(BGIN19ShareTupleBlock) * (k - (total_blocks_num - cur_k_blocks)));
            }
            else {
                memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * k);
            }

            for (size_t i = 0; i < k; i++) { 
                long this_block_value = k_share_tuple_blocks[i].rho.first;
                size_t theta_off = block_col * BLOCK_SIZE;
                unsigned long long t = (unsigned long long)this_block_value;
                while (t) {
                    unsigned int l = __builtin_ctzll(t);
                    out_ss += thetas[theta_off + l];
                    t &= t - 1;
                }
            }
            cur_k_blocks += k;
        }
    }
    else {
        for (size_t block_col = 0; block_col < block_cols_num; block_col ++) {
            if (block_col == block_cols_num - 1 && total_blocks_num - cur_k_blocks < k) {
                memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * (total_blocks_num - cur_k_blocks));
                memset(k_share_tuple_blocks + total_blocks_num - cur_k_blocks, 0, sizeof(BGIN19ShareTupleBlock) * (k - (total_blocks_num - cur_k_blocks)));
            }
            else {
                memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * k);
            }

            for (size_t i = 0; i < k; i++) { 
                long this_block_value = k_share_tuple_blocks[i].result.second ^ (k_share_tuple_blocks[i].input1.second & k_share_tuple_blocks[i].input2.second) ^ k_share_tuple_blocks[i].rho.second;
                size_t theta_off = block_col * BLOCK_SIZE;
                unsigned long long t = (unsigned long long)this_block_value;
                while (t) {
                    unsigned int l = __builtin_ctzll(t);
                    out_ss += thetas[theta_off + l];
                    t &= t - 1;
                }
            }
            cur_k_blocks += k;
        }
    }
    
    b_ss[cnt] = sum_ss - out_ss;

    BGIN19Lagrange::evaluate_bases(2 * k2 - 1, r, eval_base_2k);
    out_ss = inner_product(eval_base_2k, proof.p_evals_masked[cnt], (2 * k2 - 1));

    // ===============================  Following Rounds  ===============================

    // new evaluations at random point r
    BGIN19Lagrange::evaluate_bases(k, r, eval_base);

    s *= 2;
    size_t s0 = s;
    // use k2 as the compression parameter from the second round
    s = (s - 1) / k2 + 1;
 
    // Reuse scratch buffer for input matrix
    this->ensure_matrix(this->scratch_input_left, this->scratch_input_left_rows, k2, s);
    BGIN19Field** input = this->scratch_input_left_rows.data();

    size_t index = 0;
    cur_k_blocks = 0;

    int bits_num = k2;
    int group_num = BLOCK_SIZE / k2;
    
    uint64_t table_size = 1 << k;
    this->scratch_input_table.resize(table_size);
    BGIN19Field* input_table = this->scratch_input_table.data();

    for (uint64_t i = 0; i < table_size; i++) {
        BGIN19Field sum = 0;
        for (uint64_t j = 0; j < k; j++) {
            if ((i >> j) & 1)
                sum += eval_base[j];
        }
        input_table[i] = sum;
    }

    if (prev_party) {

        this->scratch_bit_blocks_left1.resize(k);
        this->scratch_bit_blocks_left2.resize(k);
        long* bit_blocks_left1 = this->scratch_bit_blocks_left1.data();
        long* bit_blocks_left2 = this->scratch_bit_blocks_left2.data();

        for (size_t block_col = 0; block_col < block_cols_num; block_col ++) {
            if (block_col == block_cols_num - 1 && total_blocks_num - cur_k_blocks < k) {
                memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * (total_blocks_num - cur_k_blocks));
                memset(k_share_tuple_blocks + total_blocks_num - cur_k_blocks, 0, sizeof(BGIN19ShareTupleBlock) * (k - (total_blocks_num - cur_k_blocks)));
            }
            else {
                memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * k);
            }

            for (uint64_t i = 0; i < k; i++) {

                BGIN19ShareTupleBlock cur_block = k_share_tuple_blocks[i];

                bit_blocks_left1[i] = cur_block.input2.first;
                bit_blocks_left2[i] = cur_block.input1.first;
            }

            // k = 8, bits_num = 8, group_num = 8
            // bit_id = 0, 1, 2, ..., 7
            for (int bit_id = 0; bit_id < bits_num; bit_id++) {
                // group_id = 0, 1, 2, ..., 7
                for (int group_id = 0; group_id < group_num; group_id++) {

                    // bit_id = 0:  overall_bit_id = 0, 8, 16, 24, 32, 40, 48, 56
                    // bit_id = 1:  overall_bit_id = 1, 9, 17, 25, 33, 41, 49, 57
                    // ......
                    // bit_id = 7:  overall_bit_id = 7, 15, 23, 31, 39, 47, 49, 63
                    int overall_bit_id = group_id * bits_num + bit_id;
                    int cur_index = index + overall_bit_id * 2;
                    int row = cur_index / s;
                    int col = cur_index % s;

                    if (index >= s0) {
                        if ((uint64_t)row == k2) {
                            break;
                        }
                        else {
                            input[row][col] = input[row][col + 1] = 0;
                            continue;
                        }
                    }

                    uint64_t left_id1 = 0, left_id2 = 0;
                    unsigned long long col_mask = 1ULL << overall_bit_id;

                    for (int j = 0; j < bits_num; j++) {
                        uint64_t j_bit = 1ULL << j;
                        left_id1 |= (bit_blocks_left1[j] & (long)col_mask) ? j_bit : 0ULL;
                        left_id2 |= (bit_blocks_left2[j] & (long)col_mask) ? j_bit : 0ULL;
                    }

                    input[row][col] = input_table[left_id1];
                    input[row][col + 1] = input_table[left_id2];
                } 
            }
            
            index += BLOCK_SIZE * 2;
            cur_k_blocks += k;
        }
    } 
    else {
        this->scratch_bit_blocks_right1.resize(k);
        this->scratch_bit_blocks_right2.resize(k);
        long* bit_blocks_right1 = this->scratch_bit_blocks_right1.data();
        long* bit_blocks_right2 = this->scratch_bit_blocks_right2.data();

        for (size_t block_col = 0; block_col < block_cols_num; block_col ++) {

            if (block_col == block_cols_num - 1 && total_blocks_num - cur_k_blocks < k) {
                memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * (total_blocks_num - cur_k_blocks));
                memset(k_share_tuple_blocks + total_blocks_num - cur_k_blocks, 0, sizeof(BGIN19ShareTupleBlock) * (k - (total_blocks_num - cur_k_blocks)));
            }
            else {
                memcpy(k_share_tuple_blocks, mult_triples.data() + cur_k_blocks, sizeof(BGIN19ShareTupleBlock) * k);
            }

            for (uint64_t i = 0; i < k; i++) {
                BGIN19ShareTupleBlock cur_block = k_share_tuple_blocks[i];
                bit_blocks_right1[i] = cur_block.input1.second;
                bit_blocks_right2[i] = cur_block.input2.second;
            }

            // k = 8, bits_num = 8, group_num = 8
            // bit_id = 0, 1, 2, ..., 7
            for (int bit_id = 0; bit_id < bits_num; bit_id++) {
                // group_id = 0, 1, 2, ..., 7
                for (int group_id = 0; group_id < group_num; group_id++) {

                    // bit_id = 0:  overall_bit_id = 0, 8, 16, 24, 32, 40, 48, 56
                    // bit_id = 1:  overall_bit_id = 1, 9, 17, 25, 33, 41, 49, 57
                    // ......
                    // bit_id = 7:  overall_bit_id = 7, 15, 23, 31, 39, 47, 49, 63
                    int overall_bit_id = group_id * bits_num + bit_id;
                    int cur_index = index + overall_bit_id * 2;
                    int row = cur_index / s;
                    int col = cur_index % s;

                    if (index >= s0) {
                        if ((uint64_t)row == k2) {
                            break;
                        }
                        else {
                            input[row][col] = input[row][col + 1] = 0;
                            continue;
                        }
                    }

                    uint64_t right_id1 = 0, right_id2 = 0;
                    unsigned long long col_mask = 1ULL << overall_bit_id;

                    for (int j = 0; j < bits_num; j++) {
                        uint64_t j_bit = 1ULL << j;
                        right_id1 |= (bit_blocks_right1[j] & (long)col_mask) ? j_bit : 0ULL;
                        right_id2 |= (bit_blocks_right2[j] & (long)col_mask) ? j_bit : 0ULL;
                    }

                    size_t theta_idx = block_col * BLOCK_SIZE + overall_bit_id;
                    BGIN19Field theta_val = thetas[theta_idx];
                    input[row][col] = input_table[right_id1] * theta_val;
                    input[row][col + 1] = input_table[right_id2] * theta_val;
                } 
            }
            
            index += BLOCK_SIZE * 2;
            cur_k_blocks += k;
        }
    }

    cnt++;

    while(true)
    {    
        transcript_hash.append_msges(proof.p_evals_masked[cnt]);

        if (prev_party) {
            for(size_t i = 0; i < 2 * k2 - 1; i++) { 
                proof.p_evals_masked[cnt][i] += masks_ss[cnt][i];
            } 
        } else {
            for(size_t i = 0; i < 2 * k2 - 1; i++) { 
                proof.p_evals_masked[cnt][i] = masks_ss[cnt][i];
            }
        }

        sum_ss = 0;
        for (size_t j = 0; j < k2; j++) { 
            sum_ss += proof.p_evals_masked[cnt][j];
        }

        b_ss[cnt] = sum_ss - out_ss;

        if (s == 1) {
            r = transcript_hash.get_challenge();

            BGIN19Lagrange::evaluate_bases(k2, r, eval_base);
            auto tmp_final_input = inner_product_first_dim(eval_base, input, k2, 0);
            final_input += tmp_final_input;

            BGIN19Lagrange::evaluate_bases(2 * k2 - 1, r, eval_base_2k);
            final_result_ss = inner_product(eval_base_2k, proof.p_evals_masked[cnt], (2 * k2 - 1));

            break;
        }
        
        r = transcript_hash.get_challenge();

        BGIN19Lagrange::evaluate_bases(2 * k2 - 1, r, eval_base_2k);
        out_ss = inner_product(eval_base_2k, proof.p_evals_masked[cnt], (2 * k2 - 1));

        BGIN19Lagrange::evaluate_bases(k2, r, eval_base);

        s0 = s;
        s = (s - 1) / k2 + 1;

        for (size_t i = 0; i < k2; i++) {
            for (size_t j = 0; j < s; j++) {
                index = i * s + j;
                if (index < s0) {
                    input[i][j] = inner_product_first_dim(eval_base, input, k2,
                            index);
                }
                else {
                    input[i][j] = 0;
                }
            }
        }

        cnt++;
    }

    delete[] eval_base;
    delete[] eval_base_2k;

    // no delete needed for scratch buffer

    for (size_t j = 0; j < len; j ++) {
        delete[] masks_ss[j];
    }
    delete[] masks_ss;

    BGIN19VerMsg vermsg(
        b_ss,
        final_input,
        final_result_ss
    );

    return vermsg;
}

template <class U>
bool BGIN19MC<U>::_verify(
    BGIN19DZKProof proof, 
    BGIN19VerMsg other_vermsg, 
    BGIN19Field** masks_ss,
    size_t batch_size, 
    BGIN19Field sid,
    size_t prover_ID,
    size_t party_ID,
    PRNG prng
) {
    size_t k = OnlineOptions::singleton.k_size;
    size_t k2 = OnlineOptions::singleton.k2_size;
    
    size_t len = bgin19ProofRoundCount(batch_size, k, k2);
    
    BGIN19VerMsg self_vermsg = _gen_vermsg(proof, masks_ss, batch_size, sid, prover_ID, party_ID, prng);

    BGIN19Field b;

    for(size_t i = 0; i < len; i++) {
        b = self_vermsg.b_ss[i] + other_vermsg.b_ss[i];
        
        if(b != 0) {
            cerr << "BGIN19 check failed in compression round " << i << endl;
            return false;
        }
    }
    BGIN19Field res = self_vermsg.final_input * other_vermsg.final_input;
    BGIN19Field p_eval_r = self_vermsg.final_result_ss + other_vermsg.final_result_ss;
    
    BGIN19Field diff = res - p_eval_r;
    if(diff != 0) {
        cerr << "BGIN19 check failed in final multiplication" << endl;
        return false;
    } 

    return true;
}

#endif