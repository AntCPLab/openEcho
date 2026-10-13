#ifndef _SHA1
#define _SHA1

#include <sodium.h>
#include <vector>
using namespace std;

#include "octetStream.h"

#include "blake3.h"

#ifdef USE_XXHASH
#include "xxhash.h"
#endif

class Blake2b
{
	crypto_generichash_state* state;

public:
	static const int hash_length = crypto_generichash_BYTES;

	unsigned size;

	Blake2b();
	~Blake2b();

	void reset();

	void update(const void *dataIn, unsigned long len)
	{
		crypto_generichash_update(state, (unsigned char*)dataIn, len);
		size += len;
	}
	void update(const octetStream& os);
	template<class T>
	void update(const T& x)
	{
	    update(x.get_ptr(), x.size());
	}
	template<class T>
	void update(const vector<T>& v, const vector<int>& bit_lengths)
	{
	    assert(v.size() == bit_lengths.size());
	    octetStream tmp;
	    tmp.reserve(v.size() * sizeof(T));
	    for (size_t i = 0; i < v.size(); i++)
	        v[i].pack(tmp, bit_lengths[i]);
	    tmp.append(0);
	    update(tmp);
	}
	void update(const string& str);

	void final(unsigned char hashout[hash_length])
	{
		final(hashout, hash_length);
	}
	void final(unsigned char* hashout, size_t length)
	{
		crypto_generichash_final(state, hashout, length);
	}
	void final(octetStream& os);
	octetStream final();
};

class Blake3 {
	blake3_hasher state;
public:
	static const int hash_length = BLAKE3_OUT_LEN;
	unsigned size;

	Blake3() {
		blake3_hasher_init(&state);
	}
	~Blake3() {}
	void reset() {
		blake3_hasher_init(&state);
		size = 0;
	}
	inline void update(const void *dataIn, unsigned long len) {
		blake3_hasher_update(&state, dataIn, len);
		size += len;
	}
	inline void update(const octetStream& os) {
		update(os.get_data(), os.get_length());
	}
	template<class T>
	inline void update(const T& x)
	{
	    update(x.get_ptr(), x.size());
	}
	template<class T>
	void update(const vector<T>& v, const vector<int>& bit_lengths)
	{
	    assert(v.size() == bit_lengths.size());
	    octetStream tmp;
	    tmp.reserve(v.size() * sizeof(T));
	    for (size_t i = 0; i < v.size(); i++)
	        v[i].pack(tmp, bit_lengths[i]);
	    tmp.append(0);
	    update(tmp);
	}
	inline void update(const string& str) {
		update(str.data(), str.size());
	}
	inline void final(octetStream& os) {
		auto data_ptr = os.append(hash_length);
		blake3_hasher_finalize(&state, data_ptr, hash_length);
	}
	inline void final(unsigned char hashout[hash_length])
	{
		final(hashout, hash_length);
	}
	inline void final(unsigned char* hashout, size_t length)
	{
		blake3_hasher_finalize(&state, hashout, length);
	}
	inline octetStream final() {
		octetStream os;
		final(os);
		return os;
	}
};

using Hash = Blake3;

void hash_update(Hash *ctx, const void *dataIn, unsigned long len);

#endif
