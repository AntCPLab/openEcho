#ifndef TOOLS_FASTVECTOR_H_
#define TOOLS_FASTVECTOR_H_

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <type_traits>
#include <sys/mman.h>
#include <unistd.h>

template<typename T>
class FastVector {
    T* buf;
    size_t sz;
    size_t cap;
    const bool use_hugepage;
    bool buf_is_hugepage;  // 跟踪当前 buf 是否实际使用 hugepage

    // 获取系统hugepage大小（一般2MB）
    static size_t get_hugepage_size() {
        static size_t size = 0;
        if (!size) {
            size = ::getpagesize();
            // 通常hugepage更大，可查/proc/meminfo或者尝试mmap失败时给提示
            FILE* f = fopen("/proc/meminfo", "r");
            if (f) {
                char line[256];
                while (fgets(line, sizeof(line), f)) {
                    if (sscanf(line, "Hugepagesize: %zu kB", &size) == 1) {
                        size *= 1024;
                        break;
                    }
                }
                fclose(f);
            }
        }
        return size ? size : 2 * 1024 * 1024; // fallback: 2MB
    }

    const size_t hugepage_size;

    // 返回是否使用了 hugepage
    void* alloc_mem(size_t bytes, bool& is_hugepage) {
        if (!use_hugepage) {
            is_hugepage = false;
            return std::malloc(bytes);
        }

        // 保证大小对齐hugepage
        size_t alloc_size = ((bytes + hugepage_size - 1) / hugepage_size) * hugepage_size;
        void* ptr = ::mmap(NULL, alloc_size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                           -1, 0);
        if (ptr == MAP_FAILED) {
            // hugepage 失败，回退到 malloc，但记录这一点
            is_hugepage = false;
            return std::malloc(bytes);
        }
        is_hugepage = true;
        return ptr;
    }

    void free_mem(void* ptr, size_t bytes, bool is_hugepage) {
        if (!is_hugepage) {
            std::free(ptr);
        } else {
            size_t alloc_size = ((bytes + hugepage_size - 1) / hugepage_size) * hugepage_size;
            if (::munmap(ptr, alloc_size) != 0) {
                perror("[FastVector] munmap failed");
            }
        }
    }

    void realloc(size_t new_cap) {
        size_t bytes = new_cap * sizeof(T);
        bool new_is_hugepage = false;
        T* new_buf = static_cast<T*>(alloc_mem(bytes, new_is_hugepage));
        if (buf) {
            std::memcpy(new_buf, buf, sz * sizeof(T));
            free_mem(buf, cap * sizeof(T), buf_is_hugepage);
        }
        buf = new_buf;
        cap = new_cap;
        buf_is_hugepage = new_is_hugepage;
    }

public:
    // 默认禁用 hugepage，因为:
    // 1. 系统可能没有配置足够的 hugepage
    // 2. 多线程环境下 hugepage 竞争导致严重的 page fault
    // 3. 小规模分配使用 hugepage 反而会浪费内存
    FastVector(bool hugepage = false) : buf(nullptr), sz(0), cap(0), use_hugepage(hugepage), 
        buf_is_hugepage(false), hugepage_size(get_hugepage_size()) {}
    FastVector(size_t initial_capacity, bool hugepage = false) : 
        buf(nullptr), sz(0), cap(0), use_hugepage(hugepage), buf_is_hugepage(false),
        hugepage_size(get_hugepage_size()) {
        if (initial_capacity > 0) {
            realloc(initial_capacity);
        }
    }
    ~FastVector() { if (buf) free_mem(buf, cap * sizeof(T), buf_is_hugepage); }

    FastVector(const FastVector&) = delete;
    FastVector& operator=(const FastVector&) = delete;

    FastVector(FastVector&& other) noexcept
        : buf(other.buf), sz(other.sz), cap(other.cap), use_hugepage(other.use_hugepage), 
        buf_is_hugepage(other.buf_is_hugepage), hugepage_size(get_hugepage_size()) {
        other.buf = nullptr; other.sz = other.cap = 0;
        other.buf_is_hugepage = false;
    }
    FastVector& operator=(FastVector&& other) noexcept {
        if (this != &other) {
            if (buf) free_mem(buf, cap * sizeof(T), buf_is_hugepage);
            buf = other.buf; sz = other.sz; cap = other.cap;
            buf_is_hugepage = other.buf_is_hugepage;
            other.buf = nullptr; other.sz = other.cap = 0;
            other.buf_is_hugepage = false;
        }
        return *this;
    }

    void push_back(const T& val) {
        if (sz == cap) {
            // 使用更激进的增长策略减少重分配次数
            // 但设置上限避免一次性分配过大
            size_t new_cap = cap ? cap * 2 : 16;
            // 限制单次增长不超过 1M 个元素，避免大量 page fault
            if (new_cap - cap > (1 << 20)) {
                new_cap = cap + (1 << 20);
            }
            reserve(new_cap);
        }
        buf[sz++] = val;
    }

    void pop_back() {
        assert(sz > 0);
        --sz;
    }

    void append(const T* src, size_t count) {
        size_t new_sz = sz + count;
        if (new_sz > cap) {
            // 使用类似 push_back 的增长策略，避免每次 append 都 realloc
            size_t new_cap = cap ? cap * 2 : 16;
            while (new_cap < new_sz) {
                new_cap *= 2;
            }
            reserve(new_cap);
        }
        std::memcpy(buf + sz, src, count * sizeof(T));
        sz += count;
    }

    void fill_append(const T& val, size_t count) {
        size_t new_sz = sz + count;
        if (new_sz > cap) {
            size_t new_cap = cap ? cap * 2 : 16;
            while (new_cap < new_sz) {
                new_cap *= 2;
            }
            reserve(new_cap);
        }
        for (size_t i = 0; i < count; ++i) buf[sz + i] = val;
        sz += count;
    }

    void erase(size_t pos) {
        assert(pos < sz);
        std::memmove(buf + pos, buf + pos + 1, (sz - pos - 1) * sizeof(T));
        --sz;
    }

    void remove_front(size_t size) {
        assert(size <= sz);
        std::memmove(buf, buf + size, (sz - size) * sizeof(T));
        sz -= size;
    }

    void insert(size_t pos, const T* first, const T* last) {
        assert(pos <= sz);
        size_t count = last - first;
        size_t new_sz = sz + count;
        if (new_sz > cap) {
            size_t new_cap = cap ? cap * 2 : 16;
            while (new_cap < new_sz) {
                new_cap *= 2;
            }
            reserve(new_cap);
        }
        std::memmove(buf + pos + count, buf + pos, (sz - pos) * sizeof(T));
        std::memcpy(buf + pos, first, count * sizeof(T));
        sz += count;
    }

    void insert_end(const T* first, const T* last) {
        append(first, last - first);
    }

    void reserve(size_t n) {
        if (n > cap) realloc(n);
    }

    void resize(size_t n) {
        reserve(n);
        if (n > sz) std::memset(buf + sz, 0, (n - sz) * sizeof(T));
        sz = n;
    }

    void clear() { sz = 0; }

    T& operator[](size_t i) { return buf[i]; }
    const T& operator[](size_t i) const { return buf[i]; }
    size_t size() const { return sz; }
    size_t capacity() const { return cap; }
    T* data() { return buf; }
    const T* data() const { return buf; }
    T back() const {
        return buf[sz - 1];
    }
    
    bool is_using_hugepage() const { return buf_is_hugepage; }
};



#endif
