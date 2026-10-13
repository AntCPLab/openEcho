#ifndef PROTOCOLS_CCS24_HPP_
#define PROTOCOLS_CCS24_HPP_

#include "Ccs24.h"

template <class T>
Ccs24<T>::Ccs24(Player &P) : P(P)
{
    assert(P.num_players() == 3);
    if (not P.is_encrypted())
        insecure("unencrypted communication", false);

    octet seed1[16] = "0123456789abcde";
    octet seed2[16] = "edcba9876543210";
    octet seed3[16] = "02468ace13579bd";

    if (P.my_num() == 0) {
        shared_prngs[0].SetSeed(seed1);
        shared_prngs[1].SetSeed(seed3);
    }
    else if (P.my_num() == 1) {
        shared_prngs[0].SetSeed(seed2);
        shared_prngs[1].SetSeed(seed1);
    }
    else {
        shared_prngs[0].SetSeed(seed3);
        shared_prngs[1].SetSeed(seed2);
    }
}

template<class T>
void Ccs24<T>::init_mul()
{

    if (MC) {
        MC->MaybeCheck(P);
    }

    for (auto& o : os) {
        o.reset_write_head();
    }   
    add_shares.clear();
}

template<class T>
void Ccs24<T>::prepare_mul(const T& x,
        const T& y, int n)
{
    typename T::value_type add_share = x[0] * (y[0] + y[1]) + x[1] * y[0];
    typename T::value_type tmp[2];
    // tmp[0] = random_counter ++, tmp[1] = random_counter ++;
    for (int i = 0; i < 2; i++) {
        tmp[i].randomize(shared_prngs[i], n);
    }
    
    add_share = add_share + tmp[0] - tmp[1];
    add_share.pack(os[0], n);
    add_shares.push_back(add_share);
    add_shares.push_back(tmp[1]);

    MC->add_rx(x[0]);
    MC->add_rx(y[0]);
    MC->add_ry(y[1]);
    MC->add_ry(x[1]);
    MC->add_rz1(x[0] * y[0] + tmp[0]);

}

template<class T>
void Ccs24<T>::exchange()
{
    os[0].append(0);
    if (os[0].get_length() > 0) {
        P.pass_around(os[0], os[1], 1);
    }
}

template<class T>
inline T Ccs24<T>::finalize_mul(int n)
{
    T result;
    result[0] = add_shares.next();
    result[1].unpack(os[1], n);
    MC->add_rz2(result[1] - add_shares.next());
    MC->add_size(2);
    return result;
}

template<class T>
inline void Ccs24<T>::init_dotprod()
{
    init_mul();
    dotprod_share.assign_zero();
    dotprod_share1.assign_zero();
    dotprod_share2.assign_zero();
}

template<class T>
inline void Ccs24<T>::prepare_dotprod(const T& x, const T& y)
{
    dotprod_share = dotprod_share + x[0] * (y[0] + y[1]) + x[1] * y[0];
    MC->add_rx(x[0]);
    MC->add_rx(y[0]);
    MC->add_ry(y[1]);
    MC->add_ry(x[1]);
    dotprod_share1 += x[0] * y[0];
}

template<class T>
inline void Ccs24<T>::next_dotprod()
{
    dotprod_share.normalize();
    typename T::value_type tmp[2];
    for (int i = 0; i < 2; i++)
        tmp[i].randomize(shared_prngs[i]);
    
    auto add_share = dotprod_share + tmp[0] - tmp[1];
    add_share.pack(os[0]);
    add_shares.push_back(add_share);

    dotprod_share.assign_zero();

    MC->add_rz1(dotprod_share1 + tmp[0]);
    add_shares.push_back(tmp[1]);
    dotprod_share1.assign_zero();
}

template<class T>
inline T Ccs24<T>::finalize_dotprod(int length)
{
    T result;
    result[0] = add_shares.next();
    result[1].unpack(os[1]);
    MC->add_rz2(result[1] - add_shares.next());
    MC->add_size(2 * length);
    return result;
}

template <class T>
T Ccs24<T>::get_random()
{
    T res;
    for (int i = 0; i < 2; i++)
        res[i].randomize(shared_prngs[i]);
    return res;
}

