#ifndef PROTOCOLS_ECHOMC_H_
#define PROTOCOLS_ECHOMC_H_

#include "Processor/OnlineOptions.h"
#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/MaliciousRep3Share.h"
#include "Tools/my-utils.hpp"
#include "Tools/CCS24Check.hpp"
#include "AstraMC.h"

#include "Math/ShortInteger.h"

#include <vector>

#include <algorithm>

template <class T> class Echo;
template <class T> class EchoPrepProtocol;

template <class T>
class EchoMCBase : public MaliciousRepMC<T> {

    using clear = typename T::clear;

protected:
    Hash hash;
    octetStream os;
    bool needs_checking;

    array<octetStream, 2> send_os;

    void update() {
        hash.update(os);
        needs_checking = true;
    }
    void finalize(const vector<typename T::open_type>& values) {
        os.reset_write_head();
        os.reserve(values.size() * T::open_type::size());
        for (auto& value : values)
            value.pack(os);
        update();
    }

    array<clear, 2> alpha;
    array<clear, 2> seed;

    unique_ptr<ReplicatedBase> prng_protocol = nullptr;
    int my_num;

public:

    void reset() {
        hash.reset();
        needs_checking = false;
    }

    EchoMCBase(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) : EchoMCBase()
    { (void)_; (void)__; (void)___; }
    
    EchoMCBase(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0) : EchoMCBase()
    { (void)_; (void)__; (void)___; (void)____; }

    EchoMCBase() {
        reset();
    }
    ~EchoMCBase() {
        if (needs_checking)
        {
            cerr << endl << "SECURITY BUG: insufficient checking" << endl;
            terminate();
        }
    }

    void set_alpha(array<clear, 2> _alpha) {
        alpha = _alpha;
    }

    void set_seed(const array<clear, 2>& _seed) {
        seed = _seed;
    }

    array<clear, 2> get_alpha() const {
        return alpha;
    }

    void POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        ReplicatedMC<T>::POpen(values, S, P);
        finalize(values);
    }
    void POpen_End(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        ReplicatedMC<T>::POpen_End(values, S, P);
        finalize(values);
    }

    virtual typename T::open_type finalize_raw() {
        auto res = ReplicatedMC<T>::finalize_raw();
        os.reset_write_head();
        res.pack(os);
        update();
        return res;
    }

    void CheckFor(const typename T::open_type& value, const vector<T>& check_shares, const Player& P) {
        os.reset_write_head();
        for (auto& share : check_shares)
        {
            typename T::value_type shares[3];
            for (int i = 0; i < 3; i++)
            {
                int j = (i + P.my_num()) % 3;
                if (j < 2)
                    shares[i] = share[j];
                else
                    shares[i] = value - share.sum();
            }
            for (auto& x : shares)
                x.pack(os);
        }
        update();
        Check(P);
    }

    void Check(const Player& P) {
        OpenCheck(P);
    }
    void OpenCheck(const Player& P);

    EchoMCBase& get_part_MC()
    {
        return *this;
    }
};

template <class T>
class EchoPrepMC : public EchoMCBase<T> {

    using clear = typename T::clear;
    using super = EchoMCBase<T>;
    using Protocol = typename T::Protocol;

    vector<clear> rx, ry, rz1, rz2;
    vector<size_t> vector_sizes;
    size_t buffer_size, mac_buffer_size;

    clear seed_val;
    SeededPRNG rss_prng, mac_prng;

    clear mac_z, random_coef, rss_z;

    EchoPrepProtocol<T>* echo_protocol;
public:

    // Initial capacity for vectors to reduce reallocations during MAC checking
    static constexpr size_t ECHO_MC_INITIAL_CAPACITY = 1 << 16;  // 64K elements

    EchoPrepMC() : super() {
        // Pre-allocate vectors to reduce page faults during Check operations
        rx.reserve(ECHO_MC_INITIAL_CAPACITY);
        ry.reserve(ECHO_MC_INITIAL_CAPACITY);
        rz1.reserve(ECHO_MC_INITIAL_CAPACITY);
        rz2.reserve(ECHO_MC_INITIAL_CAPACITY);
        vector_sizes.reserve(ECHO_MC_INITIAL_CAPACITY);
        buffer_size = 0;
        mac_buffer_size = 0;
        this->reset();
    }

