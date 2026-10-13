#ifndef PROTOCOLS_USSCCSRINGSHARE_H_
#define PROTOCOLS_USSCCSRINGSHARE_H_

#include "UssCcsShare.h"
#include "Protocols/MaliciousRepPrep.hpp"

#include "Math/Z2k.hpp"
#include "GC/square64.h"

template<int K>
class UssCcsRingShare : public UssCcsShare<Z2<K>>
{
    typedef Z2<K> T;
    typedef UssCcsRingShare This;
    typedef Z2<K> ExtendedOnline;

public:
    typedef Ccs24<UssCcsRingShare> Protocol;
    typedef Ccs24MC<UssCcsRingShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<UssCcsRingShare> Input;
    typedef SpecificPrivateOutput<This> PrivateOutput;
    typedef MaliciousRingPrep<UssCcsRingShare> LivePrep;
    typedef Replicated<T> Honest;

    typedef UssCcsRingShare Scalar;
    typedef SignedZ2<K> clear;

    typedef GC::Uss23Secret bit_type;

    static const bool has_split = false;

    UssCcsRingShare()
    {
    }
    template<class U>
    UssCcsRingShare(const FixedVec<U, 2>& other)
    {
        FixedVec<T, 2>::operator=(other);
    }


    template<class U>
    static void split(StackedVector<U>& dest, const vector<int>& regs, int n_bits,
            const UssCcsRingShare* source, int n_inputs,
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