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


namespace Bitboard
{
	static constexpr uint64_t RANK1 {0xff};
	static constexpr uint64_t RANK2 {RANK1 << 8};
	static constexpr uint64_t RANK3 {RANK1 << 16};
	static constexpr uint64_t RANK4 {RANK1 << 24};
	static constexpr uint64_t RANK5 {RANK1 << 32};
	static constexpr uint64_t RANK6 {RANK1 << 40};
	static constexpr uint64_t RANK7 {RANK1 << 48};
	static constexpr uint64_t RANK8 {RANK1 << 56};

	static constexpr uint64_t FILEA {0x0101010101010101};
	static constexpr uint64_t FILEB {FILEA << 1};
	static constexpr uint64_t FILEC {FILEA << 2};
	static constexpr uint64_t FILED {FILEA << 3};
	static constexpr uint64_t FILEE {FILEA << 4};
	static constexpr uint64_t FILEF {FILEA << 5};
	static constexpr uint64_t FILEG {FILEA << 6};
	static constexpr uint64_t FILEH {FILEA << 7};

	static constexpr uint64_t BOARD {UINT64_MAX};

	template<Color C> static consteval uint64_t rank_1() { if constexpr (C == WHITE) return RANK1; else return RANK8; }
	template<Color C> static consteval uint64_t rank_2() { if constexpr (C == WHITE) return RANK2; else return RANK7; }
	template<Color C> static consteval uint64_t rank_3() { if constexpr (C == WHITE) return RANK3; else return RANK6; }
	template<Color C> static consteval uint64_t rank_4() { if constexpr (C == WHITE) return RANK4; else return RANK5; }
	template<Color C> static consteval uint64_t rank_5() { if constexpr (C == WHITE) return RANK5; else return RANK4; }
	template<Color C> static consteval uint64_t rank_6() { if constexpr (C == WHITE) return RANK6; else return RANK3; }
	template<Color C> static consteval uint64_t rank_7() { if constexpr (C == WHITE) return RANK7; else return RANK2; }
	template<Color C> static consteval uint64_t rank_8() { if constexpr (C == WHITE) return RANK8; else return RANK1; }

	#if (DEBUG)
	[[maybe_unused]] static std::string bb_to_string(uint64_t bb, char marker = 'X')
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
} // namespace Bitboard


constexpr uint64_t file_bb(File f) { return Bitboard::FILEA << f; }
constexpr uint64_t rank_bb(Rank r) { return Bitboard::RANK1 << (8 * r); }

constexpr uint64_t file_bb(Square s) { return file_bb(file_of(s)); }
constexpr uint64_t rank_bb(Square s) { return rank_bb(rank_of(s)); }

constexpr bool is_ok(Square s) { return s >= A1 && s <= H8; }


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


force_inline constexpr size_t popcount(uint64_t bb)
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


force_inline constexpr Square lsb(uint64_t bb)
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

force_inline constexpr Square pop_lsb(uint64_t& bb)
{
	#if (DEBUG)
		assert(bb);
	#endif

	const Square s = lsb(bb);
	bb &= bb - 1;
	return s;
}


#endif // BITBOARD_H_0CEDFBF5F401
