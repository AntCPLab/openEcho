#ifndef PROTOCOLS_FLNWPREP_HPP_
#define PROTOCOLS_FLNWPREP_HPP_

#include "GC/FlnwPrep.h"
#include "Protocols/Replicated.h"
#include "Protocols/ShuffleSacrifice.h"

namespace GC {

template <class T>
void FlnwPrep<T>::buffer_triples() {

    size_t size = OnlineOptions::singleton.binary_batch_size;

    int B = 3;
    int C = 3;

    Player& P = *(this->P);
    Replicated<T> protocol(P);
    TripleShuffleSacrifice<T> sacrifice;

    local_triples.reserve(size * B + C);

    generate_triples(local_triples, size * B + C, &protocol);

    sacrifice.shuffle(local_triples, P);

    vector<T> shares;
    for (int i = 0; i < C; i++)
    {
        for (int j = 0; j < 3; j++)
            shares.push_back(local_triples.back()[j]);
        local_triples.pop_back();
    }
    vector<typename T::open_type> opened;
    MC.POpen(opened, shares, P);
    for (int i = 0; i < C; i++)
        if (typename T::clear(opened[3 * i] * opened[3 * i + 1])
                != typename T::clear(opened[3 * i + 2]))
            cout << "Open C failed" << endl;
            // throw Offline_Check_Error("shuffle opening");

    TripleShuffleSacrifice<T>(B, 0).triple_sacrifice(local_triples, local_triples, P, MC, 0, size);

    // 优化：使用 reserve + insert 代替 resize + 逐个赋值
    // 这避免了先分配零初始化内存再覆盖的问题，减少 page fault
    size_t now_size = this->triples.size();
    this->triples.reserve(now_size + size);  // 先预留空间
    this->triples.insert(this->triples.end(), local_triples.data(), local_triples.data() + size);
}

template <class T>
void FlnwPrep<T>::get_triples(array<T, 3>* target_triples, size_t num, int n_bits) {
    if (num == 0) return;

    prep_timer.start();
    
    if (n_bits == -1)   n_bits = T::default_length;
    
    // 预计算常量，避免重复计算
    const int triples_per_full = T::clear::N_BITS / n_bits;  // 每个完整triple可提供多少个n_bits triple
    const int usable_bits_per_triple = triples_per_full * n_bits;  // 实际使用的位数
    
    // 为了极致效率，允许浪费triple尾部的几个bit
    size_t required_full_triples = (num + triples_per_full - 1) / triples_per_full;
    
    // target_triples.resize(num);
    
    // 确保有足够的triple，减少buffer_triples调用
    while (this->triples.size() < required_full_triples) {
        buffer_triples();
    }
    
    // 高效批量处理：直接访问连续内存
    array<T, 3>* output_ptr = target_triples;
    size_t output_idx = 0;
    
    // 从后往前消耗triple，减少内存移动
    size_t start_idx = this->triples.size() - required_full_triples;
    
    for (size_t triple_idx = 0; triple_idx < required_full_triples && output_idx < num; triple_idx++) {
        const auto& source_triple = this->triples[start_idx + triple_idx];
        
        // 展开内层循环，减少分支预测失误
        if (triples_per_full >= 1 && output_idx < num) {
            // 第一个n_bits（offset = 0）
            output_ptr[output_idx][0] = source_triple[0].mask(n_bits);
            output_ptr[output_idx][1] = source_triple[1].mask(n_bits);
            output_ptr[output_idx][2] = source_triple[2].mask(n_bits);
            output_idx++;
        }
        
        if (triples_per_full >= 2 && output_idx < num) {
            // 第二个n_bits（offset = n_bits）
            output_ptr[output_idx][0] = (source_triple[0] >> n_bits).mask(n_bits);
            output_ptr[output_idx][1] = (source_triple[1] >> n_bits).mask(n_bits);
            output_ptr[output_idx][2] = (source_triple[2] >> n_bits).mask(n_bits);
            output_idx++;
        }
        
        // 如果需要更多，用循环处理剩余的（通常很少）
        for (int offset = 2 * n_bits; offset < usable_bits_per_triple && output_idx < num; offset += n_bits) {
            output_ptr[output_idx][0] = (source_triple[0] >> offset).mask(n_bits);
            output_ptr[output_idx][1] = (source_triple[1] >> offset).mask(n_bits);
            output_ptr[output_idx][2] = (source_triple[2] >> offset).mask(n_bits);
            output_idx++;
        }
    }
    
    // 直接丢弃使用过的triple，不保留部分使用的
    // 这是关键优化：为了性能牺牲一些triple的尾部bit
    // 使用 resize 而不是 erase，因为我们是从末尾移除，resize 不需要 memmove
    if (required_full_triples > 0) {
        this->triples.resize(this->triples.size() - required_full_triples);
    }

    prep_timer.stop();
}

}

#endif