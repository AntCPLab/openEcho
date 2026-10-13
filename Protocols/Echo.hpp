#ifndef PROTOCOLS_ECHO_HPP_
#define PROTOCOLS_ECHO_HPP_

#include "Echo.h"
#include "Astra.hpp"
#include "Processor/OnlineOptions.h"
#include "Tools/CodeLocations.h"
#include "Tools/ranges.h"
#include "Tools/time-func.h"

#include <chrono>

// Thread-local变量用于累积写入字节数统计
thread_local size_t g_thread_bytes_written = 0;

// Component timing support for Echo protocol
namespace EchoTiming {
    inline thread_local double arith_mul_prepare_time = 0;
    inline thread_local double arith_mul_exchange_time = 0;
    inline thread_local double arith_mul_finalize_time = 0;
    inline thread_local double dotprod_prepare_time = 0;
    inline thread_local double dotprod_next_time = 0;
    inline thread_local double dotprod_exchange_time = 0;
    inline thread_local double dotprod_finalize_time = 0;
    inline thread_local double binary_prepare_time = 0;
    inline thread_local double binary_exchange_time = 0;
    inline thread_local double binary_finalize_time = 0;
    inline thread_local double unsplit_time = 0;
    inline thread_local double prep_read_time = 0;
    inline thread_local double prep_write_time = 0;
    inline thread_local int64_t arith_mul_count = 0;
    inline thread_local int64_t dotprod_count = 0;
    inline thread_local int64_t binary_count = 0;
    inline thread_local int64_t unsplit_count = 0;
    inline thread_local int64_t prep_read_count = 0;
    inline thread_local int64_t prep_write_count = 0;
    inline thread_local bool already_printed = false;
    
    inline void reset() {
        arith_mul_prepare_time = 0;
        arith_mul_exchange_time = 0;
        arith_mul_finalize_time = 0;
        dotprod_prepare_time = 0;
        dotprod_next_time = 0;
        dotprod_exchange_time = 0;
        dotprod_finalize_time = 0;
        binary_prepare_time = 0;
        binary_exchange_time = 0;
        binary_finalize_time = 0;
        unsplit_time = 0;
        prep_read_time = 0;
        prep_write_time = 0;
        arith_mul_count = 0;
        dotprod_count = 0;
        binary_count = 0;
        unsplit_count = 0;
        prep_read_count = 0;
        prep_write_count = 0;
        already_printed = false;
    }
    
    inline void print_report(int party_num = -1, int thread_num = -1) {
        // Only print once per thread
        if (already_printed) return;
        already_printed = true;
        
        double arith_mul_total = arith_mul_prepare_time + arith_mul_exchange_time + arith_mul_finalize_time;
        double dotprod_total = dotprod_prepare_time + dotprod_next_time + dotprod_exchange_time + dotprod_finalize_time;
        double binary_total = binary_prepare_time + binary_exchange_time + binary_finalize_time;
        double prep_total = prep_read_time + prep_write_time;
        double total = arith_mul_total + dotprod_total + binary_total + unsplit_time + prep_total;
        
        if (total < 1e-6) return;
        
        std::cout << "\n=== Echo Component Timing";
        if (party_num >= 0) std::cout << " (P" << party_num;
        if (thread_num >= 0) std::cout << ", T" << thread_num;
        if (party_num >= 0) std::cout << ")";
        std::cout << " ===" << std::endl;
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Arith Mul Prepare:   " << arith_mul_prepare_time << "s" << std::endl;
        std::cout << "Arith Mul Exchange:  " << arith_mul_exchange_time << "s" << std::endl;
        std::cout << "Arith Mul Finalize:  " << arith_mul_finalize_time << "s" << std::endl;
        std::cout << "  -> Arith Mul Total: " << arith_mul_total << "s (" << arith_mul_count << " calls)" << std::endl;
        std::cout << "Dotprod Prepare:     " << dotprod_prepare_time << "s" << std::endl;
        std::cout << "Dotprod Next:        " << dotprod_next_time << "s" << std::endl;
        std::cout << "Dotprod Exchange:    " << dotprod_exchange_time << "s" << std::endl;
        std::cout << "Dotprod Finalize:    " << dotprod_finalize_time << "s" << std::endl;
        std::cout << "  -> Dotprod Total:   " << dotprod_total << "s (" << dotprod_count << " calls)" << std::endl;
        std::cout << "Binary Prepare:      " << binary_prepare_time << "s" << std::endl;
        std::cout << "Binary Exchange:     " << binary_exchange_time << "s" << std::endl;
        std::cout << "Binary Finalize:     " << binary_finalize_time << "s" << std::endl;
        std::cout << "  -> Binary Total:    " << binary_total << "s (" << binary_count << " calls)" << std::endl;
        std::cout << "Unsplit:             " << unsplit_time << "s (" << unsplit_count << " calls)" << std::endl;
        std::cout << "Prep Read:           " << prep_read_time << "s (" << prep_read_count << " calls)" << std::endl;
        std::cout << "Prep Write:          " << prep_write_time << "s (" << prep_write_count << " calls)" << std::endl;
        std::cout << "  -> Prep Total:      " << prep_total << "s" << std::endl;
        std::cout << "---" << std::endl;
        std::cout << "Protocol Total:      " << total << "s" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
    }
}

inline std::string now_ms() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::tm tm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

