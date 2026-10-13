#ifndef PROTOCOLS_CCS24_H_
#define PROTOCOLS_CCS24_H_

#include "Replicated.hpp"

template <class T>
class Ccs24 : public ProtocolBase<T>
{
    typedef Replicated<T> Honest;

    array<octetStream, 2> os;
    typename T::MAC_Check* MC;
    Preprocessing<T>* prep;
    typename T::clear dotprod_share, dotprod_share1, dotprod_share2;
    PointerVector<typename T::value_type> add_shares;
    PointerVector<typename T::open_type> trunc_buffer;

public:

    mutable array<PRNG, 2> shared_prngs;
    Player& P;

    Ccs24(Player& P);
    Ccs24(Player& P, array<PRNG, 2>& prngs): P(P) {
        for (int i = 0; i < 2; i++)
            shared_prngs[i].SetSeed(prngs[i]);
    }

    Ccs24 branch() {
        typename T::Protocol res(P);
        res.prep = prep;
        res.MC = MC;
        res.init_mul();
        return res;
    }

    void init(Preprocessing<T>& prep, typename T::MAC_Check& MC) {
        this->prep = &prep;
        this->MC = &MC;

        this->MC->init(P);
    }

    int get_n_relevant_players() { return P.num_players() - 1; }

    static void assign(T& share, const typename T::clear& value, int my_num)
    {
        Honest::assign(share, value, my_num);
    }

    void init_mul();
    void prepare_mul(const T& x, const T& y, int n = -1);
    void exchange();
    T finalize_mul(int n = -1);

    void init_dotprod();
    void prepare_dotprod(const T& x, const T& y);
    void next_dotprod();
    T finalize_dotprod(int length);

    T get_random();

    void unsplit(StackedVector<T>& dest,
            StackedVector<typename T::bit_type>& source,
            const Instruction& instruction);

    void randoms(T& res, int n_bits)
    {
        for (int i = 0; i < 2; i++)
            res[i].randomize_part(shared_prngs[i], n_bits);
    }

    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc);
    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc, true_type);
    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc, false_type);

};

#endif