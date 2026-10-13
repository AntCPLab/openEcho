#ifndef PROTOCOLS_ECHOMC_HPP_
#define PROTOCOLS_ECHOMC_HPP_

#include "EchoMC.h"

template <class T>
void EchoPrepMC<T>::init_open(const Player& P, int n) {
    this->os.reset_write_head();
    (void) P;   (void) n;
}
template <class T>
void EchoPrepMC<T>::prepare_open(const T& secret, int n_bits) {
    secret.sum().pack(this->os, n_bits);
}

template <class T>
void EchoPrepMC<T>::exchange(const Player& P) {
    if (P.my_num() != 0)    return ;
    echo_protocol->store(this->os);
}

template <class T>
typename T::clear EchoPrepMC<T>::finalize_open() {
    typename T::clear res;
    return res;
}

template <class T>
void EchoMC<T>::init_open(const Player& P, int n) {
    my_num = P.my_num();
    results.clear();
    for (auto& o : os2) {
        o.reset_write_head();
    }
    (void) n;
}

template <class T>
void EchoMC<T>::prepare_open(const T& secret, int n_bits) {
    (void) n_bits;
    results.push_back(secret.sum());
    if (my_num == 0)    return ;
    switch (my_num) {
        case 1: 
            secret[1].pack(os2[0]);
            results.back() = secret[0];
            break;
        case 2:
            secret[0].pack(os2[0]);
            break;
        default:
            throw runtime_error("invalid player number in EchoMC::prepare_open: " + to_string(my_num));
    }
}

template <class T>
void EchoMC<T>::exchange(const Player& P) {
    if (my_num == 0) {
        octetStream prep_os;
        prep_os.reserve(results.size() * T::open_type::size());
        if (OnlineOptions::singleton.has_option("nofile")) {
            prep_os.append(results.size() * T::open_type::size());
        }
        else {
            echo_protocol->read(prep_os);
        }
        memcpy(results.data(), prep_os.get_data(), results.size() * T::open_type::size());
        P.pass_around(prep_os, os2[1], 1);
    }
    else {
        P.pass_around(os2[0], os2[1], 1);
    }
    results.reset();
}

template <class T>
typename T::clear EchoMC<T>::finalize_open() {
    return results.next() + os2[1].get_no_check<typename T::open_type>();
}

template <class T>
void EchoMCBase<T>::OpenCheck(const Player& P)  {
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
                // cout << "Check hash mismatch" << endl;
            }
    }

    // ...
}

template <class T>
void EchoMC<T>::POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) {
    if (values.size() < S.size()) {
        values.resize(S.size());
    }
    this->os.reset_write_head();
    octetStream recv_os;
    recv_os.reserve(S.size() * T::open_type::size());

    int my_num = P.my_num();
    if (my_num == 0) {
        octetStream prep_os;
        if (OnlineOptions::singleton.has_option("nofile")) {
            prep_os.append(S.size() * T::open_type::size());
        }
        else {
            echo_protocol->read(prep_os);
        }
        memcpy(values.data(), prep_os.get_data(), S.size() * T::open_type::size());
        P.pass_around(prep_os, recv_os, 1);
    }
    else if (my_num == 1) {
        for (size_t i = 0; i < S.size(); i ++) {
            values[i] = S[i][0];
            S[i][1].pack(this->os);
        }
        P.pass_around(this->os, recv_os, 1);
    }
    else {
        for (size_t i = 0; i < S.size(); i ++) {
            values[i] = S[i].sum();
            S[i][0].pack(this->os);
        }
        P.pass_around(this->os, recv_os, 1);
    }

    for (size_t i = 0; i < S.size(); i ++) {
        values[i] += recv_os.get_no_check<typename T::open_type>();
    }
}