template<class T>
void Ccs24<T>::unsplit(StackedVector<T>& dest,
        StackedVector<typename T::bit_type>& source,
        const Instruction& instruction)
{
    CODE_LOCATION
    const int n_bits = instruction.get_size();
    const auto& starts = instruction.get_start();
    if (starts.empty() or starts.size() > 2)
        throw runtime_error("unsplit requires one or two outputs");

    const int my_num = P.my_num();
    const int unit = T::bit_type::default_length;
    array<vector<T>, 3> component_bits;
    for (auto& bits : component_bits)
    {
        bits.resize(n_bits);
        for (auto& bit : bits)
            bit.assign_zero();
    }

    int offset = 0;
    const int n_blocks = DIV_CEIL(n_bits, unit);
    auto source_it = source.iterator_for_size(instruction.get_r(0), n_blocks);
    for (int block = 0; block < n_blocks; block++)
    {
        const auto& packed = *source_it++;
        const int n_left = min(unit, n_bits - block * unit);
        for (int j = 0; j < n_left; j++, offset++)
        {
            // Party i holds binary components (b_i, b_{i-1}). Embedding
            // each component in the same position creates arithmetic RSS
            // shares of b_0, b_1, and b_2 without communication.
            component_bits[my_num][offset][0] = packed[0].get_bit(j);
            component_bits[(my_num + 2) % 3][offset][1] =
                    packed[1].get_bit(j);
        }
    }

    // First layer: u = b_0 XOR b_2.
    init_mul();
    for (int i = 0; i < n_bits; i++)
        prepare_mul(component_bits[0][i], component_bits[2][i]);
    exchange();

    vector<T> first_xor(n_bits);
    for (int i = 0; i < n_bits; i++)
        first_xor[i] = component_bits[0][i] + component_bits[2][i]
                - 2 * finalize_mul();

    if (starts.size() == 2)
    {
        auto first_it = dest.iterator_for_size(starts[0], n_bits);
        auto second_it = dest.iterator_for_size(starts[1], n_bits);
        for (int i = 0; i < n_bits; i++)
        {
            *first_it++ = first_xor[i];
            *second_it++ = component_bits[1][i];
        }
        return;
    }

    // Second layer: (b_0 XOR b_2) XOR b_1.
    init_mul();
    for (int i = 0; i < n_bits; i++)
        prepare_mul(first_xor[i], component_bits[1][i]);
    exchange();

    auto dest_it = dest.iterator_for_size(starts[0], n_bits);
    for (int i = 0; i < n_bits; i++)
        *dest_it++ = first_xor[i] + component_bits[1][i]
                - 2 * finalize_mul();
}


