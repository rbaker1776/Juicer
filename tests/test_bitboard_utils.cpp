#include <iostream>
#include <random>
#include <bitset>
#include "../bitboard.h"
#include "minunit.h"


namespace BitboardUtils
{
	static void bb_popcount()
	{
		mu_assert(popcount(0) == 0);
		mu_assert(popcount(1) == 1);
		mu_assert(popcount(0xf) == 4);

		for (int i = 1; i < 64; ++i) 
		{ 
			mu_assert(popcount(1ull << i) == 1);
			mu_assert(popcount((1ull << i) - 1) == i);
		}

		std::random_device rd;
		std::mt19937_64 gen(rd());
		std::uniform_int_distribution<uint64_t> dis;
		for (int i = 0; i < 10000; ++i)
		{
			uint64_t bb = dis(gen);
			mu_assert(popcount(bb) == std::bitset<64>(bb).count());
		}
	}

	static void bb_bitwise_operators()
	{
		mu_assert((RANK1_BB | RANK2_BB) == 0xffff);
		mu_assert((RANK2_BB & Square::A1) == 0);
		mu_assert((FILEF_BB | Square::F1) == FILEF_BB);

		for (File f = FILE_A; f <= FILE_H; ++f)
			for (File g = FILE_A; g < f; ++g)
				mu_assert((file_bb(f) & file_bb(g)) == 0);
		
		for (Rank r = RANK_1; r <= RANK_8; ++r)
			for (Rank s = RANK_1; s < r; ++s)
				mu_assert((rank_bb(r) & rank_bb(s)) == 0);

		for (Rank r = RANK_1; r <= RANK_8; ++r)
			for (File f = FILE_A; f <= FILE_H; ++f)
				mu_assert(popcount(rank_bb(r) & file_bb(f)) == 1);

		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			int overlaps = 0;
			for (Square s = A1; s <= H8; ++s)
			{
				if (file_bb(f) & s)
				{	
					mu_assert(popcount(file_bb(f) ^ s) == 7);
					mu_assert(popcount(file_bb(f) | s) == 8);
					++overlaps;
				}
				else
				{
					mu_assert(popcount(file_bb(f) | s) == 9);
					mu_assert(popcount(file_bb(f) ^ s) == 9);
				}
			}
			mu_assert(overlaps == 8);
		}

		for (Rank r = RANK_1; r <= RANK_8; ++r)
		{
			int overlaps = 0;
			for (Square s = A1; s <= H8; ++s)
			{
				if (rank_bb(r) & s)
				{	
					mu_assert(popcount(rank_bb(r) ^ s) == 7);
					mu_assert(popcount(rank_bb(r) | s) == 8);
					++overlaps;
				}
				else
				{
					mu_assert(popcount(rank_bb(r) | s) == 9);
					mu_assert(popcount(rank_bb(r) ^ s) == 9);
				}
			}
			mu_assert(overlaps == 8);
		}

		for (Square s = A1; s <= H8; ++s)
			mu_assert(square_to_bb(s) == (rank_bb(s) & file_bb(s)));

		mu_assert((FILEA_BB | B1) == (A8 | A7 | A6 | A5 | A4 | A3 | A2 | A1 | B1));
	}

	static void bb_distances()
	{
		for (Square s: { E4, D4, E5, D5 })
			mu_assert(CENTER_DISTANCE[s] == 0);

		for (Square s: { C3, E3, D3, F3, F4, F5, F6, D6, E6, C6, C4, C5 })
			mu_assert(CENTER_DISTANCE[s] == 1);

		for (Square s: { B2, C2, D2, E2, F2, G2, G3, G4, G5, G6, G7, F7, E7, D7, C7, B7, B6, B5, B4, B3 })
			mu_assert(CENTER_DISTANCE[s] == 2);

		for (Square s: { A1, B1, C1, D1, E1, F1, G1, H1, A8, B8, C8, D8, E8, F8, G8, H8, A2, A3, A4, A5, A6, A7, H2, H3, H4, H5, H6, H7 })
			mu_assert(CENTER_DISTANCE[s] == 3);

		mu_assert(SQUARE_DISTANCE[A1][A1] == 0);
		for (Square s: { A2, B2, B1 })
			mu_assert(SQUARE_DISTANCE[A1][s] == 1);

		for (Square s:  {A3, B3, C3, C2, C1 })
			mu_assert(SQUARE_DISTANCE[s][A1] == 2);

		for (Square s: { C8, C7, C6, C5, C4, D4, D4, E4, F4, G4, H4 })
			mu_assert(SQUARE_DISTANCE[s][F7] == 3, std::to_string(int(s)));

		for (Square s: { H8, H7, H6, H5, H4, H3, H2, H1, A1, B1, C1, D1, E1, F1, G1 })
		{
			mu_assert(SQUARE_DISTANCE[s][A8] == 7);
			mu_assert(SQUARE_DISTANCE[s][B7] == 6);
			mu_assert(SQUARE_DISTANCE[s][C6] == 5);
			mu_assert(SQUARE_DISTANCE[s][D5] == 4);
		}

		for (Square s: { H8, H7, G8 })
		{
			mu_assert(SQUARE_DISTANCE[s][A1] == 7);
			mu_assert(SQUARE_DISTANCE[s][B2] == 6);
			mu_assert(SQUARE_DISTANCE[s][C3] == 5);
			mu_assert(SQUARE_DISTANCE[s][D4] == 4);
			mu_assert(SQUARE_DISTANCE[s][E5] == 3);
			mu_assert(SQUARE_DISTANCE[s][F6] == 2);
			mu_assert(SQUARE_DISTANCE[s][G7] == 1);
		}

		for (Square s1: { A1, B1, C1, D1, E1, F1, G1, H1 })
			for (Square s2: { A8, B8, C8, D8, E8, F8, G8, H8 })
				mu_assert(SQUARE_DISTANCE[s1][s2] == 7);

		for (Square s1: { A1, A2, A3, A4, A5, A6, A7, A8 })
			for (Square s2: { H1, H2, H3, H4, H5, H6, H7, H8 })
				mu_assert(SQUARE_DISTANCE[s1][s2] == 7);
	}

	static void suite()
	{
		mu_run(BitboardUtils::bb_popcount);
		mu_run(BitboardUtils::bb_bitwise_operators);
		mu_run(BitboardUtils::bb_distances);
	}
}
