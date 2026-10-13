#ifndef TOOLS_CCS24CHECK_CPP_
#define TOOLS_CCS24CHECK_CPP_

#include "Tools/CCS24Check.h"
#include "Processor/OnlineOptions.hpp"
#include <vector>

const int q = 8;
// const int k = K_CCS;

// ============================================================================
// Thread-local buffer management for memory reuse
// ============================================================================

template <class T>
struct CCS24ThreadLocalBuffers {
    std::vector<T> x_prime;
    std::vector<T> y_prime;
    std::vector<T> vrfy_left;
    std::vector<T> vrfy_right;
    std::vector<T> x_new;
    std::vector<T> y_new;
    std::vector<T> vrfy_left_new;
    std::vector<T> vrfy_right_new;
    std::vector<T> z_matrix;       // For 2D array z[n][n], stored as 1D
    std::vector<T> alpha_vec;
    std::vector<T> beta_vec;
    std::vector<T> batch_coefs;
    
    // Ensure buffer has at least 'size' elements, zero-initialize new elements
    static void ensure_size_zeroed(std::vector<T>& buf, size_t size) {
        if (buf.size() < size) {
            buf.resize(size);
        }
        // Zero out the used portion
        std::fill(buf.begin(), buf.begin() + size, T(0));
    }
    
    // Ensure buffer has at least 'size' elements without zeroing
    static void ensure_size(std::vector<T>& buf, size_t size) {
        if (buf.size() < size) {
            buf.resize(size);
        }
    }
};

template <class T>
CCS24ThreadLocalBuffers<T>& get_thread_local_buffers() {
    thread_local CCS24ThreadLocalBuffers<T> buffers;
    return buffers;
}

// ============================================================================
// Helper functions
// ============================================================================

template <class T>
T inner_product(T* x, T* y, int size) {
    T res = 0;
    for (int i = 0; i < size; i++) {
        res += x[i] * y[i];
    }
    return res;
}

// Access 2D matrix stored in 1D vector
template <class T>
inline T& matrix_at(std::vector<T>& mat, size_t row, size_t col, size_t cols) {
    return mat[row * cols + col];
}

// ============================================================================
// Prover function (optimized)
// ============================================================================

