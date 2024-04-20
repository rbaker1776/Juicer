#include "position.h"



void Position::init()
{{{
	
}}}


Position& Position::seed(const std::string& fen)
{{{
	// 0:		"rnbqkbnr/pppppppp/8/8/8/8/pppppppp/rnbqkbnr w kqkq - 0 1"
	// 1: e4	"rnbqkbnr/pppppppp/8/8/4p3/8/pppp1ppp/rnbqkbnr b kqkq e3 0 1"
	// 1: c5	"rnbqkbnr/pp1ppppp/8/2p5/4p3/8/pppp1ppp/rnbqkbnr w kqkq c6 0 2"
	// 2: nf3	"rnbqkbnr/pp1ppppp/8/2p5/4p3/5n2/pppp1ppp/rnbqkb1r b kqkq - 1 2"
	this->clear_board();

	Square write = SQ_a8;
	std::string::iterator c;

		assert((*c)++ == ' ');
	assert(*c = 'w' || *c == 'b');
	this->white_to_move = *c == 'w';

	assert((*c)++ == ' ');
	for (; *c < s.end() && *c != ' ')
	{
		switch (c)
		{
			case 'K':
				this->Gamestate.castling_rights |= CASTLE_WK;
				break;
			case 'Q':
				this->Gamestate.castling_right

	return *this;
}}}


bool Position::place_pieces(const std::string& s)
{{{
	for (c = fen.begin();  c < s.end() && *c != ' '; ++c)
	{
		if (*c > '0' && *c <= '8') 
		{ 
			write += *c - '0';
		}
		else if (*c == '/')
		{
			continue;
		}
		else
		{
			Piece p = char_to_pc(*c);
			switch(p)
			{
				case W_PAWN: 
					armies[WHITE] |= pieces[PAWN] |= s;
					break;
				case W_KNIGHT:
					armies[WHITE] |= pieces[KNIGHT] |= s;
					break;
				case W_BISHOP:
					armies[WHITE] |= pieces[BISHOP] |= s;
					break;
				case W_ROOK: 
					armies[WHITE] |= pieces[ROOK] |= s;
					break;
				case W_QUEEN:
					armies[WHITE] |= pieces[QUEEN] |= s;
					break;
				case W_KING: 
					armies[WHITE] |= pieces[KING] |= s;
					break;
				case B_PAWN: 
					armies[BLACK] |= pieces[PAWN] |= s;
					break;
				case B_KNIGHT:
					armies[BLACK] |= pieces[KNIGHT] |= s;
					break;
				case B_BISHOP:
					armies[BLACK] |= pieces[BISHOP] |= s;
					break;
				case B_ROOK: 
					armies[BLACK] |= pieces[ROOK] |= s;
					break;
				case B_QUEEN:
					armies[BLACK] |= pieces[QUEEN] |= s;
					break;
				case B_KING: 
					armies[BLACK] |= pieces[KING] |= s;
					break;
				default:
					return false;		
			}
		}
	}
}}}


bool set_turn(char c)
{
	


void Position::clear_board()
{{{
	pieces[0] = pieces[1] =	pieces[2] =	pieces[3] =	
	pieces[4] =	pieces[5] =	armies[0] =	armies[1] = 0ull;
}}}


void Position::put(Piece p, Square s)
{{{

	}
}}}

