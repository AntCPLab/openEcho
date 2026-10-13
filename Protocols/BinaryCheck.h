#ifndef PROTOCOLS_BINARYCHECK_H_
#define PROTOCOLS_BINARYCHECK_H_

#include <vector>
#include "Tools/Hash.h"
#include "Math/mersenne.hpp"
#include "Tools/my-utils.hpp"

using namespace std;

#define BLOCK_SIZE 64

typedef unsigned __int128 uint128_t;
typedef uint64_t Field;

class PrimeFieldTranscript {
    octetStream buffer;

public:
    template <typename T>
    void update(const T& data) {
        buffer.store(data);
    }

    void append_one_msg(Field msg) {
        update(msg);
    }

    void append_msges(const vector<Field>& messages) {
        for (auto message : messages)
            update(message);
    }

    Field get_challenge(size_t excluded_upper_bound) {
        Hash hash;
        hash.reset();
        hash.update(buffer);

        octetStream digest;
        hash.final(digest);
        Field challenge;
        digest.get(challenge);
        challenge = Mersenne::modp(challenge);

        // Interpolation points are 0,...,lambda in the final round and
        // 1,...,lambda otherwise. Mapping the negligible low range away
        // avoids singular/chosen interpolation points deterministically.
        if (challenge <= excluded_upper_bound)
            challenge += excluded_upper_bound + 1;
        return challenge;
    }
};



struct DZKProof {
    vector<vector<Field>> p_evals_masked;

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

    Field get_hash() {
        LocalHash<Field> hash;
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
        if (num_p_evals_masked > 128)
            throw length_error("USS23 proof has too many rounds");
        p_evals_masked.resize(num_p_evals_masked);
        for (size_t i = 0; i < num_p_evals_masked; i++) {
            os.get(num_p_evals_masked_each);
            if (num_p_evals_masked_each > 1024)
                throw length_error("USS23 proof row is too large");
            p_evals_masked[i].resize(num_p_evals_masked_each);
            for (size_t j = 0; j < num_p_evals_masked_each; j++) {
                os.get(p_evals_masked[i][j]);
            }
        }
    }
};

struct VerMsg {
    vector<Field> b_ss;
    Field final_input;
    Field final_result_ss;

    VerMsg() {}
    VerMsg(vector<Field> b_ss, Field final_input, Field final_result_ss) {
        this->b_ss = b_ss;
        this->final_input = final_input;
        this->final_result_ss = final_result_ss;
    }

    Field get_hash() {
        LocalHash<Field> hash;
        for (Field each: b_ss) {
            hash.update(each);
        }
        hash.update(final_input);
        hash.update(final_result_ss);
        return hash.final();
    }

    void pack(octetStream &os) {
        os.store(b_ss.size());
        for(uint64_t i = 0; i < b_ss.size(); i++) {
            os.store(b_ss[i]);
        }
        os.store(final_input);
        os.store(final_result_ss);
    }

    void unpack(octetStream &os) {
        uint64_t size = 0;
        os.get(size);
        if (size > 128)
            throw length_error("USS23 verifier message is too large");
        b_ss.resize(size);
        for(uint64_t i = 0; i < size; i++) {
            os.get(b_ss[i]);
        }
        os.get(final_input);
        os.get(final_result_ss);
    }
};

class Lagrange {
public:
    static void get_bases(uint64_t n, Field** result);
    static void evaluate_bases(uint64_t n, Field r, Field* result);
};

inline void Lagrange::get_bases(uint64_t n, Field** result) {
    for (uint64_t i = 0; i < n - 1; i++) {
        for(uint64_t j = 0; j < n; j++) {
            result[i][j] = 1;
            for(uint64_t l = 0; l < n; l++) {
                if (l != j) {
                    Field denominator, numerator;
                    if (j > l) {
                        denominator = j - l;
                    }
                    else {
                        denominator = Mersenne::neg(l - j);
                    }
                    numerator = i + n - l;
                    result[i][j] = Mersenne::mul(result[i][j], Mersenne::mul(Mersenne::inverse(denominator), numerator));
                }
            }
        }
    }
}

inline void Lagrange::evaluate_bases(uint64_t n, Field r, Field* result) {
    for(uint64_t i = 0; i < n; i++) {
        result[i] = 1;
        for(uint64_t j = 0; j < n; j++) {
            if (j != i) {
                Field denominator, numerator; 
                if (i > j) { 
                    denominator = i - j;
                } 
                else { 
                    denominator = Mersenne::neg(j - i);
                }
                if (r > j) { 
                    numerator = r - j; 
                } 
                else { 
                    numerator = Mersenne::neg(j - r);
                }
                result[i] = Mersenne::mul(result[i], Mersenne::mul(Mersenne::inverse(denominator), numerator));
            }
        }
    }
}

#endif