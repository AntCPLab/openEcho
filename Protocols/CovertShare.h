#ifndef PROTOCOLS_COVERTSHARE_H_
#define PROTOCOLS_COVERTSHARE_H_

#include "Protocols/SpdzWiseRingShare.h"
#include "Protocols/SpdzWiseShare.h"
#include "GC/CovertSecret.h"
#include "GC/square64.h"
#include "Protocols/MaliciousRepPrep.hpp"
#include "Protocols/ReplicatedInput.hpp"

#include "BGIN19.h"

#include "Rep3Share.h"
#include "SemiShare.h"
#include "Ccs24.hpp"
#include "Ccs24MC.hpp"

template <int K, int S>
class CovertMacRingShare : public SpdzWiseRingShare<52, 12> {
    typedef CovertMacRingShare This;
    typedef SpdzWiseRingShare<52, 12> super;

public:
    typedef SpdzWiseMC<This> MAC_Check;
    typedef MAC_Check Direct_MC;

    typedef SpdzWiseRing<This> Protocol;
    typedef SpdzWiseRingPrep<This> LivePrep;
    typedef SpdzWiseInput<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef GC::CovertABFLSecret bit_type;

    CovertMacRingShare()
    {
    }

    template<class T>
    CovertMacRingShare(const T& other) :
            super(other)
    {
    }

    template<class T, class U>
    CovertMacRingShare(const T &share, const U &mac) :
            super(share, mac)
    {
    }

    template<class U>
    static void split(StackedVector<U>& dest, const vector<int>& regs, int n_bits,
            const CovertMacRingShare* source, int n_inputs,
            typename U::Protocol& protocol)
    {
        vector<Rep3Share2<K>> shares(n_inputs);
        for (int i = 0; i < n_inputs; i++)
            shares[i] = source[i].get_share();
        Rep3Share2<K>::split(dest, regs, n_bits, shares.data(), n_inputs, protocol);
    }

    static void shrsi(SubProcessor<This>& proc, const Instruction& inst)
    {
        typename This::part_type::Honest::Protocol protocol(proc.P);
        protocol.init_mul();
        for (int i = 0; i < inst.get_size(); i++)
        {
            auto& dest = proc.get_S_ref(inst.get_r(0) + i);
            auto& source = proc.get_S_ref(inst.get_r(1) + i);
            dest.set_share(Rep3Share2<K>(source.get_share()) >> inst.get_n());
            protocol.prepare_mul(dest.get_share(), proc.MC.get_alphai());
        }
        protocol.exchange();
        for (int i = 0; i < inst.get_size(); i++)
        {
            auto& dest = proc.get_S_ref(inst.get_r(0) + i);
            dest.set_mac(protocol.finalize_mul());
            proc.protocol.add_to_check(dest);
        }
    }
};

template <class T>
using CovertMacFieldShare = SpdzWiseRepFieldShare<T>;

template<class T>
class DzkpShare : public Rep3Share<T>
{
    typedef Rep3Share<T> super;
    typedef DzkpShare This;

public:
    typedef Ccs24<DzkpShare<T>> Protocol;
    typedef Ccs24MC<DzkpShare<T>> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<DzkpShare<T>> Input;
    typedef SpecificPrivateOutput<DzkpShare<T>> PrivateOutput;
    typedef ReplicatedPrep<DzkpShare<T>> LivePrep;
    typedef Rep3Share<T> Honest;
    typedef T random_type;
    typedef This Scalar;

    typedef GC::BGIN19Secret bit_type;

    // indicate security relevance of field size
    typedef T mac_key_type;

    const static bool expensive = true;
    static const bool has_trunc_pr = false;
    static const bool malicious = true;

    static const int OnlineSecurity = 0;
    static constexpr int CCS_RING_GAP = 12;
    static constexpr int CCS_LAMBDA = 2;

    static string type_short()
    {
        return "R" + string(1, T::type_char());
    }
    static string type_string()
    {
        return "Dzkp " + super::type_string();
    }

    DzkpShare()
    {
    }
    DzkpShare(const T& other, int my_num, T alphai = {}) :
            super(other, my_num, alphai)
    {
    }
    template<class U>
    DzkpShare(const U& other) : super(other)
    {
    }

    static DzkpShare get_zero() {
        DzkpShare res;
        res.assign_zero();
        return res;
    }
};

template<int K>
class DzkpRingShare : public DzkpShare<Z2<K>>
{
    typedef Z2<K> T;
    typedef DzkpRingShare This;
    typedef Z2<K> ExtendedOnline;

public:
    typedef Ccs24<DzkpRingShare> Protocol;
    typedef Ccs24MC<DzkpRingShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<DzkpRingShare> Input;
    typedef SpecificPrivateOutput<This> PrivateOutput;
    typedef MaliciousRingPrep<DzkpRingShare> LivePrep;
    typedef Replicated<T> Honest;

    typedef DzkpRingShare Scalar;
    typedef SignedZ2<K> clear;

    typedef GC::BGIN19Secret bit_type;

    static const bool has_split = false;

    DzkpRingShare()
    {
    }
    template<class U>
    DzkpRingShare(const FixedVec<U, 2>& other)
    {
        FixedVec<T, 2>::operator=(other);
    }


    template<class U>
    static void split(StackedVector<U>& dest, const vector<int>& regs, int n_bits,
            const DzkpRingShare* source, int n_inputs,
            typename U::Protocol& protocol)
    {
        auto& P = protocol.P;
        int my_num = P.my_num();
        int unit = GC::Clear::N_BITS;
        for (int k = 0; k < DIV_CEIL(n_inputs, unit); k++)
        {
            int start = k * unit;
            int m = min(unit, n_inputs - start);

            switch (regs.size() / n_bits)
            {
            case 3:
                for (int l = 0; l < n_bits; l += unit)
                {
                    int base = l;
                    int n_left = min(n_bits - base, unit);
                    for (int i = base; i < base + n_left; i++)
                        dest.at(regs.at(3 * i + my_num) + k) = {};

                    for (int i = 0; i < 2; i++)
                    {
                        square64 square;

                        for (int j = 0; j < m; j++)
                            square.rows[j] = source[j + start][i].get_limb(
                                    l / unit);

                        square.transpose(m, n_left);

                        for (int j = 0; j < n_left; j++)
                        {
                            auto& dest_reg = dest.at(
                                    regs.at(3 * (base + j) + ((my_num + 2 - i) % 3))
                                            + k);
                            dest_reg[1 - i] = 0;
                            dest_reg[i] = square.rows[j];
                        }
                    }
                }
                break;
            default:
                throw runtime_error("number of split summands not implemented");
            }
        }
    }
};

#endif