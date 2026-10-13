#ifndef PROTOCOLS_ECHO_H_
#define PROTOCOLS_ECHO_H_

#include "Astra.h"
#include "Processor/Data_Files.h"
#include "Processor/OnlineOptions.h"
#include "Processor/BaseMachine.h"
#include "Processor/TruncPrTuple.h"
#include "Tools/files.h"
#include <filesystem>
#include <vector>

// #define ECHO_TRUNC_PR

// Enable component timing for Echo protocol
// Define ENABLE_ECHO_COMPONENT_TIMING to enable timing instrumentation
// #define ENABLE_ECHO_COMPONENT_TIMING

// Initial capacity for dot_pairs vector to reduce reallocations during
// matrix multiplications with large inner dimensions (e.g., 784 for MNIST dense layers).
static constexpr size_t ECHO_DOT_PAIRS_INITIAL_CAPACITY = 1024;

// Forward declaration for Echo timing report (defined in Echo.hpp)
namespace EchoTiming {
    void print_report(int party_num, int thread_num);
}

template <class T> class EchoShare;
template <class T> class EchoPrepShare;

template <class T>
class Echo : public AstraOnlineBase<T> {
    using clear = typename T::clear;
    friend class EchoShare<clear>;

    typedef typename T::open_type open_type;
    typedef open_type value_type;

    typename T::MAC_Check* MC;
    array<clear, 2> alpha;
    array<clear, 2> perm_seed;

    vector<array<T, 2>> dot_pairs;
    IteratorVector<T> dot_results;
    vector<int> dot_sizes;
    clear mz, mac_z;
    int dot_results_ptr;
    clear* os_prep_ptr;

    Preprocessing<T>* preprocessing;

    void init_reduced_mul(size_t n_mul);
    void exchange_reduced_mul(size_t n_mul);
    void unsplit1(StackedVector<T>& dest,
            StackedVector<typename T::bit_type>& source,
            const Instruction& instruction);

    void exchange_input0(size_t n_inputs);

public:

    octetStream os, os_prep, recv_os;

    Echo(Player& P) : AstraOnlineBase<T>(P) {}
    ~Echo() {
        if (MC) {
            MC->FinalCheck(this->P);
        }
        // Print component timing if enabled (only prints once per thread due to flag)
#ifdef ENABLE_ECHO_COMPONENT_TIMING
        EchoTiming::print_report(this->P.my_num(), BaseMachine::thread_num);
#endif
    }

    int my_astra_num() {
        int my_num = this->P.my_num();
        if (my_num == 0)
            return -1;
        else
            return my_num - 1;
    }

    void init_prep() {
        this->prep.open(this->get_filename(true));
    }

    template <class U>
    void sync(vector<U>& values, Player& P) {
        (void) values; (void) P;
    }

    template <class U>
    void forward_sync(vector<U>& values) {
        if (OnlineOptions::singleton.has_option("nofile")) {
            values.assign(SEED_SIZE, IntBase<octet>(42));
        }
        else {
            values = this->template read<vector<U>>();
        }
    }

    void init(Preprocessing<T>& prep, typename T::MAC_Check& MC) {
        this->preprocessing = &prep;
        this->MC = &MC;
        this->MC->init(this->P, *this);
        int my_num = this->P.my_num();
        if (not this->prep.is_open()) {
            this->init_prep();
        }
        if (my_num == 0) return ;

        if (OnlineOptions::singleton.has_option("nofile")) {
            alpha[my_num - 1] = 0;
        }
        else {
            alpha[my_num - 1] = this->template read<clear>();
        }
        this->MC->set_alpha(alpha);
    }

    void init_mul() {
        this->input_pairs.clear();
        this->inputs.clear();
        this->results.clear();
        this->n_mults = 0;

        dot_results.clear();
        if (MC) {
            MC->MaybeCheck(this->P);
        }
    }

    void exchange() {
        if (this->input_pairs.size() != 0) {
            this->exchange_mul();
        }
        else if (dot_results.size() != 0) {
            this->exchange_dot();
        }
    }

    T finalize_mul(int n = -1) {
        (void) n;
        return this->results.next();
    }
    T finalize_mul_fast()
    {
        return finalize_mul();
    }

    void randoms_inst(StackedVector<T>&, const Instruction&);
    T get_random();

