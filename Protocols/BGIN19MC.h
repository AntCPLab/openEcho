#ifndef PROTOCOLS_BGIN19MC_H_
#define PROTOCOLS_BGIN19MC_H_

#include "Protocols/MaliciousRepMC.h"
#include "Protocols/Replicated.h"
#include "Tools/FastVector.h"
#include <vector>
#include "Tools/Hash.h"
#include "Math/gf2n.h"
#include "Math/gf28.h"
#include "Tools/SafeQueue.h"

#include <limits>

#define BLOCK_SIZE 64

typedef unsigned __int128 uint128_t;
typedef gf28 BGIN19Field;

typedef MyPair<long, long> ShareTypeBlock;

struct BGIN19ShareTupleBlock {
public:
    ShareTypeBlock input1, input2, result, rho;
    BGIN19ShareTupleBlock(): input1(), input2(), result(), rho() {}
};

class BGIN19LocalHash {
    octetStream buffer;
public:

    template <typename T>
    void update(T data) {
        buffer.store(data);
    }

    BGIN19Field final() {
        Hash hash;
        hash.reset();
        hash.update(buffer);
        BGIN19Field result;
        octetStream res;
        hash.final(res);
        res.get(result);
        buffer.reset_write_head();
        return result;
    }

    void append_one_msg(BGIN19Field msg) {
        update(msg);
    }

    void append_msges(vector<BGIN19Field> msges) {
        for(BGIN19Field msg: msges) {
            update(msg);
        }
    }

    BGIN19Field get_challenge() {
        BGIN19Field r = final();
        return r;
    }
};

struct BGIN19DZKProof {
    vector<vector<BGIN19Field>> p_evals_masked;

    void print_out() {
        cout << "proof: ";
        for(auto row: p_evals_masked) {
            for(auto x: row) {
                cout << x << " ";
            }
        }
        cout << endl;
    }

    size_t get_size() {
        size_t size = 0;
        for (auto& v : p_evals_masked) {
            size += v.size();
        }
        return size;
    }

    BGIN19Field get_hash() {
        BGIN19LocalHash hash;
        for (auto p_eval : p_evals_masked) {
            for (auto each : p_eval) {
                hash.update(each);
            }
        }
        return hash.final();
    }

    void pack(octetStream &os) {
        os.store(p_evals_masked.size());
        for (auto each: p_evals_masked) {
            os.store(each.size());
            for (auto each_eval: each) {
                os.store(each_eval);
            }
        }
    }

    void unpack(octetStream &os) {
        size_t num_p_evals_masked = 0;
        size_t num_p_evals_masked_each = 0;

        os.get(num_p_evals_masked);
        p_evals_masked.resize(num_p_evals_masked);
        for (size_t i = 0; i < num_p_evals_masked; i++) {
            os.get(num_p_evals_masked_each);
            p_evals_masked[i].resize(num_p_evals_masked_each);
            for (size_t j = 0; j < num_p_evals_masked_each; j++) {
                os.get(p_evals_masked[i][j]);
            }
        }
    }
};

struct BGIN19VerMsg {
    vector<BGIN19Field> b_ss;
    BGIN19Field final_input;
    BGIN19Field final_result_ss;

    BGIN19VerMsg() {}
    BGIN19VerMsg(vector<BGIN19Field> b_ss, BGIN19Field final_input, BGIN19Field final_result_ss) {
        this->b_ss = b_ss;
        this->final_input = final_input;
        this->final_result_ss = final_result_ss;
    }

    BGIN19Field get_hash() {
        BGIN19LocalHash hash;
        for (BGIN19Field each: b_ss) {
            hash.update(each);
        }
        hash.update(final_input);
        hash.update(final_result_ss);
        return hash.final();
    }

    void pack(octetStream &os) {
        os.store(b_ss.size());
        for(size_t i = 0; i < b_ss.size(); i++) {
            os.store(b_ss[i]);
        }
        os.store(final_input);
        os.store(final_result_ss);
    }

