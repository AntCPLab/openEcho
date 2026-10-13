/*
 * ComponentTimer.hpp
 * 
 * Thread-safe timing infrastructure for component-by-component performance analysis.
 * Used to compare ABY3 vs Echo protocol performance.
 */

#ifndef UTILS_COMPONENT_TIMER_HPP_
#define UTILS_COMPONENT_TIMER_HPP_

#include <map>
#include <string>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <chrono>
#include <vector>
#include <algorithm>
#include <atomic>

class ComponentTimer {
public:
    // Component names for consistency
    static constexpr const char* FILE_IO = "File I/O";
    static constexpr const char* ARITH_MUL = "Arithmetic Mul";
    static constexpr const char* ARITH_MUL_PREPARE = "Arith Mul Prepare";
    static constexpr const char* ARITH_MUL_EXCHANGE = "Arith Mul Exchange";
    static constexpr const char* ARITH_MUL_FINALIZE = "Arith Mul Finalize";
    static constexpr const char* MATRIX_OPS = "Matrix Ops";
    static constexpr const char* MATRIX_DOTPROD = "Matrix Dotprod";
    static constexpr const char* MATRIX_EXCHANGE = "Matrix Exchange";
    static constexpr const char* BINARY_OPS = "Binary Ops";
    static constexpr const char* BINARY_PREPARE = "Binary Prepare";
    static constexpr const char* BINARY_EXCHANGE = "Binary Exchange";
    static constexpr const char* BINARY_FINALIZE = "Binary Finalize";
    static constexpr const char* NETWORK_COMM = "Network Comm";
    static constexpr const char* NETWORK_SEND = "Network Send";
    static constexpr const char* NETWORK_RECV = "Network Recv";
    static constexpr const char* THREAD_WAIT = "Thread Wait";
    static constexpr const char* THREAD_SYNC = "Thread Sync";
    static constexpr const char* TRUNC_PR = "Truncation";
    static constexpr const char* REVEAL = "Reveal/Open";
    static constexpr const char* PREP_READ = "Prep Read";
    static constexpr const char* PREP_WRITE = "Prep Write";
    static constexpr const char* OTHER = "Other";

private:
    struct ComponentStats {
        double total_time = 0.0;
        int64_t call_count = 0;
        std::chrono::high_resolution_clock::time_point start_time;
        bool is_running = false;
    };

    std::map<std::string, ComponentStats> m_stats;
    std::mutex m_mutex;
    int m_party_num = -1;
    int m_thread_num = -1;
    bool m_enabled = true;

public:
    ComponentTimer() = default;

    void set_party_info(int party_num, int thread_num) {
        m_party_num = party_num;
        m_thread_num = thread_num;
    }

    void set_enabled(bool enabled) {
        m_enabled = enabled;
    }

    void start(const std::string& component) {
        if (!m_enabled) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& stats = m_stats[component];
        if (stats.is_running) {
            // Already running, ignore nested start
            return;
        }
        stats.start_time = std::chrono::high_resolution_clock::now();
        stats.is_running = true;
    }

    void stop(const std::string& component) {
        if (!m_enabled) return;
        
        auto end_time = std::chrono::high_resolution_clock::now();
        
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_stats.find(component);
        if (it == m_stats.end() || !it->second.is_running) {
            // Not started, ignore
            return;
        }
        
        auto& stats = it->second;
        auto duration = std::chrono::duration<double>(end_time - stats.start_time).count();
        stats.total_time += duration;
        stats.call_count++;
        stats.is_running = false;
    }

    void add_time(const std::string& component, double seconds, int count = 1) {
        if (!m_enabled) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& stats = m_stats[component];
        stats.total_time += seconds;
        stats.call_count += count;
    }

    double get_time(const std::string& component) const {
        auto it = m_stats.find(component);
        if (it == m_stats.end()) return 0.0;
        return it->second.total_time;
    }

    int64_t get_count(const std::string& component) const {
        auto it = m_stats.find(component);
        if (it == m_stats.end()) return 0;
        return it->second.call_count;
    }

    double get_total_time() const {
        double total = 0.0;
        for (const auto& pair : m_stats) {
            total += pair.second.total_time;
        }
        return total;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.clear();
    }