template <class T>
void EchoPrepProtocol<T>::exchange_mul() {
    CODE_LOCATION

    auto& P = this->P;
    auto& results = this->results;
    int my_num = P.my_num();

    assert(results.size() == 0);
    auto& prng_protocol = this->prng_protocol;

    size_t round_size = this->input_pairs.size();
    results.reserve(round_size);

    MC->fill_size(round_size);

    if (my_num == 0) {
        random_buffer.resize(5 * round_size * clear::N_BYTES);

        octet* os_ptr = os.append(round_size * clear::N_BYTES * 3);
        prng_protocol.shared_prngs[0].get_octets(random_buffer.data(), round_size * clear::N_BYTES * 4);
        prng_protocol.shared_prngs[1].get_octets(random_buffer.data() + round_size * clear::N_BYTES * 4, round_size * clear::N_BYTES);
        clear* random_data = reinterpret_cast<clear*>(random_buffer.data());
        clear* random_data2 = random_data + round_size * 4;
        clear* random_lambda = random_data + round_size * 3;

        for (size_t i = 0; i < round_size; i++) {
            const auto& xy = this->input_pairs[i];
            clear rxy = xy[0].sum() * xy[1].sum();

            
            clear rxy_1 = (*random_data);

            clear rxy_2 = rxy - rxy_1;
            clear alpha_x = key * xy[0].sum();
            clear alpha_y = key * xy[1].sum();

            memcpy(os_ptr, rxy_2.get_ptr(), clear::N_BYTES);
            os_ptr += clear::N_BYTES;
            memcpy(os_ptr, (alpha_x - random_data[1]).get_ptr(), clear::N_BYTES);
            os_ptr += clear::N_BYTES;
            memcpy(os_ptr, (alpha_y - random_data[2]).get_ptr(), clear::N_BYTES);
            os_ptr += clear::N_BYTES;

            results.emplace_back(*random_lambda, *random_data2);
            const T& res = results.back();

            const auto& x = xy[0], y = xy[1];
            this->MC->add_rss_z(res);
            MC->add_rz1(rxy_1 - x[0] * y[0]);
            MC->add_rz2(rxy_2 - x[1] * y[1]);
            MC->add_mac_z(rxy - res[0] - res[1]);

            random_data += 3;
            random_lambda ++;
            random_data2 ++;
        }

        P.send_to(2, os);

        for (size_t i = 0; i < round_size; i++) {
            const auto& xy = this->input_pairs[i];
            const auto& x = xy[0], y = xy[1];
            MC->add_rx(x[0]);   MC->add_rx(y[0]);
            MC->add_ry(y[1]);   MC->add_ry(x[1]);
        }
    }
    else if (my_num == 1) {

        octet* os_prep_ptr = os_prep.append(round_size * clear::N_BYTES * 4);
        prng_protocol.shared_prngs[1].get_octets(os_prep_ptr, round_size * clear::N_BYTES * 4);

        clear* random_data = reinterpret_cast<clear*>(os_prep_ptr);
        clear* random_lambda = random_data + 3 * round_size;

        for (const auto& xy: this->input_pairs) {
            const auto& x = xy[0], y = xy[1];
            MC->add_rx(x.neg_lambda(my_num));   MC->add_rx(y.neg_lambda(my_num));
            MC->add_rz1(*random_data - x.neg_lambda(my_num) * y.neg_lambda(my_num));
            results.emplace_back(0, *random_lambda);
            this->MC->add_mac_z(1);

            random_data += 3;
            random_lambda ++;
        }

        this->store(os_prep);
    }
    else {
        octet* os_prep_ptr = os_prep.append(round_size * clear::N_BYTES);
        prng_protocol.shared_prngs[0].get_octets(os_prep_ptr, round_size * clear::N_BYTES);
        clear* random_data = reinterpret_cast<clear*>(os_prep_ptr);

        for (const auto& xy : this->input_pairs) {

            results.emplace_back(0, *random_data);

            const auto& x = xy[0], y = xy[1];
            MC->add_ry(x.neg_lambda(my_num));   MC->add_ry(y.neg_lambda(my_num));
            this->MC->add_mac_z(1);
        }
        
        P.receive_player(0, os);

        octet* ptr = os.get_data();
        clear* rxy_2_ptr = reinterpret_cast<clear*>(ptr);

        for (const auto& xy: this->input_pairs) {
            const auto& x = xy[0], y = xy[1];
            MC->add_rz2((*rxy_2_ptr) - x.neg_lambda(my_num) * y.neg_lambda(my_num));
            rxy_2_ptr += 3;
        }

        this->store(os_prep);
        this->store(os);
    }

    results.reset();
    this->input_pairs.clear();
}

template <class T>
void EchoPrepProtocol<T>::exchange_dot() {
    size_t round_size = dot_results.size();
    Player& P = this->P;
    int my_num = P.my_num();

    if (my_num == 0) {
        P.send_to(2, os);
    }
    else if (my_num == 1) {
        this->store(os_prep);
    }
    else {
        P.receive_player(0, os);
        clear* os_ptr = reinterpret_cast<clear*>(os.get_data());
        for (size_t i = 0; i < round_size; i ++) {
            int this_size = dot_sizes[i];
            os_ptr += (this_size * 2 + 1);
            MC->add_rz2(os_ptr[0] - dot_results[i][0]);
        }
        this->store(os_prep);
        this->store(os);
    }

    dot_results.reset();
}

