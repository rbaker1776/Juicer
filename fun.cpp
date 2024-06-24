#include <iostream>
#include "bitboard.h"
#include "types.h"


int main()
{
	constexpr Direction D = N;
	uint64_t j = 98234579592387459ull;
	std::cout << bb_to_string(j) << std::endl;
	std::cout << bb_to_string(shift<D>(j)) << std::endl;
}
