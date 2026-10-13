#ifndef PROTOCOLS_BGIN19_H_
#define PROTOCOLS_BGIN19_H_

#include "Protocols/Replicated.h"
#include "Protocols/BGIN19MC.h"
#include "Tools/FastVector.h"

template <class T>
class BGIN19Protocol : public ReplicatedBase, public ProtocolBase<T> {
    Replicated<T> _impl;
    typename T::MAC_Check* MC = nullptr;
    using clear = typename T::clear;

public:
    BGIN19Protocol(Player& P) : ReplicatedBase(P), _impl(*this) {}
    BGIN19Protocol(const BGIN19Protocol& other) :
        ReplicatedBase(other.P, other.shared_prngs), _impl(*this) {}

    ~BGIN19Protocol() {}

    static void assign(T& share, const typename T::clear& value, int my_num) {
        Replicated<T>::assign(share, value, my_num);
    }

    void init(Preprocessing<T>&, typename T::MAC_Check& _MC) override {
        MC = &_MC;
        MC->init(this->P);
    }

    void init_mul() override {
        _impl.init_mul();

        if (MC) {
            MC->MaybeCheck(this->P, false);
        }
    }
    void prepare_mul(const T& x, const T& y, int n = -1) override {
        typename T::value_type add_share = x.local_mul(y);
        typename T::value_type tmp[2];
        for (int i = 0; i < 2; i++)
            tmp[i].randomize(_impl.shared_prngs[i], n);
        add_share = add_share + tmp[0] - tmp[1];

        _impl.add_shares.push_back(add_share);
        add_share.pack(_impl.os[0], n);

        if (n == -1) {
            n = clear::n_bits;
        }

        auto tmp0 = tmp[0].get(), tmp1 = tmp[1].get();
        auto x0 = x[0].get(), x1 = x[1].get();
        auto y0 = y[0].get(), y1 = y[1].get();

        this->MC->add_pre(tmp0, tmp1, x0, x1, y0, y1, n);
    }
    void exchange() override {
        _impl.exchange();
    }
    T finalize_mul(int n = -1) override {
        this->add_mul(n);
        T result;
        result[0] = _impl.add_shares.next();
        result[1].unpack(_impl.os[1], n);
        if (n == -1) {
            n = clear::n_bits;
        }
        auto result0 = result[0].get(), result1 = result[1].get();
        this->MC->add_post(result0, result1, n);
        return result;
    }

    void prepare_reshare(const typename T::clear& share, int n = -1) {
        _impl.prepare_reshare(share, n);
    }
    void prepare_mul_fast(const T& x, const T& y) {
        prepare_mul(x, y);
    }
    T finalize_mul_fast() {
        return finalize_mul();
    }

    void init_dotprod() {
        _impl.init_dotprod();
    }
    void prepare_dotprod(const T& x, const T& y) {
        _impl.prepare_dotprod(x, y);
    }
    void next_dotprod() {
        _impl.next_dotprod();
    }
    T finalize_dotprod(int length) {
        return _impl.finalize_dotprod(length);
    }

    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc) {
        _impl.trunc_pr(regs, size, proc);
    }

    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc, true_type) {
        _impl.trunc_pr(regs, size, proc, true_type());
    }
    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc, false_type) {
        _impl.trunc_pr(regs, size, proc, false_type());
    }

    T get_random() override {
        return _impl.get_random();
    }
    void randoms(T& res, int n_bits) override {
        _impl.randoms(res, n_bits);
    }

    void start_exchange() override {
        _impl.start_exchange();
    }
    void stop_exchange() override {
        _impl.stop_exchange();
    }

    void set_fast_mode(bool change) override {
        (void) change;
        _impl.set_fast_mode(false);
    }

    template<int = 0>
    void unsplit(StackedVector<T>& dest,
            StackedVector<typename T::bit_type>& source,
            const Instruction& instruction) {
                _impl.unsplit(dest, source, instruction);
            }

    ReplicatedInput<T>& get_helper_input(size_t i = 0) {
        return _impl.get_helper_input(i);
    }

    virtual double randomness_time() override {
        return _impl.randomness_time();
    }

    int get_n_relevant_players() { return P.num_players() - 1; }
};

#endif // PROTOCOLS_BGIN19_H_