#include <iostream>
#include <string>
#include <bitset>

#include "bitboard.h"


uint8_t PopCnt16[1 << 16];
uint8_t SquareDistance[64][64];

Bitboard between_bb[64][64];
Bitboard line_bb[64][64];
Bitboard PseudoAttacks[6][64];
Bitboard PawnAttacks[2][64];

Magic RookMagics[64];
Magic BishopMagics[64];


Bitboard safe_destination(Square s, int step) {
	Square to = Square(s + step);
	return is_ok(to) && distance(s, to) <= 2 ? square_to_bb(to) : Bitboard(0);
}


void Bitboards::print(Bitboard bb) {
	const std::string new_line = "+---+---+---+---+---+---+---+---+\n";

	std::cout << new_line;
	for (Rank r = RANK_8; r >= RANK_1; --r) {
		for (File f = FILE_A; f <= FILE_H; ++f) {
			const std::string print_sq = bb & make_square(f, r) ? "| X " : "|   ";

			std::cout << print_sq;
		}

		std::cout << "| " + std::to_string(1 + r) + "\n" + new_line;
	}

	std::cout << "  a   b   c   d   e   f   g   h\n";
} 


void Bitboards::init() {
	
	for (unsigned i = 0; i < (1 << 16); ++i)
		PopCnt16[i] = uint8_t(std::bitset<16>(i).count());

	// square distances
	for (Square s1 = SQ_a1; s1 <= SQ_h8; ++s1) {
		for (Square s2 = SQ_a1; s2 <= SQ_h8; ++s2) {
			SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));
		}
	}

	for (Square s = SQ_a1; s < SQ_h8; ++s) 
	{
		PawnAttacks[WHITE][s] = pawn_attacks<WHITE>(square_to_bb(s));
		PawnAttacks[BLACK][s] = pawn_attacks<BLACK>(square_to_bb(s));

		for (int step : {-9, -8, -7, -1, 1, 7, 8, 9}) 
			PseudoAttacks[KING][s] |= safe_destination(s, step);

		for (int step : {-17, -15, -10, -6, 6, 10, 15, 17})
			PseudoAttacks[KNIGHT][s] |= safe_destination(s, step);

		PseudoAttacks[BISHOP][s] = PseudoAttacks[QUEEN][s] = attacks_bb;
	}	
}


Bitboard sliding_attack(PieceType pt, Square sq, Bitboard occupied) {
	
	Bitboard  attacks 			  = 0;
	Direction RookDirections[4]   = {NORTH, EAST, SOUTH, WEST};
	Direction BishopDirections[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

	for (Direction d : (pt == ROOK ? RookDirections : BishopDirections))
	{
		Square s = sq;
		while (safe_destination(s, d) && !(occupied & s))
			attacks |= (s += d);
	}

	return attacks;
}


void init_magics(PieceType pt, Bitboard table[], Magic magics[]) {
	
	Bitboard occupancy[4096];
	Bitboard reference[4096];
	Bitboard edges, b;
	int		 epoch[4096] = {};
	int		 count = 0, size = 0;

	for (Square s = SQ_a1; s <= SQ_h8; ++s) 
	{
		edges = ((rank_1bb | rank_8bb) & ~rank_of(s)) | ((a_filebb | h_filebb) & ~file_of(s));
		
		Magic& m = magics[s];
		m.mask	 = sliding_attack(pt, s, 0) & ~edges;
		m.shift	 = 64 - popcount(m.mask);

		m.attacks = s == SQ_a1 ? table : magics[s - 1].attacks + size;

		b = size = 0;
		do
		{
			occupancy[size] = b;
			reference[size] = sliding_attack(pt, s, b);
		} while (b);	
	}
}