template <class T>
void Echo<T>::exchange_mul() {
    CODE_LOCATION
    
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    Timer _exchange_timer;
    _exchange_timer.start();
#endif

    if (this->P.my_num() == 0) {
        size_t batch_size = this->input_pairs.size();
        this->results.resize(batch_size);
        this->input_pairs.clear();
        this->results.reset();
#ifdef ENABLE_ECHO_COMPONENT_TIMING
        _exchange_timer.stop();
        EchoTiming::arith_mul_exchange_time += _exchange_timer.elapsed();
        EchoTiming::arith_mul_count += batch_size;  // Count actual multiplications, not batches
#endif
        return ;
    }

    auto& P = this->P;
    auto& inputs = this->inputs;
    auto& results = this->results;
    assert(results.size() == 0);

    (void) inputs;
    size_t round_size = this->input_pairs.size();
    size_t n_mults = this->inputs.size() + round_size;

    int my_num = P.my_num();

    os.clear();
    os.reserve(round_size * clear::N_BYTES);
    
    if (OnlineOptions::singleton.has_option("nofile")) {
        os_prep.append(clear::N_BYTES * round_size * 4);
    }
    else {
        this->read(os_prep);
    }

    clear* prep_ptr = reinterpret_cast<clear*>(os_prep.get_data());

    if (my_num == 1) {
        if (os_prep.left() < clear::N_BYTES * round_size * 4) {
            cout << "os_prep left: " << os_prep.left() << ", round_size: " << round_size << endl;
            throw runtime_error("insufficient preprocessing");
        }

        prep_ptr += round_size * 3;
    }

    // 预先分配内存，避免循环中多次 realloc 导致的 page fault
    results.reserve(round_size);
    MC->reserve_for_mul(round_size);

    for (size_t i = 0; i < round_size; i++) {
        T res;
        res[1] = prep_ptr[0];
        results.push_back(res);
        prep_ptr ++;
    }

    if (my_num == 2) {
        if (OnlineOptions::singleton.has_option("nofile")) {
            os_prep.append(3 * round_size * clear::N_BYTES);
        }
        else {
            this->read(os_prep);
        }
    }

    results.reset();
    prep_ptr = reinterpret_cast<clear*>(os_prep.get_data());
    clear* os_ptr = reinterpret_cast<clear*>(os.append(n_mults * clear::size()));

    for (auto& xy: this->input_pairs) {
        const T& x = xy[0], y = xy[1];
        T& res = results.next();

        clear m_z = x[0] * (y[0] * (my_num - 1) + y[1]) + x[1] * y[0] + prep_ptr[0] - res[1];
        os_ptr[0] = m_z;

        clear mac_z = x[0] * (alpha[my_num - 1] * (my_num - 1) * y[0] + prep_ptr[2]) + y[0] * prep_ptr[1];
        MC->add_mac_z(mac_z);

        res[0] = m_z;

        prep_ptr += 3;
        os_ptr ++;
    }

    octetStream recv_os;
    P.exchange(3 - P.my_num(), os, recv_os);
    if (recv_os.left() < clear::size() * results.size()) {
        throw runtime_error("insufficient data in Echo");
    }

    clear* recv_ptr = reinterpret_cast<clear*>(recv_os.get_data());

    for (size_t i = 0; i < round_size; i ++) {
        results[i][0] += recv_ptr[i];
    }
    // assert(os_prep.left() <= 8u * clear::size());

    this->MC->add_rss_z(results);
    results.reset();
    this->input_pairs.clear();
    
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    _exchange_timer.stop();
    EchoTiming::arith_mul_exchange_time += _exchange_timer.elapsed();
    EchoTiming::arith_mul_count += round_size;  // Count actual multiplications, not batches
#endif
}

template <class T>
void Echo<T>::exchange_dot() {
    int my_num = this->P.my_num();
    if (my_num == 0) {
        dot_results.reset();
        return ;
    }

    this->P.exchange(3 - my_num, os, recv_os);
    size_t round_size = dot_results.size();
    clear* os_ptr = reinterpret_cast<clear*>(recv_os.get_data());

    for (size_t i = 0; i < round_size; i ++) {
        dot_results[i][0] += os_ptr[i];
    }

    MC->add_rss_z(dot_results);
    dot_results.reset();
}

template <class T>
void EchoPrepProtocol<T>::init_dotprod() {
    dot_pairs.clear();
    dot_results.clear();
    os_prep.reset_write_head();
    os.reset_write_head();
    dot_sizes.clear();
    this->input_pairs.clear();

    // Pre-allocate a reasonable initial capacity to reduce reallocations
    // during matrix multiplications with large inner dimensions.
    if (dot_pairs.capacity() < ECHO_DOT_PAIRS_INITIAL_CAPACITY) {
        dot_pairs.reserve(ECHO_DOT_PAIRS_INITIAL_CAPACITY);
    }

    if (MC) {
        MC->MaybeCheck(this->P);
    }
}

template <class T>
void EchoPrepProtocol<T>::prepare_dotprod(const T& x, const T& y) {
    dot_pairs.push_back({x, y});
}

