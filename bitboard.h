#ifndef BITBOARD_H_F9388DB2C5E9
#define BITBOARD_H_F9388DB2C5E9

#include <cassert>
#include <algorithm>
#include <bitset>
#include <string>
#include <iostream>
#include "types.h"



constexpr uint64_t rank1_bb = 0xff;
constexpr uint64_t rank2_bb = rank1_bb << 8;
constexpr uint64_t rank3_bb = rank1_bb << 16;
constexpr uint64_t rank4_bb = rank1_bb << 24;
constexpr uint64_t rank5_bb = rank1_bb << 32;
constexpr uint64_t rank6_bb = rank1_bb << 40;
constexpr uint64_t rank7_bb = rank1_bb << 48;
constexpr uint64_t rank8_bb = rank1_bb << 56;

constexpr uint64_t filea_bb = 0x0101010101010101ull;
constexpr uint64_t fileb_bb = filea_bb << 1;
constexpr uint64_t filec_bb = filea_bb << 2;
constexpr uint64_t filed_bb = filea_bb << 3;
constexpr uint64_t filee_bb = filea_bb << 4;
constexpr uint64_t filef_bb = filea_bb << 5;
constexpr uint64_t fileg_bb = filea_bb << 6;
constexpr uint64_t fileh_bb = filea_bb << 7;

extern uint8_t PopCount16[1 << 16];
extern uint8_t SquareDistance[64][64];
extern uint8_t CenterDistance[64];

extern uint64_t BetweenBB[64][64];
extern uint64_t LineBB[64][64];
extern uint64_t PseudoAttacks[6][64];
extern uint64_t PawnAttacks[2][64];


typedef struct _Magic
{
	uint64_t mask;
	uint64_t magic;
	uint64_t* attacks;
	uint32_t shift;

	uint32_t index(uint64_t occupied) const
	{
		return (occupied & mask) * magic >> shift;
	}
} Magic;


extern Magic RookMagics[64];
extern Magic BishopMagics[64];

extern uint64_t RookTable[0x19000];
extern uint64_t BishopTable[0x1480];


constexpr uint64_t square_to_bb(Square sq) { return 1ull << sq; }

inline uint64_t  operator& (uint64_t bb, Square s)  { return bb & square_to_bb(s); }
inline uint64_t  operator| (uint64_t bb, Square s)  { return bb | square_to_bb(s); }
inline uint64_t  operator^ (uint64_t bb, Square s)  { return bb ^ square_to_bb(s); }
inline uint64_t& operator&=(uint64_t& bb, Square s) { return bb &= square_to_bb(s); }
inline uint64_t& operator|=(uint64_t& bb, Square s) { return bb |= square_to_bb(s); }
inline uint64_t& operator^=(uint64_t& bb, Square s) { return bb ^= square_to_bb(s); }

constexpr Rank rank_of(Square s) { return Rank(s >> 3); }
constexpr File file_of(Square s) { return File(s & 7); }

constexpr uint64_t rank_bb(Rank r)   { return rank1_bb << (r << 3); }
constexpr uint64_t rank_bb(Square s) { return rank_bb(rank_of(s)); }

constexpr uint64_t file_bb(File f)   { return filea_bb << f; }
constexpr uint64_t file_bb(Square s) { return file_bb(file_of(s)); }

constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f); }


inline int popcount(uint64_t bb)
{
	union
	{
		uint64_t whole;
		uint16_t chunks[4];
	};
	whole = bb;
	return PopCount16[chunks[3]] + PopCount16[chunks[2]] 
		 + PopCount16[chunks[1]] + PopCount16[chunks[0]];
}

template<PieceType P>
inline uint64_t attacks_bb(Square s, uint64_t occupied)
{
	assert(P != PAWN && is_ok(s));

	switch (P)
	{
		case BISHOP:
			return BishopMagics[s].attacks[BishopMagics[s].index(occupied)];
		case ROOK:
			return RookMagics[s].attacks[RookMagics[s].index(occupied)];
		case QUEEN:
			return attacks_bb<ROOK>(s, occupied) | attacks_bb<BISHOP>(s, occupied);
		default:
			return PseudoAttacks[P][s] & ~occupied;
	}
}

inline uint64_t attacks_bb(PieceType p, Square s, uint64_t occupied) 
{
	assert(p != PAWN && is_ok(s));

	switch (p) {
		case BISHOP:
			return BishopMagics[s].attacks[BishopMagics[s].index(occupied)];
		case ROOK:
			return RookMagics[s].attacks[RookMagics[s].index(occupied)];
		case QUEEN:
			return attacks_bb<ROOK>(s, occupied) | attacks_bb<BISHOP>(s, occupied);
		default:
			return PseudoAttacks[p][s] & ~occupied;
	}
}

uint64_t sliding_attack(PieceType p, Square s, uint64_t occupied);


template<Direction D>
constexpr uint64_t shift(uint64_t bb)
{
	switch (D)
	{
		case NORTH: 	 return bb << 8;
		case SOUTH: 	 return bb >> 8;
		case NORTH_2:	 return bb << 16;
		case SOUTH_2:	 return bb >> 16;

		case EAST: 		 return (bb & ~fileh_bb) << 1;
		case NORTH_EAST: return (bb & ~fileh_bb) << 9;
		case SOUTH_EAST: return (bb & ~fileh_bb) >> 7;

		case WEST: 		 return (bb & ~filea_bb) >> 1;
		case NORTH_WEST: return (bb & ~filea_bb) << 7;
		case SOUTH_WEST: return (bb & ~filea_bb) >> 9;
		
		default: return 0;
	}
}

template<Color C>
constexpr uint64_t pawn_attacks(uint64_t bb)
{
	switch (C)
	{
		case WHITE: return shift<NORTH_EAST>(bb) | shift<NORTH_WEST>(bb);
		case BLACK: return shift<SOUTH_EAST>(bb) | shift<SOUTH_WEST>(bb);
		default: return 0;
	}
}


namespace Bitboard
{
	void init();
	std::string to_string(uint64_t bb);
	void hunt_magics(PieceType pt, uint64_t* table, Magic* magics);
}


template<typename T=Square>
inline int distance(Square s1, Square s2);

template<>
inline int distance<File>(Square s1, Square s2) 
{ return std::abs(rank_of(s1) - rank_of(s2)); }

template<>
inline int distance<Rank>(Square s1, Square s2)
{ return std::abs(file_of(s1) - file_of(s2)); }

template<>
inline int distance<Square>(Square s1, Square s2)
{ return SquareDistance[s1][s2]; }



#endif // BITBOARD_H_F9388DB2C5E9
