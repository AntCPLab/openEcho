#include "Hash.h"
#include "octetStream.h"

void hash_update(Hash *ctx, const void *data, unsigned long len)
{
    ctx->update(data, len);
}

Blake2b::Blake2b()
{
    // deal with alignment issues
    int error = posix_memalign((void**) &state, 64,
            sizeof(crypto_generichash_state));

    if (error)
        throw runtime_error(
                string("failed to allocate hash state: ") + strerror(error));

    reset();
}

Blake2b::~Blake2b()
{
    free(state);
}

void Blake2b::reset()
{
    crypto_generichash_init(state, 0, 0, crypto_generichash_BYTES);
    size = 0;
}

void Blake2b::update(const octetStream& os)
{
    update(os.get_data(), os.get_length());
}

void Blake2b::update(const string& str)
{
    update(str.data(), str.size());
}

void Blake2b::final(octetStream& os)
{
    os.resize_precise(hash_length);
    os.reset_write_head();
    final(os.append(hash_length));
    reset();
}

octetStream Blake2b::final()
{
    octetStream res;
    final(res);
    return res;
}
