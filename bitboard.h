#ifndef BITBOARD_H_DE3A36A4E0B0
#define BITBOARD_H_DE3A36A4E0B0

#include <string>
#include <cassert>
#include "types.h"
#include "lookup_tables.h"
#include "movegen.h"


constexpr uint64_t RANK1_BB = 0xff;
constexpr uint64_t RANK2_BB = RANK1_BB << 8;
constexpr uint64_t RANK3_BB = RANK1_BB << 16;
constexpr uint64_t RANK4_BB = RANK1_BB << 24;
constexpr uint64_t RANK5_BB = RANK1_BB << 32;
constexpr uint64_t RANK6_BB = RANK1_BB << 40;
constexpr uint64_t RANK7_BB = RANK1_BB << 48;
constexpr uint64_t RANK8_BB = RANK1_BB << 56;

constexpr uint64_t FILEA_BB = 0x0101010101010101ull;
constexpr uint64_t FILEB_BB = FILEA_BB << 1;
constexpr uint64_t FILEC_BB = FILEA_BB << 2;
constexpr uint64_t FILED_BB = FILEA_BB << 3;
constexpr uint64_t FILEE_BB = FILEA_BB << 4;
constexpr uint64_t FILEF_BB = FILEA_BB << 5;
constexpr uint64_t FILEG_BB = FILEA_BB << 6;
constexpr uint64_t FILEH_BB = FILEA_BB << 7;


constexpr uint64_t square_to_bb(Square s) { return 1ull << s; }
constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f); }

constexpr uint64_t operator&(uint64_t bb, Square s) { return bb & square_to_bb(s); }
constexpr uint64_t operator|(uint64_t bb, Square s) { return bb | square_to_bb(s); }
constexpr uint64_t operator^(uint64_t bb, Square s) { return bb ^ square_to_bb(s); }
constexpr uint64_t operator&=(uint64_t& bb, Square s) { return bb &= square_to_bb(s); }
constexpr uint64_t operator|=(uint64_t& bb, Square s) { return bb |= square_to_bb(s); }
constexpr uint64_t operator^=(uint64_t& bb, Square s) { return bb ^= square_to_bb(s); }

constexpr uint64_t operator&(Square s1, Square s2) { return square_to_bb(s1) & square_to_bb(s2); }
constexpr uint64_t operator|(Square s1, Square s2) { return square_to_bb(s1) | square_to_bb(s2); }
constexpr uint64_t operator^(Square s1, Square s2) { return square_to_bb(s1) ^ square_to_bb(s2); }

constexpr File file_of(Square s) { return File(s & 7); }
constexpr Rank rank_of(Square s) { return Rank(s >> 3); }

constexpr uint64_t file_bb(File f) { return FILEA_BB << f; }
constexpr uint64_t rank_bb(Rank r) { return RANK1_BB << (8 * r); }

constexpr uint64_t file_bb(Square s) { return file_bb(file_of(s)); }
constexpr uint64_t rank_bb(Square s) { return rank_bb(rank_of(s)); }

constexpr bool is_ok(Square s) { return s >= A1 && s <= H8; }

extern uint8_t SQUARE_DISTANCE[64][64];
extern uint8_t CENTER_DISTANCE[64];


void init_bitboards();

std::string bb_to_string(uint64_t bb);
std::string sq_to_string(Square s);
Square string_to_sq(const std::string& str);


typedef struct Magic
{
	uint64_t magic;
	uint64_t mask;
	uint64_t* attacks;
	uint32_t shift;

	inline uint32_t index(uint64_t occupied) const { return (occupied & mask) * magic >> shift; }
} Magic;

extern Magic ROOK_MAGICS[64];
extern Magic BISHOP_MAGICS[64];

extern uint64_t ROOK_TABLE[0x15c00];
extern uint64_t BISHOP_TABLE[0x12c0];


template<Direction D>
constexpr uint64_t shift(uint64_t bb)
{
	switch (D)
	{
		case Direction::N: return bb << 8;
	 	case Direction::S: return bb >> 8;
	 	case Direction::NN: return bb << 16;
	 	case Direction::SS: return bb >> 16;
	 	case Direction::E: return (bb & ~FILEH_BB) << 1;
	 	case Direction::W: return (bb & ~FILEA_BB) >> 1;
	 	case Direction::NE: return (bb & ~FILEH_BB) << 9;
	 	case Direction::NW: return (bb & ~FILEA_BB) << 7;
	 	case Direction::SE: return (bb & ~FILEH_BB) >> 7;
	 	case Direction::SW: return (bb & ~FILEA_BB) >> 9;
		default: return 0ull;
	}
}

template<PieceType Pt>
constexpr uint64_t attacks_bb(Square s, uint64_t occupied)
{
	#if DEBUG == true
		assert(Pt != PAWN);
		assert(::is_ok(s));
	#endif
	switch (Pt)
	{
		case BISHOP: return BISHOP_MAGICS[s].attacks[BISHOP_MAGICS[s].index(occupied)];
		case ROOK: return ROOK_MAGICS[s].attacks[ROOK_MAGICS[s].index(occupied)];
		case QUEEN: return ROOK_MAGICS[s].attacks[ROOK_MAGICS[s].index(occupied)] | BISHOP_MAGICS[s].attacks[BISHOP_MAGICS[s].index(occupied)];
		default: return PSEUDO_ATTACKS[Pt][s];
	}
}

template<Color C>
constexpr uint64_t pawn_attacks_bb(uint64_t bb)
{
	return C == WHITE ? shift<NW>(bb) | shift<NE>(bb)
				   	  : shift<SW>(bb) | shift<SE>(bb);
}

constexpr Direction pawn_push(Color c) { return c == WHITE ? Direction::N : Direction::S; }


extern uint8_t POPCOUNT16[65536];
static constexpr int popcount(uint64_t bb)
{
	#if POPCOUNT_METHOD == MANUAL
		union
		{
			uint64_t whole;
			uint16_t quarters[4];
		};
		whole = bb;
		return POPCOUNT16[quarters[0]] 
			 + POPCOUNT16[quarters[1]] 
			 + POPCOUNT16[quarters[2]] 
			 + POPCOUNT16[quarters[3]];
	#elif defined(_MSC_VER)
		return int(_mm_popcnt_u64(b));
	#else
		return __builtin_popcountll(bb);
	#endif
}

static constexpr Square lsb(uint64_t bb)
{
	#if DEBUG == true
		assert(bb);
	#endif

	#if LSB_METHOD == MANUAL
		int idx = 0;
		for (; (bb & 1) == 0; bb >>= 1) ++idx;
		return Square(idx);
	#elif defined(__GNUC__)
		return Square(__builtin_ctz(bb));
	#elif defined(_MSC_VER)
		#ifdef _WIN64
			unsigned long idx;
			_BitScanForward64(&idx, bb);
			return Square(idx);
		#else
			unsigned long idx;
			if (bb & 0xffffffff);
			{
				_BitScanForward(&idx, int32_t(bb));
				return Square(idx);
			}
			else
			{
				_BitScanForward(&idx, int32_t(bb >> 32));
				return Square(idx + 32);
			}
		#endif
	#else
		#define LSB_METHOD MANUAL
		return lsb(bb);
	#endif
}

static constexpr Square pop_lsb(uint64_t& bb)
{
	#if DEBUG == true
		assert(bb);
	#endif
	const Square s = lsb(bb);
	bb &= bb - 1;
	return s;
}


#endif // BITBOARD_H_DE3A36A4E0B0
