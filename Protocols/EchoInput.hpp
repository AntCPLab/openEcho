/*
 * EchoInput.hpp
 *
 * Echo protocol input implementation copied from AstraInput
 * 
 * Key differences from Astra:
 * - Astra: offline P1→player0, P2→player1 | online P0,P1 (ID remapping)
 * - Echo:  offline P1→player1, P2→player2 | online P1,P2 (no ID remapping)
 * 
 * This file contains AstraInput logic copied verbatim for modification
 */

#ifndef PROTOCOLS_ECHOINPUT_HPP_
#define PROTOCOLS_ECHOINPUT_HPP_

#include "EchoInput.h"

// EchoInput implementation (copied from AstraInput)

template<class T>
EchoInput<T>::EchoInput(SubProcessor<T>& proc, MAC_Check_Base<T>&) :
        InputBase<T>(&proc), protocol(&(proc.protocol))
{
}

template<class T>
EchoInput<T>::EchoInput(SubProcessor<T>* proc, Player&) :
        EchoInput<T>(*proc, proc->MC)
{
}

template<class T>
EchoInput<T>::EchoInput(MAC_Check_Base<T>&, Preprocessing<T>&,
        Player&, Protocol* protocol) :
        protocol(protocol)
{
}

template<class T>
void EchoInput<T>::reset(int player)
{
    if (protocol->P.my_num() == player)
    {
        send_os.reset_write_head();
        inputs.clear();
        my_results.clear();
    }
    for (auto& result : results) {
        result.clear();
    }
}

template<class T>
void EchoInput<T>::add_mine(const typename T::open_type& input,
        int)
{
    inputs.push_back(input);
    add_other(protocol->P.my_num());
}

template<class T>
void EchoInput<T>::add_other(int player, int)
{
    results[player].push_back({});
}

template<class T>
void EchoInput<T>::exchange()
{
    CODE_LOCATION
    if (OnlineOptions::singleton.has_option("verbose_astra"))
        fprintf(stderr, "astra input exchange %zu\n",
                this->inputs.size() * T::open_type::size());

    octetStream prep_os;
    if (OnlineOptions::singleton.has_option("nofile")) {
        prep_os.append((results[1].size() + results[2].size()) * T::open_type::size());
        prep_os.append(results[0].size() * T::open_type::size() * 2);
        prep_os.append(inputs.size() * T::open_type::size());
        prep_os.append(results[0].size() * T::open_type::size());
    }
    else {
        protocol->read(prep_os);
    }

    const int my_num = protocol->P.my_num();

    if (my_num == 0) {
        for (int i = 1; i < 3; i ++) {
            int n = results[i].size();
            for (int j = 0; j < n; j ++) {
                T& res = results[i][j];
                res[i - 1] = prep_os.get_no_check<typename T::open_type>();
            }
            results[i].reset();
        }

        int n = results[0].size();
        for (int j = 0; j < n; j ++) {
            T& res = results[0][j];
            res[0] = prep_os.get_no_check<typename T::open_type>();
            res[1] = prep_os.get_no_check<typename T::open_type>();
            send_os.store(inputs[j] - res[0] - res[1]);
        }

        protocol->P.send_all(send_os);
        for (auto& result: results) {
            result.reset();
        }
        return ;
    }

    send_os.reserve(inputs.size() * T::open_type::size());

    if (prep_os.left() < inputs.size() * open_type::size())
        throw runtime_error("insufficient data in input");

    for (size_t i = 0; i < inputs.size(); i++)
    {
        auto gamma = prep_os.get_no_check<typename T::open_type>();
        results[my_num][i][0] = inputs[i] - gamma;
        results[my_num][i][1] = gamma;
        send_os.store_no_resize(results[my_num][i][0]);
    }

    // assert(send_os.left() == my_results.size() * T::open_type::size());

    // protocol.P.pass_around(send_os, recv_os, 1);
    protocol->P.exchange(3 - my_num, send_os, recv_os);

    if (recv_os.left() < results[3 - my_num].size() * T::open_type::size())
        throw runtime_error("insufficient data in Astra input");

    for (auto& res : results[3 - my_num]) {
        res[0] = recv_os.get_no_check<typename T::open_type>();
    }
    
    int n = results[0].size();
    if (n > 0) {
        recv_os.reset_write_head();
        protocol->P.receive_player(0, recv_os);
        for (int j = 0; j < n; j++) {
            T& res = results[0][j];
            res[0] = recv_os.get_no_check<typename T::open_type>();
            res[1] = prep_os.get_no_check<typename T::open_type>();
        }
    }

    for (auto& result: results) {
        result.reset();
    }
}

