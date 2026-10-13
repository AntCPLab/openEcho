#ifndef PROTOCOLS_ECHOSHARE_H_
#define PROTOCOLS_ECHOSHARE_H_

#include "AstraShare.h"
#include "Protocols/Echo.hpp"
#include "Protocols/EchoMC.hpp"
#include "Protocols/EchoInput.hpp"
#include "GC/EchoSecret.h"
#include "Protocols/EchoPrep.hpp"

// Echo协议: 从Astra算术格式到二进制分享的正确映射
//
// 算术分享分布 (Astra格式):
// P0: {a1, a2}, P1: {a0, a1}, P2: {a0, a2}
//
// Wallace树列分配:
// 列0: a0的二进制位 <- P1(i=0), P2(i=0)
// 列1: a1的二进制位 <- P0(i=0), P1(i=1)  
// 列2: a2的二进制位 <- P0(i=1), P2(i=1)
//
// index_mapping[my_num][i] = Wallace树列号
const int index_mapping[3][3] = {
    {1, 2, 0},  // P0: i=0(a1)->列1, i=1(a2)->列2, i=2(unused)->列0
    {0, 1, 2},  // P1: i=0(a0)->列0, i=1(a1)->列1, i=2(unused)->列2
    {0, 2, 1}   // P2: i=0(a0)->列0, i=1(a2)->列2, i=2(unused)->列1
};

template <class T>
class EchoPrepShare : public AstraPrepShare<T> {
    using This = EchoPrepShare<T>;
    using super = AstraPrepShare<T>;

public:
    using clear = T;

    using bit_type = GC::EchoSecret<EchoPrepShare<BitVec>>;
    using MAC_Check = EchoPrepMC<This>;
    using Direct_MC = MAC_Check;

    using Protocol = EchoPrepProtocol<This>;
    
    using LivePrep = EchoPrepPrep<This>;

    using Input = EchoPrepInput<This>;
    using PrivateOutput = ::PrivateOutput<This>;

    template<class U>
    using mc_template = EchoMCBase<U>;
    template<class U>
    using protocol_template = EchoPrepBinProtocol<U>;
    template<class U>
    using input_template = EchoPrepInput<U>;

    static string type_string()
    {
        return "Echo share " + T::type_string();
    }

    static string type_short()
    {
        return "echo-" + T::type_short();
    }

    EchoPrepShare() {}
    EchoPrepShare(const T& x, const T& y) {
        (*this)[0] = x;
        (*this)[1] = y;
    }

    template<class U>
    EchoPrepShare(const U& other) :
            super(other)
    {
    }

    static This constant(const clear& value, int my_num = -1,
            const typename AstraShare<T>::mac_key_type& = {})
    {
        This res;
        if (my_num == 1 || my_num == 2) {
            res[0] = value;
        }
        return res;
    }

    // Inherit split_index, matters_for_split, and split from AstraPrepShare
    // This ensures proper 3-way split handling
};

template <class T>
class EchoShare : public AstraShare<T> {
    using This = EchoShare<T>;
    using super = AstraShare<T>;

public:
    using clear = T;
    using open_type = clear;

    using bit_type = GC::EchoSecret<EchoShare<BitVec>>;
    using MAC_Check = EchoMC<This>;
    using Direct_MC = MAC_Check;

    using Protocol = Echo<This>;
    using LivePrep = EchoPrep<This>;
    using Input = EchoInput<This>;
    using PrivateOutput = ::PrivateOutput<This>;

    template <class U>
    using mc_template = EchoMCBase<U>;
    template <class U>
    using protocol_template = EchoBin<U>;
    template <class U>
    using input_template = EchoInput<U>;

    static const bool dishonest_majority = true;
    static const bool variable_players = false;
    static const bool has_trunc_pr = true;
    static const bool function_dependent = true;

    EchoShare()
    {
    }

    EchoShare(const T& x, const T& y) {
        (*this)[0] = x;
        (*this)[1] = y;
    }

    template<class U>
    EchoShare(const U& other) :
            super(other)
    {
    }

    static This constant(const clear& value, int my_num = -1,
            const typename AstraShare<T>::mac_key_type& = {})
    {
        This res;
        if (my_num == 1 || my_num == 2) {
            res[0] = value;
        }
        return res;
    }

    static string type_string()
    {
        return "Echo share " + T::type_string();
    }

    static string type_short()
    {
        return "echo-" + T::type_short();
    }

    template<class U, int MY_NUM>
    static void pre_reduced_mul(U& a, U& b, U& c, Echo<U>& protocol,
            const T& aa, const T& bb);

    template<class U, int MY_NUM>
    static pair<U, clear> post_reduced_mul(typename U::Protocol & protocol);

    static int split_index(int arithmetic_index, int my_num) {
        return index_mapping[my_num][arithmetic_index];
    }

    static bool matters_for_split(int , int )
    {
        // Echo协议：所有分量都参与split过程
        return true;
    }

    template<class U, class V>
    static void split(StackedVector<U>& dest,
            const vector<int>& regs, int n_bits, const V* source,
            int n_inputs, typename U::Protocol& protocol) {
        CODE_LOCATION
        Rep3Share2<T::N_BITS>::split(dest, regs, n_bits, source, n_inputs, protocol);
    }
};

template<class T>
template<class U, int MY_NUM>
void EchoShare<T>::pre_reduced_mul(U& a, U& b, U& c,
        Echo<U>& protocol, const T&, const T& bb)
{
    auto& os_prep = protocol.os_prep;
    os_prep.get_no_check(a[1]);
    os_prep.get_no_check(c[1]);
    auto Mi = bb * a.neg_lambda(-1) - c.neg_lambda(-1);
    protocol.os.store_no_resize(Mi);
    b.m(-1) = bb;
    c.m(-1) = Mi;
    protocol.results.push_back({});
}

template<class T>
template<class U, int MY_NUM>
pair<U, T> EchoShare<T>::post_reduced_mul(typename U::Protocol& protocol)
{
    pair<U, T> res;
    res.first = protocol.results.next();
    res.second = protocol.recv_os.template get_no_check<clear>();
    return res;
}


template <int K>
using EchoPrepShare2 = EchoPrepShare<SignedZ2<K>>;

template <int K>
using EchoShare2 = EchoShare<SignedZ2<K>>;

#endif