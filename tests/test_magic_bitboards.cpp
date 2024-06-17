#include <iostream>
#include "../bitboard.h"
#include "../xorshiftstar64.h"
#include "minunit.h"


static uint64_t safe_step(Square s, int step)
{
	Square to = Square(s + step);
	return (is_ok(to) && SQUARE_DISTANCE[s][to] <= 2) ? square_to_bb(to) : 0;
}


static uint64_t sliding_attack(PieceType pt, Square s, uint64_t occupied)
{
	uint64_t attacks = 0;
	Direction rook_directions[4] = { N, S, E, W };
	Direction bishop_directions[4] = { NE, SE, NW, SW };

	for (Direction d: (pt == ROOK ? rook_directions : bishop_directions))
	{
		Square from = s;
		while (safe_step(from, d) && !(occupied & from)) 
		{ 
			attacks |= (from += d); 
		}
	}
	return attacks;
}


namespace MagicBitboards
{
	static void rook_attacks()
	{
		for (int i = 0; i < 100'000; ++i)
		{
			const Square s = Square(xrs::rand<uint8_t>() & 63);
			const uint64_t occupied = xrs::sparse_rand<uint64_t>() & ~square_to_bb(s);
			mu_assert(sliding_attack(ROOK, s, occupied) == ROOK_MAGICS[s].attacks[ROOK_MAGICS[s].index(occupied)]);
		}
		for (int i = 0; i < 100'000; ++i)
		{
			const Square s = Square(xrs::rand<uint8_t>() & 63);
			const uint64_t occupied = xrs::rand<uint64_t>() & ~square_to_bb(s);
			mu_assert(sliding_attack(ROOK, s, occupied) == ROOK_MAGICS[s].attacks[ROOK_MAGICS[s].index(occupied)]);
		}
	}

	static void bishop_attacks()
	{
		for (int i = 0; i < 100'000; ++i)
		{
			const Square s = Square(xrs::rand<uint8_t>() & 63);
			const uint64_t occupied = xrs::sparse_rand<uint64_t>() & ~square_to_bb(s);
			mu_assert(sliding_attack(BISHOP, s, occupied) == BISHOP_MAGICS[s].attacks[BISHOP_MAGICS[s].index(occupied)]);
		}
		for (int i = 0; i < 100'000; ++i)
		{
			const Square s = Square(xrs::rand<uint8_t>() & 63);
			const uint64_t occupied = xrs::rand<uint64_t>() & ~square_to_bb(s);
			mu_assert(sliding_attack(BISHOP, s, occupied) == BISHOP_MAGICS[s].attacks[BISHOP_MAGICS[s].index(occupied)]);
		}
	}

	static void suite()
	{
		mu_run(MagicBitboards::rook_attacks);
		mu_run(MagicBitboards::bishop_attacks);
	}
}
