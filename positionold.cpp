#include <iostream>
#include <sstream>
#include <string_view>
#include "position.h"


constexpr std::string_view PIECE_TO_CHAR(" PNBRQK  pnbrqk");

Position& Position::seed(const std::string& fen, Gamestate& gs)
{
	// 1) Piece placement - each rank is described starting with 8 and going down to 1.
	//    Within each rank, the contents of each square are described from A to H using 
	//    a single character. White pieces are "PNBRQK" and black pieces are "pnbrqk".
	//    Empty squares are denoted with a number representing the number of consequtive 
	//    squares. '/' seperates ranks/.
	//
	// 2) Turn - 'w' means white to move, 'b' means black to move.
	//
	// 3) Castling rights - describes the set of all legal castling options left in the
	//    game. "K" means white can castle kingside, and 'q' means black can castle 
	//    queenside. "KQkq" means both players can castle both directions. '-' means 
	//    neither player has the right to castle. 
	//
	// 4) En passant target - if there is no legal en passan move, this field is '-'.
	//    If a pawn pushed two squares on the previous move, this is the position behind 
	//    the pawn. For example, following the move e2-e4, this field would be "e3".
	//
	// 5) Halfmove clock - this is the number of halfmoves since the last pawn push or 
	//    capture. Used for the fifty-move rule.
	//
	// 6) Fullmove number - the number of full moves played. This field is '1' at the 
	//    starting position and gets incremented after each move.
	//
	// For example:
	// 	starting position: 
	// 	  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
	//	Fisher-Spasky game 6 after 12. c5: 
	//	  "rn3rk1/p3qpp1/1p2b2p/2pp4/Q2P4/4PN2/PP3PPP/2R1KB1R w K - 0 13"
	
	this->gamestate = &gs;
	this->reset();

	char token;
	size_t idx;
	std::istringstream ss(fen);
	ss >> std::noskipws;

	// 1) Piece placement
	for (Square s = A8; (ss >> token) && !isspace(token); )
	{
		if (isdigit(token)) s += (token - '0') * E;
		else if (token == '/') s += SS;
		else if ((idx = PIECE_TO_CHAR.find(token)) != std::string::npos)
		{
			this->place_piece(Piece(idx), s);
			++s;
		}
	}

	// 2) Turn
	ss >> token;
	this->side_to_move = token == 'w' ? WHITE : BLACK;
	ss >> token;

	// 3) Castling rights
	while ((ss >> token) && !isspace(token))
	{	
		switch (token)
		{
			case 'K': this->gamestate->castling_rights |= WHITE_OO; break;
			case 'Q': this->gamestate->castling_rights |= WHITE_OOO; break;
			case 'k': this->gamestate->castling_rights |= BLACK_OO; break;
			case 'q': this->gamestate->castling_rights |= BLACK_OOO; break;
			default: break;
		}
	}

	// 4) En passant target
	bool en_passant = false;
	uint8_t row, col;
	if (((ss >> col) && (col >= 'a' && col <= 'h')) && ((ss >> row) && (row == (side_to_move == WHITE ? '6' : '3'))))
	{
		gamestate->ep_square = make_square(File(col - 'a'), Rank(row - '1'));
		en_passant = !(pieces() & (gamestate->ep_square | (gamestate->ep_square + pawn_push(side_to_move))))
				  && pieces(~side_to_move, PAWN) & (gamestate->ep_square + pawn_push(~side_to_move))
				  && pawn_attacks_bb(~side_to_move, square_to_bb(gamestate->ep_square)) & pieces(side_to_move, PAWN);
	}
	if (!en_passant) gamestate->ep_square = NO_SQUARE;

	// 5) Halfmove clock
	ss >> token;
	while ((ss >> token) && !isspace(token))
	{
		this->gamestate->halfmoves *= 10;
		this->gamestate->halfmoves += token - '0';
	}

	// Fullmove number
	while ((ss >> token) && !isspace(token))
	{
		gameply *= 10;
		gameply += token - '0';
	}
	gameply = std::max(2 * (gameply - 1), 0) + (side_to_move == BLACK);

	return *this;
}


void Position::make_move(Move m, Gamestate& gs, bool check)
{
	gs.previous = this->gamestate;
	this->gamestate = &gs;

	++gameply;
	++gamestate->halfmoves;

	this->bitboards[this->mailbox[m.from()]] ^= 
}


std::string pos_to_string(const Position& pos)
{
	const std::string newline = "+---+---+---+---+---+---+---+---+\n";
	std::string s = newline;

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			std::string newsquare = "|   ";
			newsquare[2] = PIECE_TO_CHAR[pos.piece_on(make_square(f, r))];
			s += newsquare;
		}
		s += "| " + std::to_string(r + 1) + '\n' + newline;
	}

	return s + "  a   b   c   d   e   f   g   h";
}