template<class T>
T EchoInput<T>::finalize(int player, int)
{
    return results[player].next();
}

template<class T>
T EchoInput<T>::finalize_offset(int offset)
{
    return results[protocol->P.get_player(offset)].next();
}

// EchoPrepInput implementation (copied from AstraPrepInput)

template<class T>
EchoPrepInput<T>::EchoPrepInput(SubProcessor<T>& proc, MAC_Check_Base<T>&) :
        InputBase<T>(&proc), protocol(&(proc.protocol))
{
    n_inputs.resize(3);
}

template<class T>
EchoPrepInput<T>::EchoPrepInput(SubProcessor<T>* proc, Player&) :
        EchoPrepInput<T>(*proc, proc->MC)
{
}

template<class T>
EchoPrepInput<T>::EchoPrepInput(MAC_Check_Base<T>&, Preprocessing<T>&,
        Player&, Protocol* protocol) :
        protocol(protocol)
{
    n_inputs.resize(3);
}

template<class T>
void EchoPrepInput<T>::reset(int player)
{
    if (this->is_me(player))
        prep_os.reset_write_head();
    n_inputs.at(player) = 0;
}

template<class T>
void EchoPrepInput<T>::add_mine(const typename T::open_type&,
        int)
{
    add_other(this->P->my_num());
}

template<class T>
void EchoPrepInput<T>::add_other(int player, int)
{
    n_inputs.at(player)++;
}

template<class T>
void EchoPrepInput<T>::exchange()
{
    CODE_LOCATION
    if (OnlineOptions::singleton.has_option("verbose_astra"))
    {
        for (int i = 0; i < 3; i++)
            fprintf(stderr, "astra input from %d exchange %d\n", i, n_inputs.at(i));
    }

    auto& P = *this->P;

    for (auto& x : results)
        x.clear();

    auto& shared_prngs = this->protocol->get_prngs();

    if (P.my_num() == 0) {
        prep_os.reserve((n_inputs.at(0) * 2 + n_inputs.at(1) + n_inputs.at(2)) * open_type::size());
        for (int i = 1; i < 3; i++)
        {
            T res;
            int n = n_inputs.at(i);
            results[i].reserve(n);
            for (int j = 0; j < n; j++)
            {
                res[i - 1].randomize(shared_prngs[i - 1]);
                prep_os.store_no_resize(res[i - 1]);
                results[i].push_back(res);
            }
        }

        int n = n_inputs.at(0);
        results[0].reserve(n);
        for (int j = 0; j < n; j++) {
            T res;
            res[0].randomize(shared_prngs[0]);
            res[1].randomize(shared_prngs[1]);
            prep_os.store_no_resize(res[0]);
            prep_os.store_no_resize(res[1]);
            results[0].push_back(res);
        }
    }
    else
    {
        // other online player
        int other = 3 - P.my_num();
        results[other].resize(n_inputs.at(other));

        // my inputs
        T res;
        int my_num = P.my_num();
        int n = n_inputs.at(my_num);
        prep_os.reserve((n + n_inputs.at(0)) * open_type::size());
        results[my_num].reserve(n);
        for (int j = 0; j < n; j++)
        {
            auto gamma = shared_prngs[2 - my_num].template get<typename T::open_type>();
            prep_os.store_no_resize(gamma);
            res.neg_lambda(-1) = gamma;
            results[my_num].push_back(res);
        }

        n = n_inputs.at(0);
        results[0].reserve(n);
        for (int j = 0; j < n; j++) {
            res[2 - my_num].randomize(shared_prngs[2 - my_num]);
            prep_os.store_no_resize(res[2 - my_num]);
            results[0].push_back(res);
        }
    }

    protocol->store(prep_os);

    for(auto& x : results)
        x.reset();
}

template<class T>
T EchoPrepInput<T>::finalize_offset(int offset)
{
    return results[this->P.get_player(offset)].next();
}

template<class T>
T EchoPrepInput<T>::finalize(int player, int)
{
    return results[player].next();
}

#endif /* PROTOCOLS_ECHOINPUT_HPP_ */ 