template <class T>
void EchoPrepProtocol<T>::next_dotprod() {
    size_t round_size = dot_pairs.size();
    if (round_size == 0) return;

    auto& shared_prngs = this->prng_protocol.shared_prngs;
    auto& P = this->P;

    dot_results.push_back({});
    clear rz1 = 0, rz2 = 0;

    if (P.my_num() == 0) {
        dot_rxy = 0;
        random_buffer.resize((round_size * 2 + 1) * clear::N_BYTES);
        octet* ptr = random_buffer.data();
        shared_prngs[0].get_octets(ptr, (round_size * 2 + 1) * clear::N_BYTES);
        clear* random_ptr = reinterpret_cast<clear*>(ptr);

        clear* os_ptr = reinterpret_cast<clear*>(os.append((round_size * 2 + 1) * clear::N_BYTES));

        for (size_t i = 0; i < round_size; i ++) {
            const auto& x = dot_pairs[i][0];
            const auto& y = dot_pairs[i][1];
            os_ptr[i * 2] = key * x.sum() - random_ptr[i * 2];
            os_ptr[i * 2 + 1] = key * y.sum() - random_ptr[i * 2 + 1];
            dot_rxy += x.sum() * y.sum();
            rz1 += x[0] * y[0];
            rz2 += x[1] * y[1];
            MC->add_rx(x[0]);  MC->add_rx(y[0]);
            MC->add_ry(y[1]);  MC->add_ry(x[1]);
        }
        os_ptr[round_size * 2] = dot_rxy - random_ptr[round_size * 2];
        MC->add_rz1(random_ptr[round_size * 2] - rz1);
        MC->add_rz2(os_ptr[round_size * 2] - rz2);
        T& res = dot_results.back();
        res[0].randomize(shared_prngs[0]);
        res[1].randomize(shared_prngs[1]);
        MC->add_mac_z(dot_rxy - res.sum());
        MC->add_rss_z(res);
    }
    else if (P.my_num() == 1) {
        clear* os_ptr = reinterpret_cast<clear*>(os_prep.append((round_size * 2 + 2) * clear::N_BYTES));
        shared_prngs[1].get_octets((octet*) os_ptr, (round_size * 2 + 2) * clear::N_BYTES);
        for (size_t i = 0; i < round_size; i ++) {
            const auto& x = dot_pairs[i][0];
            const auto& y = dot_pairs[i][1];
            MC->add_rx(x[1]);   MC->add_rx(y[1]);
            rz1 += x[1] * y[1];
        }

        MC->add_rz1(os_ptr[round_size * 2] - rz1);
        MC->add_mac_z(1);
        dot_results.back()[1] = os_ptr[round_size * 2 + 1];
    }
    else {
        for (size_t i = 0; i < round_size; i ++) {
            const auto& x = dot_pairs[i][0];
            const auto& y = dot_pairs[i][1];
            MC->add_ry(x[1]);   MC->add_ry(y[1]);
            rz2 += x[1] * y[1];
        }
        T& res = dot_results.back();
        res[1].randomize(shared_prngs[0]);
        res[0] = rz2;
        res[1].pack(os_prep);
        MC->add_mac_z(1);
        dot_sizes.push_back(round_size);
    }
    MC->add_size(round_size * 2);
    dot_pairs.clear();
}

template <class T>
T EchoPrepProtocol<T>::finalize_dotprod(int length) {
    (void) length;
    return dot_results.next();
    
}

template <class T>
void Echo<T>::init_dotprod() {
    dot_pairs.clear();
    dot_results.clear();
    this->input_pairs.clear();
    os.reset_write_head();

    // Pre-allocate a reasonable initial capacity to reduce reallocations
    // during matrix multiplications with large inner dimensions.
    if (dot_pairs.capacity() < ECHO_DOT_PAIRS_INITIAL_CAPACITY) {
        dot_pairs.reserve(ECHO_DOT_PAIRS_INITIAL_CAPACITY);
    }

    if (this->P.my_num() == 0)  return ;
    if (this->P.my_num() == 2) {
        if (OnlineOptions::singleton.has_option("nofile")) {
            ;
        }
        else {
            this->read(os_prep);
            size_t length = os_prep.get_length() / sizeof(clear);
            clear* os_prep_ptr = reinterpret_cast<clear*>(os_prep.get_data());
            // 预先分配内存，避免循环中多次 realloc
            dot_results.reserve(length);
            for (size_t i = 0; i < length; i ++) {
                dot_results.push_back({});
                dot_results.back()[1] = os_prep_ptr[i];
            }
        }
    }
    // Always initialize dot_results_ptr to 0, including "nofile" mode
    dot_results_ptr = 0;
    if (!OnlineOptions::singleton.has_option("nofile")) {
        this->read(os_prep);
        os_prep_ptr = reinterpret_cast<clear*>(os_prep.get_data());
    }

    if (MC) {
        MC->MaybeCheck(this->P);
    }
}

template <class T>
void Echo<T>::prepare_dotprod(const T& x, const T& y) {
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    Timer _prepare_timer;
    _prepare_timer.start();
#endif
    
    dot_pairs.push_back({x, y});
    
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    _prepare_timer.stop();
    EchoTiming::dotprod_prepare_time += _prepare_timer.elapsed();
#endif
}

