#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include "magics.h"
#include "../bitboard.h"
#include "../types.h"
#include "../xorshiftstar64.h"


std::vector<std::vector<uint64_t>> ROOK_BLOCKERS(64);
std::vector<std::vector<uint64_t>> BISHOP_BLOCKERS(64);

std::vector<std::vector<uint64_t>> ROOK_LEGAL_MOVES(64);
std::vector<std::vector<uint64_t>> BISHOP_LEGAL_MOVES(64);

uint8_t SQUARE_DISTANCE[64][64];
uint8_t CENTER_DISTANCE[64];

uint8_t POPCOUNT16[65536];


int table_size(PieceType pt)
{
	Magic* m = pt == ROOK ? ROOK_MAGICS : BISHOP_MAGICS;
	int sum = 0;
	for (Square s = A1; s <= H8; ++s)
		sum += m[s].table_size;
	return sum;
}


static void write_magics()
{
	std::fstream file("magics.h", std::ios::in | std::ios::out);
	std::string line;
	std::streampos pos = 0;

	for (int i = 0; std::getline(file, line) && i < 12; ++i)
		pos = file.tellg();

	int fd = open("magics.h", O_RDWR);
	ftruncate(fd, file.tellp());
	close(fd);

	file << "Magic ROOK_MAGICS[64] = {" << std::endl;
	for (Square s = A1; s <= H8; ++s)
	{
		file << '\t' << '{' << ' ';
		file << ".magic = 0x" << std::hex << ROOK_MAGICS[s].magic << "ull" << ',' << ' ';
		file << ".mask = 0x" << std::hex << ROOK_MAGICS[s].mask << "ull" << ',' << ' ';
		file << ".attacks = nullptr" << ',' << ' ';
		file << ".shift = " << std::dec << ROOK_MAGICS[s].shift << ',' << ' ';
		file << ".table_size = " << ROOK_MAGICS[s].table_size << ' ';
		file << '}' << ',' << " // " << sq_to_string(s) << std::endl;
	}
	file << '}' << ';' << std::endl << std::endl;

	file << "Magic BISHOP_MAGICS[64] = {" << std::endl;
	for (Square s = A1; s <= H8; ++s)
	{
		file << '\t' << '{' << ' ';
		file << ".magic = 0x" << std::hex << BISHOP_MAGICS[s].magic << "ull" << ',' << ' ';
		file << ".mask = 0x" << std::hex << BISHOP_MAGICS[s].mask << "ull" << ',' << ' ';
		file << ".attacks = nullptr" << ',' << ' ';
		file << ".shift = " << std::dec << BISHOP_MAGICS[s].shift << ',' << ' ';
		file << ".table_size = " << BISHOP_MAGICS[s].table_size << ' ';
		file << '}' << ',' << " // " << sq_to_string(s) << std::endl;
	}
	file << '}' << ';' << std::endl << std::endl;
}


static uint64_t safe_step(Square s, int step)
{
	Square to = Square(s + step);
	return (is_ok(to) && SQUARE_DISTANCE[s][to] <= 2) ? square_to_bb(to) : 0;
}


static uint64_t sliding_attack(PieceType pt, Square s, uint64_t occupied)
{
	uint64_t attacks = 0;
	Direction rook_directions[4] = { N, S, E, W };
	Direction bishop_directions[4] = { NE, SE, NW, SW };

	for (Direction d: (pt == ROOK ? rook_directions : bishop_directions))
	{
		Square from = s;
		while (safe_step(from, d) && !(occupied & from)) 
		{ 
			attacks |= (from += d); 
		}
	}
	return attacks;
}


