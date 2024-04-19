#include "bitboard.h"
#include "random.h"



uint8_t PopCount16[1 << 16];
uint8_t SquareDistance[64][64];
uint8_t CenterDistance[64];

uint64_t BetweenBB[64][64];
uint64_t LineBB[64][64];
uint64_t PseudoAttacks[6][64];
uint64_t PawnAttacks[2][64];

Magic RookMagics[64];
Magic BishopMagics[64];

uint64_t RookTable[0x19000];
uint64_t BishopTable[0x1480];


uint64_t safe_destination(Square s, int step)
{
	Square to = Square(s + step);
	return is_ok(to) && distance(s, to) <= 2 ? square_to_bb(to) : 0;
}


uint64_t sliding_attack(PieceType p, Square s, uint64_t occupied)
{{{
	uint64_t attacks = 0;
	Direction rook_directions[4] = { NORTH, SOUTH, EAST, WEST };
	Direction bishop_directions[4] = { NORTH_EAST, SOUTH_EAST, NORTH_WEST, SOUTH_WEST };

	for (Direction d: (p == ROOK ? rook_directions : bishop_directions))
	{
		Square sq = s;
		while (safe_destination(sq, d) && !(occupied & sq))
		{
			attacks |= (sq += d);
		}
	}

	return attacks;
}}}


void Bitboard::init()
{{{
	// populate PopCounts
	for (int i = 0; i < (1<<16); ++i)
	{
		PopCount16[i] = std::bitset<16>(i).count();
	}

	// populate SquareDistances and CenterDistances
	for (Square s1 = SQ_a1; s1 <= SQ_h8; ++s1)
	{
		for (Square s2 = SQ_a1; s2 <= SQ_h8; ++s2)
		{
			SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));
		}
		CenterDistance[s1] = std::max(std::abs(file_of(s1) - 3.5), std::abs(rank_of(s1) - 3.5));
	}

	Bitboard::hunt_magics(ROOK, RookTable, RookMagics);
	Bitboard::hunt_magics(BISHOP, BishopTable, BishopMagics);

	for (Square s1 = SQ_a1; s1 <= SQ_h8; ++s1)
	{
		PawnAttacks[WHITE][s1] = pawn_attacks<WHITE>(square_to_bb(s1));
		PawnAttacks[BLACK][s1] = pawn_attacks<BLACK>(square_to_bb(s1));

		for (int step: { -9, -8, -7, -1, 1, 7, 8, 9 })
		{ PseudoAttacks[KING][s1] |= safe_destination(s1, step); }

		for (int step: { -17, -15, -10, -6, 6, 10, 15, 17 })
		{ PseudoAttacks[KNIGHT][s1] |= safe_destination(s1, step); }

		PseudoAttacks[QUEEN][s1] = PseudoAttacks[ROOK][s1] = attacks_bb<ROOK>(s1, 0);
		PseudoAttacks[QUEEN][s1] |= PseudoAttacks[BISHOP][s1] = attacks_bb<BISHOP>(s1, 0);

		for (Square s2 = SQ_a1; s2 <= SQ_h8; ++s2)
		{
			for (PieceType pt: { ROOK, BISHOP })
			{
				if (PseudoAttacks[pt][s1] & s2)
				{
					LineBB[s1][s2] = (attacks_bb(pt, s1, 0) & attacks_bb(pt, s2, 0)) | s1 | s2;
					BetweenBB[s1][s2] = attacks_bb(pt, s1, square_to_bb(s2)) & attacks_bb(pt, s2, square_to_bb(s1));
					BetweenBB[s1][s2] |= s2;
				}
			}
		}
	}
}}}


void Bitboard::hunt_magics(PieceType pt, uint64_t* table, Magic* magics)
{{{ 
	const int seeds[8] = { 728, 10316, 55013, 32803, 12281, 15100, 16645, 255 };

	for (Square s1 = SQ_a1; s1 <= SQ_h8; ++s1)
	{
		const uint64_t edges = ((rank1_bb | rank8_bb) & ~rank_bb(s1)) | ((filea_bb | fileh_bb) & ~file_bb(s1));
		Magic& m = magics[s1];
		m.mask = sliding_attack(pt, s1, 0) & ~edges;
		int bit_cnt = popcount(m.mask);
		int table_size = 1ull << bit_cnt;
		m.shift = 64 - bit_cnt;
		m.attacks = s1 == SQ_a1 ? table : magics[s1-1].attacks + (1ull << popcount(magics[s1-1].mask)); 
	
		uint64_t blocker_patterns[4096] = {0};
		uint64_t legal_moves[4096] = {0};
		int	epoch[4096] = {0};
		uint64_t blockers =  0;

		PRNG rng(seeds[rank_of(s1)]);
		for (int i = 0; i < table_size; ++i)
		{
			blocker_patterns[i] = blockers;
			legal_moves[i] = sliding_attack(pt, s1, blockers);
			blockers = (blockers - m.mask) & m.mask;
		}

		int count = 0;
		for (int i = 0; i < table_size;)
		{
			for (m.magic = 0ull; popcount((m.magic * m.mask) >> 56) < 6;)
			{
				m.magic = rng.sparse_rand<uint64_t>();

				for (++count, i = 0; i < table_size; ++i)
				{
					int magic_idx = m.index(blocker_patterns[i]);

					if (epoch[magic_idx] < count) 
					{
						epoch[magic_idx] = count;
						m.attacks[magic_idx] = legal_moves[i];
					}
					else if (m.attacks[magic_idx] != legal_moves[i]) 
					{ // collision occured, magic number failed
						break;
					}
				}
			}
		}
	}
}}}


std::string Bitboard::to_string(uint64_t bb)
{{{
	const std::string newline = "+---+---+---+---+---+---+---+---+\n";
	std::string s = newline;

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			s += bb & make_square(f, r) ? "| X " : "|   ";
		}
		s += "| " + std::to_string(1 + r) + '\n' + newline;
	}

	s += "  a   b   c   d   e   f   g   h";
	return s;
}}}

