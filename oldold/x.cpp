#include <iostream>
#include <cassert>


class PRNG
{
	private: uint64_t s;

	public: uint64_t next()
	{
		s ^= s >> 12;
		s ^= s << 25;
		s ^= s >> 27;
		return s;
	}

	public: PRNG(uint64_t seed): s(seed)
	{ assert(seed != 0); }
};


int main()
{
	PRNG rng(42);
	uint64_t rand = 0;

	for (int i = 0; i < 1e9; ++i)
	{
		rand = rng.next();
	}

	std::cout << rand << std::endl;
	return 0;
}