template <class T>
void EchoPrepMC<T>::Check(const Player& P) {
    if (buffer_size == 0) {
        this->OpenCheck(P);
        return ;
    }

    if (OnlineOptions::singleton.has_option("noccs")) {
        this->OpenCheck(P);
        rx.clear();
        ry.clear();
        rz1.clear();
        rz2.clear();
        vector_sizes.clear();
        buffer_size = 0;
        this->reset();
        return ;
    }

    RunningTimer timer;

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
                rz2.data() + pointer_vals, vector_sizes.data() + pointer_vals, counter, P, this->prng_protocol->shared_prngs, 1, 64 - 12);
        
        pointer_items += now_size;
        pointer_vals += counter;
    }

    if (pointer_vals < vector_sizes.size()) {
        check_ccs(rx.data() + pointer_items, ry.data() + pointer_items, rz1.data() + pointer_vals, 
            rz2.data() + pointer_vals, vector_sizes.data() + pointer_vals, vector_sizes.size() - pointer_vals, P, this->prng_protocol->shared_prngs, 1, 64 - 12);
    }

    rx.clear();
    ry.clear();
    rz1.clear();
    rz2.clear();
    vector_sizes.clear();
    buffer_size = 0;

    timer.stop();
    this->OpenCheck(P);
}

template <class T>
void EchoPrepMC<T>::FinalCheck(const Player& P) {
    if (mac_z == 0) return ;

    cout << "Final Check." << endl;

    typename T::clear w0, w1, w0_prime, w;

    array<octetStream, 2>& send_os = this->send_os;
    auto& alpha = this->alpha;

    int my_num = P.my_num();
    if (my_num == 0) {
        mac_z *= (alpha[0] + alpha[1]);
        w0_prime = mac_z;
        send_os[1].reset_write_head();
        
        w0 = (alpha[0] + alpha[1]) * rss_z;
        w0_prime *= (alpha[0] + alpha[1]);

        (w0 - this->prng_protocol->shared_prngs[0].template get<clear>()).pack(send_os[1]);
        (w0_prime - this->prng_protocol->shared_prngs[0].template get<clear>()).pack(send_os[1]);
        P.send_to(2, send_os[1]);
    }

    else if (my_num == 2) {
        send_os[0].clear();
        P.receive_player(0, send_os[0]);
        echo_protocol->store(send_os[0]);
    }

    mac_z = 0;
    rss_z = 0;
}

template <class T>
void EchoMC<T>::MacCheck(const Player& P) {
    auto& mac_z = this->mac_z;
    size_t size = mac_z.size();
    if (size == 0 || P.my_num() == 0) {
        return ;
    }

    auto& rss_z = this->rss_z;

    clear random_coef;
    for (size_t i = 0; i < size; i ++) {
        random_coef.randomize(global_prng);
        w += random_coef * rss_z[i][0];
        wt_prime += random_coef * mac_z[i];
    }

    rss_z.clear();
    mac_z.clear();
}

template <class T>
void EchoMC<T>::FinalCheck(const Player& P) {
    auto& mac_z = this->mac_z;
    size_t size = mac_z.size();
    if (size == 0) {
        return ;
    }
    if (P.my_num() == 0) {
        mac_z.clear();
        return ;
    }

    cout << "Final Check. Size = " << size << endl;

    clear w0, w1;
    clear o;

    auto& rss_z = this->rss_z;
    auto& alpha = this->alpha;

    int my_num = P.my_num();
    array<octetStream, 2>& send_os = this->send_os;

    if (my_num == 1) {
        clear w01, w0_prime1, wt;
        wt = alpha[0] * w + w01;
        o = wt - wt_prime;
        send_os[0].reset_write_head();

        o.pack(send_os[0]);
        
    }
    else {
        if (OnlineOptions::singleton.has_option("nofile")) {
            send_os[0].append(clear::N_BYTES * 2);
        }
        else {
            echo_protocol->read(send_os[0]);
        }
        clear w02, w0_prime2, wt;
        w02.unpack(send_os[0]);
        w0_prime2.unpack(send_os[0]);

        wt_prime += w0_prime2;
        wt = alpha[1] * w + w02;

        o = wt - wt_prime;
        send_os[0].reset_write_head();

        o.pack(send_os[0]);
    }

    P.exchange(3 - my_num, send_os[0], send_os[1]);
    o += send_os[1].get<clear>();
    if (o != 0) {
        cout << "[Online Check] o = " << o << endl;
    }

    rss_z.clear();
    mac_z.clear();
}

#endif