template<class T>
template<class U>
void Ccs24<T>::trunc_pr(const vector<int>& regs, int size, U& proc,
        false_type)
{
    assert(regs.size() % 4 == 0);
    assert(proc.P.num_players() == 3);
    assert(proc.Proc != 0);
    typedef typename T::clear value_type;
    int gen_player = 2;
    int comp_player = 1;
    bool generate = P.my_num() == gen_player;
    bool compute = P.my_num() == comp_player;
    ArgList<TruncPrTupleWithGap<value_type>> infos(regs);
    auto& S = proc.get_S();

    octetStream cs;
    ReplicatedInput<T> input(P);

    // use https://eprint.iacr.org/2019/131
    bool have_small_gap = false;
    // use https://eprint.iacr.org/2018/403
    bool have_big_gap = false;

    for (auto info : infos)
        if (info.small_gap())
            have_small_gap = true;
        else
            have_big_gap = true;

    have_big_gap = false;
    have_small_gap = true;

    if (generate)
    {
        SeededPRNG G;
        for (auto info : infos)
            for (int i = 0; i < size; i++)
            {
                auto& x = S[info.source_base + i];
                if (info.small_gap())
                {
                    auto r = G.get<value_type>();
                    input.add_mine(info.upper(r));
                    input.add_mine(info.msb(r));
                    (r + x[0]).pack(cs);
                }
                else
                {
                    auto& y = S[info.dest_base + i];
                    auto r = this->shared_prngs[0].template get<value_type>();
                    y[1] = -value_type(-value_type(x.sum()) >> info.m) - r;
                    y[1].pack(cs);
                    y[0] = r;
                }
            }

        P.send_to(comp_player, cs);
    }
    else if (have_small_gap)
        input.add_other(gen_player);

    if (compute)
    {
        P.receive_player(gen_player, cs);
        for (auto info : infos)
            for (int i = 0; i < size; i++)
            {
                auto& x = S[info.source_base + i];
                if (info.small_gap())
                {
                    auto c = cs.get<value_type>() + x.sum();
                    input.add_mine(info.upper(c));
                    input.add_mine(info.msb(c));
                }
                else
                {
                    auto& y = S[info.dest_base + i];
                    y[0] = cs.get<value_type>();
                    y[1] = x[1] >> info.m;
                }
            }
    }

    if (have_big_gap and not (compute or generate))
    {
        for (auto info : infos)
            if (info.big_gap())
                for (int i = 0; i < size; i++)
                {
                    auto& x = S[info.source_base + i];
                    auto& y = S[info.dest_base + i];
                    y[0] = x[0] >> info.m;
                    y[1] = this->shared_prngs[1].template get<value_type>();
                }
    }

    if (have_small_gap)
    {
        input.add_other(comp_player);
        input.exchange();
        init_mul();

        for (auto info : infos)
            for (int i = 0; i < size; i++)
            {
                if (info.small_gap())
                {
                    this->trunc_pr_counter++;
                    auto c_prime = input.finalize(comp_player);
                    auto r_prime = input.finalize(gen_player);
                    S[info.dest_base + i] = c_prime - r_prime;

                    auto c_dprime = input.finalize(comp_player);
                    auto r_msb = input.finalize(gen_player);
                    S[info.dest_base + i] += ((r_msb + c_dprime)
                            << (info.k - info.m));
                    prepare_mul(r_msb, c_dprime);
                }
            }

        exchange();

        for (auto info : infos)
            for (int i = 0; i < size; i++)
                if (info.small_gap())
                    S[info.dest_base + i] -= finalize_mul()
                            << (info.k - info.m + 1);
    }
}

template<class T>
template<class U>
void Ccs24<T>::trunc_pr(const vector<int>& regs, int size, U& proc,
        true_type)
{
    (void) regs, (void) size, (void) proc;
    throw runtime_error("trunc_pr not implemented");
}

template<class T>
template<class U>
void Ccs24<T>::trunc_pr(const vector<int>& regs, int size,
        U& proc)
{
    this->trunc_rounds++;
    // trunc_pr(regs, size, proc, T::clear::characteristic_two);
    // return ;

    assert((regs.size() & 3) == 0);
    auto& S = proc.get_S();
    int my_num = P.my_num();
    octetStream os[2];
    os[0].reset_write_head();
    T ri;
    int k = regs[2], offset = regs[3];

    size_t size2 = size * (regs.size() >> 2);
    vector<T> r(size2), r_prime(size2), rk(size2);
    vector<T> shares(size2);
    vector<typename T::open_type> opened(size2);

    for (size_t i = 0; i < size2; i ++) {
        for (int j = 0; j < k - 1; j ++) {
            r[i] += (1 << j) * (prep->get_bit());
        }
        rk[i] = prep->get_bit();
        r[i] += (1 << (k - 1)) * rk[i];
        r_prime[i] = r[i] >> offset;
    }

    auto r_it = r.begin();

    for (size_t j = 0; j < regs.size(); j += 4) {
        int src = regs[j + 1];
        for (int i = 0; i < size; i ++) {
            shares.push_back(S[src + i] + (* r_it));
            r_it ++;
        }
    }

    MC->POpen(opened, shares, P);
    auto open_it = opened.begin();
    auto r_prime_it = r_prime.begin();
    auto rk_it = rk.begin();

    for (size_t j = 0; j < regs.size(); j += 4) {
        int dest = regs[j];
        for (int i = 0; i < size; i ++) {
            auto c = *open_it;
            S[dest + i] = T::constant((c >> offset) & ((1 << (k - offset - 1)) - 1), my_num);
            S[dest + i] -= *r_prime_it;
            S[dest + i] += ((*rk_it) ^ (c >> (k - 1))) << (k - offset - 1);

            open_it ++;
            r_prime_it ++;
            rk_it ++;
        }
    }
}

#endif