    EchoPrepMC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) : EchoPrepMC()
    { (void)_; (void)__; (void)___; }
    
    EchoPrepMC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0) : EchoPrepMC()
    { (void)_; (void)__; (void)___; (void)____; }

    ~EchoPrepMC() {
        if (this->prng_protocol != nullptr) {
            FinalCheck(this->prng_protocol->P);
        }
        if (this->needs_checking)
        {
            cerr << endl << "SECURITY BUG: insufficient checking" << endl;
            terminate();
        }
    }

    void init(Player& P, EchoPrepProtocol<T>& echo_protocol) {
        this->prng_protocol = make_unique<ReplicatedBase>(P);
        int my_num = P.my_num();

        if (my_num == 0) {
            this->alpha[0].randomize(this->prng_protocol->shared_prngs[0]);
            this->alpha[1].randomize(this->prng_protocol->shared_prngs[1]);
        }
        else {
            this->alpha[my_num - 1].randomize(this->prng_protocol->shared_prngs[2 - my_num]);
        }

        this->echo_protocol = &echo_protocol;
        ReSeed(P);
    }

    void ReSeed(const Player& P) {
        octetStream commit_os, prep_os;
        int my_num = P.my_num();
        if (my_num == 0) {
            Hash hash;
            this->seed[0].randomize(this->prng_protocol->shared_prngs[0]);
            this->seed[1].randomize(this->prng_protocol->shared_prngs[1]);
            seed_val = this->seed[0] + this->seed[1];
            rss_prng.SetSeed((octet*) seed_val.get_ptr());
            mac_prng.SetSeed((octet*) seed_val.get_ptr());

            hash.update(seed_val);
            octet* ptr = commit_os.append(Hash::hash_length);
            hash.final(ptr);

            P.send_to(1, commit_os);
            P.send_to(2, commit_os);
        }
        else {
            this->seed[my_num - 1].randomize(this->prng_protocol->shared_prngs[2 - my_num]);
            P.receive_player(0, commit_os);
            prep_os.store_bytes(commit_os.get_data(), Hash::hash_length);
            prep_os.store(this->seed[my_num - 1]);
            this->echo_protocol->store(prep_os);
        }
        mac_buffer_size = 0;
    }

    EchoPrepMC& get_part_MC() {
        return *this;
    }

    inline void add_rx(const clear& value) {
        rx.push_back(value);
    }

    inline void add_ry(const clear& value) {
        ry.push_back(value);
    }

    inline void add_rz1(const clear& value) {
        rz1.push_back(value);
    }

    inline void add_rz2(const clear& value) {
        rz2.push_back(value);
    }

    inline void add_size(size_t size) {
        vector_sizes.push_back(size);
        buffer_size += size;
    }

    inline void fill_size(size_t n_mults) {
        vector_sizes.insert(vector_sizes.end(), n_mults, 2);
        buffer_size += n_mults * 2;
    }

    inline void add_rss_z(const T& value) {
        random_coef = rss_prng.template get<clear>();
        rss_z += value.sum() * random_coef;
    }

    inline void add_mac_z(const clear& value) {
        random_coef = mac_prng.template get<clear>();
        mac_z += value * random_coef;
        mac_buffer_size ++;
    }

    void Check(const Player& P);
    void MaybeCheck(const Player& P) {
        if (buffer_size >= (size_t) OnlineOptions::singleton.batch_size) {
            Check(P);
        }
        if (mac_buffer_size >= (size_t) OnlineOptions::singleton.batch_size) {
            ReSeed(P);
        }
    }
    void FinalCheck(const Player& P);

    void POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        values.resize(S.size());
        if (P.my_num() != 0)    return ;
        this->os.reserve(S.size() * T::open_type::size());
        clear* os_ptr = reinterpret_cast<clear*>(this->os.get_data());
        for (size_t i = 0; i < S.size(); i++) {
            os_ptr[i] = S[i].sum();
        }
        echo_protocol->store(this->os);
    }

    void init_open(const Player& P, int n = 0);
    void prepare_open(const T& secret, int n_bits = -1);
    void exchange(const Player& P);
    typename T::clear finalize_open();
};

template <class T>
class EchoMC : public EchoMCBase<T> {

    using clear = typename T::clear;
    using super = EchoMCBase<T>;

    octet seed_commit[Hash::hash_length];

    vector<clear> mac_z;
    vector<T> rss_z;

    array<octetStream, 2> os2;
    IteratorVector<clear> results;

    clear w, wt_prime;
    PRNG global_prng;

    int my_num;

    Echo<T>* echo_protocol;

public:
    void init(Player& P, Echo<T>& echo_protocol) {
        this->prng_protocol = make_unique<ReplicatedBase>(P);
        this->echo_protocol = &echo_protocol;

        ReSeed(P);
    }

    EchoMC() : super() {
        this->reset();
        // Pre-allocate vectors to reduce page faults during MAC checking
        mac_z.reserve(EchoPrepMC<T>::ECHO_MC_INITIAL_CAPACITY);
        rss_z.reserve(EchoPrepMC<T>::ECHO_MC_INITIAL_CAPACITY);
        w = 0;
        wt_prime = 0;
    }

