#ifndef PROTOCOLS_ABFLMC_HPP_
#define PROTOCOLS_ABFLMC_HPP_

#include "ABFLMC.h"

template <class T>
void ABFLMC<T>::Check(const Player& P) {
    _impl.Check(P);
    if (OnlineOptions::singleton.bucket_size == 0) {
        (void) P;
        to_be_checked.clear();
        return ;
    }
    else {
        MaybeCheck(P, false);
    }
}

template <class T>
void ABFLMC<T>::MaybeCheck(const Player& P, bool final) {
    int size = to_be_checked.size();
    if (size == 0 || checking) {
        return;
    }
    if (!final && size < (int) OnlineOptions::singleton.binary_batch_size) {
        return;
    }

    checking = true;

    int B = 2;
    int C = 2;
    Player &P2 = const_cast<Player&>(P);
    Replicated<T> protocol(P2);
    TripleShuffleSacrifice<T> sacrifice;

    generate_triples(triple_buffer, size * B + C, &protocol);
    // triples.resize(size * B);
    
    sacrifice.shuffle(triple_buffer, P2);

    vector<T> shares;
    for (int i = 0; i < C; i++)
    {
        for (int j = 0; j < 3; j++)
            shares.push_back(triple_buffer.back()[j]);
        triple_buffer.pop_back();
    }
    vector<typename T::open_type> opened;
    this->POpen(opened, shares, P);
    for (int i = 0; i < C; i++)
        if (typename T::clear(opened[3 * i] * opened[3 * i + 1])
                != typename T::clear(opened[3 * i + 2]))
            cout << "Open C failed" << endl;
            // throw Offline_Check_Error("shuffle opening");

    triple_buffer.resize(size * (B + 1));
    memcpy(triple_buffer.data() + size * B, to_be_checked.data(), B * sizeof(array<T, 3>));
    to_be_checked.clear();
    vector<array<T, 3> > _(size);
    TripleShuffleSacrifice<T>(B + 1, 0).triple_sacrifice(_, triple_buffer, P2, *this, 0, size);
    triple_buffer.clear();

    checking = false;
}

#endif