#include <iostream>

#include "bitboard.h"
#include "types.h"


int main(int argc, char* argv[]) {
	Bitboards::init();
	
	Bitboards::print(sliding_attack(QUEEN, SQ_d4, 0));

	return 0;
}
