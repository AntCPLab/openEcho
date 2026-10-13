#ifndef PROTOCOLS_COVERTPROTOCOL_H_
#define PROTOCOLS_COVERTPROTOCOL_H_

#include "Protocols/Replicated.h"
#include "Tools/FastVector.h"

template <class T>
class CovertABFLProtocol : public ReplicatedBase, public ProtocolBase<T> {
    Replicated<T> _impl;
    typename T::MAC_Check* MC = nullptr;

    PointerVector<array<T, 2>> input_pairs;
public:
 
    CovertABFLProtocol(Player& P) : ReplicatedBase(P), _impl(*this) {}
    CovertABFLProtocol(const CovertABFLProtocol& other) :
        ReplicatedBase(other.P, other.shared_prngs), _impl(*this) {}

    ~CovertABFLProtocol() {}

    static void assign(T& share, const typename T::clear& value, int my_num) {
        Replicated<T>::assign(share, value, my_num);
    }

    void init(Preprocessing<T>&, typename T::MAC_Check& _MC) override {
        MC = &_MC;
        MC->init(this->P);
    }

    void init_mul() override {
        input_pairs.clear();
        _impl.init_mul();

        if (MC) {
            MC->MaybeCheck(this->P);
        }
    }
    void prepare_mul(const T& x, const T& y, int n = -1) override {
        input_pairs.push_back({x, y});
        _impl.prepare_mul(x, y, n);
    }
    void exchange() override {
        _impl.exchange();
    }
    T finalize_mul(int n = -1) override {
        T result = _impl.finalize_mul(n);
        array<T, 2> input_pair = input_pairs.next();
        MC->add_triple({input_pair[0], input_pair[1], result});
        return result;
    }

    void prepare_reshare(const typename T::clear& share, int n = -1) {
        _impl.prepare_reshare(share, n);
    }
    void prepare_mul_fast(const T& x, const T& y) {
        _impl.prepare_mul_fast(x, y);
    }
    T finalize_mul_fast() {
        return _impl.finalize_mul_fast();
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
        _impl.set_fast_mode(change);
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



#endif