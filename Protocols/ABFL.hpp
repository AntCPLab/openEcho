#ifndef PROTOCOLS_ABFL_HPP_
#define PROTOCOLS_ABFL_HPP_

#include "ABFL.h"

template <class T>
ABFL<T>::ABFL(Player &P) : ReplicatedBase(P), honest(*this), global_prng(P)
{
    assert(T::vector_length == 2);
}

template <class T>
void ABFL<T>::init_mul()
{
    if (this->MC) {
        this->MC->MaybeCheck(P);
    }
    honest.init_mul();
    mult_pairs.clear();
}

template <class T>
void ABFL<T>::prepare_mul(const T &x, const T &y, int n)
{
    honest.prepare_mul(x, y, n);
    mult_pairs.push_back({x, y});
}

template <class T>
void ABFL<T>::exchange()
{
    honest.exchange();
}

template <class T>
T ABFL<T>::finalize_mul(int n)
{
    T ret = honest.finalize_mul(n);
    array<T, 2> share_pair = mult_pairs.next();
    MC->add_triple(share_pair[0], share_pair[1], ret);
    return ret;
}

template <class T>
void ABFL<T>::init_dotprod()
{
    honest.init_dotprod();
}

template <class T>
void ABFL<T>::prepare_dotprod(const T &x, const T &y)
{
    honest.prepare_dotprod(x, y);
}

template <class T>
void ABFL<T>::next_dotprod()
{
    honest.next_dotprod();
}

template <class T>
T ABFL<T>::finalize_dotprod(int length)
{
    return honest.finalize_dotprod(length);
}

template <class T>
T ABFL<T>::get_random()
{
    return honest.get_random();
}

template<class T>
template<class U>
void ABFL<T>::trunc_pr(const vector<int>& regs, int size, U& proc,
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
    ReplicatedInput<T> input(0, *this);

    // use https://eprint.iacr.org/2019/131
    bool have_small_gap = false;
    // use https://eprint.iacr.org/2018/403
    bool have_big_gap = false;

    for (auto info : infos)
        if (info.small_gap())
            have_small_gap = true;
        else
            have_big_gap = true;

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
void ABFL<T>::trunc_pr(const vector<int>& regs, int size, U& proc,
        true_type)
{
    (void) regs, (void) size, (void) proc;
    throw runtime_error("trunc_pr not implemented");
}

template<class T>
template<class U>
void ABFL<T>::trunc_pr(const vector<int>& regs, int size,
        U& proc)
{
    this->trunc_rounds++;
    trunc_pr(regs, size, proc, T::clear::characteristic_two);
}

#endif