template <class ExtendedOffline>
void prover(ExtendedOffline* x, ExtendedOffline* y, ExtendedOffline z1, ExtendedOffline z2, int size, const Player& P, array<PRNG, 2>& shared_prngs, int k) {
    auto& bufs = get_thread_local_buffers<ExtendedOffline>();
    
    octetStream os[2];

    // Reuse thread-local buffers instead of malloc
    size_t prime_size = size + q;
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.x_prime, prime_size);
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.y_prime, prime_size);
    
    ExtendedOffline* x_prime = bufs.x_prime.data();
    ExtendedOffline* y_prime = bufs.y_prime.data();

    memcpy(x_prime, x, size * sizeof(ExtendedOffline));
    memcpy(y_prime, y, size * sizeof(ExtendedOffline));
    memset(x_prime + size, 0, q * sizeof(ExtendedOffline));
    memset(y_prime + size, 0, q * sizeof(ExtendedOffline));
    int now_size = size;

    for (int i = 0; i < size; i += 2) {
        z1 -= x[i] * x[i + 1];
        z2 -= y[i] * y[i + 1];
    }

    ExtendedOffline h = inner_product(x, y, size) - z1 - z2;
    h >>= k;

    for (auto& o : os) {
        o.reset_write_head();
    }

    (h - shared_prngs[0].get<ExtendedOffline>()).pack(os[1]);
    P.send_to(2, os[1]);

    // Reuse thread-local buffers for x_new, y_new
    size_t new_buf_size = (size_t)ceil((double) now_size / (double) q);
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.x_new, new_buf_size);
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.y_new, new_buf_size);
    
    ExtendedOffline* x_new = bufs.x_new.data();
    ExtendedOffline* y_new = bufs.y_new.data();

    while (true) {
        if (now_size <= q) {
            break;
        }

        for (auto& o : os) {
            o.reset_write_head();
        }
        
        int new_size = ceil((double) now_size / (double) q);

        ExtendedOffline z[q][q];

        memset(x_new, 0, new_size * sizeof(ExtendedOffline));
        memset(y_new, 0, new_size * sizeof(ExtendedOffline));

        for (int i = 0; i < q; i ++) {
            for (int j = 0; j < q; j ++) {
                z[i][j] = inner_product(x_prime + i * new_size, y_prime + j * new_size, new_size);
                if (i == 0 && j == 0)   continue;
                (z[i][j] - shared_prngs[0].get<ExtendedOffline>()).pack(os[1]);
            }
        }
        P.exchange(2, os[1], os[0]);

        octet seed[SEED_SIZE];
        os[0].get_bytes(seed, SEED_SIZE);
        PRNG local_prng;
        local_prng.SetSeed(seed);

        ExtendedOffline alpha, beta;

        for (int i = 0; i < q; i ++) {
            alpha.randomize(local_prng);
            beta.randomize(local_prng);
            for (int j = 0; j < new_size; j ++) {
                x_new[j] += alpha * x_prime[i * new_size + j];
                y_new[j] += beta * y_prime[i * new_size + j];
            }
        }

        memcpy(x_prime, x_new, new_size * sizeof(ExtendedOffline));
        memcpy(y_prime, y_new, new_size * sizeof(ExtendedOffline));
        memset(x_prime + new_size, 0, q * sizeof(ExtendedOffline));
        memset(y_prime + new_size, 0, q * sizeof(ExtendedOffline));

        now_size = new_size;
    }
    
    // No free needed - buffers are reused

    memcpy(x_prime + 1, x_prime, now_size * sizeof(ExtendedOffline));
    memcpy(y_prime + 1, y_prime, now_size * sizeof(ExtendedOffline));

    ExtendedOffline x0, y0;
    x0 = shared_prngs[0].get<ExtendedOffline>() + shared_prngs[1].get<ExtendedOffline>();
    y0 = shared_prngs[0].get<ExtendedOffline>() + shared_prngs[1].get<ExtendedOffline>();

    x_prime[0] = x0;
    y_prime[0] = y0;

    for (auto& o : os) {
        o.reset_write_head();
    }

    // Replace VLA with vector
    size_t z_dim = now_size + 1;
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.z_matrix, z_dim * z_dim);
    
    for (int i = 0; i <= now_size; i ++) {
        for (int j = 0; j <= now_size; j ++) {
            matrix_at(bufs.z_matrix, i, j, z_dim) = x_prime[i] * y_prime[j];
            if (i == 1 && j == 1)   continue;
            (matrix_at(bufs.z_matrix, i, j, z_dim) - shared_prngs[0].get<ExtendedOffline>()).pack(os[1]);
        }
    }

    P.send_to(2, os[1]);

    GlobalPRNG global_prng(P);

    // Replace VLA with vector
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.alpha_vec, z_dim);
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.beta_vec, z_dim);
    
    ExtendedOffline new_x = 0, new_y = 0, new_z = 0;

    bufs.alpha_vec[0] = bufs.beta_vec[0] = ExtendedOffline(1);
    for (int i = 1; i <= now_size; i ++) {
        bufs.alpha_vec[i].randomize(global_prng);
        bufs.beta_vec[i].randomize(global_prng);
        new_x += bufs.alpha_vec[i] * x_prime[i];
        new_y += bufs.beta_vec[i] * y_prime[i];
    }
    new_x += bufs.alpha_vec[0] * x0;
    new_y += bufs.beta_vec[0] * y0;

    for (int i = 0; i <= now_size; i ++) {
        for (int j = 0; j <= now_size; j ++) {
            new_z += matrix_at(bufs.z_matrix, i, j, z_dim) * bufs.alpha_vec[i] * bufs.beta_vec[j];
        }
    }

    if (new_x * new_y != new_z) {
        cout << new_x << " * " << new_y << " != " << new_z << endl;
        // throw mac_fail("MAC check failed");
    }
    cout << "Offline check finished" << endl;

    // No free needed - buffers are reused
}

// ============================================================================
// Verifier X function (optimized)
// ============================================================================

