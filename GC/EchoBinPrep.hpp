#ifndef PROTOCOLS_ECHOBINPREP_HPP_
#define PROTOCOLS_ECHOBINPREP_HPP_

#include "EchoBinPrep.h"

namespace GC {

template <class T>
inline void partial_shuffle(T& triples, size_t vector_size, size_t shuffle_size, PRNG& G) {
    for (size_t i = vector_size - shuffle_size; i < vector_size; i ++) {
        int idx = G.get_uint(i);
        swap(triples[i], triples[idx]);
    }
}

template <class T>
void EchoBinPrep<T>::get_triples(array<T, 3>* target_triples, size_t num, int n_bits) {
    if (num == 0) return;
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
}

template <class T>
void EchoBinPrep<T>::buffer_triples() {
    Player& P = this->protocol->P;
    array<PRNG, 2>& shared_prngs = this->prng_protocol->shared_prngs;

    size_t size = OnlineOptions::singleton.binary_batch_size;
    // size_t size = 30482107;
    size_t size1 = ceil((2 / (double) (2 - OnlineOptions::singleton.deterrent)) * size);
    size_t size2 = size1 - size;

    auto& tmp_triples = this->tmp_triples;
    tmp_triples.reserve(size1 + 1);
    zeros.reserve(size2);

    array<octetStream, 2> os;
    os[0].reserve((size1 + 1) * sizeof(clear));
    random_buffer.reserve((size1 + 1) * 6);
    add_shares.reserve(max(size1 + 1, 2 * size2));

    shared_prngs[0].get_octets((octet*)random_buffer.data(), (size1 + 1) * sizeof(clear) * 3);
    shared_prngs[1].get_octets((octet*)(random_buffer.data() + (size1 + 1) * 3), (size1 + 1) * sizeof(clear) * 3);

    clear* ptr1 = random_buffer.data();
    clear* ptr2 = ptr1 + (size1 + 1) * 3;
    octet* os_ptr = os[0].append((size1 + 1) * sizeof(clear));
    clear* ptr = (clear*) os_ptr;

    for (size_t i = 0; i < size1 + 1; i ++) {
        T& x = tmp_triples[i][0];
        T& y = tmp_triples[i][1];
        T& z = tmp_triples[i][2];

        x[0] = ptr1[0]; y[0] = ptr1[1];
        x[1] = ptr2[0]; y[1] = ptr2[1];

        ptr[i] = x[0] * (y[0] + y[1]) + x[1] * y[0] + ptr1[2] - ptr2[2];
        ptr1 += 3; ptr2 += 3;
        z[0] = ptr[i];
    }

    P.pass_around(os[0], os[1], 1);
    ptr1 = (clear*) os[1].get_data();

    for (size_t i = 0; i < size1 + 1; i ++) {
        T& z = tmp_triples[i][2];
        z[1] = ptr1[0];
        ptr1 ++;
    }

    partial_shuffle(tmp_triples, size1 + 1, size2 + 1, *global_prng);
    os[0].reset_write_head();
    os[0].reserve((size2 + 3) * sizeof(clear));
    add_shares.clear();

    for (int i = 0; i < 3; i ++) {
        tmp_triples[size1][i][0].pack(os[0]);
    }
    
    os_ptr = os[0].append(size2 * 2 * sizeof(clear));
    ptr1 = (clear*) os_ptr;
    
    for (size_t i = 0; i < size2; i ++) {
        T rho = tmp_triples[i][0] - tmp_triples[i + size][0];
        T sigma = tmp_triples[i][1] - tmp_triples[i + size][1];

        ptr1[0] = rho[0];
        ptr1[1] = sigma[0];

        add_shares.push_back(rho.sum());
        add_shares.push_back(sigma.sum());

        ptr1 += 2;
    }

    P.pass_around(os[0], os[1], -1);

    if ((tmp_triples[size1][0].sum() + os[1].get<clear>()) * 
        (tmp_triples[size1][1].sum() + os[1].get<clear>()) != 
        (tmp_triples[size1][2].sum() + os[1].get<clear>())) {
            cout << "Open C failed." << endl;
    }

    ptr1 = (clear*) os[1].get_data_ptr();

    for (size_t i = 0; i < size2; i ++) {
        clear rho = add_shares.next() + ptr1[0];
        clear sigma = add_shares.next() + ptr1[1];
        zeros.push_back(tmp_triples[i][2] - tmp_triples[i + size][2] - tmp_triples[i][1] * rho - tmp_triples[i + size][0] * sigma + rho * sigma);
        ptr1 += 2;
    }

    try {
        MC.CheckFor(0, zeros, P);
    }
    catch (const mac_fail& e) {
        cout << "CheckZero failed in Buffer triples. " << endl;
    }
    
    // 优化：使用 insert 或直接批量操作代替 resize + 逐个赋值
    // 这避免了先分配零初始化内存再覆盖的问题，减少 page fault
    size_t now_size = this->triples.size();
    this->triples.reserve(now_size + size);  // 先预留空间，避免多次重分配
    // 使用 insert 进行批量添加，比 resize + 循环赋值更高效
    this->triples.insert(this->triples.end(), tmp_triples.data(), tmp_triples.data() + size);
}

}

#endif