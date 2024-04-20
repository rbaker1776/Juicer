#include <iostream>
#include <chrono>

#include "bitboard.h"
#include "types.h"
#include "random.h"


int main(int argc, char* argv[]) 
{
	PRNG rng(42);

	auto start = std::chrono::high_resolution_clock::now();

	Bitboards::init();
	Bitboards::print(PseudoAttacks[PAWN][SQ_a3]);

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	std::cout << std::endl << (float)duration.count() / 1000 << " microseconds" << std::endl 
	          << (float)duration.count() / 1000000 << " milliseconds" << std::endl 
			  << (float)duration.count() / 1000000000 << " seconds" << std::endl << std::endl;

	return 0;
}
