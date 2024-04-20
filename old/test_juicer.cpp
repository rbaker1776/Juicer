#include <iostream>
#include "miniunit.h"
#include "bitboard.h"
#include "random.h"
#include "position.h"



static int test_popcount()
{{{
	mu_configure();

	mu_assert(popcount(0) == 0);
	mu_assert(popcount(1) == 1);
	mu_assert(popcount(2) == 1);
	mu_assert(popcount(4) == 1);
	mu_assert(popcount(1<<16) == 1);
	mu_assert(popcount(1<<30) == 1);
	mu_assert(popcount(1ull<<60) == 1);
	mu_assert(popcount((1ull<<60)-1) == 60);
	mu_assert(popcount(0xf0f0f0f0f0f0f0f0ull) == 32);
	mu_assert(popcount(92837450981089ull) == 26);
	mu_assert(popcount(13485980181943276534ull) == 36);
	mu_assert(popcount(999999999) == 21);
	mu_assert(popcount(1000000000) == 13);

	mu_return();
}}}


static int test_bitwise_board_operators()
{{{
	mu_configure();

	mu_assert((fileb_bb | SQ_b1) == fileb_bb);
	mu_assert((fileb_bb & SQ_b1) == square_to_bb(SQ_b1));
	mu_assert((fileb_bb & rank1_bb) == square_to_bb(SQ_b1));
	mu_assert(rank_bb(RANK_7) == rank7_bb);
	mu_assert(rank_of(SQ_c4) == 3);
	mu_assert(rank_bb(SQ_c4) == rank4_bb);
	mu_assert(file_bb(FILE_A) == filea_bb);
	mu_assert(file_of(SQ_f6) == 5);
	mu_assert(file_bb(SQ_f6) == filef_bb);

	mu_return();
}}}


static int test_square_distances()
{{{
	mu_configure();

	mu_assert(SquareDistance[SQ_d4][SQ_e4] == 1);
	mu_assert(SquareDistance[SQ_e4][SQ_d4] == 1);
	mu_assert(SquareDistance[SQ_e4][SQ_e4] == 0);
	mu_assert(SquareDistance[SQ_a1][SQ_h8] == 7);
	mu_assert(CenterDistance[SQ_a1] == 3);
	mu_assert(CenterDistance[SQ_a8] == 3);
	mu_assert(CenterDistance[SQ_h8] == 3);
	mu_assert(CenterDistance[SQ_b2] == 2);
	mu_assert(CenterDistance[SQ_b1] == 3);
	mu_assert(CenterDistance[SQ_f6] == 1);
	mu_assert(CenterDistance[SQ_e5] == 0);

	mu_return();
}}}


static int test_line_masks()
{{{
	mu_configure();

	mu_assert(LineBB[SQ_a1][SQ_a3] == filea_bb);
	mu_assert(BetweenBB[SQ_a1][SQ_d4] == (square_to_bb(SQ_b2) | SQ_c3 | SQ_d4));
	mu_assert(BetweenBB[SQ_e4][SQ_d4] == square_to_bb(SQ_d4));
	mu_assert(LineBB[SQ_e4][SQ_d4] == rank4_bb);
	mu_assert(LineBB[SQ_e4][SQ_d1] == 0);
	mu_assert(BetweenBB[SQ_e4][SQ_d1] == 0);

	mu_return();
}}}


