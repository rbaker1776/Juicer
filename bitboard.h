#ifndef BITBOARD_H_0CEDFBF5F401
#define BITBOARD_H_0CEDFBF5F401

#include <cstdint>
#include <array>
#include <bitset>
#include <string>
#include <sstream>
#include <cassert>
#include "types.h"
#include "juicer.h"


static constexpr uint64_t RANK1_BB = 0xff;
static constexpr uint64_t RANK2_BB = RANK1_BB << 8;
static constexpr uint64_t RANK3_BB = RANK1_BB << 16;
static constexpr uint64_t RANK4_BB = RANK1_BB << 24;
static constexpr uint64_t RANK5_BB = RANK1_BB << 32;
static constexpr uint64_t RANK6_BB = RANK1_BB << 40;
static constexpr uint64_t RANK7_BB = RANK1_BB << 48;
static constexpr uint64_t RANK8_BB = RANK1_BB << 56;

static constexpr uint64_t FILEA_BB = 0x0101010101010101ull;
static constexpr uint64_t FILEB_BB = FILEA_BB << 1;
static constexpr uint64_t FILEC_BB = FILEA_BB << 2;
static constexpr uint64_t FILED_BB = FILEA_BB << 3;
static constexpr uint64_t FILEE_BB = FILEA_BB << 4;
static constexpr uint64_t FILEF_BB = FILEA_BB << 5;
static constexpr uint64_t FILEG_BB = FILEA_BB << 6;
static constexpr uint64_t FILEH_BB = FILEA_BB << 7;

static constexpr uint64_t BOARD_BB = UINT64_MAX;


constexpr uint64_t square_to_bb(Square s) { return 1ull << s; }
constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f); }

constexpr uint64_t operator&(uint64_t bb, Square s) { return bb & square_to_bb(s); }
constexpr uint64_t operator|(uint64_t bb, Square s) { return bb | square_to_bb(s); }
constexpr uint64_t operator^(uint64_t bb, Square s) { return bb ^ square_to_bb(s); }

constexpr uint64_t operator&=(uint64_t& bb, Square s) { return bb &= square_to_bb(s); }
constexpr uint64_t operator|=(uint64_t& bb, Square s) { return bb |= square_to_bb(s); }
constexpr uint64_t operator^=(uint64_t& bb, Square s) { return bb ^= square_to_bb(s); }

constexpr uint64_t operator&(Square s1, Square s2) { return square_to_bb(s1) & s2; }
constexpr uint64_t operator|(Square s1, Square s2) { return square_to_bb(s1) | s2; }
constexpr uint64_t operator^(Square s1, Square s2) { return square_to_bb(s1) ^ s2; }

constexpr File file_of(Square s) { return File(s & 7); }
constexpr Rank rank_of(Square s) { return Rank(s >> 3); }

constexpr Rank operator&(Rank r, Color c) { return c == WHITE ? r : Rank(RANK_8 - r); }
constexpr Square operator&(Square s, Color c) { return c == WHITE ? s : make_square(file_of(s), rank_of(s) & BLACK); }

constexpr uint64_t file_bb(File f) { return FILEA_BB << f; }
constexpr uint64_t rank_bb(Rank r) { return RANK1_BB << (8 * r); }

constexpr uint64_t file_bb(Square s) { return file_bb(file_of(s)); }
constexpr uint64_t rank_bb(Square s) { return rank_bb(rank_of(s)); }

constexpr bool is_ok(Square s) { return s >= A1 && s <= H8; }


template<Color C> consteval uint64_t ep_rank_bb() { if constexpr (C == WHITE) return RANK5_BB; else return RANK4_BB; }
template<Color C> consteval uint64_t rank_2_bb()  { if constexpr (C == WHITE) return RANK2_BB; else return RANK7_BB; }
template<Color C> consteval uint64_t rank_6_bb()  { if constexpr (C == WHITE) return RANK6_BB; else return RANK3_BB; }
template<Color C> consteval uint64_t rank_7_bb()  { if constexpr (C == WHITE) return RANK7_BB; else return RANK2_BB; }


#if (POPCOUNT == MANUAL)
// this ugly function minimizes constexpr steps to 393222 = 6 * (2^16 + 1)
static consteval uint8_t popcount16(uint16_t n)
{
	return bool(n & 1) + bool(n & 2) + bool(n & 4) + bool(n & 8)
	     + bool(n & 16) + bool(n & 32) + bool(n & 64) + bool(n & 128)
		 + bool(n & 256) + bool(n & 512) + bool(n & 1024) + bool(2048)
		 + bool(n & 4096) + bool(n & 8192) + bool(n & 16384) + bool(n & 32768);
}

static consteval std::array<uint8_t, 65536> fill_popcount()
{
	std::array<uint8_t, 65536> popcount;
	for (uint16_t i = 0; i <= UINT16_MAX; ++i)
		popcount_16[i] = popcount16(i);
	return popcount;
}

static constexpr std::array<uint8_t, 65536> POPCOUNT16 = fill_popcount();
#endif // (POPCOUNT == MANUAL)


static constexpr size_t popcount(uint64_t bb)
{
	#if (POPCOUNT == MANUAL)
		union
		{
			uint64_t u64;
			uint16_t u16[4];
		} u = {bb};
		return POPCOUNT16[u.u16[0]] + POPCOUNT16[u.u16[1]] + POPCOUNT16[u.u16[2]] + POPCOUNT16[u.u16[3]];
	#elif defined(__GNUC__)
		return __builtin_popcountll(bb);
	#elif defined(_MSC_VER)
		return int(_mm_popcnt_u64(bb));
	#else
		#error "Compiler not supported for builtin popcount. Set POPCOUNT to MANUAL in juicer.h."
	#endif
}


static constexpr Square lsb(uint64_t bb)
{
	#if (DEBUG)
		assert(bb);
	#endif

	#if (LSB == MANUAL)
		Square s;
		for (s = A1; !(bb & s); ++s) {}
		return s;
	#elif defined(__GNUC__)
		return Square(__builtin_ctzll(bb));
	#elif defined(_MSC_VER)
		#ifdef _WIN64
			unsigned long idx;
			_BitScanForward64(&idx, bb);
			return Square(idx);
		#else
			unsigned long idx;
			if (bb & 0xffffffff)
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
		#error "Compiler not supported for builtin lsb. Set LSB to MANUAL in juicer.h"
	#endif
}

static constexpr Square pop_lsb(uint64_t& bb)
{
	#if (DEBUG)
		assert(bb);
	#endif

	const Square s = lsb(bb);
	bb &= bb - 1;
	return s;
}


#if (DEBUG)
static std::string bb_to_string(uint64_t bb, char marker = 'X')
{
	std::ostringstream ss;
	ss << "+---+---+---+---+---+---+---+---+\n";

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			ss << '|' << ' ' << (bb & make_square(f, r) ? marker : ' ') << ' ';
		}
		ss << '|' << ' ' << std::to_string(r + 1) << '\n';
		ss << "+---+---+---+---+---+---+---+---+\n";
	}

	ss << "  a   b   c   d   e   f   g   h";
	return ss.str();
}
#endif // (DEBUG)


#endif // BITBOARD_H_0CEDFBF5F401