template <class T>
void Echo<T>::next_dotprod() {
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    Timer _next_timer;
    _next_timer.start();
#endif
    
    int my_num = this->P.my_num();
    size_t round_size = dot_pairs.size();
    if (my_num != 2) {
        dot_results.push_back({});
        if (my_num == 0) {
            dot_pairs.clear();
#ifdef ENABLE_ECHO_COMPONENT_TIMING
            _next_timer.stop();
            EchoTiming::dotprod_next_time += _next_timer.elapsed();
#endif
            return ;
        }
    }
    else {
        if (OnlineOptions::singleton.has_option("nofile")) {
            // Each next_dotprod() call produces ONE result, so push_back one element
            // (matching party 1's behavior). resize(round_size) was wrong because
            // round_size is the current batch size, not the accumulated result count.
            dot_results.push_back({});
        }
    }

    if (OnlineOptions::singleton.has_option("nofile")) {
        // 使用线程局部静态缓冲区，避免每次调用都重新分配内存
        // 原来的 os_prep.append() 会导致频繁 resize 和 memcpy，引发大量 page fault
        static thread_local std::vector<clear> nofile_buffer;
        size_t needed_size = round_size * 2 + 2;
        if (nofile_buffer.size() < needed_size) {
            nofile_buffer.resize(needed_size);
        }
        os_prep_ptr = nofile_buffer.data();
    }

    mz = 0; mac_z = 0;
    int my_astra_num = my_num - 1;
    for (size_t i = 0; i < round_size; i ++) {
        const auto& x = dot_pairs[i][0];
        const auto& y = dot_pairs[i][1];
        mz += x[0] * (y[0] * my_astra_num + y[1]) + x[1] * y[0];
        mac_z += x[0] * (alpha[my_astra_num] * my_astra_num * y[0] + os_prep_ptr[1]) + y[0] * os_prep_ptr[0];

        os_prep_ptr += 2;
    }

    if (my_num == 1) {
        dot_results.back()[1] = os_prep_ptr[1];
    }
    mz += (os_prep_ptr[0] - dot_results[dot_results_ptr][1]);
    dot_results[dot_results_ptr][0] = mz;

    mz.pack(os);
    MC->add_mac_z(mac_z);

    dot_results_ptr ++;
    os_prep_ptr += (3 - my_num);       // P1 : +2, P2 : +1
    dot_pairs.clear();
    
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    _next_timer.stop();
    EchoTiming::dotprod_next_time += _next_timer.elapsed();
#endif
}

template <class T>
T Echo<T>::finalize_dotprod(int length) {
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    Timer _finalize_timer;
    _finalize_timer.start();
#endif
    
    (void) length;
    auto result = dot_results.next();
    
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    _finalize_timer.stop();
    EchoTiming::dotprod_finalize_time += _finalize_timer.elapsed();
    EchoTiming::dotprod_count++;
#endif
    return result;
}

template <class T>
void Echo<T>::unsplit(StackedVector<T>& dest,
            StackedVector<typename T::bit_type>& source,
            const Instruction& instruction) {
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    Timer _unsplit_timer;
    _unsplit_timer.start();
#endif
    
    // (void) dest; (void) source; (void) instruction; return;
    auto& P = this->P;
    int my_num = P.my_num();
    if (my_num == 0) {
#ifdef ENABLE_ECHO_COMPONENT_TIMING
        _unsplit_timer.stop();
        EchoTiming::unsplit_time += _unsplit_timer.elapsed();
        EchoTiming::unsplit_count += instruction.get_size();
#endif
        return ;
    }
    my_num -= 1;
    if (instruction.get_start().size() == 1) {
        // Note: unsplit1 will handle its own timing
        unsplit1(dest, source, instruction);
#ifdef ENABLE_ECHO_COMPONENT_TIMING
        _unsplit_timer.stop();
        EchoTiming::unsplit_time += _unsplit_timer.elapsed();
        EchoTiming::unsplit_count += instruction.get_size();
#endif
        return;
    }

    CODE_LOCATION
    int n_bits = instruction.get_size();
    int unit = T::bit_type::default_length;
    assert(instruction.get_start().size() == 2);

    octetStream os;
    if (OnlineOptions::singleton.has_option("nofile")) {
        os.append(n_bits * sizeof(T));
    }
    else {
        this->read(os);
    }
    os.require<T>(n_bits);

    for (int i = 0; i < DIV_CEIL(n_bits, unit); i++)
    {
        // conversion symmetric in binary
        auto x = T::bit_type::from_rep3(source.at(instruction.get_r(0) + i));
        int left = min(unit, n_bits - unit * i);
        typename vector<T>::iterator its[2];
        for (int k = 0; k < 2; k++)
            its[k] = dest.iterator_for_size(
                    instruction.get_start()[k] + i * unit, left);
        for (int j = 0; j < left; j++)
        {
            auto& y = *its[0]++;
            os.get_no_check(y);
            y = T::from_rep3(y);
            (*its[1]++).m(my_num) = x.m(my_num).get_bit(j);
        }
    }
    
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    _unsplit_timer.stop();
    EchoTiming::unsplit_time += _unsplit_timer.elapsed();
    EchoTiming::unsplit_count += n_bits;
#endif
}