    void unpack(octetStream &os) {
        size_t size = 0;
        os.get(size);
        b_ss.resize(size);
        for(size_t i = 0; i < size; i++) {
            os.get(b_ss[i]);
        }
        os.get(final_input);
        os.get(final_result_ss);
    }
};

class BGIN19Lagrange {
public:
    static void get_bases(size_t n, BGIN19Field** result);
    static void evaluate_bases(size_t n, BGIN19Field r, BGIN19Field* result);
};

inline void BGIN19Lagrange::get_bases(size_t n, BGIN19Field** result) {
    for (size_t i = 0; i < n - 1; i++) {
        for(size_t j = 0; j < n; j++) {
            result[i][j] = 1;
            for(size_t l = 0; l < n; l++) {
                if (l != j) {
                    BGIN19Field denominator, numerator;
                    denominator = BGIN19Field(j) - BGIN19Field(l);
                    numerator = BGIN19Field(i + n) - BGIN19Field(l);
                    result[i][j] = result[i][j] * denominator.invert() * numerator;
                }
            }
        }
    }
}

inline void BGIN19Lagrange::evaluate_bases(size_t n, BGIN19Field r, BGIN19Field* result) {
    for(size_t i = 0; i < n; i++) {
        result[i] = 1;
        for(size_t j = 0; j < n; j++) {
            if (j != i) {
                BGIN19Field denominator, numerator; 
                denominator = BGIN19Field(i) - BGIN19Field(j);
                numerator = r - BGIN19Field(j);
                result[i] = result[i] * denominator.invert() * numerator;
            }
        }
    }
}

template <class T>
class BGIN19MC : public MaliciousRepMC<T> {
    using clear = typename T::clear;
    
    HashMaliciousRepMC<T> _impl;
    FastVector<BGIN19ShareTupleBlock> mult_triples;
    size_t trip_pointer;
    unique_ptr<ReplicatedBase> prng_protocol;

    array<octetStream, 2> proof_os, vermsg_os;
    PRNG global_prng;
    BGIN19Field sid;

    bool passed;

    // Scratch buffers to avoid frequent allocations
    // Matrices stored row-major in contiguous buffers
    FastVector<BGIN19Field> scratch_eval_result;         // size up to k_max * k_max
    vector<BGIN19Field*> scratch_eval_result_rows;   // row pointers

    FastVector<BGIN19Field> scratch_input_left;          // size up to k2 * s
    FastVector<BGIN19Field> scratch_input_right;         // size up to k2 * s
    vector<BGIN19Field*> scratch_input_left_rows;    // row pointers
    vector<BGIN19Field*> scratch_input_right_rows;   // row pointers

    FastVector<BGIN19Field> scratch_eval_p_poly;         // size up to 2 * k_max - 1
    FastVector<BGIN19Field> scratch_eval_base;           // size up to k_max
    FastVector<BGIN19Field> scratch_eval_base_2k;        // size up to 2 * k_max - 1
    FastVector<BGIN19Field> scratch_input_table;         // size up to (1 << k)
    FastVector<BGIN19Field> scratch_thetas;              // size up to s

    FastVector<long> scratch_bit_blocks_left1;           // size up to k
    FastVector<long> scratch_bit_blocks_left2;           // size up to k
    FastVector<long> scratch_bit_blocks_right1;          // size up to k
    FastVector<long> scratch_bit_blocks_right2;          // size up to k

    // Mask pools reused across invocations
    vector<vector<BGIN19Field>> masks_pool;
    vector<vector<BGIN19Field>> mask_ss_next_pool;
    vector<vector<BGIN19Field>> mask_ss_prev_pool;