    void unsplit(StackedVector<T>& dest,
            StackedVector<typename T::bit_type>& source,
            const Instruction& instruction);

    string get_filename(bool preprocessing, const char* name = "Protocol") {
        const Player& P = this->P;
        auto res = get_prep_sub_dir<T>(P.num_players(), preprocessing) + name
                + this->suffix + "-P" + to_string(P.my_num()) + "-T"
                + to_string(BaseMachine::thread_num);
        if (OnlineOptions::singleton.has_option("verbose_astra"))
            fprintf(stderr, "echo filename %s\n", res.c_str());
        return res;
    }

    void init_dotprod();
    void prepare_dotprod(const T& x, const T& y);
    void next_dotprod();
    T finalize_dotprod(int length);

    void exchange_mul();
    void exchange_dot();

    #ifdef ECHO_TRUNC_PR
    template<int = 0>
    void trunc_pr(const vector<int>& regs, int size, SubProcessor<T>& proc,
            false_type);
    template<int = 0>
    void trunc_pr_small_gap(const TruncPrTupleList<T>& infos, int size,
            SubProcessor<T>& proc);
    #endif
};


template <class T>
class EchoPrepProtocol : public AstraPrepProtocol<T> {
    typedef AstraPrepProtocol<T> super;
    typedef typename T::open_type open_type;
    using clear = typename T::clear;

    octetStream os, os_prep;

    typename T::MAC_Check* MC;
    array<clear, 2> alpha;
    clear key;

    vector<octet> random_buffer;

    clear dot_rxy;
    vector<array<T, 2>> dot_pairs;
    IteratorVector<T> dot_results;
    vector<int> dot_sizes;

    Preprocessing<T>* preprocessing;
    
    size_t bytes_written_size;

public:
    EchoPrepProtocol(Player& P) : super(P) {
        // random_buffer.reserve((1 << 20) * 5 * clear::N_BYTES);
        preprocessing = nullptr;
        bytes_written_size = 0;
    }

    ~EchoPrepProtocol() {
        if (MC) {
            MC->FinalCheck(this->P);
        }
        // 累加到thread-local变量
        extern thread_local size_t g_thread_bytes_written;
        g_thread_bytes_written += bytes_written_size;
    }
    
    size_t get_bytes_written() const { return bytes_written_size; }

    void init_prep() {
        this->prep.open(this->get_filename(true));
        // Set large buffer (64MB) to reduce write system calls
        static constexpr size_t BUFFER_SIZE_IO = 64 * 1024 * 1024; // 64MB
        static thread_local vector<char> buffer(BUFFER_SIZE_IO);
        this->prep.rdbuf()->pubsetbuf(buffer.data(), BUFFER_SIZE_IO);
    }

    template <class U>
    void store(const U& value)
    {
        octetStream os;
        os.store(value);
        store(os);
    }

    template <class U>
    void forward_sync(vector<U>& values)
    {
        store(values);
        if (OnlineOptions::singleton.has_option("nofile")) {
            // Type of values: vector<IntBase<octet>>, length = SEED_SIZE
            values.assign(SEED_SIZE, IntBase<octet>(42));
        }
    }

    template <class U>
    void sync(vector<U>& values, Player& P) {
        (void) values; (void) P;
    }

    void store(const octetStream& os) {
        if (OnlineOptions::singleton.has_option("nofile")) {
            bytes_written_size += os.get_length();
            return ;
        }
        if (not this->prep.is_open())
            init_prep();
        TimeScope ts(this->P.comm_stats["Preprocessing transmission"].add(os));
        this->debug();
        os.output(this->prep);
        // this->prep.flush();
        if (not this->prep.good())
            throw runtime_error("error in preprocessing storing");
    }

    void init(Preprocessing<T>& pre, typename T::MAC_Check& MC) {
        this->preprocessing = &pre;
        this->MC = &MC;
        this->MC->init(this->P, *this);
        alpha = this->MC->get_alpha();
        key = alpha[0] + alpha[1];

        if (not this->prep.is_open()) {
            this->init_prep();
        }

        int my_num = this->P.my_num();
        if (my_num != 0) {
            this->store(alpha[my_num - 1]);
        }
    }