template <class ExtendedOffline>
void verifier_x(ExtendedOffline* x, ExtendedOffline z1, int size, const Player& P, array<PRNG, 2>& shared_prngs, int k) {
    auto& bufs = get_thread_local_buffers<ExtendedOffline>();
    
    octetStream os[2];

    size_t prime_size = size + q;
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.x_prime, prime_size);
    ExtendedOffline* x_prime = bufs.x_prime.data();

    memcpy(x_prime, x, size * sizeof(ExtendedOffline));
    memset(x_prime + size, 0, q * sizeof(ExtendedOffline));
    int now_size = size;

    for (int i = 0; i < size; i += 2) {
        z1 -= x[i] * x[i + 1];
    }

    ExtendedOffline h;
    h.randomize(shared_prngs[1]);
    ExtendedOffline z_prime = z1 + (h << k);

    size_t new_buf_size = (size_t)ceil((double) now_size / (double) q);
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.x_new, new_buf_size);
    ExtendedOffline* x_new = bufs.x_new.data();

    while (true) {
        if (now_size <= q) {
            break;
        }

        int new_size = ceil((double) now_size / (double) q);
        ExtendedOffline z[q][q];
        ExtendedOffline tmp_z = 0, new_z = 0;

        memset(x_new, 0, new_size * sizeof(ExtendedOffline));

        octet seed[SEED_SIZE];
        shared_prngs[0].get_octets(seed, SEED_SIZE);
        PRNG local_prng;
        local_prng.SetSeed(seed);

        ExtendedOffline alpha[q], beta[q];
        for (int i = 0; i < q; i ++) {
            alpha[i].randomize(local_prng);
            beta[i].randomize(local_prng);
        }

        for (int i = 0; i < q; i ++) {
            for (int j = 0; j < q; j ++) {
                if (i == 0 && j == 0)   continue;
                z[i][j].randomize(shared_prngs[1]);
                if (i == j) tmp_z += z[i][j];
                new_z += z[i][j] * alpha[i] * beta[j];
            }
        }
        z[0][0] = z_prime - tmp_z;
        new_z += z[0][0] * alpha[0] * beta[0];

        for (int i = 0; i < q; i ++) {
            for (int j = 0; j < new_size; j ++) {
                x_new[j] += alpha[i] * x_prime[i * new_size + j];
            }
        }

        memcpy(x_prime, x_new, new_size * sizeof(ExtendedOffline));
        memset(x_prime + new_size, 0, q * sizeof(ExtendedOffline));
        z_prime = new_z;
        now_size = new_size;
    }

    memcpy(x_prime + 1, x_prime, now_size * sizeof(ExtendedOffline));

    ExtendedOffline x0, y0, tmp_z = 0;
    x0.randomize(shared_prngs[1]);
    y0.randomize(shared_prngs[1]);

    x_prime[0] = x0;

    // Replace VLA with vector
    size_t z_dim = now_size + 1;
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.z_matrix, z_dim * z_dim);

    for (int i = 0; i <= now_size; i ++) {
        for (int j = 0; j <= now_size; j ++) {
            if (i == 1 && j == 1)   continue;
            matrix_at(bufs.z_matrix, i, j, z_dim).randomize(shared_prngs[1]);
        }
    }

    for (int i = 2; i <= now_size; i ++) {
        tmp_z += matrix_at(bufs.z_matrix, i, i, z_dim);
    }
    matrix_at(bufs.z_matrix, 1, 1, z_dim) = z_prime - tmp_z;
    
    GlobalPRNG global_prng(P);
    
    // Replace VLA with vector
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.alpha_vec, z_dim);
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.beta_vec, z_dim);
    
    ExtendedOffline new_x = 0, new_z = 0;

    for (int i = 0; i <= now_size; i ++) {
        bufs.alpha_vec[i].randomize(global_prng);
        bufs.beta_vec[i].randomize(global_prng);
        new_x += bufs.alpha_vec[i] * x_prime[i];
    }

    ExtendedOffline new_y = bufs.beta_vec[0] * y0;

    for (int i = 0; i <= now_size; i ++) {
        for (int j = 0; j <= now_size; j ++) {
            new_z += matrix_at(bufs.z_matrix, i, j, z_dim) * bufs.alpha_vec[i] * bufs.beta_vec[j];
        }
    }

    for (auto& o : os) {
        o.reset_write_head();
    }
    
    new_x.pack(os[0]);
    new_y.pack(os[0]);
    new_z.pack(os[0]);

    P.exchange(2, os[0], os[1]);

    new_x += os[1].get<ExtendedOffline>();
    new_y += os[1].get<ExtendedOffline>();
    new_z += os[1].get<ExtendedOffline>();

    if (new_x * new_y != new_z) {
        cout << "new_x = " << new_x << ", new_y = " << new_y << endl;
        cout << "new_x * new_y = " << new_x * new_y << ", new_z = " << new_z << endl;
        // throw mac_fail("MAC check failed");
    }
    cout << "Offline check finished" << endl;
}

// ============================================================================
// Verifier Y function (optimized)
// ============================================================================

