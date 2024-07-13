#include "movement.h"
#include "juicer.h"
#include <iostream>


Magic ROOK_MAGICS[64];
Magic BISHOP_MAGICS[64];

uint64_t ROOK_TABLE[88064];
uint64_t BISHOP_TABLE[0x12c0];

#if (POPCOUNT_METHOD == MANUAL)
//uint8_t POPCOUNT16[65536];
#endif


static uint64_t safe_step(Square s, int step)
{
	Square to = Square(s + step);
	return (::is_ok(to) && SQUARE_DISTANCE[s][to] <= 2) ? square_to_bb(to) : 0;
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


void init_bitboards()
{
	#if (POPCOUNT_METHOD == MANUAL)
	//for (int i = 0; i < 65536; ++i)
	//	POPCOUNT16[i] = std::bitset<16>(i).count();
	#endif

	for (Square s = A1; s <= H8; ++s)
	{
		const uint64_t edges = ((FILEA_BB | FILEH_BB) & ~file_bb(s)) | ((RANK1_BB | RANK8_BB) & ~rank_bb(s));

		Magic& rm = ROOK_MAGICS[s];
		rm.magic = ROOK_MAGIC_NUMBERS[s];
		rm.shift = ROOK_MAGIC_SHIFTS[s];
		rm.mask = sliding_attack(ROOK, s, 0) & ~edges;
		rm.attacks = s == A1 ? ROOK_TABLE : ROOK_MAGICS[s-1].attacks + (1 << (64 - ROOK_MAGICS[s-1].shift));
		uint64_t blockers = 0ull;
		for (int i = 0; i < 1 << popcount(rm.mask); ++i)
		{
			rm.attacks[rm.index(blockers)] = sliding_attack(ROOK, s, blockers);
			blockers = (blockers - rm.mask) & rm.mask;
		}

		Magic& bm = BISHOP_MAGICS[s];
		bm.magic = BISHOP_MAGIC_NUMBERS[s];
		bm.mask = sliding_attack(BISHOP, s, 0) & ~edges;
		bm.shift = BISHOP_MAGIC_SHIFTS[s];
		bm.attacks = s == A1 ? BISHOP_TABLE : BISHOP_MAGICS[s-1].attacks + (1 << (64 - BISHOP_MAGICS[s-1].shift));
		blockers = 0ull;
		for (int i = 0; i < 1 << popcount(bm.mask); ++i)
		{
			bm.attacks[bm.index(blockers)] = sliding_attack(BISHOP, s, blockers);
			blockers = (blockers - bm.mask) & bm.mask;
		}
	}	
}
