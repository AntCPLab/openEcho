#ifndef PROTOCOLS_CCS24MC_HPP_
#define PROTOCOLS_CCS24MC_HPP_

#include "Ccs24MC.h"
#include "Processor/OnlineOptions.h"
#include "Tools/CCS24Check.hpp"

template <class T>
void Ccs24MC<T>::MaybeCheck(const Player& P, bool final) {
    if (needs_checking)
    {
        CODE_LOCATION
        vector<octetStream> os(P.num_players());
        hash.final(os[P.my_num()]);
        reset();
        P.Broadcast_Receive(os);
        for (int i = 0; i < P.num_players(); i++)
            if (os[i] != os[P.my_num()]) {
                // throw mac_fail("check hash mismatch");
                // cout << "Check hash mismatch." << endl;
            }
    }

    if (OnlineOptions::singleton.has_option("noccs")) {
        rx.clear();
        ry.clear();
        rz1.clear();
        rz2.clear();
        vector_sizes.clear();
        buffer_size = 0;
        this->reset();
        return ;
    }

    if (buffer_size < (size_t) OnlineOptions::singleton.batch_size && !final) return ;
    
    size_t pointer_items = 0, pointer_vals = 0;
    while (true) {
        size_t now_size = 0, counter = 0;
        while (now_size < (size_t) OnlineOptions::singleton.batch_size && (counter + pointer_vals < vector_sizes.size())) {
            now_size += vector_sizes[counter + pointer_vals];
            counter++;
            if (counter + pointer_vals >= vector_sizes.size()) break;
        }
        if (now_size < (size_t) OnlineOptions::singleton.batch_size) break;
        check_ccs(rx.data() + pointer_items, ry.data() + pointer_items, rz1.data() + pointer_vals, 
                rz2.data() + pointer_vals, vector_sizes.data() + pointer_vals, counter, P, shared_prngs, 7, 52);
        
        pointer_items += now_size;
        pointer_vals += counter;
    }

    if (pointer_vals < vector_sizes.size()) {
        check_ccs(rx.data() + pointer_items, ry.data() + pointer_items, rz1.data() + pointer_vals, 
            rz2.data() + pointer_vals, vector_sizes.data() + pointer_vals, vector_sizes.size() - pointer_vals, P, shared_prngs, 7, 52);
    }
    
    rx.clear();
    ry.clear();
    rz1.clear();
    rz2.clear();
    vector_sizes.clear();
    buffer_size = 0;
    this->reset();
}

#endif