    void print_report(std::ostream& os = std::cout) const {
        if (m_stats.empty()) {
            return;
        }

        // Calculate total time
        double total_time = get_total_time();

        // Sort components by time (descending)
        std::vector<std::pair<std::string, ComponentStats>> sorted_stats(
            m_stats.begin(), m_stats.end());
        std::sort(sorted_stats.begin(), sorted_stats.end(),
            [](const auto& a, const auto& b) {
                return a.second.total_time > b.second.total_time;
            });

        os << "\n";
        os << "=== Component Timing Report";
        if (m_party_num >= 0) {
            os << " (Party " << m_party_num;
            if (m_thread_num >= 0) {
                os << ", Thread " << m_thread_num;
            }
            os << ")";
        }
        os << " ===" << std::endl;
        os << std::string(60, '-') << std::endl;
        os << std::left << std::setw(25) << "Component"
           << std::right << std::setw(12) << "Time (s)"
           << std::setw(10) << "Percent"
           << std::setw(12) << "Calls" << std::endl;
        os << std::string(60, '-') << std::endl;

        for (const auto& pair : sorted_stats) {
            double percent = (total_time > 0) ? 
                (pair.second.total_time / total_time * 100.0) : 0.0;
            
            os << std::left << std::setw(25) << pair.first
               << std::right << std::fixed << std::setprecision(4)
               << std::setw(12) << pair.second.total_time
               << std::setw(9) << std::setprecision(1) << percent << "%"
               << std::setw(12) << pair.second.call_count << std::endl;
        }

        os << std::string(60, '-') << std::endl;
        os << std::left << std::setw(25) << "TOTAL"
           << std::right << std::fixed << std::setprecision(4)
           << std::setw(12) << total_time
           << std::setw(9) << "100.0%"
           << std::setw(12) << "" << std::endl;
        os << std::string(60, '=') << std::endl;
        os << std::endl;
    }

    // Generate a CSV-friendly output for easier analysis
    void print_csv(std::ostream& os = std::cout) const {
        os << "party,thread,component,time_seconds,percent,calls" << std::endl;
        
        double total_time = get_total_time();
        
        for (const auto& pair : m_stats) {
            double percent = (total_time > 0) ? 
                (pair.second.total_time / total_time * 100.0) : 0.0;
            
            os << m_party_num << ","
               << m_thread_num << ","
               << pair.first << ","
               << std::fixed << std::setprecision(6) << pair.second.total_time << ","
               << std::setprecision(2) << percent << ","
               << pair.second.call_count << std::endl;
        }
    }

    // Merge stats from another timer (useful for aggregating thread stats)
    void merge(const ComponentTimer& other) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& pair : other.m_stats) {
            auto& stats = m_stats[pair.first];
            stats.total_time += pair.second.total_time;
            stats.call_count += pair.second.call_count;
        }
    }
};

// RAII helper for automatic timing
class ScopedComponentTimer {
    ComponentTimer& m_timer;
    std::string m_component;
    bool m_stopped = false;

public:
    ScopedComponentTimer(ComponentTimer& timer, const std::string& component)
        : m_timer(timer), m_component(component) {
        m_timer.start(m_component);
    }

    ~ScopedComponentTimer() {
        if (!m_stopped) {
            m_timer.stop(m_component);
        }
    }

    void stop() {
        if (!m_stopped) {
            m_timer.stop(m_component);
            m_stopped = true;
        }
    }
};

// Thread-local timer instance
inline ComponentTimer& get_component_timer() {
    static thread_local ComponentTimer timer;
    return timer;
}

// Global timer for aggregated stats across all threads
inline ComponentTimer& get_global_component_timer() {
    static ComponentTimer timer;
    return timer;
}

// Macro helpers for easy instrumentation
#ifdef COMPONENT_TIMING_ENABLED

#define COMPONENT_TIMER_START(component) \
    get_component_timer().start(component)

#define COMPONENT_TIMER_STOP(component) \
    get_component_timer().stop(component)

#define COMPONENT_TIMER_ADD(component, seconds) \
    get_component_timer().add_time(component, seconds)

#define COMPONENT_TIMER_SCOPED(component) \
    ScopedComponentTimer _scoped_timer_##__LINE__(get_component_timer(), component)

#define COMPONENT_TIMER_PRINT() \
    get_component_timer().print_report()

#define COMPONENT_TIMER_SET_PARTY(party, thread) \
    get_component_timer().set_party_info(party, thread)

#else

// No-op versions when timing is disabled
#define COMPONENT_TIMER_START(component) ((void)0)
#define COMPONENT_TIMER_STOP(component) ((void)0)
#define COMPONENT_TIMER_ADD(component, seconds) ((void)0)
#define COMPONENT_TIMER_SCOPED(component) ((void)0)
#define COMPONENT_TIMER_PRINT() ((void)0)
#define COMPONENT_TIMER_SET_PARTY(party, thread) ((void)0)

#endif

#endif /* UTILS_COMPONENT_TIMER_HPP_ */
