#include "fasthash.h"

// Compression function for Merkle-Damgard construction.
// This function is generated using the framework provided.
#define mix(h) {					\
			(h) ^= (h) >> 23;		\
			(h) *= 0x2127599bf4325c37ULL;	\
			(h) ^= (h) >> 47; }

uint64_t fasthash64(const void *buf, uint64_t len, uint64_t seed)
{
	const uint64_t    m = 0x880355f21e6d1965ULL;
	const uint64_t *pos = (const uint64_t *)buf;
	const uint64_t *end = pos + (len / 8);
	const unsigned char *pos2;
	uint64_t h = seed ^ (len * m);
	uint64_t v;

	while (pos != end) {
		v = *pos++;
		mix(v);
		h ^= v;
		h *= m;
	}

	pos2 = (const unsigned char*)pos;
	v = 0;

	switch (len & 7) {
	case 7: v ^= (uint64_t)pos2[6] << 48; // falls through
	case 6: v ^= (uint64_t)pos2[5] << 40; // falls through
	case 5: v ^= (uint64_t)pos2[4] << 32; // falls through
	case 4: v ^= (uint64_t)pos2[3] << 24; // falls through
	case 3: v ^= (uint64_t)pos2[2] << 16; // falls through
	case 2: v ^= (uint64_t)pos2[1] << 8; // falls through
	case 1: v ^= (uint64_t)pos2[0]; // falls through
		mix(v);
		h ^= v;
		h *= m;
	}

	mix(h);
	return h;
}

uint32_t fasthash32(const void *buf, size_t len, uint32_t seed)
{
	// the following trick converts the 64-bit hashcode to Fermat
	// residue, which shall retain information from both the higher
	// and lower parts of hashcode.
	uint64_t h = fasthash64(buf, len, seed);
	return (uint32_t)(h - (h >> 32));
}
