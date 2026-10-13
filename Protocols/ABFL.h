/*
TODO:
1. Offline multiplication
2. Offline buffer
*/

#ifndef PROTOCOLS_ABFL_H_
#define PROTOCOLS_ABFL_H_

#include "Protocols/Replicated.hpp"

template <class T>
class ABFL : public ReplicatedBase, public ProtocolBase<T>
{

    typedef Replicated<T> Honest;

    array<octetStream, 2> os;
    typename T::MAC_Check* MC;
    Preprocessing<T>* prep;
    Honest honest;
    GlobalPRNG global_prng;
    PointerVector<array<T, 2>> mult_pairs;
    typename T::clear dotprod_share;

    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc, true_type);
    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc, false_type);

public:

    ABFL(Player& P);
    ABFL(const ReplicatedBase& other):
        ReplicatedBase(other.P, other.shared_prngs), honest(*this), global_prng(other.P) {}
    ABFL(Player& P, const array<PRNG, 2>& prngs):
        ReplicatedBase(P, prngs), honest(*this), global_prng(P) {}

    ABFL branch() {
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

    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc);

    double randomness_time() {
        return 0;
    }

};


#endif