    EchoMC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) : EchoMC()
    { (void)_; (void)__; (void)___; }
    
    EchoMC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0) : EchoMC()
    { (void)_; (void)__; (void)___; (void)____; }

    ~EchoMC() {
        if (this->prng_protocol != nullptr) {
            FinalCheck(this->prng_protocol->P);
        }
            
        if (this->needs_checking)
        {
            cerr << endl << "SECURITY BUG: insufficient checking" << endl;
            terminate();
        }
    }

    EchoMC& get_part_MC()
    {
        return *this;
    }

    // 预分配 mac_z 和 rss_z 的空间，避免多次 realloc
    inline void reserve_for_mul(size_t n_mults) {
        mac_z.reserve(mac_z.size() + n_mults);
        rss_z.reserve(rss_z.size() + n_mults);
    }

    inline void add_mac_z(const clear& value) {
        mac_z.push_back(value);
    }

    inline void add_rss_z(const T& value) {
        rss_z.push_back(value);
    }

    inline void add_rss_z(const vector<T>& values) {
        this->rss_z.insert(this->rss_z.end(), values.begin(), values.end());
    }

    void set_commit(octet* seed_commit) {
        memcpy(this->seed_commit, seed_commit, Hash::hash_length);
    }

    void ReSeed(const Player& P) {
        int my_num = P.my_num();
        int my_astra_num = my_num - 1;
        if (my_num == 0)    return ;

        octetStream os_prep;
        if (OnlineOptions::singleton.has_option("nofile")) {
            // Store length prefix for get_bytes (4 bytes)
            os_prep.store_int(Hash::hash_length, 4);
            // Append random data
            os_prep.append(Hash::hash_length + clear::N_BYTES);
            os_prep.reset_read_head();
        }
        else {
            echo_protocol->read(os_prep);
        }
        os_prep.get_bytes(this->seed_commit, Hash::hash_length);
        this->seed[my_astra_num] = os_prep.get_no_check<clear>();
        auto& send_os = this->send_os;
        send_os[0].reset_write_head();

        (this->seed[my_astra_num]).pack(send_os[0]);
        P.exchange(3 - my_num, send_os[0], send_os[1]);
        (this->seed[1 - my_astra_num]).unpack(send_os[1]);

        clear seed_val = this->seed[0] + this->seed[1];
        octet seed_bytes[16];
        memcpy(seed_bytes, seed_val.get_ptr(), 16);
        global_prng.SetSeed(seed_bytes);

        Hash hash;
        hash.update(seed_val);
        octetStream commit_os;
        hash.final(commit_os);

        if (memcmp(commit_os.get_data(), seed_commit, Hash::hash_length) != 0) {
            if (OnlineOptions::singleton.has_option("nofile")) {
                ;
            }
            else {
                throw runtime_error("seed commit mismatch in EchoMC::ReSeed");
            }
        }
    }

    void POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P);

    void FinalCheck(const Player& P);
    void MacCheck(const Player& P);

    void MaybeCheck(const Player& P) {
        if (mac_z.size() >= (size_t) OnlineOptions::singleton.batch_size) {
            MacCheck(P);
            ReSeed(P);
        }
    }

    void init_open(const Player& P, int n = 0);
    void prepare_open(const T& secret, int n_bits = -1);
    void exchange(const Player& P);
    typename T::clear finalize_open();
};

template <class T>
class EchoBinMC : public MaliciousRepMC<T> {
    unique_ptr<HashMaliciousRepMC<T>> inner_mc;

public:
    EchoBinMC(typename T::mac_key_type = {}) {
        inner_mc = make_unique<HashMaliciousRepMC<T>>();
        inner_mc->reset();
    }

    EchoBinMC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) : EchoBinMC()
    { (void)_; (void)__; (void)___; }
    
    EchoBinMC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0) : EchoBinMC()
    { (void)_; (void)__; (void)___; (void)____; }

    void POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        vector<T> shares(S);
        if (P.my_num() == 2) {
            for (auto& share : shares) {
                swap(share[0], share[1]);            
            }
        }
        inner_mc->POpen(values, shares, P);
    }

    void init_open(const Player& P, int n = 0) {
        inner_mc->init_open(P, n);
    }

    void prepare_open(const T& secret, int n_bits = -1) {
        inner_mc->prepare_open(secret, n_bits);
    }

    void exchange(const Player& P) {
        // if (P.my_num() == 2) {
        //     for (auto& share : this->secrets) {
        //         swap(share[0], share[1]);
        //     }
        // }
        inner_mc->exchange(P);
    }

    typename T::open_type finalize_raw() {
        return inner_mc->finalize_raw();
    }

    void Check(const Player& P) {
        inner_mc->Check(P);
    }

    EchoBinMC& get_part_MC()
    {
        return *this;
    }

};

template <class T>
class EchoPrepBinMC : public MaliciousRepMC<T> {
public:
    void POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
        (void) P;
        values.resize(S.size());
    }

    void init_open(const Player& P, int n = 0) {
        (void) P; (void) n;
    }

    void prepare_open(const T& secret, int n_bits = -1) {
        (void) secret; (void) n_bits;
    }

    void exchange(const Player& P) {
        (void) P;
    }

    typename T::open_type finalize_raw() {
        typename T::open_type res;  return res;
    }

    void Check(const Player& P) {
        (void) P;
    }

    EchoPrepBinMC& get_part_MC()
    {
        return *this;
    }
};

#endif