template<class T>
void Echo<T>::unsplit1(StackedVector<T>& dest,
        StackedVector<typename T::bit_type>& source,
        const Instruction& instruction)
{
    CODE_LOCATION
    size_t n_bits = instruction.get_size();
    assert(instruction.get_start().size() == 1);
    auto& results = this->results;

    this->init_reduced_mul(n_bits);
    auto range = BlockRange(source, instruction.get_r(0), n_bits);

    for (auto& x : range)
    {
        for (typename T::clear bit : BitLeftRange(x.m(-1), x, range))
        {
            T a, b, c;
            T::template pre_reduced_mul<T, -1>(a, b, c, *this, {}, bit);
            results.back() = a + b - 2 * c;
        }
    }

    this->exchange_reduced_mul(n_bits);

    for (auto& x : Range(dest, instruction.get_start()[0], n_bits))
    {
        auto res = T::template post_reduced_mul<T, 1>(*this);
        x = res.first;
        x.m(-1) -= res.second * 2;
    }

    assert(not recv_os.left());
    if (!OnlineOptions::singleton.has_option("nofile")) {
        assert(not os_prep.left());
    }
}

template <class T>
void Echo<T>::init_reduced_mul(size_t n_mul) {
    if (OnlineOptions::singleton.has_option("nofile")) {
        os_prep.append(2 * n_mul * sizeof(clear));
    }
    else {
        this->read(os_prep);
    }
    os_prep.require<clear>(2 * n_mul);

    os.reset_write_head();
    os.reserve<clear>(n_mul);
    auto& results = this->results;
    results.clear();
    results.reserve(n_mul);
}

template <class T>
void Echo<T>::exchange_reduced_mul(size_t n_mul) {
    auto& P = this->P;
    int my_num = P.my_num();
    if (my_num == 0)    return ;
    P.exchange(3 - my_num, os, recv_os);
    recv_os.require<clear>(n_mul);
    this->results.reset();
    // assert(this->results.left() == n_mul);
}

template <class T>
void EchoBin<T>::exchange() {
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    Timer _exchange_timer;
    _exchange_timer.start();
#endif
    
    size_t round_size = mult_shares.size();
    os_prep.clear();
    os_prep.reserve(round_size * 6 * sizeof(clear));
    if (OnlineOptions::singleton.has_option("nofile")) {
        os_prep.append(round_size * 6 * sizeof(clear));
    }
    else {
        this->read(os_prep);
    }

    results.resize(round_size);

    // Cache data pointers and avoid repeated pointer arithmetic
    const T* __restrict prep_data = reinterpret_cast<const T*>(os_prep.get_data());
    clear* __restrict os_data = reinterpret_cast<clear*>(os[0].append(round_size * 2 * sizeof(clear)));
    
    // Pre-cache player number to avoid repeated calls
    const int my_num_cached = P.my_num();
    
    // First loop: optimize memory access pattern and reduce pointer arithmetic
    for (size_t i = 0; i < round_size; ++i) {
        // Cache frequently accessed data to improve locality
        const size_t prep_idx = i * 3;
        const auto& a = prep_data[prep_idx];
        const auto& b = prep_data[prep_idx + 1];
        
        // Cache mult_shares data - access both elements together for better cache utilization  
        const auto& x = mult_shares[i][0];
        const auto& y = mult_shares[i][1];
        
        // Combine operations to reduce temporary calculations
        const size_t os_idx = i * 2;
        os_data[os_idx] = (x - a)[1];
        os_data[os_idx + 1] = (y - b)[1];
    }
    
    P.pass_around(os[0], os[1], 1);
    
    // Reset pointers for second loop with cached data access
    prep_data = reinterpret_cast<const T*>(os_prep.get_data());
    const clear* __restrict recv_data = reinterpret_cast<const clear*>(os[1].get_data());
    
    // Cache MC alpha value to avoid repeated function calls
    const auto cached_alphai = MC->get_alphai();
    
    // Second loop: batch operations and reduce function call overhead
    for (size_t i = 0; i < round_size; ++i) {
        auto& result = results[i];
        
        // Use array indexing instead of pointer arithmetic for better optimization
        const size_t prep_idx = i * 3;
        const auto& a = prep_data[prep_idx];
        const auto& b = prep_data[prep_idx + 1]; 
        const auto& c = prep_data[prep_idx + 2];
        
        // Cache mult_shares access
        const auto& x = mult_shares[i][0];
        const auto& y = mult_shares[i][1];
        
        // Pre-compute difference operations to reduce repeated calculations
        const auto x_minus_a = x - a;
        const auto y_minus_b = y - b;
        
        const size_t recv_idx = i * 2;
        const clear rho = recv_data[recv_idx] + x_minus_a.sum();
        const clear sigma = recv_data[recv_idx + 1] + y_minus_b.sum();
        
        // Combine operations to reduce temporary variables and improve instruction pipeline
        const clear rho_sigma = rho * sigma;
        result += rho * b + sigma * a + T::constant(rho_sigma, my_num_cached, cached_alphai) + c;
    }
    
    results.reset();
    
#ifdef ENABLE_ECHO_COMPONENT_TIMING
    _exchange_timer.stop();
    EchoTiming::binary_exchange_time += _exchange_timer.elapsed();
    EchoTiming::binary_count += round_size;  // Count actual AND gates, not batches
#endif
}

template <class T>
void EchoPrepProtocol<T>::random_inst(StackedVector<T>& S, const Instruction& instruction) {
    octetStream os;

    for (int j = 0; j < instruction.get_size(); j++)
    {
        auto& res = S[instruction.get_r(0) + j];
        for (int i = 0; i < 2; i++)
            res[i].randomize_part(
                    this->prng_protocol.shared_prngs[i],
                    instruction.get_n());
        res.pack(os);
    }

    this->store(os);
}

