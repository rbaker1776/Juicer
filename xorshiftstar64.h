#ifndef XORSHIFTSTAR64_H_7FC690B3A69F
#define XORSHIFTSTAR64_H_7FC690B3A69F

#include "juicer.h"


namespace xrs
{
static uint64_t rand64()
{
	static uint64_t seed = 1234568909876543210ull;
	seed ^= seed >> 12;
	seed ^= seed << 25;
	seed ^= seed >> 27;
	return seed * 0x2545f4914f6cdd1dull;
}

template<typename T>
static T rand() { return T(xrs::rand64()); }

template<typename T>
static T sparse_rand() { return T(xrs::rand64() & xrs::rand64() & xrs::rand64()); }

template<typename T>
static T dense_rand() { return T(xrs::rand64() | xrs::rand64()); }
}


#endif // XORSHIFTSTAR64_H_7FC690B3A69F
