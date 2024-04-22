#include "position.h"
#include "types.h"



Position& Position::seed(const std::string& fen)
{{{
	// 0:		"rnbqkbnr/pppppppp/8/8/8/8/pppppppp/rnbqkbnr w kqkq - 0 1"
	// 1: e4	"rnbqkbnr/pppppppp/8/8/4p3/8/pppp1ppp/rnbqkbnr b kqkq e3 0 1"
	// 1: c5	"rnbqkbnr/pp1ppppp/8/2p5/4p3/8/pppp1ppp/rnbqkbnr w kqkq c6 0 2"
	// 2: nf3	"rnbqkbnr/pp1ppppp/8/2p5/4p3/5n2/pppp1ppp/rnbqkb1r b kqkq - 1 2"
	this->pieces[0] = this->pieces[1] = this->pieces[2] = this->pieces[3] = 
	this->pieces[4] = this->pieces[5] = this->armies[0] = this->armies[1] = 0ull;
	this->gamestate.castling_rights = 0;
	this->gamestate.half_clock = this->gamestate.full_clock = 0;
	this->gamestate.ep_square = NO_SQUARE;
	Square write = Square::A8;

	enum FENStringField
	{
		PIECES,
		TO_MOVE,
		CASTLING_RIGHTS,
		EP_SQUARE,
		HALF_CLOCK,
		FULL_CLOCK,
	};
	
	FENStringField field = PIECES;
	Square write_to = Square::A8;

	for (char c: fen)
	{
		if (c == ' ')
		{
			++c;
			++field;
		}
		switch(field)
		{
			case PIECES:
				if (c == '/') { write_to -= 15; }
				else if (c >= '1' && c <= '8') { write_to += c - '0'; }
				else 
				{
					const PieceType pt = char_to_pt(c);
					const Color cr = char_to_color(c);
					this->pieces[pt] |= write_to;
					this->armies[cr] |= write_to;
				}
				break;
			case TO_MOVE:
				this->white_to_move = c == 'w';
				break;
			case CASTLING_RIGHTS:
				this->gamestate.castling_rights |= char_to_castle(c);
				break;
			case EP_SQUARE:
				this->gamestate.ep_square = make_square(File(c), Rank(++c));
				break;
			case HALF_CLOCK:
				this->gamestate.half_clock *= 10;
				this->gamestate.half_clock += c - '0';
				break;
			case FULL_CLOCK:
				this->gamestate.full_clock *= 10;
				this->gamestate.full_clock += c - '0';
				break;
			default:
				break;
		}
	}
}}}

