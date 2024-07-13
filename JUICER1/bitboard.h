#ifndef BITBOARD_H_DE3A36A4E0B0
#define BITBOARD_H_DE3A36A4E0B0

#include <string>
#include <cassert>
#include <iostream>
#include <bitset>
#include <array>
#include "types.h"
#include "movegen.h"
#include "juicer.h"


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


#if (DEBUG)
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
#endif // DEBUG


#if (POPCOUNT_METHOD == MANUAL)
// through experimentation, this ugly function minimized constexpr steps required to 393222
// 393222 = 6(2^16 + 1)
// I didn't have to look up any of the the powers of two
// is that a brag or a sad reflection on what I carry in my mind?
static consteval uint8_t manual_popcount16(int n)
{
	return bool(n & 1) + bool(n & 2) + bool(n & 4) + bool(n & 8)
	     + bool(n & 16) + bool(n & 32) + bool(n & 64) + bool(n & 128)
	     + bool(n & 256) + bool(n & 512) + bool(n & 1024) + bool(n & 2048)
	     + bool(n & 4096) + bool(n & 8192) + bool(n & 16384) + bool(n & 32768);
}

static consteval std::array<uint8_t, 65536> fill_popcount()
{
	std::array<uint8_t, 65536> popcount_16 = {};
	for (int i = 0; i < 65536; ++i)
		popcount_16[i] = manual_popcount16(i);
	return popcount_16;
}

static constexpr std::array<uint8_t, 65536> POPCOUNT16 = fill_popcount();
#endif // POPCOUNT == MANUAL

static constexpr int popcount(uint64_t bb)
{
	#if (POPCOUNT_METHOD == MANUAL)
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
		return int(_mm_popcnt_u64(bb));
	#elif defined(__GNUC__)
		return __builtin_popcountll(bb);
	#endif
}


static constexpr Square square_of(uint64_t bb)
{
	#if (DEBUG)
		assert(bb);
	#endif

	#if LSB_METHOD == MANUAL
		int idx = 0;
		for (; (bb & 1) == 0; bb >>= 1) ++idx;
		return Square(idx);
	#elif defined(__GNUC__)
		return Square(__builtin_ctzll(bb));
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
		return square_of(bb);
	#endif
}

static constexpr Square pop_lsb(uint64_t& bb)
{
	#if (DEBUG)
		assert(bb);
	#endif
	const Square s = square_of(bb);
	bb &= bb - 1;
	return s;
}


#endif // BITBOARD_H_DE3A36A4E0B0
