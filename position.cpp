#include <string_view>
#include <sstream>
#include "bitboard.h"
#include "position.h"


constexpr std::string_view PIECE_TO_CHAR(" PNBRQK  pnbrqk ");


Position& Position::seed(const std::string& fen, Gamestate& gs)
{
	// A FEN string represents a chess position using six fields seperated by a space:
	// Piece placement, Side to move, Castling rights, En passant target, Halfmoves, Fullmoves
	// For more information: https://en.wikipedia.org/wiki/Forsyth–Edwards_Notation

	this->state = &gs;
	this->clear_board();
	
	char token;
	std::istringstream ss(fen);
	ss >> std::noskipws;
	
	// 1) Piece placement
	for (Square s = A8; (ss >> token) && !isspace(token); )
	{
		if (isdigit(token)) // skip empty squares represented by digits
			s += (token - '0') * Direction::E;
		else if (token == '/') // newline fen character, must move two squares south
			s += Direction::SS;
		else // place a piece
		{
			this->place_piece(Piece(PIECE_TO_CHAR.find(token)), s);
			++s;
		}
	}

	// 2) Side to move
	ss >> token;
	this->turn = token == 'w' ? WHITE : BLACK;
	ss >> token;

	// 3) Castling rights
	for (this->state->castling_rights = 0; ss >> token && !isspace(token); )
	{
		switch (token)
		{
			case 'K': state->castling_rights |= WHITE_OO; break;
			case 'Q': state->castling_rights |= WHITE_OOO; break;
			case 'k': state->castling_rights |= BLACK_OO; break;
			case 'q': state->castling_rights |= BLACK_OOO; break;
			default: break;
		}
	}

	// 4) En passant target
	uint8_t row, col;
	bool en_passant = false;
	if (((ss >> col) && (col >= 'a' && col <= 'h')) && ((ss >> row) && (row == (turn == WHITE ? '6' : '3'))))
	{
		this->state->ep_square = make_square(File(col - 'a'), Rank(row - '1'));
		en_passant = !(this->pieces() & (state->ep_square | (state->ep_square + pawn_push(turn))))
				    && pieces(~turn, PAWN) & (state->ep_square + pawn_push(~turn))
					&& pawn_attacks_bb(~turn, square_to_bb(state->ep_square)) & pieces(turn, PAWN);
	}
	if (!en_passant) state->ep_square = NO_SQUARE;

	// 5) Halfmoves and 6) Fullmoves
	ss >> std::skipws >> state->rule_50 >> gameply;
	gameply = 2 * (gameply - 1) + this->turn;
	
	return *this;
}

std::string Position::fen() const
{
	std::ostringstream  ss;

	// 1) Piece placement
	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			int empty_count = 0;
			for (; f <= FILE_H && mailbox[make_square(f, r)] == NO_PIECE; ++f) { ++empty_count; }
			if (empty_count) { ss << empty_count; }
			if (f <= FILE_H) { ss << PIECE_TO_CHAR[mailbox[make_square(f, r)]]; }
		}
		if (r > RANK_1) { ss << '/'; }
	}

	// 2) Side to move
	ss << (this->turn == WHITE ? " w " : " b ");

	// 3) Castling rights
	if (this->state->castling_rights & WHITE_OO)  { ss << 'K'; }
	if (this->state->castling_rights & WHITE_OOO) { ss << 'Q'; }
	if (this->state->castling_rights & BLACK_OO)  { ss << 'k'; }
	if (this->state->castling_rights & BLACK_OOO) { ss << 'q'; }
	if (this->state->castling_rights == 0) { ss << '-'; }

	// 4) En passant target
	ss << ' ' << sq_to_string(this->state->ep_square) << ' ';

	// 5) Halfmoves and 6) Fullmoves
	ss << state->rule_50 << ' ' << ((this->gameply - (turn == BLACK)) / 2 + 1); 

	return ss.str();
}

std::string Position::to_string() const
{
	const std::string newline = "+---+---+---+---+---+---+---+---+\n";
	std::string s = newline;

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			std::string newsquare = "|   ";
			newsquare[2] = PIECE_TO_CHAR[piece_on(make_square(f, r))];
			s += newsquare;
		}
		s += "| " + std::to_string(r + 1) + '\n' + newline;
	}

	return s + "  a   b   c   d   e   f   g   h";
}


void Position::make_move(const Move move, Gamestate& gs, bool is_check)
{
	std::memcpy(&gs, this->state, sizeof(Gamestate));
	gs.previous = this->state;
	this->state = &gs;

	const Square from = move.from();
	const Square to = move.to();
	const Piece pc = this->mailbox[from];
	const Piece captured_pc = move.type() == EN_PASSANT ? this->mailbox[to + pawn_push(~turn)] : this->mailbox[to];

	++this->gameply;
	++this->state->rule_50;
	
	if (move.type() == CASTLES)
	{
		const Direction d = from > to ? Direction::W : Direction::E;
		remove_piece(from);
		remove_piece(to);
		place_piece(this->turn, ROOK, from + d);
		place_piece(this->turn, KING, from + 2 * d);
		this->state->castling_rights &= ~CASTLING_RIGHTS[this->turn];
	}
	else if (captured_pc != NO_PIECE)
	{
		if (move.type() == EN_PASSANT)
			remove_piece(to + pawn_push(~this->turn));
		else
			remove_piece(to);
		this->state->rule_50 = 0;
	}

	if (move.type() != CASTLES)
	{
		this->mailbox[to] = pc;
		this->mailbox[from] = NO_PIECE;
		this->bitboards[pc] ^= square_to_bb(from) | to;
	}

	this->state->ep_square = NO_SQUARE;

	if (type_of(pc) == PAWN)
	{
		if (SQUARE_DISTANCE[to][from] == 2 && pawn_attacks_bb(turn, square_to_bb(to-pawn_push(turn))) & pieces(~turn, PAWN))
			state->ep_square = to - pawn_push(turn);

		else if (move.type() == PROMOTION)
		{
			remove_piece(to);
			place_piece(turn, move.promotion_type(), to);
		}
		this->state->rule_50 = 0;
	}
	switch (from)
	{
		case E1: state->castling_rights &= ~WHITE_CASTLES; break;
		case E8: state->castling_rights &= ~BLACK_CASTLES; break;
		case A1: state->castling_rights &= ~WHITE_OOO; break;
		case H1: state->castling_rights &= ~WHITE_OO; break;
		case A8: state->castling_rights &= ~BLACK_OOO; break;
		case H8: state->castling_rights &= ~BLACK_OO; break;
		default: break;
	}
	switch (to)
	{
		case A1: state->castling_rights &= ~WHITE_OOO; break;
		case H1: state->castling_rights &= ~WHITE_OO; break;
		case A8: state->castling_rights &= ~BLACK_OOO; break;
		case H8: state->castling_rights &= ~BLACK_OO; break;
		default: break;
	}

	this->turn ^= 1;
}


inline void Position::place_piece(Piece pc, Square s)
{
	this->mailbox[s] = pc;
	this->bitboards[pc | ALL_PIECE_TYPES] |= this->bitboards[pc] |= s;
}

inline void Position::remove_piece(Square s)
{
	this->bitboards[this->mailbox[s]] ^= s;
	this->mailbox[s] = NO_PIECE;
}

inline void Position::clear_board()
{
	for (Square s = A1; s <= H8; ++s) { mailbox[s] = NO_PIECE; }
	for (uint64_t& bb: bitboards) { bb = 0ull; }
}