template <class T>
void Echo<T>::randoms_inst(StackedVector<T>& S, const Instruction& instruction) {
    if (this->P.my_num() == 0)
        return ;
    octetStream os;
    if (OnlineOptions::singleton.has_option("nofile")) {
        os.append(instruction.get_size() * T::size());
    }
    else {
        this->read(os);
    }

    for (int j = 0; j < instruction.get_size(); j++)
    {
        auto& res = S[instruction.get_r(0) + j];
        res.unpack(os);
    }

    assert(not os.left());
}

template<class T>
T EchoPrepProtocol<T>::get_random()
{
    T res;
    for (int i = 0; i < 2; i++)
        res[i].randomize(this->prng_protocol.shared_prngs[i]);
    this->store(res);
    return res;
}

template<class T>
T Echo<T>::get_random()
{
    if (this->P.my_num() == 0)
        return T();
    if (OnlineOptions::singleton.has_option("nofile")) {
        os.append(sizeof(T));
    }
    else {
        this->read(os);
    }
    T res;
    os.get(res);
    return res;
}

template<class T>
void Echo<T>::exchange_input0(size_t n_inputs)
{
    if (OnlineOptions::singleton.has_option("nofile")) {
        this->cs_prep.append(n_inputs * sizeof(open_type));
    }
    else {
        this->read(this->cs_prep);
    }
    this->cs_prep.template require<open_type>(n_inputs);
}

#ifdef ECHO_TRUNC_PR

// Echo truncation protocol implementation
// Based on the algorithm from "Secure Evaluation of Quantized Neural Networks"
// This is a small_gap truncation protocol only (no big_gap support)
// 
// Algorithm: To compute [[x]] >> m in Z2^k:
// 1. Generate k random shared bits [[r_i]] and compute [[r]] = sum([[r_i]] * 2^i)
// 2. Open c = [[x + r]]
// 3. Compute c' = c / 2^m (i.e., c >> m)
// 4. Compute [[b]] = [[r_{k-1}]] xor (c / 2^{k-1})  (MSB correction)
// 5. Output c' - sum_{i=m}^{k-2}{[[r_i]] * 2^{i-m}} + [[b]] * 2^{k-m-1}
//
// This implementation batches all opens into a single round to minimize communication
template<class T>
template<int>
void Echo<T>::trunc_pr(const vector<int>& regs, int size,
        SubProcessor<T>& proc, false_type)
{
    TruncPrTupleList<T> infos(regs, proc.get_S(), size);
    trunc_pr_small_gap(infos, size, proc);
}

template <class T>
template <int>
void Echo<T>::trunc_pr_small_gap(const TruncPrTupleList<T>& infos, int size,
        SubProcessor<T>& proc)
{
    (void) proc;

    CODE_LOCATION

    int my_num = this->P.my_num();
    auto& DataF = *(this->preprocessing);
    
    this->trunc_rounds++;
    
    // P0 does not participate in online computation in Echo
    if (my_num == 0) {
        // for (auto info : infos) {
        //     this->trunc_pr_counter += size;
        //     for (auto& y : info.dest_range) {
        //         y = T();
        //     }
        // }
        return;
    }
    
    // Structure to store pre-computed data for each truncation
    struct TruncData {
        vector<T> r_bits;  // random bits
        int k;             // bit-width
        int m;             // truncation amount
    };
    
    // Phase 1: Prepare all random bits and compute [[x + r]] for batch opening
    vector<T> to_open;
    vector<TruncData> trunc_data;
    
    for (auto info : infos) {
        this->trunc_pr_counter += size;
        int k = info.k;
        int m = info.m;
        
        for (auto& x : info.source_range) {
            TruncData data;
            data.k = k;
            data.m = m;
            data.r_bits.resize(k);
            
            // Get k random bits from preprocessing
            for (int i = 0; i < k; i++) {
                data.r_bits[i] = DataF.get_bit();
            }
            
            // Compute [[r]] = sum([[r_i]] * 2^i)
            T r;
            for (int i = 0; i < k; i++) {
                T scaled_bit;
                scaled_bit[0] = data.r_bits[i][0] << i;
                scaled_bit[1] = data.r_bits[i][1] << i;
                r = r + scaled_bit;
            }
            
            // Prepare [[x + r]] for batch opening
            to_open.push_back(x + r);
            trunc_data.push_back(std::move(data));
        }
    }
    
    // Phase 2: Batch open all values in a single round
    vector<typename T::open_type> opened_values(to_open.size());
    MC->POpen(opened_values, to_open, this->P);
    
    // Phase 3: Compute final results using opened values
    size_t idx = 0;
    for (auto info : infos) {
        for (auto& y : info.dest_range) {
            const auto& data = trunc_data[idx];
            typename T::open_type c = opened_values[idx];
            int k = data.k;
            int m = data.m;
            
            // Compute c' = c >> m
            typename T::open_type c_prime = c >> m;
            
            // Get MSB of c
            bool c_msb = c.get_bit(k - 1);
            
            // Compute [[b]] = [[r_{k-1}]] xor c_{k-1}
            T b;
            if (!c_msb) {
                b = data.r_bits[k - 1];
            } else {
                b[0] = clear(1) - data.r_bits[k - 1][0];
                b[1] = -data.r_bits[k - 1][1];
                if (my_num == 2) {
                    b[0] = -data.r_bits[k - 1][0];
                    b[1] = clear(1) - data.r_bits[k - 1][1];
                }
            }
            
            // Compute sum_{i=m}^{k-2}{[[r_i]] * 2^{i-m}}
            T r_sum;
            for (int i = m; i < k - 1; i++) {
                T scaled_bit;
                scaled_bit[0] = data.r_bits[i][0] << (i - m);
                scaled_bit[1] = data.r_bits[i][1] << (i - m);
                r_sum = r_sum + scaled_bit;
            }
            
            // Compute [[b]] * 2^{k-m-1}
            T b_scaled;
            b_scaled[0] = b[0] << (k - m - 1);
            b_scaled[1] = b[1] << (k - m - 1);
            
            // Output = c' - r_sum + b_scaled
            y = T::constant(c_prime, my_num, {}) - r_sum + b_scaled;
            
            idx++;
        }
    }
}

