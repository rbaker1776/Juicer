#ifndef RANDOM_H_CD03DDF5000E
#define RANDOM_H_CD03DDF5000E



class PRNG
{
	private: uint64_t s;
	private: static constexpr int64_t mult = 2685821657736338717ll;

	private: uint64_t rand64()
	{
		s ^= s >> 12;
		s ^= s << 25;
		s ^= s >> 27;
		return s * mult;
	}

	public: PRNG(uint64_t seed): s(seed)
	{ assert(seed != 0); }

	public: template<typename T>
	T rand() { return T(rand64()); }

	public: template<typename T>
	T sparse_rand() { return T(rand64() & rand64() & rand64()); }
};



#endif // RANDOM_H_CD03DDF5000E
