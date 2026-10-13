#ifndef _TOOLS_MY_UTILS_H_
#define _TOOLS_MY_UTILS_H_

#include "Tools/random.h"

template <class U>
inline void shuffle(vector<U>& buffer, PRNG& prng, int size = -1);

template <class U>
inline void parallel_shuffle(vector<vector<U>>& buffer, PRNG& prng, int size = -1);

#endif