// Echo prep truncation protocol - generates the preprocessing data for truncation
// In preprocessing phase, we need to ensure bits are generated via buffer_bits()
// and store the truncation correlated randomness
//
// This implementation batches all opens into a single round to minimize communication
template<class T>
template<int>
void EchoPrepProtocol<T>::trunc_pr(const vector<int>& regs, int size,
        SubProcessor<T>& proc, false_type)
{
    CODE_LOCATION
    TruncPrTupleList<T> infos(regs, proc.get_S(), size);
    trunc_pr_small_gap(infos, size, proc);
}

template <class T>
template <int>
void EchoPrepProtocol<T>::trunc_pr_small_gap(const TruncPrTupleList<T>& infos, int size,
        SubProcessor<T>& proc)
{
    (void) proc;

    CODE_LOCATION

    int my_num = this->P.my_num();
    auto& DataF = *(this->preprocessing);
    
    this->trunc_rounds++;
    
    // P0 is the dealer in Echo protocol
    if (my_num == 0) {
        for (auto info : infos) {
            this->trunc_pr_counter += size;
            for (auto& y : info.dest_range) {
                y = T();
            }
        }
        return;
    }
    
    // Structure to store pre-computed data for each truncation
    struct TruncData {
        vector<T> r_bits;  // random bits
        int k;             // bit-width
        int m;             // truncation amount
    };
    
    // Phase 1: Prepare all random bits and compute [[x + r]] for batch opening
    vector<T> to_open;
    vector<TruncData> trunc_data;
    
    for (auto info : infos) {
        this->trunc_pr_counter += size;
        int k = info.k;
        int m = info.m;
        
        for (auto& x : info.source_range) {
            TruncData data;
            data.k = k;
            data.m = m;
            data.r_bits.resize(k);
            
            // Get k random bits from preprocessing
            for (int i = 0; i < k; i++) {
                data.r_bits[i] = DataF.get_bit();
            }
            
            // Compute [[r]] = sum([[r_i]] * 2^i)
            T r;
            for (int i = 0; i < k; i++) {
                T scaled_bit;
                scaled_bit[0] = data.r_bits[i][0] << i;
                scaled_bit[1] = data.r_bits[i][1] << i;
                r = r + scaled_bit;
            }
            
            // Prepare [[x + r]] for batch opening
            to_open.push_back(x + r);
            trunc_data.push_back(std::move(data));
        }
    }
    
    // Phase 2: Batch open all values in a single round
    vector<typename T::open_type> opened_values(to_open.size());
    MC->POpen(opened_values, to_open, this->P);
    
    // Phase 3: Compute final results using opened values
    size_t idx = 0;
    for (auto info : infos) {
        for (auto& y : info.dest_range) {
            const auto& data = trunc_data[idx];
            typename T::open_type c = opened_values[idx];
            int k = data.k;
            int m = data.m;
            
            // Compute c' = c >> m
            typename T::open_type c_prime = c >> m;
            
            // Get MSB of c
            bool c_msb = c.get_bit(k - 1);
            
            // Compute [[b]] = [[r_{k-1}]] xor c_{k-1}
            T b;
            if (!c_msb) {
                b = data.r_bits[k - 1];
            } else {
                b[0] = clear(1) - data.r_bits[k - 1][0];
                b[1] = -data.r_bits[k - 1][1];
                if (my_num == 2) {
                    b[0] = -data.r_bits[k - 1][0];
                    b[1] = clear(1) - data.r_bits[k - 1][1];
                }
            }
            
            // Compute sum_{i=m}^{k-2}{[[r_i]] * 2^{i-m}}
            T r_sum;
            for (int i = m; i < k - 1; i++) {
                T scaled_bit;
                scaled_bit[0] = data.r_bits[i][0] << (i - m);
                scaled_bit[1] = data.r_bits[i][1] << (i - m);
                r_sum = r_sum + scaled_bit;
            }
            
            // Compute [[b]] * 2^{k-m-1}
            T b_scaled;
            b_scaled[0] = b[0] << (k - m - 1);
            b_scaled[1] = b[1] << (k - m - 1);
            
            // Output = c' - r_sum + b_scaled
            y = T::constant(c_prime, my_num, {}) - r_sum + b_scaled;
            
            idx++;
        }
    }
}

#endif

#endif