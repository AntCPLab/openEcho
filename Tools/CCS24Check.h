#ifndef TOOLS_CCS24CHECK_H_
#define TOOLS_CCS24CHECK_H_

#include "Networking/Player.h"
#include "Math/Z2k.hpp"
#include "Math/gf2nlong.h"

// using ExtendedOffline = Z2<64>;

template <class T>
T inner_product(T* x, T* y, int size);

template <class ExtendedOffline>
void prover(ExtendedOffline* x, ExtendedOffline* y, ExtendedOffline z1, ExtendedOffline z2, int size, const Player& P, array<PRNG, 2>& shared_prngs, int k);

template <class ExtendedOffline>
void verifier_x(ExtendedOffline* x, ExtendedOffline z1, int size, const Player& P, array<PRNG, 2>& shared_prngs, int k);

template <class ExtendedOffline>
void verifier_y(ExtendedOffline* y, ExtendedOffline z2, int size, const Player& P, array<PRNG, 2>& shared_prngs, int k);

void prover(gf2n_long* x, gf2n_long* y, gf2n_long z1, gf2n_long z2, int size, const Player& P, array<PRNG, 2>& shared_prngs, int k) {
    (void) x; (void) y; (void) z1; (void) z2; (void) size; (void) P; (void) shared_prngs; (void) k;
}

void verifier_x(gf2n_long* x, const gf2n_long& z1, int size, const Player& P, array<PRNG, 2>& shared_prngs, int k) {
    (void) x; (void) z1; (void) size; (void) P; (void) shared_prngs; (void) k;
}

void verifier_y(gf2n_long* y, const gf2n_long& z2, int size, const Player& P, array<PRNG, 2>& shared_prngs, int k) {
    (void) y; (void) z2; (void) size; (void) P; (void) shared_prngs; (void) k;
}

template <class T>
void check_ccs(T *x, T *y, T *z0, T *z1, size_t *sizes, int m, const Player &P, array<PRNG, 2>& shared_prngs, int provers, int k);

#endif