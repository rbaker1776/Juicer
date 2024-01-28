#include "bitboard.h"


void test_magics() {

	const int test_count = 1 << 20;
	int r_passes = 0, b_passes = 0;
	PRNG rng(42);

	for (int i = 0; i < test_count; ++i)
	{
		Square	 s 		  = (Square)(rng.rand<uint8_t>() % 64);
		Bitboard occupied = rng.sparse_rand<Bitboard>() & ~square_to_bb(s);

		Bitboard r_magic_moves = RookMagics[s].attacks[RookMagics[s].index(occupied)];
		Bitboard r_legal_moves = sliding_attack(ROOK, s, occupied);

		Bitboard b_magic_moves = BishopMagics[s].attacks[BishopMagics[s].index(occupied)];
		Bitboard b_legal_moves = sliding_attack(BISHOP, s, occupied);

		if (r_magic_moves == r_legal_moves) 
		{
			++r_passes;
		}
		else 
		{
			std::cout << "\nOccupied: \n";
			Bitboards::print(occupied);
			std::cout << "\nExpected: \n";
			Bitboards::print(r_legal_moves);
			std::cout << "\nRecieved: \n\n";
			Bitboards::print(r_magic_moves);
		}

		if (b_magic_moves == b_legal_moves) 
		{
			++b_passes;
		}
		else 
		{
			std::cout << "\nOccupied: \n";
			Bitboards::print(occupied);
			std::cout << "\nExpected: \n";
			Bitboards::print(b_legal_moves);
			std::cout << "\nRecieved: \n\n";
			Bitboards::print(b_magic_moves);
		}
	}

	std::cout << "Rook:   " << r_passes << "/" << test_count << std::endl;
	std::cout << "Bishop: " << b_passes << "/" << test_count << std::endl;
}
