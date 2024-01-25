#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED


#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <cassert>

#include "types.h"


constexpr Bitboard rank_1bb = 0xFF;
constexpr Bitboard rank_2bb = rank_1bb << 8;
constexpr Bitboard rank_3bb = rank_1bb << 16;
constexpr Bitboard rank_4bb = rank_1bb << 24;
constexpr Bitboard rank_5bb = rank_1bb << 32;
constexpr Bitboard rank_6bb = rank_1bb << 40;
constexpr Bitboard rank_7bb = rank_1bb << 48;
constexpr Bitboard rank_8bb = rank_1bb << 56;

constexpr Bitboard a_filebb = 0x0101010101010101ULL;
constexpr Bitboard b_filebb = a_filebb << 1;
constexpr Bitboard c_filebb = a_filebb << 2;
constexpr Bitboard d_filebb = a_filebb << 3;
constexpr Bitboard e_filebb = a_filebb << 4;
constexpr Bitboard f_filebb = a_filebb << 5;
constexpr Bitboard g_filebb = a_filebb << 6;
constexpr Bitboard h_filebb = a_filebb << 7;

constexpr Bitboard square_to_bb(Square sq) {
	return (1ULL << sq);
}

extern uint8_t PopCnt16[1 << 16];
extern uint8_t SquareDistance[64][64];

extern Bitboard between_bb[64][64];
extern Bitboard line_bb[64][64];
extern Bitboard PseudoAttacks[6][64];
extern Bitboard PawnAttacks[2][64];


// Overload bitwise operators between a Bitboard and a Square
// ** Credit: Stockfish ** 
inline Bitboard operator&(Bitboard bb, Square sq) { return bb & square_to_bb(sq); }
inline Bitboard operator|(Bitboard bb, Square sq) { return bb | square_to_bb(sq); }
inline Bitboard operator^(Bitboard bb, Square sq) { return bb ^ square_to_bb(sq); }
inline Bitboard& operator&=(Bitboard& bb, Square sq) { return bb &= square_to_bb(sq); }
inline Bitboard& operator|=(Bitboard& bb, Square sq) { return bb |= square_to_bb(sq); }
inline Bitboard& operator^=(Bitboard& bb, Square sq) { return bb ^= square_to_bb(sq); }


constexpr Rank rank_of(Square sq) { return Rank(sq >> 3); }

constexpr File file_of(Square sq) { return File(sq & 7); } 

constexpr Bitboard rank_bb(Rank r) { return rank_1bb << (8 * r); }

constexpr Bitboard rank_bb(Square sq) { return rank_bb(rank_of(sq)); }

constexpr Bitboard file_bb(File f) { return a_filebb << f; }

constexpr Bitboard file_bb(Square sq) { return file_bb(file_of(sq)); }

constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f); }


inline int popcount(Bitboard b) {

	#ifndef USE_POPCNT
	union {
		Bitboard bb;
		uint16_t u[4];
	} v = {b};
	return PopCnt16[v.u[0]] +  PopCnt16[v.u[1]] + PopCnt16[v.u[2]] + PopCnt16[v.u[3]]; 

	#elif defined(_MSC_VER)
	return int(_mm_popcnt_u64(b));

	#else
	return __builtin_popcount11(b);

	#endif
}

template<PieceType Pt>
inline Bitboard attacks_bb(Square s, Bitboard occupied) {
	
	assert((pt != PAWN) && is_ok(s));

	switch (pt) {
		case BISHOP:
			return BishopMagics[s].attacks[BishopMagics[s].index(occupied)];
		case ROOK:
			return RookMagics[s].attacks[RookMagics[s].index(occupied)];
		case QUEEN:
			return attacks_bb<ROOK>(s, occupied) | attacks_bb<BISHOP>(s, occupied);
		default:
			return PseudoAttacks[Pt][s];
	}
}

inline Bitboard attacks_bb(PieceType pt, Square s, Bitboard occupied) {
	
	assert((pt != PAWN) && is_ok(s));

	switch (pt) {
		case BISHOP:
			return BishopMagics[s].attacks[BishopMagics[s].index(occupied)];
		case ROOK:
			return RookMagics[s].attacks[RookMagics[s].index(occupied)];
		case QUEEN:
			return attacks_bb<ROOK>(s, occupied) | attacks_bb<BISHOP>(s, occupied);
		default:
			return PseudoAttacks[Pt][s];
	}
}


template<Direction d>
constexpr Bitboard shift(Bitboard bb) {
	return d == NORTH	   ? bb << 8
		 : d == SOUTH	   ? bb >> 8
		 : d == NORTH * 2  ? bb << 16
		 : d == SOUTH * 2  ? bb >> 16
		 : d == EAST	   ? (bb & ~h_filebb) >> 1
		 : d == WEST	   ? (bb & ~a_filebb) << 1
		 : d == NORTH_EAST ? (bb & ~h_filebb) << 9
		 : d == SOUTH_EAST ? (bb & ~h_filebb) >> 7
		 : d == SOUTH_WEST ? (bb & ~a_filebb) >> 9
		 : d == NORTH_WEST ? (bb & ~a_filebb) << 7
		 : 0;
}

template<Color c>
constexpr Bitboard pawn_attacks(Bitboard bb) {
	return c == WHITE ? shift<NORTH_WEST>(bb) | shift<NORTH_EAST>(bb)
					  : shift<SOUTH_WEST>(bb) | shift<SOUTH_EAST>(bb);
}

template<typename T = Square>
inline int distance(Square s1, Square s2);

template<>
inline int distance<File>(Square s1, Square s2) {
	return std::abs(file_of(s1) - file_of(s2));
}

template<>
inline int distance<Rank>(Square s1, Square s2) {
	return std::abs(rank_of(s1) - rank_of(s2));
}

template<>
inline int distance<Square>(Square s1, Square s2) {
	return SquareDistance[s1][s2];
}


namespace Bitboards {
	void init();
	void print(Bitboard bb);
}


struct Magic {
	Bitboard mask;
	Bitboard magic;
	Bitboard* attacks;
	unsigned shift;

	// Compute the attack's index
	unsigned index(Bitboard occupied) const {
		return 0;
	}
};

extern Magic RookMagics[64];
extern Magic BishopMagics[64];

void init_magics(PieceType pt, Bitboard table[], Magic magics[]);


#endif // #ifndef BITBOARD_H_INCLUDED
