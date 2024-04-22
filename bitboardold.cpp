#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <cmath>
#include <numeric>

#include "bitboard.h"
#include "random.h"


uint8_t PopCnt16[1 << 16];
uint8_t SquareDistance[64][64];
uint8_t CenterDistance[64];

Bitboard BetweenBB[64][64];
Bitboard LineBB[64][64];
Bitboard PseudoAttacks[6][64];
Bitboard PawnAttacks[2][64];

Magic RookMagics[64];
Magic BishopMagics[64];

Bitboard RookTable[102400];
Bitboard BishopTable[5248];

Bitboard safe_destination(Square s, int step)
{
	Square to = Square(s + step);
	return is_ok(to) && distance(s, to) <= 2 ? square_to_bb(to) : Bitboard(0);
}


void hunt_magics(PieceType pt, Bitboard table[], Magic magics[]);


void Bitboards::print(Bitboard bb)
{{{
	const std::string new_line = "+---+---+---+---+---+---+---+---+\n";

	std::cout << new_line;
	for (Rank r = RANK_8; r >= RANK_1; --r) 
	{
		for (File f = FILE_A; f <= FILE_H; ++f) 
		{
			const std::string print_sq = bb & make_square(f, r) ? "| X " : "|   ";

			std::cout << print_sq;
		}

		std::cout << "| " + std::to_string(1 + r) + "\n" + new_line;
	}

	std::cout << "  a   b   c   d   e   f   g   h\n";
}}}


void Bitboards::init()
{{{ 
	for (unsigned i = 0; i < (1 << 16); ++i)
		PopCnt16[i] = uint8_t(std::bitset<16>(i).count());

	// square distances
	for (Square s1 = SQ_a1; s1 <= SQ_h8; ++s1) 
	{
		for (Square s2 = SQ_a1; s2 <= SQ_h8; ++s2)
		{
			SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));
		}

		CenterDistance[s1] = std::max(std::abs(file_of(s1) - 3.5), std::abs(rank_of(s1) - 3.5));
	}

	hunt_magics(ROOK, RookTable, RookMagics);
	hunt_magics(BISHOP, BishopTable, BishopMagics);

	for (Square s = SQ_a1; s <= SQ_h8; ++s) 
	{{{ 
		PawnAttacks[WHITE][s] = pawn_attacks<WHITE>(square_to_bb(s));
		PawnAttacks[BLACK][s] = pawn_attacks<BLACK>(square_to_bb(s));

		for (int step : {-9, -8, -7, -1, 1, 7, 8, 9}) 
			PseudoAttacks[KING][s] |= safe_destination(s, step);

		for (int step : {-17, -15, -10, -6, 6, 10, 15, 17})
			PseudoAttacks[KNIGHT][s] |= safe_destination(s, step);

		PseudoAttacks[QUEEN][s] = PseudoAttacks[BISHOP][s] = attacks_bb<BISHOP>(s, 0);
		PseudoAttacks[QUEEN][s] |= PseudoAttacks[ROOK][s] = attacks_bb<ROOK>(s, 0);
		
		for (PieceType pt : {BISHOP, ROOK})
		{{{
			for (Square s2 = SQ_a1; s2 <= SQ_h8; ++s2)
			{
				if (PseudoAttacks[pt][s] & s2)
				{
					LineBB[s][s2] = (attacks_bb(pt, s, 0) & attacks_bb(pt, s2, 0)) | s | s2;
					BetweenBB[s][s2] = attacks_bb(pt, s, square_to_bb(s2)) 
									 & attacks_bb(pt, s2, square_to_bb(s));
					BetweenBB[s][s2] |= s2;
				}
			}
		}}}
	}}}
}}}


Bitboard sliding_attack(PieceType pt, Square sq, Bitboard occupied)
{{{
	Bitboard  attacks 			  = 0;
	Direction RookDirections[4]   = {NORTH, EAST, SOUTH, WEST};
	Direction BishopDirections[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

	for (Direction d : (pt == ROOK ? RookDirections : BishopDirections))
	{
		Square s = sq;
		while (safe_destination(s, d) && !(occupied & s))
		{
			attacks |= (s += d);
		}
	}

	return attacks;
}}}


void hunt_magics(PieceType pt, Bitboard table[], Magic magics[])
{{{ 
	for (Square s = SQ_a1; s <= SQ_h8; ++s)
	{
		Bitboard edges 	   = ((rank_1bb | rank_8bb) & ~rank_bb(s)) | ((a_filebb | h_filebb) & ~file_bb(s));
		Magic&   m         = magics[s];
		m.mask             = sliding_attack(pt, s, 0) & ~edges;
		int     bit_cnt    = popcount(m.mask);
		int     table_size = 1ULL << bit_cnt;
		m.shift 		   = 64 - bit_cnt;
		m.attacks		   = s == SQ_a1 ? table : magics[s - 1].attacks + (1ULL << popcount(magics[s - 1].mask)); 
	
		Bitboard blocker_patterns[4096] = {0};
		Bitboard legal_moves[4096]		= {0};
		int		 epoch[4096]			= {0};
		Bitboard blockers 				=  0;

		const int seeds[8] = {728, 10316, 55013, 32803, 12281, 15100, 16645, 255};
		PRNG rng(seeds[rank_of(s)]);
	
		for (int i = 0; i < table_size; ++i)
		{
			blocker_patterns[i] = blockers;
			legal_moves[i] 	    = sliding_attack(pt, s, blockers);

			blockers = (blockers - m.mask) & m.mask;
		}

		int cnt = 0;
		for (int i = 0; i < table_size;)
		{{{
			for (m.magic = 0ULL; popcount((m.magic * m.mask) >> 56) < 6;)
			{
				m.magic = rng.sparse_rand<uint64_t>();

				for (++cnt, i = 0; i < table_size; ++i)
				{
					int magic_idx = m.index(blocker_patterns[i]);

					if (epoch[magic_idx] < cnt) 
					{
						epoch[magic_idx] 			 = cnt;
						m.attacks[magic_idx] = legal_moves[i];
					}
					else if (m.attacks[magic_idx] != legal_moves[i]) 
					{
						// fatal collision occured, magic number failed
						break;
					}
				}
			}
		}}}
	}
}}}
