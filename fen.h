#ifndef FEN_H_2243825804A6
#define FEN_H_2243825804A6

#include <string_view>
#include <cassert>
#include "bitboard.h"
#include "types.h"
#include "juicer.h"


namespace FEN
{
	static constexpr std::string_view STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	static constexpr std::string_view PIECES = " PNBRQK  pnbrqk";

	static constexpr uint64_t bitboard(std::string_view fen, Piece pc);
	static constexpr Color turn(std::string_view fen);
	static constexpr uint64_t castling_rights(std::string_view fen);
	static constexpr Square ep_target(std::string_view fen);
	static constexpr int halfmoves(std::string_view fen);
} // namespace FEN


static constexpr uint64_t FEN::bitboard(std::string_view fen, Piece pc)
{
	const char* token = &fen[0];
	uint64_t bb = 0;

	for (Square s = A8; !(*token == ' '); ++token)
	{
		if (*token >= '1' && *token <= '8')
			s += Direction((*token - '0') * Direction::E);
		else if (*token == '/')
			s += Direction::SS;
		else if (FEN::PIECES[pc] == *token)
		{
			bb |= s;
			++s;
		}
		else
			++s;
	}

	return bb;
}

static constexpr Color FEN::turn(std::string_view fen)
{
	const char* token = &fen[0];
	
	while (!(*token++ == ' ')) {}

	return (*token == 'w' ? WHITE : BLACK);
}

static constexpr uint64_t FEN::castling_rights(std::string_view fen)
{
	const char* token = &fen[0];
	uint64_t cr = 0;

	for (int ws = 0; ws < 2; ws += (*token++ == ' ')) {}

	for (; !(*token == ' '); ++token)
	{
		switch (*token)
		{
			case 'K': cr |= Castling::W_OO;  break;
			case 'Q': cr |= Castling::W_OOO; break;
			case 'k': cr |= Castling::B_OO;  break;
			case 'q': cr |= Castling::B_OOO; break;
		}
	}

	return cr;
}

static constexpr Square FEN::ep_target(std::string_view fen)
{
	const char* token = &fen[0];

	while (!(*token++ == ' ')) {}

	Color turn = *token++ == 'w' ? WHITE : BLACK;

	++token;
	while (!(*token++ == ' ')) {}

	char rank, file;
	if (((file = *token++) && (file >= 'a' && file <= 'h')) && ((rank = *token++) && (rank >= '1' && rank <= '8')))
		return make_square(File(file - 'a'), Rank(rank - '1')) + (turn == WHITE ? Direction::S : Direction::N);

	return NO_SQUARE;
}

static constexpr int FEN::halfmoves(std::string_view fen)
{
	const char* token = &fen[0];
	int rule_50 = 0;

	for (int i = 0; i < 4; i += (*token++ == ' ')) {}

	while (*token++ != ' ')
	{
		rule_50 *= 10;
		rule_50 += *token - '0';
	}

	return rule_50;
}


#endif // FEN_H_2243825804A6
