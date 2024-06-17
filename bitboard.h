#ifndef BITBOARD_H_DE3A36A4E0B0
#define BITBOARD_H_DE3A36A4E0B0

#include "types.h"


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


extern uint8_t POPCOUNT16[65536];
constexpr int popcount(uint64_t bb)
{
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
}


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


static std::string bb_to_string(uint64_t bb)
{
	const std::string newline = "+---+---+---+---+---+---+---+---+\n";
	std::string s = newline;

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			s += bb & make_square(f, r) ? "| X " : "|   ";
		}
		s += "| " + std::to_string(r + 1) + '\n' + newline;
	}
	
	return s + "  a   b   c   d   e   f   g   h";
}

static std::string sq_to_string(Square s)
{
	std::string str = "A1";
	str[0] += s & 7;
	str[1] += s >> 3;
	return str;
}


#endif // BITBOARD_H_DE3A36A4E0B0
