#include "position.h"



Position& Position::seed(const std::string& fen)
{{{
	// 0:		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
	// 1: e4	"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"
	// 1: c5	"rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"
	// 2: nf3	"rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"
	this->pieces[0] = this->pieces[1] = this->pieces[2] = this->pieces[3] = 
	this->pieces[4] = this->pieces[5] = this->armies[0] = this->armies[1] = 0ull;
	this->gamestate->castling_rights = 0u;
	this->gamestate->half_clock = this->gamestate->full_clock = 0;
	this->gamestate->ep_square = NO_SQUARE;

	auto c = fen.begin();

	for (Square s = SQ_a8; *c != ' '; ++c)
	{
		if (*c == '/')
		{
			s = make_square(FILE_A, Rank(rank_of(s)-2));
		}
		else if (*c >= '1' && *c <= '8') { s += *c - '0'; }
		else
		{
			const PieceType pt = char_to_pt(*c);
			const Color cr = char_to_color(*c);
			this->pieces[pt] |= s;
			this->armies[cr] |= s;
			++s;
		}
	}
	++c;
	
	this->white_to_move = *c == 'w';
	++c;
	++c;

	if (*c != '-')
	{
		for (; *c != ' '; ++c)
		{
			this->gamestate->castling_rights |= char_to_castle(*c);
		}
	}
	else { ++c; }
	++c;

	if (*c != '-')
	{
		this->gamestate->ep_square = make_square(char_to_file(*c), char_to_rank(*(c+1)));
		++c;
	}	
	++c;
	++c;

	for (; *c != ' '; ++c)
	{
		this->gamestate->full_clock *= 10;
		this->gamestate->full_clock += *c - '0';
	}
	++c;

	for (; *c != '\0' && *c != ' '; ++c)
	{
		this->gamestate->half_clock *= 10;
		this->gamestate->half_clock += *c - '0';
	}

	return *this;
}}}


std::string Position::to_string() const
{{{ 
	const std::string newline = "+---+---+---+---+---+---+---+---+\n";
	std::string s = newline;

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			s += "|   ";
			*(s.end()-2) = piece_to_char(this->get_piece(make_square(f, r)));
		}
		s += "| " + std::to_string(1+r) + '\n' + newline;
	}

	s += "  a   b   c   d   e   f   g   h";
	return s;
}}}


std::string Position::to_fen() const
{{{
	std::string fen = "";
	int whitespace = 0;

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			while (this->get_piece(make_square(f, r)) == NO_PIECE && f <= FILE_H)
			{
				++f;
				++whitespace;
			}

			if (whitespace)
			{
				fen += '0' + whitespace;
				whitespace = 0;
				--f;
			}
			else
			{
				fen += " ";
				*(fen.end()-1) = piece_to_char(this->get_piece(make_square(f, r)));
			}
		}
		if (r != RANK_1) { fen += "/"; }
	}
	fen += " ";

	fen += this->white_to_move ? "w " : "b ";
	
	for (auto castle: { WHITE_SHORT, WHITE_LONG, BLACK_SHORT, BLACK_LONG })
	{
		if (castle & this->gamestate->castling_rights)
		{
			fen += " ";
			*(fen.end()-1) = castle_to_char(castle);
		}
	}
	if (this->gamestate->castling_rights == 0)
	{
		fen += "-";
	};
	fen += " ";

	if (this->gamestate->ep_square != NO_SQUARE)
	{
		fen += " ";
		*(fen.end()-1) = file_to_char(file_of(this->gamestate->ep_square));
		fen += " ";
		*(fen.end()-1) = rank_to_char(rank_of(this->gamestate->ep_square));
		fen += " ";
	}
	else
	{
		fen += "- ";
	}
	
	fen += std::to_string(this->gamestate->full_clock) + " ";
	fen += std::to_string(this->gamestate->half_clock);

	return fen;
}}}