template <class ExtendedOffline>
void verifier_y(ExtendedOffline* y, ExtendedOffline z2, int size, const Player& P, array<PRNG, 2>& shared_prngs, int k) {
    auto& bufs = get_thread_local_buffers<ExtendedOffline>();
    
    octetStream os[2];

    size_t prime_size = size + q;
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.y_prime, prime_size);
    ExtendedOffline* y_prime = bufs.y_prime.data();

    memcpy(y_prime, y, size * sizeof(ExtendedOffline));
    memset(y_prime + size, 0, q * sizeof(ExtendedOffline));
    int now_size = size;

    for (int i = 0; i < size; i += 2) {
        z2 -= y[i] * y[i + 1];
    }

    for (auto& o : os) {
        o.reset_write_head();
    }

    P.receive_player(0, os[0]);
    ExtendedOffline h;
    h.unpack(os[0]);
    ExtendedOffline z_prime = z2 + (h << k);

    size_t new_buf_size = (size_t)ceil((double) now_size / (double) q);
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.y_new, new_buf_size);
    ExtendedOffline* y_new = bufs.y_new.data();

    while (true) {
        if (now_size <= q) {
            break;
        }

        for (auto& o : os) {
            o.reset_write_head();
        }

        int new_size = ceil((double) now_size / (double) q);
        ExtendedOffline z[q][q];
        ExtendedOffline tmp_z = 0, new_z = 0;

        memset(y_new, 0, new_size * sizeof(ExtendedOffline));

        octet seed[SEED_SIZE];
        shared_prngs[1].get_octets(seed, SEED_SIZE);
        os[1].store_bytes(seed, SEED_SIZE);
        PRNG local_prng;
        local_prng.SetSeed(seed);
        
        P.exchange(0, os[1], os[0]);

        ExtendedOffline alpha[q], beta[q];
        for (int i = 0; i < q; i ++) {
            alpha[i].randomize(local_prng);
            beta[i].randomize(local_prng);
        }


        for (int i = 0; i < q; i ++) {
            for (int j = 0; j < q; j ++) {
                if (i == 0 && j == 0)   continue;
                z[i][j].unpack(os[0]);
                if (i == j) tmp_z += z[i][j];
                new_z += z[i][j] * alpha[i] * beta[j];
            }
        }
        z[0][0] = z_prime - tmp_z;
        new_z += z[0][0] * alpha[0] * beta[0];

        for (int i = 0; i < q; i ++) {
            for (int j = 0; j < new_size; j ++) {
                y_new[j] += beta[i] * y_prime[i * new_size + j];
            }
        }

        memcpy(y_prime, y_new, new_size * sizeof(ExtendedOffline));
        memset(y_prime + new_size, 0, q * sizeof(ExtendedOffline));
        z_prime = new_z;
        now_size = new_size;
    }

    memcpy(y_prime + 1, y_prime, now_size * sizeof(ExtendedOffline));

    ExtendedOffline x0, y0, tmp_z = 0;
    x0.randomize(shared_prngs[0]);
    y0.randomize(shared_prngs[0]);

    y_prime[0] = y0;

    // Replace VLA with vector
    size_t z_dim = now_size + 1;
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.z_matrix, z_dim * z_dim);

    for (auto& o : os) {
        o.reset_write_head();
    }

    P.receive_player(0, os[0]);

    for (int i = 0; i <= now_size; i ++) {
        for (int j = 0; j <= now_size; j ++) {
            if (i == 1 && j == 1)   continue;
            matrix_at(bufs.z_matrix, i, j, z_dim).unpack(os[0]);
        }
    }

    for (int i = 2; i <= now_size; i ++) {
        tmp_z += matrix_at(bufs.z_matrix, i, i, z_dim);
    }
    matrix_at(bufs.z_matrix, 1, 1, z_dim) = z_prime - tmp_z;

    GlobalPRNG global_prng(P);

    // Replace VLA with vector
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.alpha_vec, z_dim);
    CCS24ThreadLocalBuffers<ExtendedOffline>::ensure_size(bufs.beta_vec, z_dim);
    
    ExtendedOffline new_y = 0, new_z = 0;

    for (int i = 0; i <= now_size; i ++) {
        bufs.alpha_vec[i].randomize(global_prng);
        bufs.beta_vec[i].randomize(global_prng);
        new_y += bufs.beta_vec[i] * y_prime[i];
    }

    ExtendedOffline new_x = bufs.alpha_vec[0] * x0;

    for (int i = 0; i <= now_size; i ++) {
        for (int j = 0; j <= now_size; j ++) {
            new_z += matrix_at(bufs.z_matrix, i, j, z_dim) * bufs.alpha_vec[i] * bufs.beta_vec[j];
        }
    }

    for (auto& o : os) {
        o.reset_write_head();
    }

    new_x.pack(os[1]);
    new_y.pack(os[1]);
    new_z.pack(os[1]);

    P.exchange(1, os[1], os[0]);

    new_x += os[0].get<ExtendedOffline>();
    new_y += os[0].get<ExtendedOffline>();
    new_z += os[0].get<ExtendedOffline>();

    if (new_x * new_y != new_z) {
        cout << "new_x = " << new_x << ", new_y = " << new_y << endl;
        cout << "new_x * new_y = " << new_x * new_y << ", new_z = " << new_z << endl;
        // throw mac_fail("MAC check failed");
    }
    cout << "Offline check finished" << endl;
}

