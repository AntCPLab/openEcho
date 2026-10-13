#ifndef NETWORKING_MYCRYPTOPLAYER_H_
#define NETWORKING_MYCRYPTOPLAYER_H_

#include "ssl_sockets.h"
#include "Networking/Player.h"
#include "Networking/CryptoPlayer.h"

#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>
#include <sodium.h>

class MyCryptoPlayer : public MultiPlayer<ssl_socket*>
{
    unique_ptr<CryptoPlayer> crypto_player;

    mutable vector<Hash> send_hash, recv_hash;

    unsigned char sk[crypto_sign_SECRETKEYBYTES];
    vector<unsigned char*> pks;
    vector<unsigned char*> signs;

    mutable size_t vrfy_cnt;

public:
    /**
     * Start a new set of encrypted connections.
     * @param Nms network setup
     * @param id unique identifier
     */
    MyCryptoPlayer(const Names& Nms, const string& id);
    // legacy interface
    MyCryptoPlayer(const Names& Nms, int id_base = 0);
    ~MyCryptoPlayer() {}

    bool is_encrypted() { return true; }

    void send_to_no_stats(int other, const octetStream& o) const;

    void receive_player_no_stats(int other, octetStream& o) const;

    size_t send_no_stats(int player, const PlayerBuffer& buffer,
            bool block) const;
    size_t recv_no_stats(int player, const PlayerBuffer& buffer,
            bool block) const;

    void exchange_no_stats(int other, const octetStream& to_send,
        octetStream& to_receive) const;

    void pass_around_no_stats(const octetStream& to_send, octetStream& to_receive,
        int offset) const;

    void send_receive_all_no_stats(const vector<vector<bool>>& channels,
            const vector<octetStream>& to_send,
            vector<octetStream>& to_receive) const;

    void partial_broadcast(const vector<bool>& my_senders,
            const vector<bool>& my_receivers, vector<octetStream>& os) const;

    void Broadcast_Receive_no_stats(vector<octetStream>& os) const;

    void sign_and_verify();
    void sign(octetStream &os, int to) const;
    void verify(const octetStream &os, int from) const;
};

#endif