    void init_mul() {
        this->input_pairs.clear();
        this->inputs.clear();
        this->results.clear();
        this->n_mults = 0;
        this->dot_results.clear();

        os_prep.clear();
        os.clear();

        if (MC) {
            MC->MaybeCheck(this->P);
        }
    }
    void exchange() {
        if (this->input_pairs.size() != 0) {
            this->exchange_mul();
        }
        else if (dot_results.size() != 0) {
            this->exchange_dot();
        }
    }

    array<PRNG, 2>& get_prngs() {
        return this->prng_protocol.shared_prngs;
    }

    void random_inst(StackedVector<T>& S, const Instruction& instruction);
    T get_random();

    // void unsplit(StackedVector<T>& dest,
    //         StackedVector<typename T::bit_type>& source,
    //         const Instruction& instruction) {
    //             (void) dest; (void) source; (void) instruction; return;
    //         }

    void init_dotprod();
    void prepare_dotprod(const T& x, const T& y);
    void next_dotprod();
    T finalize_dotprod(int length);

    void exchange_mul();
    void exchange_dot();

    #ifdef ECHO_TRUNC_PR
    template<int = 0>
    void trunc_pr(const vector<int>& regs, int size, SubProcessor<T>& proc,
            false_type);

    template<int = 0>
    void trunc_pr_small_gap(const TruncPrTupleList<T>& infos, int size,
            SubProcessor<T>& proc);
    #endif
};

template <class T>
class EchoPrepBinProtocol : public ReplicatedBase, public ProtocolBase<T> {
    using clear = typename T::clear;
    using This = EchoPrepBinProtocol<T>;

    typedef typename T::open_type open_type;
    typedef open_type value_type;
    
    Preprocessing<T>* prep;
    octetStream os_prep;
    unsigned bit_length = 0;
    size_t round_size;
    
    size_t bytes_written_size;
    
public:
    ReplicatedBase prng_protocol;
    ofstream outputs;
    Player& P;

    EchoPrepBinProtocol(Player& P) :
        ReplicatedBase(P), prep(nullptr), bytes_written_size(0), prng_protocol(P, this->shared_prngs), P(P) {}

    EchoPrepBinProtocol(Player& P, Preprocessing<T>& prep) :
        ReplicatedBase(P), prep(&prep), bytes_written_size(0), prng_protocol(P, this->shared_prngs), P(P) {}
    
    // 移动构造函数
    EchoPrepBinProtocol(EchoPrepBinProtocol&& other) :
        ReplicatedBase(std::move(other)), ProtocolBase<T>(std::move(other)),
        prep(other.prep), os_prep(std::move(other.os_prep)),
        bit_length(other.bit_length), round_size(other.round_size),
        bytes_written_size(other.bytes_written_size),
        prng_protocol(std::move(other.prng_protocol)),
        outputs(std::move(other.outputs)), P(other.P) {}
    
    ~EchoPrepBinProtocol() {
        // 累加到thread-local变量
        extern thread_local size_t g_thread_bytes_written;
        g_thread_bytes_written += bytes_written_size;
    }

    This branch() {
        return This(P, *prep);
    }
    
    size_t get_bytes_written() const { return bytes_written_size; }

    void init(Preprocessing<T>& prep, typename T::MAC_Check& MC) {
        this->prep = &prep;
        (void) MC;
    }

    void init_mul() {
        assert (prep);
        os_prep.clear();
        os_prep.reserve((1 << 20) * sizeof(array<T, 3>));
        bit_length = 0;
        round_size = 0;
    }
    void prepare_mul(const T& x, const T& y, int n = -1) {
        (void) x; (void) y;
        bit_length = max(bit_length, (unsigned) n);
        round_size ++;
        // array<T, 3>* ptr = reinterpret_cast<array<T, 3>*>(os_prep.append(sizeof(array<T, 3>)));
        // ptr[0] = prep->get_triple(n);
    }

    void exchange() {
        array<T, 3>* ptr = reinterpret_cast<array<T, 3>*>(os_prep.append(round_size * sizeof(array<T, 3>)));
        prep->get_triples(ptr, round_size, bit_length);
        store(os_prep);
    }

    void init_prep() {
        filesystem::path path = filesystem::current_path()\
         / "Player-Data" / "echo-bin-prep-data-";
        string filename = path.string() + to_string(P.my_num()) + "-" + to_string(BaseMachine::thread_num);
        outputs.open(filename);
        // Set large buffer (64MB) to reduce write system calls
        static constexpr size_t BUFFER_SIZE_IO = 64 * 1024 * 1024; // 64MB
        static thread_local vector<char> buffer(BUFFER_SIZE_IO);
        outputs.rdbuf()->pubsetbuf(buffer.data(), BUFFER_SIZE_IO);
    }