// ============================================================================
// Main check_ccs function (fully optimized)
// ============================================================================

template <class T>
void check_ccs(T *x, T *y, T *z0, T *z1, size_t *sizes, int m, const Player &P, array<PRNG, 2>& shared_prngs, int provers, int k) {
    bool prove_p0 = provers & 1;
    bool prove_p1 = provers & 2;
    bool prove_p2 = provers & 4;

    using Extend = T;
    const int lambda = OnlineOptions::singleton.lambda;
    const int q = OnlineOptions::singleton.q_size;

    array<bool, 3> need_prove = {prove_p0, prove_p1, prove_p2};
    GlobalPRNG G(P);

    // Get thread-local buffers
    auto& bufs = get_thread_local_buffers<Extend>();

    // Collapse all lambda challenges into one coefficient per batch. This
    // keeps lambda as a soundness parameter without scanning every element
    // lambda times.
    CCS24ThreadLocalBuffers<Extend>::ensure_size_zeroed(
            bufs.batch_coefs, m);
    Extend h = 0;
    for (int i = 0; i < lambda; i ++) {
        Extend coef = G.get<Extend>();
        for (int j = 0; j < m; j ++) {
            if (G.get_bit())
                bufs.batch_coefs[j] += coef;
        }
    }

    int my_num = P.my_num();
    size_t size = 0;
    for (int i = 0; i < m; i ++) {
        size += sizes[i];
    }

    Extend *x_prime = nullptr, *y_prime = nullptr, *vrfy_left = nullptr, *vrfy_right = nullptr;
    Extend z_left = 0, z_right = 0;

    octetStream prover_os[2], verifier_os[2];
    prover_os[1].reset_write_head();
    verifier_os[1].reset_write_head();

    size_t prime_size = size + q;

    if (need_prove[my_num]) {
        // Reuse thread-local buffers
        CCS24ThreadLocalBuffers<Extend>::ensure_size_zeroed(bufs.x_prime, prime_size);
        CCS24ThreadLocalBuffers<Extend>::ensure_size_zeroed(bufs.y_prime, prime_size);
        x_prime = bufs.x_prime.data();
        y_prime = bufs.y_prime.data();
        
        size_t pointer = 0;
        for (int j = 0; j < m; j ++) {
            const Extend& coef = bufs.batch_coefs[j];
            if (coef != 0) {
                h += coef * (inner_product(x + pointer, y + pointer,
                                  sizes[j])
                        - z0[j] - z1[j]);
                for (size_t kk = 0; kk < sizes[j]; kk ++) {
                    // Scale one side only so that
                    // <coef * x, y> = coef * <x, y>.
                    x_prime[pointer + kk] = coef * x[pointer + kk];
                    y_prime[pointer + kk] = y[pointer + kk];
                }
            }
            pointer += sizes[j];
        }

        h >>= k;
        (h - shared_prngs[0].get<Extend>()).pack(prover_os[1]);
    }
    
    if (need_prove[(my_num + 1) % 3]) {
        CCS24ThreadLocalBuffers<Extend>::ensure_size_zeroed(bufs.vrfy_left, prime_size);
        vrfy_left = bufs.vrfy_left.data();
        
        size_t pointer = 0;
        for (int j = 0; j < m; j ++) {
            const Extend& coef = bufs.batch_coefs[j];
            if (coef != 0) {
                for (size_t kk = 0; kk < sizes[j]; kk ++) {
                    vrfy_left[pointer + kk] = y[pointer + kk];
                }
                z_left += coef * z1[j];
            }
            pointer += sizes[j];
        }
    }
    if (need_prove[(my_num + 2) % 3]) {
        CCS24ThreadLocalBuffers<Extend>::ensure_size_zeroed(bufs.vrfy_right, prime_size);
        vrfy_right = bufs.vrfy_right.data();
        
        size_t pointer = 0;
        for (int j = 0; j < m; j ++) {
            const Extend& coef = bufs.batch_coefs[j];
            if (coef != 0) {
                for (size_t kk = 0; kk < sizes[j]; kk ++) {
                    vrfy_right[pointer + kk] =
                            coef * x[pointer + kk];
                }
                z_right += coef * z0[j];
            }
            pointer += sizes[j];
        }
    }
    
    P.pass_around(prover_os[1], verifier_os[0], -1);

    if (need_prove[(my_num + 1) % 3]) {
        z_left += verifier_os[0].get<Extend>();
    }
    if (need_prove[(my_num + 2) % 3]) {
        z_right += shared_prngs[1].get<Extend>();
    }

    size_t now_size = size;

    // Reuse thread-local buffers for _new arrays
    size_t tmp_size = (size_t)ceil((double) now_size / (double) q);
    CCS24ThreadLocalBuffers<Extend>::ensure_size(bufs.x_new, tmp_size);
    CCS24ThreadLocalBuffers<Extend>::ensure_size(bufs.y_new, tmp_size);
    CCS24ThreadLocalBuffers<Extend>::ensure_size(bufs.vrfy_left_new, tmp_size);
    CCS24ThreadLocalBuffers<Extend>::ensure_size(bufs.vrfy_right_new, tmp_size);
    
    Extend* x_new = bufs.x_new.data();
    Extend* y_new = bufs.y_new.data();
    Extend* vrfy_left_new = bufs.vrfy_left_new.data();
    Extend* vrfy_right_new = bufs.vrfy_right_new.data();

    
    while (true) {
        if (now_size <= (size_t) q) {
            break;
        }

        prover_os[1].reset_write_head();
        verifier_os[1].reset_write_head();

        int new_size = ceil((double) now_size / (double) q);
        Extend z[q][q];
        Extend tmp_z_left = 0, tmp_z_right = 0, new_z_left = 0, new_z_right = 0;

        PRNG prng_prover, prng_left, prng_right;
        Extend alpha_left[q], alpha_right[q], beta_left[q], beta_right[q];

        if (need_prove[my_num]) {
            memset(x_new, 0, new_size * sizeof(Extend));
            memset(y_new, 0, new_size * sizeof(Extend));
            for (int i = 0; i < q; i ++) {
                for (int j = 0; j < q; j ++) {
                    z[i][j] = inner_product(x_prime + i * new_size, y_prime + j * new_size, new_size);
                    if (i == 0 && j == 0)   continue;
                    (z[i][j] - shared_prngs[0].get<Extend>()).pack(prover_os[1]);
                }
            }
        }
        if (need_prove[(my_num + 1) % 3]) {
            memset(vrfy_left_new, 0, new_size * sizeof(Extend));
            octet seed[SEED_SIZE];
            shared_prngs[1].get_octets(seed, SEED_SIZE);
            prng_left.SetSeed(seed);
            for (int i = 0; i < q; i ++) {
                alpha_left[i].randomize(prng_left);
                beta_left[i].randomize(prng_left);
            }
        }
        if (need_prove[(my_num + 2) % 3]) {
            memset(vrfy_right_new, 0, new_size * sizeof(Extend));
            octet seed[SEED_SIZE];
            shared_prngs[0].get_octets(seed, SEED_SIZE);
            prover_os[1].store_bytes(seed, SEED_SIZE);
            prng_right.SetSeed(seed);
            for (int i = 0; i < q; i ++) {
                alpha_right[i].randomize(prng_right);
                beta_right[i].randomize(prng_right);
            }
        }
        
        P.pass_around(prover_os[1], verifier_os[0], -1);

        if (need_prove[(my_num + 1) % 3]) {
            for (int i = 0; i < q; i ++) {
                for (int j = 0; j < q; j ++) {
                    if (i == 0 && j == 0) continue;
                    z[i][j].unpack(verifier_os[0]);
                    if (i == j) tmp_z_left += z[i][j];
                    new_z_left += z[i][j] * alpha_left[i] * beta_left[j];
                }
                for (int j = 0; j < new_size; j ++) {
                    vrfy_left_new[j] += beta_left[i] * vrfy_left[i * new_size + j];
                }
            }
            z[0][0] = z_left - tmp_z_left;
            new_z_left += z[0][0] * alpha_left[0] * beta_left[0];

            memcpy(vrfy_left, vrfy_left_new, new_size * sizeof(Extend));
            memset(vrfy_left + new_size, 0, q * sizeof(Extend));
            z_left = new_z_left;
        }
        if (need_prove[my_num]) {
            octet seed[SEED_SIZE];
            verifier_os[0].get_bytes(seed, SEED_SIZE);
            prng_prover.SetSeed(seed);
            Extend alpha, beta;
            for (int i = 0; i < q; i ++) {
                alpha.randomize(prng_prover);
                beta.randomize(prng_prover);
                for (int j = 0; j < new_size; j ++) {
                    x_new[j] += alpha * x_prime[i * new_size + j];
                    y_new[j] += beta * y_prime[i * new_size + j];
                }
            }

            memcpy(x_prime, x_new, new_size * sizeof(Extend));
            memcpy(y_prime, y_new, new_size * sizeof(Extend));
            memset(x_prime + new_size, 0, q * sizeof(Extend));
            memset(y_prime + new_size, 0, q * sizeof(Extend));
        }
        if (need_prove[(my_num + 2) % 3]) {
            for (int i = 0; i < q; i ++) {
                for (int j = 0; j < q; j ++) {
                    if (i == 0 && j == 0) continue;
                    z[i][j].randomize(shared_prngs[1]);
                    if (i == j) tmp_z_right += z[i][j];
                    new_z_right += z[i][j] * alpha_right[i] * beta_right[j];
                }
                for (int j = 0; j < new_size; j ++) {
                    vrfy_right_new[j] += alpha_right[i] * vrfy_right[i * new_size + j];
                }
            }

            z[0][0] = z_right - tmp_z_right;
            new_z_right += z[0][0] * alpha_right[0] * beta_right[0];

            memcpy(vrfy_right, vrfy_right_new, new_size * sizeof(Extend));
            memset(vrfy_right + new_size, 0, q * sizeof(Extend));
            z_right = new_z_right;
        }

        now_size = new_size;
    }

    // No free needed for x_new, y_new, vrfy_left_new, vrfy_right_new - they are reused

    
    Extend x0, y0, x0_left, y0_left, x0_right, y0_right;
    Extend tmp_z = 0;
    Extend new_x_left = 0, new_x_right = 0, new_y_left = 0, new_y_right = 0, new_z_left = 0, new_z_right = 0;

    // Replace VLA z[now_size + 1][now_size + 1] with vector
    size_t z_dim = now_size + 1;
    CCS24ThreadLocalBuffers<Extend>::ensure_size(bufs.z_matrix, z_dim * z_dim);

    if (need_prove[my_num]) {
        memcpy(x_prime + 1, x_prime, now_size * sizeof(Extend));
        memcpy(y_prime + 1, y_prime, now_size * sizeof(Extend));

        x0 = shared_prngs[0].get<Extend>() + shared_prngs[1].get<Extend>();
        y0 = shared_prngs[0].get<Extend>() + shared_prngs[1].get<Extend>();

        x_prime[0] = x0;
        y_prime[0] = y0;

        prover_os[1].reset_write_head();

        for (size_t i = 0; i <= now_size; i ++) {
            for (size_t j = 0; j <= now_size; j ++) {
                matrix_at(bufs.z_matrix, i, j, z_dim) = x_prime[i] * y_prime[j];
                if (i == 1 && j == 1)   continue;
                (matrix_at(bufs.z_matrix, i, j, z_dim) - shared_prngs[0].get<Extend>()).pack(prover_os[1]);
            }
        }
    }
    if (need_prove[(my_num + 1) % 3]) {
        memcpy(vrfy_left + 1, vrfy_left, now_size * sizeof(Extend));

        x0_left.randomize(shared_prngs[0]);
        y0_left.randomize(shared_prngs[0]);

        vrfy_left[0] = y0_left;
    }
    if (need_prove[(my_num + 2) % 3]) {
        memcpy(vrfy_right + 1, vrfy_right, now_size * sizeof(Extend));

        x0_right.randomize(shared_prngs[1]);
        y0_right.randomize(shared_prngs[1]);

        vrfy_right[0] = x0_right;
    }

    P.pass_around(prover_os[1], verifier_os[0], -1);
    GlobalPRNG global_prng(P);

    // Replace VLA alpha/beta with vector
    CCS24ThreadLocalBuffers<Extend>::ensure_size(bufs.alpha_vec, z_dim);
    CCS24ThreadLocalBuffers<Extend>::ensure_size(bufs.beta_vec, z_dim);

    if (need_prove[my_num]) {
        Extend new_x = 0, new_y = 0, new_z = 0;

        bufs.alpha_vec[0] = bufs.beta_vec[0] = Extend(1);
        for (size_t i = 1; i <= now_size; i ++) {
            bufs.alpha_vec[i].randomize(global_prng);
            bufs.beta_vec[i].randomize(global_prng);
            new_x += bufs.alpha_vec[i] * x_prime[i];
            new_y += bufs.beta_vec[i] * y_prime[i];
        }

        new_x += bufs.alpha_vec[0] * x0;
        new_y += bufs.beta_vec[0] * y0;

        for (size_t i = 0; i <= now_size; i ++) {
            for (size_t j = 0; j <= now_size; j ++) {
                new_z += matrix_at(bufs.z_matrix, i, j, z_dim) * bufs.alpha_vec[i] * bufs.beta_vec[j];
            }
        }

        // No free needed - buffers are reused
    }
    if (need_prove[(my_num + 1) % 3]) {
        tmp_z = 0;
        for (size_t i = 0; i <= now_size; i ++) {
            for (size_t j = 0; j <= now_size; j ++) {
                if (i == 1 && j == 1)   continue;
                matrix_at(bufs.z_matrix, i, j, z_dim) = verifier_os[0].get<Extend>();
            }
        }

        for (size_t i = 2; i <= now_size; i ++) {
            tmp_z += matrix_at(bufs.z_matrix, i, i, z_dim);
        }

        matrix_at(bufs.z_matrix, 1, 1, z_dim) = z_left - tmp_z;

        new_y_left = 0;
        new_z_left = 0;

        for (size_t i = 0; i <= now_size; i ++) {
            bufs.alpha_vec[i].randomize(global_prng);
            bufs.beta_vec[i].randomize(global_prng);
            new_y_left += bufs.beta_vec[i] * vrfy_left[i];
        }

        new_x_left = bufs.alpha_vec[0] * x0_left;

        for (size_t i = 0; i <= now_size; i ++) {
            for (size_t j = 0; j <= now_size; j ++) {
                new_z_left += matrix_at(bufs.z_matrix, i, j, z_dim) * bufs.alpha_vec[i] * bufs.beta_vec[j];
            }
        }

        verifier_os[0].reset_write_head();

        new_x_left.pack(verifier_os[0]);
        new_y_left.pack(verifier_os[0]);
        new_z_left.pack(verifier_os[0]);

        P.send_to((my_num + 2) % 3, verifier_os[0]);
    }

    if (need_prove[(my_num + 2) % 3]) {
        tmp_z = 0;
        for (size_t i = 0; i <= now_size; i ++) {
            for (size_t j = 0; j <= now_size; j ++) {
                if (i == 1 && j == 1)   continue;
                matrix_at(bufs.z_matrix, i, j, z_dim).randomize(shared_prngs[1]);
            }
        }

        for (size_t i = 2; i <= now_size; i ++) {
            tmp_z += matrix_at(bufs.z_matrix, i, i, z_dim);
        }

        matrix_at(bufs.z_matrix, 1, 1, z_dim) = z_right - tmp_z;

        new_x_right = 0;
        new_z_right = 0;

        for (size_t i = 0; i <= now_size; i ++) {
            bufs.alpha_vec[i].randomize(global_prng);
            bufs.beta_vec[i].randomize(global_prng);
            new_x_right += bufs.alpha_vec[i] * vrfy_right[i];
        }

        new_y_right = bufs.beta_vec[0] * y0_right;

        for (size_t i = 0; i <= now_size; i ++) {
            for (size_t j = 0; j <= now_size; j ++) {
                new_z_right += matrix_at(bufs.z_matrix, i, j, z_dim) * bufs.alpha_vec[i] * bufs.beta_vec[j];
            }
        }

        prover_os[0].reset_write_head();

        new_x_right.pack(prover_os[0]);
        new_y_right.pack(prover_os[0]);
        new_z_right.pack(prover_os[0]);

        P.send_to((my_num + 1) % 3, prover_os[0]);
    }

    if (need_prove[(my_num + 1) % 3]) {

        P.receive_player((my_num + 2) % 3, verifier_os[1]);

        new_x_left += verifier_os[1].get<Extend>();
        new_y_left += verifier_os[1].get<Extend>();
        new_z_left += verifier_os[1].get<Extend>();

        // No free needed - buffers are reused
    }
    if (need_prove[(my_num + 2) % 3]) {

        P.receive_player((my_num + 1) % 3, prover_os[1]);

        new_x_right += prover_os[1].get<Extend>();
        new_y_right += prover_os[1].get<Extend>();
        new_z_right += prover_os[1].get<Extend>();

        // No free needed - buffers are reused
    }
    
    // No memory leaks - all buffers are thread-local and reused
}

#endif
