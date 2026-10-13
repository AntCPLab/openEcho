#include "MyCryptoPlayer.h"
#include "Math/Setup.h"
#include "Tools/Bundle.h"

MyCryptoPlayer::MyCryptoPlayer(const Names& Nms, const string& id_base) : 
    MultiPlayer<ssl_socket*>(Nms, id_base) {
    crypto_player = make_unique<CryptoPlayer>(Nms, id_base);
    assert (crypto_player != nullptr);

    send_hash.resize(num_players());
    recv_hash.resize(num_players());

    unsigned char my_pk[crypto_sign_PUBLICKEYBYTES];
    crypto_sign_keypair(my_pk, sk);
    pks.resize(num_players());

    octetStream send_os;
    vector<octetStream> recv_os(num_players());

    send_os.reset_write_head();
    for (auto& o: recv_os) {
        o.reset_write_head();
    }

    send_os.append(my_pk, crypto_sign_PUBLICKEYBYTES);

    for (int i = 0; i < num_players(); i ++) {
        if (i == my_num()) {
            continue;
        }
        send_to_no_stats(i, send_os);
    }

    for (int i = 0; i < num_players(); i ++) {
        if (i == my_num()) {
            continue;
        }
        receive_player_no_stats(i, recv_os[i]);
    }

    for (int i = 0; i < num_players(); i ++) {
        pks[i] = new unsigned char[crypto_sign_PUBLICKEYBYTES];
    }

    for (int i = 0; i < num_players(); i ++) {
        if (i == my_num()) {
            continue;
        }
        memcpy(pks[i], recv_os[i].get_data(), crypto_sign_PUBLICKEYBYTES);
    }
    memcpy(pks[my_num()], my_pk, crypto_sign_PUBLICKEYBYTES);
}

MyCryptoPlayer::MyCryptoPlayer(const Names& Nms, int id_base) : 
    MyCryptoPlayer(Nms, to_string(id_base))
{
}

void MyCryptoPlayer::send_to_no_stats(int other, const octetStream& o) const {
    send_hash[other].update(o);
    octetStream &non_const = const_cast<octetStream&>(o);
    sign(non_const, other);
    crypto_player->send_to_no_stats(other, o);
}

void MyCryptoPlayer::receive_player_no_stats(int other, octetStream& o) const 
{
    crypto_player->receive_player_no_stats(other, o);
    verify(o, other);
}

size_t MyCryptoPlayer::send_no_stats(int player, const PlayerBuffer& buffer,
        bool block) const {

    return crypto_player->send_no_stats(player, buffer, block);
}


size_t MyCryptoPlayer::recv_no_stats(int player, const PlayerBuffer& buffer,
        bool block) const {

    return crypto_player->recv_no_stats(player, buffer, block);
}

void MyCryptoPlayer::exchange_no_stats(int other, const octetStream& to_send,
        octetStream& to_receive) const {

    send_hash[other].update(to_send);
    octetStream &non_const = const_cast<octetStream&>(to_send);
    sign(non_const, other);

    crypto_player->exchange_no_stats(other, to_send, to_receive);

    recv_hash[other].update(to_receive);
    verify(to_receive, other);
}

void MyCryptoPlayer::pass_around_no_stats(const octetStream& to_send,
        octetStream& to_receive, int offset) const {

    send_hash[get_player(offset)].update(to_send);
    octetStream &non_const = const_cast<octetStream&>(to_send);
    sign(non_const, get_player(offset));

    crypto_player->pass_around_no_stats(to_send, to_receive, offset);

    recv_hash[get_player(-offset)].update(to_receive);
    verify(to_receive, get_player(-offset));
}

void MyCryptoPlayer::send_receive_all_no_stats(const vector<vector<bool>>& channels,
        const vector<octetStream>& to_send,
        vector<octetStream>& to_receive) const {

    crypto_player->send_receive_all_no_stats(channels, to_send, to_receive);
}

void MyCryptoPlayer::partial_broadcast(const vector<bool>& my_senders,
        const vector<bool>& my_receivers,
        vector<octetStream>& os) const {
    
    crypto_player->partial_broadcast(my_senders, my_receivers, os);
}

void MyCryptoPlayer::Broadcast_Receive_no_stats(vector<octetStream>& os) const {
    crypto_player->Broadcast_Receive_no_stats(os);
}

void printHex(const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02X", data[i]);
    }
}

void MyCryptoPlayer::sign(octetStream &os, int to) const {
    if (pks[to] == NULL)  return ;
    unsigned char signed_message[crypto_sign_BYTES];
    unsigned long long signed_message_len;
    crypto_sign_detached(signed_message, &signed_message_len, send_hash[to].final().get_data(), Hash::hash_length, sk);
    os.append(signed_message, signed_message_len);
}

void MyCryptoPlayer::verify(const octetStream &os, int from) const {
    if (pks[from] == NULL)  return ;
    unsigned char *recv_signature = os.get_data() + os.get_length() - crypto_sign_BYTES;
    int vrfy = crypto_sign_verify_detached(recv_signature, recv_hash[from].final().get_data(), Hash::hash_length, pks[from]);
    vrfy_cnt ^= vrfy;
}

void MyCryptoPlayer::sign_and_verify() {
    for (int i = 0; i < num_players(); i++) {
        if (i == my_num()) {
            continue;
        }
        
        unsigned char signed_message[crypto_sign_BYTES];
        unsigned long long signed_message_len;
        crypto_sign_detached(signed_message, &signed_message_len, send_hash[i].final().get_data(), Hash::hash_length, sk);

        octetStream send_os;
        send_os.append(signed_message, signed_message_len);
        send_to_no_stats(i, send_os);
    }

    vector<octetStream> recv_os(num_players());
    
    for (int i = 0; i < num_players(); i++) {
        if (i == my_num()) {
            continue;
        }
        receive_player_no_stats(i, recv_os[i]);
    }

    for (int i = 0; i < num_players(); i++) {
        if (i == my_num()) {
            continue;
        }

        unsigned char* recv_signature = recv_os[i].get_data();

        if (crypto_sign_verify_detached(recv_signature, recv_hash[i].final().get_data(), Hash::hash_length, pks[i]) != 0) {
            cerr << "Failed to verify the signature." << endl;
            // exit(1);
        }
        else {
            cout << "Verified signature from player " << i << endl;
        }
    }

    for (int i = 0; i < num_players(); i++) {
        send_hash[i].reset();
        recv_hash[i].reset();
        // send_backup[i].reset_write_head();
        // recv_backup[i].reset_write_head();
    }
}