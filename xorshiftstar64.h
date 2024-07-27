#ifndef XORSHIFTSTAR64_H_7FC690B3A69F
#define XORSHIFTSTAR64_H_7FC690B3A69F

#include "juicer.h"


namespace xrs
{
static inline uint64_t rand64()
{
	static constinit uint64_t seed = 1234568909876543210;
	seed ^= seed >> 12;
	seed ^= seed << 25;
	seed ^= seed >> 27;
	return seed * 0x2545f4914f6cdd1dull;
}

template<typename T>
static inline T rand() requires std::is_convertible_v<uint64_t, T>
{ return static_cast<T>(xrs::rand64()); }

template<typename T>
static inline T sparse_rand() requires std::is_convertible_v<uint64_t, T>
{ return static_cast<T>(xrs::rand64() & xrs::rand64() & xrs::rand64()); }

template<typename T>
static inline T dense_rand() requires std::is_convertible_v<uint64_t, T>
{ return static_cast<T>(xrs::rand64() | xrs::rand64()); }
}


#endif // XORSHIFTSTAR64_H_7FC690B3A69F