    void store(const octetStream& os) {
        if (OnlineOptions::singleton.has_option("nofile")) {
            bytes_written_size += os.get_length();
            return ;
        }
        if (not outputs.is_open()) {
            init_prep();
        }
        Timer timer;
        TimeScope ts(timer);
        os.output(outputs);
        // outputs.flush();
        if (not outputs.good()) {
            throw runtime_error("error in preprocessing storing");
        }
        this->P.comm_stats["Preprocessing transmission"].add(os, ts);
    }

    T finalize_mul(int n = -1) {
        (void) n;
        T res;
        return res;
    }

    int get_n_relevant_players() const {
        return 3;
    }

    double randomness_time() {
        return ReplicatedBase::randomness_time();
    }

    // 添加store()模板函数和get_random()函数
    template<class U>
    void store(const U& value) {
        octetStream os;
        os.store(value);
        store(os);
    }

    T get_random() {
        return ReplicatedBase::get_random<typename T::open_type>();
    }

    int rep_index(int i, int my_num) {
        if (my_num == 0)
            return i;
        else
        {
            if (i == 0)
                // m
                return my_num - 1;
            else
                // lambda
                return 2 - my_num;
        }
    }

    template <class U>
    EchoPrepShare<U> from_rep3(const FixedVec<U, 2>& x,
        int my_num) {
        EchoPrepShare<U> res;
        for (int i = 0; i < 2; i++)
            res[i] = x[rep_index(i, my_num)];
        return res;
    }

    template <class U>
    EchoPrepShare<U> from_rep3(const FixedVec<U, 2>& x) {
        return from_rep3<U>(x, this->P.my_num());
    }

    array<PRNG, 2>& get_prngs() {
        return prng_protocol.shared_prngs;
    }
};

template <class T>
class EchoBin : public ProtocolBase<T> {
    using clear = typename T::clear;
    using This = EchoBin<T>;

    typedef typename T::open_type open_type;
    typedef open_type value_type;
    
    octetStream os_prep;
    array<octetStream, 2> os;
    IteratorVector<array<T, 2>> mult_shares;
    IteratorVector<T> results;

    typename T::MAC_Check* MC;
    // FastVector<int> lengths;
public:

    ifstream inputs;
    Player& P;

    EchoBin(Player& P) : P(P) {
        // mult_shares.reserve(1 << 20);
    }

    This branch() {
        This res(P);
        return res;
    }

    void init(Preprocessing<T>& prep, typename T::MAC_Check& MC) {
        (void) prep;
        this->MC = &MC;
    }

    void init_prep() {
        filesystem::path path = filesystem::current_path()\
         / "Player-Data" / "echo-bin-prep-data-";
        string filename = path.string() + to_string(P.my_num()) + "-" + to_string(BaseMachine::thread_num);
        open_with_check(inputs, filename);
    }

    void init_mul() {
        mult_shares.clear();
        // lengths.clear();
        results.clear();

        for (auto& o : os) {
            o.reset_write_head();
        }
    }
    void prepare_mul(const T& x, const T& y, int n = -1) {
        mult_shares.push_back({x, y});
        // lengths.push_back(n);
        (void) n;
    }

    void exchange();

    void read(octetStream& os) {
        if (not inputs.is_open()) {
            init_prep();
        }

        Timer timer;
        TimeScope ts(timer);
        os.input(inputs);
        if (not inputs.good())
            throw runtime_error("error in preprocessing reading");
        
        this->P.comm_stats["Preprocessing transmission"].add(os, ts);
    }

    T finalize_mul(int n = -1) {
        (void) n;
        return results.next();
    }

    int get_n_relevant_players() const {
        return 3;
    }

    // 添加read()和get_random()函数
    template<class U>
    U read() {
        U res;
        octetStream os;
        read(os);
        os.get(res);
        return res;
    }

    T get_random() {
        if (OnlineOptions::singleton.has_option("nofile")) {
            os_prep.append(sizeof(T));
        }
        else {
            read(os_prep);
        }
        T res;
        os_prep.get(res);
        return res;
    }
};


#endif