    template<class Cont>
    inline void ensure_matrix(Cont& mat,
                              vector<BGIN19Field*>& rows,
                              size_t r, size_t c)
    {
        if (mat.size() < r * c) mat.resize(r * c);
        if (rows.size() < r) rows.resize(r);
        for (size_t i = 0; i < r; ++i)
            rows[i] = mat.data() + i * c;
    }

public:

    Player* P;

    explicit BGIN19MC(HashMaliciousRepMC<T>& impl) : MaliciousRepMC<T>(), _impl(impl) {}
    BGIN19MC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) : BGIN19MC()
    { (void)_; (void)__; (void)___; }
    
    // emulate Direct_MAC_Check
    BGIN19MC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0) : BGIN19MC()
    { (void)_; (void)__; (void)___; (void)____; }

    BGIN19MC() : _impl(), passed(true), P(0) {
        mult_triples.clear();
        trip_pointer = 0;
        mult_triples.reserve(1 << 26);
    }

    ~BGIN19MC() {
        if (P) {
            MaybeCheck(*P, true);
            cout << "Binary check passed? " << passed << endl;
        }
    }

    void init(Player& P) {
        if (!prng_protocol) {
            prng_protocol = make_unique<ReplicatedBase>(P);
        }
        this->P = &P;

        global_prng.SeedGlobally(P);
        sid.randomize(global_prng);
        
        BGIN19Field::reset();
        BGIN19Field::init_field(8);
    }

    void POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        _impl.POpen(values, S, P);
    }
    void POpen_End(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        _impl.POpen_End(values, S, P);
    }

    virtual typename T::open_type finalize_raw() {
        return _impl.finalize_raw();
    }

    void CheckFor(const typename T::open_type& value, const vector<T>& shares, const Player& P) {
        _impl.CheckFor(value, shares, P);
    }

    void add_triple(const BGIN19ShareTupleBlock& triple) {
        mult_triples.push_back(triple);
    }

    void add_pre(const long& tmp0, const long& tmp1, const long& x0,
            const long& x1, const long& y0, const long& y1, int n) {
        const auto mask = n >= int(8 * sizeof(unsigned long)) ? ~0UL
                : (1UL << n) - 1;
        mult_triples.push_back({});
        auto& triple = mult_triples[mult_triples.size() - 1];
        triple.rho = ShareTypeBlock(tmp0 & mask, tmp1 & mask);
        triple.input1 = ShareTypeBlock(x0 & mask, x1 & mask);
        triple.input2 = ShareTypeBlock(y0 & mask, y1 & mask);
    }

    void add_post(const long& result0, const long& result1, int n) {
        if (trip_pointer >= mult_triples.size()) {
            throw runtime_error("No pre-multiplier data available");
        }
        const auto mask = n >= int(8 * sizeof(unsigned long)) ? ~0UL
                : (1UL << n) - 1;
        mult_triples[trip_pointer].result = ShareTypeBlock(result0 & mask,
                result1 & mask);
        trip_pointer++;
    }

    void Check(const Player& P) {
        _impl.Check(P);
        MaybeCheck(P, false);
    }
    void MaybeCheck(const Player& P, bool final = false);

    BGIN19MC& get_part_MC() {
        return *this;
    }

    BGIN19DZKProof _prove(
        BGIN19Field** masks,
        size_t batch_size, 
        BGIN19Field sid,
        PRNG prng
    );

    BGIN19VerMsg _gen_vermsg(
        BGIN19DZKProof proof, 
        BGIN19Field** masks_ss,
        size_t batch_size, 
        BGIN19Field sid,
        size_t prover_ID,
        size_t party_ID,
        PRNG prng
    );

    bool _verify(
        BGIN19DZKProof proof, 
        BGIN19VerMsg other_vermsg, 
        BGIN19Field** masks_ss,
        size_t batch_size, 
        BGIN19Field sid,
        size_t prover_ID,
        size_t party_ID,
        PRNG prng
    );

    void Check_one(const Player& P, size_t block_count);
    void verify();


};

#endif