static void init_magic(Square s, PieceType pt)
{
	const uint64_t edges = ((FILEA_BB | FILEH_BB) & ~file_bb(s)) | ((RANK1_BB | RANK8_BB) & ~rank_bb(s));
	Magic& m = pt == ROOK ? ROOK_MAGICS[s] : BISHOP_MAGICS[s];

	m.mask = sliding_attack(pt, s, 0) & ~edges;
	m.table_size = 1 << popcount(m.mask);
	m.shift = 64 - popcount(m.mask);
	
	uint32_t epoch[4096] = { 0 };
	uint64_t attacks[4096] = { 0 };
	
	const std::vector<uint64_t>& blocker_boards = pt == ROOK ? ROOK_BLOCKERS[s] : BISHOP_BLOCKERS[s];
	const std::vector<uint64_t>& legal_moves = pt == ROOK ? ROOK_LEGAL_MOVES[s] : BISHOP_LEGAL_MOVES[s];

	int attempt = 0;
	for (int i = 0; i < m.table_size && attempt < INT_MAX; )
	{	
		for (m.magic = 0ull; popcount((m.magic * m.mask) >> 56) < 6; )
		{
			m.magic = xrs::sparse_rand<uint64_t>();

			for (++attempt, i = 0; i < m.table_size; ++i)
			{
				int magic_idx = m.index(blocker_boards[i]);
				if (epoch[magic_idx] < attempt) // this is a free spot in the table
				{
					epoch[magic_idx] = attempt;
					attacks[magic_idx] = legal_moves[i];
				}
				else if (attacks[magic_idx] != legal_moves[i]) // fatal collision
					break;
			}
		}
	}
}


static uint64_t next_bit_permutation(uint64_t n)
{
	uint64_t n0 = n & -n;
	uint64_t m = n + n0;
	uint64_t ones = n ^ m;
	return m | ((ones / n0) >> 2);
}


static int sub_table_size(uint64_t magic, Square s, PieceType pt)
{
	const std::vector<uint64_t>& blocker_boards = pt == ROOK ? ROOK_BLOCKERS[s] : BISHOP_BLOCKERS[s];
	const std::vector<uint64_t>& legal_moves = pt == ROOK ? ROOK_LEGAL_MOVES[s] : BISHOP_LEGAL_MOVES[s];
	const Magic& m = pt == ROOK ? ROOK_MAGICS[s] : BISHOP_MAGICS[s];

	uint32_t epoch[4096] = { 0 };
	uint64_t attacks[4096] = { 0 };
	int max_idx = 0;

	for (int i = 0; i < blocker_boards.size(); ++i)
	{
		int magic_idx = (blocker_boards[i] & m.mask) * magic >> (m.shift + 1);
		max_idx = std::max(max_idx, magic_idx);
		if (epoch[magic_idx] == 0) // this is a free spot in the table
		{
			attacks[magic_idx] = legal_moves[i];
			epoch[magic_idx] = 1;
		}
		else if (attacks[magic_idx] != legal_moves[i]) // fatal collision
		{
			return INT_MAX;
		}
	}

	return max_idx + 1;
}


int main()
{
	for (int i = 0; i < 65536; ++i) { POPCOUNT16[i] = std::bitset<16>(i).count(); }

	for (Square s1 = A1; s1 <= H8; ++s1)
	{
		CENTER_DISTANCE[s1] = int(std::max(
			std::abs(rank_of(s1) - 3.5), 
			std::abs(file_of(s1) - 3.5)
		));
		for (Square s2 = A1; s2 <= H8; ++s2)
		{
			SQUARE_DISTANCE[s1][s2] = std::max(
				std::abs(rank_of(s1) - rank_of(s2)),
				std::abs(file_of(s1) - file_of(s2))
			);
		}
	}	

	for (Square s = A1; s <= H8; ++s) for (PieceType pt: { ROOK, BISHOP })
	{
		const uint64_t edges = ((FILEA_BB | FILEH_BB) & ~file_bb(s)) | ((RANK1_BB | RANK8_BB) & ~rank_bb(s));
		const uint64_t mask = sliding_attack(pt, s, 0) & ~edges;
		const int table_size = 1 << popcount(mask);
		std::vector<uint64_t>& blocker_table = pt == ROOK ? ROOK_BLOCKERS[s] : BISHOP_BLOCKERS[s];
		std::vector<uint64_t>& moves_table = pt == ROOK ? ROOK_LEGAL_MOVES[s] : BISHOP_LEGAL_MOVES[s];

		for (uint64_t blocker_board = 0, i = 0; i < table_size; ++i)
		{
			blocker_table.push_back(blocker_board);
			moves_table.push_back(sliding_attack(pt, s, blocker_board));
			blocker_board = (blocker_board - mask) & mask;
		}
	}
	init_magic(H7, ROOK);
	write_magics();

	std::cout << std::hex << table_size(ROOK) << std::endl;	
	std::cout << std::hex << table_size(BISHOP) << std::endl;	

	return 0;
}