static int test_king_attacks()
{{{
    mu_configure();

	PRNG rng(42);

	for (int i = 0; i < 10'000; ++i)
	{
		Square s = Square(rng.rand<uint32_t>() & 0b111111);
		uint64_t occupied = rng.sparse_rand<uint64_t>();
		uint64_t attacks = 0;

		Rank r = rank_of(s);
		File f = file_of(s);

		if (r < RANK_8 && f > FILE_A)
		{ 
			attacks |= make_square(f, Rank(r+1));
			attacks |= make_square(File(f-1), Rank(r+1)); 
			attacks |= make_square(File(f-1), r); 
		}

		if (r < RANK_8 && f < FILE_H)
		{ 
			attacks |= make_square(f, Rank(r+1));
			attacks |= make_square(File(f+1), Rank(r+1)); 
			attacks |= make_square(File(f+1), r); 
		}

		if (r > RANK_1 && f < FILE_H)
		{ 
			attacks |= make_square(f, Rank(r-1));
			attacks |= make_square(File(f+1), Rank(r-1)); 
			attacks |= make_square(File(f+1), r); 
		}

		if (r > RANK_1 && f > FILE_A)
		{ 
			attacks |= make_square(f, Rank(r-1));
			attacks |= make_square(File(f-1), Rank(r-1)); 
			attacks |= make_square(File(f-1), r); 
		}

		attacks &= ~occupied;
		
		mu_set_error_message(std::to_string(int(s)));
		mu_assert(attacks_bb<KING>(s, occupied) == attacks);
	}

    mu_return();
}}}


static int test_knight_attacks()
{{{ 
    mu_configure();

	PRNG rng(42);

	for (int i = 0; i < 10'000; ++i)
	{
		Square s = Square(rng.rand<uint32_t>() & 0b111111);
		uint64_t occupied = rng.sparse_rand<uint64_t>();
		uint64_t attacks = 0;

		Rank r = rank_of(s);
		File f = file_of(s);

		if (r <= RANK_6 && f >= FILE_B) { attacks |= make_square(File(f-1), Rank(r+2)); }
		if (r <= RANK_6 && f <= FILE_G) { attacks |= make_square(File(f+1), Rank(r+2)); }
		if (r <= RANK_7 && f <= FILE_F) { attacks |= make_square(File(f+2), Rank(r+1)); }
		if (r <= RANK_7 && f >= FILE_C) { attacks |= make_square(File(f-2), Rank(r+1)); }
		if (r >= RANK_2 && f >= FILE_C) { attacks |= make_square(File(f-2), Rank(r-1)); }
		if (r >= RANK_3 && f >= FILE_B) { attacks |= make_square(File(f-1), Rank(r-2)); }
		if (r >= RANK_2 && f <= FILE_F) { attacks |= make_square(File(f+2), Rank(r-1)); }
		if (r >= RANK_3 && f <= FILE_G) { attacks |= make_square(File(f+1), Rank(r-2)); }
		
		attacks &= ~occupied;
		
		mu_set_error_message(std::to_string(int(s)));
		mu_assert(attacks_bb<KNIGHT>(s, occupied) == attacks);
	}

    mu_return();
}}}


static int test_rook_attacks()
{{{ 
    mu_configure();

	PRNG rng(42);
	
	for (int i = 0; i < 10'000; ++i)
	{
		Square s = Square(rng.rand<uint32_t>() & 0b111111);
		uint64_t occupied = rng.rand<uint64_t>();
		uint64_t attacks = 0;

		for (Square to = s+NORTH; rank_of(s) != RANK_8; to += NORTH)
		{
			attacks |= to;
			if (occupied & to || rank_of(to) == RANK_8) { break; }
		}
		for (Square to = s+SOUTH; rank_of(s) != RANK_1; to += SOUTH)
		{
			attacks |= to;
			if (occupied & to || rank_of(to) == RANK_1) { break; }
		}
		for (Square to = s+WEST; file_of(s) != FILE_A; to += WEST)
		{
			attacks |= to;
			if (occupied & to || file_of(to) == FILE_A) { break; }
		}
		for (Square to = s+EAST; file_of(s) != FILE_H; to += EAST)
		{
			attacks |= to;
			if (occupied & to || file_of(to) == FILE_H) { break; }
		}
				
		mu_set_error_message(std::to_string(int(s)));
		mu_assert(attacks_bb<ROOK>(s, occupied) == attacks);
	}

    mu_return();
}}}


static int test_bishop_attacks()
{{{
    mu_configure();

	PRNG rng(42);
	
	for (int i = 0; i < 10'000; ++i)
	{
		Square s = Square(rng.rand<uint32_t>() & 0b111111);
		uint64_t occupied = rng.rand<uint64_t>();
		uint64_t attacks = 0;

		for (Square to = s+NORTH_EAST; rank_of(s) != RANK_8 && file_of(s) != FILE_H; to += NORTH_EAST)
		{
			attacks |= to;
			if (occupied & to || rank_of(to) == RANK_8 || file_of(to) == FILE_H) { break; }
		}
		for (Square to = s+SOUTH_EAST; rank_of(s) != RANK_1 && file_of(s) != FILE_H; to += SOUTH_EAST)
		{
			attacks |= to;
			if (occupied & to || rank_of(to) == RANK_1 || file_of(to) == FILE_H) { break; }
		}
		for (Square to = s+NORTH_WEST; file_of(s) != FILE_A && rank_of(s) != RANK_8; to += NORTH_WEST)
		{
			attacks |= to;
			if (occupied & to || file_of(to) == FILE_A || rank_of(to) == RANK_8) { break; }
		}
		for (Square to = s+SOUTH_WEST; file_of(s) != FILE_A && rank_of(s) != RANK_1; to += SOUTH_WEST)
		{
			attacks |= to;
			if (occupied & to || file_of(to) == FILE_A || rank_of(to) == RANK_1) { break; }
		}
				
		mu_set_error_message(std::to_string(int(s)));
		mu_assert(attacks_bb<BISHOP>(s, occupied) == attacks);
	}

    mu_return();
}}}


static int test_queen_attacks()
{{{
    mu_configure();

	PRNG rng(42);

	for (int i = 0; i < 10'000; ++i)
	{
		Square s = Square(rng.rand<uint32_t>() & 0b111111);
		uint64_t occupied = rng.rand<uint64_t>();
		uint64_t attacks = 0;

		for (Square to = s+NORTH; rank_of(s) != RANK_8; to += NORTH)
		{
			attacks |= to;
			if (occupied & to || rank_of(to) == RANK_8) { break; }
		}
		for (Square to = s+SOUTH; rank_of(s) != RANK_1; to += SOUTH)
		{
			attacks |= to;
			if (occupied & to || rank_of(to) == RANK_1) { break; }
		}
		for (Square to = s+WEST; file_of(s) != FILE_A; to += WEST)
		{
			attacks |= to;
			if (occupied & to || file_of(to) == FILE_A) { break; }
		}
		for (Square to = s+EAST; file_of(s) != FILE_H; to += EAST)
		{
			attacks |= to;
			if (occupied & to || file_of(to) == FILE_H) { break; }
		}
		for (Square to = s+NORTH_EAST; rank_of(s) != RANK_8 && file_of(s) != FILE_H; to += NORTH_EAST)
		{
			attacks |= to;
			if (occupied & to || rank_of(to) == RANK_8 || file_of(to) == FILE_H) { break; }
		}
		for (Square to = s+SOUTH_EAST; rank_of(s) != RANK_1 && file_of(s) != FILE_H; to += SOUTH_EAST)
		{
			attacks |= to;
			if (occupied & to || rank_of(to) == RANK_1 || file_of(to) == FILE_H) { break; }
		}
		for (Square to = s+NORTH_WEST; file_of(s) != FILE_A && rank_of(s) != RANK_8; to += NORTH_WEST)
		{
			attacks |= to;
			if (occupied & to || file_of(to) == FILE_A || rank_of(to) == RANK_8) { break; }
		}
		for (Square to = s+SOUTH_WEST; file_of(s) != FILE_A && rank_of(s) != RANK_1; to += SOUTH_WEST)
		{
			attacks |= to;
			if (occupied & to || file_of(to) == FILE_A || rank_of(to) == RANK_1) { break; }
		}
				
		mu_set_error_message(std::to_string(int(s)));
		mu_assert(attacks_bb<QUEEN>(s, occupied) == attacks);
	}

    mu_return();
}}}


int main()
{
	Bitboard::init();

	mu_run(test_popcount);
	mu_run(test_bitwise_board_operators);
	mu_run(test_square_distances);
	mu_run(test_line_masks);
	mu_run(test_king_attacks);
	mu_run(test_knight_attacks);
	mu_run(test_rook_attacks);
	mu_run(test_bishop_attacks);
	mu_run(test_queen_attacks);

	return 0;
}
