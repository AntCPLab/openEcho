/*
 * EchoInput.h
 *
 * Echo protocol input classes
 * 
 * Echo is an offline-online MPC protocol where:
 * - Offline: 3 parties (P0=dealer, P1, P2) - same as Astra
 * - Online: 3 parties (P1, P2 keep original IDs) - differs from Astra
 * 
 * Astra remaps P1,P2 -> P0,P1 in online phase
 * Echo keeps P1,P2 as P1,P2 in online phase
 */

#ifndef PROTOCOLS_ECHOINPUT_H_
#define PROTOCOLS_ECHOINPUT_H_

#include "Processor/Input.h"
#include "Protocols/MAC_Check_Base.h"

template<class T> class EchoOnlineBase;

template<class T>
class EchoInput : public InputBase<T>
{
    using Protocol = typename T::Protocol;
protected:
    typedef typename T::open_type open_type;

    octetStream send_os, recv_os;
    Protocol* protocol;
    IteratorVector<T> my_results;
    CheckVector<typename T::open_type> inputs;

    array<IteratorVector<T>, 3> results;

public:
    EchoInput(SubProcessor<T>& proc, MAC_Check_Base<T>& MC);
    EchoInput(SubProcessor<T>* proc, Player& P);
    EchoInput(MAC_Check_Base<T>& MC, Preprocessing<T>&, Player& P,
            Protocol* protocol = 0);

    void reset(int player);
    void add_mine(const typename T::open_type& input, int n_bits = -1) final;
    void add_other(int player, int n_bits = -1) final;

    void exchange();
    T finalize(int player, int n_bits = -1) final;
    virtual T finalize_offset(int offset);
};

template<class T>
class EchoPrepInput : public InputBase<T>
{
    typedef typename T::open_type open_type;
    using Protocol = typename T::Protocol;

    Protocol* protocol;
    array<IteratorVector<T>, 3> results;
    octetStream prep_os;
    vector<int> n_inputs;

public:
    EchoPrepInput(SubProcessor<T>& proc, MAC_Check_Base<T>& MC);
    EchoPrepInput(SubProcessor<T>* proc, Player& P);
    EchoPrepInput(MAC_Check_Base<T>& MC, Preprocessing<T>&, Player& P,
            Protocol* protocol = 0);

    void reset(int player);
    void add_mine(const typename T::open_type& input, int n_bits = -1) final;
    void add_other(int player, int n_bits = -1) final;

    void exchange();
    T finalize(int player, int n_bits = -1) final;
    T finalize_offset(int player);
};

#endif /* PROTOCOLS_ECHOINPUT_H_ */